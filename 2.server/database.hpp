#ifndef DATABASE_HPP
#define DATABASE_HPP
#include <jsoncpp/json/value.h>
#include <mysql/mysql.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <mysql/mysql.h>
#include <iostream>
#include <event.h>
#include <stdio.h>


class Server;
class DataBase
{
public:
private:
    MYSQL* mysql;//数据库句柄
public:
    DataBase();
    ~DataBase();
    bool DatabaseConnect();
    void DatabaseDisconnect();
    bool DatabaseInitTable();
    bool RegisterUsr(struct bufferevent *bev,Json::Value value,Server* s);
    bool CheckUsrExist(Json::Value value);
    bool AddUsr(Json::Value value);
    bool CheckPassword(Json::Value value);
    bool CheckBind(Json::Value value,std::string& deviceid);
    bool AppBindDev(Json::Value value);

};
#endif