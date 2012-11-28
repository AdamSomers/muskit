#include "AudioClient.h"
#include "AudioServer.h"

AudioClient::AudioClient()
: fLastBufferSize(0)
, fCachedBuffer(NULL)
, fLastTime(0)
{
}

void AudioClient::Process(float* buffer, int frames)
{
	if (!fCachedBuffer)
	{
		fCachedBuffer = new float[frames];
		memset(fCachedBuffer, 0.f, frames * sizeof(float));
		this->Render(fCachedBuffer, frames);
		fLastTime = AudioServer::GetInstance()->Time();
	}
	
	if (fLastTime != AudioServer::GetInstance()->Time())
	{
      if (frames != fLastBufferSize)
      {
         delete[] fCachedBuffer;
         fCachedBuffer = NULL;
         fCachedBuffer = new float[frames];
         fLastBufferSize = frames;
      }
      
		memset(fCachedBuffer, 0.f, frames * sizeof(float));
		this->Render(fCachedBuffer, frames);
		fLastTime = AudioServer::GetInstance()->Time();
	}
	
	for (int i = 0; i < frames; ++i)
	{
		buffer[i] = fCachedBuffer[i];
	}
}
