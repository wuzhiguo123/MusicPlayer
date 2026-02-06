#include "link.h"
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "main.h"

MusicNode* music_head = NULL;

int InitMusicLink()
{
    music_head = (MusicNode*)malloc(sizeof(MusicNode));
    if(music_head == NULL)
    {
        perror("malloc() error");
        exit(-1);
    }
    music_head->music_name[0] = 0;
    music_head->next = NULL;
    music_head->prev = NULL;

    return 0;

}
void InsertMusic(const char* name)
{
    // printf("++++++++++++++++name:%s\n",name);
    // printf("MUSIC_HEAD_P:%p\n",music_head);
    if(music_head == NULL)
    {
        InitMusicLink();
    }
    if(music_head->music_name[0] == 0)
    {
        strcpy(music_head->music_name,name);
        // printf("MUSIC_HEAD->NAME:%s  NAME:%s\n",music_head->music_name,name);
        return;
    }
    MusicNode* new = malloc(sizeof(MusicNode));
    if(new == NULL)
    {
        perror("malloc() error");
    }
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
    CheckMusicList();
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
        // printf("#####################%s\n",json_object_get_string(name));
        InsertMusic(json_object_get_string(name));
    }
    CheckMusicList();
    return 0;
}

void ClearMusicList()
{
    MusicNode* p = music_head;
    while(p)
    {
        MusicNode* q = p->next;
        printf("清除：%s\n",p->music_name);
        free(p);
        p = q;
    }
    music_head = NULL;
    printf("链表清空\n");
}
