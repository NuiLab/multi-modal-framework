#ifndef EVENT_H
#define EVENT_H

#include <algorithm>
#include <atomic>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <typeindex>

namespace tamgef {
namespace device {

template<typename T>
class Event
{
public:
	Event();
	Event(Event<T> const&);
	Event(Event<T> &&);
	Event<T> & operator=(Event<T>);
	virtual ~Event() = default;

	// throws std::invalid_argument if typeID has not been registered
	Event(T const&);

	// adds new event type to static list of types
	// returns false if type is duplicate, otherwise true
	static void registerType(T const&);

	static void registerType(std::initializer_list<T> const&);

	// returns copy of static types list
	static std::vector<T> registeredTypes();

	// returns true if type id is registered, false otherwise
	static bool registered(T const&);

	// sets event flag to false, indicating event has been processed
	void lower();

	// sets event flag to true, indicating event is to be processed
	void raise();

	// returns type id of event
	T type() const;

	void swap(Event<T> &);

private:
	static std::vector<T> sRegisteredTypes;

	bool mFlag;
	T mType;
};

template<typename T> std::vector<T> Event<T>::sRegisteredTypes;

template<typename T>
Event<T>::Event() :
	mFlag(false)
{}

template<typename T>
Event<T>::Event(T const& type) :
	mFlag(true),
	mType(type) 
{
	if (!registered(type))
		throw std::invalid_argument("Unregistered type");
}

template<typename T>
Event<T>::Event(Event<T> const& other) :
	Event(other.mType)
{}

template<typename T>
Event<T>::Event(Event<T> && other) :
	Event(std::move(other.mType))
{}

template<typename T>
Event<T> & Event<T>::operator=(Event<T> other)
{
	swap(other);
	return *this;
}

template<typename T>
void Event<T>::registerType(T const& type)
{
	if (!registered(type))
		sRegisteredTypes.push_back(type);
}

template<typename T>
void Event<T>::registerType(std::initializer_list<T> const& types)
{
	for (auto & type : types) 
		registerType(type);
}

template<typename T>
std::vector<T> Event<T>::registeredTypes()
{
	return sRegisteredTypes;
}

template<typename T>
bool Event<T>::registered(T const& type)
{
	auto iType = std::find(sRegisteredTypes.begin(),
			sRegisteredTypes.end(), type);

	if (iType == sRegisteredTypes.end())
		return false;

	return true;
}

template<typename T>
void Event<T>::raise()
{
	mFlag = true;
}

template<typename T>
void Event<T>::lower()
{
	mFlag = false;
}

template<typename T>
T Event<T>::type() const
{
	return mType;
}

template<typename T>
void Event<T>::swap(Event<T> & other)
{
	std::swap(mFlag, other.mFlag);
	std::swap(mType, other.mType);
}


} // namespace device
} // namespace tamgef 

#endif
