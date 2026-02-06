#ifndef BIND_H
#define BIND_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
namespace Ui {
class Bind;
}

class Bind : public QWidget
{
    Q_OBJECT

public:
    explicit Bind(QTcpSocket* socket,QString appid,QWidget *parent = nullptr);
    ~Bind();

private slots:
    void on_pushButton_clicked();
    void ReciveReply();
private:
    void ReadRecData(QByteArray& rec_data);
    void SocketSendData(QJsonObject& obj);
private:
    Ui::Bind *ui;
    QTcpSocket* bind_socket;
    QString bind_appid;
    QString bind_deviceid;
};

#endif // BIND_H
