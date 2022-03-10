#include "mainwindow.h"

//#include <QLoggingCategory>

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QThread>
#include <QDebug>
#include <QFont>

bool onLoadFont(const QApplication& app, const QString& strPath);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "SpeakersTest_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    qDebug() << "Thread ID(Main): " << QThread::currentThreadId() << Qt::endl;

    //根据添加时id打印字体名
    int id =QFontDatabase::addApplicationFont("://font/SourceHanSansK-Regular.ttf");
    qDebug()<<"family"<<QFontDatabase::applicationFontFamilies(id);

    MainWindow w;
    w.show();
    w.emitMainWindowLoaded();
    return a.exec();
}

bool onLoadFont(const QApplication& app, const QString& strPath)
{
    QFile dFontFile(strPath);
    qDebug() << dFontFile.Text;
    if(!dFontFile.open(QIODevice::ReadOnly))
    {
        //说明打开字体文件失败了
        return false;
    }

    int nFontId = QFontDatabase::addApplicationFontFromData(dFontFile.readAll());
    if(nFontId == -1)
    {
        //说明加载字体文件失败了，该字体不可用
        return false;
    }

    QStringList lFontFamily = QFontDatabase::applicationFontFamilies(nFontId);
    if(lFontFamily.empty())
    {
        //说明从字体中获取字体簇失败了
        return false;
    }

    QFont font(lFontFamily.at(0));
//    qApp->setFont(font);
    app.setFont(font);
    return true;
}
