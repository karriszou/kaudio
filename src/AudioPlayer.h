#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <array>
#include <tuple>
#include <vector>

#include "al.h"
#include "alc.h"

#include "Sound.h"


/*
 * Supported format:
 *	.wav .ogg
 *
 * Usage:
 *	AudioPlayer ap;
 *	Sound& bg = ap.addSound(oggFilePath, "bg");
 *	ap.addSound(wavFilePath, "du");
 *	ap.play("du");
 *	ap.play(bg, false, 0.6f);
 *
 */

class AudioPlayer
{
public:
    static ALCdevice*  aldev;
    static ALCdevice*  micdev;
    static ALCcontext* alctx;

    AudioPlayer();
    ~AudioPlayer();

    Sound& addSound(const char * soundPath, const char * alias = NULL);

    Sound& addQueueSound(const char ** soundPath, unsigned int count, const char * alias = NULL);

    // Sound& addSound(const char * alias, Sound& sound);

    void removeSound(const char * alias);

    void removeSound(const Sound& sound);
	
    Sound* getSound(const char * alias);

    Sound* getSound(unsigned int soundID);

    bool contain(const char * alias);

    bool contain(const Sound& sound);

    void play(const char * alias, bool loop = false, float volume = 1.0f);

    void play(const Sound& sound, bool loop = false, float volume = 1.0f);

    // void play(unsigned int src);

    void pause(const char * alias);

    void pause(const Sound& sound);

    void resume(const char * alias);

    void resume(const Sound& sound);

    void stop(const char * alias);

    void stop(const Sound& sound);

    void clear();

    void setListenerVolume(float volume);

    float getListenerVolume();

    float getPlayTimes(const char * alias);

    float getPlayTimes(const Sound& sound);

    void startCapture();

    unsigned int getCaptureSamples(char** data);

    void stopCapture();

    void setListenerPos(float x, float y, float z);

    void setListenerVel(float x, float y, float z);

    void setListenerOri(float fx, float fy, float fz, float upx, float upy, float upz);

    const std::array<float, 3> getListenerPos();

    const std::array<float, 3> getListenerVel();

    const std::array<float, 6> getListenerOri();

private:
    const ALchar* devname;
    std::array<float, 3> listenerPos;
    std::array<float, 3> listenerVel;
    std::array<float, 6> listenerOri;

    static const int maxSound		= 256;
    static const int channel_capture	= 1;
    static const int frequecy_capture	= 33075;
    static const ALenum format_capture	= AL_FORMAT_MONO16;

    // 0: alias  1: source id  2: file path  3: sound object
    using AudioItem = std::tuple<std::string, unsigned int, const char *, Sound*>;
    std::vector<AudioItem> maps;

    void alCheck();

};

#endif
