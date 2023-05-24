/*
 * tmf882x_example.c
 *
 *  Created on: Aug 18, 2020
 *      Author: orya
 */

#include <stdint.h>

#include "tmf882x.h"
#include "tmf882x_interface.h"
#include "tof_bin_image.h"
#include "tof_factory_cal.h"

#include "cyhal.h"
#include "cybsp.h"
#include "cy_pdl.h"

#define TMF882X_DEMO_MAJ_VER      1
#define TMF882X_DEMO_MIN_VER      7

#define QUOTE(x)                  #x
#define STRINGIFY(x)              QUOTE(x)
#define TMF882X_DEMO_VER          STRINGIFY(TMF882X_DEMO_MAJ_VER) "." \
                                  STRINGIFY(TMF882X_DEMO_MIN_VER)

#define TMF882X_I2C_ADDR          0x41
#define PERIODIC_UTICK_CNT        601
#define POLL_PERIOD_MS            20000
#define DEBOUNCE_PERIOD_MS        50000
#define SW_DEBOUNCE_CNT           (((DEBOUNCE_PERIOD_MS) / (POLL_PERIOD_MS)) + 1)

// Wrapper for printf
#define PRINTF printf

/**
 * If defined, then the 8x8 mode will be used
 * If not defined, the 3x3 mode will be used
 */
#undef USE_8X8_MODE

/***************************************************
 *     0: only output results/data from device
 *     1: output info level messages from core driver
 *     2+: output debug level messages from core driver
 ***************************************************/
const uint32_t g_core_drv_logging = 1;


// TMF882x core driver context
static struct tmf882x_tof tof;

// TMF882x core driver APP mode-specific config structure
static struct tmf882x_mode_app_config tofcfg;

extern cyhal_timer_t Systick_obj;

struct platform_ctx tof_ctx =
	{
		.curr_num_measurements = 0,
		.debug = g_core_drv_logging,
		.gpio_ce = 0,
		.gpio_irq = 0,
		.i2c_addr = TMF882X_I2C_ADDR,
		.i2cdev = "AMS_TOF_RUTRONIK",
#ifdef USE_8X8_MODE
		.mode_8x8 = 1,
#else
		.mode_8x8 = 0,
#endif
		.tof = &tof,
	};

/*
 * This function provides the core driver a way to retrieve relative
 * timestamps at microsecond resolution.
 */
uint32_t get_uticks(void) {
    return cyhal_timer_read(&Systick_obj);
}

static void power_on_tmf882x(void)
{
    cyhal_gpio_write(ARDU_IO4, false);
    cyhal_system_delay_ms(1000); //Cold Start --> wait 1s
    cyhal_gpio_write(ARDU_IO4, true);
}

/*
 * When the demo is measuring light the board LED GREEN
 */
static void update_board_led(bool measuring) {
    if (measuring)
    {
       // Turn LED green ON
    	cyhal_gpio_write(LED1, false);
    }
    else
    {
    	// Turn LED green OFF
    	cyhal_gpio_write(LED1, true);
    }
}

/*
 * Simple debounce routine to check if the user hit the 'ISP' button on the board.
 *
 * @retval 0 No valid 0->1 transition of the switch
 * @retval != A valid transition has been detected
 */
static bool user_pressed_measure_toggle_button(uint32_t *gpio_history) {
    if (!gpio_history) return false;

    uint32_t gpio_hist = *gpio_history;
    gpio_hist = (gpio_hist << 1)| !cyhal_gpio_read(USER_BTN1);
    *gpio_history = gpio_hist;

    return ((gpio_hist & ((1 << SW_DEBOUNCE_CNT) - 1))
            == ((1 << (SW_DEBOUNCE_CNT - 1)) - 1));
}

/*
 * Simple microsecond delay routine
 */
void ams_delay_us(uint32_t usec)
{
	if(usec>65535)
	{
		cyhal_system_delay_ms(usec/1000);
	}
	else
	{
		cyhal_system_delay_us(usec); // param in --> uint16_t
	}
}


/*
 * Demo run routine, never returns
 */
void tmf882x_example_run(void) {
    // Temporary variable for storing version info
    uint8_t ver[16] = { 0 };
    bool is_measuring = false;

    // Seed initial user-button history
    //uint32_t sw_history = -GPIO_PinRead(GPIO, BOARD_SW1_GPIO_PORT,BOARD_SW1_GPIO_PIN);
    uint32_t sw_history = cyhal_gpio_read(USER_BTN1);
    PRINTF("\r\n\r\n\r\n\r\n\r\n");
    PRINTF("\t*************************************************\r\n");
    PRINTF("\t***      Starting TMF882X Example Simple      ***\r\n");
    PRINTF("\t*************************************************\r\n\r\n");

    // Start the UTICK periodic interrupt
   // UTICK_SetTick(UTICK0, kUTICK_Repeat, PERIODIC_UTICK_CNT, periodic_cb);



    PRINTF("TMF882X Example Simple version: %s\r\n", TMF882X_DEMO_VER);
    PRINTF("TMF882X Core Driver version: %s\r\n", TMF882X_MODULE_VER);

    // Assert the CE pin on the TMF882X to turn on the device
    power_on_tmf882x();

    /*************************************************************************
     *
     * Initialize the TMF882X core driver.
     *  - 'struct tmf882x_tof' is the core driver context structure
     *  - performs no I/O, only data initialization
     *
     *************************************************************************/
    tmf882x_init(&tof, &tof_ctx);

    /**************************************************************************
     *
     * DISABLE debug logging in the TMF882X core driver
     *
     *************************************************************************/
    if (g_core_drv_logging > 1)
    	tmf882x_set_debug(&tof, true);

    /**************************************************************************
     *
     * Open the TMF882X core driver
     *  - perform chip initialization
     *  - perform mode-specific initialization
     *
     *************************************************************************/
    while (tmf882x_open(&tof)) {
        PRINTF("Error opening core ToF driver, retrying...\r\n");
        ams_delay_us(3000000);
    }

    /**************************************************************************
     *
     * Switch from the current mode to the Bootloader mode
     *     - Must be in the Bootloader mode to perform Firmware Download (FWDL)
     *     - All modes support switching to the Bootloader mode
     *
     *************************************************************************/
    while (tmf882x_mode_switch(&tof, TMF882X_MODE_BOOTLOADER)) {
        PRINTF("Error switching to Bootloader for FWDL, retrying...\r\n");
        ams_delay_us(3000000);
    }

    /**************************************************************************
     *
     * Perform FWDL
     *     - FWDL supports "bin" download or "intel hex format" download
     *
     *************************************************************************/
    while (tmf882x_fwdl(&tof, FWDL_TYPE_BIN, tof_bin_image,
            tof_bin_image_length)) {
        PRINTF("Error during FWDL, retrying...\r\n");
        ams_delay_us(3000000);
    }

    /**************************************************************************
     *
     * Retrieve the current mode Firmware version information
     *
     *************************************************************************/
    (void) tmf882x_get_firmware_ver(&tof, ver, sizeof(ver));
    PRINTF("TMF882X FW: APP version: %u.%u.%u.%u\r\n", ver[0], ver[1], ver[2], ver[3]);

    /**************************************************************************
     *
     * Disable 8x8 mode (if applicable) for tmf8828 devices
     *  - IOCAPP_SET_8X8MODE is the ioctl command code used to set the 8x8 mode
     *
     *************************************************************************/
#ifdef USE_8X8_MODE
    bool mode_8x8 = true;
#else
    bool mode_8x8 = false;
#endif
    while (tmf882x_ioctl(&tof, IOCAPP_SET_8X8MODE, &mode_8x8, NULL)) {
        PRINTF("Error disabling 8x8 mode, retrying...\r\n");
        ams_delay_us(3000000);
    }

    /**************************************************************************
     *
     * Retrieve the current APP mode configuration data
     *  - 'struct tmf882x_mode_app_config' is the APP mode config structure
     *  - IOCAPP_GET_CFG is the ioctl command code used to get the APP config
     *
     *************************************************************************/
    while (tmf882x_ioctl(&tof, IOCAPP_GET_CFG, NULL, &tofcfg)) {
        PRINTF("Error retrieving APP config, retrying...\r\n");
        ams_delay_us(3000000);
    }

    /**************************************************************************
     *
     * Change the APP configuration
     *  - set the reporting period to 500 milliseconds
     *  - set the spad map configuration to 3x3 (33x32 degree FoV)
     *  OR in case of 8x8 mode set the period to 100ms (update every 400ms since we need 4 frames)
     *
     *************************************************************************/
#ifdef USE_8X8_MODE
    tofcfg.report_period_ms = 100;
#else
    tofcfg.report_period_ms = 500;
    tofcfg.spad_map_id = 1;
#endif

    /**************************************************************************
     *
     * Commit the changed APP mode configuration data
     *  - IOCAPP_SET_CFG is the ioctl command code used to set the APP config
     *
     *************************************************************************/
    while (tmf882x_ioctl(&tof, IOCAPP_SET_CFG, &tofcfg, NULL)) {
        PRINTF("Error setting APP config, retrying...\r\n");
        ams_delay_us(3000000);
    }

    /**************************************************************************
     *
     * Write the APP factory calibration data
     *  - IOCAPP_SET_CALIB is the ioctl command code used to set the APP calibration
     *
     *************************************************************************/
    while (tmf882x_ioctl(&tof, IOCAPP_SET_CALIB, &calibration_data, NULL)) {
        PRINTF("Error setting APP calib data, retrying...\r\n");
        ams_delay_us(3000000);
    }

    PRINTF("\r\n\r\nPress USER_BTN1 to start (GREEN LED ON) or stop "
           "(GREEN LED OFF) ToF measurement.\r\n\r\n");

    /**************************************************************************
     *
     * The demo runs forever polling the TMF882X device for measurement results
     *  - the poll period is POLL_PERIOD_MS
     *  - 'tmf882x_process_irq()' checks and clears the device interrupt flags
     *      and reads out any available data
     *  - check the 'ISP' user button to start/stop measurements
     *  - log measurement results to the debug serial port
     *
     *************************************************************************/
    while (1)
    {

        // delay for polling
        ams_delay_us(POLL_PERIOD_MS);

        // check & clear device IRQ status, read out available data
        tmf882x_process_irq(&tof);

        // start/stop measurements with the 'ISP' board switch
        if (user_pressed_measure_toggle_button(&sw_history))
        {
            if (is_measuring)
            {
                // Stop the measurements of TMF882X
                (void) tmf882x_stop(&tof);
                PRINTF("\r\n\r\n\t<- Stopping Measurement\r\n\r\n");
            }
            else
            {
                // Start the measurements of TMF882X
                (void) tmf882x_start(&tof);
                PRINTF("\r\n\r\n\tStarting Measurement ->\r\n\r\n");
            }

            // Retrieve the current measurement state and update the board LED
            (void) tmf882x_ioctl(&tof, IOCAPP_IS_MEAS, NULL, &is_measuring);
            update_board_led(is_measuring);
        }
    }

    // never return from demo
}
