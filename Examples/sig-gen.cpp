#include <iostream>

#include "AudioServer.h"
#include "SignalGenerators.h"

int main(int argc, const char** argv)
{
   // initialize audio
   RtAudioDriver driver;

   // instantiate sine oscillator
   SinOsc s(440);

   // Attenuate by half
   Multiplier m(&s, NULL, 0.5);

   // Connect to left and right channels
   AudioServer::GetInstance()->AddClient(&m, 0);
   AudioServer::GetInstance()->AddClient(&m, 1);
   for (;;)
   {
      sleep(1);
   }
   
   return 0;
}
