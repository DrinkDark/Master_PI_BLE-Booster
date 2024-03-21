#ifndef DISPLAY_H
#define DISPLAY_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "global.h"


void nextion_command(char *buf);

// main page function
void display_main_page();
void display_device_1(int deviceNbr, int rssi, int nbrDays, enum main_state state, bool selected);
void display_device_2(int deviceNbr, int rssi, int nbrDays, enum main_state state, bool selected);
void display_device_3(int deviceNbr, int rssi, int nbrDays, enum main_state state, bool selected);
void display_more_devices();
void hide_device_1();
void hide_device_2();
void hide_device_3();
void hide_more_devices();

// loading page 
void display_loading_page(int deviceNbr);

// device page
void display_device_page(int deviceNbr, int rssi, int nbrDays, enum main_state state);
void display_select_open();
void display_select_reset();
void display_select_toggle();
void display_select_exit();
void display_show_popup();
void display_hide_popup();


#endif /* DISPLAY_H */
