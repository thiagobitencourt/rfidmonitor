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

#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QCoreApplication>


#include "rictlmb2b30widget.h"
#include "ui_rictlmb2b30widget.h"
#include "../communication/serialcommunication.h"
#include "../communication/networkcommunication.h"
#include "systemmessageswidget.h"

RICTLMB2B30Widget::RICTLMB2B30Widget(Settings::ConnectionType connType, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RICTLMB2B30Widget),
    m_waitingForAnswer(false),
    m_waitingForCheck(false),
    m_connectionType(connType)
{
    ui->setupUi(this);

    ui->btWrite->setIcon(QIcon(":/icons/icon-signal"));
    ui->btNextIdentification->setIcon(QIcon(":/icons/icon-plus"));

    // Set up the interval time of the timeout.
    if(m_connectionType == Settings::KSerial){
        // 1 sec because serial is faster than network.
        m_timeout.setInterval(1000);
    }else {
        m_timeout.setInterval(1000*5);
    }

    // Define the timer to execute only one time.
    m_timeout.setSingleShot(true);

    // Define the regular expression for hexadecimal with 16 digits.
    QRegExp hexaRegExp("[0-9a-fA-F]{16}");
    m_hexaValidator = new QRegExpValidator(hexaRegExp,this);

    // Define the regular expression for decimal with 16 digits.
    QRegExp decRegExp("[0-9]{16}");
    m_deciValidator = new QRegExpValidator(decRegExp,this);

    // Set the default validator as hexadecimal.
    ui->leIdentification->setValidator(m_hexaValidator);

    connect(ui->leIdentification, SIGNAL(textChanged(QString)), this, SLOT(leIdentificationChanged(QString)));
    connect(ui->btWrite, SIGNAL(clicked()), this, SLOT(btWriteClicked()));
    connect(&m_timeout, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(ui->rbDecimal, SIGNAL(clicked()), this, SLOT(rbDecimalClicked()));
    connect(ui->rbHexadecimal, SIGNAL(clicked()), this, SLOT(rbHexadecimalClicked()));
    connect(ui->btNextIdentification, SIGNAL(clicked()), this, SLOT(incrementIdentification()));

    if(connType == Settings::KNetwork){
        connect(NetworkCommunication::instance(),SIGNAL(connectionFailed()),this, SLOT(communicationFinished()));
        connect(NetworkCommunication::instance(), SIGNAL(newReaderAnswer(QString)), this, SLOT(newAnswerFromReader(QString)));
    }else if(connType == Settings::KSerial){
        connect(SerialCommunication::instance(), SIGNAL(newAnswer(QString)), this, SLOT(newAnswerFromReader(QString)));
        connect(SerialCommunication::instance(),SIGNAL(connectionFailed()),this, SLOT(communicationFinished()));
    }
}

RICTLMB2B30Widget::~RICTLMB2B30Widget()
{
    delete ui;
}

void RICTLMB2B30Widget::sendCommand(const QString &command)
{
    if(m_connectionType == Settings::KSerial){
        SerialCommunication::instance()->sendCommand(command,SerialCommunication::KASCII);
    }
    else if(m_connectionType == Settings::KNetwork){
        NetworkCommunication::instance()->sendNewCommandToReader(command);
    }
}

void RICTLMB2B30Widget::lockForms()
{
    ui->btWrite->setEnabled(false);
    ui->btNextIdentification->setEnabled(false);
    ui->rbDecimal->setEnabled(false);
    ui->rbHexadecimal->setEnabled(false);
    ui->leIdentification->setEnabled(false);
}

void RICTLMB2B30Widget::leIdentificationChanged(QString newText)
{
    m_identification.clear();

    if( ! ui->leIdentification->text().isEmpty()){
        // If the field contains some value.

        if(ui->rbDecimal->isChecked()){
            // Parse to hexadecimal.

            quint64 decimal = ui->leIdentification->text().toULongLong();
            m_identification.setNum(decimal,16);

        }else if(ui->rbHexadecimal->isChecked()){
            // Do not parse, simple set it
            m_identification.append(ui->leIdentification->text());
        }
        QString finalIdentification;
        finalIdentification.fill('0', 16 - m_identification.size());
        finalIdentification.append(m_identification);
        m_identification.clear();
        m_identification.append(finalIdentification);
        ui->labelIdentificationStatus->setText(tr("Hexadecimal identification %1.").arg(m_identification));
    }else{
        // If the identification field is empty, then clear the field instead of
        // trying to handle it.
        ui->labelIdentificationStatus->clear();
    }
}

void RICTLMB2B30Widget::btWriteClicked()
{
    ui->labelRectangleStatus->close();

    if((!ui->rbDecimal->isChecked() && !ui->rbHexadecimal->isChecked())){
        SystemMessagesWidget::instance()->writeMessage(tr("Please, select the identification type."),
                                                       SystemMessagesWidget::KWarning,
                                                       SystemMessagesWidget::KDialogAndTextbox);
        return;
    }

    if(ui->leIdentification->text().isEmpty()){
        SystemMessagesWidget::instance()->writeMessage(tr("The identification must have at least 1 characters."),
                                                       SystemMessagesWidget::KWarning,
                                                       SystemMessagesWidget::KOnlyDialog);
        return;
    }

    QString message(tr("Wait... Trying to write."));
    SystemMessagesWidget::instance()->writeMessage(message, SystemMessagesWidget::KInfo);
    ui->labelRectangleStatus->setText(message);
    ui->labelRectangleStatus->setStyleSheet("QLabel { background-color : yellow;}");
    ui->labelRectangleStatus->show();

    //Process the events to repaint the widget with the new labelRectangleStatus.
    QCoreApplication::processEvents();

    ui->btWrite->setEnabled(false);

    // Mark the processing to check for a answers on reading.
    m_waitingForAnswer = true;

    if(m_connectionType == Settings::KNetwork){
        int interval = 500;
        QTimer timer;
        timer.setSingleShot(true);

        NetworkCommunication::instance()->sendFullRead(true);

        timer.start(interval);
        while(timer.remainingTime() > 0)
            ;

        sendCommand("L");

        timer.start(interval);
        while(timer.remainingTime() > 0)
            ;

        sendCommand("K0");

        timer.start(interval);
        while(timer.remainingTime() > 0)
            ;

        sendCommand("P" + m_identification);

    }else{
        /* Send the "L" command to the reader. This ensure that the reader operates in Line Mode.
         * It is needed to keep the reader responding for reads, to check the command responses
         * and the identification codes from transponder, automatically. */
        sendCommand("L");

        /* Send the "K0" command to the reader. This ensure that the reader understant it is working
         * with a 64-bits transponder, a K0. If the reader is not set with K0, it will not understand
         * the commands and will not answer, correctly. */
        sendCommand("K0");

        /* Send the "P" + identification code to the reader. This is the way to define a new code
         * to the trasponder. */
        sendCommand("P" + m_identification);
    }

    // Start the timeout to wait for a response. If the timer ends, define as failed.
    m_timeout.start();
}

void RICTLMB2B30Widget::newAnswerFromReader(QString answer)
{
    // Only check anything if is waiting for a answer, after sending a command.
    if(m_waitingForAnswer){
        bool parseOk;
        QString decimalValue = QString::number(m_identification.toULongLong(&parseOk,16));
        if(answer.contains(QString("P0"))){
            // The answer "P0" from the reader means the new code is successfully defined.

            //restart the timeout to have more time, now check the new identification.
            m_timeout.start();

            /* With a positive response from  the reader, its time to ensure the new code is
             * really in the transponder. So mark the process to compare the new code with
             * the defined one. */
            m_waitingForCheck = true;

        }else if(m_waitingForCheck && answer.contains(decimalValue)){
            /* If need to check the code, and it is inside the answer, means it was
             * really defined in the transponder. Now its consired success. */

            // Cancel the timeout.
            m_timeout.stop();
            // Does not wait for check anymore.
            m_waitingForCheck = false;

            // Print a cool message to the user.
            QString message(tr("Successfully written."));
            SystemMessagesWidget::instance()->writeMessage(message, SystemMessagesWidget::KInfo);
            ui->labelRectangleStatus->setText(message);
            ui->labelRectangleStatus->setStyleSheet("QLabel { background-color : green;}");
            ui->labelRectangleStatus->show();

            ui->btWrite->setEnabled(true);

        }else if(answer.contains(QString("P1"))){
            /* The answer "P1" from the reader means: The identification received from the
             * transponder is different to the identification transmitted. */
            SystemMessagesWidget::instance()->writeMessage(tr("Try again. The identification received from the"
                                                              " transponder is different to the identification"
                                                              " transmitted. Reader said: P1"),
                                                           SystemMessagesWidget::KError);
        m_timeout.stop();
        timeout();
        }
        else if(answer.contains(QString("P12"))){
            /* The answer "P12" from the reader means: the reader could not understand the transponder.
             * Probably because the reader is trying to handle a different kind of transponder, in the
             * wrong K1 mode. So it is needed to change the mode to K0 for a 64-bits transponder. */
            SystemMessagesWidget::instance()->writeMessage(tr("The reader is operatin in K1 mode. Change it to K0! Reader said: P12"), SystemMessagesWidget::KError);
            m_timeout.stop();
            timeout();
        }else if(answer.contains(QString("P2"))){
            /* The answer "P2" from the reader means: The reader did not receive any
             * identification from the transponder for comparison. */
            SystemMessagesWidget::instance()->writeMessage(tr("Try again. The reader did not receive any"
                                                              " identification from the transponder for comparison."
                                                              " Reader said: P2"),
                                                           SystemMessagesWidget::KError);
            m_timeout.stop();
            timeout();
        }
    }
}

void RICTLMB2B30Widget::timeout()
{
    // The answer from the reader did not come, or the checking for new code failed.
    m_waitingForAnswer = false;
    ui->btWrite->setEnabled(true);
    ui->labelRectangleStatus->setText(tr("Failed to write."));
    ui->labelRectangleStatus->setStyleSheet("QLabel { background-color : red;}");
    ui->labelRectangleStatus->show();
}

void RICTLMB2B30Widget::rbDecimalClicked()
{
    ui->leIdentification->setValidator(m_deciValidator);
    int pos;
    QString identification(ui->leIdentification->text());

    // Check if the code is valid for a decimal value.
    if(m_deciValidator->validate(identification,pos) == QRegExpValidator::Invalid)
        ui->leIdentification->clear();

    // Call leIdentificationChanged to handle the code.
    leIdentificationChanged(QString());
}

void RICTLMB2B30Widget::rbHexadecimalClicked()
{
    ui->leIdentification->setValidator(m_hexaValidator);
    int pos;
    QString identification(ui->leIdentification->text());

    // Check if the code is valid for a hexadecimal value.
    if(m_hexaValidator->validate(identification,pos) == QRegExpValidator::Invalid)
        ui->leIdentification->clear();

    // Call leIdentificationChanged to handle the code.
    leIdentificationChanged(QString());
}

void RICTLMB2B30Widget::incrementIdentification()
{
    if( ! ui->leIdentification->text().isEmpty()){
        if(ui->rbHexadecimal->isChecked()){
            bool parseOk = false;
            quint64 val = ui->leIdentification->text().toULongLong(&parseOk,16);
            if(parseOk){
                val++;
                QString newIdentification;
                newIdentification.setNum(val,16);
                int pos;
                if(m_hexaValidator->validate(newIdentification,pos) == QRegExpValidator::Invalid)
                    ui->leIdentification->clear();
                else
                    ui->leIdentification->setText(newIdentification);
            }else{
                SystemMessagesWidget::instance()->writeMessage(
                            tr("Failed to parse Hexadecimal to Decimal."),
                            SystemMessagesWidget::KError,
                            SystemMessagesWidget::KOnlyLogfile
                            );
            }
        }else if(ui->rbDecimal->isChecked()){
            quint64 val = ui->leIdentification->text().toULongLong();
            val++;
            QString newIdentification;
            newIdentification.setNum(val);
            int pos;
            if(m_deciValidator->validate(newIdentification,pos) == QRegExpValidator::Invalid)
                ui->leIdentification->clear();
            else
                ui->leIdentification->setText(newIdentification);
        }
    }
}

void RICTLMB2B30Widget::communicationFinished()
{
    lockForms();
}
