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

    auto* durationRangeValidator = new QIntValidator(100, 300,this);
    ui->lineEdit_durationRange1->setValidator(durationRangeValidator);
    ui->lineEdit_durationRange2->setValidator(durationRangeValidator);

    auto* freqValidator = new QIntValidator(-5000,5000, this);
    ui->lineEdit_Frequency1->setValidator(freqValidator);
    ui->lineEdit_Frequency2->setValidator(freqValidator);

    ui->lineEdit_FirstFreq->setValidator(freqValidator);
    ui->lineEdit_FirstFreqRange->setValidator(durationRangeValidator);

    auto * timeoutValidator = new  QIntValidator(0, 300000,this);
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

    quint64 range = ui->lineEdit_durationRange1->text().toUInt();
    QString s = (m_duration1 > range?QString::number(m_duration1 - range):"0") + "-" +QString::number(m_duration1+range);
    ui->label_duration1->setText(s);

    range = ui->lineEdit_durationRange2->text().toUInt();
    s = (m_duration2 > range?QString::number(m_duration2 - range):"0") + "-" +QString::number(m_duration2+range);
    ui->label_duration2->setText(s);

    m_recordDelay = conf.Get("AutoTest", "RecordDelay").toUInt();

    ui->lineEdit_mainWorkDir->setText(m_mainWorkDir);
    ui->lineEdit_mainWorkDir->setToolTip(m_mainWorkDir);

    ui->lineEdit_duration1->setText(QString::number(m_duration1));
    ui->lineEdit_duration2->setText(QString::number(m_duration2));

    ui->lineEdit_recordDelay->setText(QString::number(m_recordDelay));
}

void Setup4AutoTest::saveConfig4AutoTest()
{

    //    conf.Set("", "", "");
}


void Setup4AutoTest::on_btnApplyAll_clicked()
{
    // 录制延迟
    m_recordDelay = ui->lineEdit_recordDelay->text().toUInt();
    conf.Set("AutoTest", "RecordDelay", m_recordDelay);

    //
    m_duration1= ui->lineEdit_duration1->text().toUInt();
    conf.Set("AutoTest", "duration1", m_duration1);

    quint64 dur_range1 = ui->lineEdit_durationRange1->text().toUInt();
    QString s1 = (m_duration1 > dur_range1?QString::number(m_duration1 - dur_range1):"0") + "-" +QString::number(m_duration1+dur_range1);
    ui->label_duration1->setText(s1);

    m_duration2= ui->lineEdit_duration2->text().toUInt();
    conf.Set("AutoTest", "duration2", m_duration2);

    quint64 dur_range2 = ui->lineEdit_durationRange2->text().toUInt();
    QString s2 = (m_duration2 > dur_range2?QString::number(m_duration2 - dur_range2):"0")+ "-" +QString::number(m_duration2+dur_range2);
    ui->label_duration2->setText(s2);

    emit autoTestConfigChanged();
}


void Setup4AutoTest::on_btnMainWorkDirApply_clicked()
{
    // 主目录
    m_mainWorkDir = QFileDialog::getExistingDirectory();
    ui->lineEdit_mainWorkDir->setText(m_mainWorkDir);
    ui->lineEdit_mainWorkDir->setToolTip(m_mainWorkDir);
    conf.Set("Audio", "Path", m_mainWorkDir);
}

