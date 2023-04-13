#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QKeyEvent>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define TIMER_VALUE 100

class MyThread : public QThread
{

    Q_OBJECT

public:
    explicit MyThread(QString threadName, QSerialPort*, QSerialPort*, Ui::MainWindow* ui);
    void run(QString);

private:
    QString name;
    Ui::MainWindow* ui;
    QSerialPort* send_port;
    QSerialPort* receive_port;
    bool isCollision();
    bool isChannelFree();

signals:
    void collisionSignal(bool);
    void readOnlySignal(bool);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_textInput_textChanged();
    void serialReceive();
    void changingParity();

private:
    Ui::MainWindow *ui;
    QSerialPort *send_port;
    QSerialPort *receive_port;
    MyThread* thread;
    void serialSend (QString);
    QString bitStuff (QString);
    QString de_bitStuff (QString);
    void debug_bitStuff(QString);
    QString getFCS (QString);
    QString corruptData (QString);
    QString recoverData (QString, QString);
    struct packet {
        QString Flag;
        QString DestinationAddress;
        QString SourceAddress;
        QString Length;
        QString Data;
        QString FCS;
        QString getMsg()
        {
            QString msg = Flag + DestinationAddress + SourceAddress + Length + Data + FCS;
            return msg;
        }
    };
    void debug_FCS(packet);
//    bool isCollision();
//    bool isChannelFree();
};
#endif // MAINWINDOW_H
