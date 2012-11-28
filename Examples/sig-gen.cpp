#include <iostream>

#include "AudioServer.h"
#include "SignalGenerators.h"

int main(int argc, const char** argv)
{
   RtAudioDriver driver;
   WavetableOsc w(50, 1.f, 64);
   SinOsc s(80);
   Multiplier m1(&w, NULL, 0.5);
   Multiplier m2(&s, NULL, 0.5);
   AudioServer::GetInstance()->AddClient(&m1, 0);
   AudioServer::GetInstance()->AddClient(&m1, 1);
   
   int interpolationType = Interpolator::kInterpolationTypeNone;
   for (;;)
   {
      sleep(1);
      s.SetFreq(s.Freq());
      w.SetFreq(w.Freq());
      w.SetInterpolationType(interpolationType);
      interpolationType = (interpolationType + 1) % Interpolator::kNumInterpolationTypes;  
   }
   
   return 0;
}
