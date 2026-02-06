#include "player.hpp"
#include "debug.hpp"
#include "server.hpp"
#include <cstddef>
#include <jsoncpp/json/value.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <iostream>


Player::Player()
{
    online_list = new std::list<struct PLayerInfo>;
}

Player::~Player()
{
    free(online_list);
}

Json::Value Player::GetMusicName(struct bufferevent *bev,std::string singer)
{  
    Json::Value val;
    Json::Value arr;
    std::list<std::string> music_list;

    std::string path(MUSIC_PATH);
    path += singer;

    std::cout << "path=" << path <<std::endl;

    DIR* dir = opendir(path.c_str());
    if(dir == nullptr)
    {
        std::cout << "opendir failure" <<std::endl;
        return val;
    }
    struct dirent *file;

    //把同一个歌手的歌曲全部塞到链表里面
    while ((file = readdir(dir))) 
    {
        if(file->d_type != DT_REG)
            continue;
        if(!strstr(file->d_name,".mp3"))
            continue;
        std::string music_name(singer);
        music_name += "/";
        music_name+=file->d_name;
        std::cout << music_name <<std::endl;

        music_list.push_back(music_name);
        std::cout << music_name <<std::endl;
    }

    //选择前五个返回给音箱
 
    auto it = music_list.begin();
    if(music_list.size() >=5)
    {
        for(int i = 0; i<5; i++,it++)
        {
            std::cout << *it <<std::endl;
            arr.append(*it);
        }
    }
    
    else
    {
          for(int i = 0; i< (int)music_list.size(); i++,it++)
        {
            std::cout << *it <<std::endl;
            arr.append(*it);
        }
    }

    val["cmd"] = "reply_music_list";
    val["music"] = arr;
    return val;
}

void Player::DevUpdateList(struct bufferevent *bev,Json::Value value,Server* s)
{
    auto it = online_list->begin();
    for(;it != online_list->end();it++)
    {
        //如果链表中已经存在了该设备的节点信息
        if(it->deviceid == value["deviceid"].asString())
        {
            std::cout << "[DevUpdateList]设备已经在线，更新节点信息" << std::endl;
            it->cur_music = value["cur_music"].asString();
            it->mode = value["mode"].asInt();
            it->volume = value["volume"].asInt();
            it->d_time = time(NULL);
            if(it->a_bev != nullptr)
            {
                std::cout << "[DevUpdateList]app在线,转发给APP" << std::endl;
                s->SendData(it->a_bev, value);
            }
            return;

        }
    }

    if(it == online_list->end())
    {
        PLayerInfo new_node;
        new_node.cur_music = value["cur_music"].asString();
        new_node.mode = value["mode"].asInt();
        new_node.volume = value["volume"].asInt();
        new_node.deviceid = value["deviceid"].asString();
        new_node.d_time = time(NULL);
        new_node.d_bev = bev;
        new_node.a_bev = nullptr;
        online_list->emplace_back(new_node);
        debug("[DevUpdateList]设备第一次上线，创建节点成功!");
    }
}

void Player::AppUpdateList(struct bufferevent *bev,Json::Value value,Server* s)
{
    auto it = online_list->begin();
    for(;it != online_list->end() ;it++)
    {
        if(value["deviceid"] == it->deviceid)
        {
            it->a_bev = bev;
            it->a_time = time(NULL);
            it->appid = value["appid"].asString();
            debug("[AppUpdateList]APP信息已绑定");
            return;
        }
    }
    debug("[AppUpdateList]您的音响未上线");

}

void Player::TraverseList()
{
    // debug("遍历链表");
    auto it = online_list->begin();
    for(;it != online_list->end();it++)
    {
        if(time(NULL) - it->d_time > 6)
        {
            online_list->erase(it);
        }

        if(it->a_bev)
        {
            if(time(NULL) - it->a_time > 6)
            {
                it->a_bev = NULL;
            }
        }
        std::cout << "a_bev:" << it->a_bev << "d_bev:" <<it->d_bev <<std::endl;
    }
}

void Player::UploadMusic(struct bufferevent *bev,Json::Value value,Server* s)
{
    auto it = online_list->begin();
    for(;it != online_list->end();it++)
    {
        std::cout << "------------------" << "a_bev" << it->a_bev  << "d_bev" << it->d_bev << "device_id" << it->deviceid<<std::endl;
        if(it->d_bev == bev)
        {
        std::cout << "++++++++" << "a_bev" << it->a_bev  << "d_bev" << it->d_bev<<std::endl;

            if(it->a_bev != nullptr)
            {
                debug("[UploadMusic]APP在线，转发给APP");
                std::cout << value <<std::endl;
                s->SendData(it->a_bev, value);
            }
            else 
            {
                debug("[UploadMusic]APP不在线，不转发");
            }
            break;
        }
    }
}

void Player::TransAppCmd(struct bufferevent *bev,Json::Value value,Server* s)
{
    auto it = online_list->begin();
    for(;it != online_list->end();it++)
    {
        if(it->a_bev == bev)
        {
            if(it->d_bev ==nullptr)
            {
                debug("[TransAppCmd]音响不在线");
            }
            else 
            {
                s->SendData(it->d_bev, value);//转发给音响
                debug("[TransAppCmd]已转发给音响");
                return;
            }
        }
    } 
    //如果音箱不在线，由服务器直接发回不在线给APP
    Json::Value reply_value;
    std::string cmd = reply_value["cmd"].asString();
    cmd += "_reply";
    reply_value["result"] = "offline";
    s->SendData(bev, reply_value);
}


void Player::TransDevReply(struct bufferevent *bev,Json::Value value,Server* s)
{
    auto it = online_list->begin();
    for(;it != online_list->end();it++)
    {
        if(it->d_bev == bev)
        {
            if(it->a_bev ==nullptr)
            {
                debug("[TransDevReply]App已离线");
            }
            else 
            {
                s->SendData(it->a_bev, value);//转发给音响
                debug("[TransDevReply]已转发给App");
                return;
            }
        }
    } 
    //如果App不在线，由服务器直接发回不在线给音响
    Json::Value reply_value;
    std::string cmd = reply_value["cmd"].asString();
    cmd += "_reply";
    reply_value["result"] = "offline";
    s->SendData(bev, reply_value);
}

void Player::AppOrDevOffline(struct bufferevent* bev)
{
    auto it = online_list->begin();
    for (;it != online_list->end();it++)
    {
        if(bev == it->a_bev)
        {
            it->a_bev = nullptr;
            debug("APP异常下线");
            break;
        }
        else if(bev == it->d_bev)
        {
            online_list->erase(it);
            break;
        }
    }
}

void Player::AppOffline(struct bufferevent* bev)
{
    auto it = online_list->begin();
    for (;it != online_list->end();it++)
    {
        if(bev == it->a_bev)
        {
            debug("APP正常下线");
            it->a_bev = nullptr;//bev没有被系统回收，只是置空
            break;
        }

    }
}

void Player::CaptureCurMusicList(struct bufferevent *bev,Json::Value& value,Server* s)
{
    auto it = online_list->begin();
    for (;it != online_list->end();it++)
    {
        if(bev == it->a_bev)
        {
            s->SendData(it->d_bev, value);

        }

    }
    
}

