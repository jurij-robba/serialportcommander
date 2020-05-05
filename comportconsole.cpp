#include "comportconsole.hpp"
#include "ui_comportconsole.h"

#include <QIntValidator>
#include <QCompleter>
#include <cctype>

ComPortConsole::ComPortConsole(QWidget *parent) :
    QWidget(parent), _last_message(SENDER::NONE), _port(nullptr),
    ui(new Ui::ComPortConsole)
{
    ui->setupUi(this);
    ui->message_edit->installEventFilter(this);
    ui->baud_rate_combo->setValidator(new QIntValidator(0, 999999, this));

    QStringList standard_baud;
    standard_baud.append("110");
    standard_baud.append("300");
    standard_baud.append("600");
    standard_baud.append("1200");
    standard_baud.append("2400");
    standard_baud.append("4800");
    standard_baud.append("9600");
    standard_baud.append("14400");
    standard_baud.append("19200");
    standard_baud.append("38400");
    standard_baud.append("57600");
    standard_baud.append("115200");
    standard_baud.append("128000");
    standard_baud.append("256000");
    ui->baud_rate_combo->addItems(standard_baud);
}

ComPortConsole::~ComPortConsole()
{
    delete ui;
}

void ComPortConsole::set_serial(QSerialPort* port)
{
    _port = port;

    ui->baud_rate_combo->setCurrentText(QString::number(baud_rate()));
    ui->rts_button->setChecked(rts());
    ui->dtr_button->setChecked(dtr());

    if(!_port->setFlowControl(QSerialPort::NoFlowControl))
    {
        //qDebug() << "flow not set!";
    }

    QObject::connect(_port, &QSerialPort::readyRead, this, &ComPortConsole::new_message);
}

void ComPortConsole::set_baud_rate(const int& val)
{
    if(_port)
    {
        _port->setBaudRate(val);
    }
}
int ComPortConsole::baud_rate() const
{
    if(_port)
    {
        return _port->baudRate();
    }
    return 0;
}

void ComPortConsole::set_rts(const bool& b)
{
    if(_port)
    {
        _port->setRequestToSend(b);
    }
}
bool ComPortConsole::rts() const
{
    if(_port)
    {
        return _port->isRequestToSend();
    }
    return false;
}

void ComPortConsole::set_dtr(const bool& b)
{
    if(_port)
    {
        _port->setDataTerminalReady(b);
    }
}
bool ComPortConsole::dtr() const
{
    if(_port)
    {
        return _port->isDataTerminalReady();
    }
    return false;
}

void ComPortConsole::new_message()
{
    QByteArray array = _port->readAll();
    QString message;
    if(ui->communication_hex_mode->isChecked())
    {
        for(const auto& byte : array)
        {
            if(isalnum(byte) && ui->communication_protect_alphanumeric->isChecked())
            {
                message += QChar::fromLatin1(byte);
            }
            else if(byte >= 33 && byte <= 126 && ui->communication_protect_extended->isChecked())
            {
                message += QChar::fromLatin1(byte);
            }
            else
            {
                message += " 0x";
                message += QString::number(byte, 16).right(2);
                message += " ";
            }
        }
    }
    else
    {
        message = array;
    }
    print_to_console(message, SENDER::DEVICE);
}

void ComPortConsole::send_message(QString message)
{
    _history.emplace_front(message);
    if(_history.size() > 20)
    {
        _history.pop_back();
    }
    _history_idx = -1;

    message.replace("\\r", QChar('\r'));
    message.replace("\\n", QChar('\n'));
    message.replace("\\t", QChar('\t'));

    if(ui->crc8_checkbox->isChecked())
    {
        message += QString("*") + crc8(message);
    }
    if(ui->add_dollar_checkbox->isChecked())
    {
        message = "$" + message;
    }
    if(ui->line_ending_n->isChecked())
    {
        message += "\n";
    }
    else if(ui->line_ending_rn->isChecked())
    {
        message += "\r\n";
    }

    print_to_console(message, SENDER::USER);
    _port->write(message.toLatin1());
}

QString ComPortConsole::crc8(const QString& message)
{
    QByteArray data = message.toLatin1();
    char crc = 0;
    for(int i = 0; i < data.size(); ++i)
    {
        crc ^= data[i];
    }
    data = QByteArray(1, crc);
    return data.toHex();
}

void ComPortConsole::on_send_button_clicked()
{
    if(!ui->message_edit->text().isEmpty())
    {
        send_message(ui->message_edit->text());
    }
    ui->message_edit->clear();
}

void ComPortConsole::on_message_edit_returnPressed()
{
    if(!ui->message_edit->text().isEmpty())
    {
        send_message(ui->message_edit->text());
    }
    ui->message_edit->clear();
}

bool ComPortConsole::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->message_edit)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Up)
            {
                if(_history.empty())
                {
                    return true;
                }
                ++_history_idx;
                if(_history_idx > static_cast<int>(_history.size()-1))
                {
                    _history_idx = static_cast<int>(_history.size()-1);
                }
                ui->message_edit->setText(_history.at(static_cast<size_t>(_history_idx)));
                return true;
            }
            else if(keyEvent->key() == Qt::Key_Down)
            {
                if(_history.empty())
                {
                    return true;
                }
                --_history_idx;
                if(_history_idx <= -1)
                {
                    _history_idx = -1;
                    ui->message_edit->clear();
                }
                else
                {
                    ui->message_edit->setText(_history.at(static_cast<size_t>(_history_idx)));
                }
                return true;
            }
        }
        return false;
    }
    return QWidget::eventFilter(obj, event);
}

void ComPortConsole::on_save_message_button_clicked()
{
    if(!ui->message_edit->text().isEmpty())
    {
        emit save_message(ui->message_edit->text());
    }
    ui->message_edit->clear();
}

void ComPortConsole::print_to_console(QString message, const SENDER& source)
{
    if(source == SENDER::DEVICE)
    {
        if(_last_message == SENDER::NONE)
        {
             message = "<-- " + message;
        }
        else if(_last_message == SENDER::USER)
        {
             message = "\n\n<-- " + message;
        }
        _last_message = SENDER::DEVICE;
    }
    else if(source == SENDER::USER)
    {
        if(_last_message == SENDER::NONE)
        {
             message = "--> " + message;
        }
        else if(_last_message == SENDER::DEVICE)
        {
             message = "\n\n--> " + message;
        }
        _last_message = SENDER::USER;
    }

    ui->message_history->moveCursor (QTextCursor::End);
    ui->message_history->insertPlainText(message);
}

void ComPortConsole::on_dtr_button_toggled(bool checked)
{
    set_dtr(checked);
}

void ComPortConsole::on_rts_button_toggled(bool checked)
{
    set_rts(checked);
}

void ComPortConsole::on_control_no_control_radio_toggled(bool checked)
{
    if(checked)
    {
        if(!_port->setFlowControl(QSerialPort::NoFlowControl))
        {
            //qDebug() << "flow not set!";
        }
    }
}

void ComPortConsole::on_control_hardware_radio_toggled(bool checked)
{
    if(checked)
    {
        if(!_port->setFlowControl(QSerialPort::HardwareControl))
        {
            //qDebug() << "flow not set!";
        }
    }
}

void ComPortConsole::on_control_software_radio_toggled(bool checked)
{
    if(checked)
    {
        if(!_port->setFlowControl(QSerialPort::SoftwareControl))
        {
            //qDebug() << "flow not set!";
        }
    }
}

void ComPortConsole::flow_control_changed(QSerialPort::FlowControl flow)
{
    switch (flow)
    {
    case QSerialPort::NoFlowControl:
        ui->control_no_control_radio->setChecked(true);
        break;
    case QSerialPort::HardwareControl:
        ui->control_hardware_radio->setChecked(true);
        break;
    case QSerialPort::SoftwareControl:
        ui->control_software_radio->setChecked(true);
        break;
    default:
        break;
    }
}

void ComPortConsole::on_clear_button_clicked()
{
    ui->message_history->clear();
}

void ComPortConsole::on_baud_rate_combo_currentTextChanged(const QString &arg1)
{
    set_baud_rate(arg1.toInt());
}
