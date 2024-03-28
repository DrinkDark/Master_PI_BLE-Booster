

//includes
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(display_controller);
#include <zephyr/kernel.h>

#include "display_controller.h"
#include "monkeylist.h"

//! Stack size for the DISPLAY_CONTROLLER thread
#define DISPLAY_CONTROLLER_STACK_SIZE 2048
//! UDP_SERVER thread priority level
#define DISPLAY_CONTROLLER_PRIORITY 3


//! display controller stack definition
K_THREAD_STACK_DEFINE(DISPLAY_CONTROLLER_STACK, DISPLAY_CONTROLLER_STACK_SIZE);
//! Variable to identify the display controller thread
static struct k_thread displayControllerThread;


//-----------------------------------------------------------------------------------------------------------------------
/*! displayContreller
* @brief displayContreller 
*/
void displayController() 
{
    struct Monkey monkeyArray[3];

	while(true)	// --------------------------------------------------------------------Thread infinite loop
	{
        int num = getNumMonkeys();
        if(num == 1)
        {
            getMonkeyAtIndex(monkeyArray, 0);
            display_device_1(monkeyArray[0].num, monkeyArray[0].rssi, monkeyArray[0].record_time, monkeyArray[0].state, true);
            hide_device_2();
            hide_device_3();
        }
        else if(num == 2)
        {
            getMonkeyAtIndex(&(monkeyArray[0]), 0);
            display_device_1(monkeyArray[0].num, monkeyArray[0].rssi, monkeyArray[0].record_time, monkeyArray[0].state, true);
            getMonkeyAtIndex(&(monkeyArray[1]), 1);
            display_device_2(monkeyArray[1].num, monkeyArray[1].rssi, monkeyArray[1].record_time, monkeyArray[1].state, false);
            hide_device_3();
        }
        else if(num >=3)
        {
            getThreeMonkeys(monkeyArray, 0);
            display_device_1(monkeyArray[0].num, monkeyArray[0].rssi, monkeyArray[0].record_time, monkeyArray[0].state, true);
            display_device_2(monkeyArray[1].num, monkeyArray[1].rssi, monkeyArray[1].record_time, monkeyArray[1].state, false);
            display_device_3(monkeyArray[2].num, monkeyArray[2].rssi, monkeyArray[2].record_time, monkeyArray[2].state, false);
        }

        k_msleep(500);
	}	// ------------------------------------------------------------------------------  end of thread infinite loop
}




//-----------------------------------------------------------------------------------------------------------------------
/*! Task_UDP_Client_Init initializes the task UDP Client
*
* @brief 
*/
void Task_Display_Controller_Init( void ){
	k_thread_create	(														\
					&displayControllerThread,								\
					DISPLAY_CONTROLLER_STACK,								\
					DISPLAY_CONTROLLER_STACK_SIZE,							\
					(k_thread_entry_t)displayController,					\
					NULL,													\
					NULL,													\
					NULL,													\
					DISPLAY_CONTROLLER_PRIORITY,							\
					0,														\
					K_NO_WAIT);	

	 k_thread_name_set(&displayControllerThread, "displayControllerThread");
	 k_thread_start(&displayControllerThread);
}