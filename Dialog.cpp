#include "Dialog.h"
#include <QGridLayout>
#include <QApplication>
#include <QShortcut>
#include <QSerialPortInfo>
#include <QIcon>

#define STARTBYTE 0x55
#define STOPBYTE 0xAA
#define BYTESLENTH 8

#define FORMAT 'f'
#define PRECISION 2

#define BLINKTIMETX 200 // ms
#define BLINKTIMERX 500 // ms
#define DISPLAYTIME 100 // ms

#define GAIN_RANGE_MIN 0
#define GAIN_RANGE_MAX 63

#define FREQ_RANGE_MIN 0
#define FREQ_RANGE_MAX 111

#define FREQ_PHASE_SEPARATOR "|"

#define PHASE_RANGE_MIN 0
#define PHASE_RANGE_MAX 360
#define PHASE_STEP 2.25

#define OFFSET_DIGITS 4
#define GAIN_DIGITS 4
#define TEMP_DIGITS 6

#define CODE_RES_MON 0x01
#define CODE_F_STROBE 0x02
#define CODE_GAIN_IQ 0x03

#define CODE_NONE_DATA 0x00

Dialog::Dialog(QWidget *parent) :
        QDialog(parent),
        lPort(new QLabel(QString::fromUtf8("Port"), this)),
        cbPort(new QComboBox(this)),
        lBaud(new QLabel(QString::fromUtf8("Baud"), this)),
        cbBaud(new QComboBox(this)),
        bPortStart(new QPushButton(QString::fromUtf8("Start"), this)),
        bPortStop(new QPushButton(QString::fromUtf8("Stop"), this)),
        lTx(new QLabel("  Tx  ", this)),
        lRx(new QLabel("  Rx  ", this)),
        rbLowFreq(new QRadioButton(QString::fromUtf8("Low Frequency"))),
        rbHighFreq(new QRadioButton(QString::fromUtf8("High Frequency"))),
        sbSetFreq(new QSpinBox(this)),
        sbSetPhase(new QDoubleSpinBox(this)),
        cbSetFStrobe(new QComboBox(this)),
        sbSetGainIQ(new QSpinBox(this)),
        lFreqInfo(new QLabel(QString::fromUtf8("Low Frequency: NONE"), this)),
        lPhaseInfo(new QLabel(QString::fromUtf8("Phase: NONE"), this)),
        lFStrobeInfo(new QLabel(QString::fromUtf8("F-Strobe: NONE"), this)),
        lGainIQInfo(new QLabel(QString::fromUtf8("Gain I, Q: NONE"), this)),
        bSetFreq(new QPushButton(QString::fromUtf8("Set"), this)),
        bSetFStrobe(new QPushButton(QString::fromUtf8("Set"), this)),
        bSetGainIQ(new QPushButton(QString::fromUtf8("Set"), this)),
        gbSetFreq(new QGroupBox(QString::fromUtf8("RES MON"), this)),
        gbSetFStrobe(new QGroupBox(QString::fromUtf8("F-Strobe"), this)),
        gbSetGainIQ(new QGroupBox(QString::fromUtf8("Gain I, Q"), this)),
        gbConfig(new QGroupBox(QString::fromUtf8("Configure"), this)),
        gbInfo(new QGroupBox(QString::fromUtf8("Information"), this)),        
        m_Port(new QSerialPort(this)),
        m_ComPort(new ComPort(m_Port, STARTBYTE, STOPBYTE, BYTESLENTH, true, this)),
        m_Protocol(new DMDProtocol(m_ComPort, this)),
        m_BlinkTimeTxNone(new QTimer(this)),
        m_BlinkTimeRxNone(new QTimer(this)),
        m_BlinkTimeTxColor(new QTimer(this)),
        m_BlinkTimeRxColor(new QTimer(this)),
        m_TimeToDisplay(new QTimer(this))
{
    initIsDataSet();

    lTx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    lTx->setFrameStyle(QFrame::Box);
    lTx->setAlignment(Qt::AlignCenter);
    lTx->setMargin(2);

    lRx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    lRx->setFrameStyle(QFrame::Box);
    lRx->setAlignment(Qt::AlignCenter);
    lRx->setMargin(2);

    QGridLayout *gridFreq = new QGridLayout;
    gridFreq->addWidget(rbLowFreq, 0, 0, 1, 2);
    gridFreq->addWidget(rbHighFreq, 1, 0, 1, 2);
    gridFreq->addWidget(new QLabel(QString::fromUtf8("Frequency"), this), 2, 0);
    gridFreq->addWidget(sbSetFreq, 2, 1);
    gridFreq->addWidget(new QLabel(QString::fromUtf8("Phase"), this), 3, 0);
    gridFreq->addWidget(sbSetPhase, 3, 1);
    gridFreq->addWidget(bSetFreq, 4, 0, 1, 2);

    QGridLayout *gridFStrobe = new QGridLayout;
    gridFStrobe->addWidget(cbSetFStrobe, 0, 0);
    gridFStrobe->addWidget(bSetFStrobe, 1, 0);

    QGridLayout *gridGainIQ = new QGridLayout;
    gridGainIQ->addWidget(sbSetGainIQ, 0, 0);
    gridGainIQ->addWidget(bSetGainIQ, 1, 0);

    gbSetFreq->setLayout(gridFreq);
    gbSetFStrobe->setLayout(gridFStrobe);
    gbSetGainIQ->setLayout(gridGainIQ);

    QGridLayout *gridConfig = new QGridLayout;
    gridConfig->addWidget(gbSetFreq, 0, 0, 5, 2);
    gridConfig->addWidget(gbSetFStrobe, 0, 2, 2, 1);
    gridConfig->addWidget(gbSetGainIQ, 2, 2, 2, 1);

    gbConfig->setLayout(gridConfig);    

    QGridLayout *gridInfo = new QGridLayout;    
    gridInfo->addWidget(lFreqInfo, 0, 0);
    gridInfo->addWidget(lPhaseInfo, 1, 0);
    gridInfo->addWidget(lFStrobeInfo, 2, 0);
    gridInfo->addWidget(lGainIQInfo, 3, 0);
    gridInfo->setAlignment(Qt::AlignLeft);
    gridInfo->setSpacing(5);

    gbInfo->setLayout(gridInfo);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(lPort, 0, 0);
    grid->addWidget(cbPort, 0, 1);
    grid->addWidget(lBaud, 1, 0);
    grid->addWidget(cbBaud, 1, 1);
    grid->addWidget(bPortStart, 0, 2);
    grid->addWidget(bPortStop, 1, 2);
    grid->addWidget(lTx, 0, 3);
    grid->addWidget(lRx, 1, 3);
    // Inserting company logo
    grid->addWidget(new QLabel("<img src=':/Resources/elisat.png' height='40' width='150'/>", this), 0, 4, 2, 2, Qt::AlignRight);
    grid->setSpacing(5);

    QGridLayout *gridAll = new QGridLayout;
    gridAll->addItem(grid, 0, 0, 2, 6);
    gridAll->addWidget(gbConfig, 2, 0, 5, 4);
    gridAll->addWidget(gbInfo, 2, 4, 4, 2);
    gridAll->setSpacing(5);

    setLayout(new QGridLayout(this));
    layout()->addItem(gridAll);
    layout()->setSpacing(5);

    // made window of app fixed size
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    QStringList portsNames;

    foreach(QSerialPortInfo portsAvailable, QSerialPortInfo::availablePorts())
    {
        portsNames << portsAvailable.portName();
    }

    cbPort->addItems(portsNames);
#if defined (Q_OS_LINUX)
    cbPort->setEditable(true); // TODO Make correct viewing available virtual ports in Linux
#else
    cbPort->setEditable(false);
#endif

    QStringList portsBauds;
    portsBauds << "115200" << "57600" << "38400";
    cbBaud->addItems(portsBauds);
    cbBaud->setEditable(false);
    bPortStop->setEnabled(false);

    m_BlinkTimeTxNone->setInterval(BLINKTIMETX);
    m_BlinkTimeRxNone->setInterval(BLINKTIMERX);
    m_BlinkTimeTxColor->setInterval(BLINKTIMETX);
    m_BlinkTimeRxColor->setInterval(BLINKTIMERX);
    m_TimeToDisplay->setInterval(DISPLAYTIME);

    rbLowFreq->setChecked(true);

    sbSetFreq->setRange(FREQ_RANGE_MIN, FREQ_RANGE_MAX);
    sbSetFreq->setAlignment(Qt::AlignCenter);
    sbSetPhase->setRange(PHASE_RANGE_MIN, PHASE_RANGE_MAX);
    sbSetPhase->setSingleStep(PHASE_STEP);
    sbSetPhase->setSuffix(QString::fromUtf8("°"));
    sbSetPhase->setAlignment(Qt::AlignCenter);

    QStringList fStrobeList;
    fStrobeList << "1" << "2" << "3" << "4" << "6" << "8" << "9";
    cbSetFStrobe->addItems(fStrobeList);
    sbSetGainIQ->setRange(GAIN_RANGE_MIN, GAIN_RANGE_MAX);
    sbSetGainIQ->setAlignment(Qt::AlignCenter);

    connect(bPortStart, SIGNAL(clicked()), this, SLOT(openPort()));
    connect(bPortStop, SIGNAL(clicked()), this, SLOT(closePort()));

    connect(cbPort, SIGNAL(currentIndexChanged(int)), this, SLOT(closePort()));
    connect(cbBaud, SIGNAL(currentIndexChanged(int)), this, SLOT(closePort()));

    connect(m_Protocol, SIGNAL(DataIsReaded(bool)), this, SLOT(received(bool)));

    connect(m_BlinkTimeTxColor, SIGNAL(timeout()), this, SLOT(colorIsTx()));
    connect(m_BlinkTimeRxColor, SIGNAL(timeout()), this, SLOT(colorIsRx()));
    connect(m_BlinkTimeTxNone, SIGNAL(timeout()), this, SLOT(colorTxNone()));
    connect(m_BlinkTimeRxNone, SIGNAL(timeout()), this, SLOT(colorRxNone()));

    connect(m_TimeToDisplay, SIGNAL(timeout()), this, SLOT(displayData()));

    connect(bSetGainIQ, SIGNAL(clicked()), this, SLOT(writeGainIQ()));
    connect(bSetFreq, SIGNAL(clicked()), this, SLOT(writeRESMON()));
    connect(bSetFStrobe, SIGNAL(clicked()), this, SLOT(writeFStrobe()));

    QShortcut *aboutShortcut = new QShortcut(QKeySequence("F1"), this);
    connect(aboutShortcut, SIGNAL(activated()), qApp, SLOT(aboutQt()));
}

Dialog::~Dialog()
{
    m_Port->close();
}

void Dialog::openPort()
{
    m_Port->close();
    m_Port->setPortName(cbPort->currentText());

    if(m_Port->open(QSerialPort::ReadWrite))
    {
        switch (cbBaud->currentIndex()) {
        case 0:
            m_Port->setBaudRate(QSerialPort::Baud115200);
            break;
        case 1:
            m_Port->setBaudRate(QSerialPort::Baud57600);
            break;
        case 2:
            m_Port->setBaudRate(QSerialPort::Baud38400);
            break;
        default:
            m_Port->setBaudRate(QSerialPort::Baud115200);
            break;
        }

        m_Port->setDataBits(QSerialPort::Data8);
        m_Port->setParity(QSerialPort::NoParity);
        m_Port->setFlowControl(QSerialPort::NoFlowControl);

        bPortStart->setEnabled(false);
        bPortStop->setEnabled(true);
        lTx->setStyleSheet("background: none; font: bold; font-size: 10pt");
        lRx->setStyleSheet("background: none; font: bold; font-size: 10pt");
    }
    else
    {
        lTx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
        lRx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    }
}

void Dialog::closePort()
{
    m_Port->close();
    m_BlinkTimeTxNone->stop();
    m_BlinkTimeTxColor->stop();
    m_BlinkTimeRxNone->stop();
    m_BlinkTimeRxColor->stop();
    lTx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    lRx->setStyleSheet("background: yellow; font: bold; font-size: 10pt");
    bPortStop->setEnabled(false);
    bPortStart->setEnabled(true);
    m_Protocol->resetProtocol();

    initIsDataSet();
}

void Dialog::received(bool isReceived)
{
    if(isReceived) {
        if(!m_BlinkTimeRxColor->isActive() && !m_BlinkTimeRxNone->isActive()) {
            m_BlinkTimeRxColor->start();
            lRx->setStyleSheet("background: green; font: bold; font-size: 10pt");
        }

        if(!m_TimeToDisplay->isActive()) {
            m_TimeToDisplay->start();
        }

        m_DisplayList = m_Protocol->getReadedData();        
    }
}

void Dialog::write(const Dialog::CODE &code)
{
    if(m_Port->isOpen()) {
        QMultiMap<QString, QString> dataTemp;

        if(!m_BlinkTimeTxColor->isActive() && !m_BlinkTimeTxNone->isActive()) {
            m_BlinkTimeTxColor->start();
            lTx->setStyleSheet("background: red; font: bold; font-size: 10pt");
        }

        QString codeStr;
        QString data;

        switch ( static_cast<int>(code) ) {
        case 1:
            codeStr = QString::number(CODE_RES_MON);
            data = QString::number(rbHighFreq ? sbSetFreq->value() | 0x80 : sbSetFreq->value());
            data += QString::fromUtf8(FREQ_PHASE_SEPARATOR);
            data += QString::number(sbSetPhase->value()/PHASE_STEP);
            m_isDataSet.insert("RESMON", true);
            break;
        case 2:
            codeStr = QString::number(CODE_F_STROBE);
            data = cbSetFStrobe->currentText();
            m_isDataSet.insert("FSTROBE", true);
            break;
        case 3:
            codeStr = QString::number(CODE_GAIN_IQ);
            data = QString::number(sbSetGainIQ->value());
            m_isDataSet.insert("GAINIQ", true);
            break;
        default:
            codeStr = QString::number(CODE_NONE_DATA);
            break;
        }
        dataTemp.insert("CODE", codeStr);
        dataTemp.insert("DATA", data);
        m_Protocol->setDataToWrite(dataTemp);
        m_Protocol->writeData();
    }
}

void Dialog::writeRESMON()
{
    write(RESMON);
}

void Dialog::writeFStrobe()
{
    write(FSTROBE);
}

void Dialog::writeGainIQ()
{
    write(GAINIQ);
}

void Dialog::colorTxNone()
{
    m_BlinkTimeTxNone->stop();
}

void Dialog::colorIsRx()
{
    lRx->setStyleSheet("background: none; font: bold; font-size: 10pt");
    m_BlinkTimeRxColor->stop();
    m_BlinkTimeRxNone->start();
}

void Dialog::colorRxNone()
{
    m_BlinkTimeRxNone->stop();
}

void Dialog::colorIsTx()
{
    lTx->setStyleSheet("background: none; font: bold; font-size: 10pt");
    m_BlinkTimeTxColor->stop();
    m_BlinkTimeTxNone->start();
}

void Dialog::displayData()
{
    m_TimeToDisplay->stop();

    QString tempStr;

    if(!m_isDataSet.value("RESMON")
            && !m_isDataSet.value("FSTROBE")
            && !m_isDataSet.value("GAINIQ")) {
        return;
    }

    if(m_isDataSet.value("RESMON")) {
        tempStr = m_DisplayList.value("RESMON");
        int freq = tempStr.section(FREQ_PHASE_SEPARATOR, 0, 0).toInt();
        double phase = tempStr.section(FREQ_PHASE_SEPARATOR, 1, 1).toDouble();
        if(freq & 0x80) {
            freq &= 0x7F;
            tempStr = QString::fromUtf8("High Frequency: ");
        } else {
            tempStr = QString::fromUtf8("Low Frequency: ");
        }
        lFreqInfo->setText(tempStr + QString::number(freq));
        lPhaseInfo->setText(QString::fromUtf8("Phase: ") + QString::number(phase * PHASE_STEP));
    }
    if(m_isDataSet.value("FSTROBE")) {
        lFStrobeInfo->setText(QString::fromUtf8("F-Strobe: ") + m_DisplayList.value("FSTROBE"));
    }
    if(m_isDataSet.value("GAINIQ")) {
        lGainIQInfo->setText(QString::fromUtf8("Gain I, Q: ") + m_DisplayList.value("GAINIQ"));
    }

    m_DisplayList.clear();
}

void Dialog::initIsDataSet()
{
    m_isDataSet.insert("RESMON", false);
    m_isDataSet.insert("FSTROBE", false);
    m_isDataSet.insert("GAINIQ", false);

    lFreqInfo->setText(QString::fromUtf8("Low Frequency: NONE"));
    lPhaseInfo->setText(QString::fromUtf8("Phase: NONE"));
    lFStrobeInfo->setText(QString::fromUtf8("F-Strobe: NONE"));
    lGainIQInfo->setText(QString::fromUtf8("Gain I, Q: NONE"));
}
