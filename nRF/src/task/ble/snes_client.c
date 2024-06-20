/*
 * Copyright (c) 2023, HES-SO Valais-Wallis, HEI, Sion
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "snes_client.h"
#include "snes.h"

#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(snes_c, CONFIG_BT_SNES_LOG_LEVEL);

enum {
	SNES_C_INITIALIZED,
	SNES_C_STATUS_NOTIF_ENABLED,
	SNES_C_DOR_NOTIF_ENABLED,
	SNES_C_DEVICE_ID_NOTIF_ENABLED,
	SNES_C_MIC_GAIN_NOTIF_ENABLED,
	SNES_C_CMD_WRITE_PENDING
};

static uint8_t on_status_received(struct bt_conn *conn, 
                                  struct bt_gatt_subscribe_params *params,
                                  const void *data, 
                                  uint16_t length)
{
	struct snes_client *snes;

	/* Retrieve SNES Client module context. */
	snes = CONTAINER_OF(params, struct snes_client, status_notif_params);

	if (!data) {
		LOG_DBG("[STATUS UNSUBSCRIBED]");
		params->value_handle = 0;
		atomic_clear_bit(&snes->state, SNES_C_STATUS_NOTIF_ENABLED);
		if (snes->cb.status_unsubscribed) {
			snes->cb.status_unsubscribed(snes);
		}
		return BT_GATT_ITER_STOP;
	}

	LOG_DBG("[STATUS NOTIFICATION] data %p length %u", data, length);
	if (snes->cb.status_received) {
		return snes->cb.status_received(snes, data, length);
	}

	return BT_GATT_ITER_CONTINUE;
}

static uint8_t on_dor_received(struct bt_conn *conn, 
                               struct bt_gatt_subscribe_params *params,
                               const void *data, 
                               uint16_t length)
{
	struct snes_client *snes;

	/* Retrieve SNES Client module context. */
	snes = CONTAINER_OF(params, struct snes_client, dor_notif_params);

	if (!data) {
		LOG_DBG("[DOR UNSUBSCRIBED]");
		params->value_handle = 0;
		atomic_clear_bit(&snes->state, SNES_C_DOR_NOTIF_ENABLED);
		if (snes->cb.dor_unsubscribed) {
			snes->cb.dor_unsubscribed(snes);
		}
		return BT_GATT_ITER_STOP;
	}

	LOG_DBG("[DOR NOTIFICATION] data %p length %u", data, length);
	if (snes->cb.dor_received) {
		return snes->cb.dor_received(snes, data, length);
	}

	return BT_GATT_ITER_CONTINUE;
}

static uint8_t on_device_id_received(struct bt_conn *conn, 
                                     struct bt_gatt_subscribe_params *params,
                                     const void *data, 
                                     uint16_t length)
{
	struct snes_client *snes;

	/* Retrieve SNES Client module context. */
	snes = CONTAINER_OF(params, struct snes_client, device_id_notif_params);

	if (!data) {
		LOG_DBG("[DEVICE_ID UNSUBSCRIBED]");
		params->value_handle = 0;
		atomic_clear_bit(&snes->state, SNES_C_DEVICE_ID_NOTIF_ENABLED);
		if (snes->cb.device_id_unsubscribed) {
			snes->cb.device_id_unsubscribed(snes);
		}
		return BT_GATT_ITER_STOP;
	}

	LOG_DBG("[DEVICE_ID NOTIFICATION] data %p length %u", data, length);
	if (snes->cb.device_id_received) {
		return snes->cb.device_id_received(snes, data, length);
	}

	return BT_GATT_ITER_CONTINUE;
}

static uint8_t on_mic_gain_received(struct bt_conn *conn, 
                                    struct bt_gatt_subscribe_params *params,
                                    const void *data, 
                                    uint16_t length)
{
	struct snes_client *snes;

	/* Retrieve SNES Client module context. */
	snes = CONTAINER_OF(params, struct snes_client, mic_gain_notif_params);

	if (!data) {
		LOG_DBG("[MIC_GAIN UNSUBSCRIBED]");
		params->value_handle = 0;
		atomic_clear_bit(&snes->state, SNES_C_MIC_GAIN_NOTIF_ENABLED);
		if (snes->cb.mic_gain_unsubscribed) {
			snes->cb.mic_gain_unsubscribed(snes);
		}
		return BT_GATT_ITER_STOP;
	}

	LOG_DBG("[STATUS NOTIFICATION] data %p length %u", data, length);
	if (snes->cb.mic_gain_received) {
		return snes->cb.mic_gain_received(snes, data, length);
	}

	return BT_GATT_ITER_CONTINUE;
}

static void on_cmd_sent(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params)
{
	struct snes_client *snes_c;
	const void *data;
	uint16_t length;

	/* Retrieve NUS Client module context. */
	snes_c = CONTAINER_OF(params, struct snes_client, cmd_write_params);

	/* Make a copy of volatile data that is required by the callback. */
	data = params->data;
	length = params->length;

	atomic_clear_bit(&snes_c->state, SNES_C_CMD_WRITE_PENDING);
	if (snes_c->cb.cmd_sent) {
		snes_c->cb.cmd_sent(snes_c, err, data, length);
	}
}

int snes_client_init(struct snes_client *snes_c, const struct snes_client_init_param *snes_c_init)
{
	if (!snes_c || !snes_c_init) {
		return -EINVAL;
	}

	if (atomic_test_and_set_bit(&snes_c->state, SNES_C_INITIALIZED)) {
		return -EALREADY;
	}

	memcpy(&snes_c->cb, &snes_c_init->cb, sizeof(snes_c->cb));
	return 0;
}

int snes_client_cmd_send(struct snes_client *snes_c, const uint8_t *data, uint16_t len)
{
	int err;

	if (!snes_c->conn) {
		return -ENOTCONN;
	}

	if (atomic_test_and_set_bit(&snes_c->state, SNES_C_CMD_WRITE_PENDING)) {
		return -EALREADY;
	}

	snes_c->cmd_write_params.func = on_cmd_sent;
	snes_c->cmd_write_params.handle = snes_c->handles.cmd;
	snes_c->cmd_write_params.offset = 0;
	snes_c->cmd_write_params.data = data;
	snes_c->cmd_write_params.length = len;

	err = bt_gatt_write(snes_c->conn, &snes_c->cmd_write_params);
	if (err) {
		atomic_clear_bit(&snes_c->state, SNES_C_CMD_WRITE_PENDING);
	}

	return err;
}

int snes_handles_assign(struct bt_gatt_dm *dm, struct snes_client *snes)
{
	const struct bt_gatt_dm_attr *gatt_service_attr = bt_gatt_dm_service_get(dm);
	const struct bt_gatt_service_val *gatt_service  = bt_gatt_dm_attr_service_val(gatt_service_attr);
	const struct bt_gatt_dm_attr *gatt_chrc;
	const struct bt_gatt_dm_attr *gatt_desc;

	if (bt_uuid_cmp(gatt_service->uuid, BT_UUID_SNES_SERVICE)) {
		return -ENOTSUP;
	}
	LOG_DBG("Getting handles from SNES service.");
	memset(&snes->handles, 0xFF, sizeof(snes->handles));

	/* SNES CMD Characteristic */
	gatt_chrc = bt_gatt_dm_char_by_uuid(dm, BT_UUID_SNES_CMD);
	if (!gatt_chrc) {
		LOG_ERR("Missing SNES CMD characteristic.");
		return -EINVAL;
	}
	/* SNES CMD */
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_SNES_CMD);
	if (!gatt_desc) {
		LOG_ERR("Missing SNES CMD value descriptor in characteristic.");
		return -EINVAL;
	}
	LOG_DBG("Found handle for SNES CMD characteristic.");
	snes->handles.cmd = gatt_desc->handle;

	/* SNES STATUS Characteristic */
	gatt_chrc = bt_gatt_dm_char_by_uuid(dm, BT_UUID_SNES_STATUS);
	if (!gatt_chrc) {
		LOG_ERR("Missing SNES STATUS characteristic.");
		return -EINVAL;
	}
	/* SNES STATUS */
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_SNES_STATUS);
	if (!gatt_desc) {
		LOG_ERR("Missing SNES STATUS value descriptor in characteristic.");
		return -EINVAL;
	}
	LOG_DBG("Found handle for SNES STATUS characteristic.");
	snes->handles.status = gatt_desc->handle;
	/* SNES STATUS CCC */
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_GATT_CCC);
	if (!gatt_desc) {
		LOG_ERR("Missing SNES STATUS CCC in characteristic.");
		return -EINVAL;
	}
	LOG_DBG("Found handle for CCC of SNES STATUS characteristic.");
	snes->handles.status_ccc = gatt_desc->handle;

	/* SNES DOR Characteristic */
	gatt_chrc = bt_gatt_dm_char_by_uuid(dm, BT_UUID_SNES_DAYS_OF_RECORDS);
	if (!gatt_chrc) {
		LOG_ERR("Missing SNES DOR characteristic.");
		return -EINVAL;
	}
	/* SNES DOR */
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_SNES_DAYS_OF_RECORDS);
	if (!gatt_desc) {
		LOG_ERR("Missing SNES DOR value descriptor in characteristic.");
		return -EINVAL;
	}
	LOG_DBG("Found handle for SNES DOR characteristic.");
	snes->handles.dor = gatt_desc->handle;
	/* SNES DOR CCC */
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_GATT_CCC);
	if (!gatt_desc) {
		LOG_ERR("Missing SNES DOR CCC in characteristic.");
		return -EINVAL;
	}
	LOG_DBG("Found handle for CCC of SNES DOR characteristic.");
	snes->handles.dor_ccc = gatt_desc->handle;

	/* SNES DEVICE_ID Characteristic */
	gatt_chrc = bt_gatt_dm_char_by_uuid(dm, BT_UUID_SNES_DEVICE_IDENTIFIER);
	if (!gatt_chrc) {
		LOG_ERR("Missing SNES DEVICE_ID characteristic.");
		return -EINVAL;
	}
	/* SNES DEVICE_ID */
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_SNES_DEVICE_IDENTIFIER);
	if (!gatt_desc) {
		LOG_ERR("Missing SNES DEVICE_ID value descriptor in characteristic.");
		return -EINVAL;
	}
	LOG_DBG("Found handle for SNES DEVICE_ID characteristic.");
	snes->handles.device_id = gatt_desc->handle;
	/* SNES DEVICE_ID CCC */
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_GATT_CCC);
	if (!gatt_desc) {
		LOG_ERR("Missing SNES DEVICE_ID CCC in characteristic.");
		return -EINVAL;
	}
	LOG_DBG("Found handle for CCC of SNES DEVICE_ID characteristic.");
	snes->handles.device_id_ccc = gatt_desc->handle;

	/* SNES MIC_GAIN Characteristic */
	gatt_chrc = bt_gatt_dm_char_by_uuid(dm, BT_UUID_SNES_MIC_INPUT_GAIN);
	if (!gatt_chrc) {
		LOG_ERR("Missing SNES MIC_GAIN characteristic.");
		return -EINVAL;
	}
	/* SNES MIC_GAIN */
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_SNES_MIC_INPUT_GAIN);
	if (!gatt_desc) {
		LOG_ERR("Missing SNES MIC_GAIN value descriptor in characteristic.");
		return -EINVAL;
	}
	LOG_DBG("Found handle for SNES MIC_GAIN characteristic.");
	snes->handles.mic_gain = gatt_desc->handle;
	/* SNES MIC_GAIN CCC */
	gatt_desc = bt_gatt_dm_desc_by_uuid(dm, gatt_chrc, BT_UUID_GATT_CCC);
	if (!gatt_desc) {
		LOG_ERR("Missing SNES MIC_GAIN CCC in characteristic.");
		return -EINVAL;
	}
	LOG_DBG("Found handle for CCC of SNES MIC_GAIN characteristic.");
	snes->handles.mic_gain_ccc = gatt_desc->handle;

	/* Assign connection instance. */
	snes->conn = bt_gatt_dm_conn_get(dm);
	return 0;
}

int snes_status_subscribe_receive(struct snes_client *snes)
{
    int err = 0;

	if (atomic_test_and_set_bit(&snes->state, SNES_C_STATUS_NOTIF_ENABLED)) {
		return -EALREADY;
	}

	snes->status_notif_params.notify       = on_status_received;
	snes->status_notif_params.value        = BT_GATT_CCC_NOTIFY;
	snes->status_notif_params.value_handle = snes->handles.status;
	snes->status_notif_params.ccc_handle   = snes->handles.status_ccc;
	atomic_set_bit(snes->status_notif_params.flags, BT_GATT_SUBSCRIBE_FLAG_VOLATILE);

	err = bt_gatt_subscribe(snes->conn, &snes->status_notif_params);
	if (err) {
		LOG_ERR("Status Subscribe failed (err %d)", err);
		atomic_clear_bit(&snes->state, SNES_C_STATUS_NOTIF_ENABLED);
	} else {
		LOG_DBG("[STATUS SUBSCRIBED]");
	}

    return err;    
}

int snes_dor_subscribe_receive(struct snes_client *snes)
{
    int err = 0;

	if (atomic_test_and_set_bit(&snes->state, SNES_C_DOR_NOTIF_ENABLED)) {
		return -EALREADY;
	}

	snes->dor_notif_params.notify       = on_dor_received;
	snes->dor_notif_params.value        = BT_GATT_CCC_NOTIFY;
	snes->dor_notif_params.value_handle = snes->handles.dor;
	snes->dor_notif_params.ccc_handle   = snes->handles.dor_ccc;
	atomic_set_bit(snes->dor_notif_params.flags, BT_GATT_SUBSCRIBE_FLAG_VOLATILE);

	err = bt_gatt_subscribe(snes->conn, &snes->dor_notif_params);
	if (err) {
		LOG_ERR("DOR Subscribe failed (err %d)", err);
		atomic_clear_bit(&snes->state, SNES_C_DOR_NOTIF_ENABLED);
	} else {
		LOG_DBG("[DOR SUBSCRIBED]");
	}

    return err;    
}

int snes_device_id_subscribe_receive(struct snes_client *snes)
{
    int err = 0;

	if (atomic_test_and_set_bit(&snes->state, SNES_C_DEVICE_ID_NOTIF_ENABLED)) {
		return -EALREADY;
	}

	snes->device_id_notif_params.notify       = on_device_id_received;
	snes->device_id_notif_params.value        = BT_GATT_CCC_NOTIFY;
	snes->device_id_notif_params.value_handle = snes->handles.device_id;
	snes->device_id_notif_params.ccc_handle   = snes->handles.device_id_ccc;
	atomic_set_bit(snes->device_id_notif_params.flags, BT_GATT_SUBSCRIBE_FLAG_VOLATILE);

	err = bt_gatt_subscribe(snes->conn, &snes->device_id_notif_params);
	if (err) {
		LOG_ERR("DEVICE_ID Subscribe failed (err %d)", err);
		atomic_clear_bit(&snes->state, SNES_C_DEVICE_ID_NOTIF_ENABLED);
	} else {
		LOG_DBG("[DEVICE_ID SUBSCRIBED]");
	}

    return err;    
}

int snes_mic_gain_subscribe_receive(struct snes_client *snes)
{
    int err = 0;

	if (atomic_test_and_set_bit(&snes->state, SNES_C_MIC_GAIN_NOTIF_ENABLED)) {
		return -EALREADY;
	}

	snes->mic_gain_notif_params.notify       = on_mic_gain_received;
	snes->mic_gain_notif_params.value        = BT_GATT_CCC_NOTIFY;
	snes->mic_gain_notif_params.value_handle = snes->handles.mic_gain;
	snes->mic_gain_notif_params.ccc_handle   = snes->handles.mic_gain_ccc;
	atomic_set_bit(snes->mic_gain_notif_params.flags, BT_GATT_SUBSCRIBE_FLAG_VOLATILE);

	err = bt_gatt_subscribe(snes->conn, &snes->mic_gain_notif_params);
	if (err) {
		LOG_ERR("MIC_GAIN Subscribe failed (err %d)", err);
		atomic_clear_bit(&snes->state, SNES_C_MIC_GAIN_NOTIF_ENABLED);
	} else {
		LOG_DBG("[MIC_GAIN SUBSCRIBED]");
	}

    return err;    
}
