#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QCommandLineParser>
#include <QDebug>
#include <QNetworkProxy>
#include <windows.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QSysInfo>
#include <QDebug>
#include <QHostInfo>
#include <QStorageInfo>
#include <QProcessEnvironment>
#include <Lmcons.h>
#include <QJsonArray>

class MyServer : public QObject {
    Q_OBJECT

public:
    MyServer(QObject* parent = nullptr, quint16 port = 5555, QNetworkProxy proxy = QNetworkProxy::NoProxy)
        : QObject(parent), server(new QTcpServer(this)), port(port) {
        connect(server, &QTcpServer::newConnection, this, &MyServer::handleNewConnection);
        if (server->listen(QHostAddress::Any, port)) {
            qDebug() << "Server started on port" << port;
        } else {
            qDebug() << "Failed to start server: " << server->errorString();
        }
    }

private slots:
    void handleNewConnection() {
        QTcpSocket* clientSocket = server->nextPendingConnection();
        connect(clientSocket, &QTcpSocket::readyRead, this, &MyServer::handleClientData);
        connect(clientSocket, &QTcpSocket::disconnected, this, &MyServer::handleClientDisconnect);
        qDebug() << "New client connected";
    }
    QJsonObject collectSystemInfo() {
        QJsonObject systemInfo;
        SYSTEM_INFO siSysInfo;
        GetSystemInfo(&siSysInfo);
        systemInfo["oem_id"] = QString::number(siSysInfo.dwOemId);
        systemInfo["processor_type"] = QString::number(siSysInfo.dwProcessorType);
        systemInfo["processor_architecture"] = QString::number(siSysInfo.wProcessorArchitecture);
        systemInfo["number_of_processors"] = QString::number(siSysInfo.dwNumberOfProcessors);
        systemInfo["page_size"] = QString::number(siSysInfo.dwPageSize);
        TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD computerNameLen = MAX_COMPUTERNAME_LENGTH + 1;
        if (GetComputerName(computerName, &computerNameLen)) {
            systemInfo["computer_name"] = QString::fromWCharArray(computerName);
        }
        TCHAR userName[UNLEN + 1];
        DWORD userNameLen = UNLEN + 1;
        if (GetUserName(userName, &userNameLen)) {
            systemInfo["user_name"] = QString::fromWCharArray(userName);
        }
        systemInfo["os_name"] = QSysInfo::prettyProductName();
        systemInfo["os_build"] = QString::number(QSysInfo::windowsVersion());
        QList<QStorageInfo> storageList = QStorageInfo::mountedVolumes();
        return systemInfo;
    }

    void handleClientData() {
        QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
        if (clientSocket) {
            QByteArray data = clientSocket->readAll();
            qDebug() << "Received data:" << data;
            QJsonObject systemInfo = collectSystemInfo();
            clientSocket->write(JsonToString(&systemInfo).toUtf8());
        }
    }
    void handleClientDisconnect() {
        QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
        if (clientSocket) {
            qDebug() << "Client disconnected";
            clientSocket->deleteLater();
        }
    }
    QString JsonToString (QJsonObject *a)
    {
        return QString (QJsonDocument (*a).toJson(QJsonDocument::Compact));
    }

    QJsonObject StringToJson (QString a)
    {
        return QJsonDocument::fromJson (a.toUtf8()).object();
    }

private:
    QTcpServer* server;
    quint16 port;
    QNetworkProxy proxy;
};

int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("Simple TCP Server with Proxy Configuration");
    parser.addHelpOption();
    parser.addOption(QCommandLineOption("port", "Port number for the server.", "port", "5555"));
    parser.process(a);
    quint16 port = parser.value("port").toUShort();
    MyServer server(nullptr, port);
    return a.exec();
}

#include "main.moc"
