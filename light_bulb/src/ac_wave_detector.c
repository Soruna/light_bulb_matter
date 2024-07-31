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

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "ac_wave_detector.h"

#define AC_WAVE_STACK_SIZE  1024
#define AC_WAVE_THD_PRIO    10

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE        DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

K_THREAD_STACK_DEFINE(ac_wave_stack, AC_WAVE_STACK_SIZE);
struct k_work_q ac_wave_q;

static struct k_work ac_wave_handler;

static const struct gpio_dt_spec ac_wave_detector_gpio = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
                                                              {0});
static struct gpio_callback ac_wave_cb_data;

void (*main_ac_wave_detected_callback)(void) = NULL;

/*
  Parameter: k_work pointer
  Returns: N/A

  gets called by a work queue if new AC wave is detected
  simply alerts the main program that a new AC wave was detected
*/
void ac_wave_work_handler(struct k_work *work) {
  if(NULL != main_ac_wave_detected_callback) {
    main_ac_wave_detected_callback();
  }
}

/*
  Parameter: 
    which button port was pressed/depressed
    a struct holding gpio callback info
    which button pin was pressed/depressed
  Returns: N/A

  Triggers on button rising and falling edge
  The onboard press detect chip seems to handle de-bounce quite well

  If button is pressed, start a timer
  If button is de-pressed, end timer
*/
void ac_wave_detected(const struct device *dev, struct gpio_callback *cb,
                    uint32_t pins)
{
  int val = gpio_pin_get_dt(&ac_wave_detector_gpio);
  if (val >= 0) {
    int k_work_status = k_work_submit_to_queue(&ac_wave_q, &ac_wave_handler);
    if(0 > k_work_status) {
      printk("[ac_wave_detector]: Could not submit ac wave detection to queue due to %i!\n", k_work_status);
    }
  }
}

/*
  Parameter: N/A
  Returns: status variable for capacative button enable GPIO initialization

  this GPIO must be high for button detection to work due to the PCB layout
  returns 0 if initialization was successful
*/
/*
  Parameter: N/A
  Returns: status variable for capacative button GPIO initialization

  enables the capacative button on the wristband
  provides callback for when button is pressed/depressed
  returns 0 if initialization was successful
*/
int initialize_ac_detect_gpio(void)
{
  int err = 0;

  if (!gpio_is_ready_dt(&ac_wave_detector_gpio)) {
    printk("Error: ac_wave_detector_gpio device %s is not ready\n",
           ac_wave_detector_gpio.port->name);
    err += -ENODEV;
  }
  
  err += gpio_pin_configure_dt(&ac_wave_detector_gpio, GPIO_INPUT);
  if (err != 0) {
    printk("Error %d: failed to configure %s pin %d\n",
           err, ac_wave_detector_gpio.port->name, ac_wave_detector_gpio.pin);
  }
  
  err += gpio_pin_interrupt_configure_dt(&ac_wave_detector_gpio,
                                      GPIO_INT_EDGE_FALLING);
  if (err != 0) {
    printk("Error %d: failed to configure interrupt on %s pin %d\n",
        err, ac_wave_detector_gpio.port->name, ac_wave_detector_gpio.pin);
  }
  
  gpio_init_callback(&ac_wave_cb_data, ac_wave_detected, BIT(ac_wave_detector_gpio.pin));

  return err;
}

/*
  Parameter: void function pointer for when new AC wave is detected
  Returns: status variable for ac wave detection initialization. 0 = success

  Initializes the ac wave detector with edge detect interrupts (rising edge only)
  Provided callback is stored in a unique private function pointer
*/
int init_ac_wave_detector(void (*ac_wave_detected_callback)(void))
{
  int err = 0;

  main_ac_wave_detected_callback = ac_wave_detected_callback;
  err += initialize_ac_detect_gpio();

  if(0 == err) {
    k_work_init(&ac_wave_handler, ac_wave_work_handler);

    k_work_queue_start(
      &ac_wave_q,
      ac_wave_stack,
      K_THREAD_STACK_SIZEOF(ac_wave_stack),
      AC_WAVE_THD_PRIO,
      NULL
    );
  }

  return err;
}

void enable_ac_wave_detector(void)
{
  gpio_add_callback(ac_wave_detector_gpio.port, &ac_wave_cb_data);
}
