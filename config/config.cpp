#include "config.h"
#include <QtCore>
#include <QDebug>

Config::Config()
{
    m_qstrFileName = QCoreApplication::applicationDirPath() + "/Config.ini";
    m_psetting = new QSettings(m_qstrFileName, QSettings::IniFormat);
}

Config::~Config()
{
    delete m_psetting;
    m_psetting = nullptr;
}

Config &Config::getInstance()
{
    static Config conf;
    return conf;
}

void Config::Set(QString node, QString key, QVariant value)
{
    m_psetting->setValue(QString("/%1/%2").arg(node, key), value);
}

QVariant Config::Get(QString node, QString key)
{
    QVariant var = m_psetting->value(QString("/%1/%2").arg(node, key));
    return var;
}
