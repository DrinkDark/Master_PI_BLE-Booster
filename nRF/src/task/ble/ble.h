#ifndef _BLE_H_
#define _BLE_H_

#include <zephyr/types.h>
#include <zephyr/bluetooth/addr.h>
#include "../monkeylist.h"
#include "snes.h"
#include "snes_client.h"

#define BLE_STACK_SIZE 2048
#define BLE_PRIORITY 6
#define BLE_SCAN_INTERVAL 4000

#define NAME_LEN 30
#define MANUFACTURER_DATA_LEN 4

#define	BLE_MSG_HEADER 0xa5
#define	BLE_MSG_OPEN_COLLAR 0x01
#define	BLE_MSG_RESET_DEVICE 0x02
#define	BLE_MSG_TOGGLE_RECORDING 0x03
#define	BLE_MSG_SHUTDOWN_HARDWARE 0x04
#define	BLE_MSG_START_HARDWARE 0x05
#define	BLE_MSG_SEND_CURRENT_TIME 0x06
#define	BLE_MSG_SEND_NEW_DEVICE_IDENTIFIER 0x07
#define	BLE_MSG_SEND_NEW_MIC_INPUT_GAIN 0x08
#define	BLE_MSG_CONFIG_BT_CTS_CLIENT 0xff

void ble_thread_init(void);

int ble_init(void);
void ble_fem_init(void);

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

void ble_open_collar(void);
void ble_reset_collar(void);
void ble_toggle_recording(void);

void ble_discovery_complete_cb(struct bt_gatt_dm *dm, void *context);
void ble_discovery_service_not_found_cb(struct bt_gatt_dm *dm, void *context);

void ble_param_updated_cb(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout);

bool ble_param_request_cb(struct bt_conn *conn, struct bt_le_conn_param *param);
void ble_exchange_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_exchange_params *params);

int ble_parse_device_name(char* name);

void ble_data_written_cb(struct snes_client *snes, uint8_t err, const uint8_t *data, uint16_t len);
uint8_t ble_status_received_cb(struct snes_client *snes, const uint8_t *data, uint16_t len);
uint8_t ble_dor_received_cb(struct snes_client *snes, const uint8_t *data, uint16_t len);
uint8_t ble_device_id_received_cb(struct snes_client *snes, const uint8_t *data, uint16_t len);
uint8_t ble_mic_gain_received_cb(struct snes_client *snes, const uint8_t *data, uint16_t len);
void ble_status_unsubscribed_cb(struct snes_client *snes);
void ble_dor_unsubscribed_cb(struct snes_client *snes);
void ble_device_id_unsubscribed_cb(struct snes_client *snes);
void ble_mic_gain_unsubscribed_cb(struct snes_client *snes);


#endif /*_BLE_H_*/