/******************************************************************************************
 *
 * @file        triac_controller.c
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

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "triac_controller.h"

/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
struct gpio_dt_spec triac = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});

//4.2
const uint8_t LOWEST_BRIGHTNESS_LEVEL = 1;
const uint8_t HIGHEST_BRIGHTNESS_LEVEL = 10;

const uint32_t FREQUENCY_OF_AC_SIGNAL = 120;
const uint32_t NUMBER_OF_BRIGHTNESS_DIVISIONS = 10; 
const uint32_t NUMBER_OF_MS_IN_A_SEC = 1000;
const uint32_t NUMBER_OF_US_IN_A_MSEC = 1000;
const double DUTY_CYCLE = 1.0;
//We are working off Sine wave, and need to sync off zero
const double msec_offset_needed = 6.22135; 
uint32_t number_of_ticks_to_delay = 0;
uint32_t number_of_ticks_per_brightness_division = 0;
uint32_t triac_brightness_level = 10; //1 is the lowest, 10 is the highest
//Default to triac disabled for safety reasons
bool triac_enabled = false;

const int TRIAC_ON_STATE  = 1;
const int TRIAC_OFF_STATE = 0;

struct k_sem triac_enabled_sem;
struct k_sem brightness_level_sem;

void timer_expired_handler(struct k_timer *timer);
void delay_timer_expired_handler(struct k_timer *timer);

K_TIMER_DEFINE(brightness_timer, timer_expired_handler, NULL);
K_TIMER_DEFINE(delay_timer, delay_timer_expired_handler, NULL);


void timer_expired_handler(struct k_timer *timer)
{
  gpio_pin_set_dt(&triac, TRIAC_OFF_STATE);
}

void delay_timer_expired_handler(struct k_timer *timer)
{
  k_timer_start(
    &brightness_timer, 
    K_CYC(
      number_of_ticks_per_brightness_division*triac_brightness_level
    ), 
    K_NO_WAIT
  );  
  gpio_pin_set_dt(&triac, TRIAC_ON_STATE);
}

void start_triac_on_timer(void)
{
  if(true == triac_enabled) {
    k_timer_start(&delay_timer, K_CYC(number_of_ticks_to_delay), K_NO_WAIT);
  }
}

void calculate_time_frame(void)
{
  double ms_per_brightness_division = NUMBER_OF_MS_IN_A_SEC * 1.0;
  ms_per_brightness_division /= (
    FREQUENCY_OF_AC_SIGNAL * NUMBER_OF_BRIGHTNESS_DIVISIONS
  );
  uint32_t number_of_ticks_per_sec = sys_clock_hw_cycles_per_sec();
  printk("Number of hw cycles per second = %i\n", number_of_ticks_per_sec);
  double number_of_ticks_per_msec = 1.0 * number_of_ticks_per_sec;
  number_of_ticks_per_msec /= NUMBER_OF_MS_IN_A_SEC;
  number_of_ticks_to_delay = (uint32_t)(
    number_of_ticks_per_msec * msec_offset_needed
  );
  number_of_ticks_per_brightness_division = (uint32_t)(
    number_of_ticks_per_msec * ms_per_brightness_division * DUTY_CYCLE
  );
}

int initialize_triac_gpio(void)
{
  int err = 0;

  if (triac.port && !device_is_ready(triac.port)) {
  printk("Error %d: LED device %s is not ready; ignoring it\n",
         err, triac.port->name);
  triac.port = NULL;
  err += -ENODEV;
  }
  if (triac.port) {
    err = gpio_pin_configure_dt(&triac, GPIO_OUTPUT);
    if (err != 0) {
      printk("Error %d: failed to configure LED device %s pin %d\n",
             err, triac.port->name, triac.pin);
      triac.port = NULL;
    } else {
      printk("Set up LED at %s pin %d\n", triac.port->name, triac.pin);
    }
  } else {
    printk("There is no triac port\n");
  }

  return err;
}

int init_triac_controller(void)
{
  int err = 0;

  err += k_sem_init(&triac_enabled_sem, 1, 1);
  err += k_sem_init(&brightness_level_sem, 1, 1);

  calculate_time_frame();
  err += initialize_triac_gpio();

  return err;
}

void change_triac_enabled_state(bool enabled_state)
{
  if(0 == k_sem_take(&triac_enabled_sem, K_MSEC(100))) {
    triac_enabled = enabled_state;
    k_sem_give(&triac_enabled_sem);
  }
}

void change_triac_brightness_level(uint8_t brightness_level)
{
  if(
    LOWEST_BRIGHTNESS_LEVEL <= brightness_level 
      && 
    HIGHEST_BRIGHTNESS_LEVEL >= brightness_level
  ) {
    if(0 == k_sem_take(&brightness_level_sem, K_MSEC(100))) {
      triac_brightness_level = brightness_level;
      k_sem_give(&brightness_level_sem);
    }
  }
}
