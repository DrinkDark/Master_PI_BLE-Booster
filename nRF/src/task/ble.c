#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
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
    .le_param_updated = ble_param_updated_cb,
    .le_param_req = ble_param_request_cb
};

K_THREAD_STACK_DEFINE(BLE_STACK, BLE_STACK_SIZE);
static struct k_thread bleThread;

LOG_MODULE_REGISTER(ble, LOG_LEVEL_DBG);

struct k_work work;

struct bt_conn *connectedDevice;
struct Monkey connectedMonkey;
uint16_t monkey_handle;

int current_state = STATE_INIT;

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

    k_event_set(&event, BLE_EV_DEFAULT);
}

int ble_init(void){
    setConnectCallback(ble_connect);
    setDisconnectCallback(ble_disconnect);
    setRecordingToggleCallback(ble_toggle_recording);
    setResetCollarCallback(ble_reset_collar);
    setOpenCollarCallback(ble_open_collar);
    
    connectedDevice = NULL;
    connectedMonkey = (struct Monkey){0};

    int err = bt_enable(NULL);

    if (err) {
        #ifdef DEBUG_MODE
            printk("Bluetooth init failed (err %d)\n", err);
        #endif
        return 0;
    }

    #ifdef DEBUG_MODE
        printk("Bluetooth initialized\n");
    #endif

    k_event_set(&event, BLE_EV_SCAN);

    return 1;
}

// Funtion to start the ble controller
void ble_controller(struct k_work *work){
    int err;

    while (true)
    {   
        uint32_t ev = k_event_wait(&event, 0xFFF, false, K_NO_WAIT);
        printk("Event: %d\n", ev);
        switch (current_state) {
            case STATE_INIT:
            printk("STATE_INIT\n");
                if (ev == BLE_EV_SCAN) {
                    current_state = STATE_SCANNING;
                }                
                break;

            case STATE_SCANNING:
            printk("STATE_SCANNING\n");
                if (ev == BLE_EV_CONNECTING){
                    current_state = STATE_CONNECTING;
                }
                break;

            case STATE_CONNECTING:
            printk("STATE_CONNECTING\n");
                if (ev == BLE_EV_CONNECTED) {
                    current_state = STATE_CONNECTED;
                }
                break;

            case STATE_CONNECTED:
            printk("STATE_CONNECTED\n");
                if (ev == BLE_EV_DISCOVER_CHARA){
                    current_state = STATE_DISCOVER_CHARACTERISTIC;
                }
                break;

            case STATE_DISCOVER_CHARACTERISTIC:
            printk("STATE_DISCOVER_CHARACTERISTIC\n");
                if(ev == BLE_EV_CHARA_DISCOVERED){
                    current_state = STATE_WAIT;
                }
                break;
            case STATE_WAIT:
            printk("STATE_WAIT\n");
                if(ev == BLE_EV_RELEASE){
                    current_state = STATE_RELEASE;
                } else if(ev == BLE_EV_RESET){
                    current_state = STATE_RESET;
                } else if(ev == BLE_EV_TOGGLE_RECORDING){
                    current_state = STATE_TOGGLE_RECORDING;
                } else if (ev == BLE_EV_DISCONNECT){
                    current_state = STATE_DISCONNECTING;
                }
                break;
            case STATE_RELEASE:
            case STATE_RESET:
            case STATE_TOGGLE_RECORDING:
            printk("STATE_TOGGLE_RECORDING / RESET / TOGGLE\n");
                if(ev == BLE_EV_DEFAULT){
                    current_state = STATE_WAIT;
                }
                break;
            case STATE_DISCONNECTING:
            printk("STATE_DISCONNECTING\n");
                if(ev == BLE_EV_DISCONNECTED){
                    current_state = STATE_SCANNING;
                }
                break;

            default:
                printk("Unknown state encountered!\n");
                current_state = STATE_SCANNING;
                break;
        }

        switch (current_state) {
            case STATE_INIT:
                err = ble_init();
                if(err != 1){
                    return;
                }
                break;

            case STATE_SCANNING:
                err = ble_start_scan();
                if(err != 1){
                    return;
                }
                break;

            case STATE_CONNECTING:
                break;

            case STATE_CONNECTED:
                ble_discover_service();
                break;

            case STATE_DISCOVER_CHARACTERISTIC:
                break;

            case STATE_WAIT:
                break;

            case STATE_RELEASE:
                break;

            case STATE_RESET:
                break;

            case STATE_TOGGLE_RECORDING:
                break;

            case STATE_DISCONNECTING:
                ble_disconnect();
                break;

            default:
                printk("Unknown state encountered!\n");
                break;
        }
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
    k_event_set(&event, BLE_EV_CONNECTING);

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
        bt_conn_unref(conn);
    }
    connectedMonkey = monkey;

    k_event_set(&event, BLE_EV_CONNECTING);

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

		k_event_set(&event, BLE_EV_SCAN);

		return;
	}

    static struct bt_gatt_exchange_params exchange_params;

	exchange_params.func = ble_exchange_func;
	err = bt_gatt_exchange_mtu(conn, &exchange_params);
	if (err) {
		LOG_WRN("MTU exchange failed (err %d)\n", err);
	}

    #ifdef DEBUG_MODE
        printk("Connected to monkey %d\n", connectedMonkey.num);
    #endif

    connected(connectedMonkey);

    k_event_set(&event, BLE_EV_CONNECTED);
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
    k_event_set(&event, BLE_EV_SCAN);
}

bool ble_param_request_cb(struct bt_conn *conn, struct bt_le_conn_param *param){
    #ifdef DEBUG_MODE
        printk("Connection parameters update request. min: %d, max: %d, latency: %d, tmieout: %d\n", param->interval_min, param->interval_max, param->latency, param->timeout); 
    #endif

    int ret = bt_conn_le_param_update(conn, param);
    return (ret == 0);
}

void ble_param_updated_cb(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout){
    #ifdef DEBUG_MODE
        printk("Connection parameters updated. interval: %d, latency: %d, timeout: %d\n", interval, latency, timeout);
    #endif

}

void ble_discover_service() {
    int err;
    struct bt_gatt_discover_params discover_params;

    discover_params.uuid = &monkey_src_UUID.uuid;
    discover_params.func = ble_service_discovered_cb;
    discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
    discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
    discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

    #ifdef DEBUG_MODE
        printk("Discover service started\n");
    #endif

    err = bt_gatt_discover(connectedDevice, &discover_params);
    if (err) {
        #ifdef DEBUG_MODE
            printk("Discover service failed (err %d)\n", err);
        #endif
    }
}

uint8_t ble_service_discovered_cb(struct bt_conn *conn, 
                            const struct bt_gatt_attr *attr,
                            struct bt_gatt_discover_params *params){
    monkey_handle = bt_gatt_attr_value_handle(attr);
    printk("Monkey value handle %u\n", monkey_handle);

     k_event_set(&event, BLE_EV_CHARA_DISCOVERED);
    return BT_GATT_ITER_CONTINUE; 
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
    k_event_set(&event, BLE_EV_RELEASE);

    //k_event_set(&event, BLE_EV_DEFAULT);
}

// Function to reset the collar
void ble_reset_collar(void){
    k_event_set(&event, BLE_EV_RESET);

    //k_event_set(&event, BLE_EV_DEFAULT);
}

// Function to toggle the recording
void ble_toggle_recording(void){
    k_event_set(&event, BLE_EV_TOGGLE_RECORDING);

    //k_event_set(&event, BLE_EV_DEFAULT);
}