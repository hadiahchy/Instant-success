#include "websocketthread.h"

WebSocketThread::WebSocketThread(QObject* parent)
    : QThread(parent), webSocketClient(nullptr){
}

void WebSocketThread::run() {
    webSocketClient = new WebSocketClient;
    connect(this, &WebSocketThread::startConnection, webSocketClient, &WebSocketClient::connectToServer);
    connect(webSocketClient, &WebSocketClient::connectSuccess, this, &WebSocketThread::connectSuccess);
    connect(webSocketClient, &WebSocketClient::connectError, this, &WebSocketThread::connectError);
    connect(webSocketClient, &WebSocketClient::disconnected, this, &WebSocketThread::disconnected);
    connect(webSocketClient, &WebSocketClient::messageReceived, this, &WebSocketThread::forwardMessage);
    connect(webSocketClient, &WebSocketClient::updateClientListTxmt, this, &WebSocketThread::updateClientListRecv);
    exec();
}

void WebSocketThread::forwardMessage(const QString& message, const QString& src, const QString& dst) {
    emit messageReceived(message, src, dst);
}

void WebSocketThread::updateClientListRecv(const QStringList& clients) {
    emit updateClientListTxmt(clients);
}
