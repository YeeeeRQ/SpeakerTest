#ifndef SETUP4AUTOTEST_H
#define SETUP4AUTOTEST_H

#include <QWidget>

namespace Ui {
class Setup4AutoTest;
}

class Setup4AutoTest : public QWidget
{
    Q_OBJECT

public:
    explicit Setup4AutoTest(QWidget *parent = nullptr);
    ~Setup4AutoTest();

private:
    Ui::Setup4AutoTest *ui;
};

#endif // SETUP4AUTOTEST_H
