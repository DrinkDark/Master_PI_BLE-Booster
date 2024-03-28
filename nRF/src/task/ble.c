#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>

#include "ble.h"
#include "../define.h"
#include "monkeylist.h"

#define BLE_STACK_SIZE 2048
#define BLE_PRIORITY 6
#define NAME_LEN 30
#define MANUFACTURER_LEN 4

K_THREAD_STACK_DEFINE(BLE_STACK, BLE_STACK_SIZE);
static struct k_thread bleThread;


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

    ble_start_scan();
    while (true)
    {
        
    }
}

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
        //printk("ble_start_scan");
    #endif
    return 0;
}

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
        //printk("ble_stop_scan");
    #endif

    return 0;
}

void ble_device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad){
    char addr_str[BT_ADDR_LE_STR_LEN];
    char manufacturer_data[30];
    char name[30];
    struct net_buf_simple *data = ad;

    bt_data_parse(data, data_cb, name);

    //bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    if((strstr(name, "Speak No Evil") || strstr(name, "A51 de Adrien"))!= NULL){
        #ifdef DEBUG_MODE
            printk("Device found: %s (RSSI %d)\n", name, rssi);
        #endif
    }
}

int parse_device_name(char* name) {
    int value = 0;
    sscanf(name, "%*[^0-9]%d", &value);
    return value;
}

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

bool manufacturer_data_cb(struct bt_data *data, void *user_data)
{
	char *manufacturer = user_data;
    int len = 0;

	switch (data->type) {
	case BT_DATA_MANUFACTURER_DATA:
        len = MIN(data->data_len, MANUFACTURER_LEN);
        memcpy(manufacturer, data->data, MIN(data->data_len, MANUFACTURER_LEN));
        return false;
	default:
		return false;
	}
}
void ble_connect(void){

}

void ble_disconnect(void){

}

void ble_send_data(uint8_t *data, uint16_t len){

}