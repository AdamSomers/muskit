#ifndef h_AudioServer
#define h_AudioServer

#include <iostream>
#include <map>
#include <mutex>

#include "RtAudio.h"

#include "AudioClient.h"

// AudioServer
// ----------------
/// \brief AudioServer is a singleton that gets callbacks from driver interfaces (RtAudio)
///  and maintains a list of AudioClients
///
/// Clients are registered and removed with AddClient and Remove Client.
/// Registered Clients are assumed to be valid at all times, and the Server does not
/// handle deallocation--make sure to delete clients after they are removed!
///
/// There is a notion of time in the form of a running sample count used by clients
/// to know whether or not to render new audio when asked for output
///
class AudioServer
{
public:
	AudioServer();
	
	~AudioServer();
	
	static AudioServer* GetInstance()
	{
		if (!sInstance)
		{
			sInstance = new AudioServer;
		}
		return sInstance;
	}
	
	// !!! need to extend for multi-channel after it works
	void AudioServerCallback(float* inBuffer, float* outBuffer, unsigned frames);
	void AudioServerCallback(const float** inBuffer, float** outBuffer, unsigned frames);
    
	void GetInput(float* buffer, int frames, int channel);
	
   /// Call this to connect AudioClients to the DAC
	void AddClient(AudioClient* c, int channelIndex);
	
	void RemoveClient(AudioClient* c, int channelIndex);
	
	void SetFs(float fs);
	
	float Fs() const;
	
	void SetInputChannels(int channels);
	
	int InputChannels() const;
	
	void SetOutputChannels(int channels);
	
	int OutputChannels() const;
	
	unsigned Time() const;
   
   void EnterLock() { fLock.lock(); }
   void ExitLock() { fLock.unlock(); }
	
private:
	static AudioServer* sInstance;
	
	float* fInputBuffer;
	
	typedef std::vector<AudioClient*> AudioClientList;
	typedef std::map<int, AudioClientList> ChannelClientMap;
	ChannelClientMap fChannelClientMap;
	
	float fFs;
	int fInputChannels;
	int fOutputChannels;
	
	unsigned fTime;
	
    std::mutex fLock;
};

// RtAudioDriver
// ----------------
/// \brief RtAudioDriver is a wrapper for RtAudio.  It passes callbacks along
/// to AudioServer.
class RtAudioDriver
{
public:
	RtAudioDriver(unsigned bufferFrames = 1024, int fs = 44100, int device = 2, int offset = 0)
	{
		channels = 2;
		
		if ( dac.getDeviceCount() < 1 )
		{
			std::cout << "\nNo audio devices found!\n";
			exit( 1 );
		}
		
		data = (double *) calloc( channels, sizeof( double ) );
		
		// Let RtAudio print messages to stderr.
		dac.showWarnings( true );
		
		// Set our stream parameters for output only.
		RtAudio::StreamParameters oParams;
		oParams.deviceId = dac.getDefaultOutputDevice();
		oParams.nChannels = channels;
		oParams.firstChannel = offset;
		
		RtAudio::StreamParameters iParams;
		iParams.deviceId = dac.getDefaultInputDevice();
		iParams.nChannels = 1;
		iParams.firstChannel = 0;
		
		options.flags |= RTAUDIO_HOG_DEVICE;
		options.flags |= RTAUDIO_SCHEDULE_REALTIME;
		options.flags |= RTAUDIO_NONINTERLEAVED;
		
		try {
			dac.openStream( &oParams, &iParams, RTAUDIO_FLOAT32, fs, &bufferFrames, &callback, (void *)data, &options );
			dac.startStream();			
			AudioServer::GetInstance()->SetFs(fs);
			AudioServer::GetInstance()->SetOutputChannels(channels);
			std::cout << dac.getStreamSampleRate() << std::endl;
			
		}
		catch ( RtError& e ) {
			e.printMessage();
			Cleanup();
		}
		
	}
	
	~RtAudioDriver()
	{
		Cleanup();
	}
	
	void Cleanup()
	{
		try {
			// Stop the stream
			dac.stopStream();
		}
		catch ( RtError& e ) {
			e.printMessage();
		}
		if ( dac.isStreamOpen() ) 
			dac.closeStream();
		free( data );
	}
	
private:
	
	static int callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
						double streamTime, RtAudioStreamStatus status, void *data )
	{
		AudioServer::GetInstance()->AudioServerCallback((float*)inputBuffer,
														(float*)outputBuffer,
														nBufferFrames);
		return 0;
	}
	
	RtAudio::StreamOptions options; //!!! change naming convention!
	unsigned channels;
	
	RtAudio dac;
	double *data;
	
};


#endif
