#include "AudioServer.h"

#include <cassert>


AudioServer* AudioServer::sInstance = NULL;

AudioServer::AudioServer()
: fFs(44100.f)
, fTime(0)
, fInputBuffer(NULL)
, fInputChannels(1)
, fOutputChannels(1)
{
	pthread_mutex_init(&fLock, NULL);
}

AudioServer::~AudioServer()
{
	fChannelClientMap.clear();
}

void AudioServer::AudioServerCallback(float* inBuffer, float* outBuffer, unsigned frames)
{
	pthread_mutex_lock(&fLock);
	float* buffer = (float*)outBuffer;
	float tmp[frames];
	
	if (fInputBuffer)
	{
		delete[] fInputBuffer;
	}
	fInputBuffer = new float[frames * fInputChannels];
	for (int i = 0; i < frames * fInputChannels; ++i)
	{
		fInputBuffer[i] = inBuffer[i];
	}
	
	for (int channel = 0; channel < fOutputChannels; ++channel)
	{
		memset(buffer, 0.f, frames * sizeof(float));
		
		ChannelClientMap::iterator clientListIter = fChannelClientMap.find(channel);
		if (clientListIter != fChannelClientMap.end())
		{
			AudioClientList* clients = &(*clientListIter).second;
			assert(clients);
			
			AudioClientList::iterator i;
			for (i = clients->begin(); i != clients->end(); ++i)
			{
				memset(tmp, 0.f, frames * sizeof(float));
				
				(*i)->Process(tmp, frames);
				
				for (int frame = 0; frame < frames; ++frame)
				{
					buffer[frame] += tmp[frame];
				}
#if 0
				// check for signals out of range [-1, 1]
				for (int frame = 0; frame < frames; ++frame)
				{
					if (abs(buffer[frame]) > 1.f)
					{
						std::cout << "clip!\n";
						buffer[frame] = (rand() % 32768) / 32768.f - 0.5f;
					}
				}
#endif				
			}
		}
		buffer = buffer + frames;
	}
	
	fTime += frames;
	pthread_mutex_unlock(&fLock);
}

void AudioServer::GetInput(float* buffer, int frames, int channel)
{
	//pthread_mutex_lock(&fLock);
	assert(channel < fInputChannels);
	for (int i = 0; i < frames; ++i)
	{
		buffer[i] = fInputBuffer[i + frames * channel];
	}
	//pthread_mutex_unlock(&fLock);
}

void AudioServer::AddClient(AudioClient* c, int channelIndex)
{
	ChannelClientMap::iterator clientListIter = fChannelClientMap.find(channelIndex);
	if (clientListIter == fChannelClientMap.end())
	{
		fChannelClientMap.insert(std::make_pair<int, AudioClientList>(channelIndex, AudioClientList()));
		clientListIter = fChannelClientMap.find(channelIndex);
	}
	
	AudioClientList* clients = &(*clientListIter).second;
	assert(clients);
	
	AudioClientList::iterator clientiter = std::find(clients->begin(), clients->end(), c);
	if (clientiter == clients->end())
	{
		clients->push_back(c);
	}
}

void AudioServer::RemoveClient(AudioClient* c, int channelIndex)
{
	ChannelClientMap::iterator clientListIter = fChannelClientMap.find(channelIndex);
	if (clientListIter != fChannelClientMap.end())
	{
		AudioClientList* clients = &(*clientListIter).second;
		assert(clients);
		AudioClientList::iterator i;
		i = std::find(clients->begin(), clients->end(), c);
		if (i != clients->end())
		{
			clients->erase(i);
		}
		if (clients->empty())
		{
			fChannelClientMap.erase(clientListIter);
		}
	}
}

void AudioServer::SetFs(float fs)
{
	fFs = fs;
}

float AudioServer::Fs() const
{
	return fFs;
}

void AudioServer::SetInputChannels(int channels)
{
	fInputChannels = channels;
}

int AudioServer::InputChannels() const { return fInputChannels; }

void AudioServer::SetOutputChannels(int channels)
{
	fOutputChannels = channels;
}

int AudioServer::OutputChannels() const { return fOutputChannels; }

unsigned AudioServer::Time() const
{
	return fTime;
}
