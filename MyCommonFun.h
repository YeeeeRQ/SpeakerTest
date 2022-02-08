#ifndef MYCOMMONFUN_H
#define MYCOMMONFUN_H

#include <QEventLoop>
#include <QTimer>

void delaymsec(int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec,&loop,SLOT(quit()));
    loop.exec();
}

#endif // MYCOMMONFUN_H
