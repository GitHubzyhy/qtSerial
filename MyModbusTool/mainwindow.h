#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class QModbusClient;
class QModbusServer;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void InitUI();//初始化UI
public slots:
    void on_mType_currentIndexChanged(const QString &arg1);//改变master连接方式
    void on_sType_currentTextChanged(const QString &arg1);//改变slaver连接方式
    void on_mConBtn_clicked();//master的连接按钮

    //功能码
    void on_m03Btn_clicked();
    void on_m06Btn_clicked();
    void on_m16Btn_clicked();

    void on_sConBtn_clicked();//slaver的连接按钮

    void on_tableSlave_cellChanged(int row, int column);//slaver表被编辑后
    void on_sTable_currentIndexChanged(int index);//重新选择slaver表

private:
    Ui::MainWindow *ui;

    //定义主站父类对象，方便后面的new QModbusRtuSerialMaster  或者 new QModbusTcpClient
    QModbusClient *master = nullptr;

    //定义从站父类对象，方便后面的new QModbusRtuSerialSlave  或者 new QModbusTcpServer
    QModbusServer *slave = nullptr;
};
#endif // MAINWINDOW_H
