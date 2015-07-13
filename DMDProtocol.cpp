#include "DMDProtocol.h"

#ifdef DEBUG
#include <QDebug>
#endif

#define STARTBYTE 0x55
#define STOPBYTE 0xAA
#define BYTESLENTH 8

#define FREQ_PHASE_SEPARATOR "|"

#define CODE_RES_MON 0x01
#define CODE_F_STROBE 0x02
#define CODE_GAIN_IQ 0x03

#define NULL_DATA '\0'

DMDProtocol::DMDProtocol(ComPort *comPort, QObject *parent) :
    IProtocol(parent),
    itsComPort(comPort),
    m_resend(new QTimer(this)),
    m_numResends(3),
    m_currentResend(0)
{
    m_resend->setInterval(100);

    connect(itsComPort, SIGNAL(DataIsReaded(bool)), this, SLOT(readData(bool)));
    connect(itsComPort, SIGNAL(DataIsWrited(bool)), this, SIGNAL(DataIsWrited(bool)));
//    connect(m_resend, SIGNAL(timeout()), this, SLOT(writeData()));
}

void DMDProtocol::setDataToWrite(const QMultiMap<QString, QString> &data)
{
    m_WriteData = data;
}

QMultiMap<QString, QString> DMDProtocol::getReadedData() const
{
    return m_ReadData;
}

void DMDProtocol::readData(bool isReaded)
{
    m_ReadData.clear();

    if(isReaded) {
        QByteArray ba;

        ba = itsComPort->getReadData();

        m_ReadData.insert(QString("RESMON"),
                          QString::number(static_cast<int>(ba.at(1))) +
                          QString(FREQ_PHASE_SEPARATOR) +
                          QString::number(static_cast<int>(ba.at(2))));
#ifdef DEBUG
        qDebug() << "RESMON:" << m_ReadData;
#endif
        m_ReadData.insert(QString("FSTROBE"),
                          QString::number(static_cast<int>(ba.at(3))));
#ifdef DEBUG
        qDebug() << "RESMON+FSTROBE:" << m_ReadData;
#endif
        m_ReadData.insert(QString("GAINIQ"),
                          QString::number(static_cast<int>(ba.at(4))));
#ifdef DEBUG
        qDebug() << "RESMON+FSTROBE+GAINIQ:" << m_ReadData;
#endif
        emit DataIsReaded(true);

    } else {
        emit DataIsReaded(false);
    }
}

void DMDProtocol::writeData()
{
    QByteArray ba;

    ba.append(STARTBYTE);
    ba.append(m_WriteData.value("CODE").toInt());

    switch (m_WriteData.value("CODE").toInt()) {
    case CODE_RES_MON:
        ba.append(m_WriteData.value("DATA").section(FREQ_PHASE_SEPARATOR, 0, 0).toInt());
        ba.append(m_WriteData.value("DATA").section(FREQ_PHASE_SEPARATOR, 1, 1).toInt());
        break;
    case CODE_F_STROBE:
        ba.append(m_WriteData.value("DATA").toInt());
        ba.append(NULL_DATA);
        break;
    case CODE_GAIN_IQ:
        ba.append(m_WriteData.value("DATA").toInt());
        ba.append(NULL_DATA);
        break;
    }

    ba.append(NULL_DATA);
    ba.append(NULL_DATA);
    ba.append(NULL_DATA);
    ba.append(STOPBYTE);

    itsComPort->setWriteData(ba);
    itsComPort->writeData();
}

void DMDProtocol::resetProtocol()
{
    // TODO
}

int DMDProtocol::wordToInt(QByteArray ba)
{
    if(ba.size() != 2)
        return -1;

    int temp = ba[0];
    if(temp < 0)
    {
        temp += 0x100; // 256;
        temp *= 0x100;
    }
    else
        temp = ba[0]*0x100; // старший байт

    int i = ba[1];
    if(i < 0)
    {
        i += 0x100; // 256;
        temp += i;
    }
    else
        temp += ba[1]; // младший байт

    return temp;
}

QByteArray DMDProtocol::intToByteArray(const int &value, const int &numBytes)
{
    QByteArray ba;

    for(int i = numBytes - 1; i > - 1; --i) {
        ba.append((value >> 8*i) & 0xFF);
    }

    return ba;
}
