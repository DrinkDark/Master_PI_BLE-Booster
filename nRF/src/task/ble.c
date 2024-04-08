#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>

#include "ble.h"
#include "../define.h"
#include "monkeylist.h"

#define BLE_STACK_SIZE 2048
#define BLE_PRIORITY 6
#define BLE_TIMEOUT 10000
#define BLE_SCAN_INTERVAL 2000

#define NAME_LEN 30
#define MANUFACTURER_LEN 4

K_THREAD_STACK_DEFINE(BLE_STACK, BLE_STACK_SIZE);
static struct k_thread bleThread;

LOG_MODULE_REGISTER(ble, LOG_LEVEL_DBG);

//Function to initialize the ble thread
void ble_thread_init(){
    k_thread_create	(&bleThread,
                BLE_STACK,										        
                BLE_STACK_SIZE,
                (k_thread_entry_t)ble_controller,
                NULL,
                NULL,
                NULL,
                BLE_PRIORITY,
                0,
                K_NO_WAIT);	

    k_thread_name_set(&bleThread, "bleThread");
    k_thread_start(&bleThread);

    #ifdef DEBUG_MODE
    printk("ble_thread_init\n");
    #endif
}

// Funtion to start the ble controller
void ble_controller(){
    int err;

	err = bt_enable(NULL);
	if (err) {
        #ifdef DEBUG_MODE
		    printk("Bluetooth init failed (err %d)\n", err);
        #endif
		return;
	}

    #ifdef DEBUG_MODE
        printk("Bluetooth initialized\n");
    #endif

    // Start the ble scan and wait until the end of the program
    ble_start_scan();
    while (true)
    {
        
    }
}

// Function to start the ble scan
int ble_start_scan(){
    int err;

	err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, ble_device_found);
	if (err) {
        #ifdef DEBUG_MODE
            printk("ble_start_scan failed (err %d)\n", err);
        #endif
		return err;
	}
    #ifdef DEBUG_MODE
        printk("ble_start_scan\n");
    #endif
    return 0;
}

// Function to stop the ble scan
int ble_stop_scan(){
    int err;

    err = bt_le_scan_stop();
    if (err) {
        #ifdef DEBUG_MODE
            printk("ble_stop_scan failed (err %d)\n", err);
        #endif
        return err;
    }
    #ifdef DEBUG_MODE
        //printk("ble_stop_scan\n");
    #endif

    return 0;
}

// Function called when a device is found. It will parse the advertising data to find the device name and the manufacturer data
void ble_device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad){
    char addr_str[BT_ADDR_LE_STR_LEN];
    char name[NAME_LEN];
    uint8_t manufacturerData[4];

    struct net_buf_simple *data1 = malloc(sizeof(struct net_buf_simple));
    struct net_buf_simple *data2 = malloc(sizeof(struct net_buf_simple));

    memset(name, 0, NAME_LEN);
    *data1 = *ad;
    *data2 = *ad;

    // Process the received data to extract the useful information
    bt_data_parse(data1, manufacturer_data_cb, manufacturerData);
    bt_data_parse(data2, data_cb, name);
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    
    // If the device is a device of interest (manufacturer = 0x5A02 = University of Applied Sciences Valais / 
    // Haute Ecole Valaisanne), add it to the list or udpate if already exist
    if(manufacturerData[0] == 0x5A && manufacturerData[1] == 0x02){
        appendOrModifyMonkey(parse_device_name(name), rssi, manufacturerData[2], manufacturerData[3], *addr, k_uptime_get_32());
        #ifdef DEBUG_MODE
            printMonkeys();
        #endif 
    }  

    free(data1);
    free(data2);
}

// Function to remove a device from the list if it has not been seen for a certain time (BLE_TIMEOUT)
void ble_remove_device(){
    uint32_t currentTime = k_uptime_get_32();

    int tot = getNumMonkeys();
    struct Monkey array[tot];
    getAllMonkeys(array);

    // Get through the list of devices and remove the ones that have not been seen for a certain time
    for(int i = 0; i < tot;i++)
    {
        if((currentTime - array[i].lastSeen) >= BLE_TIMEOUT){
            #ifdef DEBUG_MODE
                printk("Monkey %d removed\n", array[i].num);
            #endif           
            removeMonkey(array[i].num);
        }
    }
}

// Function to parse the device name and extract the device number
int parse_device_name(char* name) {
    int value = 0;
    sscanf(name, "%*[^0-9]%d", &value);
    return value;
}

// Function to parse the advertising data and extract the device name
bool data_cb(struct bt_data *data, void *user_data)
{
	char *name = user_data;

	switch (data->type) {
	case BT_DATA_NAME_SHORTENED:
	case BT_DATA_NAME_COMPLETE:
		memcpy(name, data->data, MIN(data->data_len, NAME_LEN - 1));
        name[MIN(data->data_len, NAME_LEN - 1)] = '\0';
        return false;
	default:
		return true;
	}
}

// Function to parse the advertising data and extract the device name
bool manufacturer_data_cb(struct bt_data *data, void *user_data)
{
	char *name = user_data;

	switch (data->type) {
    case BT_DATA_MANUFACTURER_DATA:
		memcpy(name, data->data, MIN(data->data_len, 4));
		return false;
	default:
		return true;
	}
}

// Function to connect to a specific device
void ble_connect(void){

}

// Function to disconnect from a specific device
void ble_disconnect(void){

}

// Function to send data to a specific device
void ble_send_data(uint8_t *data, uint16_t len){

}