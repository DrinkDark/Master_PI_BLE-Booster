#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <bluetooth/gatt_dm.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>

#include "ble.h"
#include "../define.h"
#include "connection.h"

K_EVENT_DEFINE(event);

// Define callbacks for the BLE connection
BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = ble_connected_cb,
	.disconnected = ble_disconnected_cb,
};

K_THREAD_STACK_DEFINE(BLE_STACK, BLE_STACK_SIZE);
static struct k_thread bleThread;

LOG_MODULE_REGISTER(ble, LOG_LEVEL_DBG);

struct k_work work;

struct snes_client snes;
const struct snes_client_cb snes_callbacks = {
    .cmd_sent = ble_data_written_cb,
    .status_received = ble_status_received_cb,
    .dor_received = ble_dor_received_cb,
    .device_id_received = ble_device_id_received_cb,
    .mic_gain_received = ble_mic_gain_received_cb,
    .status_unsubscribed = ble_status_unsubscribed_cb,
    .dor_unsubscribed = ble_dor_unsubscribed_cb,
    .device_id_unsubscribed = ble_device_id_unsubscribed_cb,
    .mic_gain_unsubscribed = ble_mic_gain_unsubscribed_cb
};

struct snes_client_init_param snes_init_params = {
    .cb = snes_callbacks
};

//struct bt_gatt_dm snes_dm;

static struct bt_gatt_dm_cb dm_callbacks = {
    .completed = ble_discovery_complete_cb,
    .service_not_found = ble_discovery_service_not_found_cb,
};

struct Monkey connectedMonkey;
uint16_t monkey_handle;

struct bt_uuid_128 monkey_src_UUID = BT_UUID_INIT_128(BT_UUID_SNES_VAL);

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

void ble_fem_init(void) {
    const struct device *dev;

    // Set GPIO pins for TX enable
    dev = device_get_binding(TX_EN);
    gpio_pin_configure(dev, TX_EN_PIN, GPIO_OUTPUT_ACTIVE);

    #ifdef DEBUG_MODE
        printk("ble_fem_init\n");
    #endif
}

int ble_init(void){
    setConnectCallback(ble_connect);
    setDisconnectCallback(ble_disconnect);
    setRecordingToggleCallback(ble_toggle_recording);
    setResetCollarCallback(ble_reset_collar);
    setOpenCollarCallback(ble_open_collar);
    
    snes = (struct snes_client){0};
    connectedMonkey = (struct Monkey){0};

    int err = snes_client_init(&snes, &snes_init_params);
    if (err) {
        #ifdef DEBUG_MODE
            printk("SNES client init failed (err %d)\n", err);
        #endif
        return -1;
    }

    err = bt_enable(NULL);

    if (err) {
        #ifdef DEBUG_MODE
            printk("Bluetooth init failed (err %d)\n", err);
        #endif
        return -1;
    }

    #ifdef DEBUG_MODE
        printk("Bluetooth initialized\n");
    #endif
    return 0;
}

// Funtion to start the ble controller
void ble_controller(struct k_work *work){
    ble_init(); 
    ble_fem_init();
    ble_start_scan();

    while (true)
    {   
        ble_remove_device();
        k_sleep(K_MSEC(200));
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
        uint32_t currentTime = k_uptime_get_32();
        appendOrModifyMonkey(ble_parse_device_name(name), rssi, manufacturerData[2], manufacturerData[3], *addr, currentTime);
        #ifdef DEBUG_MODE
            //printMonkeys();
        #endif 
    }  

    free(data1);
    free(data2);
}

// Function to remove a device from the list if it has not been seen for a certain time (BLE_SCAN_INTERVAL)
void ble_remove_device(){
    uint32_t currentTime = k_uptime_get_32();

    int nbrMonkeys = getNumMonkeys();
    struct Monkey array[nbrMonkeys];
    getAllMonkeys(array);

    // Get through the list of devices and remove the ones that have not been seen for a certain time
    for(int i = 0; i < nbrMonkeys;i++)
    {
        if((currentTime - array[i].lastSeen) >= BLE_SCAN_INTERVAL){
            #ifdef DEBUG_MODE
                //printk("Monkey %d removed\n", array[i].num);
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
	}
        
    snes.conn = bt_conn_ref(conn);
    bt_conn_unref(conn);
    
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

    if(conn != snes.conn){
        return;
    }

    if (err != 0) {
		printk("Failed to connect to %s (%u)\n", addr, err);

		bt_conn_unref(snes.conn);

        connectionFailed();
        ble_start_scan();
		return;
	}

    err = bt_gatt_dm_start(snes.conn, &monkey_src_UUID.uuid, &dm_callbacks, &snes);
      
    if (err != 0) {
		printk("Failed to assign service handle");

		bt_conn_unref(snes.conn);
		snes = (struct snes_client){0};
        connectedMonkey = (struct Monkey){0};

        connectionFailed();
		return;
	}

    connected(connectedMonkey);

    #ifdef DEBUG_MODE
        printk("Connected to monkey %d\n", connectedMonkey.num);
    #endif
}

void ble_discovery_complete_cb(struct bt_gatt_dm *dm, void *context){
    snes.conn = context;
    #ifdef DEBUG_MODE
        printk("Discovery complete\n");
    #endif

    snes_handles_assign(dm, &snes);
    snes_status_subscribe_receive(&snes);
    snes_dor_subscribe_receive(&snes);
    snes_device_id_subscribe_receive(&snes);
    snes_mic_gain_subscribe_receive(&snes);

    bt_gatt_dm_data_release(dm);
}

void ble_discovery_service_not_found_cb(struct bt_gatt_dm *dm, void *context){
    #ifdef DEBUG_MODE
        printk("Service not found\n");
    #endif

    ble_disconnect();
}

// Function to disconnect from a specific device
void ble_disconnect(void){
    bt_conn_disconnect(snes.conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

// Function called when a device is disconnected
void ble_disconnected_cb(struct bt_conn *conn, uint8_t reason)
{
	if (conn != snes.conn) {
        printk("Conn different from snes.conn");
		return;
	}

    #ifdef DEBUG_MODE
        printk("Disconnected from monkey %d, reason 0x%x\n", connectedMonkey.num, reason);
    #endif
	
	bt_conn_unref(snes.conn);
    
    ble_remove_device();
    
    disconnected();
    ble_start_scan();
}

// Function to open the collar
void ble_open_collar(void) {
    uint8_t data[] = {BLE_MSG_HEADER, BLE_MSG_OPEN_COLLAR};
    snes_client_cmd_send(&snes, data, sizeof(data));

    #ifdef DEBUG_MODE
        printk("Open collar\n");
    #endif
}

// Function to reset the collar
void ble_reset_collar(void){
    uint8_t data[] = {BLE_MSG_HEADER, BLE_MSG_RESET_DEVICE};
    snes_client_cmd_send(&snes, data, sizeof(data));
    
    #ifdef DEBUG_MODE
        printk("Reset collar\n");
    #endif
}

// Function to toggle the recording
void ble_toggle_recording(void){
    uint8_t data[] = {BLE_MSG_HEADER, BLE_MSG_TOGGLE_RECORDING};
    snes_client_cmd_send(&snes, data, sizeof(data));
    
    #ifdef DEBUG_MODE
        printk("Toggle recording\n");
    #endif
}

// Callback function for when the data has been written
void ble_data_written_cb(struct snes_client *snes, uint8_t err, const uint8_t *data, uint16_t len) {
    #ifdef DEBUG_MODE
        printk("Data written\n");
    #endif
}

// Callback function for when the status is received
uint8_t ble_status_received_cb(struct snes_client *snes, const uint8_t *data, uint16_t len) {
    connectedMonkey.state = data[0];
    updateInfos(connectedMonkey);
    #ifdef DEBUG_MODE
        printk("Status received\n");
    #endif

    return 0;
}

// Callback function for when the DOR is received
uint8_t ble_dor_received_cb(struct snes_client *snes, const uint8_t *data, uint16_t len) {
    connectedMonkey.record_time = data[0];
    updateInfos(connectedMonkey);
    #ifdef DEBUG_MODE
        printk("Days of recording received\n");
    #endif

    return 0;
}

// Callback function for when the device ID is received
uint8_t ble_device_id_received_cb(struct snes_client *snes, const uint8_t *data, uint16_t len) {
    connectedMonkey.num = data[0];
    updateInfos(connectedMonkey);
    #ifdef DEBUG_MODE
        printk("Device ID received\n");
    #endif

    return 0;
}

// Callback function for when the mic gain is received
uint8_t ble_mic_gain_received_cb(struct snes_client *snes, const uint8_t *data, uint16_t len) {
    #ifdef DEBUG_MODE
        printk("Microphone gain received\n");
    #endif

    return 0;
}

// Callback function for when the status is unsubscribed
void ble_status_unsubscribed_cb(struct snes_client *snes) {
    #ifdef DEBUG_MODE
        printk("Status notification unsubscribed\n");
    #endif
}

// Callback function for when the DOR is unsubscribed
void ble_dor_unsubscribed_cb(struct snes_client *snes) {
    #ifdef DEBUG_MODE
        printk("Days of recording notification unsubscribed\n");
    #endif
}

// Callback function for when the device ID is unsubscribed
void ble_device_id_unsubscribed_cb(struct snes_client *snes) {
    #ifdef DEBUG_MODE
        printk("Device ID notification unsubscribed\n");
    #endif
}

// Callback function for when the mic gain is unsubscribed
void ble_mic_gain_unsubscribed_cb(struct snes_client *snes) {
    #ifdef DEBUG_MODE
        printk("Microphone gain notification unsubscribed\n");
    #endif
}
