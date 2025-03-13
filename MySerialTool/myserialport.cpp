#include "myserialport.h"

MySerialPort::MySerialPort(QObject *parent)
    : QSerialPort{parent}
{
    Read();
}

//读取数据
void MySerialPort::Read()
{
    connect(this, &QSerialPort::readyRead, [this]
    {
        //收到串口数据
        QByteArray arr = readAll();
        emit sigReceived(arr);
    });
}

//开启一个串口
void MySerialPort::Start(Settings sets)
{
    //设置串口参数
    QSerialPort::setPortName(sets.name);
    QSerialPort::setParity(sets.parity);
    QSerialPort::setBaudRate(sets.baudRate);
    QSerialPort::setDataBits(sets.dataBits);
    QSerialPort::setStopBits(sets.stopBits);
    QSerialPort::setFlowControl(sets.flowControl);

    //打开串口
    if (QSerialPort::open(QIODevice::ReadWrite))
        emit sigStarted();
    else
        emit sigStoped(1);//打开失败了关闭传1

}

//停止一个串口
void MySerialPort::Stop()
{
    //关闭串口
    if (QSerialPort::isOpen())
        QSerialPort::close();

    //正常停止传0
    emit sigStoped(0);
}

//向串口写数据
void MySerialPort::Send(QByteArray data)
{
    if (QSerialPort::isOpen())
        QSerialPort::write(data);
}
