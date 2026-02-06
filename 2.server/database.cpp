#include "database.hpp"
#include "server.hpp"
#include <cstddef>
#include <jsoncpp/json/value.h>
#include <mysql/mysql.h>

DataBase::DataBase()
{}

DataBase::~DataBase()
{}

bool DataBase::DatabaseConnect()
{
    mysql = mysql_init(NULL);//初始化数据库句柄
    std::cout << "mysql_init:" << mysql <<std::endl;

    //向数据库发起连接
    MYSQL* tmp_mysql = mysql_real_connect(mysql, "localhost", "root", "root", "musicplayer", 0, nullptr, 0);
    if(tmp_mysql != nullptr)
        mysql=tmp_mysql;
    else
        return false;
    std::cout << "mysql_real_connect" <<mysql <<std::endl;
    
    if(mysql == nullptr)
    {
        std::cout << "[DATABASE CONNECT FAILURE] " << mysql_errno(mysql) <<std::endl;
        return false;
    }
    //把数据库的字符集设置成为UTF-8,支持中文
    if(mysql_query(mysql, "set names utf8;") !=0 )
    {
        std::cout << "字符集设置UTF=8失败" << std::endl;
        return false;
    }
    return true;
}

void DataBase::DatabaseDisconnect()
{
    mysql_close(mysql);
}

bool DataBase::DatabaseInitTable()
{
    if(!this->DatabaseConnect())
    {
        return false;
    }

    const char* sql = "create table if not exists account(appid char(11),password varchar(16),deviceid varchar(8))charset utf8;";

    mysql_query(mysql, sql);
    this->DatabaseDisconnect();
    return true;
}

 bool DataBase::RegisterUsr(struct bufferevent *bev,Json::Value value,Server* s)
 {
    Json::Value reply_value;
    reply_value["cmd"] = "app_register_reply";
    if(!DatabaseConnect())
    {
        debug("数据库连接失败");
        reply_value["result"] = "failure";
        s->SendData(bev, reply_value);
        return false;
    }
    if(CheckUsrExist(value))
    {
        debug("用户已经存在");
        reply_value["result"] = "failure";
        s->SendData(bev, reply_value);
        DatabaseDisconnect();
        return true;
    }
    else 
    {
        if(AddUsr(value))
            {
                debug("注册成功");
                reply_value["result"] = "success";
                s->SendData(bev, reply_value);
                DatabaseDisconnect();
                return true;
            }
        else 
        {
            debug("注册失败");
            reply_value["result"] = "failure";
            s->SendData(bev, reply_value);
            DatabaseDisconnect();   
            return true;
        }
    }
 }

 bool DataBase::CheckUsrExist(Json::Value value)
 {
    std::string appid = value["appid"].asString();
    char sql[256] = {0};
    sprintf(sql, "select * from account where appid = '%s';", appid.c_str());
    mysql_query(mysql, sql);
    MYSQL_RES* res = mysql_store_result(mysql);
    if(res == NULL)
        debug("mysql_store_result()");
    MYSQL_ROW row = mysql_fetch_row(res);
    if(row == NULL)
        return false;
    else
        return true;
 }

 bool DataBase::AddUsr(Json::Value value)
 {
    std::string appid = value["appid"].asString();
    std::string password = value["password"].asString();

    char sql[256] = {0};
    sprintf(sql, "insert into account (appid,password) values ('%s','%s');", appid.c_str(),password.c_str());
    if(mysql_query(mysql, sql) != 0)
        return false;
    else
        return true;;
 }

bool DataBase::CheckPassword(Json::Value value)
{
    char sql[256] = {0};
    std::string appid = value["appid"].asString();
    std::string password = value["password"].asString();
    sprintf(sql,"select password from account where appid = '%s';",appid.c_str());
    if(mysql_query(mysql, sql) != 0)
    {
        std::cout <<"mysql_query error" << mysql_error(mysql) <<std::endl;
        return false;
    }

    MYSQL_RES* res = mysql_store_result(mysql);
    if(res == nullptr)
    {
        std::cout <<"mysql_store_result error" << mysql_error(mysql) <<std::endl;
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if(row == NULL)
    {
        std::cout <<"mysql_fetch_row error" << mysql_error(mysql) <<std::endl;
        return false;
    }

    if(password == std::string(row[0]))
    {
        return true;
    }
    else 
    {
        return false;
    }
}


bool DataBase::CheckBind(Json::Value value,std::string& deviceid)
{
    char sql[256] = {0};
    std::string appid = value["appid"].asString();
    sprintf(sql,"select deviceid from account where appid = '%s';",appid.c_str());
    if(mysql_query(mysql, sql) != 0)
    {
        std::cout << "mysql_query error" << mysql_error(mysql) << std::endl;
        return false;
    }

    MYSQL_RES* res = mysql_store_result(mysql);
    if(res == nullptr)
    {
        std::cout << "mysql_store_result error" << mysql_error(mysql) << std::endl;
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if(row == NULL)
    {
       std::cout << "mysql_fetch_row error" << mysql_error(mysql) << std::endl;
       return false;   
    }

    if(row[0] != NULL)
    {
        deviceid = row[0];
        return true;
    }
    else
     return false;
}

bool DataBase::AppBindDev(Json::Value value)
{
    std::string appid = value["appid"].asString();
    std::string deviceid = value["deviceid"].asString();


    char sql[256] = {0};
    sprintf(sql,"update account set deviceid = '%s' where appid = '%s';",deviceid.c_str(),appid.c_str());
    if(mysql_query(mysql, sql) != 0)
    {
        std::cout << "[MYSQL QUERY ERROR] ";
		std::cout << mysql_error(mysql) << std::endl;
        return false;
    }
    return true;
}

