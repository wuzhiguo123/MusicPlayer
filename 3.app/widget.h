#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    void HandleConnect();
    void HandleDisconnect();
    ~Widget();

private slots:
    void on_usr_register_clicked();
    void ReciveReply();
    void on_login_clicked();

private:
    void SocketSendData(QJsonObject& obj);
    void ReadRecData(QByteArray& rec_data);
    void HandleRegisterReply(QJsonObject);
    void HandleLoginReply(QJsonObject);
private:
    Ui::Widget *ui;
    QString m_appid;
    QTcpSocket* socket;
};
#endif // WIDGET_H
