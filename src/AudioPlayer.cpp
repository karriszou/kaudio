#include <iostream>
#include <cstring>
#include <algorithm>
#include "AudioPlayer.h"


ALCdevice*  AudioPlayer::aldev	= NULL;
ALCdevice*  AudioPlayer::micdev	= NULL;
ALCcontext* AudioPlayer::alctx	= NULL;


AudioPlayer::AudioPlayer()
    :devname(NULL),
     listenerPos(),
     listenerVel(),
     listenerOri(),
     maps()
{
    if(alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") == ALC_TRUE)
    	this->devname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    AudioPlayer::aldev = alcOpenDevice(this->devname);
    if(AudioPlayer::aldev)
    {
	AudioPlayer::alctx = alcCreateContext(AudioPlayer::aldev, NULL);
	alcMakeContextCurrent(AudioPlayer::alctx);
    }
    else
    {
	std::cout << "ERROR::AL::OPEN_DEVICE_FAILED!" << std::endl;
	return;
    }
    this->alCheck();

    this->listenerPos = { 0.0f, 0.0f, 0.0f };
    this->listenerVel = { 0.0f, 0.0f, 0.0f };
    this->listenerOri = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
    alListenerfv(AL_POSITION, listenerPos.data());
    alListenerfv(AL_VELOCITY, listenerVel.data());
    alListenerfv(AL_ORIENTATION, listenerOri.data());
    this->alCheck();
}


Sound& AudioPlayer::addSound(const char * soundPath, const char * alias)
{
    if(this->maps.size() > AudioPlayer::maxSound)
    {
	std::cout << "ERROR::AUDIOPLAYER::ADD_SOUND_EXCEED_MAX_SOUND_NUMBER!" << std::endl;	
	return *(new Sound());
    }
    for(AudioItem& ai : maps)
    {
    	if(alias && std::strcmp(std::get<0>(ai).c_str(), alias)  == 0)
    	{
    	    std::cout << "WARNNING::AUDIOPLAYER::ADD_SOUND_ALIAS_HAVEN_EXIST!" << std::endl;
	    return *std::get<3>(ai);
    	}
    	if(std::strcmp(std::get<2>(ai), soundPath)  == 0)
    	{
    	    std::cout << "WARNNING::AUDIOPLAYER::ADD_SOUND_FILE_HAVEN_EXIST!" << std::endl;
	    return *std::get<3>(ai);
    	}
    }

    Sound* sound = new Sound(soundPath);
    if(sound)
    {
    	AudioItem ai = std::make_tuple(alias ? alias : soundPath, sound->alsrc, soundPath, sound);
    	this->maps.push_back(ai);
    }
    else
    {
    	std::cout << "ERROR::AUDIOPLAYER::ADD_SOUND_FIALED!" << std::endl;
    	return *(new Sound());	
    }
    return *sound;
}


Sound& AudioPlayer::addQueueSound(const char ** soundPathes, unsigned int count, const char * alias)
{
    if(this->maps.size() > AudioPlayer::maxSound)
    {
	std::cout << "ERROR::AUDIOPLAYER::ADD_SOUND_EXCEED_MAX_SOUND_NUMBER!" << std::endl;	
	return *(new Sound());
    }
    for(AudioItem& ai : maps)
    {
    	if(alias && std::strcmp(std::get<0>(ai).c_str(), alias)  == 0)
    	{
    	    std::cout << "WARNNING::AUDIOPLAYER::ADD_SOUND_ALIAS_HAVEN_EXIST!" << std::endl;
	    return *std::get<3>(ai);
    	}
    }
    Sound* sound = new Sound(soundPathes, count);
    if(sound)
    {
	std::string pathes;
	for(unsigned int i = 0; i < count; i++)
	    pathes.append(soundPathes[i]).append(";");
    	AudioItem ai = std::make_tuple(alias ? alias : pathes, sound->alsrc, pathes.c_str(), sound);
    	this->maps.push_back(ai);
    }
    else
    {
    	std::cout << "ERROR::AUDIOPLAYER::ADD_SOUND_FIALED!" << std::endl;
    	return *(new Sound());	
    }
    return *sound;
    
}


void AudioPlayer::removeSound(const char * alias)
{
    if(!this->contain(alias))
    {
	std::cout << "WARNNING::AUDIO_PALYER::REMOVE_NOT_FOUND!" << std::endl;
	return;
    }
    std::vector<AudioItem>::iterator it;
    for(it = this->maps.begin(); it != this->maps.end(); it++)
	    	if(std::strcmp(std::get<0>(*it).c_str(), alias)  == 0)
		    break;
    if(it != this->maps.end())
    {
	Sound* snd = std::get<3>(*it);
	if(snd) this->stop(*snd);
    	this->maps.erase(it);
    }
    else
	std::cout << "WARNNING::AUDIO_PALYER::REMOVE_NOT_FOUND!" << std::endl;
}


void AudioPlayer::removeSound(const Sound& sound)
{
    if(!this->contain(sound))
    {
    	std::cout << "WARNNING::AUDIO_PALYER::REMOVE_NOT_FOUND!" << std::endl;
    	return;
    }
    std::vector<AudioItem>::iterator it;
    for(it = this->maps.begin(); it != this->maps.end(); it++)
    	    	if(std::get<1>(*it) ==  sound.alsrc)
    		    break;
    if(it != this->maps.end())
    {
    	Sound* snd = std::get<3>(*it);
    	if(snd) this->stop(*snd);
    	this->maps.erase(it);
    }
    else
	std::cout << "WARNNING::AUDIO_PALYER::REMOVE_NOT_FOUND!" << std::endl;
}

	
Sound* AudioPlayer::getSound(const char * alias)
{
    Sound* sptr = NULL;
    for(AudioItem& ai : this->maps)
    	if(std::strcmp(std::get<0>(ai).c_str(), alias)  == 0)
	    sptr = std::get<3>(ai);
    if(!sptr)
	std::cout << "ERROR::AUDIO_PLAYER::GET_NOT_FOUND!" << std::endl;
    return sptr;
}


Sound* AudioPlayer::getSound(unsigned int soundID)
{
    Sound* sptr = NULL;
    for(AudioItem& ai : this->maps)
    	if(std::get<1>(ai) == soundID)
	    sptr = std::get<3>(ai);
    return sptr;
}


void AudioPlayer::play(const char * alias, bool loop, float volume)
{
    Sound* sptr = this->getSound(alias);
    if(sptr)
    {
	sptr->setLoop(loop);
	sptr->setVolume(volume);
	alSourcePlay(sptr->alsrc);
    }
    else
	std::cout << "WARNNING::AUDIO_PLAYER::PLAY_NOT_FOUND!" << std::endl;
}


void AudioPlayer::play(const Sound& sound, bool loop, float volume)
{
    sound.setLoop(loop);
    sound.setVolume(volume);
    alSourcePlay(sound.alsrc);
}


void AudioPlayer::pause(const char * alias)
{
    Sound* sptr = this->getSound(alias);
    if(sptr)
	alSourcePause(sptr->alsrc);
    else
	std::cout << "WARNNING::AUDIO_PLAYER::PAUSE_NOT_FOUND!" << std::endl;
}


void AudioPlayer::pause(const Sound& sound)
{
    alSourcePause(sound.alsrc);
}


void AudioPlayer::resume(const char * alias)
{
    Sound* sptr = this->getSound(alias);
    if(sptr)
    {
	if(sptr->isPause())
	    alSourcePlay(sptr->alsrc);
    }
    else
	std::cout << "WARNNING::AUDIO_PLAYER::RESUME_NOT_FOUND!" << std::endl;
}


void AudioPlayer::resume(const Sound& sound)
{
    if(sound.isPause())
	alSourcePlay(sound.alsrc);
}


void AudioPlayer::stop(const char * alias)
{
    Sound* sptr = this->getSound(alias);
    if(sptr)
	alSourceStop(sptr->alsrc);
    else
	std::cout << "WARNNING::AUDIO_PLAYER::STOP_NOT_FOUND!" << std::endl;
}


void AudioPlayer::stop(const Sound& sound)
{
    alSourceStop(sound.alsrc);
}


bool AudioPlayer::contain(const char * alias)
{
    for(AudioItem& ai : this->maps)
    	if(std::strcmp(std::get<0>(ai).c_str(), alias)  == 0)
	    return true;
    return false;
}


bool AudioPlayer::contain(const Sound& sound)
{
    for(AudioItem& ai : this->maps)
    	if(std::get<1>(ai) == sound.alsrc)
	    return true;
    return false;
}


void AudioPlayer::setListenerVolume(float volume)
{
    volume = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);
    alListenerf(AL_GAIN, volume);
}


float AudioPlayer::getListenerVolume()
{
    float volume = 0.0f;
    alGetListenerf(AL_GAIN, &volume);
    return volume;
}


float AudioPlayer::getPlayTimes(const char * alias)
{
    float seconds = 0.0f;
    Sound* sptr = this->getSound(alias);
    if(sptr) alGetSourcef(sptr->alsrc, AL_SEC_OFFSET, &seconds);
    return seconds;
}


float AudioPlayer::getPlayTimes(const Sound& sound)
{
    float seconds = 0.0f;
    alGetSourcef(sound.alsrc, AL_SEC_OFFSET, &seconds);
    return seconds;
}


void AudioPlayer::startCapture()
{
    if(!AudioPlayer::micdev)
    {
	const ALCchar* micdevname = NULL;
	if(alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") == ALC_TRUE)
	    micdevname = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
	AudioPlayer::micdev = alcCaptureOpenDevice(micdevname,
						   AudioPlayer::frequecy_capture,
						   AudioPlayer::format_capture,
						   AudioPlayer::frequecy_capture /* * 2 */ );
    }
    this->alCheck();
    if(!AudioPlayer::micdev)
    {
	std::cout << "WARNNING::OpenAL can not find capture device\n" << std::endl;
	return;
    }
    alcCaptureStart(AudioPlayer::micdev);
}


unsigned int AudioPlayer::getCaptureSamples(char** data)
{
    if(!AudioPlayer::micdev)
    {
	std::cout << "ERROR::AUDIO_PLAYER::GET_NONE_CAPTURE_SAMPLES!\n" << std::endl;	
	return 0;
    }
    
    int sample_count = 0;
    alcGetIntegerv(AudioPlayer::micdev, ALC_CAPTURE_SAMPLES, 1, &sample_count);
    if(sample_count < 1)
	return 0;

    // Bytes here, convert samples to bytes
    // (bytes = frames * channels * bytesPerSample) Right?
    // our sample's format is MONO16, so each sample have two bytes
    // char data[sample_count * 2];
    unsigned int bytes = sample_count * 2;
    char* samples = new char[bytes];
    alcCaptureSamples(micdev, samples, sample_count);
    this->alCheck();
    *data = samples;
    return bytes;
}


void AudioPlayer::stopCapture()
{
    if(AudioPlayer::micdev)
	alcCaptureStop(AudioPlayer::micdev);    
    this->alCheck();
}


void AudioPlayer::setListenerPos(float x, float y, float z)
{
    this->listenerPos = { x, y, z };
    alListener3f(AL_POSITION, x, y, z);
}


void AudioPlayer::setListenerVel(float x, float y, float z)
{
    this->listenerVel = { x, y, z };
    alListener3f(AL_VELOCITY, x, y, z);
}
    

void AudioPlayer::setListenerOri(float fx, float fy, float fz, float upx, float upy, float upz)
{
    this->listenerOri = { fx, fy, fz, upx, upy, upz };
    alListenerfv(AL_ORIENTATION, this->listenerOri.data());
}


const std::array<float, 3> AudioPlayer::getListenerPos()
{
    return this->listenerPos;
}


const std::array<float, 3> AudioPlayer::getListenerVel()
{
    return this->listenerVel;
}


const std::array<float, 6> AudioPlayer::getListenerOri()
{
    return this->listenerOri;    
}


void AudioPlayer::clear()
{
    this->maps.clear();
}


void AudioPlayer::alCheck()
{
    ALenum err = alGetError();
    if(err != AL_NO_ERROR)
    {
	std::cout << "ERROR::OPENAL: " << err << std::endl;
	switch (err)
	{
	case AL_NO_ERROR:
	    std::cout <<  "AL_NO_ERROR"		    << std::endl;
	    break;
	case AL_INVALID_OPERATION:
	    std::cout <<  "AL_INVALID_OPERATION"    << std::endl;
	    break;
	case AL_INVALID_ENUM:
	    std::cout <<  "AL_INVALID_ENUM"	    << std::endl;
	    break;
	case AL_INVALID_VALUE:
	    std::cout <<  "AL_INVALID_VALUE"	    << std::endl;
	    break;
	case AL_OUT_OF_MEMORY:
	    std::cout <<  "AL_OUT_OF_MEMORY"	    << std::endl;
	    break;
	default:
	    std::cout <<  "Unknown error code"	    << std::endl;
	    break;
	}
    }
}


AudioPlayer::~AudioPlayer()
{
    alcMakeContextCurrent(NULL);
    alcDestroyContext(AudioPlayer::alctx);
    alcCloseDevice(AudioPlayer::aldev);
    if(AudioPlayer::micdev)
    {
	alcCloseDevice(AudioPlayer::micdev);
    }
    this->clear();
}

