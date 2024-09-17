#include "websocketclient.h"

WebSocketClient::WebSocketClient(QObject* parent)
    : QObject(parent) {
    connect(&webSocket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onMessageReceived);
    connect(this, &WebSocketClient::sendMessageSig, &webSocket, &QWebSocket::sendTextMessage);
    connect(&webSocket, &QWebSocket::connected, this, &WebSocketClient::connectedWelcome);
    connect(&webSocket, &QWebSocket::errorOccurred, this, &WebSocketClient::connectError);
    connect(&webSocket, &QWebSocket::disconnected, this, &WebSocketClient::disconnected);
}

void WebSocketClient::connectToServer(const QUrl& url, QString usernameW) {
    username = usernameW;
    webSocket.open(url);
    qInfo() << "Connecting to " << url << Qt::endl;
}

void WebSocketClient::connectedWelcome() {
    //-- send blank message to register username
    sendMessage("", username, "");
    //-- Update UI connected
    emit connectSuccess();
}

void WebSocketClient::sendMessage(const QString& message, const QString& src, const QString& dst) {
    if (webSocket.isValid()) {
        QJsonObject JsonObj;
        JsonObj["type"] = "msg";
        JsonObj["src"] = src;
        JsonObj["dst"] = dst;
        JsonObj["data"] = message;
        QJsonDocument JsonDoc(JsonObj);
        QString JsonPacket = JsonDoc.toJson(QJsonDocument::Compact);

        qInfo() << "Sending message: " << JsonPacket << Qt::endl;
        // webSocket.sendTextMessage(message);      //! cannot call across threads, must use slots and signals
        emit sendMessageSig(JsonPacket);

    } else {
        qInfo() << "Invalid WebSocket when sending: " << message << Qt::endl;
    }
}

void WebSocketClient::onMessageReceived(const QString& message) {
    qInfo() << "Message received, decoding JSON..." << Qt::endl;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

    if (!doc.isNull() && doc.isObject()) {
        QJsonObject jsonObj = doc.object();
        QString parsedMessage = parseJsonMessage(jsonObj);

        if (jsonObj.contains("type") && jsonObj["type"].isString()) {
            QString type = jsonObj["type"].toString();

            //! listing recv is handled before this in parseJsonMessage

            if (type == "msg" && jsonObj.contains("data")
                       && jsonObj["data"].isString() && jsonObj.contains("src")
                       && jsonObj["src"].isString() && jsonObj.contains("dst")
                       && jsonObj["dst"].isString()) {
                // recieved ordinary message
                QString src = jsonObj["src"].toString();
                QString dst = jsonObj["dst"].toString();
                emit messageReceived(parsedMessage, src, dst);

            } else {
                qWarning() << "Invalid JSON received" << Qt::endl;
                return;
            }

        } else {
            qWarning() << "Invalid JSON received" << Qt::endl;
            return;
        }
    } else {
        qWarning() << "Invalid JSON received" << Qt::endl;
        return;
    }
}

QString WebSocketClient::parseJsonMessage(const QJsonObject& jsonObj) {
    if (jsonObj.contains("type") && jsonObj["type"].isString() &&
        jsonObj.contains("data") && (jsonObj["data"].isString() || jsonObj["data"].isArray())) {

        QString type = jsonObj["type"].toString();
        QString returns;

        if (type == "listing") {
            QStringList data;
            QJsonArray dataArr = jsonObj["data"].toArray();
            for (const QJsonValue &value : dataArr) {
                if (value.isString()) {
                    data.append(value.toString());
                }
            }

            // returns = QString("Users: [%1]").arg(data.join(", "));
            emit updateClientListTxmt(data);
            returns = QString();

        } else if (type == "msg") {
            QString data = jsonObj["data"].toString();
            if (jsonObj.contains("dst") && jsonObj["dst"].isString()
                && jsonObj.contains("src") && jsonObj["src"].isString()) {

                QString src = jsonObj["src"].toString();

                returns = QString("%1:\t%2").arg(src, data);

            } else {
                return "Invalid JSON structure";
            }
        } else {
            return "Invalid JSON structure";
        }





        qInfo() << "Parsed received message: " << returns << Qt::endl;

        return returns;
    }

    return "Invalid JSON structure";
}
