#include "commandlistitem.hpp"
#include "ui_commandlistitem.h"

CommandListItem::CommandListItem(const QString& command, QWidget *parent) :
    QWidget(parent), ui(new Ui::CommandListItem)
{
    ui->setupUi(this);

    ui->command_label->setText(command);
}

CommandListItem::~CommandListItem()
{
    delete ui;
}

QString CommandListItem::command() const
{
    return ui->command_label->text();
}

void CommandListItem::on_delete_button_clicked()
{
    emit delete_command(ui->command_label->text());
}

void CommandListItem::on_send_button_clicked()
{
    send_command(ui->command_label->text());
}
