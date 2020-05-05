#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include<QSerialPortInfo>
#include <QSettings>
#include <QFileDialog>
#include <QTextStream>

#include "comportconsole.hpp"
#include "commandlistitem.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), _refresh_timer(),
    _refreshing(false)
{
    ui->setupUi(this);
    QSettings settings;
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    restoreState(settings.value("MainWindow/state").toByteArray());

    refresh_ports();

    ui->messages_file_path->setText(settings.value("messagespath").toString());

    _refresh_timer.setInterval(1000);
    connect(&_refresh_timer, &QTimer::timeout, this, &MainWindow::refresh_ports);
    _refresh_timer.start();
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/state", saveState());

    QFile file(ui->messages_file_path->text());
    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream ss(&file);
        for(int i = 0; i < ui->messages_list->count(); ++i)
        {
            CommandListItem* item = qobject_cast<CommandListItem*>(ui->messages_list->itemWidget(ui->messages_list->item(i)));
            if(item)
            {
                QString command = item->command();
                command.replace(QChar('\r'), "\\r");
                command.replace(QChar('\n'), "\\n");
                command.replace(QChar('\t'), "\\t");
                ss << command << "\n";

            }
        }
    }

    delete ui;
}

void MainWindow::refresh_ports()
{
    _refreshing = true;
    ui->ports_list->clear();
    for(auto& port : QSerialPortInfo::availablePorts())
    {
        ui->ports_list->addItem(port.portName());
        ui->ports_list->item(ui->ports_list->count()-1)->
                setSizeHint(QSize(ui->ports_list->item(ui->ports_list->count()-1)->sizeHint().width(), 64));
    }
    for(int i = 0; i < ui->ports_list->count(); ++i)
    {
        if(is_connected(ui->ports_list->item(i)->text()))
        {
            ui->ports_list->item(i)->setCheckState(Qt::Checked);
        }
        else
        {
            ui->ports_list->item(i)->setCheckState(Qt::Unchecked);
        }
    }

    for(auto it = _connected_ports.begin(); it != _connected_ports.end(); ++it)
    {
        bool connected = false;
        for(auto port : QSerialPortInfo::availablePorts())
        {
            if(port.portName() == it->get()->portName() && it->get()->isOpen())
            {
                connected = true;
                break;
            }
        }
        if(!connected)
        {
            for(int i = 0; i < ui->main_tab_widget->count(); ++i)
            {
                if(ui->main_tab_widget->tabText(i) == (*it)->portName())
                {
                    ui->main_tab_widget->removeTab(i);
                }
            }
            _connected_ports.erase(it);
            return;
        }
    }

    _refreshing = false;
}

bool MainWindow::is_connected(const QString& port)
{
    for(auto& p : _connected_ports)
    {
        if(p->portName() == port)
        {
            return true;
        }
    }
    return false;
}

void MainWindow::on_ports_list_itemChanged(QListWidgetItem *item)
{
    if(item->checkState() == Qt::Checked)
    {
        if(is_connected(item->text()))
        {
            return;
        }
        // connect to port
        for(auto& port : QSerialPortInfo::availablePorts())
        {
            if(port.portName() == item->text())
            {
                _connected_ports.emplace_back(std::make_unique<QSerialPort>(port));
                if(!_connected_ports.back()->open(QIODevice::ReadWrite))
                {
                    _connected_ports.pop_back();
                    item->setCheckState(Qt::Unchecked);
                    refresh_ports();
                }
                else
                {
                    ComPortConsole* console = new ComPortConsole(ui->main_tab_widget);
                    connect(console, &ComPortConsole::save_message, this, &MainWindow::save_command);
                    console->set_serial(_connected_ports.back().get());
                    ui->main_tab_widget->addTab(console, port.portName());
                }
                return;
            }
        }
    }
    else if(item->checkState() == Qt::Unchecked)
    {
        if(_refreshing)
        {
            return;
        }
        // disconnect from the port
        for(auto it = _connected_ports.begin(); it != _connected_ports.end(); ++it)
        {
            if((*it)->portName() == item->text())
            {
                _connected_ports.erase(it);
                for(int i = 0; i < ui->main_tab_widget->count(); ++i)
                {
                    if(ui->main_tab_widget->tabText(i) == item->text())
                    {
                        ui->main_tab_widget->removeTab(i);
                    }
                }
                return;
            }
        }
    }
}

void MainWindow::on_main_tab_widget_currentChanged(int)
{

}

void MainWindow::save_command(QString command)
{
    CommandListItem* item = new CommandListItem(command);

    connect(item, &CommandListItem::send_command, this, &MainWindow::send_command);
    connect(item, &CommandListItem::delete_command, this, &MainWindow::delete_command);

    QListWidgetItem* widget_item = new QListWidgetItem();

    widget_item->setSizeHint(QSize(widget_item->sizeHint().width(),44));

    ui->messages_list->addItem(widget_item);
    ui->messages_list->setItemWidget(widget_item, item);
}

void MainWindow::send_command(const QString& command)
{
    ComPortConsole* port = qobject_cast<ComPortConsole*>(ui->main_tab_widget->currentWidget());
    if(port)
    {
        port->send_message(command);
    }
}

void MainWindow::delete_command(const QString& command)
{
    for(int i = 0; i < ui->messages_list->count(); ++i)
    {
        if(qobject_cast<CommandListItem*>(ui->messages_list->itemWidget(ui->messages_list->item(i)))->command() == command)
        {
            delete ui->messages_list->takeItem(i);
        }
    }
}

void MainWindow::on_messages_file_button_clicked()
{
    // save (even on cancel)
    QFile file(ui->messages_file_path->text());
    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream ss(&file);
        for(int i = 0; i < ui->messages_list->count(); ++i)
        {
            CommandListItem* item = qobject_cast<CommandListItem*>(ui->messages_list->itemWidget(ui->messages_list->item(i)));
            if(item)
            {
                ss << item->command() << "\n";

            }
        }
    }

    QSettings settings;
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDefaultSuffix("txt");
    dialog.setNameFilter("*.txt");
    dialog.setDirectory(settings.value("messagespathdirectory").toString());
    QString path = "";
    if(dialog.exec())
    {
        path = dialog.selectedFiles().first();
    }
    if(path.size() != 0)
    {
        ui->messages_file_path->setText(path);
        settings.setValue("messagespath", path);
        settings.setValue("messagespathdirectory", dialog.directory().path());
    }
}

void MainWindow::on_messages_file_path_textChanged(const QString &arg1)
{
    ui->messages_list->clear();
    QFile file(arg1);
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            save_command(in.readLine());
        }
    }
}
