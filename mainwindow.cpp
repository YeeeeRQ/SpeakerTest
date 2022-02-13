#include <QDateTime>
#include <QScrollBar>
#include <numeric>
#include <typeinfo>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#define WAV_FILE_L  "/L.wav"
#define WAV_FILE_R  "/R.wav"

// Todo:
// 程序启动时打开外部设备串口

// --------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,log(SimpleLog::getInstance(&textedit4log))
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 载入 自动测试流程
    this->loadAutoProcess();
    return;

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

//    connect(this, &MainWindow::allRecordOver, this, &MainWindow::startTestAudio);

    // 自定义测试流程
    connect(this, &MainWindow::custom_cmd_done, this, &MainWindow::customTestAudio);

    // 自定义测试流程 录制结束动作
    connect(this, &MainWindow::allRecordOver, this, &MainWindow::custom_do_record_done);

    connect(this, &MainWindow::parseCmd, this, &MainWindow::customCmdParser);


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

        m_pRecWorkerL = new RecordWorker;
        m_pRecWorkerL->moveToThread(&m_recordThread4L);


        // 录音前提：设定好2个麦克风输入
        connect(this, SIGNAL(sig_startRecording(quint64)), m_pRecWorkerL, SLOT(doWork(quint64)));
        connect(&m_recordThread4L, &QThread::finished, m_pRecWorkerL, &QObject::deleteLater);
        connect(m_pRecWorkerL, SIGNAL(resultReady()), this, SLOT(slot_onLMicRecordingOver()));

        connect(this, &MainWindow::sig_setRecordInputL, m_pRecWorkerL, &RecordWorker::setAudioInput);
        connect(this, &MainWindow::sig_setRecordOutputL, m_pRecWorkerL, &RecordWorker::setOutputFile);

        m_pRecWorkerL->setOutputFile("D:\\Temp\\L.wav");

        // 启动录制线程
        m_recordThread4L.start();

        // -------------------------------------------------------------------------

        m_pRecWorkerR = new RecordWorker;
        m_pRecWorkerR->moveToThread(&m_recordThread4R);

        connect(this, SIGNAL(sig_startRecording(quint64)), m_pRecWorkerR, SLOT(doWork(quint64)));
        connect(&m_recordThread4L, &QThread::finished, m_pRecWorkerR, &QObject::deleteLater);
        connect(m_pRecWorkerR, SIGNAL(resultReady()), this, SLOT(slot_onRMicRecordingOver()));

        connect(this, &MainWindow::sig_setRecordInputR, m_pRecWorkerR, &RecordWorker::setAudioInput);
        connect(this, &MainWindow::sig_setRecordOutputR, m_pRecWorkerR, &RecordWorker::setOutputFile);

        m_pRecWorkerR->setOutputFile("D:\\Temp\\R.wav");
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

// --------------------------------------------------------------------------

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

// --------------------------------------------------------------------------

void MainWindow::emitUILoaded()
{
    emit uiLoaded();
}

void MainWindow::onUILoaded()
{
    this->uiInit(); //

    QString autoline_com = conf.Get("AutoLine", "Com").toString();
    qint32 autoline_baud = conf.Get("AutoLine", "Baud").toInt();

    QString readesn_com = conf.Get("ReadEsn", "Com").toString();
    qint32 readesn_baud = conf.Get("ReadEsn", "Baud").toInt();

    QString siggen_com = conf.Get("PG", "Com").toString();
    qint32 siggen_baud = conf.Get("PG", "Baud").toInt();

// 连接外部设备

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

    // 连接 SigGenerator
    if(m_SigGenerator){
        bool open = m_SigGenerator->connectDevice(siggen_com, siggen_baud);
        if(open){
            log.info("SigGeneratro COM 打开.");
        }else{
            log.info("SigGeneratro COM 打开失败.");
        }
    }
}

// 运行模式 （手动|自动）-------------------------------------------------------
bool MainWindow::setAutoMode(bool mode)
{
    emit sig_autoModeStateChanged(mode);
    return this->m_autoMode = mode;
}

bool MainWindow::isAutoMode()
{
    return this->m_autoMode;
}

// --------------------------------------------------------------------------
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

// 两麦克风都录制结束时
void MainWindow::onCheckAllRecordOver()
{
    if(m_recordCount[0] == true &&
       m_recordCount[1] == true){
        log.warn("录制流程结束.");
        emit allRecordOver();
    }
}


// --------------------------------------------------------------------------

void MainWindow::initConfig()
{
    // AutoLine
    conf.Set("AutoLine","Com","COM1");
    conf.Set("AutoLine","Baud",9600);
    conf.Set("AutoLine","Delay",100);
    conf.Set("AutoLine","StartCmd","START");
    conf.Set("AutoLine","PassCmd","PASS");
    conf.Set("AutoLine","FailCmd","PASS");
    conf.Set("AutoLine","SendCount",2);
    conf.Set("AutoLine","SendSpanTime",200);

    // ReadEsn
    conf.Set("ReadEsn","Com","COM2");
    conf.Set("ReadEsn","Baud",9600);

    // PG
    conf.Set("PG","Enable",true);
    conf.Set("PG","Com","COM3");
    conf.Set("PG","Baud",9600);

    // MNT
//    conf.Set("MNT","Enable",true);
//    conf.Set("MNT","Com","COM3");
//    conf.Set("MNT","Baud",9600);

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

// --------------------------------------------------------------------------
void MainWindow::loadAutoProcess()
{
    QString process_file = QCoreApplication::applicationDirPath() + "\\process.xls";
    QFileInfo fi(process_file);
    if(!fi.isFile()){
        log.warn("process.xls 文件不存在！");
        return;
    }

    m_processTable = loadExcel("Sheet1");
    QVector<QString>row0 = m_processTable[0];
    m_processTable_rows = m_processTable.size();
    m_processTable_cols =  row0.size();
    qDebug() << "Table rows:"<< m_processTable_rows;
    qDebug() << "Table cols:"<< m_processTable_cols;
    log.warn("载入测试流程文件 成功");

    return ;

    // Todo：
    // 1. 参数校验
    // 3. 测试流程
    // 2. parser

    // 未知指令？
    // 参数过少？
    // 参数类型? 整型， 字符串
    //

    // PC <--> tester <--> master
    // Speaker
    // Mic
    // PG  信号生成器
    // MNT 监视器

    // 时间单位统一 : ms
    // 频率单位统一 : Hz

    // PC 控制端指令
    // sleep 500    -- PC暂停测试流程，延时等待500ms
    // record 500   -- PC左+右两麦克风开始录制，录制时长500ms,(注意：录制过程中测试流程暂停，不进行下一步操作)

    // set_order L R  -- 告知 左右扬声器播放顺序 先左后右(默认)
    // set_order R L  -- 告知 左右扬声器播放顺序 先右后左

    //                  时刻指针 时刻±范围  频率  频率误差±
    // get_audio_info 1  1500    200     1000   200    -- 立即开始获取音频信息 ，读取工作目录下L1.wav+R1.wav文件 1500ms±200 时段提取音频信息, 结果存放在内存中 ,包含强度level 频率pitch
    // get_audio_info 2  2000    300     2500   300    -- 立即开始获取音频信息 ，读取工作目录下L2.wav+R2.wav文件 2000ms±300 时段提取音频信息, 结果存放在内存中 ,包含强度level 频率pitch

    // autotest_start // 默认第一条, 保留，可不写
    // autotest_end   // 测试流程结束
    // 根据内存中已存储的数据在界面给出结果，并发送pass|fail给AutoLine
    // 测试规则固定。 根据 1. 指定的扬声器播放顺序 2. 强度信息（4个） 3. 频率信息（4个）， 判断扬声器状态 , 两扬声器正常 pass，否则fail
    // (备注，务必保证该指令前已经获取了L1.wav+R1.wav L2.wav+R2.wav 的信息)

    // sendcmd2pg "RUN PATTERN 103;"  -- 指令由用户自定义, 双引号包围

    // sendcmd2mnt "RUN PATTERN 103;"




//    for(int row=1; row<rows;++row){
//        for(int col=1; col<rows;++col){
            // parse
//        }
//    }

    // parser
    QString cmd;
    if(cmd=="sleep"){
        // do sleep
    }
    if(cmd=="record"){
        // do record
    }


    // 解析
}

bool MainWindow::checkCustomTestProcess()
{

    // Todo:

    // check
    // * 不含未知指令
    // * 录制次数 === 2
    // * 必要设定
        // * set_order
        // * get_audio_info 1
        // * get_audio_info 2
    // * get_audio_info 时刻指针 不能越界


//    sleep
//    record
//    sendcmd2pg
//    sendcmd2mnt
//    set_order
//    get_audio_info
//    autotest_start
//    autotest_end






    this->m_customTestProcessIsOK = true;

    return this->m_customTestProcessIsOK;
}

void MainWindow::startCustomTestAudio()
{
    if(!this->m_customTestProcessIsOK){
        log.warn("错误！自定义流程配置有误.");
        log.warn("无法进行测试!");
        return ;
    }

    // Start
    emit custom_cmd_done(false);
}

void MainWindow::custom_do_sleep(quint64 duration)
{
        delaymsec(duration);
        emit custom_cmd_done(false);
}

void MainWindow::custom_do_record(quint64 duration)
{
    emit sig_startRecording(duration);
}

void MainWindow::custom_do_record_done()
{
    emit custom_cmd_done(false);
}

void MainWindow::custom_do_sendcmd2pg(const QString &cmd)
{
    if(m_SigGenerator){
        m_SigGenerator->sendCmd(cmd);
    }
    emit custom_cmd_done(false);
}

void MainWindow::custom_do_sendcmd2mnt(const QString &cmd)
{
    emit custom_cmd_done(false);
}

void MainWindow::custom_do_set_order(const QString & first_speaker)
{
    if(first_speaker == "L"){
        m_firstSpeaker = "L";
    }else if(first_speaker == "R"){
        m_firstSpeaker = "R";
    }else{
        m_firstSpeaker = "L";
        log.warn("Custom Cmd [set order] ERROR");
    }
    log.warn("首次发声麦克风 : " + m_firstSpeaker);

    emit custom_cmd_done(false);
}

void MainWindow::custom_do_get_audio_info(int order, quint64 tick, quint64 tick_range, quint64 freq, quint64 freq_range)
{

    qint64 f1 = freq-freq_range;
    qint64 f2 = freq+freq_range;

    qint64 t1 = tick-tick_range;
    qint64 t2 = tick+tick_range;

    if(1 == order){
        //校验
        if(t1<0 || t2>m_recordDuration1){
            //error
            log.warn("get audio info error!");
            log.warn("取样时间范围大于或小于录制时间！");
        }

        m_testTime1[0] = t1;
        m_testTime1[1] = t2;
        m_accept_pitch1[0]  = f1;
        m_accept_pitch1[1]  = f2;

        emit custom_cmd_done(false); //参数设定成功

    }else if(2 == order){
        //校验
        if(t1<0 || t2>m_recordDuration2){
            //error
            log.warn("get audio info error!");
            log.warn("取样时间范围大于或小于录制时间！");
        }

        m_testTime2[0] = t1;
        m_testTime2[1] = t2;
        m_accept_pitch2[0]  = f1;
        m_accept_pitch2[1]  = f2;
        emit custom_cmd_done(false); //参数设定成功

    }else{
        //error
        log.warn("get audio info error!");
        log.warn("不存在指定次序录制的音频!");

        emit custom_cmd_done(true); //参数设定失败
    }
}

void MainWindow::custom_do_autotest_end()
{

    // 载入音频
    // 载入csv

    // 判断并输出结果

    // 告知结束流程

    emit custom_cmd_done(false); //测试流程结束
}

void MainWindow::customTestAudio(bool has_err)
{
    static quint64 step = 1;
    static QString cmd;
    static QList<QString> cmd_args;

    // 发生错误， 步骤置1
    if(has_err){
        step = 1;
        return;
    }

    if(step < m_processTable_rows){

        // 读取自定义测试指令
        cmd = m_processTable.at(step).at(1);
        // 读取自定义测试指令参数
        cmd_args.clear();
        for(int i = 2; i<m_processTable_cols ;++i){
            QString arg = m_processTable.at(step).at(i);
            if(arg.isEmpty()) break;
            cmd_args.append(arg);
        }

        // 解析并执行自定义指令
        emit parseCmd(cmd, cmd_args);

        ++step;
    }else{
        cmd.clear();
        cmd_args.clear();
        step = 1; //置为初始位置
    }
}

void MainWindow::customCmdParser(const QString& cmd, const QList<QString>&cmd_args)
{
    // 解析 指令 + 参数 并执行.

    if(cmd == "autotest_start"){
        // do nothing

    }else if(cmd == "sleep"){

        quint64 duration = cmd_args.at(0).toUInt();
        custom_do_sleep(duration);

    }else if(cmd == "record"){

        quint64 duration = cmd_args.at(0).toUInt();
        custom_do_record(duration);

    }else if(cmd == "sendcmd2pg"){

        custom_do_sendcmd2pg(cmd_args.at(0));

    }else if(cmd == "sendcmd2mnt"){

        custom_do_sendcmd2mnt(cmd_args.at(0));

    }else if(cmd == "set_order"){

        custom_do_set_order(cmd_args.at(0));

    }else if(cmd == "get_audio_info"){

        custom_do_get_audio_info(
                    cmd_args.at(0).toInt(),
                    cmd_args.at(1).toUInt(),
                    cmd_args.at(2).toUInt(),
                    cmd_args.at(3).toUInt(),
                    cmd_args.at(4).toUInt()
                    );

    }else if(cmd == "autotest_end"){

        // 假定参数设定完毕， 音频录制完毕
        // 判断并输出结果
        custom_do_autotest_end();

    }else{

        qDebug() << "未知指令";
        log.warn("未知指令");

    }
}

// --------------------------------------------------------------------------


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


// --------------------------------------------------------------------------
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
    QFileInfo fi(QCoreApplication::applicationDirPath() + "\\Config.ini");
    if(!fi.isFile()) initConfig();
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


    // PG
    m_SigGenerator = new AutoLine;
    connect(m_SigGenerator, &AutoLine::connectStatusChanged, this, &MainWindow::slot_onAutoLineConnectStatusChanged);
}


// --------------------------------------------------------------------------

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

// --------------------------------------------------------------------------

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


// --------------------------------------------------------------------------

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
            // 开始自动测试流程(流程自定义)
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

void MainWindow::slot_onSigGeneratorConnectStatusChanged()
{
    if(!m_AutoLine->isConnected()){
        QMessageBox::warning(this, "警告", "SigGenerator连接断开", QMessageBox::Ok);
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
    m_pRecWorkerL->setOutputFile(wavdir+ "\\L.wav");
    m_pRecWorkerR->setOutputFile(wavdir+ "\\R.wav");

    // Start Record
    m_wavDuration = ui->lineEditDurationOfRecord->text().toUInt();

    ui->btnStartRecord->setEnabled(false);
    ui->btnTest->setEnabled(false);

    log.info("开始录制");
    ui->widgetShowInfo->startTimer();
    emit sig_startRecording(m_wavDuration);
}

// --------------------------------------------------------------------------

// 手动模式下开始测试
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

// 自动模式下开始测试
void MainWindow::startTestAudioInAutoMode()
{
    textedit4log.clear();
    QString workdir = ui->lineEdit4WavDir->text().trimmed();

    QString wav1L =  QDir::toNativeSeparators(workdir + "1L.wav");
    QString wav1R =  QDir::toNativeSeparators(workdir + "1R.wav");

    QString wav2L =  QDir::toNativeSeparators(workdir + "2L.wav");
    QString wav2R =  QDir::toNativeSeparators(workdir + "2R.wav");

    if(workdir.isEmpty()){
        log.warn("未指定音频存放目录！");
        return;
    }
    if(!QFile::exists(wav1L) || !QFile::exists(wav1R)){
        log.warn("指定目录下时段1的音频不存在！");
        return;
    }
    if(!QFile::exists(wav2L) || !QFile::exists(wav2R)){
        log.warn("指定目录下时段2的音频不存在！");
        return;
    }

    log.info("载入音频成功.");
    log.info("开始测试");

//    emit sig_startTestAudio();
}

// --------------------------------------------------------------------------

// 手动模式下开始测试->获取音频信息
void MainWindow::slot_testAudio()
{
    //调用ConsoleAppAudioTest.exe 输出CSV测试结果
    QString target_dir = ui->lineEdit4WavDir->text().trimmed();

    QString app = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "\\AudioTest\\ConsoleAppAudioTest.exe");

    QString cmd = app + " " + target_dir;
    cmd += " " +  QString::number(setup4autotest->m_duration1);
    cmd += " " +  QString::number(setup4autotest->m_duration2);

    qDebug() << app;
    qDebug() << cmd;
    log.warn(app);
    log.warn(cmd);

    // system(cmd.toLatin1());
    system_hide((char*)cmd.toLatin1().data());

    ui->btnStartRecord->setEnabled(true);
    ui->btnTest->setEnabled(true);
    emit sig_audioTestFinished();
    log.info("...");
}

// 自动模式下开始测试->获取音频信息
void MainWindow::slot_testAudioInAutoMode()
{
    //调用ConsoleAppAudioTest.exe 输出CSV测试结果
    QString target_dir = ui->lineEdit4WavDir->text().trimmed();

    QString app = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "\\AudioTest\\ConsoleAppAudioTest.exe");

    QString cmd = app + " -a " + target_dir;
    cmd += " " +  QString::number(m_testTime1[0]);
    cmd += " " +  QString::number(m_testTime1[1]);
    cmd += " " +  QString::number(m_testTime2[0]);
    cmd += " " +  QString::number(m_testTime2[1]);

    qDebug() << app;
    qDebug() << cmd;
    log.warn(app);
    log.warn(cmd);

    // system(cmd.toLatin1());
    system_hide((char*)cmd.toLatin1().data());

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
        log.info("请检查目标目录下test.csv文件");
    }

    QStringList L_data= line_l.split(',');
    QStringList R_data= line_r.split(',');
    // 判断 Pass | Fail

    // Todo: 输出好看点

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
    */

    /*
     *  时段1 频率
     *  时段2 频率
     *
     *  时段1 强度等级
     *  时段2 强度等级
     */

    log.info("时段1: ");
    // 时段1 左侧
    if(lpitch1 > m_accept_pitch1[0] && lpitch1 < m_accept_pitch1[1]){
        log.info("左侧频率正常");
    }else{
        goto ERROR_L;
    }

    // 时段1 右侧
    if(rpitch1 > m_accept_pitch1[0] && rpitch1 < m_accept_pitch1[1]){
        log.info("右侧频率正常");
    }else{
        goto ERROR_L;
    }

    log.info("时段2: ");
    // 时段2左侧
    if(lpitch2 > m_accept_pitch2[0] && lpitch2 < m_accept_pitch2[1]){
        log.info("左侧频率正常");
    }else{
        goto ERROR_L;
    }
    // 时段2右侧
    if(rpitch2 > m_accept_pitch2[0] && rpitch2 < m_accept_pitch2[1]){
        log.info("右侧频率正常");
    }else{
        goto ERROR_R;
    }

    if(m_firstSpeaker == "L"){//左侧第一个响
        if(llevel1 > rlevel1){
            log.info("时段1 左侧强 > 右侧弱 正常");
        }else{
        goto ERROR_BOTH;
        }
        if(llevel2 < rlevel2){
            log.info("时段2 左侧弱 < 右侧强 正常");
        }else{
        goto ERROR_BOTH;
        }
    }

    if(m_firstSpeaker == "R"){//右侧第一个响
        if(llevel1 < rlevel1){
            log.info("时段1 左侧弱 < 右侧强 正常");
        }else{
            goto ERROR_BOTH;
        }
        if(llevel2 > rlevel2){
            log.info("时段2 左侧强 > 右侧弱 正常");
        }else{
            goto ERROR_BOTH;
        }
    }

PASS:
    printResult(true , "正常,  测试通过!");
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

// --------------------------------------------------------------------------

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


// --------------------------------------------------------------------------

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

// --------------------------------------------------------------------------

QVector<QVector<QString>> loadExcel(QString strSheetName)
{
    QVector<QVector<QString>> vecDatas;//获取所有数据

    if(strSheetName.contains(".xls")){//兼容老版本
        strSheetName = strSheetName.left(strSheetName.length()-4);
    }

    QString strPath = QCoreApplication::applicationDirPath() + "/process.xls";
    QFile file(strPath);
    if(!file.exists()){
        qWarning() << "CExcelTool::loadExcel 路径错误，或文件不存在,路径为"<<strPath;
        return vecDatas;
    }

    QAxObject *excel = new QAxObject("Excel.Application");//excel应用程序
    excel->dynamicCall("SetVisible(bool)", false); //true 表示操作文件时可见，false表示为不可见
    QAxObject *workbooks = excel->querySubObject("WorkBooks");//所有excel文件
    QAxObject *workbook = workbooks->querySubObject("Open(QString&)", strPath);//按照路径获取文件
    QAxObject * worksheets = workbook->querySubObject("WorkSheets");//获取文件的所有sheet页
    QAxObject * worksheet = worksheets->querySubObject("Item(QString)", strSheetName);//获取文件sheet页
    if(nullptr == worksheet){
        qWarning()<<strSheetName<<"Sheet页不存在。";
        return vecDatas;
    }
    QAxObject * usedrange = worksheet->querySubObject("UsedRange");//有数据的矩形区域

    //获取行数
    QAxObject * rows = usedrange->querySubObject("Rows");
    int nRows = rows->property("Count").toInt();
    if(nRows <= 1){
        qWarning()<<"无数据，跳过该文件";
        return vecDatas;
    }

    //获取列数
    QAxObject * columns = usedrange->querySubObject("Columns");
    int nColumns = columns->property("Count").toInt();


    QVariant var = usedrange->dynamicCall("Value");
    foreach(QVariant varRow,var.toList()){
        QVector<QString> vecDataRow;
        foreach(QVariant var,varRow.toList()){
            vecDataRow.push_back(var.toString());
        }
        vecDatas.push_back(vecDataRow);
    }

    //关闭文件
    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");
    if (excel)
    {
        delete excel;
        excel = NULL;
    }

    return vecDatas;
}

// --------------------------------------------------------------------------
