

//includes
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(display_controller);
#include <zephyr/kernel.h>

#include "display_controller.h"
#include "connection.h"


//! Stack size for the DISPLAY_CONTROLLER thread
#define DISPLAY_CONTROLLER_STACK_SIZE 2048
//! UDP_SERVER thread priority level
#define DISPLAY_CONTROLLER_PRIORITY 3


//! display controller stack definition
K_THREAD_STACK_DEFINE(DISPLAY_CONTROLLER_STACK, DISPLAY_CONTROLLER_STACK_SIZE);
//! Variable to identify the display controller thread
static struct k_thread displayControllerThread;

static K_WORK_DELAYABLE_DEFINE(timeout_work, disablePopup);
static K_WORK_DELAYABLE_DEFINE(popup_confirmation_work, display_hide_popup);


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

    current_page = LOADING_PAGE;

    //call bluetooth function
    connect(monkey);
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
/*! disablePopup
* @brief disablePopup after timeout
*/
void disablePopup()
{
    if(popup)
    {
        display_hide_popup();
        popup=false;
    }
    
}

//-----------------------------------------------------------------------------------------------------------------------
/*! deviceDisconnected
* @brief deviceDisconnected is called by the BLE controller when device is connected
*/
void deviceDisconnected()
{
    display_main_page();
    select = 0;
    current_page = MAIN_PAGE;
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
        
        return;
    }

    if(current_page == DEVICE_PAGE && popup == false)
    {
        if(select<3)
            select++;
        return;
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
        return;
    }

    if(current_page == DEVICE_PAGE && popup == false)
    {
        if(select>0)
            select--;
        return;
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
        return;
    }

    if(current_page == DEVICE_PAGE)
    {
        if(select==3)
        {
            //call function to disconnect
            disconnect();
            return;
        }
        else
        {
            display_show_popup();
            popup=true;
            k_work_reschedule(&timeout_work, K_SECONDS(3)); //timeout popup
        }
        return;
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! triggerPressed
* @brief triggerPressed is called by the button mangager
*/
void triggerPressed()
{
    if(current_page == DEVICE_PAGE && popup == true)
    {
        popup=false;
        display_show_popup_highlighted();
        k_work_reschedule(&popup_confirmation_work,K_MSEC(1000));
        if(select==0)
        {
            //open collar
            openCollar();
            return;
        }
        if(select==1)
        {
            //reset collar
            resetCollar();
            return;
        }
        if(select==2)
        {
            //call function to toggle recording
            toggleRecording();
            return;
        }
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! onConnected
* @brief onConnected callback called by the bluetooth connection
*/
void onConnected(struct Monkey monkey)
{
    deviceConnected(monkey);
}

//-----------------------------------------------------------------------------------------------------------------------
/*! onDisconnected
* @brief onDisconnected callback called by the bluetooth connection
*/
void onDisconnected()
{
    deviceDisconnected();
}

//-----------------------------------------------------------------------------------------------------------------------
/*! onConnectionFailed
* @brief onConnectionFailed callback called by the bluetooth connection
*/
void onConnectionFailed()
{
    deviceDisconnected();
}

//-----------------------------------------------------------------------------------------------------------------------
/*! onUpdateInfos
* @brief onUpdateInfos callback called by the bluetooth connection
*/
void onUpdateInfos(struct Monkey monkey)
{
    display_device_page(monkey.num,monkey.rssi,monkey.record_time,monkey.state);
}


//-----------------------------------------------------------------------------------------------------------------------
/*! init Display controller
*
* @brief 
*/
void Task_Display_Controller_Init( void ){
    display_init();
    k_msleep(1000);
    display_main_page();

    setConnectedCallback(onConnected);
    setDisconnectedCallback(onDisconnected);
    setConnectionFailedCallback(onConnectionFailed);
    setUpdateInfosCallback(onUpdateInfos);

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