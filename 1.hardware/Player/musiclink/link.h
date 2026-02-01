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
int LinkMusicList(const char*);
void ClearMusicList();
void InsertMusic(const char* name);
#endif