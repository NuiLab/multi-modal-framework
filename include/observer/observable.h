#ifndef OBSERVER_H
#define OBSERVER_H

#include <algorithm>
#include <memory>
#include <mutex>
#include <initializer_list>
#include <iostream>
#include <vector>

#include <observer/iobservable.h>
#include <observer/iobserver.h>

namespace tamgef {
namespace observer {

template<typename T> class IObserver;

template<typename T>
class Observable : public IObservable<T>
{
public:
	Observable() = default;
	Observable(Observable<T> const&);
	Observable(Observable<T> &&);
	Observable(std::initializer_list<std::shared_ptr<IObserver<T>>>);

	virtual void attachObserver(std::initializer_list<std::shared_ptr<IObserver<T>>>);

	virtual void attachObserver(
			std::shared_ptr<IObserver<T>> observer) override;

	virtual void detachObserver(
			std::shared_ptr<IObserver<T>> observer) override;

	virtual void notifyObservers(T const&) override;

	void swap(Observable<T> &);

private:
	std::mutex mObserversMutex;
	std::vector<std::weak_ptr<IObserver<T>>> mObservers;
};

template<typename T>
Observable<T>::Observable(Observable<T> const& other) :
	mObservers(other.mObservers)
{}

template<typename T>
Observable<T>::Observable(Observable<T> && other) :
	mObservers(std::move(other.mObservers))
{}

template<typename T>
Observable<T>::Observable(std::initializer_list<std::shared_ptr<IObserver<T>>> observers) :
	mObservers(observers)
{}

template<typename T>
void Observable<T>::attachObserver(
		std::initializer_list<std::shared_ptr<IObserver<T>>> observers)
{
	for (auto& observer : observers)
		attachObserver(observer);
}

template<typename T>
void Observable<T>::attachObserver(
		std::shared_ptr<IObserver<T>> observer)
{
	if (!observer)
		throw std::invalid_argument("Empty reference to observer");
	{
		std::lock_guard<std::mutex> lock(mObserversMutex);
		mObservers.emplace_back(observer);
	}
}

template<typename T>
void Observable<T>::detachObserver(
		std::shared_ptr<IObserver<T>> observer)
{
	if (!observer)
		throw std::invalid_argument("Empty reference to observer");

	{	
		std::lock_guard<std::mutex> lock(mObserversMutex);
		auto newEnd = std::remove_if(
				mObservers.begin(),
				mObservers.end(),
				[&](std::weak_ptr<IObserver<T>> const& o)
				{
					return o.lock() == observer;
				});

		if (newEnd != mObservers.end())
			mObservers.erase(newEnd, mObservers.end());
	}
}

template<typename T>
void Observable<T>::notifyObservers(T const& message)
{
	std::vector<std::weak_ptr<IObserver<T>>> observers;

	{
		std::lock_guard<std::mutex> lock(mObserversMutex);
		observers = mObservers;
	}

	for(auto& o : observers) {
		if(auto observer = o.lock())
			observer->update(message);
	}
}

} // namespace observer 
} // namespace tamgef

#endif
