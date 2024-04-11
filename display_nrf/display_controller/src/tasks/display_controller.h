#ifndef DISPLAY_CONTROLLER_H_
#define DISPLAY_CONTROLLER_H_

#include <zephyr/kernel.h>

#include "display.h"
#include "monkeylist.h"

enum pages {
        MAIN_PAGE         = 0x00,
        LOADING_PAGE      = 0x01,
        DEVICE_PAGE       = 0x02
};


void displayController();

void updateMainPage();
void updateDevicePage();

void connectDevice();
void deviceConnected(struct Monkey monkey);

void downPressed();
void upPressed();
void selectPressed();
void triggerPressed();

void onConnected(struct Monkey monkey);
void onDisconnected();
void onConnectionFailed();
void onUpdateInfos(struct Monkey monkey);

void Task_Display_Controller_Init(void);

#endif /* DISPLAY_CONTROLLER_H_ */
