#ifndef READSENSORPROTOCOL_H
#define READSENSORPROTOCOL_H

#include "IProtocol.h"
#include "ComPort.h"

#include <QTimer>
#include <QVector>

class DMDProtocol : public IProtocol
{
    Q_OBJECT
public:
    explicit DMDProtocol(ComPort *comPort, QObject *parent = 0);
    virtual void setDataToWrite(const QMultiMap<QString, QString> &data);
    virtual QMultiMap<QString, QString> getReadedData() const;
signals:

public slots:
    virtual void writeData();
    virtual void resetProtocol();
private slots:
    void readData(bool isReaded);
private:
    ComPort *itsComPort;

    QMultiMap<QString, QString> m_WriteData;
    QMultiMap<QString, QString> m_ReadData;

    QTimer *m_resend;
    int m_numResends;
    int m_currentResend;

    // converting 2 bytes to int
    int wordToInt(QByteArray ba);
    // converting number to byte array
    QByteArray intToByteArray(const int &value, const int &numBytes);
};

#endif // READSENSORPROTOCOL_H
