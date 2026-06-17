#ifndef LOVE_SOUND_HPP
#define LOVE_SOUND_HPP

#include "../lib/audiogc/dr_flac.h"
#include "../lib/audiogc/dr_wav.h"
#include "../lib/audiogc/dr_mp3.h"
#include <cstdio>

typedef struct
{
  size_t sample_rate;
  size_t bit_depth;
  size_t channels;
  size_t size;
  char data[1];
} love_sounddata_t;

enum audio_file_type {
  RAW,
  OGG,
  MP3,
  WAV,
  FLAC
};

typedef struct
{
  FILE* file;
  enum audio_file_type kind;
  size_t chunck_size;
  size_t sample_rate;
  size_t duration;
  size_t bit_depth;
  size_t channels;
} love_decoder_t;

//Constructors
love_decoder_t    dc_newDecoder(FILE* file,enum audio_file_type type ,size_t buffer, size_t rate);
love_decoder_t    dc_clone(love_decoder_t* src);

love_sounddata_t* sd_newSoundData(size_t size);

//Methods
love_sounddata_t* dc_decode(love_decoder_t* src);
size_t            dc_getBitDepth(love_decoder_t* src);
size_t            dc_getChannelCount(love_decoder_t* src);
double            dc_getDuration(love_decoder_t* src);
size_t            dc_getSampleRate(love_decoder_t* src);
void              dc_seek(love_decoder_t* src, double pos);

size_t            sd_getBitDepth(love_sounddata_t* src);
size_t            sd_getChannelCount(love_sounddata_t* src);
double            sd_getDuration(love_sounddata_t* src);
double            sd_getSample(love_sounddata_t* src, size_t idx, size_t channel);
size_t            sd_getSampleCount(love_sounddata_t* src);
size_t            sd_getSampleRate(love_sounddata_t* src);
bool              sd_setSample(love_sounddata_t* src, size_t idx, size_t channel ,double v);

#endif
