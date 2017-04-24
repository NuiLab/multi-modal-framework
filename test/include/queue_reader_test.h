#include <memory>

#include <gtest/gtest.h>
#include <queue/queue.h>
#include <queue/queue_reader.h>

class QueueReaderTest : public ::testing::Test
{
protected:
	std::shared_ptr<tamgef::queue::QueueReader<int>> queue_reader_connected_ptr;
	std::shared_ptr<tamgef::queue::QueueReader<int>> queue_reader_empty_ptr;
	std::shared_ptr<tamgef::queue::Queue<int>> queue_ptr;

	virtual void SetUp()
	{
		queue_ptr = std::make_shared<tamgef::queue::Queue<int>>();
		queue_reader_connected_ptr = std::make_shared<tamgef::queue::QueueReader<int>>(queue_ptr);
		queue_reader_empty_ptr = std::make_shared<tamgef::queue::QueueReader<int>>();
	}
};
