#include "audio.hpp"
#include <ogc/mutex.h>
#include <string>

#include "classes/Source.hpp"
#include <aesndlib.h>
#include <audiogc/audiogc.hpp>
#include <fat.h>
#include <sol/sol.hpp>
extern "C" {
#include <lua.h>
}

namespace {
mutex_t audio_mutex = LWP_MUTEX_NULL;
std::vector<love::audio::Source *> sources;
} // namespace

namespace love {
namespace audio {
void __init(sol::state &luastate) {
  LWP_MutexInit(&audio_mutex, false);
  AESND_Init();
  fatInitDefault();

  __registerTypes(luastate);
}

void __registerTypes(sol::state &luastate) {
  luastate.new_usertype<love::audio::Source>(
      "Source", sol::no_constructor, "play", &love::audio::Source::play, "stop",
      &love::audio::Source::stop, "pause", &love::audio::Source::pause,

      "isPlaying", &love::audio::Source::isPlaying, "seek",
      sol::overload(
          static_cast<void (love::audio::Source::*)(double)>(
              &love::audio::Source::seek_time),
          static_cast<void (love::audio::Source::*)(double, std::string)>(
              &love::audio::Source::seek_time_unit)),
      "tell", &love::audio::Source::tell,

      "setLooping", &love::audio::Source::setLooping, "isLooping",
      &love::audio::Source::isLooping,

      "setPitch", &love::audio::Source::setPitch, "getPitch",
      &love::audio::Source::getPitch,

      "setVolume", &love::audio::Source::setVolume, "getVolume",
      &love::audio::Source::getVolume);
}

love::audio::Source* newSource_file_type(std::string file, std::string type) {
  return new love::audio::Source(file, type);
}

void registerSource(love::audio::Source *source) {
  LWP_MutexLock(audio_mutex);
  sources.push_back(source);
  LWP_MutexUnlock(audio_mutex);
}

void unregisterSource(love::audio::Source *source) {
  LWP_MutexLock(audio_mutex);
  // Safely remove the source from the vector
  sources.erase(std::remove(sources.begin(), sources.end(), source),
                sources.end());
  LWP_MutexUnlock(audio_mutex);
}

double getVolume(love::audio::Source *source) { return source->getVolume(); }
void setVolume(love::audio::Source *source, double volume) {
  source->setVolume(volume);
}
void play(love::audio::Source *source) { source->play(); }
void stop_source(love::audio::Source *source) { source->stop(); }
void stop() {
  LWP_MutexLock(audio_mutex);
  for (size_t i = 0; i < sources.size(); ++i) {
    sources[i]->stop();
  }
  LWP_MutexUnlock(audio_mutex);
}
void pause(love::audio::Source *source) { source->pause(); }
} // namespace audio
} // namespace love

int luaopen_love_audio(lua_State *L) {
  sol::state_view luastate(L);

  luastate["love"]["audio"] = luastate.create_table_with(
      "newSource", love::audio::newSource_file_type, "getVolume",
      love::audio::getVolume, "setVolume", love::audio::setVolume, "play",
      love::audio::play, "stop",
      sol::overload(static_cast<void (*)(love::audio::Source *)>(
                        &love::audio::stop_source),
                    love::audio::stop),
      "pause", love::audio::pause);

  return 1;
}
