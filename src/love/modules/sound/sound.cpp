#include "sound.hpp"
#include <cstdio>
#include <cstdlib>

love_decoder_t    dc_newDecoder(FILE* file,enum audio_file_type type , size_t buffer, size_t rate)
{
  return (love_decoder_t){
    .file = file,
    .kind = type,
    .chunck_size = buffer,
    .sample_rate = rate
  };
}

love_decoder_t    dc_clone(love_decoder_t* src)
{
  return *src;
}

love_sounddata_t* sd_newSoundData(size_t size)
{
  love_sounddata_t* t = (love_sounddata_t*)malloc(sizeof(love_sounddata_t) + size - 1);
  t->size = size;
  return t;
}

love_sounddata_t* dc_decode(love_decoder_t* src)
{
  love_sounddata_t* sd = sd_newSoundData(src->chunck_size);
  sd->sample_rate = src->sample_rate;
  sd->channels = src->channels;
  sd->bit_depth = src->bit_depth;

  switch(src->kind)
  {
    case RAW:
      fread(sd->data, 1, sd->size, src->file);
      break;
    default:
      printf("[LOVE SOUND C++] Currently unsupported file format\n");
      break;
  }
  return sd; 
}

size_t            dc_getBitDepth(love_decoder_t* src)
{
  return src->bit_depth;
}

size_t            dc_getChannelCount(love_decoder_t* src)
{
  return src->channels;
}

double            dc_getDuration(love_decoder_t* src)
{
  long current_pos = ftell(src->file);

  fseek(src->file, 0, SEEK_SET);
  fseek(src->file, 0, SEEK_END); 
  long bit_len = ftell(src->file);

  long samples = bit_len / (src->bit_depth >> 3);
  double duration = (double)samples  / (double)(src->sample_rate);

  fseek(src->file, current_pos, SEEK_SET);
  return duration;
}

size_t            dc_getSampleRate(love_decoder_t* src)
{
  return src->sample_rate;
}

void              dc_seek(love_decoder_t* src, double pos)
{
  long sample = pos * src->sample_rate;
  long fpos = sample * (src->bit_depth >> 3);

  fseek(src->file, fpos, SEEK_SET);
}

size_t            sd_getBitDepth(love_sounddata_t* src)
{
  return src->bit_depth;
}
size_t            sd_getChannelCount(love_sounddata_t* src)
{
  return src->channels;
}

double            sd_getDuration(love_sounddata_t* src)
{
  return src->size * (src->bit_depth >> 3) / src->sample_rate;
}

double            sd_getSample(love_sounddata_t* src, size_t idx, size_t channel)
{
  double sample = 0;

  char* sidx = src.

  return sample;
}

size_t            sd_getSampleCount(love_sounddata_t* src);
size_t            sd_getSampleRate(love_sounddata_t* src);
bool              sd_setSample(love_sounddata_t* src, size_t idx, size_t channel ,double v);
