#include "connection.h"

// Define callback function pointers
static ConnectCallback connectCallback = NULL;
static DisconnectCallback disconnectCallback = NULL;
static RecordingToggleCallback recordingToggleCallback = NULL;
static ResetCollarCallback resetCollarCallback = NULL;
static OpenCollarCallback openCollarCallback = NULL;

static ConnectedCallback connectedCallback = NULL;
static ConnectionFailedCallback connectionFailedCallback = NULL;
static UpdateInfosCallback updateInfosCallback = NULL;
static DisconnectedCallback disconnectedCallback = NULL;

//-----------------------------------------------------------------------------------------------------------------------
/*! setConnectCallback
* @brief Set the callback function for the connect event
* @param callback The callback function
*/
void setConnectCallback(ConnectCallback callback) {
    connectCallback = callback;
}

//-----------------------------------------------------------------------------------------------------------------------
/*! setDisconnectCallback
* @brief Set the callback function for the disconnect event
* @param callback The callback function
*/
void setDisconnectCallback(DisconnectCallback callback) {
    disconnectCallback = callback;
}

//-----------------------------------------------------------------------------------------------------------------------
/*! setRecordingToggleCallback
* @brief Set the callback function for the recording toggle event
* @param callback The callback function
*/
void setRecordingToggleCallback(RecordingToggleCallback callback) {
    recordingToggleCallback = callback;
}

//-----------------------------------------------------------------------------------------------------------------------
/*! setResetCollarCallback
* @brief Set the callback function for the reset collar event
* @param callback The callback function
*/
void setResetCollarCallback(ResetCollarCallback callback) {
    resetCollarCallback = callback;
}

//-----------------------------------------------------------------------------------------------------------------------
/*! setOpenCollarCallback
* @brief Set the callback function for the open collar event
* @param callback The callback function
*/
void setOpenCollarCallback(OpenCollarCallback callback) {
    openCollarCallback = callback;
}

//-----------------------------------------------------------------------------------------------------------------------
/*! setConnectedCallback
* @brief Set the callback function for the connected event
* @param callback The callback function
*/
void setConnectedCallback(ConnectedCallback callback) {
    connectedCallback = callback;
}

//-----------------------------------------------------------------------------------------------------------------------
/*! setConnectionFailedCallback
* @brief Set the callback function for the connection failed event
* @param callback The callback function
*/
void setConnectionFailedCallback(ConnectionFailedCallback callback) {
    connectionFailedCallback = callback;
}

//-----------------------------------------------------------------------------------------------------------------------
/*! setUpdateInfosCallback
* @brief Set the callback function for the update infos event
* @param callback The callback function
*/
void setUpdateInfosCallback(UpdateInfosCallback callback) {
    updateInfosCallback = callback;
}

//-----------------------------------------------------------------------------------------------------------------------
/*! setDisconnectedCallback
* @brief Set the callback function for the disconnected event
* @param callback The callback function
*/
void setDisconnectedCallback(DisconnectedCallback callback) {
    disconnectedCallback = callback;
}

//-----------------------------------------------------------------------------------------------------------------------
/*! connect
* @brief Call the connect callback function
* @param monkey The Monkey struct
*/
void connect(struct Monkey monkey) {
    if (connectCallback != NULL) {
        connectCallback(monkey);
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! disconnect
* @brief Call the disconnect callback function
*/
void disconnect() {
    if (disconnectCallback != NULL) {
        disconnectCallback();
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! toggleRecording
* @brief Call the recording toggle callback function
*/
void toggleRecording() {
    if (recordingToggleCallback != NULL) {
        recordingToggleCallback();
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! resetCollar
* @brief Call the reset collar callback function
*/
void resetCollar() {
    if (resetCollarCallback != NULL) {
        resetCollarCallback();
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! openCollar
* @brief Call the open collar callback function
*/
void openCollar() {
    if (openCollarCallback != NULL) {
        openCollarCallback();
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! connected
* @brief Call the connected callback function
* @param monkey The Monkey struct
*/
void connected(struct Monkey monkey) {
    if (connectedCallback != NULL) {
        connectedCallback(monkey);
    }  
}

//-----------------------------------------------------------------------------------------------------------------------
/*! connectionFailed
* @brief Call the connection failed callback function
*/
void connectionFailed() {
    if (connectionFailedCallback != NULL) {
        connectionFailedCallback();
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! updateInfos
* @brief Call the update infos callback function
* @param monkey The Monkey struct
*/
void updateInfos(struct Monkey monkey) {
    if (updateInfosCallback != NULL) {
        updateInfosCallback(monkey);
    }
}

//-----------------------------------------------------------------------------------------------------------------------
/*! disconnected
* @brief Call the disconnected callback function
*/
void disconnected() {
    if (disconnectedCallback != NULL) {
        disconnectedCallback();
    }
}
