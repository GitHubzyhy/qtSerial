#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initUI();//初始化UI
    initSerial();//初始化串口

    //注册Settings，以便在信号槽中作为参数使用
    qRegisterMetaType<MySerialPort::Settings>("MySerialPort::Settings");
}

MainWindow::~MainWindow()
{
    //关闭串口
    emit sigStop();
    m_thread.quit();//退出线程的消息循环
    m_thread.wait();//等待线程结束
    delete m_serial;
    m_serial = nullptr;

    delete ui;
}

//初始化串口
void MainWindow::initSerial()
{
    m_serial = new MySerialPort(nullptr);
    m_serial->moveToThread(&m_thread);
    m_thread.start();//启动线程

    //连接响应的信号与槽,  界面主动发送的信号
    connect( this, &MainWindow::sigStart, m_serial, &MySerialPort::Start);
    connect( this, &MainWindow::sigStop, m_serial, &MySerialPort::Stop);
    connect( this, &MainWindow::sigSend, m_serial, &MySerialPort::Send);

    //串口将事件处理完后，界面要更新
    connect(m_serial, &MySerialPort::sigStarted, this, [this]()
    {
        ui->openButton->setText(QStringLiteral("关闭串口"));
        ui->groupBox->setEnabled(false);
    });
    connect(m_serial, &MySerialPort::sigStoped, this, [this]()
    {
        ui->openButton->setText(QStringLiteral("打开串口"));
        ui->groupBox->setEnabled(true);
    });
    connect(m_serial, &MySerialPort::sigReceived, this, [this](QByteArray data)
    {

        QString t = QDateTime::currentDateTime()
                    .toString("yyyy-MM-dd hh:mm:ss.zzz : ");

        ui->recvTextEdit->appendPlainText( t + data + "\n" ); //追加到末尾
    });
    //连接定时器的信号与槽
    connect(&m_timer, &QTimer::timeout, this, [this]()
    {
        on_sendButton_clicked();//发送数据
    });
}

//初始化UI
void MainWindow::initUI()
{
    //获取系统存在的串口
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        ui->portName->addItem(info.portName());
    //波特率
    ui->baudRate->addItem("1200", QSerialPort::Baud1200 );
    ui->baudRate->addItem("2400", QSerialPort::Baud2400 );
    ui->baudRate->addItem("4800", QSerialPort::Baud4800 );
    ui->baudRate->addItem("9600", QSerialPort::Baud9600 );
    ui->baudRate->addItem("19200", QSerialPort::Baud19200 );

    //数据位
    ui->dataBits->addItem("8", QSerialPort::Data8);
    ui->dataBits->addItem("7", QSerialPort::Data7);
    ui->dataBits->addItem("6", QSerialPort::Data6);
    ui->dataBits->addItem("5", QSerialPort::Data5);

    //奇偶校验
    ui->parity->addItem("None", QSerialPort::NoParity);
    ui->parity->addItem("Even", QSerialPort::EvenParity);
    ui->parity->addItem("Odd", QSerialPort::OddParity);

    //停止位
    ui->stopBits->addItem("1", QSerialPort::OneStop);
    ui->stopBits->addItem("1.5", QSerialPort::OneAndHalfStop);
    ui->stopBits->addItem("2", QSerialPort::TwoStop);

    //流控制
    ui->flowControl->addItem("None", QSerialPort::NoFlowControl);
    ui->flowControl->addItem("RTS/CTS", QSerialPort::HardwareControl);
    ui->flowControl->addItem("XON/XOFF", QSerialPort::SoftwareControl);

    //定时器的默认时间间隔
    ui->timelineSpinBox->setValue(1000);
}

void MainWindow::on_openButton_clicked()
{
    QString text = ui->openButton->text();
    if (text == QStringLiteral("打开串口"))
    {
        MySerialPort::Settings s;
        s.name = ui->portName->currentText();
        s.baudRate = (QSerialPort::BaudRate)ui->baudRate->currentData().toInt();
        s.dataBits = (QSerialPort::DataBits) ui->dataBits->currentData().toInt();
        s.stopBits = (QSerialPort::StopBits) ui->stopBits->currentData().toInt();
        s.parity = (QSerialPort::Parity) ui->parity->currentData().toInt();
        s.flowControl = (QSerialPort::FlowControl) ui->flowControl->currentData().toInt();

        //发信号在线程中打开串口
        emit sigStart(s);
    }
    else
        emit sigStop();//发信号在线程中关闭串口

}

void MainWindow::on_sendButton_clicked()
{
    QByteArray arr = ui->sendTextEdit->toPlainText()
                     .toUtf8();
    //发信号在线程中写串口数据
    emit sigSend(arr);
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    if (arg1) //勾选了
        m_timer.start(ui->timelineSpinBox->value());
    else
        m_timer.stop();
}
