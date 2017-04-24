#ifndef QUEUE_READER_H
#define QUEUE_READER_H

#include <memory>

#include <queue/iqueue.h>

namespace tamgef {
namespace queue {

template<typename T>
class QueueReader
{
public:
	QueueReader() = default;
	QueueReader(QueueReader<T> const&);
	QueueReader(QueueReader<T> &&);
	QueueReader<T> & operator=(QueueReader<T>);

	QueueReader(std::shared_ptr<IQueue<T>>);
	virtual ~QueueReader() = default;

	void connect(std::shared_ptr<IQueue<T>>);
	T dequeue();
	void disconnect();
	bool empty() const;
	bool expired() const;
	size_t size() const;

	void swap(QueueReader<T> &);

private:
	std::weak_ptr<IQueue<T>> pQueue;

};// class QueueReader

template<typename T>
QueueReader<T>::QueueReader(QueueReader<T> const& other) :
	pQueue(other.pQueue)
{}

template<typename T>
QueueReader<T>::QueueReader(QueueReader<T> && other)
{
	swap(other);
}

template<typename T>
QueueReader<T> & QueueReader<T>::operator=(QueueReader<T> other)
{
	swap(other);
	return *this;
}

template<typename T>
QueueReader<T>::QueueReader(std::shared_ptr<IQueue<T>> queue) :
	pQueue(queue)
{}

template<typename T>
void QueueReader<T>::connect(std::shared_ptr<IQueue<T>> queue)
{
	if (!queue)
		throw std::invalid_argument("Queue reference empty");

	pQueue = queue;
}

template<typename T>
T QueueReader<T>::dequeue()
{
	if (expired())
		throw std::runtime_error("Queue reference expired");

	return pQueue.lock()->dequeue();
}

template<typename T>
void QueueReader<T>::disconnect()
{
	pQueue.reset();
}

template<typename T>
bool QueueReader<T>::empty() const
{
	if (expired())
		throw std::runtime_error("Queue reference expired");

	return pQueue.lock()->empty();
}

template<typename T>
bool QueueReader<T>::expired() const
{
	return pQueue.expired();
}

template<typename T>
size_t QueueReader<T>::size() const
{
	if (expired())
		throw std::runtime_error("Queue reference expired");

	return pQueue.lock()->size();
}

template<typename T>
void QueueReader<T>::swap(QueueReader<T> & other)
{
	std::swap(pQueue, other.pQueue);
}

} // namespace queue
} // namespace tamgef

#endif
