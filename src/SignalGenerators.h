#ifndef h_SignalGenerators
#define h_SignalGenerators

#include <cmath>
#include <cstdlib>
#include <queue>
#include <atomic>

#include "AudioClient.h"
#include "AudioServer.h"
#include "MathHelpers.h"
#include "Interpolators.h"

// Noise Source
// ----------------
/// \brief White noise generation pilfered from http://musicdsp.org/showone.php?id=113
///
/// Pink noise not yet implemented
///
class NoiseSource : public AudioClient
{
public:
	NoiseSource(float gain = 1.f, int color = kWhite)
	: fGain(gain)
	, fColor(color)
	{
	}
	
	void Render(float* buffer, int frames)
	{
		const float fs = AudioServer::GetInstance()->Fs();
		
		for (int i = 0; i < frames; ++i)
		{
			float R1 = (float) rand() / (float) RAND_MAX;
			float R2 = (float) rand() / (float) RAND_MAX;
			
			buffer[i] = tanh((float) sqrt( -2.0f * log( R1 )) * cos( 2.0f * MusKit::PI * R2 ));
		}
	}
	
	enum NoiseColor
	{
		kWhite = 0,
		kPink,
		
		kNumNoiseColors
	};
	
private:
	float fGain;
	int fColor;
};

// Input Source
// ----------------
/// \brief Provides one channel of input data from the Server
///
class InputSource : public AudioClient
{
public:
	InputSource(int channel = 0)
	: fChannel(channel)
	{
	}
	
	void Render(float* buffer, int frames)
	{
		float* inputBuffer = new float[frames];
		AudioServer::GetInstance()->GetInput(inputBuffer, frames, fChannel);
		for (int i = 0; i < frames; ++i)
		{
			buffer[i] = inputBuffer[i];
		}
      delete[] inputBuffer;
	}
	
private:
	int fChannel;
};

// SampleAccumulator
// --------------------
/// \brief Stores all samples that come in and provides access to them
///
/// Stored buffer is cleared when accessed
///
class SampleAccumulator : public AudioClient
{
public:
	SampleAccumulator()
   : fSamplesPerPixel(12)
   , fMinSample(0.f)
   , fMaxSample(0.f)
   , fCount(0)
   , fThePeakBuffer(&fPeakBuffer)
	{
	}
   
   ~SampleAccumulator()
   {
   }
	
	void Render(float* buffer, int frames)
	{
      
      if (fInput == NULL)
         return;

      fInput->Process(buffer, frames);
      PeakBuffer* peakBuffer = fThePeakBuffer.load();
		for (int i = 0; i < frames; ++i)
		{
         if (fCount >= (fSamplesPerPixel - 1))
         {
            peakBuffer->push_back(std::make_pair(fMaxSample, fMinSample));
            fMinSample = buffer[i];
            fMaxSample = buffer[i];
            fCount = 0;
         }
         else
         {
            if (buffer[i] > fMaxSample)
            {
               fMaxSample = buffer[i];
            }
            else if (buffer[i] < fMinSample)
            {
               fMinSample = buffer[i];
            }
            ++fCount;
         }
      }
   }
   
   typedef std::pair<float, float> PeakSample;
   typedef std::deque<PeakSample> PeakBuffer;
   
   PeakBuffer Get()
   {
      AudioServer::GetInstance()->EnterLock();
      PeakBuffer samples;
      if (fThePeakBuffer == &fPeakBuffer)
      {
         fThePeakBuffer = &fPeakBuffer2;
         samples = PeakBuffer(fPeakBuffer);
         fPeakBuffer.clear();

      }
      else
      {
         fThePeakBuffer = &fPeakBuffer;
         samples = PeakBuffer(fPeakBuffer2);
         fPeakBuffer2.clear();
      }
      AudioServer::GetInstance()->ExitLock();
      return samples;
   }
   
   int GetSize() const
   {
      return fThePeakBuffer.load()->size();
   }
    
   void SetInput(AudioClient* input)
   {
      fInput = input;
   }

   void SetSamplesPerPixel(int samplesPerPixel)
   {
      fSamplesPerPixel = samplesPerPixel;
   }

private:
   
   PeakBuffer fPeakBuffer;
   PeakBuffer fPeakBuffer2;
   std::atomic<PeakBuffer*> fThePeakBuffer;
   int fSamplesPerPixel;
   float fMinSample;
   float fMaxSample;
   int fCount;

   AudioClient* fInput;
};

// Oscillator
// ----------------
/// \brief Base class for oscillators.  Just a basic set of parameters to avoid
/// code duplication.
///
class Oscillator : public AudioClient
{
public:
	Oscillator(float freq = 440.f, float gain = 1.f, float width = 0.5f)
	: fFreq(freq)
	, fGain(gain)
	, fWidth(width)
   , fFreqZ(0)
   , fGainZ(0)
	, fT(0)
   , fPhase(0)
	{
	}
	
	void SetFreq(float freq)
	{
		fFreq = freq;
	}
	
	void SetGain(float g)
	{
		fGain = g;
	}
	
	void SetWidth(float w)
	{
		fWidth = w;
	}
    
    float Freq() const { return fFreq; }
	
protected:
	float fFreq;
	float fGain;
	float fWidth;
   
	float fT; // delete me
   float fPhase;
   
   float fFreqZ;
   float fGainZ;
};

// SinOsc
// ----------------
/// \brief Generates a sine wave
///  
/// Would be nice to provide selectable approximations for efficiency
///
class SinOsc : public Oscillator
{
public:
	SinOsc(float freq = 440.f, float gain = 1.f)
	: Oscillator(freq, gain)
	{
	}
	
	void Render(float* buffer, int frames)
	{
		const float fs = AudioServer::GetInstance()->Fs();
		const int period = 1.f / fFreq * fs;
		
		for (int i = 0; i < frames; ++i)
		{
			buffer[i] = fGainZ * sin(fPhase);
			
			fPhase += fFreq * (2*MusKit::PI / fs);
			if (fPhase >= 2*MusKit::PI)
				fPhase -= 2*MusKit::PI;
         
            float a1 = exp(-(1/fs)/(1/6.91));
         fFreqZ = fFreq * (1-a1) + a1 * fFreqZ;
         fGainZ = fGainZ * 0.999 + fGain * 0.001;
         //fFreqZ = fFreq;
         //fGainZ = fGain;
		}
	}
};

// FMOsc
// ----------------
/// \brief Two-operator FM oscillator
///
class FMOsc : public Oscillator
{
public:
	FMOsc(float freq = 440.f, float gain = 1.f)
	: Oscillator(freq, gain)
	{
      fModOsc = new SinOsc(100.f);
      fModOsc->SetGain(0);
	}
	
   ~FMOsc()
   {
      delete fModOsc;
   }
   
	void Render(float* buffer, int frames)
	{
      
		const float fs = AudioServer::GetInstance()->Fs();
		const int period = 1.f / fFreqZ * fs;
		
      float* modBuffer = new float[frames];
      fModOsc->Render(modBuffer, frames);
      
		for (int i = 0; i < frames; ++i)
		{
			buffer[i] = fGainZ * sin(fPhase + modBuffer[i]);
			
         fPhase += fFreqZ * (2*MusKit::PI / fs);
			if (fPhase >= 2*MusKit::PI)
				fPhase -= 2*MusKit::PI;
         
         fFreqZ = fFreqZ * 0.999 + fFreq * 0.001;
         fGainZ = fGainZ * 0.999 + fGain * 0.001;
		}
      delete[] modBuffer;
	}
   
   void SetModIndex(float index)
   {
      fModOsc->SetGain(index);
   }
   
   void SetModFreq(float freq)
   {
      fModOsc->SetFreq(freq);
   }
   
private:
   SinOsc *fModOsc;
};

// SawOsc
// ----------------
/// \brief Generates a naive sawtooth (ramp)
///  
/// width parameter for changing the shape from ramp to triangle to inverse ramp
//
class SawOsc : public Oscillator
{
public:
	SawOsc(float freq = 440.f, float gain = 1.f, float width = 0.f)
	: Oscillator(freq, gain, width)
	{
	}
	
	void Render(float* buffer, int frames)
	{
		const float fs = AudioServer::GetInstance()->Fs();
		
		int period = 1.f / fFreq * fs;
		int rising = period * (1 - fWidth);
		int falling = period - rising;
		
		for (int i = 0; i < frames; ++i)
		{
			if (fT < rising)
			{
				buffer[i] = 2 * fT / (float)rising - 1;
			}
			else
			{
				buffer[i] = 2 * (1 - ((fT - rising) / (float)falling)) - 1;
			}
			++fT;
			if (fT >= period)
				fT = 0;
		}
	}
};

// PwmOsc
// ----------------
/// \brief Naive pulse width modulation oscillator class
//
class PwmOsc : public Oscillator
{
public:
	PwmOsc(float freq = 440.f, float gain = 1.f, float width = 5.f)
	: Oscillator(freq, gain, width)
	{
	}
	
	void Render(float* buffer, int frames)
	{
		const float fs = AudioServer::GetInstance()->Fs();
		
		int period = 1.f / fFreq * fs;
		int pos = period * (1 - fWidth);
		
		for (int i = 0; i < frames; ++i)
		{
			if (fT < pos)
			{
				buffer[i] = 1.f;
			}
			else
			{
				buffer[i] = 0.f;
			}
			
			++fT;
			if (fT >= period)
				fT = 0;
		}
	}
};

// PulseTrain
// ----------------
/// \brief Generates impulses at a given frequency
//
class PulseTrain : public Oscillator
{
public:
	PulseTrain(float freq = 440.f, float gain = 1.f, float width = 0.f)
	: Oscillator(freq, gain, width)
	{
	}
	
	void Render(float* buffer, int frames)
	{
		const float fs = AudioServer::GetInstance()->Fs();
		
		int period = 1.f / fFreq * fs;
		
		for (int i = 0; i < frames; ++i)
		{
			if (fT == 0)
			{
				buffer[i] = 1.f;
			}
			
			++fT;
			if (fT >= period)
				fT = 0;
		}
	}
};

// AdditiveSinOsc
// ----------------
/// \brief Uses an array of SinOscs to recreate the harmonic series for sawtooth and
/// square waves.
///
/// Selection of square vs. saw unimplemented
///
class AdditiveSinOsc : public Oscillator
{
public:
	AdditiveSinOsc(float freq = 440.f, float gain = 1.f, int order = 1)
	: Oscillator(freq, gain)
	, fOrder(order)
	{
		for (int i = 0; i < order; ++i)
		{
			// saw
			fSinOscs.push_back(new SinOsc(fFreq * (i+1), 1.f / (i+1)));
			
			// square
			//fSinOscs.push_back(new SinOsc(fFreq * (i * 2 + 1), 1.f / (i * 2 + 1)));
		}
	}
	
	~AdditiveSinOsc()
	{
		std::vector<SinOsc*>::iterator i;
		for (i  = fSinOscs.begin(); i != fSinOscs.end(); ++i)
		{
			delete (*i);
		}
		
		fSinOscs.clear();
	}
	
	void Render(float* buffer, int frames)
	{
		float* tmp = new float[frames];
		
		std::vector<SinOsc*>::iterator i;
		for (i = fSinOscs.begin(); i != fSinOscs.end(); ++i)
		{
			memset(tmp, 0.f, frames * sizeof(float));
			
			(*i)->Render(tmp, frames);
			
			for (int j = 0; j < frames; ++j)
			{
				buffer[j] += tmp[j];
			}
		}
      delete[] tmp;
	}
	
	void SetFreq(float freq)
	{
		std::vector<SinOsc*>::iterator i;
		int n = 0;
		for (i = fSinOscs.begin(); i != fSinOscs.end(); ++i)
		{
			(*i)->SetFreq(freq * (++n));
		}
		
		fFreq = freq;
	}
	
private:
	float fOrder;
	std::vector<SinOsc*> fSinOscs;
};

// Wavetable Osc
// ----------------
/// \brief Uses a lookup table for synthesis
///
class WavetableOsc : public Oscillator
{
public:
	WavetableOsc(float freq = 440.f, float gain = 1.f, int tableSize = 512)
	: Oscillator(freq, gain)
	, fTableSize(tableSize)
   , fReadIndex(0)
   , fStep(1)
	{
      fLookupTable = new float[tableSize];
		memset(fLookupTable, 0, tableSize*sizeof(float));
      
      for (int i = 0; i < tableSize; ++i)
      {
         fLookupTable[i] = sin(2*MusKit::PI*(i/(double)tableSize));
      }
      
      this->SetFreq(freq);
	}
	
	~WavetableOsc()
	{
		delete[] fLookupTable;
	}
	
	void Render(float* buffer, int frames)
	{  
      for (int i = 0; i < frames; ++i)
      {
         buffer[i] = fInterpolator.Interpolate(fLookupTable, fReadIndex, fTableSize);
         fReadIndex += fStep;
         if (fReadIndex >= fTableSize) 
         {
            fReadIndex -= fTableSize;
         }
      }
	}
	
	void SetFreq(double freq)
	{
      fFreq = freq;
		const double fs = AudioServer::GetInstance()->Fs();
		double period = 1.0 / fFreq * fs;
      std::cout << period << std::endl;
      fStep = fTableSize / period;
	}
   
   void SetInterpolationType(int type) { fInterpolator.SetType(type); }
	
private:
	int fTableSize;
   double fReadIndex;
   double fStep;
	float* fLookupTable;
   Interpolator fInterpolator;
};



// Multiplier
// ----------------
/// \brief Multiplier either multiplies two signals (fA * fB) or one signal by a constant value (fA * fConst)
///
/// the constant is used when fB == NULL
//
class Multiplier : public AudioClient
{
public:
	Multiplier(AudioClient* a = NULL, AudioClient* b = NULL, float constVal = 1.f)
	: fA(a)
	, fB(b)
	, fConst(constVal)
    , fConstZ(constVal)
	{
	}
	
	void Render(float* buffer, int frames)
	{
		if (fA)
		{
			fA->Process(buffer, frames);
			
			if (!fB)
			{
				for (int i = 0; i < frames; ++i)
				{
                    fConstZ = fConst * 0.01 + fConstZ * 0.99;
					buffer[i] *= fConstZ;
				}
			}
			else
			{
				float* tmp = new float[frames];
				fB->Process(tmp, frames);
				for (int i = 0; i < frames; ++i)
				{
					buffer[i] *= tmp[i];
				}
                delete[] tmp;
			}
		}
	}
	
	void SetA(AudioClient* a)
	{
		fA = a;
	}
	
	void SetB(AudioClient* b)
	{
		fB = b;
	}
	
	void SetVal(float val)
	{
		fConst = val;
	}
	
private:
	AudioClient* fA;
	AudioClient* fB;
	float fConst;
    float fConstZ;
};

// Adder
// ----------------
/// \brief Adder sums signals, and applies a constant offset (default 0) to the summed signals
///
/// Signals cannot be added to themselves
///
class Adder : public AudioClient
{
public:
	Adder(float constVal = 0.f)
	: fConst(constVal)
	{
	}
	
	void Render(float* buffer, int frames)
	{
		float* tmp = new float[frames];
		memset(tmp, 0.f, frames * sizeof(float));
		
		std::vector<AudioClient*>::iterator i;
		for (i = fClients.begin(); i != fClients.end(); ++i)
		{
			(*i)->Process(buffer, frames);
			for (int i = 0; i < frames; ++i)
			{
				tmp[i] += buffer[i];
			}
		}
		
		for (int i = 0; i < frames; ++i)
		{
			buffer[i] = tmp[i] + fConst;
		}
      delete[] tmp;
	}
	
	void SetVal(float val)
	{
		fConst = val;
	}
	
	void AddInput(AudioClient* c)
	{
		std::vector<AudioClient*>::iterator i = std::find(fClients.begin(), fClients.end(), c);
		if (i == fClients.end())
		{
			fClients.push_back(c);
		}
	}
	
	void RemoveInput(AudioClient* c)
	{
		std::vector<AudioClient*>::iterator i = std::find(fClients.begin(), fClients.end(), c);
		if (i != fClients.end())
		{
			fClients.erase(i);
		}
	}
	
private:
	std::vector<AudioClient*> fClients;
	float fConst;
};

// StateVariable
// ----------------
/// \brief StateVariable implements a filter with LP, HP, BP & Notch modes
///
/// 
//
class StateVariable : public AudioClient
{
public:
	StateVariable(AudioClient* input = NULL)
	: fInput(input),
    _sr(44100.f),
    _freq(1000.f),
    _freqZ(1000.f),
    _res(0),
    _q(2.f),
    _f(0.f),
    _type(kOff)
    {
        reset();
        setType(2);
        _updateCoefficient();
        _updateQ();
    }
    
    enum Type
    {
        kOff = 0,
        kLowpass,
        kHighpass,
        kBandpass,
        kNotch,
        kNumFilterTypes
    };
	
	void Render(float* buffer, int frames)
	{
		if (fInput)
		{
            fInput->Process(buffer, frames);
            for (int i = 0; i < frames; ++i)
            {
                float input = buffer[i];
                _low = _low + _f * _band;
                _high = input - _low - _q * _band;
                _band = tanh(_f * _high + _band);
                _notch = _low + _high;
                if (_freqZ != _freq) {
                    _freqZ = 0.9999 * _freqZ + 0.0001 * _freq;
                    _updateCoefficient();
                }
                buffer[i] = *_out;
            }
		}
	}
    
    int getType() const { return _type; }
    float getRes() const { return _res; }
    void setType(int type)
    {
        _type = type;
        switch (type)
        {
            case kLowpass:
                _out = &_low;
                break;
            case kHighpass:
                _out = &_high;
                break;
            case kBandpass:
                _out = &_band;
                break;
            case kNotch:
                _out = &_notch;
                break;
        }
    }
    
    void setFreq(float freq)
    {
        _freq = freq;
        _updateCoefficient();
    }
    
    void setRes(float res)
    {
        _res = res;
        _updateQ();
    }
    
    void setSampleRate(float sr)
    {
        _sr = sr;
    }
    
    void reset()
    {
        _low = _high = _band = _notch = 0.f;
    }

	
	void SetInput(AudioClient* in)
	{
		fInput = in;
	}
	
private:
	AudioClient* fInput;
    void _updateCoefficient()
    {
        _f = 2 * sinf(3.141593f * _freqZ / _sr);
    }
    
    void _updateQ()
    {
        _q = 2 - 2 * _res;
    }
    
    float _sr;
    float _freq;
    float _freqZ;
    float _res;
    float _q;
    float _f;
    
    int _type;
    float _low;
    float _high;
    float _band;
    float _notch;
    
    float* _out;
};

#endif