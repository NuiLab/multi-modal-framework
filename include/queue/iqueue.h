//! @file iqueue.h
#ifndef IQUEUE_H
#define IQUEUE_H

#include <memory>

namespace tamgef {
namespace queue {

/// @brief Queueable Interface.
template <typename T> 
class IQueue
{
public:
	virtual ~IQueue() = default;

	virtual T dequeue() = 0;
	virtual bool empty() const = 0;
	virtual void enqueue(T element) = 0;
	virtual size_t size() const = 0;

};

} // namespace queue 
} // namespace tamgef
#endif
