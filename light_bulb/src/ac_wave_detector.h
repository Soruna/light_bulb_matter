/******************************************************************************************
 *
 * @file        ac_wave_detector.h
 *
 * @brief       Detects new AC wave signal
 *
 * @details     Calls the callback provided when a new AC wave is detected
 *
 * @author(s)   Joe Rippee
 *
 * @created     2/09/2024   (MM/DD/YYYY)
 * 
 * @copyright       © 2024 SWYM, LLC. All Rights Reserved
 *
 *****************************************************************************************/

#ifndef AC_WAVE_DETECTOR_H__
#define AC_WAVE_DETECTOR_H__

/*
  Parameter: void function pointer for when new AC wave is detected
  Returns: status variable for ac wave detection initialization. 0 = success

  Initializes the ac wave detector with edge detect interrupts (rising edge only)
  Provided callback is stored in a unique private function pointer
*/
int init_ac_wave_detector(void (*ac_wave_detected_callback)(void));

void enable_ac_wave_detector(void);

#endif



