#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
    char subchunk1_id[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t channel_count;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t frame_size; // block_align;
    uint16_t bits_per_sample;
    char subchunk2_id[4];
    uint32_t data_size;
    uint8_t data[];
} PcmWav;

bool PcmWav_is_valid(const PcmWav *data);
void PcmWav_log(const PcmWav *wav, const char *header);
void PcmWav_play_once(const PcmWav *wav);
