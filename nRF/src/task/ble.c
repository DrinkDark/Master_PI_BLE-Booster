#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>

void ble_init(){}

void ble_start_scan(){

}

void ble_stop_scan(){

}

void ble_device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad){

}

void ble_connect(void){

}

void ble_disconnect(void){

}

void ble_send_data(uint8_t *data, uint16_t len){

}