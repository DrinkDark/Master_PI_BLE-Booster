

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


//selection variable
int select = 0;

//number of monkeys
int monkeyNbr;

enum pages current_page = MAIN_PAGE;

//-----------------------------------------------------------------------------------------------------------------------
/*! displayContreller
* @brief displayContreller 
*/
void displayController() 
{
    

	while(true)	// --------------------------------------------------------------------Thread infinite loop
	{
        if(current_page == MAIN_PAGE)
            updateMainPage();
       
        k_msleep(500);
	}	// ------------------------------------------------------------------------------  end of thread infinite loop
}


//-----------------------------------------------------------------------------------------------------------------------
/*! updateMainPage
* @brief updateMainPage 
*/
void updateMainPage()
{
    static struct Monkey monkeyArray[3];

    monkeyNbr = getNumMonkeys();

    if(monkeyNbr == 1)
    {
        if(select >= 1)
            select = 0;
            
        getMonkeyAtIndex(monkeyArray, 0);
        display_device_1(monkeyArray[0].num, monkeyArray[0].rssi, monkeyArray[0].record_time, monkeyArray[0].state, (select == 0));
        hide_device_2();
        hide_device_3();
        hide_more_devices();
    }
    else if(monkeyNbr == 2)
    {
        if(select == 2)
            select = 1;
        getMonkeyAtIndex(&(monkeyArray[0]), 0);
        display_device_1(monkeyArray[0].num, monkeyArray[0].rssi, monkeyArray[0].record_time, monkeyArray[0].state, (select == 0));
        getMonkeyAtIndex(&(monkeyArray[1]), 1);
        display_device_2(monkeyArray[1].num, monkeyArray[1].rssi, monkeyArray[1].record_time, monkeyArray[1].state, (select == 1));
        hide_device_3();
        hide_more_devices();
    }
    else if(monkeyNbr >=3)
    {
        getThreeMonkeys(monkeyArray, 0);
        display_device_1(monkeyArray[0].num, monkeyArray[0].rssi, monkeyArray[0].record_time, monkeyArray[0].state, (select == 0));
        display_device_2(monkeyArray[1].num, monkeyArray[1].rssi, monkeyArray[1].record_time, monkeyArray[1].state, (select == 1));
        display_device_3(monkeyArray[2].num, monkeyArray[2].rssi, monkeyArray[2].record_time, monkeyArray[2].state, (select == 2));

        if(monkeyNbr == 3)
            hide_more_devices();
        else
            display_more_devices();
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! downPressed
* @brief downPressed is called by the button mangager
*/
void downPressed()
{
    if(current_page == MAIN_PAGE)
    {
        if(select<monkeyNbr-1 && select < 2)
            select++;
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! upPressed
* @brief upPressed is called by the button mangager
*/
void upPressed()
{
    if(current_page == MAIN_PAGE)
    {
        if(select>0)
            select--;
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! selectPressed
* @brief selectPressed is called by the button mangager
*/
void selectPressed()
{

}

//-----------------------------------------------------------------------------------------------------------------------
/*! triggerPressed
* @brief triggerPressed is called by the button mangager
*/
void triggerPressed()
{

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