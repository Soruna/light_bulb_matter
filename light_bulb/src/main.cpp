/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

extern "C" {
	#include "ac_wave_detector.h"
	#include "triac_controller.h"
}

#include "app_task.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, CONFIG_CHIP_APP_LOG_LEVEL);

void new_ac_wave_detected(void)
{
	start_triac_on_timer();
}

int main()
{
	int err_c;
	err_c = init_ac_wave_detector(new_ac_wave_detected);
	if(err_c)
	{
		printk("Failed to init ac wave detector\n");
		return err_c;
	}

	err_c = init_triac_controller();
	if(err_c)
	{
		printk("Failed to init triac controller\n");
		return err_c;
	}

	enable_ac_wave_detector();

	change_triac_enabled_state(true);
	change_triac_brightness_level(10);

 	k_msleep(1000);

	CHIP_ERROR err = AppTask::Instance().StartApp();

	LOG_ERR("Exited with code %" CHIP_ERROR_FORMAT, err.Format());
	return err == CHIP_NO_ERROR ? EXIT_SUCCESS : EXIT_FAILURE;
}
