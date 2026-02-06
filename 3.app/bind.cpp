#include "bind.h"
#include "ui_bind.h"
#include "player.h"
Bind::Bind(QTcpSocket* socket,QString appid,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Bind)
{
    ui->setupUi(this);
    bind_socket = socket;
    bind_appid = appid;
    connect(socket,&QTcpSocket::connected,this,[this](){QMessageBox::information(this,"连接提示","连接成功");});
    connect(socket,&QTcpSocket::disconnected,this,[this](){QMessageBox::warning(this,"连接提示","断开连接");});

    connect(socket,&QTcpSocket::readyRead,this,&Bind::ReciveReply);
}

Bind::~Bind()
{
    delete ui;
}



void Bind::on_pushButton_clicked()
{
    QJsonObject value;
    QString appid = bind_appid;
    bind_deviceid = ui->deviceid->text();
    value.insert("appid",appid);
    value.insert("cmd","app_bind");
    value.insert("deviceid",bind_deviceid);

    SocketSendData(value);

}

void Bind::ReciveReply()
{
    QByteArray rec_data;
    ReadRecData(rec_data);
    qDebug() << "[REC DATA:]" <<rec_data;
    QJsonObject obj = QJsonDocument::fromJson(rec_data).object();
    QString cmd = obj.value("cmd").toString();
    QString result = obj.value("result").toString();
    if(cmd == "app_bind_reply")
    {
        if(result == "success")
        {
            //        QMessageBox::information(this,"绑定结果","绑定成功");
            bind_socket->disconnect(SIGNAL(connected()));
            bind_socket->disconnect(SIGNAL(disconnected()));
            bind_socket->disconnect(SIGNAL(readyRead()));
            Player* player = new Player(bind_socket,bind_appid,bind_deviceid);

            player->show();
            this->hide();
        }

        else if(result == "failure")
        QMessageBox::warning(this,"绑定结果","绑定失败");
    }


}

void Bind::ReadRecData(QByteArray& rec_data)
{
    char buffer[1024] = {0};
    qint64 size = 0;
    while(true)
    {
        size += bind_socket->read(buffer+size,sizeof(int)-size);
        if(size == sizeof(int))
              break;
    }

    size = 0;
    int len = *(int*)buffer;
    while(true)
    {
        size += bind_socket->read(buffer+size,len-size);
        if(size == len)
            break;
    }

    rec_data.append(buffer);
}


void Bind::SocketSendData(QJsonObject& obj)
{
    QByteArray send_data;
    QByteArray obj_array = QJsonDocument(obj).toJson();

    int size = obj_array.size();
    send_data.insert(0,(const char*)&size,sizeof(int));
    send_data.append(obj_array);

    bind_socket->write(send_data);
}
