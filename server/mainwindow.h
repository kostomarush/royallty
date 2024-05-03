#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QThread>
#include "monitoringobject.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void makeThread();

    void sendCommand(QJsonDocument& json_cmd,
                     QTextStream& client_stream);

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onClientReadData();
    void FileMonitor();

private:
    void dispatchCommand(QJsonObject& json_cmd,
                         QTextStream& client_stream);

    void command_getFiles(QJsonObject& json_cmd, QTextStream& client_stream);

    void command_StartMonitoringFiles(QJsonObject& json_cmd,
                               QTextStream& client_stream);
    void command_StopMonitoringFiles(QJsonObject& json_cmd,
                               QTextStream& client_stream);


    Ui::MainWindow *ui;
    QThread thread_monitoring;
    MonitoringObject monitoringObject;
    QTcpServer* m_tcpServer = nullptr;
    QString filePath;
    qint64 size;
    bool flag;

};

#endif // MAINWINDOW_H











