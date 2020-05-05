#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

/*
Main application window
*/

#include <vector>
#include <memory>
#include <QTimer>

#include <QMainWindow>
#include <QSerialPort>
#include <QListWidgetItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void refresh_ports();
    bool is_connected(const QString& port);

    void on_ports_list_itemChanged(QListWidgetItem *item);
    void on_main_tab_widget_currentChanged(int);
    void save_command(QString command);

    void send_command(const QString& command);
    void delete_command(const QString& command);

    void on_messages_file_button_clicked();

    void on_messages_file_path_textChanged(const QString &arg1);

private:

    Ui::MainWindow *ui;

    QTimer _refresh_timer;

    std::vector<std::unique_ptr<QSerialPort> > _connected_ports;
    bool _refreshing;
};

#endif // MAINWINDOW_HPP
