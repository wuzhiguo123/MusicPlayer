#include <fcntl.h>
#include <json-c/json.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void ParseJson(char* buffer,char* content)
{
    struct json_object* obj = json_tokener_parse(buffer);
    struct json_object* arr = json_object_object_get(obj, "choices");
    struct json_object* first_choice = json_object_array_get_idx(arr, 0);
    struct json_object* message = json_object_object_get(first_choice,"message");
    struct json_object* cont = json_object_object_get(message,"content");

    strcpy(content, json_object_get_string(cont));

    json_object_put(obj);
}

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        return -1;
    }

    char commond[1024] = {0};
    sprintf(commond, "/root/MusicPlayer/1.hardware/Chatmodel/qianwen.sh %s 2>/dev/null", argv[1]);

    FILE* fp = popen(commond, "r");
    if(fp == NULL)
    {
        fprintf(stderr,"popen error\n");
        return -1;
    }

    char buffer[2048] = {0};
    fgets(buffer, sizeof(buffer), fp);
    fclose(fp);
    char content[1024] = {0};
    ParseJson(buffer,content);

    if(strlen(content) > 0)
    {
        int tts_fd = open("/root/fifo/tts_fifo", O_WRONLY);
        write(tts_fd, content, strlen(content));
        close(tts_fd);
    }
    return 0;
}