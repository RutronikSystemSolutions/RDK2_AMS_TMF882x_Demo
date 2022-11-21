/*
 * ams_rutdk2_i2c.h
 *
 *  Created on: April 2022
 *      Author: A.Heder
 */

#include <stdio.h>
#include <string.h>
#include "cyhal.h"
#include "cybsp.h"
#include "ams_rutdk2_i2c.h"
#include <stdio.h>
#include <stdlib.h>
//#include "board.h"
//#include "fsl_i2c.h"
//#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
//#define EXAMPLE_I2C_MASTER_BASE    (I2C4_BASE)
//#define EXAMPLE_I2C_MASTER ((I2C_Type *)EXAMPLE_I2C_MASTER_BASE)

/*******************************************************************************
 * Variables
 ******************************************************************************/
cyhal_i2c_t I2C_scb3;
cyhal_i2c_cfg_t I2C_cfg;
bool i2c_busy_flag = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

void master_event_handler(void* callback_arg, cyhal_i2c_event_t event)
    {
    // To remove unused variable warning
    (void)callback_arg;

    // Check write complete event
    if (0UL != (CYHAL_I2C_MASTER_WR_CMPLT_EVENT & event))
    {
	i2c_busy_flag = 0;
    }
    // Check read complete event
    if (0UL != (CYHAL_I2C_MASTER_RD_CMPLT_EVENT & event))
    {
	i2c_busy_flag = 0;
    }
    if (0UL != (CYHAL_I2C_MASTER_ERR_EVENT & event))
    {
        // In case of error abort transfer
        cyhal_i2c_abort_async(&I2C_scb3);
    }
    }

void rutdk2_i2c_init(void)
    {
    cy_rslt_t result;
    I2C_cfg.is_slave = false;
    I2C_cfg.address = 0;
    I2C_cfg.frequencyhal_hz = 100000UL;
    result = cyhal_i2c_init(&I2C_scb3, ARDU_SDA, ARDU_SCL, NULL);
    if (result != CY_RSLT_SUCCESS)
	{
	CY_ASSERT(0);
	}
    result = cyhal_i2c_configure(&I2C_scb3, &I2C_cfg);
    if (result != CY_RSLT_SUCCESS)
	{
	CY_ASSERT(0);
	}
    cyhal_i2c_register_callback(&I2C_scb3, (cyhal_i2c_event_callback_t) master_event_handler,NULL);

    cyhal_i2c_enable_event(&I2C_scb3, (cyhal_i2c_event_t)   \
                              (  CYHAL_I2C_MASTER_WR_CMPLT_EVENT \
                        	|CYHAL_I2C_MASTER_RD_CMPLT_EVENT \
                               | CYHAL_I2C_MASTER_ERR_EVENT),    \
                              3u, true);
    }

int32_t ams_i2c_write_block(uint8_t slave_addr, uint8_t reg, const uint8_t * buf, uint32_t len)
{
    cy_rslt_t  cy_rslt = 0;

    uint8_t *data;
    data = (uint8_t*)calloc(len+1, sizeof(uint8_t));
    data[0]=reg;
    for (int n = 0; n < len; ++n)
	{
	data[n+1]=buf[n];
	}
    cyhal_i2c_master_write(&I2C_scb3, slave_addr, data, len+1, 100, true);
    if (cy_rslt != CY_RSLT_SUCCESS)
    	{
    	CY_ASSERT(0); // timeout?
    	}
    free( data );
    return cy_rslt;
}
int32_t ams_i2c_read_block(uint8_t slave_addr, uint8_t reg, uint8_t * buf, uint32_t len)
{
    cy_rslt_t  cy_rslt = 0;
    uint8_t data[1];
    data[0] = reg;
    i2c_busy_flag = 1;
    cy_rslt =cyhal_i2c_master_write(&I2C_scb3, slave_addr, data, 1, 100, false);
    if (cy_rslt != CY_RSLT_SUCCESS)
    	{
    	CY_ASSERT(0); // timeout?
    	}
    i2c_busy_flag = 1;
    cy_rslt = cyhal_i2c_master_read(&I2C_scb3, slave_addr, buf, len, 100, true);
    if (cy_rslt != CY_RSLT_SUCCESS)
    	{
	CY_ASSERT(0); // timeout?
    	}
    return cy_rslt;
}

//void ams_i2c_reinit(void)
//{
//    I2C_MasterEnable(EXAMPLE_I2C_MASTER, false);
//    I2C_MasterEnable(EXAMPLE_I2C_MASTER, true);
//}
//int32_t ams_i2c_write_block(uint8_t slave_addr, uint8_t reg, const uint8_t * buf, uint32_t len)
//{
//    i2c_master_transfer_t masterXfer = {0};
//    status_t reVal                   = kStatus_Fail;
//	ams_i2c_reinit();
//
//    /* subAddress = 0x01, data = g_master_txBuff - write to slave.
//      start + slaveaddress(w) + subAddress + length of data buffer + data buffer + stop*/
//    masterXfer.slaveAddress   = slave_addr;
//    masterXfer.direction      = kI2C_Write;
//    masterXfer.subaddress     = (uint32_t)reg;
//    masterXfer.subaddressSize = 1;
//    masterXfer.data           = (uint8_t *)buf;
//    masterXfer.dataSize       = len;
//    masterXfer.flags          = kI2C_TransferDefaultFlag;
//
//    /* Send master non-blocking data to slave */
//    reVal = I2C_MasterTransferBlocking(EXAMPLE_I2C_MASTER, &masterXfer);
//
//    return reVal != kStatus_Success;
//}
//
//int32_t ams_i2c_read_block(uint8_t slave_addr, uint8_t reg, uint8_t * buf, uint32_t len)
//{
//    i2c_master_transfer_t masterXfer = {0};
//    status_t reVal                   = kStatus_Fail;
//	ams_i2c_reinit();
//
//    /* subAddress = 0x01, data = g_master_txBuff - write to slave.
//      start + slaveaddress(w) + subAddress + length of data buffer + data buffer + stop*/
//    masterXfer.slaveAddress   = slave_addr;
//    masterXfer.direction      = kI2C_Read;
//    masterXfer.subaddress     = (uint32_t)reg;
//    masterXfer.subaddressSize = 1;
//    masterXfer.data           = buf;
//    masterXfer.dataSize       = len;
//    masterXfer.flags          = kI2C_TransferDefaultFlag;
//
//    /* Send master non-blocking data to slave */
//    reVal = I2C_MasterTransferBlocking(EXAMPLE_I2C_MASTER, &masterXfer);
//
//    return reVal != kStatus_Success;
//}
//
