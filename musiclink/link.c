#include "link.h"
#include "json_object.h"
#include "json_tokener.h"
#include "json_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
void InsertMusic(const char* name)
{
    if(music_head == NULL)
    {
        music_head = malloc(sizeof(MusicNode));
    }
    MusicNode* new = malloc(sizeof(MusicNode));
    strcpy(new->music_name, name);
    MusicNode* cur;
    cur = music_head;
    while(cur->next)
    {
        cur = cur->next;
    }
    cur->next = new;
    new->next = NULL;
    new->prev = cur;
}
int LinkMusicList(const char* music_name)
{
    struct json_object* obj = json_tokener_parse(music_name);
    if(obj == NULL)
    {
        fprintf(stderr,"[ERROR]This is not a jason object");
        return -1;
    }

    struct json_object* cmd = json_object_object_get(obj, "cmd");
    if(strcmp("reply_music_list", json_object_get_string(cmd)) != 0)
    {
        fprintf(stderr,"[ERROR] JSON FROMAT IS NOT VAILD");
        return -1;
    }

    struct json_object* music_array = json_object_object_get(obj, "music");
    for(int i = 0; i < json_object_array_length(music_array); i++)
    {
        struct json_object* name = json_object_array_get_idx(music_array, i);
        InsertMusic(json_object_get_string(name));
    }
    return 0;
}