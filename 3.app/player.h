#ifndef PLAYER_H
#define PLAYER_H


#include <QWidget>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QMessageBox>
#include <unistd.h>
#define SEQUENCE 1
#define CIRCLE 2
namespace Ui {
class Player;
}

class Player : public QWidget
{
    Q_OBJECT

public:
    explicit Player(QTcpSocket* socket,QString appid,QString deviceid,QWidget *parent = nullptr);
    ~Player();
private slots:
    void HandleTimerInfo();
    void HandleTimerGetMusiclist();
    void ReciveReply();
    void on_pauseButton_clicked();

    void on_nextPlayButton_clicked();

    void on_prevPlayButton_clicked();

    void on_UpVolumeButton_clicked();

    void on_downVolumButton_clicked();

    void on_seqButton_clicked();

    void on_cirButton_clicked();

private:
    void ReadRecData(QByteArray& rec_data);
    void SocketSendData(QJsonObject& obj);
    void HandleInfo(QJsonObject& obj);
    void HandleUpload(QJsonObject& obj);


private:
    Ui::Player *ui;
    QTcpSocket* player_socket;
    QString app_id;
    QString player_deviceid;
    QTimer* timer_info;
    QTimer* timer_musiclist;
};

#endif // PLAYER_H
