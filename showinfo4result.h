#ifndef SHOWINFO4RESULT_H
#define SHOWINFO4RESULT_H

#include <QWidget>

namespace Ui {
class ShowInfo4Result;
}

class ShowInfo4Result : public QWidget
{
    Q_OBJECT

public:
    explicit ShowInfo4Result(QWidget *parent = nullptr);
    ~ShowInfo4Result();

private slots:
    void on_btnClear_clicked();

private:
    Ui::ShowInfo4Result *ui;
};

#endif // SHOWINFO4RESULT_H
