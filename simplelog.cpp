#include "simplelog.h"
#include <QTime>

#define LINELIMIT 100

SimpleLog::SimpleLog()
{

}

SimpleLog &SimpleLog::getInstance(QPlainTextEdit *output)
{
    static SimpleLog instance;

    instance.output = output;

    return instance;
}

SimpleLog::~SimpleLog()
{
    output = nullptr;
}

void SimpleLog::info(QString s)
{
    this->gray(s);
}


void SimpleLog::warn(QString s)
{
    this->red(s);
}

void SimpleLog::blue(QString s)
{
    printWithColor(s, "blue");
}

void SimpleLog::red(QString s)
{
    printWithColor(s, "red");
}

void SimpleLog::green(QString s)
{
    printWithColor(s, "green");
}

void SimpleLog::gray(QString s)
{
    printWithColor(s, "gray");
}

void SimpleLog::clear()
{
    output->clear();
}

void SimpleLog::setupOutput()
{
    if(output){
//        QFont font = QFont("Microsoft YaHei",20,2);
//        output->setFont(font);
    }
}

void SimpleLog::printWithColor(QString s, QString color)
{
    checkOutput();

    QString currTime = QTime::currentTime().toString("hh:mm:ss zzz");
    currTime.prepend("[");
    currTime.append("]:");

    QString qHtmlText(QString("<p style='color:%1;font-family:Source Han Sans K Regular;font-size:19px;'>").arg(color));
    qHtmlText.append(currTime);
    qHtmlText.append(s);
    qHtmlText.append("</p>");
    output->appendHtml(qHtmlText);
}

void SimpleLog::checkOutput()
{
    if(output->blockCount() > LINELIMIT)
    {
        output->clear();
    }
}
