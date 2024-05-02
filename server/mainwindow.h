#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onClientReadData();

private:
    void dispatchCommand(QJsonObject& json_cmd,
                         QTextStream& client_stream);

    void command_getFiles(QJsonObject& json_cmd, QTextStream& client_stream);

    void command_MonitoringFiles(QJsonObject& json_cmd,
                               QTextStream& client_stream);

    void sendCommand(QJsonDocument& json_cmd,
                     QTextStream& client_stream);

    void FileMonitor(QString &filePath);

    Ui::MainWindow *ui;
    QTcpServer* m_tcpServer = nullptr;
};

#endif // MAINWINDOW_H











