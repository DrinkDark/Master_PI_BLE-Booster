#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <bluetooth/gatt_dm.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>


#include "ble.h"
#include "../define.h"
#include "connection.h"
#include "snes.h"
#include "snes_client.h"

K_EVENT_DEFINE(event);

// Define callbacks for the BLE connection
BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = ble_connected_cb,
	.disconnected = ble_disconnected_cb,
    .le_param_updated = ble_param_updated_cb,
    .le_param_req = ble_param_request_cb
};

K_THREAD_STACK_DEFINE(BLE_STACK, BLE_STACK_SIZE);
static struct k_thread bleThread;

LOG_MODULE_REGISTER(ble, LOG_LEVEL_DBG);

struct k_work work;

struct snes_client snes;
const struct snes_client_cb snes_callbacks = {
    .cmd_sent = ble_write_data,
    /*.status_received = your_status_received_callback_function,
    .dor_received = your_dor_received_callback_function,
    .device_id_received = your_device_id_received_callback_function,
    .mic_gain_received = your_mic_gain_received_callback_function,
    .status_unsubscribed = your_status_unsubscribed_callback_function,
    .dor_unsubscribed = your_dor_unsubscribed_callback_function,
    .device_id_unsubscribed = your_device_id_unsubscribed_callback_function,
    .mic_gain_unsubscribed = your_mic_gain_unsubscribed_callback_function*/
};

struct snes_client_init_param snes_init_params = {
    .cb = snes_callbacks
};

//struct bt_gatt_dm snes_dm;

static struct bt_gatt_dm_cb dm_callbacks = {
    .completed = ble_discovery_complete_cb,
    .service_not_found = ble_discovery_service_not_found_cb,
};

struct bt_conn *connectedDevice;
struct Monkey connectedMonkey;
uint16_t monkey_handle;

struct bt_uuid_128 monkey_src_UUID = BT_UUID_INIT_128(BT_UUID_SNES_VAL);
struct bt_uuid_128 monkey_cmd_UUID = BT_UUID_INIT_128(BT_UUID_SNES_CMD_VAL);

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

    k_work_init(&work, ble_controller);
}

int ble_init(void){
    setConnectCallback(ble_connect);
    setDisconnectCallback(ble_disconnect);
    setRecordingToggleCallback(ble_toggle_recording);
    setResetCollarCallback(ble_reset_collar);
    setOpenCollarCallback(ble_open_collar);
    
    connectedDevice = NULL;
    connectedMonkey = (struct Monkey){0};
    

    int err = snes_client_init(&snes, &snes_init_params);
    if (err) {
                #ifdef DEBUG_MODE
            printk("SNES client init failed (err %d)\n", err);
        #endif
        return 0;
    }

    //bt_gatt_dm_init(&snes_dm, &dm_callbacks, NULL);

    err = bt_enable(NULL);

    if (err) {
        #ifdef DEBUG_MODE
            printk("Bluetooth init failed (err %d)\n", err);
        #endif
        return 0;
    }

    #ifdef DEBUG_MODE
        printk("Bluetooth initialized\n");
    #endif
    return 1;
}

// Funtion to start the ble controller
void ble_controller(struct k_work *work){
    ble_init(); 
    ble_start_scan();

    while (true)
    {   
        
    }
}

// Function to start the ble scan
int ble_start_scan(){
    int err;

	err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, ble_device_found_cb);
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
void ble_device_found_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad){
    char addr_str[BT_ADDR_LE_STR_LEN];
    char name[NAME_LEN];
    uint8_t manufacturerData[MANUFACTURER_DATA_LEN];

    memset(name, 0, NAME_LEN);
    memset(manufacturerData, 0, MANUFACTURER_DATA_LEN);

    struct net_buf_simple *data1 = malloc(sizeof(struct net_buf_simple));
    struct net_buf_simple *data2 = malloc(sizeof(struct net_buf_simple));

    *data1 = *ad;
    *data2 = *ad;

    // Process the received data to extract the useful information
    bt_data_parse(data1, ble_manufacturer_data_cb, manufacturerData);
    bt_data_parse(data2, ble_data_cb, name);
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    
    // If the device is a device of interest (manufacturer = 0x5A02 = University of Applied Sciences Valais / 
    // Haute Ecole Valaisanne), add it to the list or udpate if already exist
    if(manufacturerData[0] == 0x5A && manufacturerData[1] == 0x02){
        appendOrModifyMonkey(ble_parse_device_name(name), rssi, manufacturerData[2], manufacturerData[3], *addr, k_uptime_get_32());
        #ifdef DEBUG_MODE
            printMonkeys();
        #endif 
        ble_remove_device();
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
int ble_parse_device_name(char* name) {
    int value = 0;
    sscanf(name, "%*[^0-9]%d", &value);
    return value;
}

// Function to parse the advertising data and extract the device name
bool ble_data_cb(struct bt_data *data, void *user_data)
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
bool ble_manufacturer_data_cb(struct bt_data *data, void *user_data)
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
void ble_connect(struct Monkey monkey){
    struct bt_conn* conn;
    int err = ble_stop_scan();

	if (err) {
        #ifdef DEBUG_MODE
            printk("%s: Stop LE scan failed (err %d)\n", __func__, err);
        #endif  
        connectionFailed();
		return;
	}

    err = bt_conn_le_create((const bt_addr_le_t*) &monkey.btAddress, BT_CONN_LE_CREATE_CONN,
				        BT_LE_CONN_PARAM_DEFAULT, &conn);

	if (err) {
        #ifdef DEBUG_MODE
            printk("%s: Create conn failed (err %d)\n", __func__, err);
        #endif  
        connectionFailed();
		ble_start_scan();
        return;
	} else {
        connectedDevice = bt_conn_ref(conn);
    }
    connectedMonkey = monkey;

    #ifdef DEBUG_MODE
        printk("Connecting to Monkey %d\n", connectedMonkey.num);
    #endif
}

// Function called when a device is connected
void ble_connected_cb(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if(conn != connectedDevice){
        return;
    }

    if (err != 0) {
		printk("Failed to connect to %s (%u)\n", addr, err);

		bt_conn_unref(connectedDevice);
		connectedDevice = NULL;
        connectedMonkey = (struct Monkey){0};

        connectionFailed();
		return;
	}

    err = bt_gatt_dm_start(connectedDevice, &monkey_src_UUID.uuid, &dm_callbacks, &snes);
      
    if (err != 0) {
		printk("Failed to assign service handle");

		bt_conn_unref(connectedDevice);
		connectedDevice = NULL;
        connectedMonkey = (struct Monkey){0};

        connectionFailed();
		return;
	}

    #ifdef DEBUG_MODE
        printk("Connected to monkey %d\n", connectedMonkey.num);
    #endif
}

void ble_discovery_complete_cb(struct bt_gatt_dm *dm, void *context){
    struct snes_client *snes = context;
    #ifdef DEBUG_MODE
        printk("Discovery complete\n");
    #endif

    snes_handles_assign(dm, snes);
    snes_status_subscribe_receive(snes);
    snes_dor_subscribe_receive(snes);
    snes_device_id_subscribe_receive(snes);
    snes_mic_gain_subscribe_receive(snes);

    bt_gatt_dm_data_release(dm);

}

void ble_discovery_service_not_found_cb(struct bt_gatt_dm *dm, void *context){
    #ifdef DEBUG_MODE
        printk("Service not found\n");
    #endif
}

void ble_exchange_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_exchange_params *params)
{
	if (!err) {
		printk("MTU exchange done\n");
	} else {
		printk("MTU exchange failed (err %" PRIu8 ")", err);
	}
}

// Function to disconnect from a specific device
void ble_disconnect(void){
    bt_conn_disconnect(connectedDevice, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

// Function called when a device is disconnected
void ble_disconnected_cb(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (conn != connectedDevice) {
		return;
	}

    #ifdef DEBUG_MODE
        printk("Disconnected from monkey %d, reason 0x%x\n", connectedMonkey.num, reason);
    #endif
	
	bt_conn_unref(connectedDevice);

	connectedDevice = NULL;
    connectedMonkey = (struct Monkey){0};

    disconnected();

    ble_start_scan();
}

bool ble_param_request_cb(struct bt_conn *conn, struct bt_le_conn_param *param){
    #ifdef DEBUG_MODE
        printk("Connection parameters update request. min: %d, max: %d, latency: %d, timeout: %d\n", param->interval_min, param->interval_max, param->latency, param->timeout); 
    #endif

    int ret = bt_conn_le_param_update(conn, param);
    return (ret == 0);
}

void ble_param_updated_cb(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout){
    #ifdef DEBUG_MODE
        printk("Connection parameters updated. interval: %d, latency: %d, timeout: %d\n", interval, latency, timeout);
    #endif
    connected(connectedMonkey);
}

// Function to send data to a specific device
void ble_write_data(uint8_t *data, uint16_t len){
    int err = bt_gatt_write_without_response_cb(connectedDevice, monkey_handle, data, len,
						false, ble_data_written_cb,
						(void *)((uint32_t)len));
    if(err){
        #ifdef DEBUG_MODE
            printk("Write data failed (err %d)\n", err);
        #endif
        return;
    }
    #ifdef DEBUG_MODE
        printk("Write data : %s\n", data);
    #endif
}

// Function called when the data has been written
void ble_data_written_cb(){
    #ifdef DEBUG_MODE
        printk("Data written\n");
    #endif
}

// Function to open the collar
void ble_open_collar(void) {
    printk("Open collar\n");
}

// Function to reset the collar
void ble_reset_collar(void){
    printk("Reset collar\n");
}

// Function to toggle the recording
void ble_toggle_recording(void){
    printk("Toggle recording\n");
}