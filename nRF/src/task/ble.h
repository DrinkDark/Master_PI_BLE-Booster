#ifndef _BLE_H_
#define _BLE_H_

#include <zephyr/types.h>
#include <zephyr/bluetooth/addr.h>
#include "monkeylist.h"

#define BLE_STACK_SIZE 2048
#define BLE_PRIORITY 6
#define BLE_TIMEOUT 3000
#define BLE_SCAN_INTERVAL 2000

#define NAME_LEN 30
#define MANUFACTURER_DATA_LEN 4

/** @brief UUID of the VAL Characteristic. **/
#define BT_UUID_SNES_VAL \
	BT_UUID_128_ENCODE(0x00000201, 0x4865, 0x7673, 0x025A, 0x4845532D534F)

/** @brief UUID of the CMD Characteristic. **/
#define BT_UUID_SNES_CMD_VAL \
	BT_UUID_128_ENCODE(0x00000202, 0x4865, 0x7673, 0x025A, 0x4845532D534F)

/** @brief UUID of the Status Characteristic. **/
#define BT_UUID_SNES_STATUS_VAL \
	BT_UUID_128_ENCODE(0x00000203, 0x4865, 0x7673, 0x025A, 0x4845532D534F)

/** @brief UUID of the Status Characteristic. **/
#define BT_UUID_SNES_DOR_VAL \
	BT_UUID_128_ENCODE(0x00000204, 0x4865, 0x7673, 0x025A, 0x4845532D534F)

/** @brief UUID of the Device Identifier. **/

#define BT_UUID_SNES_DEVICE_ID_VAL \
	BT_UUID_128_ENCODE(0x00000205, 0x4865, 0x7673, 0x025A, 0x4845532D534F)

/** @brief UUID of the Mic Input Gain. **/
#define BT_UUID_SNES_MIC_INPUT_GAIN_VAL \
	BT_UUID_128_ENCODE(0x00000206, 0x4865, 0x7673, 0x025A, 0x4845532D534F)

void ble_thread_init(void);

int ble_init(void);

void ble_controller(struct k_work *work);
int ble_start_scan(void);
int ble_stop_scan(void);

void ble_device_found_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad);
void ble_remove_device();

bool ble_manufacturer_data_cb(struct bt_data *data, void *user_data);
bool ble_data_cb(struct bt_data *data, void *user_data);

void ble_connect(struct Monkey monkey);
void ble_connected_cb(struct bt_conn *conn, uint8_t err);
void ble_disconnect(void);
void ble_disconnected_cb(struct bt_conn *conn, uint8_t reason);

void ble_data_written_cb();
void ble_open_collar(void);
void ble_reset_collar(void);
void ble_toggle_recording(void);

void ble_discovery_complete_cb(struct bt_gatt_dm *dm, void *context);
void ble_discovery_service_not_found_cb(struct bt_gatt_dm *dm, void *context);

void ble_param_updated_cb(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout);

bool ble_param_request_cb(struct bt_conn *conn, struct bt_le_conn_param *param);
void ble_exchange_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_exchange_params *params);

int ble_parse_device_name(char* name);


#endif /*_BLE_H_*/