#ifndef SETUP4DEVICES_H
#define SETUP4DEVICES_H

#include <QWidget>

namespace Ui {
class Setup4Devices;
}

class Setup4Devices : public QWidget
{
    Q_OBJECT

public:
    explicit Setup4Devices(QWidget *parent = nullptr);
    ~Setup4Devices();

private:
    Ui::Setup4Devices *ui;
};

#endif // SETUP4DEVICES_H
