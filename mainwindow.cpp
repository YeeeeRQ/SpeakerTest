#include <QDateTime>
#include <QScrollBar>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#define WAV_FILE_L  "/L.wav"
#define WAV_FILE_R  "/R.wav"

////////////////////////////////////////////////////////////////////////////////
#include   <windows.h>

//调用命令行命令而不显示命令行窗口
BOOL system_hide(char* CommandLine)
{
    SECURITY_ATTRIBUTES   sa;
    HANDLE   hRead,hWrite;

    sa.nLength   =   sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor   =   NULL;
    sa.bInheritHandle   =   TRUE;
    if   (!CreatePipe(&hRead,&hWrite,&sa,0))
    {
        return   FALSE;
    }

    STARTUPINFO   si;
    PROCESS_INFORMATION   pi;
    si.cb   =   sizeof(STARTUPINFO);
    GetStartupInfo(&si);
    si.hStdError   =   hWrite;
    si.hStdOutput   =   hWrite;
    si.wShowWindow   =   SW_HIDE;
    si.dwFlags   =   STARTF_USESHOWWINDOW   |   STARTF_USESTDHANDLES;
    //关键步骤，CreateProcess函数参数意义请查阅MSDN
    if   (!CreateProcess(NULL, CommandLine, NULL,NULL,TRUE,NULL,NULL,NULL,&si,&pi))
    {
        return   FALSE;
    }
    CloseHandle(hWrite);

    char   buffer[4096]   =   {0};
    DWORD   bytesRead;
    while(true)
    {
        memset(buffer,0,strlen(buffer));
        if(ReadFile(hRead,buffer,4095,&bytesRead,NULL)==NULL)
            break;
        //buffer中就是执行的结果，可以保存到文本，也可以直接输出
        //printf(buffer);//这行注释掉就可以了
        Sleep(100);
    }
    return   TRUE;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,log(SimpleLog::getInstance(&textedit4log))
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug()<<"current applicationDirPath: "<<QCoreApplication::applicationDirPath();
    qDebug()<<"current currentPath: "<<QDir::currentPath();

    default_WorkDir = QCoreApplication::applicationDirPath() + "\\temp\\";
    default_AudioTestDir = QCoreApplication::applicationDirPath() + "\\temp\\test";
    current_WorkDir = default_WorkDir;
    current_AudioTestDir = default_AudioTestDir;

    ui->lineEdit4WavDir->setText(default_AudioTestDir);

    QDir dir;
    if(!dir.exists(default_WorkDir)){
        dir.mkdir(default_WorkDir);
    }
    if(!dir.exists(default_AudioTestDir)){
        dir.mkdir(default_AudioTestDir);
    }

    ui->lineEdit4WavDir->setText(default_AudioTestDir);


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
    connect(this, &MainWindow::startTestAudio, this, &MainWindow::testAudio);
    connect(this, &MainWindow::audioTestFinished, this, &MainWindow::onAudioTestFinished);
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
    emit startRecording(2000);
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
    if(!QFile::exists(wavL)){
        log.warn("指定目录下 \"L.wav\" 文件不存在！");
        return;
    }
    if(!QFile::exists(wavR)){
        log.warn("指定目录下 \"R.wav\" 文件不存在！");
        return;
    }
    log.info("载入指定目录下 L.wav & R.wav 文件成功.");

    // call test dll
    emit startTestAudio();
}

void MainWindow::testAudio()
{
    // ConsoleAppAudioTest.exe
    QString app = QCoreApplication::applicationDirPath() + "\\AudioTest\\ConsoleAppAudioTest.exe";
    QString cmd = app + " " + current_AudioTestDir + " 2>NUL 1>NUL ";
    qDebug() << app;
    qDebug() << cmd;

//    system(cmd.toLatin1());
    system_hide((char*)cmd.toLatin1().data());

    emit audioTestFinished();
}

void MainWindow::onAudioTestFinished()
{
    QString csv_file = current_AudioTestDir + "\\test.csv";
    qDebug() << csv_file;

    if(!QFile::exists(csv_file)){
      log.warn("测试目录下无测试结果文件！测试失败！");
    }

    // 读取csv文件, 输出结果
    QFile f{csv_file};

    if (!f.open(QIODevice::ReadOnly)) {
        log.warn("测试结果文件读取失败！");
    }

    QString line_l, line_r;
    QTextStream in{&f};
    if(!in.atEnd())
        line_l = in.readLine();
    if(!in.atEnd())
        line_r = in.readLine();

    if(line_l.isEmpty() || line_r.isEmpty()){
        log.warn("测试结果文件异常！");
    }

    QStringList L_data= line_l.split(',');
    QStringList R_data= line_r.split(',');
    // 判断 Pass | Fail
    qDebug() << "----------result----------\n";
    QString result_l, result_r;
    for(size_t i=0 ; i <L_data.length();++i){
        result_l += L_data.at(i) + "  ";
    }
    for(size_t i=0 ; i <R_data.length();++i){
        result_r += R_data.at(i) + "  ";
    }
    qDebug() << result_l;
    qDebug() << result_r;

    // pitch
    double lpitch1, rpitch1, lpitch2, rpitch2;
    lpitch1 = L_data.at(2).toDouble();
    rpitch1 = R_data.at(2).toDouble();
    lpitch2 = L_data.at(4).toDouble();
    rpitch2 = R_data.at(4).toDouble();



    //

    double llevel1, rlevel1, llevel2, rlevel2;
    llevel1 = L_data.at(1).toDouble();
    rlevel1 = R_data.at(1).toDouble();
    llevel2 = L_data.at(3).toDouble();
    rlevel2 = R_data.at(3).toDouble();



}


void MainWindow::on_btnSetting4Model_clicked()
{//机种管理


}

