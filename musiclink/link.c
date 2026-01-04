#include "link.h"
#include <stdio.h>
#include <stdlib.h>

MusicNode* music_head = NULL;

int InitMusicLink()
{
    music_head = (MusicNode*)malloc(sizeof(MusicNode));
    if(music_head == NULL)
    {
        perror("malloc() error");
        exit(-1);
    }

    music_head->next = NULL;
    music_head->prev = NULL;

    return 0;

}