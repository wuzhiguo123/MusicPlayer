#ifndef _DEVICE_H
#define _DEVICE_H

#define DEFAULT_VOLUME 80
#define CARD_NAME "hw:audiocodec"
#define SELE_NAME "lineout volume"

int SetVolume(long volume);
int GetVolume(int* value);
void InitButton();
void ReadButton();

typedef enum 
{
    STATE_IDLE,
    STATE_FIRST_PRESS,
    STATE_FIRST_RELEASE,
    STATE_SECOND_PRESS
}BUTTON_STATE;

#endif