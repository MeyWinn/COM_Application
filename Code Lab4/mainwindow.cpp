#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QString>
#include <QRandomGenerator>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    send_port = new QSerialPort(this);
    send_port->setBaudRate(QSerialPort::Baud9600);
    send_port->setDataBits(QSerialPort::Data8);
    send_port->setParity(QSerialPort::NoParity);
    send_port->setStopBits(QSerialPort::OneStop);
    send_port->setFlowControl(QSerialPort::NoFlowControl);

    receive_port = new QSerialPort(this);
    receive_port->setBaudRate(QSerialPort::Baud9600);
    receive_port->setDataBits(QSerialPort::Data8);
    receive_port->setParity(QSerialPort::NoParity);
    receive_port->setStopBits(QSerialPort::OneStop);
    receive_port->setFlowControl(QSerialPort::NoFlowControl);

    QString errorString;

    send_port->setPortName("COM4");
    if (send_port->open(QSerialPort::WriteOnly))
    {
        ui->Com1->setText("Write mode: COM4");
        receive_port->setPortName("COM5");
        if(receive_port->open(QSerialPort::ReadOnly))
            ui->Com2->setText("Read mode: COM5");
        else
            errorString = "Unable to initialize COM5";
    }
    else
    {
        send_port->setPortName("COM6");
        if(send_port->open(QSerialPort::WriteOnly))
        {
            ui->Com1->setText("Write mode: COM6");
            receive_port->setPortName("COM7");
            if(receive_port->open(QSerialPort::ReadOnly))
                ui->Com2->setText("Read mode: COM7");
            else
                errorString = "Unable to initialize COM7";
        }
        else
            errorString = "Unable to initialize COM6";
    }
    if (!errorString.isEmpty())
    {
        QMessageBox::critical(this, "Error", errorString + "\nPlease make sure that the ports are available, and then restart the program.");
        ui->textInput->setReadOnly(true);
        ui->Com1->setText("Unable to initialize COM Port in Write Mode");
        ui->Com2->setText("Unable to initialize COM Port in Read Mode");
    }
    else
    {
        thread = new MyThread("thread", send_port, receive_port, this->ui);

        connect(receive_port, &QSerialPort::readyRead, this, &MainWindow::serialReceive);

        connect(ui->NoParityButton, SIGNAL(clicked()), this, SLOT(changingParity()));
        connect(ui->EvenParityButton, SIGNAL(clicked()), this, SLOT(changingParity()));
        connect(ui->OddParityButton, SIGNAL(clicked()), this, SLOT(changingParity()));
        connect(ui->SpaceParityButton, SIGNAL(clicked()), this, SLOT(changingParity()));
        connect(ui->MarkParityButton, SIGNAL(clicked()), this, SLOT(changingParity()));
    }
}

MainWindow::~MainWindow()
{
    delete ui;

    send_port->close();
    delete send_port;

    receive_port->close();
    delete receive_port;
}

MyThread::MyThread(QString threadName, QSerialPort* send_port, QSerialPort* receive_port, Ui::MainWindow* ui)
    : name(threadName) {
    this->send_port = send_port;
    this->receive_port = receive_port;
    this->ui = ui;
}

void MyThread::run(QString str)
{
    this->ui->textInput->setReadOnly(true);
    ui->textStatus->moveCursor(QTextCursor::End);
    for (int i = 0; i < str.length(); i++)
    {
        while (!isChannelFree());
        QString symb = str[i];
        send_port->write(symb.toUtf8());
        send_port->waitForBytesWritten(-1);
        QThread::usleep(TIMER_VALUE);
        for (int N = 1; N <= 10; N++)
        {
            if (!isCollision())
                break;
            ui->textStatus->insertPlainText("c");
            ui->textStatus->repaint();
            int L = QRandomGenerator::global()->bounded(qPow(2, N));
            QThread::usleep(L * TIMER_VALUE);
            while (!isChannelFree());
            send_port->write(symb.toUtf8());
            send_port->waitForBytesWritten(-1);
        }
        ui->textStatus->insertPlainText("\n");
    }
    send_port->write("\n");
    this->ui->textInput->setReadOnly(false);
}

void MainWindow::on_textInput_textChanged()
{
    QString str = ui->textInput->toPlainText();
    QTextCursor cursor = ui->textInput->textCursor();
    int c = cursor.position();
    if (c > 0 && str[c - 1] == '\n')
    {
        str.remove(c - 1, 1);
        thread->run(str);
        ui->textInput->clear();
    }
    else if (c > 0 && str[c - 1] != '0' && str[c - 1] != '1')
    {
        str.remove(c - 1, 1);
        ui->textInput->setText(str);
        ui->textInput->moveCursor(QTextCursor::End);
    }
}

void MainWindow::changingParity()
{
    QRadioButton* button = (QRadioButton *)sender();
    send_port->close();
    if(button->text()=="NoParity")
        send_port->setParity(QSerialPort::NoParity);
    else if(button->text()=="EvenParity")
        send_port->setParity(QSerialPort::EvenParity);
    else if(button->text()=="OddParity")
        send_port->setParity(QSerialPort::OddParity);
    else if(button->text()=="SpaceParity")
        send_port->setParity(QSerialPort::SpaceParity);
    else if(button->text()=="MarkParity")
        send_port->setParity(QSerialPort::MarkParity);
    send_port->open(QSerialPort::WriteOnly);
}

//void MainWindow::serialSend(QString str)
//{
//    ui->textStatus->moveCursor(QTextCursor::End);
//    for (int i = 0; i < str.length(); i++)
//    {
//        while (!isChannelFree());
//        QString symb = str[i];
//        send_port->write(symb.toUtf8());
//        QThread::usleep(TIMER_VALUE);
//        for (int N = 1; N <= 10; N++)
//        {
//            if (!isCollision())
//                break;
//            ui->textStatus->insertPlainText("c");
//            ui->textStatus->repaint();
//            int L = QRandomGenerator::global()->bounded(qPow(2, N));
//            QThread::usleep(L * TIMER_VALUE);
//            while (!isChannelFree());
//            send_port->write(symb.toUtf8());
//        }
//        ui->textStatus->insertPlainText("\n");
//    }

////    packet sendMsg;
////    sendMsg.Flag = "01101110";
////    sendMsg.DestinationAddress = "0000";
////    sendMsg.SourceAddress = "0000";
////    while (!isChannelFree());
////    if (str.size() == 0)
////    {
////        sendMsg.Length = "0000";
////        sendMsg.FCS = "0";
////        QString tmp = sendMsg.Flag + bitStuff(sendMsg.getMsg().remove(0, 8));
////        for (int i = 0; i < tmp.length(); i++)
////        {
////            QThread::usleep(TIMER_VALUE);
////            int N = 1;
////            while (isCollision())
////            {
////                if (N == 11)
////                    break;
////                int L = QRandomGenerator::global()->bounded(qPow(2, N));
////                QThread::usleep(L * TIMER_VALUE);
////                ui->textStatus->insertPlainText("c");
////                N++;
////                while (!isChannelFree());
////            }
////            if (N != 11)
////            {
////                ui->textStatus->insertPlainText("\n");
////                QString symb = tmp[i];
////                send_port->write(symb.toUtf8());
////            }
////            else
////                i--;
////        }
////    }
////    else
////    {
////        while (str.size() > 0)
////        {
////            QString tmp = str.left(15);
////            sendMsg.Data = tmp;
////            sendMsg.Length = QString("%1").arg(tmp.size(), 4, 2).replace(' ', '0');
////            sendMsg.FCS = getFCS(sendMsg.Data);
////            tmp = sendMsg.Flag + bitStuff(sendMsg.getMsg().remove(0, 8));
////            for (int i = 0; i < tmp.length(); i++)
////            {
////                QThread::usleep(TIMER_VALUE);
////                int N = 1;
////                while (isCollision())
////                {
////                    if (N == 11)
////                        break;
////                    int L = QRandomGenerator::global()->bounded(qPow(2, N));
////                    QThread::usleep(L * TIMER_VALUE);
////                    ui->textStatus->insertPlainText("c");
////                    N++;
////                    while (!isChannelFree());
////                }
////                if (N != 11)
////                {
////                    ui->textStatus->insertPlainText("\n");
////                    QString symb = tmp[i];
////                    send_port->write(symb.toUtf8());
////                }
////                else
////                    i--;
////            }
////            str.remove(0, sendMsg.Length.toInt(0, 2));
////        }
////    }
//}

void MainWindow::serialReceive()
{
    ui->textOutput->moveCursor(QTextCursor::End);
    QString str = QString(receive_port->readAll());
    ui->textOutput->insertPlainText(str);
//    ui->textStatus->moveCursor(QTextCursor::End);
//    ui->textOutput->moveCursor(QTextCursor::End);
//    QByteArray message = receive_port->readAll();
//    QString str = QString(message);
//    QString out;
//    while (str.indexOf("01101110") != -1)
//    {
////        bool bs = false;
//        str.remove(0, str.indexOf("01101110"));
//        QString tmp = str.left(str.indexOf("01101110", 20));
//        str.remove(0, 8);
////        if (tmp.indexOf("1101", 8) != -1)
////        {
////            debug_bitStuff(tmp);
////            bs = true;
////        }
//        packet receiveMsg;
//        receiveMsg.Flag = tmp.first(8);
//        tmp.remove(0, 8);
//        tmp = de_bitStuff(tmp);
//        receiveMsg.DestinationAddress = tmp.first(4);
//        tmp.remove(0, 4);
//        receiveMsg.SourceAddress = tmp.first(4);
//        tmp.remove(0, 4);
//        receiveMsg.Length = tmp.first(4);
//        tmp.remove(0, 4);
//        receiveMsg.Data = tmp.first(receiveMsg.Length.toInt(0, 2));
//        tmp.remove(0, receiveMsg.Length.toInt(0, 2));
//        receiveMsg.FCS = tmp;
////        if (!bs)
////        {
////            if (receiveMsg.Data != "")
////                receiveMsg.Data = corruptData(receiveMsg.Data);
////            debug_FCS(receiveMsg);
////            if (receiveMsg.Data != "")
////                receiveMsg.Data = recoverData(receiveMsg.Data, receiveMsg.FCS);
////        }
//        out.push_back(receiveMsg.Data);
//    }
//    ui->textOutput->append(out);
}

QString MainWindow::bitStuff(QString str)
{
    QString res;
    while (str.indexOf("1101") != -1)
    {
        res.append(str.first(str.indexOf("1101") + 4));
        str.remove(0, str.indexOf("1101") + 4);
        res.append("0");
    }
    res.append(str);
    return res;
}

QString MainWindow::de_bitStuff(QString str)
{
    QString res;
    while (str.indexOf("1101") != -1)
    {
        res.append(str.first(str.indexOf("1101") + 4));
        str.remove(0, str.indexOf("1101") + 5);
    }
    res.append(str);
    return res;
}

void MainWindow::debug_bitStuff(QString str)
{
    ui->textStatus->insertPlainText(str.first(8));
    str.remove(0, 8);
    while (str.indexOf("1101") != -1)
    {
        ui->textStatus->insertPlainText(str.first(str.indexOf("1101") + 4));
        str.remove(0, str.indexOf("1101") + 4);
        ui->textStatus->setFontUnderline(true);
        ui->textStatus->insertPlainText(str.first(1));
        str.remove(0, 1);
        ui->textStatus->setFontUnderline(false);
    }
    ui->textStatus->insertPlainText(str + "\n");
}

QString MainWindow::getFCS(QString data)
{
    QString fcs;
    int length = data.length();
    for (int i = 1; i <= length; i *= 2)
    {
        int sum = 0;
        for (int j = i - 1; j < length; j += 2 * i)
            for (int k = j, t = 0; t < i && k < length; k++, t++)
                if (data[k] == '1')
                    sum++;
        fcs.push_back(QString::number(sum % 2));
    }
    int sum = 0;
    for (int i = 0; i < length; i++)
        if (data[i] == '1')
            sum++;
    fcs.push_back(QString::number(sum % 2));
    return fcs;
}

QString MainWindow::corruptData(QString data)
{
    if (QRandomGenerator::global()->bounded(2) == 0)
    {
        int i = QRandomGenerator::global()->bounded(data.length());
        if (data[i] == '0')
            data[i] = '1';
        else
            data[i] = '0';
    }
    else if (data.length() > 1 && QRandomGenerator::global()->bounded(2) == 0)
    {
        int i = QRandomGenerator::global()->bounded(data.length());
        int j = i;
        while (i == j)
            j = QRandomGenerator::global()->bounded(data.length());
        if (data[i] == '0')
            data[i] = '1';
        else
            data[i] = '0';
        if (data[j] == '0')
            data[j] = '1';
        else
            data[j] = '0';
    }
    return data;
}

QString MainWindow::recoverData(QString data, QString fcsOld)
{
    QString fcsNew = getFCS(data);
    if (fcsNew != fcsOld && fcsNew.back() != fcsOld.back())
    {
        int ind = 0;
        for (int i = 0; i < fcsNew.length() - 1; i++)
        {
            if (fcsNew[i] != fcsOld[i])
                ind += qPow(2, i);
        }
        if (data[ind - 1] == '0')
            data[ind - 1] = '1';
        else
            data[ind - 1] = '0';
    }
    return data;
}

void MainWindow::debug_FCS(packet msg)
{
    ui->textStatus->insertPlainText(msg.Flag);
    ui->textStatus->insertPlainText(msg.DestinationAddress);
    ui->textStatus->insertPlainText(msg.SourceAddress);
    ui->textStatus->insertPlainText(msg.Length);
    ui->textStatus->insertPlainText(msg.Data);
    ui->textStatus->setTextColor(Qt::red);
    ui->textStatus->insertPlainText(msg.FCS);
    ui->textStatus->setTextColor(Qt::black);
    ui->textStatus->insertPlainText("\n");
}

bool MyThread::isCollision()
{
    if (QRandomGenerator::global()->bounded(4) == 0)
        return true;
    else
        return false;
}

bool MyThread::isChannelFree()
{
    if (QRandomGenerator::global()->bounded(4) == 0)
        return true;
    else
        return false;
}

