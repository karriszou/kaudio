#include <iostream>
#include <string>
#include <cassert>
#include "Sound.h"

#ifndef STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_HEADER_ONLY
#endif
#include "stb_vorbis.c"


Sound::Sound()
    :size(0),
     frequency(0),
     channel(0),
     bitsPerSample(0),
     format(0),
     data(NULL),
     alsrc(-1),
     buffers(NULL),
     buf_number(0)
{
    
}


Sound::Sound(const char * filepath)
    :size(0),
     frequency(0),
     channel(0),
     bitsPerSample(0),
     format(0),
     data(NULL),
     alsrc(-1),
     buffers(NULL),
     buf_number(1)
{
    std::string path(filepath);
    size_t dot_pos = path.find_last_of('.');
    std::string ext = dot_pos == std::string::npos ? "" : path.substr(dot_pos + 1, path.length() - dot_pos - 1);
    
    auto toLower = [](std::string str) -> std::string
		   {
		       std::string res;
		       for(auto c : str)
			   res += std::tolower(c);
		       return res;
		   };

    if(toLower(ext) == "wav" || toLower(ext) == "wave")
	this->loadWAV(filepath);
    else if(toLower(ext) == "ogg")
	this->loadOGG(filepath);
    else
	std::cout << "ERROR::AL::NOT_SUPPORTED_SOUND_FILE_FORMAT(.WAV .OGG): " << filepath << std::endl;

    this->buf_number = 1;
    this->buffers = new unsigned int[1];
    if(this->data)
    {
    	alGenBuffers(this->buf_number, &this->buffers[0]);
    	alBufferData(this->buffers[0], this->format, this->data, this->size, this->frequency);
    	alGenSources(1, &this->alsrc);
    	alSourcei(this->alsrc, AL_BUFFER, this->buffers[0]);
    }
    else
    	std::cout << "ERROR::SOUND_NOT_LOAD_DATA" << std::endl;
}


Sound::Sound(const char ** filepathes, unsigned int count)
    :size(0),
     frequency(0),
     channel(0),
     bitsPerSample(0),
     format(0),
     data(NULL),
     alsrc(-1),
     buffers(NULL),
     buf_number(1)
{
    assert(count > 1 && count < 64);
    this->buf_number = count;
    this->buffers = new unsigned int[count];
    alGenBuffers(this->buf_number, this->buffers);
    for(unsigned int i = 0; i < count; i++)
    {
	std::string path(filepathes[i]);
	size_t dot_pos = path.find_last_of('.');
	std::string ext = dot_pos == std::string::npos ? "" : path.substr(dot_pos + 1, path.length() - dot_pos - 1);
    
	auto toLower = [](std::string str) -> std::string
		       {
			   std::string res;
			   for(auto c : str)
			       res += std::tolower(c);
			   return res;
		       };

	if(toLower(ext) == "wav" || toLower(ext) == "wave")
	    this->loadWAV(path.c_str());
	else if(toLower(ext) == "ogg")
	    this->loadOGG(path.c_str());
	else
	{
	    std::cout << "ERROR::AL::NOT_SUPPORTED_SOUND_FILE_FORMAT(.WAV .OGG): " << path << std::endl;
	    return;
	}

	static int first_foramt = this->format;
	if(this->data)
	{
	    if(this->format != first_foramt)
	    {
		std::cout << "ERROR::SOUND::ADD_QUEUE_SOUND::ALL_SOUNDS_FORMAT_MUST_BE_SAME!" << std::endl;
		return;
	    }
	    alBufferData(this->buffers[i], this->format, this->data, this->size, this->frequency);
	}
	else
	{
	    std::cout << "ERROR::SOUND_NOT_LOAD_DATA" << std::endl;
	    return;
	}
    }
    alGenSources(1, &this->alsrc);
    alSourceQueueBuffers(this->alsrc, this->buf_number, this->buffers);
}


void Sound::setVolume(float value) const
{
    value = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
    alSourcef(this->alsrc, AL_GAIN, value);
}


void Sound::setPitch(float value) const
{
    value = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
    alSourcef(this->alsrc, AL_PITCH, value);
}


void Sound::setLoop(bool value) const
{
    alSourcei(this->alsrc, AL_LOOPING, value ? AL_TRUE : AL_FALSE);
}


float Sound::getVolume() const
{
    float volume = 0.0f;
    alGetSourcef(this->alsrc, AL_GAIN, &volume);
    return volume;
}


float Sound::getPitch() const
{
    float pitch = 0.0f;
    alGetSourcef(this->alsrc, AL_GAIN, &pitch);
    return pitch;
}


bool Sound::isPlaying() const
{
    int state;
    alGetSourcei(this->alsrc, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}


bool Sound::isPause() const
{
    int state;
    alGetSourcei(this->alsrc, AL_SOURCE_STATE, &state);
    return state == AL_PAUSED;
}

                           
bool Sound::isStopped() const
{
    int state;
    alGetSourcei(this->alsrc, AL_SOURCE_STATE, &state);
    return state == AL_STOPPED;
}


unsigned int Sound::getSourceId() const
{
    return this->alsrc;
}


void Sound::loadWAV(const char * path)
{
    FILE* file = fopen(path, "rb");
    if(!file)
    {
	std::cout << "ERROR::LOAD_WAV_FILE_FAILED: " << path << std::endl;
	return;
    }
    
    char riff[4];
    uint32_t chunkSize;
    char wave[4];
    char fmt[4];
    uint32_t subChunck1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint32_t listChunckSize = 0;
    char* info = NULL;
    uint32_t subChunck2Size = 0;
    
    fread(riff, sizeof(riff), 1, file);
    if(!(riff[0] == 'R' && riff[1] == 'I' && riff[2] == 'F' && riff[3] == 'F'))
	std::cout << "ERROR::LOAD_WAV_RIFF: " << riff << std::endl;
    
    fread(&chunkSize, sizeof(chunkSize), 1, file);
    
    fread(wave, sizeof(wave), 1, file);
    if(!(wave[0] == 'W' && wave[1] == 'A' && wave[2] == 'V' && wave[3] == 'E'))
	std::cout << "ERROR::LOAD_WAV_WAVE: " << wave << std::endl;

    fread(fmt, sizeof(fmt), 1, file);
    if(!(fmt[0] == 'f' && fmt[1] == 'm' && fmt[2] == 't' && fmt[3] == ' '))
	std::cout << "ERROR::LOAD_WAV_FMT: " << fmt << std::endl;

    fread(&subChunck1Size, sizeof(subChunck1Size), 1, file);
    
    fread(&audioFormat, sizeof(audioFormat), 1, file);

    fread(&numChannels, sizeof(numChannels), 1, file);

    fread(&sampleRate, sizeof(sampleRate), 1, file);

    fread(&byteRate, sizeof(byteRate), 1, file);

    fread(&blockAlign, sizeof(blockAlign), 1, file);

    fread(&bitsPerSample, sizeof(bitsPerSample), 1, file);

    // If is list chunck
    char chunckid[4];
    fread(&chunckid, sizeof(chunckid), 1, file);
    if(chunckid[0] == 'L' && chunckid[1] == 'I' && chunckid[2] == 'S' && chunckid[3] == 'T')
    {
	fread(&listChunckSize, sizeof(listChunckSize), 1, file);
	info = static_cast<char*>(malloc(listChunckSize * sizeof(char)));
	fread(info, listChunckSize, 1, file);
	fread(&chunckid, sizeof(chunckid), 1, file);
    }

    if(!(chunckid[0] == 'd' && chunckid[1] == 'a' && chunckid[2] == 't' && chunckid[3] == 'a'))
    	std::cout << "ERROR::LOAD_WAV_DATAID: " << chunckid << std::endl;
    fread(&subChunck2Size, sizeof(subChunck2Size), 1, file);

#if defined(DEBUG) || defined(_DEBUG)
    std::cout << "load wav file: " << path << std::endl;
    std::cout << "chunkSize: " << chunkSize << " bytes" << std::endl;
    std::cout << "subChunck1Size: " << subChunck1Size << std::endl;
    std::cout << "audioFormat: " << audioFormat << std::endl;
    std::cout << "numChannels: " << numChannels << std::endl;
    std::cout << "sampleRate: " << sampleRate << std::endl;
    std::cout << "byteRate: " << byteRate << std::endl;
    std::cout << "blockAlign: " << blockAlign << std::endl;
    std::cout << "bitsPerSample: " << bitsPerSample << std::endl;	// stero+
    std::cout << "subChunck2Size: " << subChunck2Size << " bytes" << std::endl;
    std::cout << "info: " << info << std::endl;
#endif
    
    this->format = AL_FORMAT_STEREO16;
    if(numChannels == 1)
    {
	if(bitsPerSample == 8)
	    this->format = AL_FORMAT_MONO8;
	else if(bitsPerSample == 16)
	    this->format = AL_FORMAT_MONO16;
	else
	    std::cout << "ERROR::AL_UNSUPPORTED_BITSPERSAMPLES_NUMBER: " <<
		bitsPerSample << std::endl;
    }
    else if(numChannels == 2)
    {
	if(bitsPerSample == 8)
	    this->format = AL_FORMAT_STEREO8;
	else if(bitsPerSample == 16)
	    this->format = AL_FORMAT_STEREO16;
	else
	    std::cout << "ERROR::AL_UNSUPPORTED_BITSPERSAMPLES_NUMBER: " <<
		bitsPerSample << std::endl;
    }
    else
	std::cout << "ERROR::AL_UNSUPPORTED_CHANNEL_NUMBER: " << numChannels << std::endl;

    this->channel	= numChannels;
    this->bitsPerSample = bitsPerSample;
    this->size		= subChunck2Size;
    this->frequency	= sampleRate;

    // this->data = static_cast<char*>(malloc(subChunck2Size * sizeof(char)));
    // this->data = std::unique_ptr<char>(new char[subChunck2Size]);
    this->data = new char[subChunck2Size];
    fread(this->data, 1, subChunck2Size, file);
    fclose(file);

    return;
}


void Sound::loadOGG(const char * path)
{
    int channel;
    int sampleRate;
    short* data_ogg;
    int data_len = stb_vorbis_decode_filename(path, &channel, &sampleRate, &data_ogg);

#if defined(DEBUG)
    // std::cout << sizeof(short) << std::endl;	// 2
    // std::cout << sizeof(char)  << std::endl;	// 1
    std::cout << "Channel: "	<< channel	<< std::endl;
    std::cout << "SampleRate: " << sampleRate	<< std::endl;
    std::cout << "Data len: "	<< data_len	<< std::endl;
#endif

    this->channel = channel;
    this->frequency = sampleRate;
    // this->size = data_len;
    // this->size = data_len * (sizeof(short) / sizeof(char)) * 2;
    this->size = data_len * (sizeof(short) / sizeof(char)) * this->channel;
    ALenum format = AL_FORMAT_STEREO16;
    if(channel == 1)
	format = AL_FORMAT_MONO16;
    else if(channel == 2)
	format = AL_FORMAT_STEREO16;
    else if(channel == 4)
	format = alGetEnumValue("AL_FORMAT_QUAD16");
    else if(channel == 6)
	format = alGetEnumValue("AL_FORMAT_51CHN16");
    else if(channel == 7)
	format = alGetEnumValue("AL_FORMAT_61CHN16");
    else if(channel == 8)
	format = alGetEnumValue("AL_FORMAT_71CHN16");
    else
	std::cout << "WARNNING::SOUND_CHANNEL_NUMBER_EXCEED_TWO! " << channel << std::endl;
    this->format = format;
    this->data = data_ogg;
}


Sound::~Sound()
{
    alDeleteBuffers(this->buf_number, this->buffers);
    alDeleteSources(1, &this->alsrc);
    // if(this->data)
    // {
    // 	delete this->data;
    // 	this->data = NULL;
    // }
}
