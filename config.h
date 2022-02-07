#ifndef CONFIG_H
#define CONFIG_H

#include <QVariant>
#include <QSettings>

class Config
{
public:
    static Config & getInstance();

    void Set(QString, QString, QVariant);
    QVariant Get(QString, QString);
private:
    QString m_qstrFileName;
    QSettings *m_psetting;
private:
    Config();
    ~Config();
    Config(const Config&) = delete;
    Config& operator =(const Config&) = delete;
};

#endif // CONFIG_H
