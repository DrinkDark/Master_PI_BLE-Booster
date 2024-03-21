#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include <stdio.h>

#include "tasks/global.h"
#include "tasks/display.h"


int main(void)
{
        display_main_page();

        display_device_1(100,-4,0,ST_LOW_BATT,false);
        display_device_2(56,-23,6,ST_ERROR,false);    
        display_device_3(123,-54,3,ST_DISK_FULL,true);
        display_more_devices();

        k_msleep(2000);
        
        display_loading_page(123);

        k_msleep(2000);

        display_device_page(123,-54,3,ST_DISK_FULL);

        k_msleep(2000);

        display_select_reset();

        k_msleep(2000);

        display_show_popup();


        return 0;
}
