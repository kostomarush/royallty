#include "monitoringobject.h"


MonitoringObject::MonitoringObject(QObject *parent) :
    QObject(parent),
    m_filePath(""),
    m_currentSize(0)
{

}

bool MonitoringObject::running() const
{
    return m_running;
}

QString MonitoringObject::filePath() const
{
    return m_filePath;
}

qint64 MonitoringObject::currentSize() const
{
    return m_currentSize;
}

void MonitoringObject::run()
{
    while (m_running)
    {
        if (QFileInfo(m_filePath).size() > m_currentSize){
            m_currentSize = QFileInfo(m_filePath).size();
//            QJsonDocument doc;
//            QJsonObject answer;
//            answer["answer"] = "getInfoFile";
//            answer["data"] = "info";
//            doc.setObject(answer);
            //m_mainWindow->sendCommand(doc,);
            qDebug() << "Размер файла превышен!";
            QString path = m_filePath;
            qint64 size = QFileInfo(m_filePath).size();
            QString fileName = QFileInfo(m_filePath).fileName();
            QString dataCreate = QFileInfo(m_filePath).created().toString();
            QString lastModified = QFileInfo(m_filePath).lastModified().toString();
            emit sendData(path, size, fileName, dataCreate, lastModified);
        }
    }
    emit finished();
}

void MonitoringObject::setRunning(bool running)
{
    if (m_running == running)
        return;

    m_running = running;
    emit runningChanged(m_running);
}

void MonitoringObject::setFilePath(QString filePath)
{
    if (m_filePath == filePath)
        return;

    m_filePath = filePath;
    emit filePathChanged(m_filePath);
}

void MonitoringObject::setcurrentSize(qint64 currentSize)
{
    if (m_currentSize == currentSize)
        return;

    m_currentSize = currentSize;
    emit currentSizeChanged(m_currentSize);
}
