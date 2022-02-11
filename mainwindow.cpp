#include <QDateTime>
#include <QScrollBar>
#include <numeric>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#define WAV_FILE_L  "/L.wav"
#define WAV_FILE_R  "/R.wav"

////////////////////////////////////////////////////////////////////////////////

// Todo:
// 程序启动时打开外部设备串口
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,log(SimpleLog::getInstance(&textedit4log))
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->themeSetting();
    this->pathSetting();
    this->configSetting();
    this->devicesSetting();

    // Test
    connect(this, &MainWindow::sig_startAutoTest, this, &MainWindow::slot_startAutoTest);
    connect(this, &MainWindow::sig_startTestAudio, this, &MainWindow::slot_testAudio);
    connect(this, &MainWindow::sig_audioTestFinished, this, &MainWindow::slot_onAudioTestFinished);
//    connect(this, &MainWindow::startTesting, this, &MainWindow::);
    connect(this, &MainWindow::sig_autoModeStateChanged, this, &MainWindow::slot_onAutoModeStateChanged);

    connect(this, &MainWindow::checkAllRecordOver, this, &MainWindow::onCheckAllRecordOver);
    connect(this, &MainWindow::allRecordOver, this, &MainWindow::startTestAudio);


    loadConfig();

    ui->lineEditDurationOfRecord->setText(QString::number(m_wavDuration));


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
        connect(this, SIGNAL(sig_startRecording(quint64)), recWorkerL, SLOT(doWork(quint64)));
        connect(&m_recordThread4L, &QThread::finished, recWorkerL, &QObject::deleteLater);
        connect(recWorkerL, SIGNAL(resultReady()), this, SLOT(slot_onLMicRecordingOver()));

        connect(this, &MainWindow::sig_setRecordInputL, recWorkerL, &RecordWorker::setAudioInput);
        connect(this, &MainWindow::sig_setRecordOutputL, recWorkerL, &RecordWorker::setOutputFile);

        recWorkerL->setOutputFile("D:\\Temp\\L.wav");

        // 启动录制线程
        m_recordThread4L.start();

        // -------------------------------------------------------------------------

        recWorkerR = new RecordWorker;
        recWorkerR->moveToThread(&m_recordThread4R);

        connect(this, SIGNAL(sig_startRecording(quint64)), recWorkerR, SLOT(doWork(quint64)));
        connect(&m_recordThread4L, &QThread::finished, recWorkerR, &QObject::deleteLater);
        connect(recWorkerR, SIGNAL(resultReady()), this, SLOT(slot_onRMicRecordingOver()));

        connect(this, &MainWindow::sig_setRecordInputR, recWorkerR, &RecordWorker::setAudioInput);
        connect(this, &MainWindow::sig_setRecordOutputR, recWorkerR, &RecordWorker::setOutputFile);

        recWorkerR->setOutputFile("D:\\Temp\\R.wav");
        //    recWorkerR->setAudioInput(audioInputs.begin());

        // 启动录制线程
        m_recordThread4R.start();

        // -------------------------------------------------------------------------
    }

    // UI 麦克风设定
    setup4mic = new Setup4Mic();

    connect(setup4mic, &Setup4Mic::setupMic, this, &MainWindow::slot_onSetupMic);
    connect(this, &MainWindow::sig_loadMicConf, setup4mic, &Setup4Mic::onLoadDeviceConf);
    emit sig_loadMicConf(m_micIndexL, m_micIndexR);

    // UI 音频测试设定
    setup4autotest = new Setup4AutoTest();
    connect(setup4autotest, &Setup4AutoTest::autoTestConfigChanged, this, &MainWindow::slot_onAutoTestConfigChanged);


    // UI载入结束
    connect(this, &MainWindow::uiLoaded, this, &MainWindow::onUILoaded);
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton result = \
        QMessageBox::question(this, "确认",  "确认退出！",  \
        QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
    if(result == QMessageBox::Yes){

        //程序退出时保存当前设定的延时
        conf.Set("Audio", "RecordDuration", m_wavDuration);
        if(setup4autotest) setup4autotest->close();
        if(setup4mic) setup4mic->close();

        event->accept();
    }else{
        event->ignore();
    }
}

void MainWindow::emitUILoaded()
{
    emit uiLoaded();
}

void MainWindow::onUILoaded()
{
    this->uiInit();

    QString autoline_com = conf.Get("AutoLine", "Com").toString();
    qint32 autoline_baud = conf.Get("AutoLine", "Baud").toInt();

    QString readesn_com = conf.Get("ReadEsn", "Com").toString();
    qint32 readesn_baud = conf.Get("ReadEsn", "Baud").toInt();

    // 连接 AutoLine
    if(m_AutoLine){
        bool open = m_AutoLine->connectDevice(autoline_com, autoline_baud);
        if(open){
            log.info("AutoLine COM 打开.");
        }else{
            log.info("AutoLine COM 打开失败.");
        }
    }

    // 连接 ReadEsn
    if(m_CodeReader){
        bool open = m_CodeReader->connectDevice(readesn_com, readesn_baud);
        if(open){
            log.info("ReadEsn COM 打开.");
        }else{
            log.info("ReadEsn COM 打开失败.");
        }
    }
}


void MainWindow::slot_onLMicRecordingOver()
{ //录制结束 L
    ui->widgetShowInfo->stopTimer();
    ui->btnStartRecord->setEnabled(true);
    ui->btnTest->setEnabled(true);

    m_recordCount[0] = true;
    log.blue("录制结束 L");
    emit checkAllRecordOver();
}

void MainWindow::slot_onRMicRecordingOver()
{ //录制结束 R
    ui->widgetShowInfo->stopTimer();
    ui->btnStartRecord->setEnabled(true);
    ui->btnTest->setEnabled(true);

    m_recordCount[1] = true;
    log.blue("录制结束 R");
    emit checkAllRecordOver();
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
    m_wavDuration  = conf.Get("Audio", "RecordDuration").toUInt();

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

    emit sig_loadMicConf(m_micIndexL, m_micIndexR);

    if(!m_micL.isEmpty()){
        emit sig_setRecordInputL(m_micL);
    }else{
        log.warn("L 左侧麦克风未设定");
    }
    if(!m_micR.isEmpty()){
        emit sig_setRecordInputR(m_micR);
    }else{
        log.warn("R 右侧麦克风未设定");
    }
}

void MainWindow::resetConfig()
{

}

void MainWindow::onCheckAllRecordOver()
{
    if(m_recordCount[0] == true &&
       m_recordCount[1] == true){
        log.warn("录制流程结束.");
        emit allRecordOver(); //开始自动测试
    }
}

// 注意安全
bool MainWindow::setAutoMode(bool mode)
{
    emit sig_autoModeStateChanged(mode);
    return this->m_autoMode = mode;
}

bool MainWindow::isAutoMode()
{
    return this->m_autoMode;
}

void MainWindow::slot_onAutoModeStateChanged(bool mode)
{
    if(mode == true){
        //自动模式

        // 禁用界面上测试模块
        const static bool enable = false;
        ui->groupBox4AppSetting->setEnabled(enable);
        ui->groupBox4SelectModel->setEnabled(enable);
        ui->groupBox4Test->setEnabled(enable);
    }else{
        //手动模式

        // 启用界面上测试模块
        const static bool enable = true;
        ui->groupBox4AppSetting->setEnabled(enable);
        ui->groupBox4SelectModel->setEnabled(enable);
        ui->groupBox4Test->setEnabled(enable);
    }
}

void MainWindow::themeSetting()
{
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
}

void MainWindow::pathSetting()
{
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

    // 主要输出目录检测 m_outputDir
    if(m_outputDir.isEmpty()){
        log.warn("异常！ 主要输出目录未设定。");
    }
    QDir outdir(m_outputDir);
    if(!outdir.exists()){
        bool ismkdir = outdir.mkdir(m_outputDir);
        if(!ismkdir){
            log.warn("创建"+m_outputDir+"失败!");
            log.warn("程序无法正确运行");
        }else{
            log.info("创建"+m_outputDir+"成功!");
        }
    }
}

void MainWindow::configSetting()
{
    // Config
    bool hasConf = QFile::exists(QCoreApplication::applicationDirPath()
                                 + "\\Config.ini");
    if(!hasConf) initConfig();
}

void MainWindow::devicesSetting()
{
    // 外部设备连接  1. AutoLine 2. 读码器
    // AutoLine
    m_AutoLine = new AutoLine;
    connect(m_AutoLine, &AutoLine::receiveCmd, this, &MainWindow::slot_onAutoLineReceiveCmd);
    connect(m_AutoLine, &AutoLine::connectStatusChanged, this, &MainWindow::slot_onAutoLineConnectStatusChanged);

    // 读码器
    m_CodeReader = new CodeReader;
    m_CodeReader->setLogger(&log);
    connect(m_CodeReader, &CodeReader::receiveBarcode, this, &MainWindow::slot_onCodeReaderReceiveBarcode);
    connect(m_CodeReader, &CodeReader::connectStatusChanged, this, &MainWindow::slot_onCodeReaderConnectStatusChanged);

}

void MainWindow::uiInit()
{
    // 设定程序窗口标题
    this->setWindowTitle(tr("双通道扬声器测试"));

    // 打印启动时间
    QDateTime cur = QDateTime::currentDateTime();
    ui->statusbar->showMessage("启动时间 : " +
                               cur.toString("yyyy-MM-dd hh:mm:ss"), 0);

    // TextEdit log 输出设定
    textedit4log.setEnabled(false);
    textedit4log.verticalScrollBar()->hide();
    ui->verticalLayout4log->addWidget(&textedit4log);
}

///////////////////////////////////////////////////////////////////
/*
1. 机种名 XMXXXAFED
2. 日期  2022-02-01
3. 产品ID+测试时间+（OK|NG）

* 匿名机种 类型 anonymous
* 匿名产品 ID

*/
///////////////////////////////////////////////////////////////////
void MainWindow::slot_startAutoTest()
{

    QString model_name = ui->comboBoxModelName->currentText();
    QString product_id = ui->lineEdit4ProductID->text();

    QString curDate = QDate::currentDate().toString("yyyy-MM-dd");
    QString curTime = QTime::currentTime().toString("hh:mm:ss");

//    QString testResult;  // OK | NG

    // 延时
    delaymsec(m_AutoTestDelay);

    // 录制
    m_wavDuration = ui->lineEditDurationOfRecord->text().toUInt();

    log.info("开始录制");

    ui->widgetShowInfo->startTimer();
    emit sig_startRecording(m_wavDuration);
}


void MainWindow::slot_onCodeReaderReceiveBarcode(QString barcode)
{
    log.info("Received Barcode: "+barcode);
    ui->lineEdit4ProductID->setText(barcode);
}

void MainWindow::slot_onAutoLineReceiveCmd(QString rev_cmd)
{
    qDebug() << "Received Command: " << rev_cmd;
    log.info("Received Command: "+rev_cmd);

    if(!this->isAutoMode()){
        log.info("当前模式: 手动");
        return ;
    }else{
        if(rev_cmd == m_cmd_start){
            // 开始自动测试流程
            log.warn("[AutoMode]: 开始测试流程.");
            emit sig_startAutoTest();
        }
    }
}

void MainWindow::slot_onAutoTestConfigChanged()
{// 自动测试设定变更
    if(!setup4autotest){
        qDebug() << "UI (Setup4AutoTest) 未载入！";
        return ;
    }
    // do nothing
}

void MainWindow::slot_onAutoLineConnectStatusChanged()
{
    if(!m_AutoLine->isConnected()){
        QMessageBox::warning(this, "警告", "AutoLine连接断开", QMessageBox::Ok);
    }
}

void MainWindow::slot_onCodeReaderConnectStatusChanged()
{

    if(!m_CodeReader->isConnected()){
        QMessageBox::warning(this, "警告", "ReadEsn连接断开", QMessageBox::Ok);
    }
}

void MainWindow::slot_onSetupMic(int l_idx, const QString &lmic, int r_idx, const QString &rmic)
{
    // 设置立即生效
    this->m_micIndexL = l_idx;
    this->m_micL = lmic;

    this->m_micIndexR = r_idx;
    this->m_micR = rmic;

    log.warn(lmic);
    log.warn(rmic);
    emit sig_setRecordInputL(lmic);
    emit sig_setRecordInputR(rmic);

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



void MainWindow::on_btnLoadWavDir_clicked()
{
    this->m_wavDir = QFileDialog::getExistingDirectory();
    ui->lineEdit4WavDir->setText(m_wavDir);
    ui->lineEdit4WavDir->setToolTip(m_wavDir);
    log.warn(m_wavDir);
}

void MainWindow::on_btnStartRecord_clicked()
{//Start Record
    // 输出 当前麦克风
    log.info(m_micL);
    log.info(m_micR);
    QString wavdir = ui->lineEdit4WavDir->text();
    recWorkerL->setOutputFile(wavdir+ "\\L.wav");
    recWorkerR->setOutputFile(wavdir+ "\\R.wav");

    // Start Record
    m_wavDuration = ui->lineEditDurationOfRecord->text().toUInt();

    ui->btnStartRecord->setEnabled(false);
    ui->btnTest->setEnabled(false);

    log.info("开始录制");
    ui->widgetShowInfo->startTimer();
    emit sig_startRecording(m_wavDuration);
}
void MainWindow::startTestAudio()
{

    // 载入指定目录下 L.wav R.wav文件

    textedit4log.clear();

    ui->btnStartRecord->setEnabled(false);
    ui->btnTest->setEnabled(false);

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

    emit sig_startTestAudio();
}

void MainWindow::slot_testAudio()
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
    ui->btnTest->setEnabled(true);
    emit sig_audioTestFinished();
    log.info("...");
}

void MainWindow::slot_onAudioTestFinished()
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
    printResult(true, "测试通过!");
    return;

ERROR_L:
    printResult(false, "异常， 请检查 左侧扬声器！");
    return;

ERROR_R:
    printResult(false, "异常， 请检查 右侧扬声器！");
    return;

ERROR_BOTH:
    printResult(false, "异常， 请检查 [左+右]两侧扬声器！");
    return;
}

void MainWindow::printResult(bool isOk, const QString& msg)
{
    qDebug() << "pass:"<<m_cmd_pass;
    qDebug() << "fail:"<<m_cmd_fail;
    if(isOk){
        ui->widgetShowInfo->okNumPlusOne();
        m_AutoLine->sendCmd(m_cmd_pass);
        log.info(msg);
    }else{
        ui->widgetShowInfo->ngNumPlusOne();
        m_AutoLine->sendCmd(m_cmd_fail);
        log.warn(msg);
    }
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
    setup4autotest->setHidden(true);
    setup4autotest->show();
}

void MainWindow::on_btnSetting4Mic_clicked()
{
    if(!setup4mic){
        qDebug() << "UI (Setup4Mic) 未载入！";
        return ;
    }
    setup4mic->setHidden(true);
    setup4mic->show();
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
        setAutoMode(false);
        ui->radioButton_manualMode->setChecked(true);
    }else{
        setAutoMode(true);
        ui->radioButton_autoMode->setChecked(true);
    }
}


void MainWindow::on_btnTest_clicked()
{
    this->startTestAudio();
}

