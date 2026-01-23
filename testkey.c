
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>
int main()
{
    int fd = open("/dev/input/event1", O_RDONLY);
    if(fd == -1)
    {
        perror("OPEN ERROR");
        return -1;
    }
    struct input_event ev;
    while(1)
    {
        int ret = read(fd, &ev, sizeof(ev));
        if(ret == -1)
        {
            perror("READ ERROR");
            continue;
        }
        if(ev.type != EV_KEY)
        {
            continue;
        }

        if(ev.value == 1)
        {
            printf("按下\n");
        }
        else if(ev.value == 0)
        {
            printf("松开\n");
        }
    }
    close(fd);
    return 0;
}