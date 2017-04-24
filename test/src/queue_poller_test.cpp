#include <atomic>
#include <chrono>

#include <gtest/gtest.h>
#include <queue/queue.h>
#include <queue/queue_reader.h>
#include <queue/queue_poller.h>

TEST(QueuePollerTest, constructor)
{
	std::shared_ptr<tamgef::queue::Queue<int>> queue_ptr;

	EXPECT_THROW(
		{
			tamgef::queue::QueuePoller<int> queue_poller(
					(tamgef::queue::QueueReader<int>(queue_ptr)),
					(std::function<void(int)>()));
		}, 
		std::invalid_argument);

	EXPECT_THROW(
		{
			tamgef::queue::QueuePoller<int> queue_poller(
					(tamgef::queue::QueueReader<int>()), 
					([](int){}));
		}, 
		std::invalid_argument);

	queue_ptr = std::make_shared<tamgef::queue::Queue<int>>();

	EXPECT_NO_THROW(
		{
			tamgef::queue::QueuePoller<int> queue_poller(
					(tamgef::queue::QueueReader<int>(queue_ptr)),
					([](int){}));

			EXPECT_TRUE(queue_poller.polling());
		});
}

TEST(QueuePollerTest, exception)
{
	auto timeout(std::chrono::milliseconds(10));
	auto queue_ptr = std::make_shared<tamgef::queue::Queue<int>>();
	tamgef::queue::QueueReader<int> queue_reader(queue_ptr);

	tamgef::queue::QueuePoller<int> queue_poller(
			queue_reader,
			[](int) {});

	// should cause polling thread to throw std::runtime_error
	queue_ptr.reset();

	// I know, I know
	std::this_thread::sleep_for(timeout);

	// should stop polling?
	EXPECT_FALSE(queue_poller.polling());
	EXPECT_TRUE(queue_poller.error());
}

TEST(QueuePollerTest, poll)
{
	auto sent(10);
	auto timeout(std::chrono::milliseconds(10));
	std::atomic<int> recieved(0);

	auto queue_ptr = std::make_shared<tamgef::queue::Queue<int>>();
	queue_ptr->enqueue(sent);
	
	tamgef::queue::QueuePoller<int> queue_poller(
			tamgef::queue::QueueReader<int>(queue_ptr),
			[&recieved](int message) 
			{ 
				recieved.store(message); 
			});

	// I know, I know 
	std::this_thread::sleep_for(timeout);

	EXPECT_EQ(recieved.load(), sent);
}

