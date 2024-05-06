#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDataStream>
#include <QByteArray>
#include <QtNetwork>
#include <QTextStream>
#include <QMessageBox>
#include <QMultiMap>
#include <QStringList>
#include <QJsonObject>
#include <QFile>
#include <QFileDialog>
#include <QtAlgorithms>
#include <QAxObject>
#include <QAxWidget>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QColor>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    QString JsonToString (QJsonObject *a);

    QJsonObject StringToJson (QString a);

    void onServerResponse();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket* serverConnection = nullptr;
    QTextStream tcpStream;
    QMultiMap <QString, QString> agents_memory;
    QStringList agents;
    QFile file;
    QSqlDatabase db;
    QString standard_computer_name = "k33-378-23";
    QString standard_number_of_processors = "4";
    QString standard_oem_id = "0";
    QString standard_os_name = "Windows 10";
    QString standard_page_size = "4096";
    QString standard_processor_architecture = "0";
    QString standard_processor_type = "586";
    QString standard_user_name = "User";
    QStringList standardValues = {standard_computer_name, standard_number_of_processors, standard_oem_id, standard_os_name, standard_page_size, standard_processor_architecture, standard_processor_type, standard_user_name};
};
#endif // MAINWINDOW_H
