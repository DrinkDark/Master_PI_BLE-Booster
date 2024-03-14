#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>

#include "ble.h"
#include "../define.h"

#define BLE_STACK_SIZE 2048
#define BLE_PRIORITY 6

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

void ble_start_scan(){
    int err;

	err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, ble_device_found);
	if (err) {
        #ifdef DEBUG_MODE
            printk("ble_start_scan failed (err %d)\n", err);
        #endif
		return;
	}
        #ifdef DEBUG_MODE
            printk("ble_start_scan");
        #endif
}

void ble_stop_scan(){
    int err;

    err = bt_le_scan_stop();
    if (err) {
        #ifdef DEBUG_MODE
            printk("ble_stop_scan failed (err %d)\n", err);
        #endif
        return;
    }
    #ifdef DEBUG_MODE
        printk("ble_stop_scan");
    #endif
}

void ble_device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad){
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

    #ifdef DEBUG_MODE
        
        printk("Device found: %s (RSSI %d)\n", addr_str, rssi);
    #endif
}

void ble_connect(void){

}

void ble_disconnect(void){

}

void ble_send_data(uint8_t *data, uint16_t len){

}