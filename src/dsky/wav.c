#include <dsky/wav.h>
#include <dsky/log.h>
#include <string.h>
#include <inttypes.h>

#define TAG "PcmWav"


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
void PcmWav_convert_endianness(const PcmWav *wav) {
    (void)wav;
}
#else
#error PcmWav_convert_endianness not yet implemented for big endians!
#endif

bool PcmWav_is_valid(const PcmWav *wav) {
    if(strncmp(wav->chunk_id, "RIFF", 4))
        return false;
    if(strncmp(wav->format, "WAVE", 4))
        return false;
    if(strncmp(wav->subchunk1_id, "fmt ", 4))
        return false;
    if(wav->audio_format != 1) // PCM = 1
        return false;
    if(strncmp(wav->subchunk2_id, "data", 4))
        return false;
    return true;
}
void PcmWav_log(const PcmWav *wav, const char *header) {
    uint32_t frame_count = wav->data_size / wav->frame_size;
    unsigned seconds = frame_count / (float)wav->sample_rate;
    unsigned minutes = seconds / 60;
    logi("%s %"PRIu32"-bit %"PRIu32" channels @%"PRIu32"Hz, %"PRIu32" frames (%u:%u)\n", 
        header, 
        wav->bits_per_sample, 
        wav->channel_count, 
        wav->sample_rate, 
        frame_count,
        minutes, seconds % 60
    );
}

#include <alsa/asoundlib.h>
#include <pthread.h>

/*
#define check(expr) do { \
    if((err = (expr)) < 0) { \
        loge("%s failed: \n\t%s\n", #expr, snd_strerror(err)); \
        hope(!#expr); \
    } \
} while(0);
*/


static void* threadproc(void *arg) {
    const PcmWav *wav = arg;
    int err;
    snd_pcm_t *handle;
    snd_pcm_sframes_t frames, wav_frame_count;
    if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_set_params(handle,
            SND_PCM_FORMAT_S16,
            SND_PCM_ACCESS_RW_INTERLEAVED,
            wav->channel_count,
            wav->sample_rate,
            1,
            500000)) < 0) 
    {   /* 0.5sec */
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    wav_frame_count = wav->data_size / wav->frame_size;
    frames = snd_pcm_writei(handle, wav->data, wav_frame_count);
    if (frames < 0)
        frames = snd_pcm_recover(handle, frames, 0);
    if (frames < 0) {
        printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
        return NULL;
    }
    if (frames > 0 && frames < (long)wav_frame_count)
        printf("Short write (expected %li, wrote %li)\n", (long)wav->data_size, frames);
    return NULL;
}
// NOTE: This creates a thread each time
// it's a quick ugly dirty hack, don't ship this
void PcmWav_play_once(const PcmWav *wav) {
    pthread_t thread;
    pthread_create(&thread, NULL, threadproc, (void*)wav);
}
