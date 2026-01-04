#ifndef __LINK__C
#define __LINK__C
#include <stdio.h>
typedef struct MusicNode
{
    char music_name[127];
    struct MusicNode* prev;
    struct MusicNode* next;
}MusicNode;

int InitMusicLink();
#endif