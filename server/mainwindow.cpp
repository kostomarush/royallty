#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&thread_monitoring, &QThread::started, &monitoringObject, &MonitoringObject::run);
    connect(&monitoringObject, &MonitoringObject::finished, &thread_monitoring, &QThread::terminate);
    connect(&monitoringObject, &MonitoringObject::sendData, this, &MainWindow::sendDataSize);
    monitoringObject.moveToThread(&thread_monitoring);


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
    tcp_stream.setDevice(clientConnection);
    QString message = tcp_stream.readAll();
    ui->listWidget->addItem(message);
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("command")) {
            dispatchCommand(obj);
        }
        else
            ui->listWidget->addItem("Ошибка JSON");
    }
    else
        ui->listWidget->addItem("Ошибка JSON");
}

void MainWindow::dispatchCommand(QJsonObject &json_cmd)
{
    QJsonValue val = json_cmd["command"];
    if (val.isString()) {
        QString cmd = val.toString();
        ui->listWidget->addItem("command: " + cmd);
        if (cmd == "getFindFiles") {

            command_getFiles(json_cmd);

        }
        else if (cmd == "StartMonitoring") {
            command_StartMonitoringFiles(json_cmd);
        }
        else if (cmd == "StopMonitoring") {
            command_StopMonitoringFiles(json_cmd);
        }
        else
            ui->listWidget->addItem("Неизвестная команда");
    }
    else
        ui->listWidget->addItem("Ошибка JSON");
}

void MainWindow::command_getFiles(QJsonObject &json_cmd)
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
            sendCommand(doc);
        }
        else
            ui->listWidget->addItem("Ошибка JSON");
    }
    else
        ui->listWidget->addItem("Ошибка JSON");
}

void MainWindow::command_StartMonitoringFiles(QJsonObject &json_cmd)
{
    if (json_cmd.contains("data")) {
        QJsonValue val = json_cmd["data"];
                QJsonValue path_info = val["path"];
                QJsonValue size_info = val["size"];
                if (path_info.isString()){

                    monitoringObject.setFilePath(path_info.toString());
                    monitoringObject.setcurrentSize(size_info.toInt());
                    //monitoringObject.setclientStream(client_stream.to);
                    monitoringObject.setRunning(true);
                    thread_monitoring.start();
            }
            else
                ui->listWidget->addItem("Ошибка JSON");
    }
            else
                ui->listWidget->addItem("Ошибка JSON");

}

void MainWindow::command_StopMonitoringFiles(QJsonObject &json_cmd)
{
    monitoringObject.setRunning(false);
}

QString MainWindow::JsonToString (QJsonObject *a)
{
    return QString (QJsonDocument (*a).toJson(QJsonDocument::Compact));
}

QJsonObject MainWindow::StringToJson (QString a)
{
    return QJsonDocument::fromJson (a.toUtf8()).object();
}

void MainWindow::sendCommand(QJsonDocument &json_cmd)
{
    QString msg(json_cmd.toJson());
    ui->listWidget->addItem(msg);
    tcp_stream << msg << flush;
}

void MainWindow::sendDataSize(QString path, qint64 size, QString fileName, QString dataCreate, QString lastModified)
{
    QJsonDocument doc;
    QJsonObject answer;
    QJsonObject data;
    answer["answer"] = "getInfoFile";
    data["path"] = path;
    data["size"] = size;
    data["fileName"] = fileName;
    data["dataCreate"] = dataCreate;
    data["lastModified"] = lastModified;
    answer["data"] = data;
    doc.setObject(answer);
    sendCommand(doc);
}

void MainWindow::FileMonitor(){
    //sendCommand(doc, client_stream);


}








