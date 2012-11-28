#ifndef h_Voices
#define h_Voices

#include "Poly.h"
#include "MathHelpers.h"

// Karplus
// ----------------
/// \brief Karplus-Strong string model
///
class Karplus : public Voice
{
public:
   Karplus(float time = 1.f)
   : fMaxSize(65536)
   , fFeedback(0.8f)
   , fGain(1.f)
   {
      const float fs = AudioServer::GetInstance()->Fs();
      fBufferSize = fs * time;
      fBuffer = new float[fMaxSize];
      memset(fBuffer, 0.f, fMaxSize * sizeof(float));
      fR = 0;
      fW = fBufferSize;
   }
   
   ~Karplus()
   {
      delete[] fBuffer;
   }
   
   void Render(float* buffer, int frames)
   {
      for (int i = 0; i < frames; ++i)
      {
         int r = fR & (fMaxSize - 1);
         int w = fW & (fMaxSize - 1);
         fBuffer[w] = fBuffer[r] * fFeedback;
         fBuffer[w] = (fBuffer[w] + fBuffer[(fW - 1) & (fMaxSize - 1)]) * 0.5f;
         buffer[i] = fBuffer[r] * fGain;
         ++fW;
         ++fR;
         
         ++fTotalRendered;
      }
   }
   
   void NoteOn(int note, int velocity)
   {
      Voice::NoteOn(note, velocity);
      
      const float c0 = 8.1757989156;
      float f = 0;
      f = powf(2, note/12.f)*c0;
      
      fTotalRendered = 0;
      
      std::cout << "note: " << note << " freq: " << f << std::endl;
      
      this->SetLength(AudioServer::GetInstance()->Fs() / f);
      
      fFeedback = 0.99f;
      fGain = velocity / 127.f;
      
      this->Excite();
   }
   
   void NoteOff()
   {
      Voice::NoteOff();
      
      fFeedback = 0.5;
   }
   
   void SetTime(float time)
   {
      const float fs = AudioServer::GetInstance()->Fs();
      SetLength(fs * time);
   }
   
   void SetLength(int length)
   {
      assert(length <= fMaxSize);
      fBufferSize = length;
      
      fW = fR + fBufferSize;
      
      fMax = length * 100;
      
   }
   
   void SetFeedback(float feedback)
   {
      fFeedback = feedback;
   }
   
   void Excite()
   {
      for (int i = 0; i < fBufferSize; ++i)
      {
         float R1 = (float) rand() / (float) RAND_MAX;
         float R2 = (float) rand() / (float) RAND_MAX;
         
         fBuffer[(fR + i) & (fMaxSize - 1)] = tanh((float) sqrt( -2.0f * log( R1 )) * cos( 2.0f * PI * R2 ));
      }
   }
   
private:
   float* fBuffer;
   float fMaxTime;
   int fMaxSize;
   int fBufferSize;
   int fR;
   int fW;
   float fFeedback;
   float fGain;
};
   
#endif
