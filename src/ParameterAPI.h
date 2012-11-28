#ifndef h_ParameterAPI
#define h_ParameterAPI

#include <string>
#include <vector>

class Broadcaster;
class Observable;
class Parameter;

/// \brief Interface for classes that register for callbacks from Broadcasters
class Listener
{
public:
	virtual void Listen(Broadcaster const* b) = 0;
};

/// \brief Interface for classes that should listen for callbacks from Parameters
///
/// Note that all Parameters are Broadcasters, so any ParameterListener can register
/// for changes to a parameter's value (unless the parameter is set not to broadcast)
///
/// A ParameterListener's behavior is determined by overriding Listen (required)
class ParameterListener : public Listener
{
public:
	virtual void Listen(Broadcaster const* b)
	{
		Parameter const* p = reinterpret_cast<Parameter const*> (b);
		this->Listen(p);
	}
	
	virtual void Listen(Parameter const* p) = 0;
};

/// \brief Interface for classes that have information to broadcast to Listeners.
///
/// Listeners will register for callbacks
class Broadcaster // : public PseudoSingleton (TODO)
{
public:
	void AddListener(Listener* l);
	void RemoveListener(Listener* l);
protected:
	virtual void Broadcast() = 0;
	std::vector<Listener*> _listeners;
};

/// \brief Interface for classes that register for callbacks from Observables
class Observer
{
public:
	virtual void Observe(Observable* o) = 0;
};

/// \brief Interface for classes that register for callbacks from Parameters
///
/// Note that all Parameters are Observables
class ParameterObserver : public Observer
{
public:
	virtual void Observe(Observable* o)
	{
		Parameter* p = reinterpret_cast<Parameter*> (o);
		this->Observe(p);	
	}
	
	virtual void Observe(Parameter* p) = 0;
	
};

/// \brief Interface for classes that should be 'observed'.
///
/// Observables are directly linked to Observers.  This differs from the Broadcaster/Listener
/// scheme in that there is no singleton which manages the connection between listeners
/// and observers. 
class Observable
{
public:
	void AddObserver(Observer* o);
	void RemoveObserver(Observer* o);
protected:
	virtual void NotifyObservers() = 0;
	std::vector<Observer*> _observers;
};

/// \brief A Parameter is an entity which stores a value, numerical ID, textual name, 
/// and supplies a formatted string to present its state.
///
/// Parameters have a flag to specify whether or not its value should be broadcast to
/// any interested listeners.
///
/// The "isPublished" flag is meant for exposing the parameter within a plug-in API
/// such as VST or Audio Units such that the host program can collect the necessary
/// information.  In this case, the implementation of the plug-in's parameter management
/// can be handled entirely through interaction with the Parameter and ParameterManager classes.
class Parameter : public Broadcaster
                , public Observable
{
public:
	Parameter(const int id, const std::string& name, const float initialValue = 0.f);
	
	const float Value() const;
	const std::string Name() const;
	const std::string Text() const;
   const int Id() const;
	
	void SetValue(const float value);

protected:
	// override Observable
	virtual void NotifyObservers();
	
	// override Broadcast
	virtual void Broadcast();
private:
	int _id;
	float _value;
	std::string _name;
	
	bool _isPublished;
	bool _isBroadcast;
};

/// \brief ParameterManager is a singleton that manages all of a program's parameters.
///
/// All Parameters should be uniquely ID'ed and submitted to the ParameterManager
/// with AddParameter.  Programs can 
///
/// Note that the ParameterManager IS responsible for deleting Parameter objects,
/// so don't write your code to assume otherwise.
class ParameterManager // : public PsuedoSingleton (TODO)
{
public:

   ~ParameterManager();

   static ParameterManager* GetInstance()
	{
		if (!sInstance)
		{
			sInstance = new ParameterManager;
		}
		return sInstance;
	}

	void AddParameter(Parameter* p);
	Parameter* GetParameter(int id);
private:
   static ParameterManager* sInstance;

	std::vector<Parameter*> _parameters;
};

#endif
