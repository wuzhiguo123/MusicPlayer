#ifndef _DEVICE_H
#define _DEVICE_H

#define DEFAULT_VOLUME 50
#define CARD_NAME "hw:AudioPCI"
#define SELE_NAME "Master"

int SetVolume(long volume);
int GetVolume(int* value);

#endif