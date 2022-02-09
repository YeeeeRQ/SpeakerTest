#ifndef MYCOMMONFUN_H
#define MYCOMMONFUN_H

#include <QEventLoop>
#include <QTimer>
#include   <windows.h>

// Qt基于事件循环实现的非阻塞延时
void delaymsec(int msec);

//调用命令行命令而不显示命令行窗口
BOOL system_hide(char* CommandLine);

#endif // MYCOMMONFUN_H
