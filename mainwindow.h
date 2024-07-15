#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>

#include "BroadLinkMP1.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initSocketsTimers();
    void addLog(bool on, int socketID);

private slots:
    void on_socket1On_clicked();
    void on_socket1Off_clicked();
    void on_socket2On_clicked();
    void on_socket2Off_clicked();
    void on_socket3On_clicked();
    void on_socket3Off_clicked();
    void on_socket4On_clicked();
    void on_socket4Off_clicked();
    void on_allSocketsOn_clicked();
    void on_allSocketsOff_clicked();

    void on_deviceConnect_clicked();

private:
    Ui::MainWindow *ui = nullptr;

    BroadLinkMP1     *mp1 = nullptr;

    QTimer socket1Timer;
    QTimer socket2Timer;
    QTimer socket3Timer;
    QTimer socket4Timer;
};

#endif // MAINWINDOW_H
