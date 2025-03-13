#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include "myserialport.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class QSerialPort;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initUI();//初始化界面控件
    void initSerial();//初始化串口

private slots:
    void on_openButton_clicked();//打开串口
    void on_sendButton_clicked();//发送数据
    void on_checkBox_stateChanged(int arg1);//是否开启定时器发送

signals:
    //定义一些信号，方便去调用线程中的 MySerialPort中的槽函数
    void sigStart(MySerialPort::Settings s);
    void sigStop();
    void sigSend(QByteArray data);

private:
    Ui::MainWindow *ui;

    MySerialPort *m_serial;//定义串口类的对象
    QThread m_thread;
    QTimer m_timer;//定时器对象
};
#endif // MAINWINDOW_H
