/******************************************************************************************
 *
 * @file        triac_controller.h
 *
 * @brief       Controls Triac On/Off states
 *
 * @details     Accepts inputs for if Triac is enabled/disabled, and for how long
 *
 * @author(s)   Joe Rippee
 *
 * @created     2/12/2024   (MM/DD/YYYY)
 * 
 * @copyright       © 2024 SWYM, LLC. All Rights Reserved
 *
 *****************************************************************************************/

#ifndef TRIAC_CONTROLLER_H__
#define TRIAC_CONTROLLER_H__

#include <stdint.h>

/*
  Parameter: void function pointer for when new AC wave is detected
  Returns: status variable for ac wave detection initialization. 0 = success

  Initializes the ac wave detector with edge detect interrupts (rising edge only)
  Provided callback is stored in a unique private function pointer
*/
int init_triac_controller(void);
void start_triac_on_timer(void);
void change_triac_enabled_state(bool enabled_state);
void change_triac_brightness_level(uint8_t brightness_level);

#endif
