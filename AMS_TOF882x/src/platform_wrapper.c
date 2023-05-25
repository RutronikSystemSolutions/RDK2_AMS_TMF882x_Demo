/*
*****************************************************************************
* Copyright by ams AG                                                       *
* All rights are reserved.                                                  *
*                                                                           *
* IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
* THE SOFTWARE.                                                             *
*                                                                           *
* THIS SOFTWARE IS PROVIDED FOR USE ONLY IN CONJUNCTION WITH AMS PRODUCTS.  *
* USE OF THE SOFTWARE IN CONJUNCTION WITH NON-AMS-PRODUCTS IS EXPLICITLY    *
* EXCLUDED.                                                                 *
*                                                                           *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
*****************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "mcu_tmf882x_config.h"
#include "platform_wrapper.h"
#include "tmf882x.h"
#include "tmf882x_interface.h"

#define TMF882X_CAL_ITERATIONS        4000
#define TMF882X_8X8_ITERATIONS        125
#define TMF882X_DEFAULT_CONF_TH       6
#define TMF882X_DEFAULT_ITERATIONS    550
#define TMF882X_DEFAULT_REP_PERIOD_MS 30


/**
 * Following defines are needed to display the values of the sensor in case
 * the 8x8 mode is selected
 */
#define NUM_MEAS_MSG_IN_8X8		  4
#define NUM_ROWS_IN_8X8           8
#define NUM_COLS_IN_8X8           8

static void print_result(struct platform_ctx *ctx, struct tmf882x_msg_meas_results *result_msg)
{
    if (!ctx || !result_msg)
    {
    	return;
    }

	if (ctx->mode_8x8 != 0)
	{
		static struct tmf882x_msg_meas_results msgs[NUM_MEAS_MSG_IN_8X8] = {0};
		uint32_t distances_cm[64] = { 0 };
		uint32_t msg_idx = 0;
		uint32_t row = 0, col = 0, ch = 0;

		// which measurement in the 4x group is this?
		msg_idx = result_msg->result_num % NUM_MEAS_MSG_IN_8X8;

		// Buffer up 4 messages (for 64 zones)
		memcpy(&msgs[msg_idx], result_msg, sizeof(*msgs));

		if (msg_idx != (NUM_MEAS_MSG_IN_8X8 - 1))
			return;	// wait until we have all measurement messages in an 8x8 group before logging

		// Map 4 measurement messages to one combined 64 pixel depth map
		for (uint32_t idx = 0; idx < NUM_MEAS_MSG_IN_8X8; ++idx) {
			for (uint32_t res = 0; res < msgs[idx].num_results; ++res) {

				// Result Number, SubCapture, Channel indicate 1:64 zone mapping
				ch = msgs[idx].results[res].channel;
				row = col = 0;

				row += (1 - (ch - 1) / 4) * 4;
				row += (1 - (((ch - 1) % 4) / 2)) * 2;
				row += (1 - idx/2);

				col += (1 - (ch % 2)) * 4;
				col += (idx % 2) * 2;
				col += msgs[idx].results[res].sub_capture;

				distances_cm[row * NUM_COLS_IN_8X8 + col] = msgs[idx].results[res].distance_mm / 10;
			}
		}

		// Print 8x8 distance results (1st object only)
		for (uint32_t idx = 0; idx < sizeof(distances_cm)/sizeof(*distances_cm); ++idx) {
			printf("%4.0lu", distances_cm[idx]);
			if (((idx+1) % 8) == 0) { // print in 8x8 grid
				printf("\r\n");
			}
		}

		printf("\r\n\r\n");
	}
	else
	{
		printf("\x1b[2J\x1b[;H");
		printf("\n\r");
		printf("    TMF8820 3x3 multi-zone Time-of-Flight Sensor\n\r");
		printf("measurement_num: %u num_results: %u\n\r",
				(unsigned int)result_msg->result_num,
				(unsigned int)result_msg->num_results);

		for (uint32_t i = 0; i < result_msg->num_results; ++i)
		{
			printf("conf: %u distance_mm: %u channel: %u sub_capture: %u\n\r",
					(unsigned int)result_msg->results[i].confidence,
					(unsigned int)result_msg->results[i].distance_mm,
					(unsigned int)result_msg->results[i].channel,
					(unsigned int)result_msg->results[i].sub_capture);
		}

		printf("photon: %u ref_photon: %u ALS: %u\n\r",
				(unsigned int)result_msg->photon_count,
				(unsigned int)result_msg->ref_photon_count,
				(unsigned int)result_msg->ambient_light);
	}
}

int32_t platform_wrapper_power_on(struct platform_ctx *ctx)
{
    cyhal_gpio_write(ARDU_IO4, true);
    return 0;
}

void platform_wrapper_power_off(struct platform_ctx *ctx)
{
    cyhal_gpio_write(ARDU_IO4, false);
    usleep(5000); // let the device fully power off
}

int32_t platform_wrapper_init_device(struct platform_ctx *ctx,
                                const uint8_t *hex_records,
                                uint32_t hex_size)
{
    int32_t rc = 0;
    uint8_t app_ver[32] = {0};

    if (!ctx) return -1;
    tmf882x_init(ctx->tof, ctx);

    // Open the driver
    if (tmf882x_open(ctx->tof)) {
        fprintf(stderr, "%s Error opening driver\n", __func__);
        return -1;
    }

    // if FWDL file is supplied, use that
    if (hex_records && hex_size) {

        // Do a mode switch to the bootloader (bootloader mode necessary for FWDL)
        if (tmf882x_mode_switch(ctx->tof, TMF882X_MODE_BOOTLOADER)) {
            fprintf(stderr, "%s mode switch failed\n", __func__);
            tmf882x_dump_i2c_regs(tmf882x_mode_hndl(ctx->tof));
            return -1;
        }

        rc = tmf882x_fwdl(ctx->tof, FWDL_TYPE_HEX, hex_records, hex_size);
        if (rc) {
            fprintf(stderr, "Error (%ld) performing FWDL with hex records\n",rc);
            return rc;
        }

    }
#ifdef _HAS_BUILTIN_FW
    // else if FWDL is built-in, use that
    else if(tof_bin_image && tof_bin_image_length) {

        printf("Using builtin fw image start addr: 0x%08x size: 0x%08x\n",
               tof_bin_image_start, tof_bin_image_length);

        // Do a mode switch to the bootloader (bootloader mode necessary for FWDL)
        if (tmf882x_mode_switch(ctx->tof, TMF882X_MODE_BOOTLOADER)) {
            fprintf(stderr, "%s mode switch failed\n", __func__);
            tmf882x_dump_i2c_regs(tmf882x_mode_hndl(ctx->tof));
            return -1;
        }

        rc = tmf882x_fwdl(ctx->tof, FWDL_TYPE_BIN, tof_bin_image, tof_bin_image_length);
        if (rc) {
            fprintf(stderr, "Error (%d) performing FWDL with built-in firmware\n", rc);
            return rc;
        }

    }
#endif
    // else use builtin ROM image
    else {

        // Mode switch while in bootloader mode tries to load the requested
        //  mode from ROM/Flash/etc since FWDL failed / is not available
        if (tmf882x_mode_switch(ctx->tof, TMF882X_MODE_APP)) {
            fprintf(stderr, "%s ROM switch to APP mode failed\n", __func__);
            tmf882x_dump_i2c_regs(tmf882x_mode_hndl(ctx->tof));
            return -1;
        }

    }

    // Make sure we are running application mode
    if  (tmf882x_get_mode(ctx->tof) != TMF882X_MODE_APP) {
        fprintf(stderr, "%s failed to open APP mode\n", __func__);
        return -1;
    }

    if (tmf882x_get_firmware_ver(ctx->tof, app_ver, sizeof(app_ver)) > 0)
        printf("Firmware Application mode version: %u.%u.%u.%u\n",
               app_ver[0], app_ver[1], app_ver[2], app_ver[3]);

    return 0;
}

int32_t platform_wrapper_cfg_device(struct platform_ctx *ctx)
{
    struct tmf882x_mode_app_config tof_cfg;
    int32_t error;
    if (!ctx) {
        fprintf(stderr, "Error null context pointer\n");
        return -1;
    }

    // get current config
    error = tmf882x_ioctl(ctx->tof, IOCAPP_GET_CFG, NULL, &tof_cfg);
    if (error) {
        fprintf(stderr, "Error, app get config failed.\n");
        return error;
    }

    if (ctx->mode_8x8) {

        error = tmf882x_ioctl(ctx->tof, IOCAPP_SET_8X8MODE, &ctx->mode_8x8, NULL);
        if (error) {
            fprintf(stderr, "Error setting 8x8 mode\n");
            return error;
        }

        // Change the iterations for 8x8 mode
        tof_cfg.kilo_iterations = TMF882X_8X8_ITERATIONS;

    }

    // commit new config here
    // Note: config should be set after 8x8 mode since setting 8x8 mode will
    //       clear the current config
    error = tmf882x_ioctl(ctx->tof, IOCAPP_SET_CFG, &tof_cfg, NULL);
    if (error) {
        fprintf(stderr, "Error, setting configuration\n");
        return error;
    }

    return error;
}

int32_t platform_wrapper_factory_calibration(struct platform_ctx *ctx,
                                             struct tmf882x_mode_app_calib *tof_calib)
{
    struct tmf882x_mode_app_config tof_cfg;
    int32_t error;
    if (!ctx || !tof_calib) {
        fprintf(stderr, "Error null context pointer\n");
        return -1;
    }

    error = tmf882x_ioctl(ctx->tof, IOCAPP_GET_CFG, NULL, &tof_cfg);
    if (error) {
        fprintf(stderr, "Error, app get config failed.\n");
        return error;
    }

    // Change the iterations for Factory Calibration
    tof_cfg.kilo_iterations = TMF882X_CAL_ITERATIONS;

    error = tmf882x_ioctl(ctx->tof, IOCAPP_SET_CFG, &tof_cfg, NULL);
    if (error) {
        fprintf(stderr, "Error, setting calibration iterations\n");
        return error;
    }

    return tmf882x_ioctl(ctx->tof, IOCAPP_DO_FACCAL, NULL, tof_calib);
}

void platform_wrapper_start_measurements(struct platform_ctx *ctx,
                                    uint32_t num_measurements,
                                    const struct tmf882x_mode_app_calib *tof_calib)
{
    int32_t rc;
    if (!ctx) {
        fprintf(stderr, "Error null context pointer\n");
        return;
    }

    ctx->curr_num_measurements = 0;

    //set fac cal
    if (tof_calib && tof_calib->calib_len) {
        rc = tmf882x_ioctl(ctx->tof, IOCAPP_SET_CALIB, tof_calib, NULL);
        if (rc) {
            fprintf(stderr, "Error setting calibration data\n");
            return;
        }
    }

    //start measurements
    rc = tmf882x_start(ctx->tof);
    if (rc) {
        fprintf(stderr, "Error starting measurements\n");
        return;
    }

    while(1) {
        rc = tmf882x_process_irq(ctx->tof);
        if (rc == -1) {
            fprintf(stderr, "Error processing IRQ\n");
            (void) tmf882x_stop(ctx->tof);
            return;
        }

        usleep(5000); // poll period
        if (num_measurements &&
            ctx->curr_num_measurements == num_measurements) {
            printf("Read %lu results\n", num_measurements);
            break;
        }
    }

    tmf882x_stop(ctx->tof);
    return;
}

int32_t platform_wrapper_handle_msg(struct platform_ctx *ctx,
                               struct tmf882x_msg *msg)
{
    if (!ctx || !msg) return -1;

    if (msg->hdr.msg_id == ID_MEAS_RESULTS) {
        ctx->curr_num_measurements++;
        print_result(ctx, &msg->meas_result_msg);
    }
    return 0;
}

int32_t platform_wrapper_write_i2c_block(struct platform_ctx *ctx, uint8_t reg,
                                const uint8_t *buf, uint32_t len)
{
    if (!ctx) return -1;
    return ams_i2c_write_block(ctx->i2c_addr, reg, buf, len);
}

int32_t platform_wrapper_read_i2c_block(struct platform_ctx *ctx, uint8_t reg,
                               uint8_t *buf, uint32_t len)
{
    if (!ctx) return -1;
    return ams_i2c_read_block(ctx->i2c_addr, reg, buf, len);
}
