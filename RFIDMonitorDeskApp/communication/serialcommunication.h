#ifndef SERIALCOMMUNICATION_H
#define SERIALCOMMUNICATION_H

#include <QObject>
#include <QIODevice>
#include <QSerialPort>
#include <QFile>
#include <QStringList>

class SerialCommunication : public QObject
{
    Q_OBJECT
public:
    enum CommandType {KASCII = 0, KNumber};
    static SerialCommunication* instance();
    ~SerialCommunication();
    QStringList availablePorts();
    bool connectToDevice(const QString &device,
                                              const QIODevice::OpenModeFlag &openMode,
                                              const QSerialPort::BaudRate &baudRate,
                                              const QSerialPort::DataBits &dataBits,
                                              const QSerialPort::StopBits &stopBits,
                                              const QSerialPort::Parity &parity);
    void disconnectFromDevice();
    bool sendCommand(const QString &command, const SerialCommunication::CommandType &type);

public slots:
    void handleError(const QSerialPort::SerialPortError error);
    void dataRead();

signals:
    void newAnswer(QString answer);

private:
    explicit SerialCommunication(QObject *parent = 0);
    QSerialPort *m_serialPort;
    void writeLog(QString text);
};

#endif // SERIALCOMMUNICATION_H
