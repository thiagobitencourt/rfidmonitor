/****************************************************************************
**
** www.celtab.org.br
**
** Copyright (C) 2013
**                     Gustavo Valiati <gustavovaliati@gmail.com>
**                     Luis Valdes <luisvaldes88@gmail.com>
**                     Thiago R. M. Bitencourt <thiago.mbitencourt@gmail.com>
**
** This file is part of the RFIDMonitor project
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; version 2
** of the License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
****************************************************************************/

#include <QSerialPortInfo>
#include <QTextStream>
#include <QTime>
#include <QDebug>

#include "serialcommunication.h"
#include "../gui/systemmessageswidget.h"

SerialCommunication *SerialCommunication::instance()
{
    static SerialCommunication *singleton = 0;
    if(!singleton)
        singleton = new SerialCommunication;
    return singleton;
}

SerialCommunication::SerialCommunication(QObject *parent) :
    QObject(parent)
{
    m_serialPort = new QSerialPort(this);

    connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialCommunication::dataRead);
}

QStringList SerialCommunication::availablePorts()
{
    QStringList deviceList;
    foreach (QSerialPortInfo spi, QSerialPortInfo::availablePorts()) {
        deviceList.append(spi.portName());
    }

    SystemMessagesWidget::instance()->writeMessage(
                tr("Device list refreshed."),
                SystemMessagesWidget::KDebug,
                SystemMessagesWidget::KOnlyLogfile
                );

    return deviceList;
}

void SerialCommunication::writeAnswer(QString text)
{
    // Emits the answer with time and the string trimmed().
    QString time("[" + QTime::currentTime().toString() + "] ");
    emit newAnswer(time + text.trimmed());
}

bool SerialCommunication::connectToDevice(const QString &device,
                                          const QIODevice::OpenModeFlag &openMode,
                                          const QSerialPort::BaudRate &baudRate,
                                          const QSerialPort::DataBits &dataBits,
                                          const QSerialPort::StopBits &stopBits,
                                          const QSerialPort::Parity &parity)
{
    if( ! m_serialPort->isOpen() ) {
        QSerialPortInfo spi(device);
        if(spi.isValid()){
            m_serialPort->setPort(spi);
            if(m_serialPort->open(openMode)){
                m_serialPort->setBaudRate(baudRate);
                m_serialPort->setDataBits(dataBits);
                m_serialPort->setStopBits(stopBits);
                m_serialPort->setParity(parity);
                SystemMessagesWidget::instance()->writeMessage(tr("Successfully connected to device."));
                return true;
            }else{
                SystemMessagesWidget::instance()->writeMessage(tr("Cannot connect to device."));
                return false;
            }
        }else{
            SystemMessagesWidget::instance()->writeMessage(tr("Device [ %1 ] not valid.").arg(device));
            return false;
        }
    }else{
        SystemMessagesWidget::instance()->writeMessage(tr("The device is already open."));
        return false;
    }
}

void SerialCommunication::disconnectFromDevice()
{
    if(m_serialPort->isOpen()){
        m_serialPort->close();
        SystemMessagesWidget::instance()->writeMessage(tr("Disconnected from device."));
    }else{
        SystemMessagesWidget::instance()->writeMessage(tr("INTERNAL ERROR: Can't disconnect from device because it is not connected."),
                                                       SystemMessagesWidget::KError,
                                                       SystemMessagesWidget::KOnlyLogfile);
    }

}

bool SerialCommunication::sendCommand(const QString &command, const SerialCommunication::CommandType &type)
{
    SystemMessagesWidget::instance()->writeMessage(tr("Command to be sent: [%1] Type: [%2]").arg(command).arg(type),
                                                   SystemMessagesWidget::KDebug,
                                                   SystemMessagesWidget::KOnlyLogfile);
    if(m_serialPort->isWritable()){
        if(type == KASCII){
            if (m_serialPort->write(command.toUtf8()) == -1){
                SystemMessagesWidget::instance()->writeMessage(tr("Error occurred writing to device."),
                                                               SystemMessagesWidget::KError,
                                                               SystemMessagesWidget::KDialogAndTextbox);
                return false;
            }
        }else if(type == KNumber){

            //convert QString to number
            bool parseOK = false;
            int parsedValue = command.toInt(&parseOK, 16);
            if(parseOK){
                if (m_serialPort->write(reinterpret_cast<char*>(&parsedValue), sizeof(int)) == -1){
                    SystemMessagesWidget::instance()->writeMessage(tr("Error occurred writing to device."),
                                                                   SystemMessagesWidget::KError,
                                                                   SystemMessagesWidget::KDialogAndTextbox);
                    return false;
                }
            }else{
                SystemMessagesWidget::instance()->writeMessage(tr("Could not convert the command [%1] to number.").arg(command),
                                                               SystemMessagesWidget::KError,
                                                               SystemMessagesWidget::KDialogAndTextbox);
                return false;
            }
        }

        SystemMessagesWidget::instance()->writeMessage(tr("Command sent: '%1'").arg(command),
                                                       SystemMessagesWidget::KDebug,
                                                       SystemMessagesWidget::KOnlyLogfile);
        return true;
    }else{
        SystemMessagesWidget::instance()->writeMessage(tr("Cannot send command to device. Device is not writable."),
                                                       SystemMessagesWidget::KError,
                                                       SystemMessagesWidget::KDialogAndTextbox);
        return false;
    }
}

void SerialCommunication::handleError(const QSerialPort::SerialPortError error)
{
    if(error != QSerialPort::NoError){
        SystemMessagesWidget::instance()->writeMessage(tr("Serial Port Error: %1").arg(m_serialPort->errorString()),
                                                       SystemMessagesWidget::KError,
                                                       SystemMessagesWidget::KDialogAndTextbox);
        m_serialPort->close();
        emit connectionFailed();
    }
}

void SerialCommunication::dataRead()
{
    // If have a complete line in the buffer, read it.
    if(m_serialPort->canReadLine())
        writeAnswer(QString(m_serialPort->readLine()));
}
