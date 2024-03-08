#ifndef _BLE_H_
#define _BLE_H_

#include <zephyr/types.h>

void ble_init(void);

void ble_start_scan(void);
void ble_stop_scan(void);
void ble_device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad);
void ble_connect(void);
void ble_disconnect(void);
void ble_send_data(uint8_t *data, uint16_t len);