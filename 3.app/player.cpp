#include "player.h"
#include "ui_player.h"
int g_start_flag = 0;
int g_suspend_flag = 1;
Player::Player(QTcpSocket* socket,QString appid,QString deviceid,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Player)
{
    ui->setupUi(this);
    player_socket = socket;
    app_id = appid;
    player_deviceid = deviceid;
    timer_info = new QTimer();
    timer_info->start(2000);
    timer_musiclist = new QTimer();
    timer_musiclist->start(3000);
    connect(timer_info,&QTimer::timeout,this,&Player::HandleTimerInfo);
    connect(timer_musiclist,&QTimer::timeout,this,&Player::HandleTimerGetMusiclist);
    connect(player_socket,&QTcpSocket::disconnected,this,[this](){QMessageBox::warning(this,"离线提醒","设备已离线");});

    connect(player_socket,&QTcpSocket::readyRead,this,&Player::ReciveReply);

}

Player::~Player()
{
    delete ui;
}

void Player::HandleTimerInfo()
{
    QJsonObject obj;
    obj.insert("cmd","app_info");
    obj.insert("appid",app_id);
    obj.insert("deviceid",player_deviceid);
    SocketSendData(obj);
}

void Player::HandleTimerGetMusiclist()
{
    QJsonObject obj;
    obj.insert("cmd","app_capture_cur_musiclist");
    SocketSendData(obj);
    timer_musiclist->stop();
}

void Player::ReciveReply()//从服务器接收消息
{
    QByteArray rec_data;
    ReadRecData(rec_data);
    qDebug() << "[REC DATA:]" <<rec_data;
    QJsonObject obj = QJsonDocument::fromJson(rec_data).object();
    QString cmd = obj.value("cmd").toString();
    if(cmd == "info")
    {
        HandleInfo(obj);
    }
    else if(cmd == "upload_music")
    {
        HandleUpload(obj);
    }

}



void Player::ReadRecData(QByteArray& rec_data)//读取服务器的消息
{
    char buffer[1024] = {0};
    qint64 size = 0;
    while(true)
    {
        size += player_socket->read(buffer+size,sizeof(int)-size);
        if(size == sizeof(int))
              break;
    }

    size = 0;
    int len = *(int*)buffer;
    while(true)
    {
        size += player_socket->read(buffer+size,len-size);
        if(size == len)
            break;
    }

    rec_data.append(buffer);
}


void Player::SocketSendData(QJsonObject& obj)//给服务器发送消息
{
    QByteArray send_data;
    QByteArray obj_array = QJsonDocument(obj).toJson();

    int size = obj_array.size();
    send_data.insert(0,(const char*)&size,sizeof(int));
    send_data.append(obj_array);

    player_socket->write(send_data);
}

void Player::HandleInfo(QJsonObject &obj)
{
    QString cur_music = obj.value("cur_music").toString();
    QString status = obj.value("status").toString();
    QString deviceid = obj.value("deviceid").toString();
    int volume = obj.value("volume").toInt();
    int cur_mode = obj.value("mode").toInt();

    ui->curMusicLabel->setText(cur_music);
    if(status == "stop")
    {
        ui->pauseButton->setText(">");
        g_start_flag = 0;
    }
    else if(status == "suspend")
    {
        ui->pauseButton->setText(">");
        g_suspend_flag = 1;
    }
    else if(status == "start")
    {
        g_start_flag = 1;
        g_suspend_flag = 0;
        ui->pauseButton->setText("||");
    }
    ui->devIdLabel->setText(deviceid);
    ui->volumeLabel->setText(QString::number(volume));
    if(cur_mode == SEQUENCE)
    {
        ui->seqButton->setChecked(true);
    }
    else if(cur_mode == CIRCLE)
    {
        ui->cirButton->setChecked(true);
    }
}

void Player::HandleUpload(QJsonObject &obj)
{
    QString full_text; // 创建一个空字符串
    QJsonArray music_list = obj.value("music").toArray();

    for(int i = 0; i < music_list.size(); i++)
    {
        // 将歌名和换行符拼接到字符串中
        full_text += music_list.at(i).toString() + "\n";
    }

    // 循环结束后，一次性设置所有文本
    ui->MusicListText->setText(full_text);
}


void Player::on_pauseButton_clicked()
{
    QJsonObject val;
    if(g_start_flag == 0)
        val.insert("cmd","app_start");
    else if(g_start_flag == 1 && g_suspend_flag == 1)
        val.insert("cmd","app_continue");
    else if(g_start_flag == 1 && g_suspend_flag == 0)
        val.insert("cmd","app_suspend");
    SocketSendData(val);
}

void Player::on_nextPlayButton_clicked()
{
    QJsonObject val;
    val.insert("cmd","app_next");
    SocketSendData(val);
}

void Player::on_prevPlayButton_clicked()
{
    QJsonObject val;
    val.insert("cmd","app_prev");
    SocketSendData(val);
}

void Player::on_UpVolumeButton_clicked()
{
    QJsonObject val;
    val.insert("cmd","app_upvolume");
    SocketSendData(val);
}

void Player::on_downVolumButton_clicked()
{
    QJsonObject val;
    val.insert("cmd","app_downvolume");
    SocketSendData(val);
}

void Player::on_seqButton_clicked()
{
    QJsonObject val;
    val.insert("cmd","app_sequence");
    SocketSendData(val);
}

void Player::on_cirButton_clicked()
{
    QJsonObject val;
    val.insert("cmd","app_circle");
    SocketSendData(val);
}

void Player::closeEvent(QCloseEvent* e)
{
    QJsonObject val;
    val.insert("cmd","app_offline");
    SocketSendData(val);
    e->accept();
}
