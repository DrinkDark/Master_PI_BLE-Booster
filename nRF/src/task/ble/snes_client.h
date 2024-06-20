/*
 * Copyright (c) 2023 HESSO-VS, HEI Sion
 */

#ifndef BT_SPEAK_NO_EVIL_SRV_CLIENT_H_
#define BT_SPEAK_NO_EVIL_SRV_CLIENT_H_

/**
 * @file
 * @defgroup snes_client Bluetooth LE GATT SNES Client API
 * @{
 * @brief API for the Bluetooth LE GATT HEI Speak No Evil Service (SNES) Client.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/conn.h>
#include <bluetooth/gatt_dm.h>

/** @brief Handles on the connected peer device that are needed to interact with
 * the device.
 */
struct snes_client_handles {

    /** Handle of the SNES CMD characteristic, as provided by a discovery. */
	uint16_t cmd;

    /** Handle of the SNES STATUS characteristic, as provided by a discovery. */
	uint16_t status;

    /** Handle of the CCC descriptor of the SNES STATUS characteristic, as provided by a discovery. */
	uint16_t status_ccc;

    /** 
     * Handle of the SNES DOR characteristic, as provided by a discovery. 
     * This characterisitic is used to read the number of days already recorded (DOR) on the device.
     */
	uint16_t dor;

    /** Handle of the CCC descriptor of the SNES DOR characteristic, as provided by a discovery. */
	uint16_t dor_ccc;

    /** Handle of the SNES DEVICE_ID characteristic, as provided by a discovery. */
	uint16_t device_id;

    /** Handle of the CCC descriptor of the SNES DEVICE ID characteristic, as provided by a discovery. */
	uint16_t device_id_ccc;

    /** Handle of the SNES MIC GAIN characteristic, as provided by a discovery. */
	uint16_t mic_gain;

    /** Handle of the CCC descriptor of the SNES MIC GAIN characteristic, as provided by a discovery. */
	uint16_t mic_gain_ccc;
};

struct snes_client;

/** @brief SNES Client callback structure. */
struct snes_client_cb {
	/** @brief Data sent callback.
	 *
	 * The data has been sent and written to the SNES CMD Characteristic.
	 *
	 * @param[in] snes SNES Client instance.
	 * @param[in] err  ATT error code.
	 * @param[in] data Transmitted data.
	 * @param[in] len  Length of transmitted data.
	 */
	void (*cmd_sent)(struct snes_client *snes, uint8_t err, const uint8_t *data, uint16_t len);

	/** @brief Status received callback.
	 *
	 * The data has been received as a notification of the SNES STATUS Characteristic.
	 *
	 * @param[in] snes SNES Client instance.
	 * @param[in] data Received data.
	 * @param[in] len  Length of received data.
	 *
	 * @retval BT_GATT_ITER_CONTINUE To keep notifications enabled.
	 * @retval BT_GATT_ITER_STOP To disable notifications.
	 */
	uint8_t (*status_received)(struct snes_client *snes, const uint8_t *data, uint16_t len);

	/** @brief Days of Records received callback.
	 *
	 * The data has been received as a notification of the SNES DOR Characteristic.
	 *
	 * @param[in] snes SNES Client instance.
	 * @param[in] data Received data.
	 * @param[in] len  Length of received data.
	 *
	 * @retval BT_GATT_ITER_CONTINUE To keep notifications enabled.
	 * @retval BT_GATT_ITER_STOP To disable notifications.
	 */
	uint8_t (*dor_received)(struct snes_client *snes, const uint8_t *data, uint16_t len);

	/** @brief Device ID received callback.
	 *
	 * The data has been received as a notification of the SNES DEVICE_ID Characteristic.
	 *
	 * @param[in] snes SNES Client instance.
	 * @param[in] data Received data.
	 * @param[in] len  Length of received data.
	 *
	 * @retval BT_GATT_ITER_CONTINUE To keep notifications enabled.
	 * @retval BT_GATT_ITER_STOP To disable notifications.
	 */
	uint8_t (*device_id_received)(struct snes_client *snes, const uint8_t *data, uint16_t len);

	/** @brief Mic gain received callback.
	 *
	 * The data has been received as a notification of the SNES MIC_GAIN Characteristic.
	 *
	 * @param[in] snes SNES Client instance.
	 * @param[in] data Received data.
	 * @param[in] len  Length of received data.
	 *
	 * @retval BT_GATT_ITER_CONTINUE To keep notifications enabled.
	 * @retval BT_GATT_ITER_STOP To disable notifications.
	 */
	uint8_t (*mic_gain_received)(struct snes_client *snes, const uint8_t *data, uint16_t len);

	/** @brief STATUS notifications disabled callback.
	 *
	 * STATUS notifications have been disabled.
	 *
	 * @param[in] snes  SNES Client instance.
	 */
	void (*status_unsubscribed)(struct snes_client *snes);

	/** @brief DOR notifications disabled callback.
	 *
	 * DOR notifications have been disabled.
	 *
	 * @param[in] snes  SNES Client instance.
	 */
	void (*dor_unsubscribed)(struct snes_client *snes);

	/** @brief DEVICE_ID notifications disabled callback.
	 *
	 * DEVICE_ID notifications have been disabled.
	 *
	 * @param[in] snes  SNES Client instance.
	 */
	void (*device_id_unsubscribed)(struct snes_client *snes);

	/** @brief MIC_GAIN notifications disabled callback.
	 *
	 * MIC_GAIN notifications have been disabled.
	 *
	 * @param[in] snes  SNES Client instance.
	 */
	void (*mic_gain_unsubscribed)(struct snes_client *snes);
};

/** @brief SNES Client structure. */
struct snes_client {

    /** Connection object. */
	struct bt_conn *conn;

    /** Internal state. */
	atomic_t state;

    /** Handles on the connected peer device that are needed to interact with the device. */
	struct snes_client_handles handles;

    /** GATT subscribe parameters for SNES STATUS Characteristic. */
	struct bt_gatt_subscribe_params status_notif_params;

    /** GATT subscribe parameters for SNES DOR Characteristic. */
	struct bt_gatt_subscribe_params dor_notif_params;

    /** GATT subscribe parameters for SNES DEVICE_ID Characteristic. */
	struct bt_gatt_subscribe_params device_id_notif_params;

    /** GATT subscribe parameters for SNES MIC_GAIN Characteristic. */
	struct bt_gatt_subscribe_params mic_gain_notif_params;

    /** GATT write parameters for SNES CMD Characteristic. */
	struct bt_gatt_write_params cmd_write_params;

    /** Application callbacks. */
	struct snes_client_cb cb;
};

/** @brief SNES Client initialization structure. */
struct snes_client_init_param {

    /** Callbacks provided by the user. */
	struct snes_client_cb cb;
};

/** @brief Initialize the SNES Client module.
 *
 * This function initializes the SNES Client module with callbacks provided by the user.
 *
 * @param[in,out] snes       SNES Client instance.
 * @param[in]     init_param SNES Client initialization parameters.
 *
 * @retval 0 If the operation was successful. Otherwise, a negative error code is returned.
 */
int snes_client_init(struct snes_client *snes, const struct snes_client_init_param *init_param);

/** @brief Send data to the server.
 *
 * This function writes to the CMD Characteristic of the server.
 *
 * @note This procedure is asynchronous. Therefore, the data to be sent must remain valid while the function is active.
 *
 * @param[in,out] snes SNES Client instance.
 * @param[in]     data Data to be transmitted.
 * @param[in]     len  Length of data.
 *
 * @retval 0 If the operation was successful. Otherwise, a negative error code is returned.
 */
int snes_client_cmd_send(struct snes_client *snes, const uint8_t *data, uint16_t len);

/** @brief Assign handles to the SNES Client instance.
 *
 * This function should be called when a link with a peer has been established to associate the link to this instance
 * of the module. This makes it possible to handle several links and associate each link to a particular instance of
 * this module. The GATT attribute handles are provided by the GATT DB discovery module.
 *
 * @param[in]     dm   Discovery object.
 * @param[in,out] snes SNES Client instance.
 *
 * @retval 0 If the operation was successful.
 * @retval (-ENOTSUP) Special error code used when UUID of the service does not match the expected UUID.
 * @retval Otherwise, a negative error code is returned.
 */
int snes_handles_assign(struct bt_gatt_dm *dm, struct snes_client *snes);

/** @brief Request the peer to start sending notifications for the STATUS Characteristic.
 *
 * This function enables notifications for the SNES STATUS Characteristic at the peer by writing to the CCC descriptor of
 * the SNES STATUS Characteristic.
 *
 * @param[in,out] snes SNES Client instance.
 *
 * @retval 0 If the operation was successful. Otherwise, a negative error code is returned.
 */
int snes_status_subscribe_receive(struct snes_client *snes);

/** @brief Request the peer to start sending notifications for the DOR Characteristic.
 *
 * This function enables notifications for the SNES STATUS Characteristic at the peer by writing to the CCC descriptor of
 * the SNES DOR Characteristic.
 *
 * @param[in,out] snes SNES Client instance.
 *
 * @retval 0 If the operation was successful. Otherwise, a negative error code is returned.
 */
int snes_dor_subscribe_receive(struct snes_client *snes);

/** @brief Request the peer to start sending notifications for the DEVICE_ID Characteristic.
 *
 * This function enables notifications for the SNES STATUS Characteristic at the peer by writing to the CCC descriptor of
 * the SNES DEVICE_ID Characteristic.
 *
 * @param[in,out] snes SNES Client instance.
 *
 * @retval 0 If the operation was successful. Otherwise, a negative error code is returned.
 */
int snes_device_id_subscribe_receive(struct snes_client *snes);

/** @brief Request the peer to start sending notifications for the MIC_GAIN Characteristic.
 *
 * This function enables notifications for the SNES STATUS Characteristic at the peer by writing to the CCC descriptor of
 * the SNES MIC_GAIN Characteristic.
 *
 * @param[in,out] snes SNES Client instance.
 *
 * @retval 0 If the operation was successful. Otherwise, a negative error code is returned.
 */
int snes_mic_gain_subscribe_receive(struct snes_client *snes);

#ifdef __cplusplus
}
#endif

/**
 *@}
 */

#endif /* BT_SPEAK_NO_EVIL_SRV_CLIENT_H_ */
