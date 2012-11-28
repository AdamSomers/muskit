#include <iostream>

#include "AudioServer.h"
#include "SignalGenerators.h"

int main(int argc, const char** argv)
{
   RtAudioDriver driver;
   FMOsc fmo1(440);
   fmo1.SetModFreq(10);
   fmo1.SetModIndex(5);
   Multiplier m(&fmo1, NULL, 0.5);
   AudioServer::GetInstance()->AddClient(&m, 0);
   AudioServer::GetInstance()->AddClient(&m, 1);
   
   for (;;)
   {
      sleep(1);
   }
   
   return 0;
}
