#ifndef _BLE_H_
#define _BLE_H_

#include <zephyr/types.h>
#include <zephyr/bluetooth/addr.h>
#include "monkeylist.h"

void ble_thread_init(void);

void ble_controller(void);
int ble_start_scan(void);
int ble_stop_scan(void);

void ble_device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad);
void ble_remove_device();

bool ble_manufacturer_data_cb(struct bt_data *data, void *user_data);
bool ble_data_cb(struct bt_data *data, void *user_data);

void ble_connect(struct Monkey monkey);
void ble_connected(struct bt_conn *conn, uint8_t err);
void ble_disconnect(void);
void ble_disconnected(struct bt_conn *conn, uint8_t reason);

void ble_send_data(uint8_t *data, uint16_t len);
void ble_open_collar(void);
void ble_reset_collar(void);
void ble_toggle_recording(void);

int ble_parse_device_name(char* name);


#endif /*_BLE_H_*/