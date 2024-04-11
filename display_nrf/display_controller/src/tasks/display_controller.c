

//includes
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(display_controller);
#include <zephyr/kernel.h>

#include "display_controller.h"


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
int selectOffset = 0;
bool popup = false;


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

        if(current_page == DEVICE_PAGE)
            updateDevicePage();

        k_msleep(200);
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
        getThreeMonkeys(monkeyArray, 0+selectOffset);
        display_device_1(monkeyArray[0].num, monkeyArray[0].rssi, monkeyArray[0].record_time, monkeyArray[0].state, (select == 0));
        display_device_2(monkeyArray[1].num, monkeyArray[1].rssi, monkeyArray[1].record_time, monkeyArray[1].state, (select == 1));
        display_device_3(monkeyArray[2].num, monkeyArray[2].rssi, monkeyArray[2].record_time, monkeyArray[2].state, (select == 2));

        if(monkeyNbr-selectOffset == 3)
            hide_more_devices();
        else
            display_more_devices();
    }

}

//-----------------------------------------------------------------------------------------------------------------------
/*! updateDevicePage
* @brief updateDevicePage 
*/
void updateDevicePage()
{
    static int prevSelect = -1;

    if(prevSelect != select)
    {
        prevSelect = select;

        if(select == 0)
            display_select_open();
        else if(select == 1)
            display_select_reset();
        else if(select == 2)
            display_select_toggle();
        else if(select == 3)
            display_select_exit();
        
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! connectDevice
* @brief connectDevice is called to connect to a device
*/
void connectDevice()
{
    struct Monkey monkey;
    getMonkeyAtIndex(&monkey,selectOffset+select);
    display_loading_page(monkey.num);
    //call bluetooth callback function
    current_page = LOADING_PAGE;

    //TEST CODE
    k_msleep(1000);
    deviceConnected(monkey);
}

//-----------------------------------------------------------------------------------------------------------------------
/*! deviceConnected
* @brief deviceConnected is called by the BLE controller when device is connected
*/
void deviceConnected(struct Monkey monkey)
{
    display_device_page(monkey.num,monkey.rssi,monkey.record_time,monkey.state);
    select = 0;
    current_page = DEVICE_PAGE;
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
        else if(select == 2 && monkeyNbr-3-selectOffset>0)
            selectOffset++;
    }

    if(current_page == DEVICE_PAGE)
    {
        if(select<3)
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
        else if(selectOffset>0)
            selectOffset--;
    }

    if(current_page == DEVICE_PAGE)
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
    if(current_page == MAIN_PAGE)
    {
        connectDevice();
    }

    if(current_page == DEVICE_PAGE)
    {
        display_show_popup();
        popup=true;
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! triggerPressed
* @brief triggerPressed is called by the button mangager
*/
void triggerPressed()
{
    if(current_page == DEVICE_PAGE)
    {
        display_hide_popup();
        popup=false;
        if(select==0)
        {
            //open collar
        }
        if(select==1)
        {
            //reset collar

        }
        if(select==2)
        {
            //call function to toggle recording

        }
        if(select==3)
        {
            //call function to disconnect

            select = 0;
            display_main_page();
        }
    }
}



//-----------------------------------------------------------------------------------------------------------------------
/*! Task_UDP_Client_Init initializes the task UDP Client
*
* @brief 
*/
void Task_Display_Controller_Init( void ){
    display_init();
    k_msleep(1000);
    display_main_page();
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