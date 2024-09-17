#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>
#include <QMap>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>

class WebSocketServer : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketServer(quint16 port, QObject *parent = nullptr);
    ~WebSocketServer();

    QStringList getClientList() const;

signals:
    void newMessage(const QString &message);

public slots:
    void onNewConenction();
    void processTextMessage(const QString &message);
    void socketDisconnected();

private:
    QWebSocketServer *m_server;
    QMap<QString, QWebSocket*> m_clients;

    void serverBroadcast(const QString& message);
    QString getClientListPacket() const;
    void serverBroadcastClientList();
};

#endif // WEBSOCKETSERVER_H
