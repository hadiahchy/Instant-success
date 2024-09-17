#include "websocketserver.h"

WebSocketServer::WebSocketServer(quint16 port, QObject *parent)
    : QObject{parent},
    m_server(new QWebSocketServer(QStringLiteral("Echo Server"), QWebSocketServer::NonSecureMode, this))
{
    if (m_server->listen(QHostAddress::Any, port)) {
        qInfo() << "WebSocket server started on port " << port << Qt::endl;
        connect(m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConenction);
    } else {
        qInfo() << "Failed to start WebSocket server on port " << port << Qt::endl;
    }
}

WebSocketServer::~WebSocketServer() {
    m_server->close();
    qDeleteAll(m_clients);
}

void WebSocketServer::onNewConenction() {
    QWebSocket *clientSocket = m_server->nextPendingConnection();

    connect(clientSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::processTextMessage);
    connect(clientSocket, &QWebSocket::disconnected, this, &WebSocketServer::socketDisconnected);

    qInfo() << "New client connected! " << clientSocket->peerAddress().toString() << Qt::endl;
}

void WebSocketServer::processTextMessage(const QString &message) {
    QWebSocket *senderSocket = qobject_cast<QWebSocket*>(sender());

    // Decode message
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject jsonObj = doc.object();
        if (jsonObj.contains("type") && jsonObj["type"].isString()) {

            QString type = jsonObj["type"].toString();
            if (type == "listing") {
                // Send client list
                QString OutJsonPacket = getClientListPacket();
                // -- Send to original requester
                senderSocket->sendTextMessage(OutJsonPacket);
                qInfo() << "Sent client list to " << senderSocket->peerAddress().toString() << Qt::endl;


            } else if (type == "msg" && jsonObj.contains("data")
                       && jsonObj["data"].isString() && jsonObj.contains("src")
                       && jsonObj["src"].isString() && jsonObj.contains("dst")
                       && jsonObj["dst"].isString()) {

                QString dstUsername = jsonObj["dst"].toString();
                QString srcUsername = jsonObj["src"].toString();

                // If a new user, add to clients list
                if (!m_clients.contains(srcUsername)) {
                    m_clients[srcUsername] = senderSocket;
                    qInfo() << srcUsername << " client sent first message!" << Qt::endl;
                    // Inform all clients
                    serverBroadcast(srcUsername+" has joined the chat.");
                    //-- update all clients client list
                    serverBroadcastClientList();
                }

                if (jsonObj["data"].toString() == "") {
                    //-- Ignore empty messages, but still add user to client list
                    //-- see above
                    return;
                }

                // Send message to destination
                if (dstUsername == "Everyone") {
                    //-- Send to all clients
                    for (QWebSocket *client : std::as_const(m_clients)) {
                        client->sendTextMessage(message);
                    }
                    // Also print to console
                    qInfo() << "Received broadcast message from client: " << message << Qt::endl;
                    emit newMessage(message);
                    return;

                } else {
                    //-- Send to specific client (DM)
                    if (m_clients.contains(dstUsername)) {
                        //-- Client found online
                        QWebSocket* destinationClient = m_clients[dstUsername];
                        destinationClient->sendTextMessage(message);
                        //-- Echo
                        if (srcUsername != dstUsername) {
                            senderSocket->sendTextMessage(message);
                        }

                        qInfo() << "Received DM message from client: " << message << Qt::endl;
                        emit newMessage("");
                        return;

                    } else {
                        //-- DM destination user is offline
                        qWarning() << "Server: Error sending to offline user: " << dstUsername << Qt::endl;
                        emit newMessage("");
                        return;
                    }
                }

            } else {
                qWarning() << "Server: Invalid JSON received" << Qt::endl;
                return;
            }

        }

    } else {
        qWarning() << "Server: Invalid JSON received" << Qt::endl;
        return;
    }

}

void WebSocketServer::serverBroadcast(const QString& message) {
    //-- Build message
    QJsonObject JsonObj;
    JsonObj["type"] = "msg";
    JsonObj["src"] = "Server";
    JsonObj["dst"] = "Everyone";
    JsonObj["data"] = message;
    QJsonDocument JsonDoc(JsonObj);
    QString JsonPacket = JsonDoc.toJson(QJsonDocument::Compact);
    //-- Send to all clients
    for (QWebSocket *client : std::as_const(m_clients)) {
        client->sendTextMessage(JsonPacket);
    }
    emit newMessage("");
    qInfo() << "Sent broadcast to Everyone: " << message << Qt::endl;
}

void WebSocketServer::socketDisconnected() {
    QWebSocket *clientSocket = qobject_cast<QWebSocket*>(sender());
    if (clientSocket) {
        //-- Reverse lookup who disconnected
        QString byeClientName;
        for (auto it = m_clients.constBegin(); it != m_clients.constEnd(); ++it) {
            if (it.value() == clientSocket) {
                byeClientName = it.key();
                break;
            }
        }
        //--Inform all clients
        serverBroadcast(byeClientName+" has left the chat.");

        //-- Remove client
        m_clients.remove(byeClientName);
        clientSocket->deleteLater();
        qInfo() << "Client disconnected!" << Qt::endl;

        //-- update all clients client list
        serverBroadcastClientList();
        emit newMessage("");
    }
}

void WebSocketServer::serverBroadcastClientList() {
    QString JsonPacket = getClientListPacket();
    for (QWebSocket *client : std::as_const(m_clients)) {
        client->sendTextMessage(JsonPacket);
    }
    qInfo() << "Sent client list to Everyone" << Qt::endl;
}

QStringList WebSocketServer::getClientList() const {
    return m_clients.keys();
}

QString WebSocketServer::getClientListPacket() const {
    // -- Prepare JSON doc
    QJsonArray JsonClients;
    for (auto it = m_clients.constBegin(); it != m_clients.constEnd(); ++it) {
        JsonClients.append(it.key());
    }
    QJsonObject OutJsonObj;
    OutJsonObj["type"] = "listing";
    OutJsonObj["data"] = JsonClients;
    // -- Convert to string
    QJsonDocument OutJsonDoc(OutJsonObj);
    return OutJsonDoc.toJson(QJsonDocument::Compact);
}
