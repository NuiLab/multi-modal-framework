#include "queue_reader_test.h"

#include <memory>
#include <queue>

#include <gtest/gtest.h>
#include <queue/queue_reader.h>
#include <queue/queue.h>

TEST_F(QueueReaderTest, connect)
{
	std::shared_ptr<tamgef::queue::Queue<int>> empty_queue_ptr;
	EXPECT_THROW(queue_reader_empty_ptr->connect(empty_queue_ptr),
			std::invalid_argument);
}	

TEST_F(QueueReaderTest, dequeue)
{
	EXPECT_THROW(queue_reader_empty_ptr->dequeue(), 
			std::runtime_error); 	
	EXPECT_NO_THROW(queue_reader_connected_ptr->dequeue());
}

TEST_F(QueueReaderTest, disconnect)
{
	ASSERT_FALSE(queue_reader_connected_ptr->expired());
	
	queue_reader_connected_ptr->disconnect();

	EXPECT_TRUE(queue_reader_connected_ptr->expired());
}

TEST_F(QueueReaderTest, empty)
{
	EXPECT_THROW(queue_reader_empty_ptr->empty(), std::runtime_error);
	ASSERT_NO_THROW(queue_reader_connected_ptr->empty());
	EXPECT_TRUE(queue_reader_connected_ptr->empty());
	EXPECT_FALSE(queue_reader_connected_ptr->empty());
}

TEST_F(QueueReaderTest, expired)
{
	EXPECT_TRUE(queue_reader_empty_ptr->expired());
	EXPECT_FALSE(queue_reader_connected_ptr->expired());

	queue_reader_connected_ptr->disconnect();

	EXPECT_TRUE(queue_reader_connected_ptr->expired());
}
