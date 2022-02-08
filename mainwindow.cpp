#include <QDateTime>
#include <QScrollBar>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#define WAV_FILE_L  "/L.wav"
#define WAV_FILE_R  "/R.wav"

////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,log(SimpleLog::getInstance(&textedit4log))
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug()<<"current applicationDirPath: "<<QCoreApplication::applicationDirPath();
    qDebug()<<"current currentPath: "<<QDir::currentPath();

    // Config
    bool hasConf = QFile::exists(QCoreApplication::applicationDirPath()
                                 + "\\Config.ini");
    if(!hasConf) initConfig();

    // AutoLine
    m_AutoLine = new AutoLine;
    connect(m_AutoLine, &AutoLine::receiveCmd, this, &MainWindow::onAutoLineReceiveCmd);
    connect(m_AutoLine, &AutoLine::connectStatusChanged, this, &MainWindow::onAutoLineConnectStatusChanged);

    // 读码器
    m_CodeReader = new CodeReader;
    m_CodeReader->setLogger(&log);
    connect(m_CodeReader, &CodeReader::receiveBarcode, this, &MainWindow::onCodeReaderReceiveBarcode);
    connect(m_CodeReader, &CodeReader::connectStatusChanged, this, &MainWindow::onCodeReaderConnectStatusChanged);

    // Test
    connect(this, &MainWindow::startAutoProcess, this, &MainWindow::autoProcess);
//    connect(this, &MainWindow::startTesting, this, &MainWindow::);

    // Todo:
    // 麦克风检测 个数小于2 给警报
    // 程序启动时打开外部设备串口

    init();
}

MainWindow::~MainWindow()
{

    recordThread4L.quit();
    recordThread4R.quit();

    recordThread4L.wait();
    recordThread4R.wait();


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

    // 麦克风数量检测
    QAudioRecorder* recorder = new QAudioRecorder(this);
    QStringList  devList= recorder->audioInputs();
    log.info("麦克风输入"+QString::number(devList.count()));
    QSet<QString> audioInputs = QSet<QString>(devList.begin(), devList.end());
    foreach (const QString &device, audioInputs)
        qDebug() << (device); //音频录入设备列表

    if(audioInputs.count() < 2){
        log.warn("麦克风数量小于2, 测试无法进行.");
    }else{
    // -------------------------------------------------------------------------

    RecordWorker* recWorkerL = new RecordWorker;
    recWorkerL->moveToThread(&recordThread4L);

    connect(this, SIGNAL(startRecording(quint64)), recWorkerL, SLOT(doWork(quint64)));
    connect(&recordThread4L, &QThread::finished, recWorkerL, &QObject::deleteLater);
    connect(recWorkerL, SIGNAL(resultReady()), this, SLOT(onRecordingOver()));

    recWorkerL->setOutputFile("D:\\Temp\\L.wav");

    // 启动录制线程
    recordThread4L.start();

    // -------------------------------------------------------------------------

    RecordWorker* recWorkerR = new RecordWorker;
    recWorkerR->moveToThread(&recordThread4R);

    connect(this, SIGNAL(startRecording(quint64)), recWorkerR, SLOT(doWork(quint64)));
    connect(&recordThread4L, &QThread::finished, recWorkerR, &QObject::deleteLater);
    connect(recWorkerR, SIGNAL(resultReady()), this, SLOT(onRecordingOver()));

    recWorkerR->setOutputFile("D:\\Temp\\R.wav");
//    recWorkerR->setAudioInput(audioInputs.begin());

    // 启动录制线程
    recordThread4R.start();

    // -------------------------------------------------------------------------
    }

    // UI
    setup4mic = new Setup4Mic();
//    setup4mic->showNormal();
//    setup4mic->setWindowState(Qt::WindowActive);

//    this->setWindowState(Qt::WindowMinimized);



    // todo

//    ui->comboBoxModel->set


    // 主要输出目录检测
    if(m_outputDir.isEmpty()){
        log.warn("异常！ 主要输出目录未设定。");
    }
    QDir dir(m_outputDir);
    if(!dir.exists()){
        bool ismkdir = dir.mkdir(m_outputDir);
        if(!ismkdir){
            log.warn("创建"+m_outputDir+"失败!");
            log.warn("程序无法正确运行");
        }else{
            log.info("创建"+m_outputDir+"成功!");
        }
    }
}


void MainWindow::onRecordingOver()
{
    //录制结束
    qDebug() << "录制结束";
}

void MainWindow::onTestStarted()
{

}

void MainWindow::initConfig()
{
    // AutoLine
    conf.Set("AutoLine","Com","COM2");
    conf.Set("AutoLine","Baud",9600);
    conf.Set("AutoLine","Delay",100);
    conf.Set("AutoLine","StartCmd","START");
    conf.Set("AutoLine","PassCmd","PASS");
    conf.Set("AutoLine","FailCmd","PASS");
    conf.Set("AutoLine","SendCount",2);
    conf.Set("AutoLine","SendSpanTime",200);

    // ReadEsn
    conf.Set("ReadEsn","Com","COM1");
    conf.Set("ReadEsn","Baud",9600);

    // DCT
    conf.Set("DCT","Enable",true);
    conf.Set("DCT","Com","COM6");
    conf.Set("DCT","Baud",9600);

    // Audio
    conf.Set("Audio", "Path", "D:\\AudioTest");

    // AutoTest
    conf.Set("AutoTest", "Delay", 0);
}

void MainWindow::saveConfig()
{
}

void MainWindow::loadConfig()
{
    m_outputDir = conf.Get("Audio", "Path").toString();

    m_cmd_start = conf.Get("AutoLine", "StartCmd").toString();
    m_cmd_pass = conf.Get("AutoLine", "PassCmd").toString();
    m_cmd_fail = conf.Get("AutoLine", "FailCmd").toString();

    m_AutoTestDelay = conf.Get("AutoTest", "Delay").toUInt();

}

void MainWindow::resetConfig()
{

}

/*
1. 机种名 XMXXXAFED
2. 日期  2022-02-01
3. 产品ID+测试时间+（OK|NG）

* 匿名机种 类型 anonymous
* 匿名产品 ID

*/
void MainWindow::autoProcess()
{
    QString model_name = ui->comboBoxModelName->currentText();
    QString product_id = ui->lineEdit4ProductID->text();

    QString curDate = QDate::currentDate().toString("yyyy-MM-dd");
    QString curTime = QTime::currentTime().toString("hh:mm:ss");

    QString testResult;  // OK | NG

    // 延时
    delaymsec(m_AutoTestDelay);

    // 录制

    // 测试 音频
}

void MainWindow::testAudio()
{

    // 测试 音频
}

void MainWindow::onCodeReaderReceiveBarcode(QString barcode)
{

}

void MainWindow::onCodeReaderConnectStatusChanged()
{

}

void MainWindow::onAutoLineReceiveCmd(QString rev_cmd)
{
    qDebug() << "Received Command: " << rev_cmd;

    if(rev_cmd == m_cmd_start){
         emit startAutoProcess();
    }
}

void MainWindow::onAutoLineConnectStatusChanged()
{

}


void MainWindow::on_btnLockOption4Model_clicked()
{
    log.blue("hello");
}


void MainWindow::on_btnSetting4Mic_clicked()
{
    setup4mic->show();
}

void MainWindow::on_btnLoadWavDir_clicked()
{
    this->wavDir = QFileDialog::getExistingDirectory();
    ui->lineEdit4WavDir->setText(wavDir);
    log.warn(wavDir);
}


void MainWindow::on_btnStartRecord_clicked()
{//Start Record
    static quint64 duration = ui->lineEditDurationOfRecord->text().toUInt();
    emit startRecording(duration);
}


void MainWindow::on_btnStartTest_clicked()
{
    // 载入指定目录下 L.wav R.wav文件
    QString workdir = ui->lineEdit4WavDir->text().trimmed();
    QString wavL = workdir + WAV_FILE_L;
    QString wavR = workdir + WAV_FILE_R;

    if(workdir.isEmpty()){
        log.warn("未指定wav文件存放目录！");
        return;
    }
    if(QFile::exists(wavL)){
        log.warn("指定目录下 \"L.wav\" 文件不存在！");
        return;
    }
    if(QFile::exists(wavR)){
        log.warn("指定目录下 \"R.wav\" 文件不存在！");
        return;
    }
    log.info("载入指定目录下 L.wav & R.wav 文件成功.");

    // call test dll
    emit startTestAudio();
}


void MainWindow::on_btnSetting4Model_clicked()
{//机种管理


}

