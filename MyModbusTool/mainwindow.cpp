#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QModbusDataUnit>
#include <QModbusClient>
#include <QModbusRtuSerialMaster>
#include <QModbusTcpClient>

#include <QModbusServer>
#include <QModbusRtuSerialSlave>
#include <QModbusTcpServer>

#include <QSerialPort>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    InitUI();
}

MainWindow::~MainWindow()
{
    delete ui;

    if (master)//已经分配了内存
    {
        //断开连接
        master->disconnectDevice();

        delete master;
        master = nullptr;
    }

    if (slave)
    {
        //断开连接
        slave->disconnectDevice();
        delete slave;
        slave = nullptr;
    }
}

//初始化UI
void MainWindow::InitUI()
{
    /*主站UI*/
    //表格设置为10行，2列
    ui->tableMaster->setRowCount(10);
    ui->tableMaster->setColumnCount(2);

    QStringList hStrs;
    hStrs << "Alisa" << "00000";
    ui->tableMaster->setHorizontalHeaderLabels(hStrs);

    QStringList vStrs;
    vStrs << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9";
    ui->tableMaster->setVerticalHeaderLabels(vStrs);

    //初始化每个表格单元格的内容
    for (int i = 0; i < 10; ++i)
    {
        ui->tableMaster->setItem(i, 0, new QTableWidgetItem(""));
        ui->tableMaster->setItem(i, 1, new QTableWidgetItem(""));
    }

    //通讯方式下拉框
    ui->mType->addItem("SerialPort");
    ui->mType->addItem("TCP");

    //设置从站默认地址
    ui->mAddr->setText("1");

    /*从站UI*/
    //表格设置为10行，2列
    ui->tableSlave->setRowCount(10);
    ui->tableSlave->setColumnCount(2);

    hStrs.clear();
    hStrs << "Alisa" << "00000";
    ui->tableSlave->setHorizontalHeaderLabels(hStrs);

    vStrs.clear();
    vStrs << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9";
    ui->tableSlave->setVerticalHeaderLabels(vStrs);

    //初始化每个表格单元格的内容
    for (int i = 0; i < 10; ++i)
    {
        ui->tableSlave->setItem(i, 0, new QTableWidgetItem(""));
        ui->tableSlave->setItem(i, 1, new QTableWidgetItem(""));
    }

    //通讯方式下拉框
    ui->sType->addItem("SerialPort");
    ui->sType->addItem("TCP");

    //设置从站默认地址
    ui->sAddr->setText("1");

    //从站表类型下拉框选项
    ui->sTable->addItem( "Coils", QModbusDataUnit::Coils );
    ui->sTable->addItem( "InputRegisters", QModbusDataUnit::InputRegisters );
    ui->sTable->addItem( "DiscreteInputs", QModbusDataUnit::DiscreteInputs );
    ui->sTable->addItem( "HoldingRegisters", QModbusDataUnit::HoldingRegisters );
}

//改变master连接方式
void MainWindow::on_mType_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    if (ui->mType->currentText() == "TCP")
    {
        ui->mIP_COM->setText("127.0.0.1");
        ui->m_Port->setText("4000");
    }
    else
    {
        ui->mIP_COM->setText("COM1");
        ui->m_Port->setText("");
    }
}

//改变slaver连接方式
void MainWindow::on_sType_currentTextChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    if (ui->sType->currentText() == "TCP")
    {
        ui->sIP_COM->setText("127.0.0.1");
        ui->s_Port->setText("4000");
    }
    else
    {
        ui->sIP_COM->setText("COM2");
        ui->s_Port->setText("");
    }
}

//master的连接按钮
void MainWindow::on_mConBtn_clicked()
{
    QString strText = ui->mConBtn->text();
    if (strText == QStringLiteral("连接"))
    {
        //Modbus RTU
        if (ui->mType->currentText() == "SerialPort")
        {
            //new 主站对象
            master = new QModbusRtuSerialMaster();

            //设置串口参数
            master->setConnectionParameter (//设置串口名称
                QModbusDevice::SerialPortNameParameter, ui->mIP_COM->text());
            master->setConnectionParameter(//设置校验位
                QModbusDevice::SerialParityParameter, QSerialPort::EvenParity);
            master->setConnectionParameter(//设置波特率
                QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud9600);
            master->setConnectionParameter(//设置数据位
                QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
            master->setConnectionParameter(//设置停止位
                QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);
        }
        else //Modbus TCP
        {
            //new 主站对象
            master = new QModbusTcpClient();

            //走的网络,设置IP与端口
            master->setConnectionParameter(
                QModbusDevice::NetworkAddressParameter, ui->mIP_COM->text() );
            master->setConnectionParameter(
                QModbusDevice::NetworkPortParameter, ui->m_Port->text().toUInt() );
        }

        master->setTimeout(1000);//设置连接超时时间1s
        master->setNumberOfRetries(3);//失败后重试的次数

        //连接信号槽
        connect(master, &QModbusDevice::stateChanged
                , this, [this]()
        {
            //成连接状态了
            if (master->state() == QModbusDevice::ConnectedState)
            {
                ui->mConBtn->setText(QStringLiteral("断开"));
                ui->statusbar->showMessage("Connect Success!", 3000);
            }
            else
                ui->mConBtn->setText(QStringLiteral("连接"));
        });

        // 设备发生错误时信号、槽
        connect(master, &QModbusClient::errorOccurred, [this](QModbusDevice::Error error)
        {
            Q_UNUSED(error);
            statusBar()->showMessage(master->errorString(), 3000);
            ui->mConBtn->setText(QStringLiteral("连接"));
        });

        //连接设备
        master->connectDevice();
    }
    else
    {
        if (master)//已经分配了内存
        {
            //断开连接
            master->disconnectDevice();

            delete master;
            master = nullptr;
        }
    }
}

//03功能码
void MainWindow::on_m03Btn_clicked()
{
    /*原始请求*/
    // if (master == nullptr) return;

    // //向从站发送QModbusRequest请求
    // QModbusRequest mr(QModbusPdu::ReadHoldingRegisters );//创建一个读取保持寄存器的请求，功能码为 0x03。
    // //0000：起始寄存器地址（高字节和低字节）。000A：寄存器数量（10 个寄存器）。
    // mr.setData(QByteArray::fromHex("0000000A"));

    // //向从站发送raw原始请求
    // if ( QModbusReply *reply = master->sendRawRequest(mr, ui->mAddr->text().toUInt()))
    // {
    // //如果还没完成回复
    // if (!reply->isFinished())
    // {
    // //绑定回复完成的信号
    // connect( reply, &QModbusReply::finished, this, [this, reply]
    // {
    // //如果没有错误
    // if (reply->error() == QModbusDevice::NoError)
    // {
    // //得到原始回复
    // QModbusResponse response = reply->rawResult();
    // //得到功能码
    // QModbusPdu::FunctionCode code = response.functionCode();
    // //然后得到数据部分
    // QByteArray arr = response.data();

    // //第一个字节表示后面的数据长度
    // int len = arr[0];

    // //每个数据占两个字节
    // for (int i = 0; i < len / 2; ++i)
    // {
    // unsigned char high = arr[2 * i + 1];
    // unsigned char low = arr[2 * i + 2];
    // //得到寄存器的值
    // quint16 v = (high << 8 ) | low;
    // //设置到表格
    // ui->tableMaster->item(i, 1)->setText( QString::number(v));
    // }

    // ui->statusbar->showMessage("ReadHoldingRegisters Success!", 5000);

    // }
    // else
    // ui->statusbar->showMessage(reply->errorString(), 5000);

    // //通过事件循环去延迟删除
    // reply->deleteLater();
    // });
    // }
    // }
    // else
    // {
    // //reply是NULL
    // ui->statusbar->showMessage(master->errorString(), 5000);
    // }

    /*高层API请求*/
    if (master == nullptr) return;

    //保持寄存器+ 起始位置+读取个数
    QModbusDataUnit mdu(QModbusDataUnit::HoldingRegisters, 0, 10);

    //读请求
    if (QModbusReply *reply = master->sendReadRequest(mdu, ui->mAddr->text().toUInt()))
    {
        //请求未完成
        if (!reply->isFinished())
        {
            //绑定回复完成的信号
            connect( reply, &QModbusReply::finished, this, [this, reply]
            {
                //如果没有错误
                if (reply->error() == QModbusDevice::NoError)
                {
                    //得到普通回复
                    QModbusDataUnit unit = reply->result();

                    for (uint i = 0; i < unit.valueCount(); ++i)
                    {
                        //得到该位置的值
                        quint16 v = unit.value(i);

                        //设置到表格
                        ui->tableMaster->item(unit.startAddress() + i, 1)->setText( QString::number(v));

                    }
                    ui->statusbar->showMessage("ReadHoldingRegisters Success!", 5000);

                }
                else
                    ui->statusbar->showMessage(reply->errorString(), 5000);

                //通过事件循环去延迟删除
                reply->deleteLater();
            });
        }
    }
    else
        //reply是NULL
        ui->statusbar->showMessage(master->errorString(), 5000);
}

//06功能码
void MainWindow::on_m06Btn_clicked()
{
    /*原始请求*/
    // if (master == nullptr) return;

    // //向从站发送QModbusRequest请求

    // //创建一个读取保持寄存器的请求，功能码为 0x06。
    // QModbusRequest mr(QModbusPdu::WriteSingleRegister );
    // //2字节的写入位置 0003 + 写入的2字节的值 666（0x029A）
    // mr.setData(QByteArray::fromHex("0003029A"));

    // //向从站发送raw原始请求
    // if ( QModbusReply *reply = master->sendRawRequest(mr, ui->mAddr->text().toUInt()))
    // {
    // //如果还没完成回复
    // if (!reply->isFinished())
    // {
    // //绑定回复完成的信号
    // connect( reply, &QModbusReply::finished, this, [this, reply]
    // {
    // //如果没有错误
    // if (reply->error() == QModbusDevice::NoError)
    // {
    // //得到原始回复
    // QModbusResponse response = reply->rawResult();
    // //得到功能码
    // QModbusPdu::FunctionCode code = response.functionCode();
    // //然后得到数据部分
    // QByteArray arr = response.data();

    // //06功能码，请求与应对报文一样的
    // unsigned char high1 = arr[0];
    // unsigned char low1 = arr[1];

    // unsigned char high2 = arr[2];
    // unsigned char low2 = arr[3];

    // quint16 pos = (high1 << 8 ) | low1;//写入位置
    // quint16 v = (high2 << 8 ) | low2;//写入的值

    // //设置到表格
    // ui->tableMaster->item(pos, 1)->setText( QString::number(v));

    // ui->statusbar->showMessage("WriteSingleRegister Success!", 5000);

    // }
    // else
    // ui->statusbar->showMessage(reply->errorString(), 5000);

    // //通过事件循环去延迟删除
    // reply->deleteLater();
    // });
    // }
    // }
    // else
    // //reply是NULL
    // ui->statusbar->showMessage(master->errorString(), 5000);

    /*高层API请求*/
    if (master == nullptr) return;

    //放入单个值
    QVector<quint16> vec;
    vec.push_back(666);

    //保持寄存器+ 起始位置+单个值
    QModbusDataUnit mdu(QModbusDataUnit::HoldingRegisters, 3, vec);

    //写请求
    if ( QModbusReply *reply = master->sendWriteRequest(mdu, ui->mAddr->text().toUInt() ))
    {
        //请求未完成
        if (!reply->isFinished())
        {
            //绑定回复完成的信号
            connect( reply, &QModbusReply::finished, this, [this, reply]
            {
                //如果没有错误
                if (reply->error() == QModbusDevice::NoError)
                {
                    //得到普通回复
                    QModbusDataUnit unit = reply->result();

                    //得到该位置的值
                    quint16 v = unit.value(0);

                    //设置到表格
                    ui->tableMaster->item(unit.startAddress(), 1)->setText( QString::number(v));

                    ui->statusbar->showMessage("WriteSingleRegister Success!", 5000);

                }
                else
                    ui->statusbar->showMessage(reply->errorString(), 5000);

                //通过事件循环去延迟删除
                reply->deleteLater();
            });
        }
    }
    else //reply是NULL
        ui->statusbar->showMessage(master->errorString(), 5000);
}

//16功能码
void MainWindow::on_m16Btn_clicked()
{
    /*原始请求*/
    // if (master == nullptr) return;

    // QByteArray arr;

    // //2字节的写入位置 0000(十六进制)
    // //(char)0 = 一个字节值为0
    // arr.push_back((char)0);// 高字节
    // arr.push_back((char)0);// 低字节

    // //2个字节的写入数量，10个值 000A(十六进制)
    // arr.push_back((char)0);
    // arr.push_back((char)10);

    // //1字节的 数据大小
    // arr.push_back((char)20);

    // //modbus中数据是大端存储的
    // for (int i = 0; i < 10; ++i)
    // {
    // QString text = ui->tableMaster->item(i, 1)->text();
    // if (text.isEmpty()) text = "0";

    // quint16 v = text.toUInt();

    // unsigned char high = v >> 8;//提取高字节
    // unsigned char low = v ;//提取低字节

    // arr.push_back(high);//高字节放到低地址
    // arr.push_back(low);//低字节放到高地址
    // }

    // //向从站发送QModbusRequest请求
    // //16功能码+ 数据
    // QModbusRequest mr(QModbusPdu::WriteMultipleRegisters, arr );

    // //向从站发送raw原始请求
    // if ( QModbusReply *reply = master->sendRawRequest(mr, ui->mAddr->text().toUInt()))
    // {
    // //如果还没完成回复
    // if (!reply->isFinished())
    // {
    // //绑定回复完成的信号
    // connect( reply, &QModbusReply::finished, this, [this, reply]
    // {
    // //如果没有错误
    // if (reply->error() == QModbusDevice::NoError)
    // {
    // //得到原始回复
    // QModbusResponse response = reply->rawResult();
    // //得到功能码
    // QModbusPdu::FunctionCode code = response.functionCode();
    // //然后得到数据部分
    // QByteArray arr = response.data();

    // //16功能码应答数据
    // // 两个字节的起始位置
    // unsigned char high1 = arr[0];
    // unsigned char low1 = arr[1];

    // //两个字节的写入数量
    // unsigned char high2 = arr[2];
    // unsigned char low2 = arr[3];

    // quint16 pos = (high1 << 8 ) | low1;
    // quint16 cnt = (high2 << 8 ) | low2;

    // ui->statusbar->showMessage(
    // QString("WriteMultipleRegisters  %1 , %2  Success!").arg(pos).arg(cnt), 5000);

    // }
    // else
    // ui->statusbar->showMessage(reply->errorString(), 5000);

    // //通过事件循环去延迟删除
    // reply->deleteLater();
    // });
    // }
    // }
    // else
    // //reply是NULL
    // ui->statusbar->showMessage(master->errorString(), 5000);

    /*高层API请求*/
    if (master == nullptr) return;

    //放入多个值
    QVector<quint16> vec;

    for (int i = 0; i < 10; ++i)
    {
        QString text = ui->tableMaster->item(i, 1)->text();
        if (text.isEmpty()) text = "0";

        quint16 v = text.toUInt();
        vec.push_back(v);
    }

    //保持寄存器+ 起始位置+多个值
    QModbusDataUnit mdu(QModbusDataUnit::HoldingRegisters, 0, vec);

    //写请求
    if ( QModbusReply *reply = master->sendWriteRequest(mdu, ui->mAddr->text().toUInt() ))
    {
        //请求未完成
        if (!reply->isFinished())
        {
            //绑定回复完成的信号
            connect( reply, &QModbusReply::finished, this, [this, reply]
            {
                //如果没有错误
                if (reply->error() == QModbusDevice::NoError)
                {
                    //得到普通回复
                    QModbusDataUnit unit = reply->result();

                    ui->statusbar->showMessage(
                        QString("WriteMultipleRegisters  %1 , %2  Success!").
                        arg(unit.startAddress()).arg(unit.valueCount()), 5000);
                }
                else
                    ui->statusbar->showMessage(reply->errorString(), 5000);

                //通过事件循环去延迟删除
                reply->deleteLater();
            });
        }
    }
    else//reply是NULL
        ui->statusbar->showMessage(master->errorString(), 5000);
}

//slaver的连接按钮
void MainWindow::on_sConBtn_clicked()
{
    QString strText = ui->sConBtn->text();

    if (strText == QStringLiteral("连接"))
    {
        //Modbus RTU
        if (ui->sType->currentText() == "SerialPort")
        {
            //new 从站对象
            slave = new QModbusRtuSerialSlave();

            //设置串口参数
            slave->setConnectionParameter (
                QModbusDevice::SerialPortNameParameter, ui->sIP_COM->text());
            slave->setConnectionParameter(
                QModbusDevice::SerialParityParameter, QSerialPort::EvenParity);
            slave->setConnectionParameter(
                QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud9600);
            slave->setConnectionParameter(
                QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
            slave->setConnectionParameter(
                QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);

        }
        else //Modbus TCP
        {
            //new 从站对象
            slave = new QModbusTcpServer();

            //走的网络,设置IP与端口
            slave->setConnectionParameter(
                QModbusDevice::NetworkAddressParameter, ui->sIP_COM->text() );
            slave->setConnectionParameter(
                QModbusDevice::NetworkPortParameter, ui->s_Port->text().toUInt() );
        }
        //设置从站地址
        slave->setServerAddress(ui->sAddr->text().toUInt());
        //连接信号槽
        connect(slave, &QModbusDevice::stateChanged, this, [this]()
        {
            //成连接状态了
            if (slave->state() == QModbusDevice::ConnectedState)
            {
                ui->sConBtn->setText(QStringLiteral("断开"));
                ui->statusbar->showMessage("Connect Success!", 3000);

                //设置从站的四张表
                QModbusDataUnitMap map;

                //在从站为线圈表 分配10个值的数据
                map.insert( QModbusDataUnit::Coils, QModbusDataUnit(QModbusDataUnit::Coils, 0, 10));
                map.insert( QModbusDataUnit::DiscreteInputs, QModbusDataUnit(QModbusDataUnit::DiscreteInputs, 0, 10));
                map.insert( QModbusDataUnit::InputRegisters, QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 10));
                map.insert( QModbusDataUnit::HoldingRegisters, QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 0, 10));

                //设置从站的map
                slave->setMap(map);

                // //提供HoldingRegisters一些测试值
                // slave->setData(QModbusDataUnit::HoldingRegisters, 0,101 );
                // slave->setData(QModbusDataUnit::HoldingRegisters, 1,102 );
                // slave->setData(QModbusDataUnit::HoldingRegisters, 2,103 );
            }
            else
                ui->sConBtn->setText(QStringLiteral("连接"));
        });
        //当主站向从站写入数据的时候，QModbusServer里面表的数据被改变，同时会发出信号
        connect(slave, &QModbusServer::dataWritten, this,
                [this ]( QModbusDataUnit::RegisterType table, int address, int size)
        {
            //同步更新UI
            for (int i = 0; i < size; ++i)
            {
                quint16 v = 0;

                //获取从站某张表里面某个位置的数据
                slave->data( table, address + i, &v );

                //取出来的值更新到从站UI
                ui->tableSlave->item( address + i, 1)->setText(QString::number(v));
            }
        });

        //连接设备
        slave->connectDevice();
    }
    else
    {
        if (slave)
        {
            //断开连接
            slave->disconnectDevice();
            delete slave;
            slave = nullptr;
        }
    }
}

//slaver表被编辑后
void MainWindow::on_tableSlave_cellChanged(int row, int column)
{
    if (slave == nullptr) return ;

    //获取被编辑的单元格的内容
    QString text = ui->tableSlave->item(row, column)->text();

    if (text.isEmpty()) text = "0";

    //将改变的值写入从站对应表中
    QModbusDataUnit::RegisterType registerType =
        ( QModbusDataUnit::RegisterType)ui->sTable->currentData().toUInt();
    slave->setData(registerType, row, text.toUInt() );
}

//重新选择slaver表
void MainWindow::on_sTable_currentIndexChanged(int index)
{
    Q_UNUSED(index);

    if (slave == nullptr) return ;

    //将改变的值写入从站对应表中
    QModbusDataUnit::RegisterType registerType =
        ( QModbusDataUnit::RegisterType)ui->sTable->currentData().toUInt();

    //将UI中的值全部设置到切换的表中去
    for (int i = 0; i < 10; ++i)
    {
        //获取被编辑的单元格的内容
        QString text = ui->tableSlave->item(i, 1)->text();

        if (text.isEmpty()) text = "0";
        slave->setData(registerType, i, text.toUInt() );
    }
}
