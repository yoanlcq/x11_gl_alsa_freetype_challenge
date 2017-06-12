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


PcmDuration PcmDuration_from(uint64_t frame_count, uint32_t sample_rate) {
    PcmDuration d = {0};
    float seconds_total = frame_count / (float)sample_rate;
    d.hours = seconds_total / 3600;
    d.minutes = seconds_total / 60 - d.hours*60;
    d.seconds = seconds_total - d.minutes*60 - d.hours*3600;
    float milliseconds = 1000.f*(seconds_total - d.seconds*60 - d.minutes*3600 - d.hours*3600*60);
    if(milliseconds >= 0)
        d.milliseconds = milliseconds;
    return d;
}

void PcmWav_log(const PcmWav *wav, const char *header) {
    uint32_t frame_count = wav->data_size / wav->frame_size;
    PcmDuration d = PcmDuration_from(frame_count, wav->sample_rate);
    logi("%s %"PRIu32"-bit %"PRIu32" channels @%"PRIu32"Hz, %"PRIu32" frames (%u:%u:%u:%u)\n", 
        header, 
        wav->bits_per_sample, 
        wav->channel_count, 
        wav->sample_rate, 
        frame_count,
        d.hours, d.minutes, d.seconds, d.milliseconds
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

#include <math.h>

static void* threadproc(void *arg) {
    const PcmWav *wav = arg;
    int err;
    snd_pcm_t *handle;
    snd_pcm_sframes_t frames = 0, wav_frame_count;
    if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0/*SND_PCM_NONBLOCK*/)) < 0) {
        loge("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_set_params(handle,
            SND_PCM_FORMAT_S16_LE, // NOTE: be careful to mention _LE
            SND_PCM_ACCESS_RW_INTERLEAVED,
            wav->channel_count,
            wav->sample_rate,
            1,
            500000)) < 0) 
    {   /* 0.5sec */
        loge("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }

    wav_frame_count = wav->data_size / wav->frame_size;
    const snd_pcm_sframes_t framechunk = /*4096<<2*/ wav_frame_count;
    int16_t *cur = wav->data;
    float factor = 1;
    for(;;) {
        if(cur + framechunk > wav->data + wav->data_size)
            break;
        for(int i=0 ; i<framechunk ; ++i) {
            cur[i*2+0] *= factor;
            cur[i*2+1] *= factor;
        }
        factor = fmaxf(0, factor-0.005f);
        frames = snd_pcm_writei(handle, cur, framechunk);
        PcmDuration d = PcmDuration_from(frames, wav->sample_rate);
        logi("Wrote %u:%u:%u:%u worth of audio data\n",
             d.hours, d.minutes, d.seconds, d.milliseconds
        );
        if(frames > 0)
            cur += frames;
        if (frames < 0)
            frames = snd_pcm_recover(handle, frames, 0);
        if (frames < 0) {
            switch(frames) {
            case -EBUSY:
            case -EAGAIN:
                break;
            default:
                loge("snd_pcm_writei failed: %s\n", snd_strerror(frames));
                return NULL;
            }
        }
        if (frames > 0 && frames < framechunk) {
            PcmDuration e, w;
            e = PcmDuration_from(wav_frame_count, wav->sample_rate);
            w = PcmDuration_from(frames, wav->sample_rate);
            logw("Short write (expected %li frames (%u:%u:%u:%u), wrote %li frames (%u:%u:%u:%u))\n", 
                framechunk, e.hours, e.minutes, e.seconds, e.milliseconds,
                frames, w.hours, w.minutes, w.seconds, w.milliseconds
            );
        } else if(!frames) {
            logw("No frames were written\n");
        }
    }
    logi("Done playing! Thread exited.\n");
    return NULL;
}
// NOTE: This creates a thread each time
// it's a quick ugly dirty hack, don't ship this
void PcmWav_play_once(const PcmWav *wav) {
    pthread_t thread;
    pthread_create(&thread, NULL, threadproc, (void*)wav);
}
