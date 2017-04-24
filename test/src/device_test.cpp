#include "device_test.h"

#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <gtest/gtest.h>
#include <queue/queue_reader.h>

TEST_F(DeviceTest, constuctor)
{	
	// fixture class test
	// initial state decided by default constructor
	EXPECT_FALSE(circuit_device_ptr->state().is_on);
	EXPECT_TRUE(circuit_device_ptr->state().is_intact);
}

TEST_F(DeviceTest, connect)
{
	ASSERT_NO_THROW(circuit_device_ptr->connect(*current_queue_reader_ptr));
	EXPECT_FALSE(current_queue_reader_ptr->expired());

	ASSERT_NO_THROW(circuit_device_ptr->connect(*event_queue_reader_ptr));
	EXPECT_FALSE(event_queue_reader_ptr->expired());
}

TEST_F(DeviceTest, read_connection)
{
	// no connections were made	
	EXPECT_THROW(circuit_device_ptr->read(), std::runtime_error);

	ASSERT_NO_THROW(circuit_device_ptr->connect(tamgef::queue::QueueReader<circuit::volts>(voltage_queue_ptr)));

	// connection empty
	EXPECT_FALSE(circuit_device_ptr->read());
	voltage_queue_ptr->enqueue(circuit::volts(5));
	EXPECT_TRUE(circuit_device_ptr->read());
}

TEST_F(DeviceTest, read_input)
{
	// setup readers to test writes to output and event queue
	ASSERT_NO_THROW(circuit_device_ptr->connect(*current_queue_reader_ptr));
	ASSERT_NO_THROW(circuit_device_ptr->connect(*event_queue_reader_ptr));

	// not in input domain, queues should be empty
	EXPECT_FALSE(circuit_device_ptr->read(circuit::volts(-1)));
	EXPECT_TRUE(current_queue_reader_ptr->empty());

	// in domain, should call resolution function and update output
	EXPECT_TRUE(circuit_device_ptr->read(circuit::volts(5)));
	EXPECT_FALSE(current_queue_reader_ptr->empty());

	// updates state  
	EXPECT_TRUE(circuit_device_ptr->state().is_on); 

	auto dummy_device_ptr = std::make_shared<circuit_device>();

	ASSERT_TRUE(dummy_device_ptr);	
	// no device functions were defined
	EXPECT_THROW(dummy_device_ptr->read(circuit::volts(1)), 
			std::bad_function_call);
}

