#ifndef MONITORINGOBJECT_H
#define MONITORINGOBJECT_H

#include <QObject>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>

class MonitoringObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(qint64 currentSize READ currentSize WRITE setcurrentSize NOTIFY currentSizeChanged)

    bool m_running;
    QString m_filePath;
    qint64 m_currentSize;


public:
    explicit MonitoringObject(QObject *parent = nullptr);
    bool running() const;
    QString filePath() const;
    qint64 currentSize() const;

signals:
    void finished();
    void runningChanged(bool running);
    void filePathChanged(QString filePath);
    void currentSizeChanged(qint64 currentSize);

public slots:
    void run();
    void setRunning(bool running);
    void setFilePath(QString filePath);
    void setcurrentSize(qint64 currentSize);



};

#endif // MONITORINGOBJECT_H
