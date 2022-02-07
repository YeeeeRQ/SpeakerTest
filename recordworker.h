#ifndef RECORDWORKER_H
#define RECORDWORKER_H

#include <QObject>
#include <QTimer>
#include <QDebug>

class RecordWorker : public QObject
{
    Q_OBJECT
public:
    explicit RecordWorker(QObject *parent = nullptr);

signals:

};

#endif // RECORDWORKER_H
