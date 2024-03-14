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
        nextion_command("page 1");

        display_device_1(100,-4,0,ST_LOW_BATT,false);
        display_device_2(56,-23,6,ST_ERROR,true);    
        display_device_3(123,-54,3,ST_DISK_FULL,false);
        
        return 0;
}
