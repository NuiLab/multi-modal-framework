/// @file iobservable.h
/// @sa iobserver.h
/// @sa http://xinhuang.github.io/multi-threading/2015/02/11/how-a-multi-threaded-implementation-of-the-observer-pattern-can-fail/

#ifndef IOBSERVABLE_H
#define IOBSERVABLE_H

#include <memory>

template<typename T> class IObserver;

namespace tamgef {
namespace observer {

/// @brief Observable Interface.
/// @details Entrusted by @p IObserver to implement Observer Pattern.
/// @sa IObserver<T>
template<typename T> class IObservable
{
public:
	virtual ~IObservable() = default; 

	virtual void attachObserver(
			std::shared_ptr<IObserver<T>> observer) = 0;

	virtual void detachObserver(
			std::shared_ptr<IObserver<T>> observer) = 0;

	virtual void notifyObservers() = 0;

};// class IObservable

} // namespace observer 
} // namespace tamgef

#endif
