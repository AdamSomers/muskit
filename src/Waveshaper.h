#ifndef h_Waveshaper
#define h_Waveshaper

#include "AudioClient.h"
#include "Interpolators.h"
#include <memory>

// Interpolator
// ----------------
/// \brief Class with inline interpolation routines of various types
///

class Waveshaper : public AudioClient
{
public:
   Waveshaper()
   : fWavetable(NULL)
   , fReadIndex(0)
   , fWavetableSize(0)
   {
      
   }
   
   ~Waveshaper()
   {
      if (fWavetable)
      {
         delete[] fWavetable;
      }
   }
   
   void SetWavetable(float* buffer, int frames)
   {
      if (fWavetable)
      {
         delete[] fWavetable;
      }
      fWavetable = new float[frames*sizeof(float)];
      fWavetableSize = frames;
      memcpy(fWavetable, buffer, frames);
   }
   
   virtual void Render(float* buffer, int frames)
   {
      float input = 0.f;
      double index = 0.f;
      for (int i = 0; i < frames; ++i)
      {
         input = buffer[i];
         // !!! inputs will exceed [-1, 1] need a way to determine scaling correctly
         index = (input * 0.5 + 0.5) * fWavetableSize;
         buffer[i] = fInterpolator.Interpolate(fWavetable, index, fWavetableSize);
      }
   }
   
private:
   float* fWavetable;
   int fWavetableSize;
   double fReadIndex;
   Interpolator fInterpolator;
};

#endif