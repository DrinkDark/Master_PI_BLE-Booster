#ifndef CONNECTION_H
#define CONNECTION_H

#include "monkeylist.h"     //include monkey list to get Monkey struct type

// Define callback function pointer types
typedef void (*ConnectCallback)(struct Monkey);
typedef void (*DisconnectCallback)();
typedef void (*RecordingToggleCallback)();
typedef void (*ResetCollarCallback)();
typedef void (*OpenCollarCallback)();

typedef void (*ConnectedCallback)(struct Monkey);
typedef void (*ConnectionFailedCallback)();
typedef void (*UpdateInfosCallback)(struct Monkey);
typedef void (*DisconnectedCallback)();

// Set callback functions
void setConnectCallback(ConnectCallback callback);
void setDisconnectCallback(DisconnectCallback callback);
void setRecordingToggleCallback(RecordingToggleCallback callback);
void setResetCollarCallback(ResetCollarCallback callback);
void setOpenCollarCallback(OpenCollarCallback callback);

void setConnectedCallback(ConnectedCallback callback);
void setConnectionFailedCallback(ConnectionFailedCallback callback);
void setUpdateInfosCallback(UpdateInfosCallback callback);
void setDisconnectedCallback(DisconnectedCallback callback);

// Methods
void connect(struct Monkey monkey);
void disconnect();
void toggleRecording();
void resetCollar();
void openCollar();

void connected(struct Monkey monkey);
void connectionFailed();
void updateInfos(struct Monkey monkey);
void disconnected();

#endif /* CONNECTION_H */