#include "simplelog.h"

#define LINELIMIT 200

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

void SimpleLog::gray(QString s)
{
    printWithColor(s, "gray");
}

void SimpleLog::clear()
{
    output->clear();
}

void SimpleLog::printWithColor(QString s, QString color)
{
    checkOutput();
    QString qHtmlText(QString("<p style='color:%1'>").arg(color));
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
