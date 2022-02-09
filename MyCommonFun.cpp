#include "MyCommonFun.h"

void delaymsec(int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec,&loop,SLOT(quit()));
    loop.exec();
}
