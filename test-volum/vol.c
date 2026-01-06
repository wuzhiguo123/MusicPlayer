#include <stdio.h>
#include <asoundlib.h>

int main()
{
    int err;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "hw:AudioPCI";
    const char *selem_name = "Master";

    // 打开混音器
    if ((err = snd_mixer_open(&handle, 0)) < 0) {
        fprintf(stderr, "Mixer %s open error: %s\n", card, snd_strerror(err));
        return err;
    }

    // 连接到声卡
    if ((err = snd_mixer_attach(handle, card)) < 0) {
        fprintf(stderr, "Mixer attach %s error: %s\n", card, snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }

    // 注册混音器
    if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
        fprintf(stderr, "Mixer register error: %s\n", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }

    // 加载混音器元素
    if ((err = snd_mixer_load(handle)) < 0) {
        fprintf(stderr, "Mixer load error: %s\n", snd_strerror(err));
        snd_mixer_close(handle);
        return err;
    }

    // 创建混音器元素标识符
    snd_mixer_selem_id_malloc(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);

    // 获取混音器元素
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
    if (!elem) {
        fprintf(stderr, "Unable to find simple control '%s',%i\n", selem_name, 0);
        snd_mixer_selem_id_free(sid);
        snd_mixer_close(handle);
        return -1;
    }

    // 设置音量为50%
    long minv, maxv;
    snd_mixer_selem_get_playback_volume_range(elem, &minv, &maxv);
    long volume = (maxv - minv) / 2 + minv;
    if ((err = snd_mixer_selem_set_playback_volume_all(elem, volume)) < 0) {
        fprintf(stderr, "Error setting volume: %s\n", snd_strerror(err));
    } else {
        printf("Volume set to 50%%\n");
    }  
    // 清理
    snd_mixer_selem_id_free(sid);
    snd_mixer_close(handle);

    return 0;
}