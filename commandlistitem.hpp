#ifndef COMMANDLISTITEM_HPP
#define COMMANDLISTITEM_HPP

/*
Visual item that represents stored command
*/

#include <QWidget>

namespace Ui {
class CommandListItem;
}

class CommandListItem : public QWidget
{
    Q_OBJECT

signals:

    // tells parent to delete this specific command
    void delete_command(const QString&);

    // tells parent to send this specific command to the serial port
    void send_command(const QString&);

public:

    //
    explicit CommandListItem(const QString &command, QWidget *parent = nullptr);
    ~CommandListItem();

    // accessors
    QString command() const;

private slots:

    void on_delete_button_clicked();
    void on_send_button_clicked();

private:

    Ui::CommandListItem *ui;
};

#endif // COMMANDLISTITEM_HPP
