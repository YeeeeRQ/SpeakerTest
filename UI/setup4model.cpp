#include "setup4model.h"
#include "ui_setup4model.h"
#include <QMessageBox>
#include <QFileDialog>

Setup4Model::Setup4Model(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Setup4Model)
{
    ui->setupUi(this);
    //   tableView显示属性设置
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    this->setWindowTitle("机种管理");
}

Setup4Model::~Setup4Model()
{
    delete ui;
}

void Setup4Model::openDB(const QString & dbFile)
{

    m_dbfile = dbFile;

//    QString dbFile=QFileDialog::getOpenFileName(this,"选择数据库文件","",
//                             "SQL Lite数据库(*.db *.db3)");
    if (dbFile.isEmpty())  //选择SQL Lite数据库文件
       return;

//打开数据库
    DB=QSqlDatabase::addDatabase("QSQLITE"); //添加 SQL LITE数据库驱动
    DB.setDatabaseName(dbFile); //设置数据库名称

//    DB.setHostName();
//    DB.setUserName();
//    DB.setPassword();

    if (!DB.open())   //打开数据库
    {
        QMessageBox::warning(this, "错误", "打开数据库失败",
                                 QMessageBox::Ok,QMessageBox::NoButton);
        this->close();
        return;
    }

//打开数据表
    this->openTable();
}

void Setup4Model::closeEvent(QCloseEvent *event)
{
    DB.close();
    emit closeWindow(m_dbfile);
}


void Setup4Model::openTable()
{
    tabModel = new QSqlTableModel(this, DB); //
    tabModel->setTable("ModelTable");

    tabModel->setEditStrategy(QSqlTableModel::OnManualSubmit); //数据保存方式
    tabModel->setSort(tabModel->fieldIndex("RecNo"), Qt::AscendingOrder); //排序

    if(!(tabModel->select()))
    {
        QMessageBox::critical(this, "错误信息",
              "打开数据表错误，错误信息\n"
              + tabModel->lastError().text(),
              QMessageBox::Ok, QMessageBox::NoButton);
        this->close();
        return;
    }

    tabModel->setHeaderData(tabModel->fieldIndex("RecNo"), Qt::Horizontal, "ID");
    tabModel->setHeaderData(tabModel->fieldIndex("ModelName"), Qt::Horizontal, "机种名");
    tabModel->setHeaderData(tabModel->fieldIndex("Memo"), Qt::Horizontal, "备注");

    theSelection=new QItemSelectionModel(tabModel);//关联选择模型
//theSelection当前项变化时触发currentChanged信号
    connect(theSelection,&QItemSelectionModel::currentChanged,
            this,&Setup4Model::on_currentChanged);
//选择行变化时
    connect(theSelection,&QItemSelectionModel::currentRowChanged,
            this,&Setup4Model::on_currentRowChanged);

    ui->tableView->setModel(tabModel);//设置数据模型
    ui->tableView->setSelectionModel(theSelection); //设置选择模型
}

void Setup4Model::revertTable()
{
    tabModel->revertAll();
    ui->btnSave->setEnabled(false); //有未保存修改时可用
    ui->btnUndo->setEnabled(false);
}

void Setup4Model::saveTable()
{
    bool res=tabModel->submitAll();

    if (!res)
        QMessageBox::information(this, "消息", "数据保存错误,错误信息\n"+tabModel->lastError().text(),
                                 QMessageBox::Ok,QMessageBox::NoButton);
    else
    {
        ui->btnSave->setEnabled(false); //有未保存修改时可用
        ui->btnUndo->setEnabled(false);
    }
}

void Setup4Model::appendRecord()
{
    tabModel->insertRow(tabModel->rowCount(),QModelIndex()); //在末尾添加一个记录

    QModelIndex curIndex=tabModel->index(tabModel->rowCount()-1,1);//创建最后一行的ModelIndex
    theSelection->clearSelection();//清空选择项
    theSelection->setCurrentIndex(curIndex,QItemSelectionModel::Select);//设置刚插入的行为当前选择行

//    int currow=curIndex.row(); //获得当前行
//    tabModel->setData(tabModel->index(currow,0),2000+tabModel->rowCount()); //自动生成编号
//    tabModel->setData(tabModel->index(currow,2),"男");
// 插入行时设置缺省值，需要在primeInsert()信号里去处理
}

void Setup4Model::deleteRecord()
{
    QModelIndex curIndex=theSelection->currentIndex();//获取当前选择单元格的模型索引
    tabModel->removeRow(curIndex.row());
}

void Setup4Model::on_currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);
    ui->btnSave->setEnabled(tabModel->isDirty()); //有未保存修改时可用
    ui->btnUndo->setEnabled(tabModel->isDirty());
}

void Setup4Model::on_currentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);
// 行切换时的状态控制
    ui->btnDelRecord->setEnabled(current.isValid());
}


void Setup4Model::on_btnNewRecord_clicked()
{
    this->appendRecord();
}


void Setup4Model::on_btnDelRecord_clicked()
{
    this->deleteRecord();
}


void Setup4Model::on_btnSave_clicked()
{
    this->saveTable();
}


void Setup4Model::on_btnUndo_clicked()
{
    this->revertTable();
}
