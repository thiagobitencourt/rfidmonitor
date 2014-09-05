#ifndef SYSTEMMESSAGESWIDGET_H
#define SYSTEMMESSAGESWIDGET_H

#include <QWidget>
#include <QFile>

namespace Ui {
class SystemMessagesWidget;
}

class SystemMessagesWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief The MessageBehavior enum defines the behavior of the message.
     */
    enum MessageBehavior {
        KOnlyDialog=0, /**< The message will be displayed only in a dialog. */
        KOnlyTextbox, /**< The message will be displayed only in text box "System Messages". */
        KDialogAndTextbox, /**< The message will be displayed in the dialog and also in the text box "System Messages".*/
        KOnlyLogfile /**< The message will be displayed only in the log file.*/
    };

    /**
     * @brief The MessageLevel enum defines the severity of the message.
     */
    enum MessageLevel {
        KInfo=0, /**< The message will be displayed with "[INFORMATIVE]" prefix in log, and with a
                    information icon in the dialog. */
        KWarning, /**< The message will be displayed with "[WARNING]" prefix in log, and with a
                    warning icon in the dialog. */
        KError, /**< The message will be displayed with "[ERROR]" prefix in log, and with a
                    critical icon in the dialog. */
        KFatal, /**< The message will be displayed with "[FATAL ERROR]" prefix in log, and with a
                    critical icon in the dialog. */
        KDebug /**< To debug purpose.*/
    };

    /**
     * @brief instance is the implementation of singleton pattern in this class.
     * @return the unique instance of this class.
     */
    static SystemMessagesWidget *instance();

    ~SystemMessagesWidget();

    /**
     * @brief writeMessage offers the way to show system messages in the app.
     * @param message is the message itself.
     * @param messageLevel is the severity of the message.
     * @param messageBehavior is the behavior of the message.
     */
    void writeMessage(const QString &message,
                      const MessageLevel &messageLevel= KInfo,
                      const MessageBehavior &messageBehavior = KOnlyTextbox);

private:
    explicit SystemMessagesWidget(QWidget *parent = 0);
    Ui::SystemMessagesWidget *ui;

    /**
     * @brief m_logFile holds the instance of the logging file.
     */
    QFile *m_logFile;

    /**
     * @brief prepareLogfile will check the logs dir, and instantiate the QFile.
     *
     * The log file name will have the application name + the current time stamp.
     */
    void prepareLogfile();
};

#endif // SYSTEMMESSAGESWIDGET_H