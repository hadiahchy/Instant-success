#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>

class WebSocketClient : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketClient(QObject* parent = nullptr);

    void connectToServer(const QUrl& url, QString usernameW);
    void sendMessage(const QString& message, const QString& src, const QString& dst);

signals:
    void messageReceived(const QString& parsedMessage, const QString& src, const QString& dst);
    void sendMessageSig(const QString& packet);
    void connectSuccess();
    void connectError();
    void disconnected();
    void updateClientListTxmt(const QStringList& clients);

private slots:
    void onMessageReceived(const QString& message);
    void connectedWelcome();

private:
    QWebSocket webSocket;

    QString username;

    QString parseJsonMessage(const QJsonObject& jsonObj);
};

#endif // WEBSOCKETCLIENT_H
