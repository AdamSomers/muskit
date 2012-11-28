#include "ParameterAPI.h"

#include <iostream>

#define qVerbose 1

ParameterManager* ParameterManager::sInstance = NULL;

//------ Broadcaster ------//

void Broadcaster::AddListener(Listener* l)
{
	std::vector<Listener*>::iterator i = std::find(_listeners.begin(), _listeners.end(), l);
	if (i == _listeners.end())
	{
		_listeners.push_back(l);
	}
}

void Broadcaster::RemoveListener(Listener* l)
{
	std::vector<Listener*>::iterator i = std::find(_listeners.begin(), _listeners.end(), l);	if (i != _listeners.end())
	{
		_listeners.erase(i);
	}
}

//------ Observable ------//

void Observable::AddObserver(Observer* o)
{
	std::vector<Observer*>::iterator i = std::find(_observers.begin(), _observers.end(), o);
	if (i == _observers.end())
	{
		_observers.push_back(o);
	}
}

void Observable::RemoveObserver(Observer* o)
{
	std::vector<Observer*>::iterator i = std::find(_observers.begin(), _observers.end(), o);
	if (i != _observers.end())
	{
		_observers.erase(i);
	}
}

//------ Parameter ------//

Parameter::Parameter(const int id, const std::string& name, const float initialValue /*=0.f*/)
: _id(id)
, _name(name)
, _value(initialValue)
, _isBroadcast(true)
, _isPublished(true)
{
}

const float Parameter::Value() const
{
	return _value;
}

const std::string Parameter::Name() const
{
	return _name;
}

const int Parameter::Id() const
{
   return _id;
}

const std::string Parameter::Text() const
{
	// TODO: use modular formatter!!
	std::string s = _name;
	s += ": ";
	s+= _value;
	return s;
}

void Parameter::SetValue(const float value)
{
	_value = value;
	this->NotifyObservers();
	if (this->_isBroadcast)
	{
		this->Broadcast();
	}
   
#ifdef qVerbose 
   std::cout << _name << ": " << _value << std::endl;
#endif
}

void Parameter::NotifyObservers()
{
	// TODO: post a message for asynchronous consumption
	// for now, directly invoke obververs
	for (std::vector<Observer*>::iterator i = _observers.begin(); i != _observers.end(); ++i)
	{
		(*i)->Observe(this);
	}
}

void Parameter::Broadcast()
{
	// TODO: post a message for asynchronous consumption
	// for now, directly invoke listeners
	for (std::vector<Listener*>::iterator i = _listeners.begin(); i != _listeners.end(); ++i)
	{
		(*i)->Listen(this);
	}
}

//------ ParameterManager ------//

ParameterManager::~ParameterManager()
{
   std::vector<Parameter*>::iterator i;
	for (i = _parameters.begin(); i != _parameters.end(); ++i)
	{
      delete(*i);
   }
}

void ParameterManager::AddParameter(Parameter* p)
{
   std::vector<Parameter*>::iterator i = std::find(_parameters.begin(), _parameters.end(), p);
	if (i == _parameters.end())
	{
		_parameters.push_back(p);
	}
}

Parameter* ParameterManager::GetParameter(int id)
{
   Parameter* result = NULL;

   std::vector<Parameter*>::iterator i;
	for (i = _parameters.begin(); i != _parameters.end(); ++i)
	{
      if ((*i)->Id() == id) {
         result = (*i);
         break;
      }
	}

   return result;
}
