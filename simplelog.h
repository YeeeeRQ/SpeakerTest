#ifndef SIMPLELOG_H
#define SIMPLELOG_H

#include <QPlainTextEdit>

// 仅单线程, 单模块中使用
class SimpleLog
{
public:
    static SimpleLog& getInstance(QPlainTextEdit * output); // need a QPlainTextEdit

private:
    SimpleLog();
    ~SimpleLog();
    SimpleLog(const SimpleLog&) = delete;
    SimpleLog& operator =(const SimpleLog&) = delete;
    QPlainTextEdit* output = nullptr; // !警告!不负责析构


public:
    void info(QString s);
    void warn(QString s);
    void blue(QString s);
    void red(QString s);
    void gray(QString s);
    void clear();
    void setupOutput();
private:
    void checkOutput();
    void printWithColor(QString s, QString color);
};

#endif // SIMPLELOG_H
