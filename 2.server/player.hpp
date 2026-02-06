#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <jsoncpp/json/value.h>
#include <time.h>
#include <list>
#include <jsoncpp/json/json.h>
#include "debug.hpp"

class Server;
struct PLayerInfo
{
    std::string deviceid;
    std::string appid;
    std::string cur_music;
    int volume;
    int mode;
    time_t d_time;//设备上报时间
    time_t a_time;//APP上报时间

    struct bufferevent* d_bev;
    struct bufferevent* a_bev;
};
class Player{
public:
    Player();
    Json::Value GetMusicName(struct bufferevent *bev,std::string singer);
    void DevUpdateList(struct bufferevent *bev,Json::Value value,Server* s);
    void AppUpdateList(struct bufferevent *bev,Json::Value value,Server* s);

    void UploadMusic(struct bufferevent *bev,Json::Value value,Server* s);
    void TraverseList();
    void TransAppCmd(struct bufferevent *bev,Json::Value value,Server* s);
    void TransDevReply(struct bufferevent *bev,Json::Value value,Server* s);
    void AppOrDevOffline(struct bufferevent* bev);
    void AppOffline(struct bufferevent* bev);
    void CaptureCurMusicList(struct bufferevent *bev,Json::Value& val,Server* s);



    ~Player();
// private:
    std::list<struct PLayerInfo>* online_list;

};
#endif