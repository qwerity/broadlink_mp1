#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QtWidgets/QMessageBox>

#define SECONDS_IN_HOUR (60 * 60)
#define STR_CURRENCY   (" $")

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);

    initSocketsTimers();
}

MainWindow::~MainWindow()
{
//    on_allSocketsOff_clicked();

    delete ui;
}

void MainWindow::initSocketsTimers()
{
    connect(&socket1Timer, &QTimer::timeout, this, [this](){
        QTime time = ui->socket1Timer->time().addSecs(1);
        ui->socket1Timer->setTime(time);

        auto oneSecondCost = float(ui->hourCost->value()) / SECONDS_IN_HOUR;
        auto currentCost = int(QTime(0,0,0,0).secsTo(time) * oneSecondCost);
        ui->socket1CurrentCost->setText(QString::number(currentCost) + STR_CURRENCY);

        auto prepaidValue = ui->socket1Prepaid->value();
        if (prepaidValue > 0 && currentCost >= prepaidValue)
            on_socket1Off_clicked();
    });

    connect(&socket2Timer, &QTimer::timeout, this, [this](){
        QTime time = ui->socket2Timer->time().addSecs(1);
        ui->socket2Timer->setTime(time);

        auto oneSecondCost = float(ui->hourCost->value()) / SECONDS_IN_HOUR;
        auto currentCost = int(QTime(0,0,0,0).secsTo(time) * oneSecondCost);
        ui->socket2CurrentCost->setText(QString::number(currentCost) + STR_CURRENCY);

        auto prepaidValue = ui->socket2Prepaid->value();
        if (prepaidValue > 0 && currentCost > prepaidValue)
            on_socket2Off_clicked();
    });

    connect(&socket3Timer, &QTimer::timeout, this, [this](){
        QTime time = ui->socket3Timer->time().addSecs(1);
        ui->socket3Timer->setTime(time);

        auto oneSecondCost = float(ui->hourCost->value()) / SECONDS_IN_HOUR;
        auto currentCost = int(QTime(0,0,0,0).secsTo(time) * oneSecondCost);
        ui->socket3CurrentCost->setText(QString::number(currentCost) + STR_CURRENCY);

        auto prepaidValue = ui->socket3Prepaid->value();
        if (prepaidValue > 0 && currentCost > prepaidValue)
            on_socket3Off_clicked();
    });

    connect(&socket4Timer, &QTimer::timeout, this, [this](){
        QTime time = ui->socket4Timer->time().addSecs(1);
        ui->socket4Timer->setTime(time);

        auto oneSecondCost = float(ui->hourCost->value()) / SECONDS_IN_HOUR;
        auto currentCost = int(QTime(0,0,0,0).secsTo(time) * oneSecondCost);
        ui->socket4CurrentCost->setText(QString::number(currentCost) + STR_CURRENCY);

        auto prepaidValue = ui->socket4Prepaid->value();
        if (prepaidValue > 0 && currentCost > prepaidValue)
            on_socket4Off_clicked();
    });
}

void MainWindow::addLog(bool on, int socketID)
{
    int prepaid = 0;
    QString timer;
    QString cost;
    switch(socketID)
    {
        case 1:
            prepaid = ui->socket1Prepaid->value();
            timer = ui->socket1Timer->time().toString("h:m:s");
            cost = ui->socket1CurrentCost->text() + STR_CURRENCY;
            break;
        case 2:
            prepaid = ui->socket2Prepaid->value();
            timer = ui->socket2Timer->time().toString("h:m:s");
            cost = ui->socket2CurrentCost->text() + STR_CURRENCY;
            break;
        case 3:
            prepaid = ui->socket3Prepaid->value();
            timer = ui->socket3Timer->time().toString("h:m:s");
            cost = ui->socket3CurrentCost->text() + STR_CURRENCY;
            break;
        case 4:
            prepaid = ui->socket4Prepaid->value();
            timer = ui->socket4Timer->time().toString("h:m:s");
            cost = ui->socket4CurrentCost->text() + STR_CURRENCY;
            break;
        default:
            return;
    }

    qInfo() << "Socket" << socketID << "power:" << (on ? "ON" : "OFF")
            << "\ttimer:" << timer
            << "\tcurrent cost:" << cost
            << "\tprepaid:" << prepaid;
}

void MainWindow::on_socket1On_clicked()
{
    if (nullptr != mp1)
    {
        ui->socket1Timer->setTime(QTime(0, 0, 0, 0));
        socket1Timer.start(1000);

        mp1->setPower(true, 1);
        addLog(true, 1);
    }
}

void MainWindow::on_socket1Off_clicked()
{
    if (nullptr != mp1)
    {
        socket1Timer.stop();
        ui->socket1Prepaid->setValue(0);

        mp1->setPower(false, 1);
        addLog(false, 1);
    }
}

void MainWindow::on_socket2On_clicked()
{
    if (nullptr != mp1)
    {
        ui->socket2Timer->setTime(QTime(0, 0, 0, 0));
        socket2Timer.start(1000);

        mp1->setPower(true, 2);
        addLog(true, 2);
    }
}

void MainWindow::on_socket2Off_clicked()
{
    if (nullptr != mp1)
    {
        socket2Timer.stop();
        ui->socket2Prepaid->setValue(0);

        mp1->setPower(false, 2);
        addLog(false, 2);
    }
}

void MainWindow::on_socket3On_clicked()
{
    if (nullptr != mp1)
    {
        ui->socket3Timer->setTime(QTime(0, 0, 0, 0));
        socket3Timer.start(1000);

        mp1->setPower(true, 3);
        addLog(true, 3);
    }
}

void MainWindow::on_socket3Off_clicked()
{
    if (nullptr != mp1)
    {
        socket3Timer.stop();
        ui->socket3Prepaid->setValue(0);

        mp1->setPower(false, 3);
        addLog(false, 3);
    }
}

void MainWindow::on_socket4On_clicked()
{
    if (nullptr != mp1)
    {
        ui->socket4Timer->setTime(QTime(0, 0, 0, 0));
        socket4Timer.start(1000);

        mp1->setPower(true, 4);
        addLog(true, 4);
    }
}

void MainWindow::on_socket4Off_clicked()
{
    if (nullptr != mp1)
    {
        socket4Timer.stop();
        ui->socket4Prepaid->setValue(0);

        mp1->setPower(false, 4);
        addLog(false, 4);
    }
}

void MainWindow::on_allSocketsOn_clicked()
{
    if (nullptr != mp1)
    {
        mp1->setPower(true);
    }
}

void MainWindow::on_allSocketsOff_clicked()
{
    if (nullptr != mp1)
    {
        mp1->setPower(false);
    }
}

void MainWindow::on_deviceConnect_clicked()
{
    static QString deviceIP;
    const QString disconnectStr{"Disconnect"};

    auto ip = ui->deviceIP->text();
    auto mac = QByteArray::fromHex(ui->deviceMAC->text().toUtf8());

    if (disconnectStr != ui->deviceConnect->text())
    {
        if (! deviceIP.isNull() && deviceIP != ip)
        {
            QMessageBox::warning(this, "BroadLink MP1", "You can use only one device");
            return;
        }

        mp1 = new BroadLinkMP1(ip, mac);
        if (mp1->auth())
        {
            ui->deviceConnect->setText(disconnectStr);
            deviceIP = ip;
        }
        else
        {
            ui->deviceConnect->setText("Check ip/mac & try again");

            delete mp1;
            mp1 = nullptr;
        }
    }
    else
    {
        delete mp1;
        mp1 = nullptr;
        ui->deviceConnect->setText("Connect");
    }
}
