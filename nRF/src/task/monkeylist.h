#ifndef MONKEYLIST_H
#define MONKEYLIST_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/mutex.h>

#include <zephyr/bluetooth/bluetooth.h>

extern struct k_heap monkeyListHeap;

enum main_state {
        ST_INIT         = 0x00,
        ST_WAIT_SD_CARD = 0x01,
        ST_IDLE         = 0x02,
        ST_RECORDING    = 0x03,
        ST_DISK_FULL    = 0x04,
        ST_LOW_BATT     = 0x05,
        ST_POWER_SAVING = 0x06,
        ST_ERROR        = 0xff
};

// Define the struct Monkey
struct Monkey {         
    int num;
    int rssi;
    int record_time;
    enum main_state state;
    struct Monkey * next;
    bt_addr_le_t btAddress;
    
};

// Function prototypes
void initMonkeyList();
void appendOrModifyMonkey(int num, int rssi, int record_time, enum main_state state, bt_addr_le_t address);
void removeMonkey(int num);
void printMonkeys();
void getAllMonkeys(struct Monkey* monkeysArray);
bool getMonkeyAtIndex(struct Monkey* monkey, int index);
bool getMonkeyByID(struct Monkey* monkey, int id);
bool getThreeMonkeys(struct Monkey* monkeys, int startIndex);
int getNumMonkeys();

#endif /* MONKEYLIST_H */
