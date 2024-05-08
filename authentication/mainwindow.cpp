#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->Ip_Addr->setText("127.0.0.1");
    ui->PortNumber->setValue(5555);
    ui->FindDirectoryEdit->setText("C:/Users/Admin/Desktop");

    ui->statusbar->showMessage("Клиент Отключен");
    ui->FindFileButton->setEnabled(false);
    ui->pushButton->setEnabled(false);
    ui->StartMonitoringButton->setEnabled(false);
    ui->Disconnect_Button->setEnabled(false);
    ui->FindDirectoryEdit->setEnabled(false);
    ui->lineEdit_3->setEnabled(false);
    ui->lineEdit->setEnabled(false);
    ui->pushButton_2->setEnabled(false);

    m_serverConnection = new QTcpSocket(this);
    m_tcpStream.setDevice(m_serverConnection);
    connect(m_serverConnection, &QTcpSocket::connected, this, &MainWindow::onServerConnected);
    connect(m_serverConnection, &QTcpSocket::disconnected, this, &MainWindow::onServerDisconnected);
    connect(m_serverConnection, &QTcpSocket::readyRead, this, &MainWindow::onServerReadData);
    ui->tableWidget->setColumnCount(6);
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->hideColumn(0);
    ui->tableWidget->setHorizontalHeaderLabels({"", "Путь к файлу", "Имя файла", "Размер", "Дата создания", "Дата последнего изменения"});
    ui->tableWidget->setColumnWidth(4,150);
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
    ui->FindDirectoryEdit->setEnabled(true);
    ui->lineEdit_3->setEnabled(true);

}

void MainWindow::onServerDisconnected()
{
    ui->statusbar->showMessage("Клиент Отключен");
    ui->Connect_Button->setEnabled(true);
    ui->FindFileButton->setEnabled(false);
    ui->StartMonitoringButton->setEnabled(false);
    ui->Disconnect_Button->setEnabled(false);
    ui->FindDirectoryEdit->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
    ui->lineEdit_3->setEnabled(false);

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

            QJsonValue path_info = val["path"];
            QJsonValue size_info = val["size"];
            QJsonValue fileName = val["fileName"];
            QJsonValue dataCreate = val["dataCreate"];
            QJsonValue lastModified = val["lastModified"];

            QSqlQuery query;

            query.prepare("INSERT INTO monitoring (path, fileName, size, dataCreate, lastModified) "
                          "VALUES (:path, :fileName, :size, :dataCreate, :lastModified)");
            query.bindValue(":path",            path_info.toString());
            query.bindValue(":fileName",        fileName.toString());
            query.bindValue(":size",             size_info.toInt());
            query.bindValue(":dataCreate",     dataCreate.toString());
            query.bindValue(":lastModified", lastModified.toString());

            if (!query.exec()) {
                qDebug() << "Ошибка добавления данных:" << query.lastError().text();
            } else {
                qDebug() << "Данные успешно добавлены!";
            }
        }

        else {
            ui->FilesList->addItem("Ошибка JSON");
        }
    QSqlQuery query("SELECT id, path, fileName, size, dataCreate, lastModified FROM monitoring;");

    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);

    ui->tableWidget->setHorizontalHeaderLabels({"", "Путь к файлу", "Имя файла", "Размер", "Дата создания", "Дата последнего изменения"});


    for (int i = 0; query.next(); i++){
        ui->tableWidget->insertRow(i);
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(query.value(2).toString()));
        ui->tableWidget->setItem(i, 3, new QTableWidgetItem(query.value(3).toString()));
        ui->tableWidget->setItem(i, 4, new QTableWidgetItem(query.value(4).toString()));
        ui->tableWidget->setItem(i, 5, new QTableWidgetItem(query.value(5).toString()));
    }
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
            for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
                QTableWidgetItem *item = ui->tableWidget->item(row, col);
                if (item)
                    item->setTextAlignment(Qt::AlignCenter);
            }
        }
    if (ui->tableWidget->rowCount() == 0) {
        ui->pushButton->setEnabled(false);
        ui->lineEdit->setEnabled(false);
        ui->action_6->setEnabled(false);
    } else {
        ui->pushButton->setEnabled(true);
        ui->lineEdit->setEnabled(true);
        ui->action_6->setEnabled(true);
    }

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
    if (item && ui->lineEdit_3->text() != "") {
        ui->StartMonitoringButton->setEnabled(false);
        ui->Disconnect_Button->setEnabled(false);
        ui->FindFileButton->setEnabled(false);
        ui->FindDirectoryEdit->setEnabled(false);
        ui->pushButton_2->setEnabled(true);
        ui->pushButton->setEnabled(false);
        ui->menu_2->setEnabled(false);
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
        ui->statusbar->showMessage("Нет выделенной строки или размер порога не указан");
    }
}

void MainWindow::on_action_4_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Выбор файла SQL_Lite", "", "SQLite(*.db *.sqlite *.sqlite3);;All Files (*.*)");
        if (filePath.isEmpty()) {
            return;
        }

        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(filePath);

        if (!db.open()) {
                ui->statusbar->showMessage("Ошибка при подключении к базе данных:" + db.lastError().text());
                return;
            }

        QSqlQuery query(db);

        if (db.tables().contains("monitoring")) {
                // Очистка таблицы, если она уже существует
                if (!query.exec("DELETE FROM monitoring"))
                {
                     ui->statusbar->showMessage("Ошибка при очистке таблицы: " + query.lastError().text());
                    return;
                }
            }
            else {
                // Создание таблицы, если она не существует
                if (!query.exec("create table monitoring"
                                "(id integer primary key,"
                                "path varchar(100),"
                                "fileName varchar(60),"
                                "size integer,"
                                "dataCreate varchar(60),"
                                "lastModified varchar(60))")){

                    ui->statusbar->showMessage("Ошибка при создании таблицы: " +  query.lastError().text());
                    return;
                }
            }
            ui->pushButton->setEnabled(true);
            ui->action_6->setEnabled(false);
            ui->lineEdit->setEnabled(true);
            ui->StartMonitoringButton->setEnabled(true);
            ui->tableWidget->clear();
            ui->tableWidget->setRowCount(0);
            ui->tableWidget->setHorizontalHeaderLabels({"", "Путь к файлу", "Имя файла", "Размер", "Дата создания", "Дата последнего изменения"});
            ui->statusbar->showMessage("Таблица успешно создана или очищена!");

}



void MainWindow::on_action_5_triggered()
{
        QString filePath = QFileDialog::getOpenFileName(this, "Выберите Базу данных SQLite", "", "SQLite(*.db *.sqlite *.sqlite3);;All Files (*.*)");
        if (filePath.isEmpty()) {
            ui->statusbar->showMessage("Отсутствует файл .db");
            return;
        }
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(filePath);
        if (db.open()) {
            ui->statusbar->showMessage("База данных успешно подключена");
        } else{
            ui->statusbar->showMessage("Ошибка при загрузке Базы даных: " + db.lastError().text());
            return;
        }


        QSqlQuery query("SELECT id, path, fileName, size, dataCreate, lastModified FROM monitoring;");

        ui->pushButton->setEnabled(true);
        ui->lineEdit->setEnabled(true);
        ui->StartMonitoringButton->setEnabled(true);

        ui->tableWidget->clear();
        ui->tableWidget->setRowCount(0);

        ui->tableWidget->setHorizontalHeaderLabels({"", "Путь к файлу", "Имя файла", "Размер", "Дата создания", "Дата последнего изменения"});

        for (int i = 0; query.next(); i++){
            ui->tableWidget->insertRow(i);
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(query.value(1).toString()));
            ui->tableWidget->setItem(i, 2, new QTableWidgetItem(query.value(2).toString()));
            ui->tableWidget->setItem(i, 3, new QTableWidgetItem(query.value(3).toString()));
            ui->tableWidget->setItem(i, 4, new QTableWidgetItem(query.value(4).toString()));
            ui->tableWidget->setItem(i, 5, new QTableWidgetItem(query.value(5).toString()));
        }
        for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
                for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
                    QTableWidgetItem *item = ui->tableWidget->item(row, col);
                    if (item)
                        item->setTextAlignment(Qt::AlignCenter);
                }
            }
        if (ui->tableWidget->rowCount() == 0) {
            ui->pushButton->setEnabled(false);
            ui->lineEdit->setEnabled(false);
            ui->action_6->setEnabled(false);
        } else {
            ui->pushButton->setEnabled(true);
            ui->lineEdit->setEnabled(true);
            ui->action_6->setEnabled(true);
        }
}

void MainWindow::on_pushButton_clicked()
{
    // Извлечение данных из базы данных
        QVector<int> fileSizes;
        QSqlQuery query("SELECT size FROM monitoring"); // Замените на имя вашей таблицы и столбца
        while (query.next()) {
            int size = query.value(0).toInt();
            fileSizes.append(size);
        }

        int k = fileSizes.size();

        QVector<double> averages;
            if (fileSizes.size() < k) {
                ui->statusbar->showMessage("Данное значение велико для размера окна");
                qDebug() << "Data is smaller than window size.";
                return;
            }

            // Начальная сумма для первых k элементов
            double current_sum = 0;
            for (int i = 0; i < k; ++i) {
                current_sum += fileSizes[i];
            }

            // Добавляем среднее значение для начального окна
            averages.append(current_sum / k);

            // Сдвигаем окно и пересчитываем среднее значение
            for (int i = k; i < fileSizes.size(); ++i) {
                current_sum = current_sum - fileSizes[i - k] + fileSizes[i];
                averages.append(current_sum / k);
            }

            QString avg = QString::number(averages[0]);

            ui->lineEdit->setText(avg);

}

void MainWindow::on_action_6_triggered()
{

    if (!db.open()) {
            ui->statusbar->showMessage("База данных не подключена");
            return;
        }

     QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранение отчета"), "", tr("Excel Files (*.xlsx)"));
     fileName.replace("/","\\");

        // Получаем список таблиц из базы данных
        QStringList tables = db.tables();
        if (!tables.contains("monitoring")) {
            ui->statusbar->showMessage("Таблица не найдена");
            return;
        }

        // Выполняем запрос к базе данных
        QSqlQuery query;
        query.exec("SELECT * FROM monitoring");

        // Создаем новый экземпляр Excel
        excel = new QAxObject("Excel.Application");
        excel->setProperty("Visible", false);



        // Создаем новую книгу Excel
        QAxObject *workbook = excel->querySubObject("Workbooks")->querySubObject("Add()");
        QAxObject *worksheet = workbook->querySubObject("Worksheets(int)", 1);

        // Устанавливаем сетку и границы для ячеек



        QString avg_size = ui->lineEdit->text();

        // Указываем заголовки
        QStringList headers = {"№", "Путь к файлу", "Имя файла", "Размер", "Дата создания", "Дата последнего изменения"}; // Замените на ваши заголовки
        int columnWidths[] = {5, 50, 15,15, 30, 30}; // Замените на желаемые размеры


        for (int column = 0; column < headers.size(); ++column) {
            int width = columnWidths[column];
            QAxObject *excelColumn = worksheet->querySubObject("Columns(int)", column + 1);
            excelColumn->setProperty("ColumnWidth", width);
            delete excelColumn;
        }


        // Записываем заголовки в первую строку таблицы Excel
        for (int column = 0; column < headers.size(); ++column) {
            QString header = headers.at(column);
            QAxObject *cell = worksheet->querySubObject("Cells(int,int)", 1, column + 1); // Заголовки начинаются с первой строки
            QAxObject *font = cell->querySubObject("Font");
            QAxObject *borders = cell->querySubObject("Borders");
            borders->setProperty("LineStyle", 1); // Solid line style
            borders->setProperty("Weight", 2); // Medium weight
            font->setProperty("Bold", true);
            cell->setProperty("Value", header);
            cell->setProperty("HorizontalAlignment", -4108);
            delete cell;
        }

        // Выводим результаты запроса в Excel
        int row = 2;
        while (query.next()) {
            for (int column = 0; column < 6; ++column) {
                QString value = query.value(column).toString();
                QAxObject *cell = worksheet->querySubObject("Cells(int,int)", row, column + 1);
                QAxObject *borders = cell->querySubObject("Borders");
                borders->setProperty("LineStyle", 1); // Solid line style
                borders->setProperty("Weight", 2); // Medium weight
                cell->setProperty("Value", value);
                cell->setProperty("HorizontalAlignment", -4108);
                delete cell;
            }
            ++row;
        }


        QAxObject *avg_head = worksheet->querySubObject("Cells(int,int)", 1, 9); // Предполагается, что значение из QLineEdit добавляется в следующий столбец после данных из базы
        QAxObject *avg_cell = worksheet->querySubObject("Cells(int,int)", 1, 14); // Предполагается, что значение из QLineEdit добавляется в следующий столбец после данных из базы
        avg_cell->setProperty("Value", avg_size);
        avg_cell->setProperty("HorizontalAlignment", -4108);
        avg_head->setProperty("Value", "Среднее значение размера выбранного файла: ");
        delete avg_head;
        delete avg_cell;


        // Сохраняем книгу Excel.
        if (!fileName.isEmpty()) {
            workbook->dynamicCall("SaveAs(const QString&)", fileName);
            workbook->dynamicCall("Close()");
            excel->dynamicCall("Quit()");
        } else {
            qDebug() << "No file selected.";
        }
        // Освобождаем ресурсы
        delete worksheet;
        delete workbook;
        delete excel;
        ui->statusbar->showMessage("Отчет успешно сформирован");

      }

void MainWindow::on_pushButton_2_clicked()
{
    ui->StartMonitoringButton->setEnabled(true);
    ui->Disconnect_Button->setEnabled(true);
    ui->FindFileButton->setEnabled(true);
    ui->FindDirectoryEdit->setEnabled(true);
    ui->pushButton_2->setEnabled(false);
    ui->menu_2->setEnabled(true);
    if (ui->tableWidget->rowCount() == 0) {
        ui->pushButton->setEnabled(false);
        ui->lineEdit->setEnabled(false);
    } else {
        ui->pushButton->setEnabled(true);
        ui->lineEdit->setEnabled(true);
    }
    QJsonDocument doc;
    QJsonObject command;
    QJsonObject data;
    command["command"] = "StopMonitoring";
    doc.setObject(command);
    sendCommand(doc);
    ui->statusbar->showMessage("Мониторинг остановлен");

}

