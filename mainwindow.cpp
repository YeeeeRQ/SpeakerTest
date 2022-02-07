#include <QDateTime>
#include <QScrollBar>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,log(SimpleLog::getInstance(&textedit4log))
{
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow()
{
    delete setup4mic;
    delete ui;
}

void MainWindow::init()
{
    this->setWindowTitle(tr("双通道扬声器测试"));

    textedit4log.setEnabled(false);
    textedit4log.verticalScrollBar()->hide();

    QDateTime cur = QDateTime::currentDateTime();
    ui->statusbar->showMessage("启动时间 : " +
                               cur.toString("yyyy-MM-dd hh:mm:ss"), 0);

    ui->verticalLayout4log->addWidget(&textedit4log);

    // UI
    setup4mic = new Setup4Mic();
    //    debug
    setup4mic->showNormal();
    setup4mic->setWindowState(Qt::WindowActive);

    this->setWindowState(Qt::WindowMinimized);



    // todo

//    ui->comboBoxModel->set

}

void MainWindow::saveConfig()
{

}

void MainWindow::loadConfig()
{

}

void MainWindow::resetConfig()
{

}

void MainWindow::startRecord()
{
    //驱动两麦克风同时录音



}


void MainWindow::on_btnLockOption4Model_clicked()
{
    log.blue("hello");
    log.warn("hell");

}


void MainWindow::on_btnSetting4Mic_clicked()
{
    setup4mic->show();
}

void MainWindow::on_btnLoadWavDir_clicked()
{
    this->wavdir = QFileDialog::getExistingDirectory();
    ui->lineEdit4ShowWavDir->setText(wavdir);
    log.warn(wavdir);
}


void MainWindow::on_btnLoadWavDir4Test_clicked()
{

}






void MainWindow::on_btnStartRecord_clicked()
{
    //Start Record

    // 超时自动停止 > 3s

}


void MainWindow::on_btnStopRecord_clicked()
{
    //Stop Record
}


void MainWindow::on_btnStartTest_clicked()
{
    // 手动测试
    // Test
    // Test
    // Test
}


void MainWindow::on_btnSetting4Model_clicked()
{//机种管理


}

