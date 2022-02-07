#ifndef SETUP4MODEL_H
#define SETUP4MODEL_H

#include <QWidget>

namespace Ui {
class Setup4Model;
}

class Setup4Model : public QWidget
{
    Q_OBJECT

public:
    explicit Setup4Model(QWidget *parent = nullptr);
    ~Setup4Model();

private:
    Ui::Setup4Model *ui;
};

#endif // SETUP4MODEL_H
