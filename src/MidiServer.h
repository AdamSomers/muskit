#ifndef h_MidiServer
#define h_MidiServer

#include "RtMidi.h"

//#define qVerbose 1

// MidiClient
// ----------------
/// \brief MidiClient is the base class for the anything that consumes MIDI.
///
/// Client must be registered with the MidiServer singleton to get midi callbacks.
/// They must also override all MidiClient methods.
class MidiClient
{
public:
	virtual void NoteOn(int note, int velocity) = 0;
   virtual void NoteOff(int note) = 0;
};

// MidiServer
// ----------------
/// \brief MidiServer is a singleton that gets callbacks from RtMidi and maintains
/// a list of midi clients.
///
/// Clients are registered with AddClient and removed with RemoveClient.  The
/// MidiServer is not responsible for deallocating removed clients! 
class MidiServer
{
public:
	MidiServer() {}
	
	~MidiServer();
	
	static MidiServer* GetInstance()
	{
		if (!sInstance)
		{
			sInstance = new MidiServer;
		}
		return sInstance;
	}
	
	void MidiServerCallback(double deltatime, std::vector< unsigned char > *message)
	{
		unsigned int nBytes = message->size();
		
		// only care about 3-byte messages
		if(nBytes == 3)
		{
			unsigned char status = message->at(0);
			unsigned char data1 = message->at(1);
			unsigned char data2 = message->at(2);
			
			unsigned short messageType = status & 0xF0;
			unsigned short channel = status & 0x0F;
			
#ifdef qVerbose
			std::cout << "MidiServer got: msg " << messageType << " channel " << channel << std::endl;
#endif
			
			switch(messageType)
			{
				case 0x90: // note on
				{			
					std::vector<MidiClient*>::iterator i;
					for (i = fClients.begin(); i != fClients.end(); ++i)
					{
						if (data2 > 0)
							(*i)->NoteOn(data1, data2);
                  else
                     (*i)->NoteOff(data1);
					}
               break;
				}

            case 0x80: // note off
            {
               std::vector<MidiClient*>::iterator i;
					for (i = fClients.begin(); i != fClients.end(); ++i)
					{
                  (*i)->NoteOff(data1);
					}
                  break;
            }
					
				default:
					break;
			}
		}
		

	}
	
	void AddClient(MidiClient* c, int channelIndex)
	{
		MidiClientList::iterator clientiter = std::find(fClients.begin(), fClients.end(), c);
		if (clientiter == fClients.end())
		{
			fClients.push_back(c);
		}
	}
	
	void RemoveClient(MidiClient* c, int channelIndex);
	
private:
	static MidiServer* sInstance;
	
	typedef std::vector<MidiClient*> MidiClientList;
	MidiClientList fClients;
};

MidiServer* MidiServer::sInstance = NULL;

// RtMidiDriver
// ----------------
/// \brief RtMidiDriver is a wrapper for RtMidi.  It passes callbacks along
/// to MidiServer.
class RtMidiDriver
{
public:
	RtMidiDriver() {}
	
	void OpenInputPort(unsigned int portNumber)
	{
		assert (portNumber < fMidiIn.getPortCount());
		
		std::cout << "opening MIDI port: " << fMidiIn.getPortName(portNumber) << "\n";
		fMidiIn.openPort(portNumber);
		
		// Set our callback function.  This should be done immediately after
		// opening the port to avoid having incoming messages written to the
		// queue.
		fMidiIn.setCallback(&callback);
		
		// Don't ignore sysex, timing, or active sensing messages.
		fMidiIn.ignoreTypes( false, false, false );		
	}
	
	std::vector<std::string> getInputPortNameList()
	{
		unsigned int nPorts = fMidiIn.getPortCount();
		if ( nPorts == 0 ) {
			std::cout << "No midi ports available!\n";
		}
		
		std::vector<std::string> list;
		
		for (int port = 0; port < nPorts; ++port)
		{
			list.push_back(fMidiIn.getPortName(port));
		}
		
		return list;
	}
	
	
	
private:
	static void callback(double deltatime, std::vector< unsigned char > *message, void *userData)
	{
		MidiServer::GetInstance()->MidiServerCallback(deltatime, message);
	}
	
	RtMidiIn fMidiIn;
};

#endif
