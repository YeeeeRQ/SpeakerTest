#ifndef SETUP4MODEL_H
#define SETUP4MODEL_H

#include <QWidget>
#include <QtSql>
#include <QDataWidgetMapper>

namespace Ui {
class Setup4Model;
}

class Setup4Model : public QWidget
{
    Q_OBJECT

public:
    explicit Setup4Model(QWidget *parent = nullptr);
    ~Setup4Model();

signals:
    void closeWindow(QString dbfile);

public slots:
    void openDB(const QString & dbFile);


private:
    Ui::Setup4Model *ui;
    void closeEvent(QCloseEvent *event) override;
    QString m_dbfile;


private:
    QSqlDatabase DB; //数据库连接
    QSqlTableModel *tabModel; //数据模型
    QItemSelectionModel *theSelection; //选择模型

    void openTable();
    void revertTable();
    void saveTable();

    void appendRecord();
    void deleteRecord();


    void on_currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void on_currentRowChanged(const QModelIndex &current, const QModelIndex &previous);

private slots:
    void on_btnNewRecord_clicked();

    void on_btnDelRecord_clicked();

    void on_btnSave_clicked();

    void on_btnUndo_clicked();
};

#endif // SETUP4MODEL_H
