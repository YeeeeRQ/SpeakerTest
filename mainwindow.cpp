#include <QDateTime>
#include <QScrollBar>
#include <numeric>
#include <typeinfo>
#include <QMessageBox>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#define WAV_FILE_L  "/L.wav"
#define WAV_FILE_R  "/R.wav"

#define CONFIG_FILE "/Config.ini"
#define DATABASE_FILE "/Model.db"

//Todo:

// --------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,log(SimpleLog::getInstance(&textedit4log))
    ,ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->m_firstSpeaker = "L";

    this->setting4Mic();
    this->Setting4Config();
    this->Setting4Path();
    this->Setting4Theme();

    // 外部设备连接  1. AutoLine 2. 读码器 3. PG
    this->Setting4Devices();  // 外部设备连接

    // UI载入结束
    connect(this, &MainWindow::MainWindowLoaded, this, &MainWindow::onMainWindowLoaded);

    // 模式切换 （自动|手动）后UI变更
    connect(this, &MainWindow::sig_autoModeStateChanged, this, &MainWindow::slot_onAutoModeStateChanged);

    connect(this, &MainWindow::recordStatusChanged, this, &MainWindow::onRecordStatusChanged);

    // 载入机种列表
    connect(this, &MainWindow::sig_loadModel, this, &MainWindow::loadModel);

    // 两麦克风输入录制结束检查
    connect(this, &MainWindow::checkAllRecordOver, this, &MainWindow::onCheckAllRecordOver);

    // 读取CSV, 判断结果
    connect(this, &MainWindow::sig_audioTestFinished, this, &MainWindow::slot_onAudioTestFinished);

// Test --------------------------------------------------------------------------------------------
    /*
     * 1. Intercept
     * 2. Recording
     * 3. Get Audio Info
     * 4. Analyse
     * 5. Print Result
     */
    // 自动模式 接收到 AutoLine Start 指令，开始进行测试
    connect(this, &MainWindow::sig_startAutoTest, this, &MainWindow::slot_startAutoTest);

    // 两麦克风输入录制完毕后动作
    connect(this, &MainWindow::allRecordOver, this, &MainWindow::startTestAudio);

    // 音频载入 -> 获取音频信息 并输出至CSV
    connect(this, &MainWindow::sig_startTestAudio, this, &MainWindow::slot_testAudio);

}

MainWindow::~MainWindow()
{
    m_recordThread4L.quit();
    m_recordThread4R.quit();

    m_recordThread4L.wait();
    m_recordThread4R.wait();

    delete setup4model;
    delete setup4autotest;
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
        m_wavDuration = ui->lineEditDurationOfRecord->text().toUInt();
        conf.Set("Audio", "RecordDuration", m_wavDuration);
        if(setup4autotest) setup4autotest->close();
        if(setup4mic) setup4mic->close();

        event->accept();
    }else{
        event->ignore();
    }
}

void MainWindow::clearFileList()
{
    static QStringList list{
        "test.csv",
        "L.wav",
        "R.wav",
        "L_filtered.wav",
        "R_filtered.wav",
    };
    for(int i =0;i<list.size();++i){
        QFile file_temp(m_audioTestDir + "/" +list[i]);

        fi.setFile(file_temp);
        if(fi.isFile()){
            qDebug() << "Delete:" << file_temp;
            file_temp.remove();
        }
    }

}

// --------------------------------------------------------------------------

void MainWindow::emitMainWindowLoaded()
{
    emit MainWindowLoaded();
}

void MainWindow::onMainWindowLoaded()
{
    this->Setting4MainWindow(); //


    QString autoline_com = conf.Get("AutoLine", "Com").toString();
    qint32 autoline_baud = conf.Get("AutoLine", "Baud").toInt();

    QString readesn_com = conf.Get("ReadEsn", "Com").toString();
    qint32 readesn_baud = conf.Get("ReadEsn", "Baud").toInt();

//    QString siggen_com = conf.Get("PG", "Com").toString();
//    qint32 siggen_baud = conf.Get("PG", "Baud").toInt();

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
//    if(m_SigGenerator){
//        bool open = m_SigGenerator->connectDevice(siggen_com, siggen_baud);
//        if(open){
//            log.info("SigGeneratro COM 打开.");
//        }else{
//            log.info("SigGeneratro COM 打开失败.");
//        }
//    }


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
void MainWindow::slot_onLMicRecordingOver(bool done, const QString& result)
{ //录制结束 L
    l_record_done = done;
    l_record_reason = result;

    m_recordCount[0] = true;
    emit checkAllRecordOver();
}

void MainWindow::slot_onRMicRecordingOver(bool done, const QString& result)
{ //录制结束 R
    r_record_done = done;
    r_record_reason = result;
    m_recordCount[1] = true;
    emit checkAllRecordOver();
}

// 两麦克风都录制结束时
void MainWindow::onCheckAllRecordOver()
{
    if(m_recordCount[0] == true &&
       m_recordCount[1] == true){
        m_recordCount[0]= false;
        m_recordCount[1]= false;

        m_isRecording = false;
        emit recordStatusChanged(false);

        log.warn("录制流程结束.");

        if(l_record_done && r_record_done){
            // 正常
            log.warn("结果:成功");
            emit allRecordOver();
        }else{
            // 失败
            log.warn("结果:失败");
            log.warn("L:" + l_record_reason);
            log.warn("R:" + r_record_reason);
            printResult(false, "");
        }
    }
}

void MainWindow::onInterceptTimeout()
{
    // 侦测超时
    log.warn("侦测超时!");
    log.warn("侦测超时!");
}

// 侦听完成 业务逻辑 (备注：仅连接一个线程)
void MainWindow::onInterceptDone()
{
    Q_ASSERT(true == m_isRecording);
    Q_ASSERT(false == m_recordCount[0]);
    Q_ASSERT(false == m_recordCount[1]);

    // 关闭侦听 // 强制闲置录制线程
    m_pRecWorkerL->stopRecord();
    m_pRecWorkerR->stopRecord();

    Q_ASSERT(true == m_pRecWorkerL->isIdle());
    Q_ASSERT(true == m_pRecWorkerR->isIdle());

    // 侦听完成，两线程开始录制(备注：录制参数已在开始侦听前设定完毕)
    m_pRecWorkerL->closeIntercept();
    m_pRecWorkerR->closeIntercept();
    //标志位置位
    m_recordCount[0]= false;
    m_recordCount[1]= false;

    emit recordStatusChanged(m_isRecording);
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
    conf.Set("AutoLine","FailCmd","FAIL");
    conf.Set("AutoLine","SendCount",2);
    conf.Set("AutoLine","SendSpanTime",200);

    // ReadEsn
    conf.Set("ReadEsn","Com","COM3");
    conf.Set("ReadEsn","Baud",9600);

    // PG
//    conf.Set("PG","Enable",true);
//    conf.Set("PG","Com","COM5");
//    conf.Set("PG","Baud",9600);

    // MNT
//    conf.Set("MNT","Enable",true);
//    conf.Set("MNT","Com","COM3");
//    conf.Set("MNT","Baud",9600);

    // DCT
    conf.Set("DCT","Enable",true);
    conf.Set("DCT","Com","COM6");
    conf.Set("DCT","Baud",9600);

    // Audio
    QString audio_path(QCoreApplication::applicationDirPath() + "/Output");
    conf.Set("Audio", "Path", QDir::toNativeSeparators(audio_path));

    // AutoTest
}

void MainWindow::loadConfig()
{
    //从配置文件中载入 主目录+wav输出目录 设定
    QString temp_dir = conf.Get("Audio", "Path").toString();
    if(QFile::exists(temp_dir)){
        m_workDir = conf.Get("Audio", "Path").toString();
        m_audioTestDir = m_workDir + "/temp";
    }else{
        m_workDir.clear();
        m_audioTestDir.clear();
    }

    m_wavDuration  = conf.Get("Audio", "RecordDuration").toUInt();

    m_cmd_start = conf.Get("AutoLine", "StartCmd").toString();
    m_cmd_pass = conf.Get("AutoLine", "PassCmd").toString();
    m_cmd_fail = conf.Get("AutoLine", "FailCmd").toString();

    // Mic
    m_micL = conf.Get("Mic", "L").toString().trimmed();
    m_micR = conf.Get("Mic", "R").toString().trimmed();
    m_micIndexL = conf.Get("Mic", "L_idx").toInt();
    m_micIndexR = conf.Get("Mic", "R_idx").toInt();
    qDebug() << "Load Device Conf L: " << m_micL;
    qDebug() << "Load Device Conf R: " << m_micR;

    emit sig_loadMicConf(m_micIndexL, m_micIndexR);

    if(!m_micL.isEmpty()){
//        emit sig_setRecordInputL(m_micL);
        emit sig_setRecordInputL(m_micIndexL);
//        m_pRecWorkerL->setMic(m_micIndexL);

    }else{
        log.warn("L 左侧麦克风未设定");
    }
    if(!m_micR.isEmpty()){
//        emit sig_setRecordInputR(m_micR);
        emit sig_setRecordInputR(m_micIndexR);
//        m_pRecWorkerR->setMic(m_micIndexR);
    }else{
        log.warn("R 右侧麦克风未设定");
    }
}


void MainWindow::slot_onAutoModeStateChanged(bool mode)
{

    // UI 变化
    bool enable = mode?false:true;

    ui->groupBox4AppSetting->setEnabled(enable);
    ui->groupBox4SelectModel->setEnabled(enable);

//    ui->groupBox4Test->setEnabled(enable);
    ui->btnLoadTempWavDir->setEnabled(enable);
    ui->btnLoadWavDir->setEnabled(enable);
    ui->btnTest->setEnabled(enable);
    ui->groupBox4Record->setEnabled(enable);

    ui->groupBox4ProductID->setEnabled(enable);


    //自动切手动 -> 自动载入临时目录
    if(mode == false){
        this->on_btnLoadTempWavDir_clicked();
    }
}


// --------------------------------------------------------------------------

void MainWindow::Setting4MainWindow()
{


    // 机种载入 数据库文件检测
    QString s_dbfile = QCoreApplication::applicationDirPath() + DATABASE_FILE;
    QFileInfo dbfile(s_dbfile);
    if(!dbfile.isFile()){
        QMessageBox::critical(this,"文件缺失", "机种数据库文件不存在", QMessageBox::Ok);

    }else{
        emit sig_loadModel(s_dbfile);
    }

    // 设定程序窗口标题
    this->setWindowTitle(tr("双通道扬声器测试"));

    // 打印启动时间
    QDateTime cur = QDateTime::currentDateTime();
    label_startUpTime = new QLabel("启动时间 : " + cur.toString("yyyy-MM-dd hh:mm:ss"),this);
    ui->statusbar->addWidget(label_startUpTime);

// QPlainTextEdit log 输出设定
//    textedit4log.setEnabled(false);
//    textedit4log.verticalScrollBar()->hide();
    textedit4log.setReadOnly(true);
    textedit4log.setMinimumWidth(400);
    ui->verticalLayout4log->addWidget(&textedit4log);

    //无边框
    textedit4log.setFrameShape(QFrame::NoFrame);
    //透明
    textedit4log.setStyleSheet(
                "QPlainTextEdit{"
                "background-color:rgba(255,255,200, 0)"
                "}"
                );
    textedit4log.setMaximumBlockCount(150);
    textedit4log.setUndoRedoEnabled(false);
//    setMaximumBlockCount(int)函数控制总行数，并且使用setUndoRedoEnabled(false),可以有效控制内存增长。

    // 初始化默认录制时长
    ui->lineEditDurationOfRecord->setText(QString::number(m_wavDuration));

// 输入验证
//    auto* durationValidator = new QIntValidator(0, 20000,this);
    QIntValidator* durationValidator = new QIntValidator(1000, 20000,this);
    ui->lineEditDurationOfRecord->setValidator(durationValidator);


// UI界面 麦克风设定
    setup4mic = new Setup4Mic();

    connect(setup4mic, &Setup4Mic::setupMic, this, &MainWindow::slot_onSetupMic);
    connect(this, &MainWindow::sig_loadMicConf, setup4mic, &Setup4Mic::onLoadDeviceConf);
    emit sig_loadMicConf(m_micIndexL, m_micIndexR);

// UI界面 音频测试设定
    setup4autotest = new Setup4AutoTest();
    connect(setup4autotest, &Setup4AutoTest::autoTestConfigChanged, this, &MainWindow::slot_onAutoTestConfigChanged);

// UI界面 机种管理
    setup4model = new Setup4Model();
    connect(this, &MainWindow::setup4model_loadDB,setup4model, &Setup4Model::openDB);
    connect(setup4model, &Setup4Model::closeWindow, this, &MainWindow::loadModel);

}

void MainWindow::setWavDir4UI(const QString & dir)
{
    fi.setFile(dir);
    if(fi.isDir() && fi.exists()){
        QString new_dir = QDir::toNativeSeparators(dir);
        ui->lineEdit4WavDir->setText(new_dir);
        ui->lineEdit4WavDir->setToolTip(new_dir);
    }else{
        log.info("WAV输出目录设定有误.");
        ui->lineEdit4WavDir->setText("");
        ui->lineEdit4WavDir->setToolTip("");
    }

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
    static QDir dir;

    QString model_name = ui->comboBoxModelName->currentText();
    QString product_id = ui->lineEdit4ProductID->text();

    if(model_name.isEmpty()){
        model_name = "UNKOWN_MODEL";
    }
    if(product_id.isEmpty()){
        product_id = "DefaultID";
    }

    QString curDate = QDate::currentDate().toString("yyyy-MM-dd");
    QString curTime = QTime::currentTime().toString("hh_mm_ss_zzz");

    // 主目录 + 测试目录 : m_workDir + m_audioTestDir

    // 根据 [机种名] - [日期] - [产品ID + 测试时间] 创建文件夹(单次测试目录)

    // 变更工作目录至新文件夹


    if(m_workDir.isEmpty()){
        log.warn("主目录为空!");
        return;
    }

    fi.setFile(m_workDir);
    dir.setPath(fi.path());
    if(!dir.exists(m_workDir)){
        dir.mkdir(m_workDir);
    }
    if(!fi.isDir()){
        log.warn("主输出目录不存在!");
        return;
    }

    m_audioTestDir = m_workDir;
    m_audioTestDir.append("/" + model_name);
    m_audioTestDir.append("/" + curDate);
    m_audioTestDir.append("/" + curTime+ "__" + product_id);
    m_audioTestDir.append("/");
    qDebug() << "主目录 :" <<m_workDir;
    qDebug() << "测试目录:" <<m_audioTestDir;

    fi.setFile(m_audioTestDir);
    dir.setPath(fi.path());
    if(!dir.exists()){
        mkMutiDir(m_audioTestDir);
    }
    if(!fi.isDir()){
        log.warn("测试目录不存在!");
        return;
    }


    ui->lineEdit4WavDir->setText(m_audioTestDir);
    ui->lineEdit4WavDir->setToolTip(m_audioTestDir);

//    return ; //for debug

    // 开始测试流程

    // 录制时长
    m_wavDuration = ui->lineEditDurationOfRecord->text().toUInt();

//    log.blue("开始录制");
    qDebug() << "录制时长:" << m_wavDuration << "ms";

    ui->widgetShowInfo->startTimer();
//    ui->widgetShowInfo->changeStatus2Recording();

    // 麦克风开始录制
//    emit sig_startRecording();

    this->on_btnStartRecord_clicked();
}

// --------------------------------------------------------------------------

void MainWindow::slot_onCodeReaderReceiveBarcode(QString barcode)
{
    log.info("Received Barcode: "+barcode);
    ui->lineEdit4ProductID->setText(barcode);
}

// 接收到AutoLine发送的指令
void MainWindow::slot_onAutoLineReceiveCmd(QString rev_cmd)
{
    qDebug() << "Received Command: " << rev_cmd;
    log.info("Received Command: "+rev_cmd);

    if(!this->isAutoMode()){
        log.info("当前模式: 手动");
        return ;
    } else{
        if(rev_cmd == m_cmd_start){
            // 开始自动测试流程(流程自定义)
            log.warn("[AutoMode]: 开始测试流程.");
            ui->widgetShowInfo->changeStatus2Start();
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

    // 载入主目录
    m_workDir = conf.Get("Audio", "Path").toString();

    // 载入测试设定
//    m_testTime1_= conf.Get("AudioTest", "duration1").toString();
//    m_testTime1[0] = conf.Get("AudioTest", "duration1").toString().toUInt();
//    m_testTime1[1] = conf.Get("AudioTest", "duration2").toString().toUInt();
//    m_testTime1[1] = conf.Get("AudioTest", "duration2").toString().toUInt();
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

    emit sig_setRecordInputL(l_idx);
    emit sig_setRecordInputR(r_idx);

    conf.Set("Mic", "L", lmic);
    conf.Set("Mic", "L_idx", l_idx);
    conf.Set("Mic", "R", rmic);
    conf.Set("Mic", "R_idx", r_idx);
}

void MainWindow::on_btnLoadWavDir_clicked()
{
    QString temp_dir = QFileDialog::getExistingDirectory();
    fi.setFile(temp_dir);

    if(fi.isDir()){
        m_audioTestDir = temp_dir;
        setWavDir4UI(m_audioTestDir);
    }else{
        // do nothing
        log.warn("载入目录为空.");
    }
}

void MainWindow::on_btnStartRecord_clicked()
{//Start Record

    // 输出 当前麦克风
    log.info("当前输入:");
    log.info(m_micL);
    log.info(m_micR);
    log.info("");

    // 输出文件夹检测
    fi.setFile(m_audioTestDir);
    if(!fi.isDir()){
        log.warn("输出目录未指定!");
        return;
    }

    // 清空输出目录下相关文件
    this->clearFileList();

    ///////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Start Record
    m_wavDuration = ui->lineEditDurationOfRecord->text().toUInt();
    qDebug()<< "录制时长：" << m_wavDuration;

   if(m_wavDuration >= 1000){
       // 录制设定
       //     文件输出
       QString output_l = QDir::toNativeSeparators(m_audioTestDir+ "/L.wav");
       QString output_r = QDir::toNativeSeparators(m_audioTestDir+ "/R.wav");
       m_pRecWorkerL->setRecord(m_wavDuration, output_l);
       m_pRecWorkerR->setRecord(m_wavDuration, output_r);

       // 频率侦测
       bool isOpenIntercept = ui->checkBox_Intercept->isChecked();
       qDebug() << "Open Intercept:" << isOpenIntercept;
       if(isOpenIntercept){

           quint64 freq = setup4autotest->m_interceptFreq;
           quint64 range = setup4autotest->m_interceptFreqRange;
           quint64 timeout= setup4autotest->m_interceptTimeout;

           qDebug() << "Timeout: " << timeout;
           qDebug() << "Freq   : " << freq;
           qDebug() << "Range  : ±" << range;
           m_pRecWorkerL->openIntercept(
                           timeout,
                           freq,
                           range);
           m_pRecWorkerR->openIntercept(
                           timeout,
                           freq,
                           range);
       }else{
           m_pRecWorkerL->closeIntercept();
           m_pRecWorkerR->closeIntercept();
       }
       //标志位置位
       m_recordCount[0]= false;
       m_recordCount[1]= false;

       m_isRecording = true;
       emit recordStatusChanged(m_isRecording);

    }else{
       log.warn("录制时长不能小于1000ms");
       return;
    }
}

// --------------------------------------------------------------------------

// ~~XX模式~~(划掉) 下开始测试
void MainWindow::startTestAudio()
{
    // debug
//    if(m_autoMode){
//        return ;
//    }

    bool fileIsOK= true;

    // 载入指定目录下 L.wav R.wav文件
//    log.clear();


//    QString workdir = ui->lineEdit4WavDir->text().trimmed();
//    QString workdir = m_wavDir;

    QString wavL =  QDir::toNativeSeparators(m_audioTestDir + WAV_FILE_L);
    QString wavR =  QDir::toNativeSeparators(m_audioTestDir + WAV_FILE_R);

    fi.setFile(m_audioTestDir);
    if(!fi.isDir()){
        log.warn("未指定wav文件存放目录！");
        fileIsOK = false;
    }

    fi.setFile(wavL);
    if(!fi.isFile()){
        log.warn("指定目录下 \"L.wav\" 文件不存在！");
        fileIsOK = false;
    }
    fi.setFile(wavR);
    if(!fi.isFile()){
        log.warn("指定目录下 \"R.wav\" 文件不存在！");
        fileIsOK = false;
    }

    if(fileIsOK){
        log.info("载入 L.wav & R.wav 文件成功.");
        log.info("开始测试");
        ui->btnStartRecord->setEnabled(false);
        ui->btnTest->setEnabled(false);

        emit sig_startTestAudio();

    }else{
        ui->widgetShowInfo->changeStatus2Fail();
        log.warn("测试失败!");
        log.warn("dir: "+m_audioTestDir);
        log.warn("file: "+wavL);
        log.warn("file: "+wavR);
    }
}

void MainWindow::onRecordStatusChanged(bool is_recording)
{
    if(is_recording){
       ui->btnStopRecord->setEnabled(true);
       ui->btnStartRecord->setEnabled(false);
       ui->btnTest->setEnabled(false);
       ui->lineEditDurationOfRecord->setEnabled(false);
       ui->checkBox_Intercept->setEnabled(false);

       ui->widgetShowInfo->startTimer();
       ui->widgetShowInfo->changeStatus2Recording();

       emit sig_startRecording();
    }else{

        ui->btnStopRecord->setEnabled(false);
//        ui->btnStartRecord->setEnabled(true);

        if(m_audioInputs.count() < 2){
            ui->btnStartRecord->setEnabled(false); //关闭录制按钮
        }else{
            ui->btnStartRecord->setEnabled(true);
        }

        ui->btnTest->setEnabled(true);
        ui->lineEditDurationOfRecord->setEnabled(true);
        ui->checkBox_Intercept->setEnabled(true);

        ui->widgetShowInfo->stopTimer();
        ui->widgetShowInfo->changeStatus2Done();

        emit sig_stopRecording();
    }
}

// --------------------------------------------------------------------------

// 开始测试->获取音频信息
void MainWindow::slot_testAudio()
{
    //调用ConsoleAppAudioTest.exe 输出CSV测试结果
//    QString target_dir = ui->lineEdit4WavDir->text().trimmed();
    QString target_dir = m_audioTestDir;
    quint64 duration1 = setup4autotest->m_duration1;
    quint64 duration2 = setup4autotest->m_duration2;
    quint64 range1 = setup4autotest->m_duration1range;
    quint64 range2 = setup4autotest->m_duration2range;


    AudioProcess* ap = new AudioProcess();
    ap->setDuration(duration1, range1, duration2, range2);
    ap->setAudioFilePath(m_audioTestDir);
    ap->mainProcess();

//    if (ap->setAudioFilePath(source_path)) {
//    }
//    else {
//        std::cerr << "输入路径有误.";
//        return -3;
//    }

    /*
    QString app = QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/AudioTest/ConsoleAppAudioTest.exe");

    QString cmd = app + " " + target_dir;
    cmd += " " +  QString::number(setup4autotest->m_duration1);
    cmd += " " +  QString::number(setup4autotest->m_duration2);

    qDebug() << app;
    qDebug() << cmd;
//    log.warn(app);
    log.warn(cmd);

    // Todo:时长限制
    // system(cmd.toLatin1());
    system_hide((char*)cmd.toLatin1().data());
    */


    if(m_audioInputs.count() < 2){
        ui->btnStartRecord->setEnabled(false); //关闭录制按钮
    }else{
        ui->btnStartRecord->setEnabled(true);
    }
    ui->btnTest->setEnabled(true);
    emit sig_audioTestFinished();
    log.info("...");
}

void MainWindow::loadModel(QString dbfile)
{

    // 读取指定数据库文件
    // Table : ModelTable
    // col   : ModelName


    QSqlDatabase db;

    if(dbfile.isEmpty())  //选择SQL Lite数据库文件
       return;

    db=QSqlDatabase::addDatabase("QSQLITE"); //添加 SQL LITE数据库驱动
    db.setDatabaseName(dbfile); //设置数据库名称

    if (!db.open())   //打开数据库
    {
        QMessageBox::warning(this, "错误", "打开数据库失败",
                                 QMessageBox::Ok,QMessageBox::NoButton);
        return;
    }
    ui->comboBoxModelName->clear();
    QSqlQuery query(QString("select ModelName from ModelTable"));
    while (query.next())
    {
        QString item = query.value(0).toString();
        ui->comboBoxModelName->addItem(item);
    }
    db.close();

    //setup4model 打开数据库

}

void MainWindow::slot_onGetLFrequency(qint64 freq)
{
    ui->label_LFreq->setText(QString::number(freq));
}

void MainWindow::onRecordWorkerStatusChanged(RecordStatus status)
{
    if(status == RecordStatus::InterceptMode){
        log.blue("侦听");
        ui->widgetShowInfo->changeStatus2Intercept();
    }
    if(status == RecordStatus::RecordingMode){
        log.blue("录制");
        ui->widgetShowInfo->changeStatus2Recording();
    }
}

void MainWindow::slot_onGetRFrequency(qint64 freq)
{
    ui->label_RFreq->setText(QString::number(freq));
}


void MainWindow::slot_onAudioTestFinished()
{
    //已获取CSV测试结果， 开始判断

    //    QString csv_file = current_AudioTestDir + "\\test.csv";
//    QString csv_file = ui->lineEdit4WavDir->text().trimmed() + "\\test.csv";
    QString csv_file = m_audioTestDir+ "/test.csv";
    qDebug() << csv_file;
    log.blue(csv_file);

    if(!QFile::exists(csv_file)){
        printResult(false, "测试目录下无测试结果文件！测试失败！");
        ui->widgetShowInfo->changeStatus2Fail();
        return;
    }

    // 读取csv文件, 输出结果
    QFile f{csv_file};

    if (!f.open(QIODevice::ReadOnly)) {
        printResult(false, "测试结果文件读取失败！");
        ui->widgetShowInfo->changeStatus2Fail();
        return;
    }

    QString line_l, line_r;
    QTextStream in{&f};
    if(!in.atEnd())
        line_l = in.readLine();
    if(!in.atEnd())
        line_r = in.readLine();

    if(line_l.isEmpty() || line_r.isEmpty()){
        printResult(false, "测试结果文件异常！");
        ui->widgetShowInfo->changeStatus2Fail();
        log.info("请检查目标目录下test.csv文件");
        return;
    }

    QStringList L_data= line_l.split(',');
    QStringList R_data= line_r.split(',');

    if(L_data.size()!=5 || R_data.size()!=5){
        printResult(false, "异常， 测试结果不存在");
        ui->widgetShowInfo->changeStatus2Fail();
        log.warn("1. 尝试检查音频信息获取程序是否存在。");
        log.warn("2. 尝试检查设定的音频信息获取时段是否有误。");
        return;
    }

    // 判断 Pass | Fail

    // Todo: 输出好看点

    qDebug() << "----------result----------\n";
    log.info("----------result----------");

    QString result_l, result_r;
    for(size_t i=0 ; i <L_data.length();++i){
        if(L_data.at(i) == "L"){
            result_l += L_data.at(i) + ": ";
        }else{
            double d = L_data.at(i).toDouble();
            result_l += QString::number(d,'f',0) + " ";
        }
    }
    for(size_t i=0 ; i <R_data.length();++i){
        if(R_data.at(i) == "R"){
            result_r += R_data.at(i) + ": ";
        }else{
            double d = R_data.at(i).toDouble();
            result_r += QString::number(d,'f',0) + " ";
        }
    }
    qDebug() << result_l;
    qDebug() << result_r;

    log.red(result_l);
    log.green(result_r);

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

    // 频率范围
    quint64 freq1Range = setup4autotest->m_duration1freqRange;
    quint64 freq2Range = setup4autotest->m_duration2freqRange;
    m_accept_pitch1[0] = setup4autotest->m_duration1freq - freq1Range;
    m_accept_pitch1[1] = setup4autotest->m_duration1freq + freq1Range;
    m_accept_pitch2[0] = setup4autotest->m_duration2freq - freq2Range;
    m_accept_pitch2[1] = setup4autotest->m_duration2freq + freq2Range;

    qDebug() << m_accept_pitch1[0] << " - " << m_accept_pitch1[1];
    qDebug() << m_accept_pitch2[0] << " - " << m_accept_pitch2[1];

    // 时段1 左侧
    if(lpitch1 > m_accept_pitch1[0] && lpitch1 < m_accept_pitch1[1]){
        log.info("✔ 时段1: 左侧频率 正常");
    }else{
        log.warn("✘ 时段1: 左侧频率 异常");
        goto ERROR_L;
    }

    // 时段1 右侧
    if(rpitch1 > m_accept_pitch1[0] && rpitch1 < m_accept_pitch1[1]){
        log.info("✔ 时段1: 右侧频率 正常");
    }else{
        log.warn("✘ 时段1: 右侧频率 异常");
        goto ERROR_R;
    }

    // 时段2左侧
    if(lpitch2 > m_accept_pitch2[0] && lpitch2 < m_accept_pitch2[1]){
        log.info("✔ 时段2: 左侧频率 正常");
    }else{
        log.warn("✘ 时段2: 左侧频率 异常");
        goto ERROR_L;
    }
    // 时段2右侧
    if(rpitch2 > m_accept_pitch2[0] && rpitch2 < m_accept_pitch2[1]){
        log.info("✔ 时段2: 右侧频率 正常");
    }else{
        log.warn("✘ 时段2: 右侧频率 异常");
        goto ERROR_R;
    }

    if(m_firstSpeaker == "L"){//左侧第一个响
        if(llevel1 > rlevel1){
            log.info("✔ 时段1 左侧强 ▇▆▅▄▃▂ 右侧弱 正常");
        }else{
            log.warn("✘ 时段1 左侧弱 ▂▃▄▅▆▇ 右侧强 异常");
            goto ERROR_BOTH;
        }
        if(llevel2 < rlevel2){
            log.info("✔ 时段2 左侧弱 ▂▃▄▅▆▇ 右侧强 正常");
        }else{
            log.warn("✘ 时段2 左侧强 ▇▆▅▄▃▂ 右侧弱 异常");
            goto ERROR_BOTH;
        }
    }

    if(m_firstSpeaker == "R"){//右侧第一个响
        if(llevel1 < rlevel1){
            log.info("✔时段1 左侧弱 ▂▃▄▅▆▇ 右侧强 正常");
        }else{
            log.warn("✘ 时段1 左侧强 ▇▆▅▄▃▂ 右侧弱 异常");
            goto ERROR_BOTH;
        }
        if(llevel2 > rlevel2){
            log.info("✔时段2 左侧强 ▇▆▅▄▃▂ 右侧弱 正常");
        }else{
            log.warn("✘ 时段2 左侧弱 ▂▃▄▅▆▇ 右侧强 异常");
            goto ERROR_BOTH;
        }
    }

PASS:
    printResult(true , "✔ 正常,  测试通过! ");
    return;

ERROR_L:
    printResult(false, "✘ ◧ 异常， 请检查 左侧扬声器！");
    return;

ERROR_R:
    printResult(false, "✘ ◨ 异常， 请检查 右侧扬声器！");
    return;

ERROR_BOTH:
    printResult(false, "✘ ◼ 异常， 请检查 [左+右]两侧扬声器！");
    return;
}

// --------------------------------------------------------------------------

void MainWindow::printResult(bool isOk, const QString& msg)
{
//    qDebug() << "pass:"<<m_cmd_pass;
//    qDebug() << "fail:"<<m_cmd_fail;
    if(isOk){
        QSound::play("://audio/pass.wav");
        ui->widgetShowInfo->changeStatus2Pass();
        ui->widgetShowInfo->okNumPlusOne();
        m_AutoLine->sendCmd(m_cmd_pass);
        log.info(msg);
    }else{
        QSound::play("://audio/fail.wav");
        ui->widgetShowInfo->changeStatus2Fail();
        ui->widgetShowInfo->ngNumPlusOne();
        m_AutoLine->sendCmd(m_cmd_fail);
        log.warn(msg);
    }
}


// --------------------------------------------------------------------------

void MainWindow::Setting4Theme()
{
//    this->resize(QSize(1280,800));
    this->setMinimumSize(QSize(1440,900));
    this->resize(QSize(1440,900));
//    this->setMaximumSize(QSize(1440,900));
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

void MainWindow::Setting4Path()
{
    QDir dir;

    //默认目录设定
    default_WorkDir = QCoreApplication::applicationDirPath() + "\\Output\\";
    default_AudioTestDir = QCoreApplication::applicationDirPath() + "\\Output\\temp";

    // 若配置文件未设定输出目录，则载入默认目录设定
    if(m_workDir.isEmpty()){
        m_workDir =default_WorkDir;
        m_audioTestDir = default_AudioTestDir;
    }
    if(!dir.exists(default_WorkDir)){
        dir.mkdir(default_WorkDir);
    }

    if(!dir.exists(default_AudioTestDir)){
        dir.mkdir(default_AudioTestDir);
    }

    if(!dir.exists(m_workDir)){
        dir.mkdir(m_workDir);
    }

    if(!dir.exists(m_audioTestDir)){
        dir.mkdir(m_audioTestDir);
    }

    fi.setFile(m_audioTestDir);
    if(fi.isDir()){
        setWavDir4UI(m_audioTestDir);
    }else{
        log.warn("Setting4Path: WAV音频存放目录设定失败.");
    }
}

void MainWindow::Setting4Config()
{
    // Config
    QFileInfo fi(QCoreApplication::applicationDirPath() + CONFIG_FILE);
    if(!fi.isFile()) initConfig();
    this->loadConfig();

    // 主要输出目录检测 m_outputDir
    if(m_workDir.isEmpty()){
        log.warn("异常！ 主目录未设定。");
    }else{
        log.info("主目录:"+m_workDir);
    }

    QDir outdir(m_workDir);
    if(!outdir.exists()){
        bool ismkdir = outdir.mkdir(m_workDir);
        if(!ismkdir){
            log.warn("创建"+m_workDir+"失败!");
            log.warn("程序无法正确运行");
        }else{
            log.info("创建"+m_workDir+"成功!");
        }
    }
}

void MainWindow::Setting4Devices()
{
    // 外部设备连接   1. AutoLine 2. 读码器 3. PG

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
//    m_SigGenerator = new AutoLine;
//    connect(m_SigGenerator, &AutoLine::connectStatusChanged, this, &MainWindow::slot_onAutoLineConnectStatusChanged);
}

void MainWindow::setting4Mic()
{
    // 麦克风数量检测
    QAudioRecorder* recorder = new QAudioRecorder(this);
    QStringList  devList= recorder->audioInputs();
    log.info("麦克风输入设备数量："+QString::number(devList.count()));
    m_audioInputs = QSet<QString>(devList.begin(), devList.end());
    foreach (const QString &device, m_audioInputs)
        qDebug() << (device); //音频录入设备列表

    if(m_audioInputs.count() < 2){
        log.warn("麦克风数量小于2, 测试无法进行.");
        ui->btnStartRecord->setEnabled(false); //关闭录制按钮
    }else{

        // 注册新类型
        qRegisterMetaType<RecordStatus>("RecordStatus");

        m_pRecWorkerL = new RecordWorker;
        m_pRecWorkerL->moveToThread(&m_recordThread4L);

        // 录音前提：设定好2个麦克风输入
        connect(this, &MainWindow::sig_startRecording, m_pRecWorkerL, &RecordWorker::startRecord);
        connect(this, &MainWindow::sig_stopRecording, m_pRecWorkerL, &RecordWorker::stopRecord);
        connect(&m_recordThread4L, &QThread::finished, m_pRecWorkerL, &QObject::deleteLater);

        connect(m_pRecWorkerL, &RecordWorker::recordDone, this, &MainWindow::slot_onLMicRecordingOver);

        connect(this, &MainWindow::sig_setRecordInputL, m_pRecWorkerL, &RecordWorker::setMic);

        // -------------------------------------------------------------------------

        m_pRecWorkerR = new RecordWorker;
        m_pRecWorkerR->moveToThread(&m_recordThread4R);

        connect(this, &MainWindow::sig_startRecording, m_pRecWorkerR, &RecordWorker::startRecord);
        connect(this, &MainWindow::sig_stopRecording, m_pRecWorkerR, &RecordWorker::stopRecord);
        connect(&m_recordThread4R, &QThread::finished, m_pRecWorkerR, &QObject::deleteLater);

        connect(m_pRecWorkerR, &RecordWorker::recordDone, this, &MainWindow::slot_onRMicRecordingOver);

        connect(this, &MainWindow::sig_setRecordInputR, m_pRecWorkerR, &RecordWorker::setMic);

        // 启动录制线程
        m_recordThread4L.start();
        m_recordThread4R.start();

        // 侦测频率获取
        connect(m_pRecWorkerL, &RecordWorker::getFrequency, this, &MainWindow::slot_onGetLFrequency);
        connect(m_pRecWorkerR, &RecordWorker::getFrequency, this, &MainWindow::slot_onGetRFrequency);

        // 状态变更
        connect(m_pRecWorkerL, &RecordWorker::statusChanged, this, &MainWindow::onRecordWorkerStatusChanged);

        // 侦听结束
        connect(m_pRecWorkerL, &RecordWorker::interceptDone, this, &MainWindow::onInterceptDone);
    }
}

// --------------------------------------------------------------------------
void MainWindow::on_btnSetting4Model_clicked()
{//机种管理

//    setup4model->setHidden(true);
//    setup4model->show();
    if(!setup4model){
        qDebug() << "UI (Setup4Model) 未载入！";
        return ;
    }

    // 载入数据库文件
    emit setup4model_loadDB(QCoreApplication::applicationDirPath() + DATABASE_FILE);

    setup4model->setWindowModality(Qt::ApplicationModal);
    setup4model->show();
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
    this->m_audioTestDir = default_AudioTestDir;

    fi.setFile(this->m_audioTestDir);
    if(fi.isDir()){
        setWavDir4UI(this->m_audioTestDir);
    }else{
        this->m_audioTestDir.clear();
        log.warn("载入默认WAV目录失败.");
        log.warn(this->m_audioTestDir);
    }
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

// --------------------------------------------------------------------------

void MainWindow::on_btnLockOption4Model_clicked()
{
    static bool lock = false;

    lock = !lock;
    ui->comboBoxModelName->setEnabled(!lock);

    if(lock){
        ui->btnLockOption4Model->setText("解锁");
    }else{
        ui->btnLockOption4Model->setText("锁定");

    //Todo:
    //保存最后一次机种名
    }
}


void MainWindow::on_btnOpenWithExplorer_clicked()
{
    //打开资源管理器并高亮文件
    const QString explorer = "explorer";
    QStringList param;
    if(!QFileInfo(m_audioTestDir).isDir()){
        param<<QLatin1String("/select,");
    }
    param<<QDir::toNativeSeparators(m_audioTestDir);
    QProcess::startDetached(explorer,param);
}

QString mkMutiDir(const QString path){
    QDir dir(path);
    if ( dir.exists(path)){
        return path;
    }
    QString parentDir = mkMutiDir(path.mid(0,path.lastIndexOf('/')));
    QString dirname = path.mid(path.lastIndexOf('/') + 1);
    QDir parentPath(parentDir);
    if ( !dirname.isEmpty() )
        parentPath.mkpath(dirname);
    return parentDir + "/" + dirname;
}

void MainWindow::on_btnStopRecord_clicked()
{
    m_isRecording = false;
//    emit sig_stopRecording();
    emit recordStatusChanged(m_isRecording);
}

