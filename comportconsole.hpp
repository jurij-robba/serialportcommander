#ifndef COMPORTCONSOLE_HPP
#define COMPORTCONSOLE_HPP

/*
Widget controling single connection to serial port
*/

#include <deque>

#include <QWidget>
#include <QSerialPort>

namespace Ui {
class ComPortConsole;
}

class ComPortConsole : public QWidget
{
    Q_OBJECT

signals:

    void save_message(const QString&);

public:

    enum class SENDER
    {
        NONE = 0,
        USER,
        DEVICE
    };

    explicit ComPortConsole(QWidget *parent = nullptr);
    ~ComPortConsole() override;

    void set_serial(QSerialPort* port);

    void set_baud_rate(const int& val);
    int baud_rate() const;

    void set_rts(const bool& b);
    bool rts() const;

    void set_dtr(const bool& b);
    bool dtr() const;

    void send_message(QString message);

private slots:

    void new_message();
    QString crc8(const QString& message);

    void on_send_button_clicked();

    void on_message_edit_returnPressed();

    bool eventFilter(QObject *obj, QEvent *event) override;

    void on_save_message_button_clicked();

    void print_to_console(QString message, const SENDER& source);

    void on_dtr_button_toggled(bool checked);

    void on_rts_button_toggled(bool checked);

    void on_control_no_control_radio_toggled(bool checked);

    void on_control_hardware_radio_toggled(bool checked);

    void on_control_software_radio_toggled(bool checked);

    void flow_control_changed(QSerialPort::FlowControl flow);

    void on_clear_button_clicked();

    void on_baud_rate_combo_currentTextChanged(const QString &arg1);

private:

    SENDER _last_message;
    QSerialPort* _port;
    std::deque<QString> _history;
    int _history_idx;
    Ui::ComPortConsole *ui;
};

#endif
