 #ifndef h_Poly
#define h_Poly

#include "AudioClient.h"
#include "MidiServer.h"

#include <deque>

// Voice
// ----------------
/// \brief Base class for generators that should play polyphonically (with Poly)
///
/// Subclasses should call base class versions of NoteOn and NoteOff
///
class Voice : public AudioClient
{
public:
   Voice()
   : fTotalRendered(0)
   , fMax(0)
   , fPlaying(false)
   {}
   
   bool Done() const
   {
      if (fTotalRendered == fMax)
      {
         return true;
      }
      else
      {
         return false;
      }
   }
   
   bool Playing() const
   {
      return fPlaying;
   }
   
   virtual void NoteOn(int note, int velocity)
   {
      fPlaying = true;
   }
   
   virtual void NoteOff()
   {
      fPlaying = false;
   }
   
protected:
   int fTotalRendered;
   int fMax;
   bool fPlaying;
};

// Poly
// ----------------
/// \brief Manager of Voices.  Accepts midi data and selects from a pool of pre-allocated
///   voices.
//
//
class Poly : public AudioClient
           , public MidiClient
{
public:
	Poly() {}
	
	~Poly()
	{
		std::deque<Voice*>::iterator i;
		for (i = fVoices.begin(); i != fVoices.end(); ++i)
		{
			delete (*i);
		}
		
		fVoices.clear();
		
	}
	
	void Render(float* buffer, int frames)
	{
		float tmp[frames];
		memset(tmp, 0.f, frames * sizeof(float));
		
		std::deque<Voice*>::iterator i;
		for (i = fVoices.begin(); i != fVoices.end(); ++i)
		{
			//if (!(*i)->Done())  // only play voices that have data to render
			{
				(*i)->Process(buffer, frames);
				for (int i = 0; i < frames; ++i)
				{
					tmp[i] += buffer[i];
				}
			}
		}
		
		for (int i = 0; i < frames; ++i)
		{
			buffer[i] = tmp[i];
		}
	}
	
	void NoteOn(int note, int velocity)
	{
      // Check to see if this note is already playing
      NoteMap::iterator i = fNoteMap.find(note);
      if (i != fNoteMap.end())
      {
         // just re-trigger
         Voice* v = (*i).second;
         
         v->NoteOn(note, velocity);
      }
      else
      {
         Voice* v = NULL;
         
         // find the first voice that isn't playing
         VoiceQueue::iterator i;
         for (i = fVoices.begin(); i != fVoices.end(); ++i)
         {
            if (!(*i)->Playing())
            {
               v = (*i);
               fVoices.erase(i);
               break;
            }
         }
         
         if (!v) // all voices in use, steal the oldest one
         {
            v = fVoices.front();
            fVoices.pop_front();
         }
         
         fNoteMap.insert(std::make_pair(note, v)); // insert into note map for note-off handling
         v->NoteOn(note, velocity);
         
         fVoices.push_back(v);
      }
	}
   
   void NoteOff(int note)
   {
      NoteMap::iterator i = fNoteMap.find(note);
      if (i != fNoteMap.end())
      {
         Voice* v = (*i).second;
         v->NoteOff();
         fNoteMap.erase(i);
      }
   }
	
	void AddVoice(Voice* c)
	{
		std::deque<Voice*>::iterator i = std::find(fVoices.begin(), fVoices.end(), c);
		if (i == fVoices.end())
		{
			fVoices.push_back(c);
		}
	}
   
	void RemoveVoice(Voice* c)
	{
		std::deque<Voice*>::iterator i = std::find(fVoices.begin(), fVoices.end(), c);
		if (i != fVoices.end())
		{
			fVoices.erase(i);
		}
	}
	
private:
   typedef std::deque<Voice*> VoiceQueue;
	VoiceQueue fVoices;
   
   typedef std::map<int, Voice*> NoteMap;
   NoteMap fNoteMap;
};

#endif
