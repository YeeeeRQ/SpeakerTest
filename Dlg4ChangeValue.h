#ifndef DLG4CHANGEVALUE_H
#define DLG4CHANGEVALUE_H

#include <QDialog>

namespace Ui {
class Dlg4ChangeValue;
}

class Dlg4ChangeValue : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg4ChangeValue(QWidget *parent = nullptr);
    ~Dlg4ChangeValue();

private:
    Ui::Dlg4ChangeValue *ui;
};

#endif // DLG4CHANGEVALUE_H
