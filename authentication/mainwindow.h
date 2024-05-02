#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_Connect_Button_clicked();

    void on_Disconnect_Button_clicked();

    void onServerConnected();

    void onServerDisconnected();

    void onServerReadData();

    void on_FindFileButton_clicked();

    void on_StartMonitoringButton_clicked();

private:
    Ui::MainWindow *ui;

    void sendCommand(QJsonDocument& json_cmd);
    void dispatchCommand(QJsonObject& json_cmd);
    void view_data(QJsonObject& json_cmd);
    void answer_getMonitoring(QJsonObject& json_cmd);


    QTcpSocket* m_serverConnection = nullptr;
    QTextStream m_tcpStream;
};

#endif // MAINWINDOW_H
