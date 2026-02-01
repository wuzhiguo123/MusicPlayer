#include "net.h"
#include "device.h"
// #include <json-c/json_object_iterator.h>
#include "link.h"
#include "main.h"
#include "player.h"
#include "select.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <json-c/json.h>
// #include <json-c/json_object.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
// #include "../select/select.h"
extern fd_set READSET;
int g_sockfd = 0;
int g_maxfd = 0;
pthread_t tid;
extern int g_start_flag;
extern int g_suspend_flag;
extern int g_device_mode;
extern MusicNode *music_head;

void SendMusicInfo(struct json_object *obj) {
  if (g_device_mode == ONLINE_MODE) {
    char buffer[1024] = {0};
    int len = 0;
    // 序列化
    const char *info = json_object_to_json_string(obj);
    if (info == NULL) {
      perror("jason_object_to_json_string() error");
    }
    // 简单的头部长度校验协议，前四个字节放入消息的字节长度
    len = strlen(info);
    memcpy(buffer, &len, sizeof(int));
    memcpy(buffer + sizeof(int), info, len);
    printf("[CLIENT SEND]%s\n", info);
    if (send(g_sockfd, buffer, len + 4, 0) < 0) {
      perror("send() error");
      return;
    }
  }
}

void *SendServer(void *arg) {
  (void)arg;
  while (1) {
    // 创建json对象
    struct json_object *obj = json_object_new_object();

    // 往json中添加键对值
    json_object_object_add(obj, "cmd", json_object_new_string("info"));

    Shm music_info;
    GetShm(&music_info);

    json_object_object_add(obj, "cur_music",
                           json_object_new_string(music_info.cur_music));
    json_object_object_add(obj, "mode",
                           json_object_new_int(music_info.cur_mode));

    char status[8] = {0};

    // 当前状态
    if (g_start_flag == 0)
      strcpy(status, "stop");
    else if (g_start_flag == 1 && g_suspend_flag == 0)
      strcpy(status, "start");
    else if (g_start_flag == 1 && g_suspend_flag == 1)
      strcpy(status, "suspend");

    json_object_object_add(obj, "status", json_object_new_string(status));
    json_object_object_add(obj, "deviceid", json_object_new_string(DEVICEID));
    // 获取当前音量
    int value;
    GetVolume(&value);
    json_object_object_add(obj, "volume", json_object_new_int(value));

    // 发送给服务器
    SendMusicInfo(obj);
    json_object_put(obj);
    sleep(2);
  }
}

int InitSocket() {
  g_sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (g_sockfd < 0) {
    perror("client socket() error");
  }

  int connect_cnt = 10;
  int connect_ret = 0;

  struct sockaddr_in server_info;
  memset(&server_info, 0, sizeof(server_info));
  server_info.sin_addr.s_addr = inet_addr(IP);
  server_info.sin_family = AF_INET;
  server_info.sin_port = htons(PORT);
  while (connect_cnt--) {
    connect_ret =
        connect(g_sockfd, (struct sockaddr *)&server_info, sizeof(server_info));
    if (connect_ret < 0) {
      perror("connect() error");
      sleep(1);
      continue;
    }

    FD_SET(g_sockfd, &READSET);
    g_maxfd = (g_maxfd < g_sockfd) ? g_sockfd : g_maxfd;
    g_device_mode = ONLINE_MODE;
    printf("DEVICE MODE:ONLINE MODE!");

    if (pthread_create(&tid, NULL, SendServer, 0) != 0) {
      perror("pthread_create() error");
      break;
    }
    pthread_detach(tid);
    break;
  }
  return connect_ret;
}

int RecSocketData(char *data) {
  size_t len = 0;
  size_t size = 0; // 一定要初始化
  while (1) {
    size += recv(g_sockfd, data + size, sizeof(int) - size, 0);
    if (size == sizeof(int))
      break;
    else if (size == 0) {
      FD_CLR(g_sockfd, &READSET);
      g_maxfd = (g_maxfd == g_sockfd) ? g_maxfd - 1 : g_maxfd;
      close(g_sockfd);
      pthread_cancel(tid);
    }
  }

  size = 0;
  len = *(int *)data;
  memset(data, 0, sizeof(int));

  while (1) {
    size += recv(g_sockfd, data + size, len - size, 0);
    if (size == len)
      break;
  }
  printf("[CLIENT REC]%s\n", data);
  return 0;
}

// 向服务器根据歌手名请求音乐数据

int GetMusicName(const char *singer) {
    ClearMusicList();
  //   printf("GETMUSCINAME\nSINGER:%s\n", singer);
  if (g_device_mode == ONLINE_MODE) {
    struct json_object *obj = json_object_new_object();
    json_object_object_add(obj, "cmd",
                           json_object_new_string("get_music_list"));
    json_object_object_add(obj, "singer", json_object_new_string(singer));
    SendMusicInfo(obj);
    char music_name[1024] = {0};

    RecSocketData(music_name);

    LinkMusicList(music_name);
    json_object_put(obj);
    UploadMusic();
  }

  else if (g_device_mode == OFFLINE_MODE) {
    printf("离线模式获取音乐中.....\n");
    DIR *singer_dir = opendir(OFFLINE_URL);
    struct dirent *dir_file;
    while ((dir_file = readdir(singer_dir)) != NULL) {
      if (!strcmp(dir_file->d_name, ".") || !strcmp(dir_file->d_name, ".."))
        continue;
      if (!strcmp(dir_file->d_name, singer))
        break;
    }
    char music_path[256] = {0};
    char off_url[128] = "/mnt/usb/music/";
    strcpy(music_path, off_url);
    // printf("musci_path:%s\n",music_path);

    strcat(music_path, dir_file->d_name);

    // printf("musci_path:%s\n",music_path);

    DIR *music_dir = opendir(music_path);
    struct dirent *music_file;

    while ((music_file = readdir(music_dir)) != NULL) {
      char name[128] = {0};
      strcpy(name, dir_file->d_name);
      strcat(name, "/");
      if (music_file->d_type != DT_REG) {
        continue;
      }
      if (strstr(music_file->d_name, ".mp3")) {

        strcat(name + strlen(name), music_file->d_name);
        InsertMusic(name);
        printf("插入的歌曲名：%s\n",name);
        memset(name, 0, sizeof(name));
      }
    
    }
  }

  CheckMusicList();

  return 0;
}

void ParseMessage(char *buffer, char *cmd) {
  struct json_object *obj = json_tokener_parse(buffer);
  if (obj == NULL) {
    fprintf(stderr, "不是一个json对象\n");
    return;
  }
  struct json_object *value;
  value = json_object_object_get(obj, "cmd");

  if (value == NULL) {
    fprintf(stderr, "不包含cmd命令");
    return;
  }
  strcpy(cmd, json_object_get_string(value));
  json_object_put(obj);
}

void SocketStartPlay() {
  // 启动播放
  StartPlay();
  // 查询启动状态
  char result[128] = {0};
  FILE *fp = popen("pgrep mplayer", "r");
  printf("%p\n", fp);
  fgets(result, 128, fp);
  pclose(fp);

  struct json_object *obj = json_object_new_object();
  json_object_object_add(obj, "cmd", json_object_new_string("app_start_reply"));
  if (strlen(result) == 0) {
    json_object_object_add(obj, "result", json_object_new_string("failure"));
  } else {
    json_object_object_add(obj, "result", json_object_new_string("success"));
  }
  SendMusicInfo(obj);
  json_object_put(obj);
}

void SocketStopPlay() {
  StopPlay();
  // 查询关闭状态
  printf("停止播放\n");

  char result[128] = {0};
  FILE *fp = popen("pgrep mplayer", "r");

  printf("%p\n", fp);
  fgets(result, 128, fp);
  pclose(fp);

  struct json_object *obj = json_object_new_object();
  json_object_object_add(obj, "cmd", json_object_new_string("app_stop_reply"));

  if (strlen(result) == 0) {
    json_object_object_add(obj, "result", json_object_new_string("success"));
  } else {
    json_object_object_add(obj, "result", json_object_new_string("failure"));
  }
  SendMusicInfo(obj);
  json_object_put(obj);
  printf("停止播放\n");
}

void SocketSuspendPlay() {
  SuspendPlay();
  struct json_object *obj = json_object_new_object();
  json_object_object_add(obj, "cmd",
                         json_object_new_string("app_suspend_reply"));
  json_object_object_add(obj, "result", json_object_new_string("success"));
  SendMusicInfo(obj);
  json_object_put(obj);
}

void SocketContinuePlay() {
  ContinuePlay();
  struct json_object *obj = json_object_new_object();
  json_object_object_add(obj, "cmd",
                         json_object_new_string("app_suspend_reply"));
  json_object_object_add(obj, "result", json_object_new_string("success"));
  SendMusicInfo(obj);
  json_object_put(obj);
}

void SocketNextPlay() {
  NextPlay();
  struct json_object *obj = json_object_new_object();
  json_object_object_add(obj, "cmd", json_object_new_string("app_next_reply"));
  json_object_object_add(obj, "result", json_object_new_string("success"));
  SendMusicInfo(obj);
  json_object_put(obj);
}

void SocketPrevPlay() {
  PrevPlay();
  struct json_object *obj = json_object_new_object();
  json_object_object_add(obj, "cmd", json_object_new_string("app_prev_reply"));
  json_object_object_add(obj, "result", json_object_new_string("success"));
  SendMusicInfo(obj);
  json_object_put(obj);
}

void SocketDownVolume() {
  DownVolume();
  struct json_object *obj = json_object_new_object();
  json_object_object_add(obj, "cmd",
                         json_object_new_string("app_downvolume_reply"));
  json_object_object_add(obj, "result", json_object_new_string("success"));
  SendMusicInfo(obj);
  json_object_put(obj);
}

void SocketUpVolume() {
  UpVolume();
  struct json_object *obj = json_object_new_object();
  json_object_object_add(obj, "cmd",
                         json_object_new_string("app_upvolume_reply"));
  json_object_object_add(obj, "result", json_object_new_string("success"));
  SendMusicInfo(obj);
  json_object_put(obj);
}

void SocketCirPlay() {
  CirclePlay();
  struct json_object *obj = json_object_new_object();
  json_object_object_add(obj, "cmd",
                         json_object_new_string("app_circle_reply"));
  json_object_object_add(obj, "result", json_object_new_string("success"));
  SendMusicInfo(obj);
  json_object_put(obj);
}

void SocketSeqPlay() {
  SequencePlay();
  struct json_object *obj = json_object_new_object();
  json_object_object_add(obj, "cmd",
                         json_object_new_string("app_sequence_reply"));
  json_object_object_add(obj, "result", json_object_new_string("success"));
  SendMusicInfo(obj);
  json_object_put(obj);
}

void UploadMusic() {
  struct json_object *obj = json_object_new_object();
  json_object_object_add(obj, "cmd", json_object_new_string("upload_music"));

  struct json_object *array = json_object_new_array();
  MusicNode *p = music_head;
  printf("UploadMusic():music_head:%p\n", music_head);
  while (p) {
    json_object_array_add(array, json_object_new_string(p->music_name));
    printf("UPLOAD:musicname:%s\n", p->music_name);
    p = p->next;
  }
  json_object_object_add(obj, "music", array);
  SendMusicInfo(obj);
  json_object_put(obj);
  // json_object_put(array);//已经把array交给obj了，就只用释放obj
}

void ReadSocket() {
  char buffer[1024] = {0};
  char cmd[32] = {0};
  RecSocketData(buffer);
  ParseMessage(buffer, cmd);
  if (!strcmp(cmd, "app_start")) {
    SocketStartPlay();
  } else if (!strcmp(cmd, "app_stop")) {
    SocketStopPlay();
  } else if (!strcmp(cmd, "app_suspend")) {
    SocketSuspendPlay();
  } else if (!strcmp(cmd, "app_continue")) {
    SocketContinuePlay();
  } else if (!(strcmp(cmd, "app_next"))) {
    SocketNextPlay();
  } else if (!(strcmp(cmd, "app_prev"))) {
    SocketPrevPlay();
  } else if (!(strcmp(cmd, "app_downvolume"))) {
    SocketDownVolume();
  } else if (!(strcmp(cmd, "app_upvolume"))) {
    SocketUpVolume();
  } else if (!(strcmp(cmd, "app_circle"))) {
    SocketCirPlay();
  } else if (!(strcmp(cmd, "app_sequence"))) {
    SocketSeqPlay();
  } else if (!strcmp(cmd, "app_get_music")) {
    UploadMusic();
  }
}
