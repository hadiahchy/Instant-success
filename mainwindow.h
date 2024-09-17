#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>
#include <QHBoxLayout>

#include <QInputDialog>
#include <QMessageBox>

#include <QHostInfo>
#include <QNetworkInterface>

#include <QMap>

#include "websocketthread.h"
#include "websocketserver.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString username;

private slots:
    void connectToWebSocket();
    void connectSuc();
    void connectErr();
    void disconnected();
    void sendMessage();
    void displayMessage(const QString& parsedMessage, const QString& src, const QString& dst);
    void startServer();
    void updateClientList();
    void updateClientListRecv(const QStringList& clients);
    void userTyping();

    void clientSelectionChanged();

private:
    QLineEdit* ipLineEdit;
    QLineEdit* msgLineEdit;
    QTextEdit* textEdit;
    QListWidget* clientListWidget;
    QPushButton* serverButton;
    QPushButton* connectButton;
    QPushButton* sendButton;

    WebSocketThread* webSocketThread;
    WebSocketServer *m_server;

    QString getHostIPAddress();

    QMap<QString, QString> clientMessageCache;
    QString oldSelection;
    void updateClientSelection();
    void insertToClientSelectionCache(const QString& client, const QString& update);

    void addGreeting(const QString& user);
};

#endif // MAINWINDOW_H
