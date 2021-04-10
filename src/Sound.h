#ifndef SOUND_H
#define SOUND_H

#include <fstream>
#include <memory>
#include "al.h"
//#include "AudioPlayer.h"

class AudioPlayer;

class Sound
{
public:

    virtual ~Sound();

    virtual void setVolume(float value) const;

    virtual void setPitch(float value) const;

    virtual void setLoop(bool value) const;

    virtual float getVolume() const;

    virtual float getPitch() const;

    virtual bool isPlaying() const;

    virtual bool isPause() const;

    virtual bool isStopped() const;

    unsigned int getSourceId() const;

private:
    Sound();
    // Supported sound file format: .wav, .ogg
    Sound(const char * filepath);
    Sound(const char ** filepathes, unsigned int count);

    friend class AudioPlayer;

    int size;
    int frequency;
    int channel;
    int bitsPerSample;
    int format;
    // char* data;
    void* data;

    unsigned int alsrc;
    unsigned int* buffers;
    unsigned int buf_number;

    void loadWAV(const char * path);
    void loadOGG(const char * path);

};

#endif
