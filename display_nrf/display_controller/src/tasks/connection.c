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

// Set callback functions
void setConnectCallback(ConnectCallback callback) {
    connectCallback = callback;
}

void setDisconnectCallback(DisconnectCallback callback) {
    disconnectCallback = callback;
}

void setRecordingToggleCallback(RecordingToggleCallback callback) {
    recordingToggleCallback = callback;
}

void setResetCollarCallback(ResetCollarCallback callback) {
    resetCollarCallback = callback;
}

void setOpenCollarCallback(OpenCollarCallback callback) {
    openCollarCallback = callback;
}



void setConnectedCallback(ConnectedCallback callback) {
    connectedCallback = callback;
}

void setConnectionFailedCallback(ConnectionFailedCallback callback) {
    connectionFailedCallback = callback;
}

void setUpdateInfosCallback(UpdateInfosCallback callback) {
    updateInfosCallback = callback;
}

void setDisconnectedCallback(DisconnectedCallback callback) {
    disconnectedCallback = callback;
}

// Methods implementation
void connect(struct Monkey monkey) {
    if (connectCallback != NULL) {
        connectCallback(monkey);
    }
}

void disconnect() {
    if (disconnectCallback != NULL) {
        disconnectCallback();
    }
}

void toggleRecording() {
    if (recordingToggleCallback != NULL) {
        recordingToggleCallback();
    }
}

void resetCollar() {
    if (resetCollarCallback != NULL) {
        resetCollarCallback();
    }
}

void openCollar() {
    if (openCollarCallback != NULL) {
        openCollarCallback();
    }
}



void connected(struct Monkey monkey) {
    if (connectedCallback != NULL) {
        connectedCallback(monkey);
    }  
}

void connectionFailed() {
    if (connectionFailedCallback != NULL) {
        connectionFailedCallback();
    }
}

void updateInfos(struct Monkey monkey) {
    if (updateInfosCallback != NULL) {
        updateInfosCallback(monkey);
    }
}

void disconnected() {
    if (disconnectedCallback != NULL) {
        disconnectedCallback();
    }
}
