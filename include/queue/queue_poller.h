#ifndef QUEUE_POLLER_H
#define QUEUE_POLLER_H

#include <atomic>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>

#include <queue/queue_reader.h>

namespace tamgef {
namespace queue {

template<typename T>
class QueuePoller
{
public:
	QueuePoller(QueuePoller<T> const&);
	QueuePoller(QueuePoller<T> &&);
	QueuePoller(QueueReader<T> const&, std::function<void(T)>);
	virtual ~QueuePoller();

	std::exception_ptr error();
	bool polling() const;
	void stop();

private:
	static QueueReader<T> const& 
		checkQueueReader(QueueReader<T> const&);
	static std::function<void(T)> 
		checkHandler(std::function<void(T)>);

	QueueReader<T> mQueueReader;

	std::atomic<bool> mPolling;
	std::function<void(T)> mHandler;
	std::mutex mExceptionMutex;
	std::exception_ptr mException;
	std::thread mThread;

	void poll();
};

template<typename T>
QueueReader<T> const& QueuePoller<T>::
checkQueueReader(QueueReader<T> const& queueReader)
{
	if (queueReader.expired())
		throw std::invalid_argument("Queue reference expired");

	return queueReader;
}

template<typename T>
std::function<void(T)> QueuePoller<T>::
checkHandler(std::function<void(T)> handler)
{
	if (!handler)
		throw std::invalid_argument("Empty function handler");

	return handler;
}

template<typename T>
QueuePoller<T>::QueuePoller(
		QueueReader<T> const& queueReader, 
		std::function<void(T)> handler) :
	mPolling(true),
	mQueueReader(checkQueueReader(queueReader)),
	mHandler(checkHandler(handler)),
	mThread(&QueuePoller<T>::poll, this)
{}

template<typename T>
QueuePoller<T>::QueuePoller(QueuePoller<T> const& other) :
	QueuePoller(other.mQueueReader, other.mHandler)
{}

template<typename T>
QueuePoller<T>::QueuePoller(QueuePoller<T> && other) :
	mQueueReader(std::move(other.mQueueReader)),
	mPolling(other.mPolling.load()),
	mHandler(std::move(other.mHandler)),
	mThread(std::move(other.mThread)),
	mException(std::move(other.mException))
{}

template<typename T>
QueuePoller<T>::~QueuePoller()
{
	mPolling.store(false);
	mThread.join();
}

template<typename T>
bool QueuePoller<T>::polling() const
{
	return mPolling.load();
}

template<typename T>
std::exception_ptr QueuePoller<T>::error()
{
	std::lock_guard<std::mutex> lock(mExceptionMutex);
	return mException;
}

template<typename T>
void QueuePoller<T>::poll()
{
	while (polling())
	{
		try 
		{
			if (!mQueueReader.empty()) 
			{
				T message = mQueueReader.dequeue();
				mHandler(message);
			}
			else
				std::this_thread::yield();
		}
		catch (...)
		{
			{
				std::lock_guard<std::mutex> lock(mExceptionMutex);
				mException = std::current_exception();
			}

			mPolling.store(false);
		}
	}
}

} // namespace queue
} // namespace tamgef

#endif
