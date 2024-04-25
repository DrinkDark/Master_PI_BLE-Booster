/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include "define.h"
#include "task/led.h"
#include "task/button_manager.h"
#include "task/ble.h"
#include "task/monkeylist.h"

K_HEAP_DEFINE(monkeyListHeap,32768);

int main(void)
{
	#ifdef DEBUG_MODE
	 	printk("Programm started!\n");
	#endif

	led_thread_init();
	button_manager_init();
	ble_thread_init();
	initMonkeyList();
	Task_Display_Controller_Init();

	k_sleep( K_FOREVER );
	return 0;
}

