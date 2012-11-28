#ifndef h_TypingKeyboard
#define h_TypingKeyboard

/// \brief TypingKeyboard converts ascii characters to midi notes or frequency
class TypingKeyboard
{
public:
	TypingKeyboard()
	: fOctave(4)
	{}
	
	void Run()
	{
	}
	
	void SetOctave(unsigned octave)
	{
		fOctave = octave;
	}
	
	typedef struct _KeyResult
	{
		_KeyResult() : midiNote(-1) {}
		int midiNote;
		
	} KeyResult;
	
	KeyResult ProcessKeyInput(char ascii)
	{
		int noteResult = KeyToMidiNote(ascii);
		
		KeyResult r;
		if (noteResult != -1)
		{
			r.midiNote = noteResult;
		}
		else
		{
			int octaveResult = KeyToOctave(ascii);
			if (octaveResult != -1)
			{
				// something to do here?
			}
			else // dynamics change?
			{
			}
			
		}
		return r;
	}
	
	int KeyToMidiNote(char ascii)
	{
		int midiNote = -1;
		switch (ascii) {
			case 'a':
				midiNote = 0;
				break;
			case 'w':
				midiNote = 1;
				break;
			case 's':
				midiNote = 2;
				break;
			case 'e':
				midiNote = 3;
				break;
			case 'd':
				midiNote = 4;
				break;
			case 'f':
				midiNote = 5;
				break;
			case 't':
				midiNote = 6;
				break;
			case 'g':
				midiNote = 7;
				break;
			case 'y':
				midiNote = 8;
				break;
			case 'h':
				midiNote = 9;
				break;
			case 'u':
				midiNote = 10;
				break;
			case 'j':
				midiNote = 11;
				break;
			case 'k':
				midiNote = 12;
				break;
				
			default:
				break;
		}
		
		if (midiNote != -1)
			midiNote += 12 * fOctave;
		
		return midiNote;
	}
	
	int KeyToOctave(char ascii)
	{
		int octave = -1;
		switch (ascii) {
			case '1':
				octave = 0;
				break;
			case '2':
				octave = 1;
				break;
			case '3':
				octave = 2;
				break;
			case '4':
				octave = 3;
				break;
			case '5':
				octave = 4;
				break;
			case '6':
				octave = 5;
				break;
			case '7':
				octave = 6;
				break;
			case '8':
				octave = 7;
				break;
			case '9':
				octave = 8;
				break;
			default:
				break;
		}
		
		if (octave != -1)
			fOctave = octave;
		
		return octave;
	}
	
	float KeyToFrequency(char ascii)
	{
		const float c0 = 8.1757989156;
		const int midiNote = KeyToMidiNote(ascii);
		float f = 0;
		if (midiNote != -1)
			f = powf(2, midiNote/12.f)*c0;
		return f; 
	}
	
	
private:
	unsigned fOctave;
};

#endif
