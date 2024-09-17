#ifndef WEBSOCKETTHREAD_H
#define WEBSOCKETTHREAD_H

#include <QThread>
#include "websocketclient.h"

class WebSocketThread : public QThread
{
    Q_OBJECT

public:
    explicit WebSocketThread(QObject* parent = nullptr);
    void run() override;

    WebSocketClient* webSocketClient;

signals:
    void startConnection(const QUrl& url, QString usernameW);
    void messageReceived(const QString& message, const QString& src, const QString& dst);
    void updateClientListTxmt(const QStringList& clients);
    void connectSuccess();
    void connectError();
    void disconnected();

private:


private slots:
    void forwardMessage(const QString& message, const QString& src, const QString& dst);
    void updateClientListRecv(const QStringList& clients);
};

#endif // WEBSOCKETTHREAD_H
