#include <QDateTime>
#include <QScrollBar>
#include <numeric>

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

    // 主题设定
    QApplication::setStyle(QStyleFactory::create("Fusion"));               // 更改风格
    QApplication::setPalette(QApplication::style()->standardPalette());     // 使用风格默认的颜色

    // 背景
    QPalette palette;
    palette.setColor(QPalette::Background,QColor(250,250,250));
    //palette.setBrush(QPalette::Background,QBrush(QPixmap(":/background.png")));
    this->setPalette(palette);


    //
    qDebug()<<"current applicationDirPath: "<<QCoreApplication::applicationDirPath();
    qDebug()<<"current currentPath: "<<QDir::currentPath();

    // 目录设定
    default_WorkDir = QCoreApplication::applicationDirPath() + "\\temp\\";
    default_AudioTestDir = QCoreApplication::applicationDirPath() + "\\temp\\test";
    current_WorkDir = default_WorkDir;
    current_AudioTestDir = default_AudioTestDir;

    QDir dir;
    if(!dir.exists(default_WorkDir)){
        dir.mkdir(default_WorkDir);
    }
    if(!dir.exists(default_AudioTestDir)){
        dir.mkdir(default_AudioTestDir);
    }

    ui->lineEdit4WavDir->setText(default_AudioTestDir);
    ui->lineEdit4WavDir->setToolTip(default_AudioTestDir);


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



    loadConfig();

    init();
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

    recWorkerL = new RecordWorker;
    recWorkerL->moveToThread(&m_recordThread4L);


    // 录音前提：设定好2个麦克风输入
    connect(this, SIGNAL(startRecording(quint64)), recWorkerL, SLOT(doWork(quint64)));
    connect(&m_recordThread4L, &QThread::finished, recWorkerL, &QObject::deleteLater);
    connect(recWorkerL, SIGNAL(resultReady()), this, SLOT(onRecordingOver()));

    connect(this, &MainWindow::setRecordInputL, recWorkerL, &RecordWorker::setAudioInput);
    connect(this, &MainWindow::setRecordOutputL, recWorkerL, &RecordWorker::setOutputFile);

    recWorkerL->setOutputFile("D:\\Temp\\L.wav");

    // 启动录制线程
    m_recordThread4L.start();

    // -------------------------------------------------------------------------

    recWorkerR = new RecordWorker;
    recWorkerR->moveToThread(&m_recordThread4R);

    connect(this, SIGNAL(startRecording(quint64)), recWorkerR, SLOT(doWork(quint64)));
    connect(&m_recordThread4L, &QThread::finished, recWorkerR, &QObject::deleteLater);
    connect(recWorkerR, SIGNAL(resultReady()), this, SLOT(onRecordingOver()));

    connect(this, &MainWindow::setRecordInputR, recWorkerR, &RecordWorker::setAudioInput);
    connect(this, &MainWindow::setRecordOutputR, recWorkerR, &RecordWorker::setOutputFile);

    recWorkerR->setOutputFile("D:\\Temp\\R.wav");
//    recWorkerR->setAudioInput(audioInputs.begin());

    // 启动录制线程
    m_recordThread4R.start();

    // -------------------------------------------------------------------------
    }

    // UI
//    setup4mic->showNormal();
//    setup4mic->setWindowState(Qt::WindowActive);

//    this->setWindowState(Qt::WindowMinimized);



    // todo

//    ui->comboBoxModel->set


    // UI 麦克风设定
    setup4mic = new Setup4Mic();

    connect(setup4mic, &Setup4Mic::setupMic, this, &MainWindow::onSetupMic);
    connect(this, &MainWindow::loadMicConf, setup4mic, &Setup4Mic::onLoadDeviceConf);

    emit loadMicConf(m_micIndexL, m_micIndexR);

    // UI 音频测试设定
    setup4autotest = new Setup4AutoTest();
    connect(setup4autotest, &Setup4AutoTest::autoTestConfigChanged, this, &MainWindow::onAutoTestConfigChanged);


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

MainWindow::~MainWindow()
{

    m_recordThread4L.quit();
    m_recordThread4R.quit();

    m_recordThread4L.wait();
    m_recordThread4R.wait();


    delete setup4mic;
    delete ui;

}


void MainWindow::onRecordingOver()
{
    //录制结束
    ui->widgetShowInfo->stopTimer();
    ui->btnStartRecord->setEnabled(true);
    ui->btnStartTest->setEnabled(true);
    log.blue("录制结束");
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

    // Mic
    m_micL = conf.Get("Mic", "L").toString().trimmed();
    m_micR = conf.Get("Mic", "R").toString().trimmed();
    m_micIndexL = conf.Get("Mic", "L_idx").toInt();
    m_micIndexR = conf.Get("Mic", "R_idx").toInt();
    qDebug() << "Load Device Conf L: " << m_micL;
    qDebug() << "Load Device Conf R: " << m_micR;

    emit loadMicConf(m_micIndexL, m_micIndexR);

    if(!m_micL.isEmpty()){
        emit setRecordInputL(m_micL);
    }else{
        log.warn("L 左侧麦克风未设定");
    }
    if(!m_micR.isEmpty()){
        emit setRecordInputR(m_micR);
    }else{
        log.warn("R 右侧麦克风未设定");
    }
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

    //Todo 设置录音输出


    // 录制
    quint64 duration = ui->lineEditDurationOfRecord->text().toUInt();

    // Todo: 输入限制
    emit startRecording(duration);
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

void MainWindow::onAutoTestConfigChanged()
{// 自动测试设定变更
    if(!setup4autotest){
        qDebug() << "UI (Setup4AutoTest) 未载入！";
        return ;
    }
//    setup4autotest->m_duration1;
//    setup4autotest->m_duration2;
//    setup4autotest->m_recordDelay;
//    setup4autotest->m_mainWorkDir;

    // do nothing
}

void MainWindow::onSetupMic(int l_idx, const QString &lmic, int r_idx, const QString &rmic)
{
    // 设置立即生效
    this->m_micIndexL = l_idx;
    this->m_micL = lmic;

    this->m_micIndexR = r_idx;
    this->m_micR = rmic;

    log.warn(lmic);
    log.warn(rmic);
    emit setRecordInputL(lmic);
    emit setRecordInputR(rmic);

    // Todo 保存至配置文件
   conf.Set("Mic", "L", lmic);
   conf.Set("Mic", "L_idx", l_idx);
   conf.Set("Mic", "R", rmic);
   conf.Set("Mic", "R_idx", r_idx);

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
    ui->lineEdit4WavDir->setToolTip(wavDir);
    log.warn(wavDir);
}


void MainWindow::on_btnStartRecord_clicked()
{//Start Record
    //Todo
    // 输入限制, 按钮逻辑

    quint64 duration = ui->lineEditDurationOfRecord->text().toUInt();


    // 输出 当前麦克风
    log.info(m_micL);
    log.info(m_micR);


    ui->widgetShowInfo->startTimer();

    QString wavdir = ui->lineEdit4WavDir->text();
    recWorkerL->setOutputFile(wavdir+ "\\L.wav");
    recWorkerR->setOutputFile(wavdir+ "\\R.wav");

    ui->btnStartRecord->setEnabled(false);
    ui->btnStartTest->setEnabled(false);

    log.info("开始录制");
    emit startRecording(duration);
}


void MainWindow::on_btnStartTest_clicked()
{
    // 载入指定目录下 L.wav R.wav文件

    textedit4log.clear();

    ui->btnStartRecord->setEnabled(false);
    ui->btnStartTest->setEnabled(false);

    QString workdir = ui->lineEdit4WavDir->text().trimmed();
    QString wavL =  QDir::toNativeSeparators(workdir + WAV_FILE_L);
    QString wavR =  QDir::toNativeSeparators(workdir + WAV_FILE_R);


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
    log.warn("dir: "+workdir);
    log.warn("file: "+wavL);
    log.warn("file: "+wavR);

    log.info("载入 L.wav & R.wav 文件成功.");

    log.info("开始测试");
    // call test dll
    emit startTestAudio();
}

void MainWindow::testAudio()
{
    //获取CSV测试结果
    QString target_dir = ui->lineEdit4WavDir->text().trimmed();

    QString app = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "\\AudioTest\\ConsoleAppAudioTest.exe");
    QString cmd = app + " " + target_dir;

    cmd += " " +  QString::number(setup4autotest->m_duration1);
    cmd += " " +  QString::number(setup4autotest->m_duration2);

    qDebug() << app;
    qDebug() << cmd;
    log.warn(app);
    log.warn(cmd);


//    system(cmd.toLatin1());
    system_hide((char*)cmd.toLatin1().data());

    ui->btnStartRecord->setEnabled(true);
    ui->btnStartTest->setEnabled(true);
    emit audioTestFinished();
    log.info("...");
}

void MainWindow::onAudioTestFinished()
{
    //已获取CSV测试结果， 开始判断

//    QString csv_file = current_AudioTestDir + "\\test.csv";
    QString csv_file = ui->lineEdit4WavDir->text().trimmed() + "\\test.csv";
    qDebug() << csv_file;
    log.blue(csv_file);

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
    log.blue("----------result----------");
    QString result_l, result_r;
    for(size_t i=0 ; i <L_data.length();++i){
        result_l += L_data.at(i) + "  ";
    }
    for(size_t i=0 ; i <R_data.length();++i){
        result_r += R_data.at(i) + "  ";
    }
    qDebug() << result_l;
    qDebug() << result_r;

    log.blue(result_l);
    log.blue(result_r);

    double llevel1, rlevel1, llevel2, rlevel2;
    double lpitch1, rpitch1, lpitch2, rpitch2;
    llevel1 = L_data.at(1).toDouble();
    rlevel1 = R_data.at(1).toDouble();
    llevel2 = L_data.at(3).toDouble();
    rlevel2 = R_data.at(3).toDouble();
    lpitch1 = L_data.at(2).toDouble();
    rpitch1 = R_data.at(2).toDouble();
    lpitch2 = L_data.at(4).toDouble();
    rpitch2 = R_data.at(4).toDouble();

    /*
     * 1. 有无不响的情况
     * 2. 有无左右放置错误的情况
     *
     * 时段1，2 的频率分别是多少？
     * 左侧和右侧哪个先响?
    // 前提先左响， 后右响

    // 播放顺序是否正常， 根据频率判断
    // 时段1 频率是不是 1K, 是则正常
    lpitch1 == rpitch1 == 10000;

    // 时段2 频率是不是 2K, 是则正常
    lpitch2 == rpitch2 == 20000;


    // 左右是否正常放置

    //时段1,左侧强度>右侧强度, 大于则正常
    llevel1 > rlevel1;

    //时段2,左侧强度<右侧强度, 小于则正常
    llevel2 < rlevel2;
    */
    quint64 accept_pitch1[2] = {900,1100};
    quint64 accept_pitch2[2] = {1900,2100};

    // 左侧 时段1
    if(lpitch1 > accept_pitch1[0] && lpitch1 < accept_pitch1[1]){
        log.info("左侧频率 时段1 正常");
    }else{
        goto ERROR_L;
    }
    // 左侧 时段2
    if(lpitch2 > accept_pitch2[0] && lpitch2 < accept_pitch2[1]){
        log.info("左侧频率 时段2 正常");
    }else{
        goto ERROR_L;
    }
    // 右侧 时段1
    if(rpitch1 > accept_pitch1[0] && rpitch1 < accept_pitch1[1]){
        log.info("右侧频率 时段1 正常");
    }else{
        goto ERROR_R;
    }
    // 右侧 时段2
    if(rpitch2 > accept_pitch2[0] && rpitch2 < accept_pitch2[1]){
        log.info("右侧频率 时段2 正常");
    }else{
        goto ERROR_R;
    }

    if(llevel1 > rlevel1){// 1K
        log.info("时段1 左侧强度>右侧强度 正常");
    }else{
        goto ERROR_BOTH;
    }
    if(llevel2 < rlevel2){// 2K
        log.info("时段2 左侧强度<右侧强度 正常");
    }else{
        goto ERROR_BOTH;
    }

PASS:

    ui->widgetShowInfo->okNumPlusOne();
    log.info("测试通过");
    return;

ERROR_L:
    ui->widgetShowInfo->ngNumPlusOne();
    log.warn("异常， 请检查 左侧扬声器！");
    return;

ERROR_R:
    ui->widgetShowInfo->ngNumPlusOne();
    log.warn("异常， 请检查 右侧扬声器！");
    return;

ERROR_BOTH:
    ui->widgetShowInfo->ngNumPlusOne();
    log.warn("异常， 请检查 [左+右]两侧扬声器！");
    return;

}


void MainWindow::on_btnSetting4Model_clicked()
{//机种管理


}


void MainWindow::on_btnSetting4AutoTest_clicked()
{//测试设定
    if(!setup4autotest){
        qDebug() << "UI (Setup4AutoTest) 未载入！";
        return ;
    }
    setup4autotest->show();
}


void MainWindow::on_btnLoadTempWavDir_clicked()
{
    //载入 音频测试的临时目录 default_workdir/temp/test
    ui->lineEdit4WavDir->setText(default_AudioTestDir);
    ui->lineEdit4WavDir->setToolTip(default_AudioTestDir);
    log.warn(default_AudioTestDir);
}

void MainWindow::on_btnSwitchRunningMode_clicked()
{
    if(ui->radioButton_autoMode->isChecked()){
        ui->radioButton_manualMode->setChecked(true);
    }else{
        ui->radioButton_autoMode->setChecked(true);
    }
}

