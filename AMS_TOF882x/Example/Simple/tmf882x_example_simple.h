/*
 * tmf882x_example.h
 *
 *  Created on: Aug 18, 2020
 *      Author: orya
 */
#include "tmf882x.h"

#ifndef TMF882X_EXAMPLE_SIMPLE_H_
#define TMF882X_EXAMPLE_SIMPLE_H_

/* Interface to get 1MHz ticks */
uint32_t get_uticks(void);

/* Simple delay function */
void ams_delay_us(uint32_t usec);

/* Demo run function, never returns */
void tmf882x_example_run(void);

#endif /* TMF882X_EXAMPLE_SIMPLE_H_ */
