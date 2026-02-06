#include <jsoncpp/json/value.h>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <thread>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int sockfd;
auto SendInfo = [](Json::Value value){
    while(true)
    {
        char buffer[1024] = {0};
        std::string str = Json::FastWriter().write(value);
        int len = str.size();
        memcpy(buffer, &len, sizeof(int));
        memcpy(buffer+sizeof(int), str.c_str(), len);
        write(sockfd,buffer , sizeof(int) + len);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

};

void SendOnceToServer(Json::Value& value)
{
    char buffer[1024] = {0};
    std::string str = Json::FastWriter().write(value);
    int len = str.size();
    memcpy(buffer, &len, sizeof(int));
    memcpy(buffer+sizeof(int), str.c_str(), len);
    write(sockfd,buffer , sizeof(int) + len);
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

auto RevMsg = [](){
    while(true)
    {
        char buffer[1024] = {0};
        ssize_t r = 0;
        int len = sizeof(int);
        while(true)
        {
            r += read(sockfd, buffer+r, len-r);
            if(r == len)
                break;
        }

        r = 0;
        int msg_len = *(int*)buffer;
        memset(buffer, 0, sizeof(buffer));
        while(true)
        {
             r += read(sockfd, buffer+r, msg_len-r);
            if(r == msg_len)
                break;
        }
        std::cout << "[APP REC]" << buffer <<std::endl;
        
    }

};

int main()
{
   

    Json::Value value;
    value["cmd"] = "app_info";
    value["appid"] = "1001";
    value["deviceid"] = "0001";

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("172.29.2.94");
    server_addr.sin_port = htons(8008);

    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(sockaddr_in)) < 0)
    {
        std::cout << "connect error" <<std::endl;
    }
    std::thread SendLoop(SendInfo,value);
    std::thread ReviceLoop(RevMsg);
    SendLoop.detach();
    ReviceLoop.detach();

    // sleep(5);

    // value["cmd"] = "app_start";
    // value["appid"] = "1001";
    // value["deviceid"] = "0001";
    // SendOnceToServer(value);

    sleep(1);
    Json::Value value_register;
    value_register["cmd"] = "app_offline";
    value_register["appid"] = "1001";
    value_register["deviceid"] = "0001";
    SendOnceToServer(value_register);
    while(true){}
    
}