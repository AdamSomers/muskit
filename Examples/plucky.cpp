#include <iostream>

#include "AudioServer.h"
#include "TypingKeyboard.h"
#include "MidiServer.h"
#include "Voices.h"
#include "Poly.h"

#include <termios.h>
#include <iomanip>

#define MAX_VOICES 20

termios SetupTermios()
{
	termios before, after;
	tcgetattr (STDIN_FILENO, &before); // fill 'before' with current termios values
	after = before; // make a copy to be modified
	after.c_lflag &= (~ICANON); // Disable canonical mode, including line buffering
	after.c_lflag &= (~ECHO); // Don't echo characters on the screen (optional)
	tcsetattr (STDIN_FILENO, TCSANOW, &after); // Set the modified flags
	return before;
}

void CleanupTermios(termios before)
{
	tcsetattr (STDIN_FILENO, TCSANOW, &before);
}

int main(int argc, const char** argv)
{
	// Set up terminal to give input without 'enter' key
	termios before = SetupTermios();
	char ch;
	
	std::cout << "\n\nWelcome to Plucky!\nYou can play with the computer keys (letters for notes, numbers for octaves) or use a midi controller.\n\nPress ESC at any time to quit\n\n";
	
	
	std::cout << "Hit Escape to quit.\n";
	
	RtAudioDriver driver;    // init rtaudio
	RtMidiDriver midiDriver; // init rtmidi
	
	std::vector<std::string> midiPortNames = midiDriver.getInputPortNameList();
	
	if (midiPortNames.size() > 0)
	{
		int portNumber = 0; 
		if (midiPortNames.size() > 1)
		{
			std::cout << "\nYou have more than one midi controller! please select which one to use:\n";
			for (int i = 0; i < midiPortNames.size(); ++i)
			{
				std::cout << "   " << i << ": " << midiPortNames.at(i) << std::endl;
			}
			
			do {
				std::cin.get(ch);
				portNumber = ch - 48;
				
				if (portNumber < 0 || portNumber >= midiPortNames.size())
					std::cout << "Not a valid port number!\n";
				
				if (ch==27)
				{
					CleanupTermios(before);
					exit(0);
				}
			} while (portNumber < 0 || portNumber >= midiPortNames.size());
		}
		
		midiDriver.OpenInputPort(portNumber);
		
	}
	
	Poly p;
	for (int i = 0; i < MAX_VOICES; ++i)
	{
		Karplus* string = new Karplus(0.009);
		p.AddVoice(string); // poly owns its clinets and WILL delete them when it goes out of scope
	}
	
	// connect poly object to audio and midi servers
	AudioServer::GetInstance()->AddClient(&p, 0);
	AudioServer::GetInstance()->AddClient(&p, 1);
	MidiServer::GetInstance()->AddClient(&p, 0);
	
	TypingKeyboard k;
	k.SetOctave(4);
   
	// read input and send note-ons to poly object
	do
	{
		std::cin.get(ch);
      
		// convert a character to a midi note, or change the octave
		TypingKeyboard::KeyResult kr = k.ProcessKeyInput(ch);
		
		if (kr.midiNote != -1)
		{
			p.NoteOn(kr.midiNote, 127);
		}
	} while((int) ch != 27); // exit on 'esc'
	
	// restore terminal to previous state
	CleanupTermios(before);
	
	return 0;
}
