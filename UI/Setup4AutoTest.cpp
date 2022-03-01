#include "Setup4AutoTest.h"
#include "ui_Setup4AutoTest.h"
#include <QFileDialog>
#include <QValidator>

Setup4AutoTest::Setup4AutoTest(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Setup4AutoTest)
{
    ui->setupUi(this);
    this->setWindowTitle("音频测试设定");
    this->loadConfig4AutoTest();

//输入验证
    auto* durationValidator = new QIntValidator(0, 20000,this);
    ui->lineEdit_duration1->setValidator(durationValidator);
    ui->lineEdit_duration2->setValidator(durationValidator);

    auto* durationRangeValidator = new QIntValidator(50, 500,this);
    ui->lineEdit_durationRange1->setValidator(durationRangeValidator);
    ui->lineEdit_durationRange2->setValidator(durationRangeValidator);

    auto* freqValidator = new QIntValidator(0,5000, this);
    ui->lineEdit_Frequency1->setValidator(freqValidator);
    ui->lineEdit_Frequency2->setValidator(freqValidator);
    ui->lineEdit_Frequency1Range->setValidator(durationRangeValidator);
    ui->lineEdit_Frequency2Range->setValidator(durationRangeValidator);

    ui->lineEdit_FirstFreq->setValidator(freqValidator);
    ui->lineEdit_FirstFreqRange->setValidator(durationRangeValidator);

    auto * timeoutValidator = new  QIntValidator(1000, 300000,this);
    ui->lineEdit_InterceptTimeout->setValidator(timeoutValidator);
}

Setup4AutoTest::~Setup4AutoTest()
{
    delete ui;
}

void Setup4AutoTest::loadConfig4AutoTest()
{
    m_mainWorkDir= conf.Get("Audio", "Path").toString();
    m_duration1= conf.Get("AutoTest", "duration1").toUInt();
    m_duration2=conf.Get("AutoTest", "duration2").toUInt();

//    m_recordDelay = conf.Get("AutoTest", "RecordDelay").toUInt();
    m_duration1 = conf.Get("AutoTest", "duration1").toUInt();
    m_duration2 =conf.Get("AutoTest", "duration2").toUInt();
    m_duration1range=conf.Get("AutoTest", "duration1range").toUInt();
    m_duration2range= conf.Get("AutoTest", "duration2range").toUInt();
    m_duration1freq =conf.Get("AutoTest", "duration1freq").toUInt();
    m_duration2freq=conf.Get("AutoTest", "duration2freq").toUInt();

    m_duration1freqRange=conf.Get("AutoTest", "duration2freqRange").toUInt();
    m_duration2freqRange=conf.Get("AutoTest", "duration2freqRange").toUInt();

    m_firstFreq = conf.Get("AutoTest", "firstFreq").toUInt();
    m_firstFreqRange=conf.Get("AutoTest", "firstFreqRange").toUInt();
    m_interceptTimeout = conf.Get("AutoTest", "interceptTimeout").toUInt();

    quint64 range = ui->lineEdit_durationRange1->text().toUInt();
    QString s = (m_duration1 > range?QString::number(m_duration1 - range):"0") + "-" +QString::number(m_duration1+range);
    ui->label_duration1->setText(s);

    range = ui->lineEdit_durationRange2->text().toUInt();
    s = (m_duration2 > range?QString::number(m_duration2 - range):"0") + "-" +QString::number(m_duration2+range);
    ui->label_duration2->setText(s);

    ui->lineEdit_recordDelay->setText(QString::number(m_recordDelay));

    ui->lineEdit_mainWorkDir->setText(m_mainWorkDir);
    ui->lineEdit_mainWorkDir->setToolTip(m_mainWorkDir);

    ui->lineEdit_duration1->setText(QString::number(m_duration1));
    ui->lineEdit_duration2->setText(QString::number(m_duration2));
    ui->lineEdit_durationRange1->setText(QString::number(m_duration1range));
    ui->lineEdit_durationRange2->setText(QString::number(m_duration2range));
    ui->lineEdit_Frequency1->setText(QString::number(m_duration1freq));
    ui->lineEdit_Frequency2->setText(QString::number(m_duration2freq));

    ui->lineEdit_Frequency1Range->setText(QString::number(m_duration1freqRange));
    ui->lineEdit_Frequency2Range->setText(QString::number(m_duration2freqRange));

    ui->lineEdit_FirstFreq->setText(QString::number(m_firstFreq));
    ui->lineEdit_FirstFreqRange->setText(QString::number(m_firstFreqRange));
    ui->lineEdit_InterceptTimeout->setText(QString::number(m_interceptTimeout));
}

void Setup4AutoTest::saveConfig4AutoTest()
{

    //    conf.Set("", "", "");
}


void Setup4AutoTest::on_btnApplyAll_clicked()
{
    // 录制延迟
//    m_recordDelay = ui->lineEdit_recordDelay->text().toUInt();

    //
    m_duration1= ui->lineEdit_duration1->text().toUInt();

    m_duration1range = ui->lineEdit_durationRange1->text().toUInt();
    QString s1 = (m_duration1 > m_duration1range?QString::number(m_duration1 - m_duration1range):"0") +  "-" +QString::number(m_duration1+m_duration1range);
    ui->label_duration1->setText(s1);

    m_duration2= ui->lineEdit_duration2->text().toUInt();

    m_duration2range = ui->lineEdit_durationRange2->text().toUInt();
    QString s2 = (m_duration2 > m_duration2range?QString::number(m_duration2 - m_duration2range):"0")+ "-" +QString::number(m_duration2+m_duration2range);
    ui->label_duration2->setText(s2);

    m_duration1freq = ui->lineEdit_Frequency1->text().toUInt();
    m_duration2freq = ui->lineEdit_Frequency2->text().toUInt();

    m_duration1freqRange = ui->lineEdit_Frequency1Range->text().toUInt();
    m_duration2freqRange = ui->lineEdit_Frequency2Range->text().toUInt();

    m_firstFreq = ui->lineEdit_FirstFreq->text().toUInt();
    m_firstFreqRange = ui->lineEdit_FirstFreqRange->text().toUInt();
    m_interceptTimeout = ui->lineEdit_InterceptTimeout->text().toUInt();

//    conf.Set("AutoTest", "RecordDelay", m_recordDelay);
    conf.Set("AutoTest", "duration1", m_duration1);
    conf.Set("AutoTest", "duration2", m_duration2);
    conf.Set("AutoTest", "duration1range", m_duration1range);
    conf.Set("AutoTest", "duration2range", m_duration2range);
    conf.Set("AutoTest", "duration1freq", m_duration1freq);
    conf.Set("AutoTest", "duration2freq", m_duration2freq);
    conf.Set("AutoTest", "duration1freqRange", m_duration1freqRange);
    conf.Set("AutoTest", "duration2freqRange", m_duration2freqRange);
    conf.Set("AutoTest", "firstFreq", m_firstFreq);
    conf.Set("AutoTest", "firstFreqRange", m_firstFreqRange);
    conf.Set("AutoTest", "interceptTimeout", m_interceptTimeout);

    emit autoTestConfigChanged();
}


void Setup4AutoTest::on_btnMainWorkDirApply_clicked()
{
    // 主目录
    m_mainWorkDir = QFileDialog::getExistingDirectory();
    ui->lineEdit_mainWorkDir->setText(m_mainWorkDir);
    ui->lineEdit_mainWorkDir->setToolTip(m_mainWorkDir);
    conf.Set("Audio", "Path", m_mainWorkDir);
    emit autoTestConfigChanged();
}

