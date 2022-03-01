#include "MyCommonFun.h"

void delaymsec(int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec,&loop,SLOT(quit()));
    loop.exec();
}

//调用命令行命令而不显示命令行窗口
BOOL system_hide(char* CommandLine)
{
    SECURITY_ATTRIBUTES   sa;
    HANDLE   hRead,hWrite;

    sa.nLength   =   sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor   =   NULL;
    sa.bInheritHandle   =   TRUE;
    if   (!CreatePipe(&hRead,&hWrite,&sa,0))
    {
        return   FALSE;
    }

    STARTUPINFO   si;
    PROCESS_INFORMATION   pi;
    si.cb   =   sizeof(STARTUPINFO);
    GetStartupInfo(&si);
    si.hStdError   =   hWrite;
    si.hStdOutput   =   hWrite;
    si.wShowWindow   =   SW_HIDE;
    si.dwFlags   =   STARTF_USESHOWWINDOW   |   STARTF_USESTDHANDLES;
    //关键步骤，CreateProcess函数参数意义请查阅MSDN
    if   (!CreateProcess(NULL, CommandLine, NULL,NULL,TRUE,NULL,NULL,NULL,&si,&pi))
    {
        return   FALSE;
    }
    CloseHandle(hWrite);

    char   buffer[4096]   =   {0};
    DWORD   bytesRead;
    while(true)
    {
        memset(buffer,0,strlen(buffer));
        if(ReadFile(hRead,buffer,4095,&bytesRead,NULL)==NULL)
            break;
        //buffer中就是执行的结果，可以保存到文本，也可以直接输出
        //printf(buffer);//这行注释掉就可以了
        Sleep(100);
    }
    return   TRUE;
}
