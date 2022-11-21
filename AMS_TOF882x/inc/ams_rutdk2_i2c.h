/*
 * ams_rutdk2_i2c.h
 *
 *  Created on: April 2022
 *      Author: A.Heder
 */

#ifndef AMS_RUTDK2_I2C_H_
#define AMS_RUTDK2_I2C_H_

void ams_i2c_reinit(void);
void rutdk2_i2c_init(void);
int32_t ams_i2c_write_block(uint8_t slave_addr, uint8_t reg, const uint8_t * buf, uint32_t len);
int32_t ams_i2c_read_block(uint8_t slave_addr, uint8_t reg, uint8_t * buf, uint32_t len);



#endif /* AMS_RUTDK2_I2C_H_ */
