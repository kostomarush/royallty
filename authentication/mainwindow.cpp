#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->Ip_Addr->setText("127.0.0.1");
    ui->PortNumber->setValue(5555);
    ui->FindDirectoryEdit->setText("C:/Users/vladk/OneDrive");

    ui->statusbar->showMessage("Клиент Отключен");
    ui->FindFileButton->setEnabled(false);
    ui->pushButton->setEnabled(false);
    ui->StartMonitoringButton->setEnabled(false);
    ui->Disconnect_Button->setEnabled(false);
    ui->FindDirectoryEdit->setEnabled(false);
    ui->lineEdit_3->setEnabled(false);
    ui->lineEdit->setEnabled(false);

    m_serverConnection = new QTcpSocket(this);
    m_tcpStream.setDevice(m_serverConnection);
    connect(m_serverConnection, &QTcpSocket::connected, this, &MainWindow::onServerConnected);
    connect(m_serverConnection, &QTcpSocket::disconnected, this, &MainWindow::onServerDisconnected);
    connect(m_serverConnection, &QTcpSocket::readyRead, this, &MainWindow::onServerReadData);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_Connect_Button_clicked()
{
    m_serverConnection->abort();
    m_serverConnection->connectToHost(ui->Ip_Addr->text(), ui->PortNumber->value());

}

void MainWindow::on_Disconnect_Button_clicked()
{
    m_serverConnection->close();
}

void MainWindow::onServerConnected()
{
    ui->statusbar->showMessage("Клиент Подключен");
    ui->Connect_Button->setEnabled(false);
    ui->Disconnect_Button->setEnabled(true);
    ui->FindFileButton->setEnabled(true);
    ui->pushButton->setEnabled(true);
    ui->StartMonitoringButton->setEnabled(true);
    ui->Disconnect_Button->setEnabled(true);
    ui->FindDirectoryEdit->setEnabled(true);
    ui->lineEdit_3->setEnabled(true);
    ui->lineEdit->setEnabled(true);
}

void MainWindow::onServerDisconnected()
{
    ui->statusbar->showMessage("Клиент Отключен");
    ui->Connect_Button->setEnabled(true);
    ui->FindFileButton->setEnabled(false);
    ui->pushButton->setEnabled(false);
    ui->StartMonitoringButton->setEnabled(false);
    ui->Disconnect_Button->setEnabled(false);
    ui->FindDirectoryEdit->setEnabled(false);
    ui->lineEdit_3->setEnabled(false);
    ui->lineEdit->setEnabled(false);
}

void MainWindow::onServerReadData()
{
    QString message = m_tcpStream.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("answer")) {
            dispatchCommand(obj);
        }
        else
            ui->FilesList->addItem("Ошибка JSON");
    }
    else
        ui->FilesList->addItem("Ошибка JSON");
}


void MainWindow::on_FindFileButton_clicked()
{
    QJsonDocument doc;
    QJsonObject command;
    command["command"] = "getFindFiles";
    command["data"] = ui->FindDirectoryEdit->text();
    doc.setObject(command);
    sendCommand(doc);
}


void MainWindow::dispatchCommand(QJsonObject &json_cmd)
{
    QJsonValue val = json_cmd["answer"];
    if (val.isString()) {
        QString cmd = val.toString();
        if (cmd == "getFiles") {

            view_data(json_cmd);
        }
        else if (cmd == "getInfoFile") {
           answer_getMonitoring(json_cmd);
       }
        else if (cmd == "other_command_2") {

        }
        else
            ui->FilesList->addItem("Неизвестная команда");
    }
    else
        ui->FilesList->addItem("Ошибка JSON");
}

void MainWindow::view_data(QJsonObject &json_cmd)
{
    if (json_cmd.contains("files")) {
        QJsonValue val = json_cmd["files"];
        if (val.isArray()) {
            ui->FilesList->clear();
            QJsonArray ja = val.toArray();
            for(int i = 0; i < ja.count(); i++){
            ui->FilesList->addItem(ja.at(i).toString());
            }

        }
        else
            ui->FilesList->addItem("Ошибка JSON");
    }
    else
        ui->FilesList->addItem("Ошибка JSON");
}

void MainWindow::answer_getMonitoring(QJsonObject &json_cmd){

    if (json_cmd.contains("data")) {
        QJsonValue val = json_cmd["data"];
        if (val.isString()){
            QString name = val.toString();
            ui->tableWidget->setRowCount(0);
            ui->tableWidget->setColumnCount(0);
        }
        else
            ui->FilesList->addItem("Ошибка JSON");
    }
    else
        ui->FilesList->addItem("Ошибка JSON");

}


void MainWindow::sendCommand(QJsonDocument &json_cmd)
{
    QString msg(json_cmd.toJson());
    if (m_serverConnection->isWritable())
        m_tcpStream << msg << flush;
}

void MainWindow::on_StartMonitoringButton_clicked()
{
    QListWidgetItem *item = ui->FilesList->currentItem();
    if (item) {
        QString selectedItemText = item->text();
        QJsonDocument doc;
        QJsonObject command;
        QJsonObject data;
        command["command"] = "StartMonitoring";
        data["path"] = selectedItemText;
        data["size"] = ui->lineEdit_3->text().toInt();
        command["data"] = data;
        doc.setObject(command);
        sendCommand(doc);
    }
    else {
        ui->statusbar->showMessage("Нет выделенной строки");
    }
}
