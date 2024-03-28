#ifndef DISPLAY_CONTROLLER_H_
#define DISPLAY_CONTROLLER_H_

#include <zephyr/kernel.h>

enum pages {
        MAIN_PAGE         = 0x00,
        LOADING_PAGE      = 0x01,
        DEVICE_PAGE       = 0x02
};

//-----------------------------------------------------------------------------------------------------------------------
/*! displayContreller
* @brief displayContreller 
*/
void displayController();

//-----------------------------------------------------------------------------------------------------------------------
/*! updateMainPage
* @brief updateMainPage function update the main page component with the monkeylist values
*/
void updateMainPage();

//-----------------------------------------------------------------------------------------------------------------------
/*! connectDevice
* @brief connectDevice is called to connect to a device
*/
void connectDevice();

//-----------------------------------------------------------------------------------------------------------------------
/*! deviceConnected
* @brief deviceConnected is called by the BLE controller when device is connected
*/
void deviceConnected(struct Monkey monkey);

//-----------------------------------------------------------------------------------------------------------------------
/*! downPressed
* @brief downPressed is called by the button mangager
*/
void downPressed();

//-----------------------------------------------------------------------------------------------------------------------
/*! upPressed
* @brief upPressed is called by the button mangager
*/
void upPressed();

//-----------------------------------------------------------------------------------------------------------------------
/*! selectPressed
* @brief selectPressed is called by the button mangager
*/
void selectPressed();

//-----------------------------------------------------------------------------------------------------------------------
/*! triggerPressed
* @brief triggerPressed is called by the button mangager
*/
void triggerPressed();

//-----------------------------------------------------------------------------------------------------------------------
/*! Task_UDP_Client_Init initializes the task UDP Client
*
* @brief 
*/
void Task_Display_Controller_Init(void);



#endif /* DISPLAY_CONTROLLER_H_ */
