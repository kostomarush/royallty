#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_tcpServer = new QTcpServer(this);
    if (m_tcpServer->listen(QHostAddress::AnyIPv4, 5555))
        ui->statusBar->showMessage("Сервер запущен");

    else
        ui->statusBar->showMessage("Ошибка запуска сервера");

    connect(m_tcpServer, &QTcpServer::newConnection, this, &MainWindow::onClientConnected);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onClientConnected()
{
    QTcpSocket* clientConnection = m_tcpServer->nextPendingConnection();


    connect(clientConnection, &QTcpSocket::disconnected, this, &MainWindow::onClientDisconnected);
    connect(clientConnection, &QTcpSocket::readyRead, this, &MainWindow::onClientReadData);

//    QTextStream tcp_stream(clientConnection);
//    tcp_stream << "Hello" << flush;
    ui->listWidget->addItem("Подключился " + clientConnection->peerAddress().toString());
}

void MainWindow::onClientDisconnected()
{
    QTcpSocket* clientConnection = qobject_cast<QTcpSocket*>(sender());
    if (clientConnection) {
        ui->listWidget->addItem("Отключился " + clientConnection->peerAddress().toString());
        clientConnection->deleteLater();
    }
}

void MainWindow::onClientReadData()
{
    QTcpSocket* clientConnection = qobject_cast<QTcpSocket*>(sender());
    QTextStream tcp_stream(clientConnection);

    QString message = tcp_stream.readAll();
    ui->listWidget->addItem(message);
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("command")) {
            dispatchCommand(obj, tcp_stream);
        }
        else
            ui->listWidget->addItem("Ошибка JSON");
    }
    else
        ui->listWidget->addItem("Ошибка JSON");
}

void MainWindow::dispatchCommand(QJsonObject &json_cmd,
                                 QTextStream &client_stream)
{
    QJsonValue val = json_cmd["command"];
    if (val.isString()) {
        QString cmd = val.toString();
        ui->listWidget->addItem("command: " + cmd);
        if (cmd == "getFindFiles") {

            command_getFiles(json_cmd, client_stream);
        }
        else if (cmd == "StartMonitoring") {
            command_MonitoringFiles(json_cmd, client_stream);
        }
        else if (cmd == "other_command_2") {

        }
        else
            ui->listWidget->addItem("Неизвестная команда");
    }
    else
        ui->listWidget->addItem("Ошибка JSON");
}

void MainWindow::command_getFiles(QJsonObject &json_cmd,
                                 QTextStream &client_stream)
{
    if (json_cmd.contains("data")) {
        QJsonValue val = json_cmd["data"];
        if (val.isString()){
            QJsonDocument doc;
            QString dirs = val.toString();
            QJsonObject answer;
            QJsonArray filesArray;
            answer["answer"] = "getFiles";
            QDirIterator dirit(dirs, QDir::Files | QDir::NoDotAndDotDot);
            while (dirit.hasNext()) {
                dirit.next();
                filesArray.push_back(dirit.filePath());
                ui->listWidget->addItem(dirit.filePath());
            }
            answer.insert("files", filesArray);
            doc.setObject(answer);
            sendCommand(doc, client_stream);
        }
        else
            ui->listWidget->addItem("Ошибка JSON");
    }
    else
        ui->listWidget->addItem("Ошибка JSON");
}

void MainWindow::command_MonitoringFiles(QJsonObject &json_cmd,
                                       QTextStream &client_stream)
{
    if (json_cmd.contains("data")) {
        QJsonValue val = json_cmd["data"];
                QJsonValue path_info = val["path"];
                QJsonValue size_info = val["size"];
                QJsonDocument doc;
                if (path_info.isString()){
                    QString path = path_info.toString();
                    qint64 size = size_info.toInt();
                    thread = new QThread(this);
                    connect(thread, SIGNAL(started()), this, MainWindow::FileMonitor(path,size));
                    thread->start();
                    QJsonObject answer;
                    answer["answer"] = "getInfoFile";
                    answer["data"] = "info";
                    doc.setObject(answer);
                    sendCommand(doc, client_stream);
            }
            else
                ui->listWidget->addItem("Ошибка JSON");
    }
            else
                ui->listWidget->addItem("Ошибка JSON");

}

void MainWindow::sendCommand(QJsonDocument &json_cmd,
                             QTextStream &client_stream)
{
    QString msg(json_cmd.toJson());
    ui->listWidget->addItem(msg);
    client_stream << msg << flush;
}

void MainWindow::FileMonitor(QString &filePath, qint64 &size){
    // Получаем начальный размер файла

    qDebug() << filePath;
    qDebug() << QFileInfo(filePath).size();
    qDebug() << QFileInfo(filePath).fileName();
    qDebug() << QFileInfo(filePath).created().toString();
    qDebug() << QFileInfo(filePath).lastModified().toString();



}





