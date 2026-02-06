#include "widget.h"
#include "ui_widget.h"
#include "bind.h"
#include "player.h"
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    socket = new QTcpSocket;
    socket->connectToHost(QHostAddress("8.130.123.33"),8008);

    connect(socket,&QTcpSocket::connected,this,&Widget::HandleConnect);
    connect(socket,&QTcpSocket::disconnected,this,&Widget::HandleDisconnect);

    connect(socket,&QTcpSocket::readyRead,this,&Widget::ReciveReply);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::HandleConnect()
{
    QMessageBox::information(this,"连接提示","连接成功");
}

void Widget::HandleDisconnect()
{
    QMessageBox::warning(this,"连接提示","网络异常，断开连接");
}




void Widget::SocketSendData(QJsonObject& obj)
{
    QByteArray send_data;
    QByteArray obj_array = QJsonDocument(obj).toJson();

    int size = obj_array.size();
    send_data.insert(0,(const char*)&size,sizeof(int));
    send_data.append(obj_array);

    socket->write(send_data);
}



void Widget::on_usr_register_clicked()
{
    QString app_id = ui->appid->text();
    QString psd = ui->psd->text();

    QJsonObject obj;
    obj.insert("cmd","app_register");
    obj.insert("appid",app_id);
    obj.insert("password",psd);
    qDebug("注册");
    SocketSendData(obj);
}

void Widget::on_login_clicked()
{
    QString app_id = ui->appid->text();
    QString psd = ui->psd->text();

    QJsonObject obj;
    obj.insert("cmd","app_login");
    obj.insert("appid",app_id);
    obj.insert("password",psd);
    m_appid = app_id;
    SocketSendData(obj);
}

void Widget::ReciveReply()
{
    QByteArray rec_data;
    ReadRecData(rec_data);
    qDebug() << "[REC DATA:]" <<rec_data;
    QJsonObject obj = QJsonDocument::fromJson(rec_data).object();
    QString cmd = obj.value("cmd").toString();
    if(cmd == "app_register_reply")
    {
        HandleRegisterReply(obj);
    }

    if(cmd == "app_login_reply")
    {
        HandleLoginReply(obj);
    }

}

void Widget::ReadRecData(QByteArray& rec_data)
{
    char buffer[1024] = {0};
    qint64 size = 0;
    while(true)
    {
        size += socket->read(buffer+size,sizeof(int)-size);
        if(size == sizeof(int))
              break;
    }

    size = 0;
    int len = *(int*)buffer;
    while(true)
    {
        size += socket->read(buffer+size,len-size);
        if(size == len)
            break;
    }

    rec_data.append(buffer);
}

void Widget::HandleRegisterReply(QJsonObject obj)
{
    QString result = obj.value("result").toString();
    if(result == "success")
    {
        QMessageBox::information(this,"注册结果","注册成功");
    }
    else if(result == "failure")
    {
        QMessageBox::warning(this,"注册结果","注册失败");
    }
}
void Widget::HandleLoginReply(QJsonObject obj)
{
    QString result = obj.value("result").toString();
    if(result == "bind")
    {
//        QMessageBox::information(this,"登录结果","登录成功");
        socket->disconnect(SIGNAL(connected()));
        socket->disconnect(SIGNAL(disconnected()));
        socket->disconnect(SIGNAL(readyRead()));
        QString deviceid = obj.value("deviceid").toString();
        Player* player = new Player(socket,m_appid,deviceid);

        player->show();
        this->hide();
    }
    else if(result == "not_bind")
    {
        QMessageBox::warning(this,"登录结果","未绑定设备");
        socket->disconnect(SIGNAL(connected()));
        socket->disconnect(SIGNAL(disconnected()));
        socket->disconnect(SIGNAL(readyRead()));
        Bind* bind = new Bind(socket,m_appid);

        bind->show();
        this->hide();
    }
    else if(result == "not_exist")
    {
        QMessageBox::warning(this,"登录结果","用户不存在，请先注册");
    }
    else if(result == "password_error")
    {
        QMessageBox::warning(this,"登录结果","密码错误");
    }
}




