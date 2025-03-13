#ifndef MYSERIALPORT_H
#define MYSERIALPORT_H

#include<QSerialPort>

class MySerialPort : public QSerialPort
{
    Q_OBJECT
public:
    MySerialPort(QObject *parent);

    //串口参数结构体
    struct Settings
    {
        QString name;//端口名称COM1、COM2
        BaudRate baudRate;//波特率
        DataBits dataBits;//数据位
        Parity parity;//奇偶校验
        StopBits stopBits;//停止位
        FlowControl flowControl;//流控制
    };

    void Read();//读取数据

public slots://将在线程中执行的内容写成槽函数
    void Start(Settings sets);//开启一个串口
    void Stop();//停止一个串口
    void Send(QByteArray data);//向串口写数据

signals://向外面传递的一些信号
    void sigStarted();//串口打开了
    void sigStoped(int status);//串口停止了
    void sigReceived(QByteArray data);//串口收到数据了
};

#endif // MYSERIALPORT_H
