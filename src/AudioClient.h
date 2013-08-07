#ifndef h_AudioClient
#define h_AudioClient

#include <cstdlib>

// AudioClient
// ----------------
/// \brief AudioClient is the base class for anything that produces audio.
///
/// Clients can be registered with the AudioServer singleton for direct connection
/// to a DAC channel, or clients can be registered with other clients in some cases.
///
/// Clients must override Render, and probably shouldn't override Process
///
/// Consumers of a client's output (i.e. the DAC or another client) should
/// obtain the output using Process, as this method will return either a freshly
/// rendered block or a cached copy of a previously rendered block

class AudioClient
{
public:
	AudioClient();
    ~AudioClient();
	
	// Subclasses must override this
	virtual void Render(float* buffer, int frames) = 0;
	
	// Consumers of client output should call this
	virtual void Process(float* buffer, int frames);
	
protected:
	int fLastBufferSize;
	float* fCachedBuffer;
	unsigned fLastTime;
};

#endif
