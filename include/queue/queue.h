#ifndef QUEUE_H
#define QUEUE_H

#include <internal/concurrentqueue/concurrentqueue.h>
#include <queue/iqueue.h>

namespace tamgef
{

namespace queue 
{

template<typename T>
class Queue : public IQueue<T>
{
public:
	T dequeue() override;
	bool empty() const override;
	void enqueue(T element) override;
	size_t size() const override;

private:
	moodycamel::ConcurrentQueue<T> mQueue;
};

template<typename T>
T Queue<T>::dequeue()
{
	T element = T();
	mQueue.try_dequeue(element);

	return element;
}

template<typename T>
bool Queue<T>::empty() const 
{
	return size() == 0;
}

template<typename T>
void Queue<T>::enqueue(T element)
{
	mQueue.enqueue(element);
}

template<typename T>
size_t Queue<T>::size() const
{
	return mQueue.size_approx();
}

} // namespace queue
} // namespace tamgef

#endif
