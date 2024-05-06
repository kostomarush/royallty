#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    QRegExp ipRegex ("^" + ipRange + "\\." + ipRange + "\\." + ipRange + "\\." + ipRange + "$");
    QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);
    ui->lineEdit->setValidator(ipValidator);
    ui->lineEdit_2->setValidator(new QIntValidator (1000, 9999, this));
    serverConnection = new QTcpSocket(this);
    tcpStream.setDevice(serverConnection);
    connect(serverConnection, &QTcpSocket::readyRead, this, &MainWindow::onServerResponse);
    ui->tableWidget->setColumnCount(10);
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->hideColumn(0);
    QStringList lst;
    lst << trUtf8("ID") << trUtf8("IP") << trUtf8("Name") << trUtf8("Number") << trUtf8("Oem") << trUtf8("OS") << trUtf8("Page") << trUtf8("Architecture") << trUtf8("Type") << trUtf8("User");
    ui->tableWidget->setHorizontalHeaderLabels(lst);
}

MainWindow::~MainWindow()
{
    serverConnection->abort();
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    if (db.open()){
        if ((ui->lineEdit->text() != "") && (ui->lineEdit_2->text() != "")){
            serverConnection->abort();
            serverConnection->connectToHost(ui->lineEdit->text(), ui->lineEdit_2->text().toInt());
            QMessageBox::information(this, "Успешно", "Агент успешно подключен");
            agents.append(ui->lineEdit->text());
            ui->comboBox->addItems(agents);
            ui->lineEdit->clear();
            ui->lineEdit_2->clear();
        }
        else{
            QMessageBox::critical(this, "Ошибка", "Введите IP-адрес/Порт агентов");
        }
    }
    else {
        QMessageBox::critical(this, "Ошибка", "Создайте или откройте базу данных");
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    serverConnection->connectToHost(ui->comboBox->currentText(), 5555);
    QJsonObject obj;
    obj["command"] = "start";
    serverConnection->write(JsonToString(&obj).toUtf8());

}

QString MainWindow::JsonToString (QJsonObject *a)
{
    return QString (QJsonDocument (*a).toJson(QJsonDocument::Compact));
}

QJsonObject MainWindow::StringToJson (QString a)
{
    return QJsonDocument::fromJson (a.toUtf8()).object();
}

void MainWindow::onServerResponse()
{
    int count_nest = 0;
    QMap <QString, int> map;
    QByteArray data = serverConnection->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        return;
    }
    qDebug() << data;
    QJsonObject response = jsonDoc.object();
    if (response.contains("computer_name")) {
        QString computer_name = response["computer_name"].toString();
        QString number_of_processors = response["number_of_processors"].toString();
        QString oem_id = response["oem_id"].toString();
        QString os_name = response["os_name"].toString();
        QString page_size = response["page_size"].toString();
        QString processor_architecture = response["processor_architecture"].toString();
        QString processor_type = response["processor_type"].toString();
        QString user_name = response["user_name"].toString();
        ui->listWidget->addItem("Имя компьютера " + computer_name);
        ui->listWidget->addItem("Номер процессора " + number_of_processors);
        ui->listWidget->addItem("Номер изделия " + oem_id);
        ui->listWidget->addItem("Операционная система " + os_name);
        ui->listWidget->addItem("Размер страницы " + page_size);
        ui->listWidget->addItem("Архитектура процессора " + processor_architecture);
        ui->listWidget->addItem("Тип процессора " + processor_type);
        ui->listWidget->addItem("Имя пользователя " + user_name);
        QString getMaxIdQuery = "SELECT MAX(id) FROM infor;";
        QSqlQuery query;
        int lastId = 0;
        if (query.exec(getMaxIdQuery)) {
            if (query.next()) {
                lastId = query.value(0).toInt();
            }
        }
        QString infor_in = computer_name + "; " + number_of_processors + "; " + oem_id + "; " + os_name + "; " + page_size + "; " + processor_architecture + "; " + processor_type + "; " + user_name;
        QString strF = "INSERT INTO infor (id, ip, info) "
                         "VALUES(%1, '%2', '%3');";
        QString str = strF.arg(lastId + 1).arg(ui->comboBox->currentText()).arg(infor_in);
        if (!query.exec(str)) {
            qDebug() << "Unable to do insert opeation";
        }
        ui->tableWidget->setRowCount(0);
        QSqlQuery que("SELECT id, ip, info FROM infor;");
        for (int i = 0; que.next(); i++){
            ui->tableWidget->insertRow(i);
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(que.value(0).toString()));
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(que.value(1).toString()));
            ui->tableWidget->setItem(i, 2, new QTableWidgetItem(query.value(2).toString().split("; ").at(0)));
            ui->tableWidget->setItem(i, 3, new QTableWidgetItem(query.value(2).toString().split("; ").at(1)));
            ui->tableWidget->setItem(i, 4, new QTableWidgetItem(query.value(2).toString().split("; ").at(2)));
            ui->tableWidget->setItem(i, 5, new QTableWidgetItem(query.value(2).toString().split("; ").at(3)));
            ui->tableWidget->setItem(i, 6, new QTableWidgetItem(query.value(2).toString().split("; ").at(4)));
            ui->tableWidget->setItem(i, 7, new QTableWidgetItem(query.value(2).toString().split("; ").at(5)));
            ui->tableWidget->setItem(i, 8, new QTableWidgetItem(query.value(2).toString().split("; ").at(6)));
            ui->tableWidget->setItem(i, 9, new QTableWidgetItem(query.value(2).toString().split("; ").at(7)));
            if (!standardValues.contains(query.value(2).toString().split("; ").at(0))) count_nest ++;
            if (!standardValues.contains(query.value(2).toString().split("; ").at(1))) count_nest ++;
            if (!standardValues.contains(query.value(2).toString().split("; ").at(2))) count_nest ++;
            if (!standardValues.contains(query.value(2).toString().split("; ").at(3))) count_nest ++;
            if (!standardValues.contains(query.value(2).toString().split("; ").at(4))) count_nest ++;
            if (!standardValues.contains(query.value(2).toString().split("; ").at(5))) count_nest ++;
            if (!standardValues.contains(query.value(2).toString().split("; ").at(6))) count_nest ++;
            if (!standardValues.contains(query.value(2).toString().split("; ").at(7))) count_nest ++;
            map.insert(query.value(1).toString(), count_nest);
            count_nest = 0;
        }
        auto maxIt = map.begin();
        for (auto it = map.begin(); it != map.end(); ++it) {
            if (it.value() > maxIt.value()) {
                maxIt = it;
            }
        }
        ui->lineEdit_3->setText(maxIt.key());
        bool hasDifferences = false;
        QStringList differences;
        if (computer_name != standard_computer_name) {
            differences.append(QString("computer_name отличается. Ожидалось: %1, получено: %2")
                               .arg(standard_computer_name)
                               .arg(computer_name));
            hasDifferences = true;
        }
        if (number_of_processors != standard_number_of_processors) {
            differences.append(QString("number_of_processors отличается. Ожидалось: %1, получено: %2")
                               .arg(standard_number_of_processors)
                               .arg(number_of_processors));
            hasDifferences = true;
        }
        if (oem_id != standard_oem_id) {
            differences.append(QString("oem_id отличается. Ожидалось: %1, получено: %2")
                               .arg(standard_oem_id)
                               .arg(oem_id));
            hasDifferences = true;
        }
        if (os_name != standard_os_name) {
            differences.append(QString("os_name отличается. Ожидалось: %1, получено: %2")
                               .arg(standard_os_name)
                               .arg(os_name));
            hasDifferences = true;
        }
        if (page_size != standard_page_size) {
            differences.append(QString("page_size отличается. Ожидалось: %1, получено: %2")
                               .arg(standard_page_size)
                               .arg(page_size));
            hasDifferences = true;
        }
        if (processor_architecture != standard_processor_architecture) {
            differences.append(QString("processor_architecture отличается. Ожидалось: %1, получено: %2")
                               .arg(standard_processor_architecture)
                               .arg(processor_architecture));
            hasDifferences = true;
        }
        if (processor_type != standard_processor_type) {
            differences.append(QString("processor_type отличается. Ожидалось: %1, получено: %2")
                               .arg(standard_processor_type)
                               .arg(processor_type));
            hasDifferences = true;
        }
        if (user_name != standard_user_name) {
            differences.append(QString("user_name отличается. Ожидалось: %1, получено: %2")
                               .arg(standard_user_name)
                               .arg(user_name));
            hasDifferences = true;
        }
        if (hasDifferences) {
            QString message = "Обнаружены следующие отличия:\n" + differences.join("\n");
            QMessageBox::warning(this, "Отличия найдены", message);
        } else {
            QMessageBox::information(this, "Совпадение", "Все значения соответствуют эталону.");
        }
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    int count_nest = 0;
    QMap <QString, int> map;
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите базу данных SQLite", "", "SQLite(*.db *.sqlite *.sqlite3);;All Files (*.*)");
    if (filePath.isEmpty()) {
        qDebug() << "No file db.";
        return;
    }
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filePath);
    if (db.open()) {
        QMessageBox::information(this, "База данных открыта", "База данных подключена");
    } else {
        QMessageBox::critical(this, "Ошибка открытия базы данных", "Ошибка открытия базы данных: " + db.lastError().text());
    }
    QSqlQuery query("SELECT id, ip, info FROM infor;");
    for (int i = 0; query.next(); i++){
        ui->tableWidget->insertRow(i);
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(query.value(1).toString()));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(query.value(2).toString().split("; ").at(0)));
        ui->tableWidget->setItem(i, 3, new QTableWidgetItem(query.value(2).toString().split("; ").at(1)));
        ui->tableWidget->setItem(i, 4, new QTableWidgetItem(query.value(2).toString().split("; ").at(2)));
        ui->tableWidget->setItem(i, 5, new QTableWidgetItem(query.value(2).toString().split("; ").at(3)));
        ui->tableWidget->setItem(i, 6, new QTableWidgetItem(query.value(2).toString().split("; ").at(4)));
        ui->tableWidget->setItem(i, 7, new QTableWidgetItem(query.value(2).toString().split("; ").at(5)));
        ui->tableWidget->setItem(i, 8, new QTableWidgetItem(query.value(2).toString().split("; ").at(6)));
        ui->tableWidget->setItem(i, 9, new QTableWidgetItem(query.value(2).toString().split("; ").at(7)));
        if (!standardValues.contains(query.value(2).toString().split("; ").at(0))) count_nest ++;
        if (!standardValues.contains(query.value(2).toString().split("; ").at(1))) count_nest ++;
        if (!standardValues.contains(query.value(2).toString().split("; ").at(2))) count_nest ++;
        if (!standardValues.contains(query.value(2).toString().split("; ").at(3))) count_nest ++;
        if (!standardValues.contains(query.value(2).toString().split("; ").at(4))) count_nest ++;
        if (!standardValues.contains(query.value(2).toString().split("; ").at(5))) count_nest ++;
        if (!standardValues.contains(query.value(2).toString().split("; ").at(6))) count_nest ++;
        if (!standardValues.contains(query.value(2).toString().split("; ").at(7))) count_nest ++;
        map.insert(query.value(1).toString(), count_nest);
        count_nest = 0;
    }
    auto maxIt = map.begin();
    for (auto it = map.begin(); it != map.end(); ++it) {
        if (it.value() > maxIt.value()) {
            maxIt = it;
        }
    }
    ui->lineEdit_3->setText(maxIt.key());
}

void MainWindow::on_pushButton_5_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Создание новой базы данных", "", "SQLite(*.db *.sqlite *.sqlite3);;All Files (*.*)");
    if (filePath.isEmpty()) {
        return;
    }
    if (QFile::exists(filePath)) {
        int response = QMessageBox::warning(this, "Файл существует", "Указанный файл уже существует. Вы хотите его перезаписать?", QMessageBox::Yes | QMessageBox::No);
        if (response == QMessageBox::No) {
            return;
        }
    }
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filePath);
    if (db.open()) {
        QMessageBox::information(this, "База данных создана", "База данных успешно создана.");
    } else {
        QMessageBox::critical(this, "Ошибка базы данных", "Ошибка создания базы данных: " + db.lastError().text());
    }
    QSqlQuery query;
    QString   str  = "CREATE TABLE infor ( "
                     "id INTEGER PRIMARY KEY NOT NULL, "
                     "ip VARCHAR(20),"
                     "info VARCHAR(1000)"
                     ");";
    if (!query.exec(str)) {
        qDebug() << "Unable to create a table";
    }
}

void MainWindow::on_pushButton_2_clicked() {
    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить отчет", "", "Word документы (*.docx);;Все файлы (*.*)");
    if (filePath.isEmpty()) {
        QMessageBox::information(this, "Отмена", "Файл не выбран.");
        return;
    }
    int selectedRow = ui->tableWidget->currentRow();
    if (selectedRow == -1) {
        QMessageBox::warning(this, "Предупреждение", "Строка не выбрана.");
        return;
    }
    QAxObject *wordApp = new QAxObject("Word.Application");
    if (!wordApp) {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать Word.");
        return;
    }
    wordApp->setProperty("Visible", true);
    QAxObject *documents = wordApp->querySubObject("Documents");
    if (!documents) {
        QMessageBox::critical(this, "Ошибка", "Не удалось получить документы.");
        delete wordApp;
        return;
    }
    QAxObject *document = documents->querySubObject("Add()");
    if (!document) {
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить документ.");
        delete wordApp;
        return;
    }
    QAxObject *content = wordApp->querySubObject("ActiveDocument()");
    if (!content) {
        QMessageBox::critical(this, "Ошибка", "Не удалось получить ActiveDocument.");
        document->dynamicCall("Close(bool)", true);
        delete wordApp;
        return;
    }
    QAxObject *range = content->querySubObject("Range()");
    if (!range) {
        QMessageBox::critical(this, "Ошибка", "Не удалось получить Range.");
        document->dynamicCall("Close(bool)", true);
        delete wordApp;
        return;
    }
    QString headers;
    for (int col = 2; col < ui->tableWidget->columnCount(); ++col) {
        QTableWidgetItem *headerItem = ui->tableWidget->horizontalHeaderItem(col);
        if (headerItem) {
            headers += headerItem->text() + "\t";
        }
    }
    range->dynamicCall("InsertAfter(Text)", headers + "\n");
    QString rowData;
    bool shouldHighlight = false;
    for (int col = 2; col < ui->tableWidget->columnCount(); ++col) {
        QTableWidgetItem *item = ui->tableWidget->item(selectedRow, col);
        if (item) {
            QString cellValue = item->text();
            if (!standardValues.contains(cellValue.trimmed())) {
                shouldHighlight = true;
            }
            rowData += cellValue + "\t";
        }
    }
    range->dynamicCall("InsertAfter(Text)", rowData + "\n");
    if (shouldHighlight) {
        range->dynamicCall("InsertAfter(Text)", "");
        range->dynamicCall("Shading.BackgroundPatternColor", 65535);
    }
    document->dynamicCall("SaveAs(const QString&)", filePath);
    document->dynamicCall("Close(bool)", true);
    wordApp->dynamicCall("Quit()");
    delete wordApp;
    QMessageBox::information(this, "Успех", "Данные выделенной строки экспортированы в Word.");
}
