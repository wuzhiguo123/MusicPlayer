#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
typedef enum 
{
    STATE_IDLE,
    STATE_FIRST_PRESS,
    STATE_FIRST_RELEASE,
    STATE_SECOND_PRESS
}BUTTON_STATE;

BUTTON_STATE state = STATE_IDLE;
struct itimerval tv;

void Handle(int sig)
{
    printf("短按\n");
    state = STATE_IDLE;
}

int main()
{
    int fd = open("/dev/input/event1", O_RDONLY);

    signal(SIGALRM, Handle);
    if(fd == -1)
    {
        perror("OPEN ERROR");
        return -1;
    }

    struct input_event ev;
    struct timeval old,new;
    while(1)
    {
        int ret = read(fd, &ev, sizeof(ev));
        if(ret == -1)
        {
            perror("READ ERROR");
            continue;;
        }

        if(ev.type != EV_KEY)
            continue;

        if(ev.value == 1)
        {
            if(state == STATE_IDLE)
            {
                gettimeofday(&old, NULL);
                state = STATE_FIRST_PRESS;
            }
            else if(state == STATE_FIRST_RELEASE)
            {
                printf("双击\n");
                //多长时间触发一次定时器信号
                state = STATE_IDLE;
                tv.it_value.tv_sec = 0;
                tv.it_value.tv_usec = 0;

                //多长时间重复启动一次定时器
                tv.it_interval.tv_sec = 0;
                tv.it_interval.tv_usec = 0;
                setitimer(ITIMER_REAL, &tv, NULL);
            }

        }
        else if(ev.value == 0)
        {
            if(state == STATE_FIRST_PRESS)
            {
                gettimeofday(&new,NULL);
                if((new.tv_sec-old.tv_sec) * 1000 + (new.tv_usec-old.tv_usec)/1000 > 300)
                {
                    printf("长按\n");
                    state = STATE_IDLE;
                }
                else {
                    state = STATE_FIRST_RELEASE;
                    tv.it_value.tv_sec = 0;
					tv.it_value.tv_usec = 300 * 1000;

					tv.it_interval.tv_sec = 0;
					tv.it_interval.tv_usec = 0;

                    setitimer(ITIMER_REAL, &tv, NULL);
                }
            }
        }
    }
}