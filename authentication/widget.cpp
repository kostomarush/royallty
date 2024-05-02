#include "widget.h"
#include "ui_widget.h"
#include "mainwindow.h"
#include <QMessageBox>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->lineEdit->setText("admin");
    ui->lineEdit_2->setText("admin");
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    QString login = ui->lineEdit->text();
    QString password = ui->lineEdit_2->text();


    if (login == "admin" && password == "admin"){
        MainWindow *window = new MainWindow;
        window->show();
        this->close();
    }
    else{
        QMessageBox::critical(this, "Ошибка", "Неверные данные");
    }
}
