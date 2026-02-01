#include <event2/event.h>
#include <stdint.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#define Ip "172.29.2.94"
#define Port 8000
class Server
{
public:
    Server();
    void Listen(const char* ,int16_t );
    static void ListenCb(struct evconnlistener *, int, struct sockaddr *, int, void *);
   // ~Server();
private:
    struct sockaddr_in server_addr;
    struct event_base* m_base;

};