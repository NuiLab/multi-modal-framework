/// @file controller.h ///

#ifndef QUEUEOBSERVER_H
#define QUEUEOBSERVER_H

#include <assert.h>
#include <functional>
#include <initializer_list>
#include <memory>
#include <thread>

#include <observer/observable.h>
#include <queue/queue_poller.h>

//! @brief tamgef Project namespace.
namespace tamgef {
namespace queue {

template<typename T> 
class QueueObserver
{
public:
	QueueObserver() = delete;
	QueueObserver(QueueObserver<T> const&) = delete;
	QueueObserver(QueueObserver<T> &&) = delete;
	QueueObserver(
			QueueReader<T>, 
			std::initializer_list<std::shared_ptr<observer::IObserver<T>>>
		);

	virtual ~QueueObserver();

	void attachObserver(std::shared_ptr<observer::IObserver<T>>);
	void detachObserver(std::shared_ptr<observer::IObserver<T>>);

private:
	observer::Observable<T> mObservable;
	QueuePoller<T> mQueuePoller;

};// class QueueObserver

template<typename T>
QueueObserver<T>::
QueueObserver(QueueReader<T> queueReader,
		std::initializer_list<std::shared_ptr<observer::IObserver<T>>> observers) :
	mObservable(observers),
	mQueuePoller(queueReader, 
			std::bind(&observer::Observable<T>::notifyObservers, 
				std::ref(mObservable))
		)
{}

template<typename T>
void QueueObserver<T>::
attachObserver(std::shared_ptr<observer::IObserver<T>> observer_ptr)
{
	mObservable.attachObserver(observer_ptr);
}

template<typename T>
void QueueObserver<T>::
detachObserver(std::shared_ptr<observer::IObserver<T>> observer_ptr)
{
	mObservable.detachObserver(observer_ptr);
}

} // namespace queue 
} // namespace tamgef

#endif
