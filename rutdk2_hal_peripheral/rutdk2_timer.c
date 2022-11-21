/*
 * rutdk2_timer.c
 *
 *  Created on: 06.05.2022
 *      Author: EDE
 */

#ifndef RUTDK2_HAL_PERIPHERAL_RUTDK2_TIMER_C_
#define RUTDK2_HAL_PERIPHERAL_RUTDK2_TIMER_C_

#include "rutdk2_timer.h"

cyhal_timer_t Systick_obj;
const cyhal_timer_cfg_t Systick_cfg =
   {
       .compare_value = 0,                  // Timer compare value, not used
       .period        = 0,              	// Timer period set to a large enough value
					    //   compared to event being measured
       .direction     = CYHAL_TIMER_DIR_UP, // Timer counts up
       .is_compare    = false,              // Don't use compare mode
       .is_continuous = false,              // Do not run timer indefinitely
       .value         = 0                   // Initial value of counter
   };
    // Initialize the timer object. Does not use pin output ('pin' is NC) and does not use a
     // pre-configured clock source ('clk' is NULL).

void rutdk2_hal_timer_init(void)
    {
    cyhal_timer_init(&Systick_obj, NC, NULL);
    cyhal_timer_configure(&Systick_obj, &Systick_cfg);
    cyhal_timer_set_frequency(&Systick_obj, 1000000);
    cyhal_timer_start(&Systick_obj);

    }


#endif /* RUTDK2_HAL_PERIPHERAL_RUTDK2_TIMER_C_ */
