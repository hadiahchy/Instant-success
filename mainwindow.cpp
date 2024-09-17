#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), webSocketThread(new WebSocketThread(this)),
    m_server(nullptr)
{

    // Create widgets
    ipLineEdit = new QLineEdit(this);
    ipLineEdit->setText("localhost:8080");
    connectButton = new QPushButton("Connect", this);
    textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    // textEdit->append(QString("Welcome to Instant Success v0.1b"));
    clientListWidget = new QListWidget(this);
    clientListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    msgLineEdit = new QLineEdit(this);
    msgLineEdit->setEnabled(false);
    sendButton = new QPushButton("Send Message", this);
    sendButton->setEnabled(false);
    serverButton = new QPushButton("Start Server", this);

    // Layout for IP input and connect button
    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->addWidget(ipLineEdit);
    topLayout->addWidget(connectButton);
    topLayout->addWidget(serverButton);

    // Layout for Messages and client list
    QHBoxLayout* middleLayout = new QHBoxLayout;
    clientListWidget->setMaximumWidth(200);
    middleLayout->addWidget(clientListWidget);
    middleLayout->addWidget(textEdit);

    // Layout for message line and send button
    QHBoxLayout* bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(msgLineEdit);
    bottomLayout->addWidget(sendButton);

    // Main Layout
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(middleLayout);
    mainLayout->addLayout(bottomLayout);

    // Central Widget
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // Connect signals and slots
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::connectToWebSocket);
    connect(webSocketThread, &WebSocketThread::connectSuccess, this, &MainWindow::connectSuc);
    connect(webSocketThread, &WebSocketThread::connectError, this, &MainWindow::connectErr);
    connect(webSocketThread, &WebSocketThread::disconnected, this, &MainWindow::disconnected);
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(msgLineEdit, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);
    connect(msgLineEdit, &QLineEdit::textChanged, this, &MainWindow::userTyping);
    connect(webSocketThread, &WebSocketThread::messageReceived, this, &MainWindow::displayMessage);
    connect(webSocketThread, &WebSocketThread::updateClientListTxmt, this, &MainWindow::updateClientListRecv);
    connect(serverButton, &QPushButton::clicked, this, &MainWindow::startServer);

    connect(clientListWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::clientSelectionChanged);

    // Welcome Message
    clientListWidget->addItem(QString("Instant Success Logs"));
    clientListWidget->setCurrentRow(0);
    insertToClientSelectionCache("Instant Success Logs", "Welcome to Instant Success v0.1b\n");

    // Start the Websocket thread
    webSocketThread->start();
}

MainWindow::~MainWindow() {
    qInfo() << "Shutting down" << Qt::endl;

    webSocketThread->quit();
    webSocketThread->wait();    // Cleanly exit

    // if (m_serverThread->isRunning()) {
        // m_serverThread->quit();
        // m_serverThread->wait();
        delete m_server;
        m_server = nullptr;
    // }
}

void MainWindow::connectToWebSocket() {
    QString ip = ipLineEdit->text();
    if (!ip.isEmpty()) {
        if (username.isEmpty()) {
            username = QInputDialog::getText(this, "Enter Username", "Username:");
        }
        if (!username.isEmpty() && username != QString("Everyone") && username != QString("Server") && username != QString("Instant Success Logs")) {
            QUrl url(QString("ws://%1").arg(ip));
            qInfo() << "Connecting to " << url << Qt::endl;
            emit webSocketThread->startConnection(url, username);
            this->setWindowTitle("IS2 - "+username);
        } else {
            username = "";
            QMessageBox::warning(this, "Username Error", "Invalid username entered.");
        }
    } else {
        QMessageBox::warning(this, "Connection Error", "Invalid server IP entered.");
    }
}

void MainWindow::connectSuc() {
    serverButton->setEnabled(false);
    serverButton->hide();
    ipLineEdit->setEnabled(false);
    connectButton->setEnabled(false);
    connectButton->hide();

    clientListWidget->setEnabled(true);
    clientListWidget->addItem(QString("Everyone"));
    oldSelection = "Everyone";
    // clientListWidget->setCurrentItem(clientListWidget->itemAt(0,0));
    clientListWidget->setCurrentRow(1);

    // sendButton->setEnabled(true);
    msgLineEdit->setEnabled(true);

    updateClientList();

    QString connecMsg = "Connected as "+username;
    // textEdit->append(connecMsg);
    insertToClientSelectionCache("Instant Success Logs", connecMsg);
}

void MainWindow::disconnected() {
    this->setWindowTitle("IS2 - "+username);

    serverButton->setEnabled(true);
    serverButton->show();
    ipLineEdit->setEnabled(true);
    connectButton->setEnabled(true);
    connectButton->show();

    clientListWidget->setEnabled(false);

    sendButton->setEnabled(false);
    msgLineEdit->setEnabled(false);

    // textEdit->append("Server connection lost");
    insertToClientSelectionCache("Instant Success Logs", "Server connection lost");
    QMessageBox::critical(this, "Network Error", "Server connection lost");
}

void MainWindow::connectErr() {
    this->setWindowTitle("IS2 - "+username);

    qWarning() << "Failed to connect to server" << Qt::endl;
    // textEdit->append("Failed to connect to server");
    insertToClientSelectionCache("Instant Success Logs", "Failed to connect to server");
}

void MainWindow::sendMessage() {
    QString message = msgLineEdit->text();
    msgLineEdit->clear();
    sendButton->setEnabled(false);
    QString dst = clientListWidget->currentItem()->text();
    webSocketThread->webSocketClient->sendMessage(message, username, dst);
}

void MainWindow::userTyping() {
    if (!msgLineEdit->text().isEmpty()) {
        sendButton->setEnabled(true);
    }
}

void MainWindow::clientSelectionChanged() {
    if (!clientListWidget->selectedItems().isEmpty()) {
        //-- get new selection
        QString newSelection = clientListWidget->selectedItems().first()->text();

        if (newSelection != oldSelection) {
            qDebug() << "Client selection changed to " << newSelection << Qt::endl;
            //-- switch selection
            updateClientSelection();
            //-- set old selection
            oldSelection = newSelection;
        }
    } else {
        qDebug() << "Empty client selection, ignoring update" << Qt::endl;
    }
}

void MainWindow::updateClientSelection() {
    // Get current selection
    if (!clientListWidget->selectedItems().isEmpty()) {
        QString currentSelection = clientListWidget->selectedItems().first()->text();
        if (!currentSelection.isEmpty()) {
            this->setWindowTitle("IS2"+(username.isEmpty() ? "" : " - "+username+" - "+currentSelection));
            // Add greetings to new chats
            if (!clientMessageCache.contains(currentSelection)) {
                addGreeting(currentSelection);
            }
            // Read from map
            QString update = clientMessageCache[currentSelection];
            // Update textEdit
            textEdit->clear();
            textEdit->append(update);
            // textEdit->insertHtml(update);
            // Scroll to bottom
            textEdit->moveCursor(QTextCursor::End);
            textEdit->ensureCursorVisible();
        }
    } else {
        qDebug() << "Empty client selection, ignoring update" << Qt::endl;
    }
}

void MainWindow::insertToClientSelectionCache(const QString& client, const QString& update) {
    // Insert into map
    clientMessageCache[client].append(update+"\n");

    // Update
    updateClientSelection();
}

void MainWindow::displayMessage(const QString& parsedMessage, const QString& src, const QString& dst) {
    if (!parsedMessage.isEmpty()) {
        qDebug() << "Displaying message: " << parsedMessage << Qt::endl;
        if (dst != "Everyone") {
            //-- if not for everyone, route to src conversation (DM)
            // textEdit->append(parsedMessage);

            //-- if the first message, add greeting
            if (!clientMessageCache.contains(src)) {
                addGreeting(src);
            }

            if (src != username) {
                //-- this is an incoming message, insert to src
                insertToClientSelectionCache(src, parsedMessage);
            } else {
                //-- this is an outgoing message echo, insert to dst
                //-- "message is from me"
                insertToClientSelectionCache(dst, parsedMessage);
            }
        } else {
            //-- otherwise, route to Everyone channel
            // textEdit->append(parsedMessage);
            insertToClientSelectionCache("Everyone", parsedMessage);
        }
    }
}

void MainWindow::startServer() {
    qInfo() << "Starting server" << Qt::endl;

    if (!m_server) {
        quint16 port = 8080;

        // Create the WebSocket server
        m_server = new WebSocketServer(port);

        // Update client list periodically
        connect(m_server, &WebSocketServer::newMessage, this, &MainWindow::updateClientList);

        // Update the start button
        serverButton->setText("Server Started");
        serverButton->setEnabled(false);

        QString host = "Server started on ";
        host.append(getHostIPAddress());
        host.append(":8080");
        QMessageBox::information(this, "Chat Server", host);
        insertToClientSelectionCache("Instant Success Logs", host);
    }
}

QString MainWindow::getHostIPAddress() {
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).protocol() == QAbstractSocket::IPv4Protocol) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    if (ipAddress.isEmpty()) {
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    }
    return ipAddress;
}

void MainWindow::updateClientList() {
    QStringList clientList;

    if (m_server){
        //-- If we are running the server, access the clients directly
        clientList = m_server->getClientList();

    }
    // Dispatch
    if (!clientList.isEmpty()) {
        updateClientListRecv(clientList);
    }
}

void MainWindow::updateClientListRecv(const QStringList& clients) {
    qInfo() << "Updating Client List" << Qt::endl;
    QStringList clientList = clients;
    // Remove self from list
    clientList.removeOne(username);
    //-- Save current selection
    QListWidgetItem* selectedItem = clientListWidget->currentItem();
    QString selectedText = selectedItem ? selectedItem->text() : QString();
    //-- Clear and update
    clientListWidget->clear();
    clientListWidget->addItem(QString("Instant Success Logs"));
    clientListWidget->addItem(QString("Everyone"));
    clientListWidget->addItems(clientList);
    //-- Restore selection
    if (!selectedText.isEmpty()) {
        QList<QListWidgetItem*> matchingItems = clientListWidget->findItems(selectedText, Qt::MatchExactly);
        if (!matchingItems.isEmpty()) {
            clientListWidget->setCurrentItem(matchingItems.first());
            return;
        }
    }
    //-- If no match, default to first item
    clientListWidget->setCurrentRow(0);
}

void MainWindow::addGreeting(const QString& user) {
    //--> Add greeting to new chats
    if (user != "Instant Success Logs") {
        if (user != "Everyone") {
            //--> DM
            clientMessageCache[user].append(
                "This is a private chat with "+user+". Say hi!\n\n"
                );
        } else {
            //--> Public Chatroom
            clientMessageCache[user].append(
                "Welcome to the public chatroom.\n\n"
                );
        }
    }
}
