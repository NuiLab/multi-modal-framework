#ifndef GENERIC_DEVICE_H
#define GENERIC_DEVICE_H

#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <vector>

#include <device/event.h>
#include <queue/iqueue.h>
#include <queue/queue.h>
#include <queue/queue_reader.h>

namespace tamgef {
namespace device {

using namespace tamgef::queue;

template<
	typename InputT, 
	typename OutputT, 
	typename StateT, 
	typename EventT>
class GenericDevice
{
public:
	typedef std::function<bool(InputT)> InputDomain;
	typedef std::function<bool(OutputT)> OutputDomain;
	typedef std::function<OutputT(InputT)> ResolutionFunction;
	typedef std::function<StateT(StateT, InputT, OutputT)> StateFunction;
	typedef std::function<Event<EventT>(StateT)> EventFunction;
	typedef std::vector<EventFunction> EventList;

	GenericDevice();
	GenericDevice(GenericDevice<
					InputT, 
					OutputT, 
					StateT, 
					EventT> const&);

	GenericDevice(GenericDevice<
					InputT, 
					OutputT, 
					StateT, 
					EventT> &&);

	GenericDevice<InputT, OutputT, StateT, EventT> & 
	operator=(GenericDevice<InputT, OutputT, StateT, EventT>);

	GenericDevice(
			InputDomain,
			OutputDomain,
			ResolutionFunction,
			StateFunction,
			std::initializer_list<EventFunction>);

	virtual ~GenericDevice() = default;
	GenericDevice<InputT, OutputT, StateT, EventT>
	combine(GenericDevice<InputT, OutputT, StateT, EventT> const&);

	template<typename OtherInputT, typename OtherStateT, typename OtherEventT>
	void connect(GenericDevice<OtherInputT, InputT, OtherStateT, OtherEventT> const&);
	void connect(QueueReader<InputT>);
	void connect(QueueReader<OutputT> &);
	void connect(QueueReader<Event<EventT>> &);
	void disconnect();

	bool read();
	bool read(InputT);
	StateT state();

	void swap(GenericDevice<InputT, OutputT, StateT, EventT> &);

private:
	std::shared_ptr<Queue<OutputT>> pOutputQueue;
	std::shared_ptr<Queue<Event<EventT>>> pEventQueue;

	InputDomain mInputDomain;
	OutputDomain mOutputDomain;
	ResolutionFunction mResolutionFunction;
	StateFunction mStateFunction;
	EventList mEventList;
	QueueReader<InputT> mInputConnection;
	StateT mCurrentState;

};// class GenericDevice

template<
	typename InputT, 
	typename OutputT, 
	typename StateT, 
	typename EventT>
GenericDevice<InputT, OutputT, StateT, EventT>::
GenericDevice() :
	pOutputQueue(std::make_shared<Queue<OutputT>>()),
	pEventQueue(std::make_shared<Queue<Event<EventT>>>())
{}

template<
	typename InputT, 
	typename OutputT, 
	typename StateT, 
	typename EventT>
GenericDevice<InputT, OutputT, StateT, EventT>::
GenericDevice(
		InputDomain inputDomain,
		OutputDomain outputDomain,
		ResolutionFunction resolutionFunction,
		StateFunction stateFunction,
		std::initializer_list<EventFunction> eventList) :
	mInputDomain(inputDomain),
	mOutputDomain(outputDomain),
	mResolutionFunction(resolutionFunction),
	mStateFunction(stateFunction),
	mEventList(eventList),
	pOutputQueue(std::make_shared<Queue<OutputT>>()),
	pEventQueue(std::make_shared<Queue<Event<EventT>>>())
{}

template<
	typename InputT, 
	typename OutputT, 
	typename StateT, 
	typename EventT>
GenericDevice<InputT, OutputT, StateT, EventT>::
GenericDevice(GenericDevice<InputT, OutputT, StateT, EventT> const& other) :
	GenericDevice(
			other.mInputDomain,
			other.mOutputDomain,
			other.mResolutionFunction,
			other.mStateFunction, 
			other.mEventList)
{}

template<
	typename InputT, 
	typename OutputT, 
	typename StateT, 
	typename EventT>
GenericDevice<InputT, OutputT, StateT, EventT>::
GenericDevice(GenericDevice<InputT, OutputT, StateT, EventT> && other) :
	GenericDevice()
{
	swap(other);
}
	
template<
	typename InputT, 
	typename OutputT, 
	typename StateT, 
	typename EventT>
GenericDevice<InputT, OutputT, StateT, EventT> & 
GenericDevice<InputT, OutputT, StateT, EventT>::
operator=(GenericDevice<InputT, OutputT, StateT, EventT> other)
{
	swap(other);
	return *this;
}

template<
	typename InputT,
	typename OutputT,
	typename StateT,
	typename EventT>
GenericDevice<InputT, OutputT, StateT, EventT> 
GenericDevice<InputT, OutputT, StateT, EventT>::
combine(GenericDevice<InputT, OutputT, StateT, EventT> const& other)
{
	auto inputDomain = mInputDomain;
	auto outputDomain = mOutputDomain;
	auto stateFunction = mStateFunction;
	auto eventList = mEventList;

	InputDomain compositeInputDomain(
			[](InputT input) -> bool
			{});

	OutputDomain compositeOutputDomain(
			[](OutputT output) -> bool
			{});

	ResolutionFunction compositeResolutionFunction(
			[](InputT input) -> OutputT
			{});

	StateFunction compositeStateFunction(
			[](StateT, InputT, OutputT) -> StateT
			{});

	EventList compositeEventList;
	compositeEventList.reserve(
			mEventList.size() + other.mEventList.size());

	compositeEventList.insert(
			compositeEventList.end(), 
			mEventList.begin(), 
			mEventList.end());

	compositeEventList.insert(
			compositeEventList.end(), 
			other.mEventList.begin(), 
			other.mEventList.end());

	return GenericDevice<InputT, OutputT, StateT, EventT>(
			compositeInputDomain,
			compositeOutputDomain,
			compositeResolutionFunction,
			compositeStateFunction,
			compositeEventList);
}

template<
	typename InputT,
	typename OutputT,
	typename StateT,
	typename EventT>
template<
	typename OtherInputT, 
	typename OtherStateT,
	typename OtherEventT>
void GenericDevice<InputT, OutputT, StateT, EventT>::
connect(GenericDevice<OtherInputT, InputT, OtherStateT, OtherEventT> const& other)
{
	mInputConnection = QueueReader<InputT>(other.pOutputQueue);
}

template<
	typename InputT,
	typename OutputT,
	typename StateT,
	typename EventT>
void GenericDevice<InputT, OutputT, StateT, EventT>::
connect(QueueReader<InputT> inputConnection)
{
	if (inputConnection.expired())
		throw std::invalid_argument("Queue reference expired");

	mInputConnection = std::move(inputConnection);
}

template<
	typename InputT,
 	typename OutputT,
 	typename StateT,
 	typename EventT>
void GenericDevice<InputT, OutputT, StateT, EventT>::
connect(QueueReader<OutputT> & outputReader) 
{
	outputReader.connect(pOutputQueue);
}

template<
	typename InputT,
 	typename OutputT,
 	typename StateT,
 	typename EventT>
void GenericDevice<InputT, OutputT, StateT, EventT>::
connect(QueueReader<Event<EventT>> & eventReader)
{
	eventReader.connect(pEventQueue);
}

template<
	typename InputT,
	typename OutputT,
	typename StateT,
	typename EventT>
void GenericDevice<InputT, OutputT, StateT, EventT>::
disconnect()
{
	mInputConnection.disconnect();
}

template<
	typename InputT, 
	typename OutputT, 
	typename StateT, 
	typename EventT>
bool GenericDevice<InputT, OutputT, StateT, EventT>::
read(InputT input)
{
	if (!mInputDomain(input))
		return false;
	
	auto output(mResolutionFunction(input));

	if (mOutputDomain(output))
		pOutputQueue->enqueue(output);

	auto state(mStateFunction(mCurrentState, input, output));

	for (auto & event : mEventList)
		pEventQueue->enqueue(event(state)); 

	mCurrentState = state;
	
	return true;
}

template<
	typename InputT,
	typename OutputT,
	typename StateT,
	typename EventT>
bool GenericDevice<InputT, OutputT, StateT, EventT>::
read()
{
	if (mInputConnection.expired())
		throw std::runtime_error("No input connected");

	if (mInputConnection.empty())
		return false;

	return read(mInputConnection.dequeue());
}

template<
	typename InputT, 
	typename OutputT, 
	typename StateT, 
	typename EventT>
StateT GenericDevice<InputT, OutputT, StateT, EventT>::
state() 
{
	return mCurrentState;
}

template<
	typename InputT, 
	typename OutputT, 
	typename StateT, 
	typename EventT>
void GenericDevice<InputT, OutputT, StateT, EventT>::
swap(GenericDevice<InputT, OutputT, StateT, EventT> & other)
{
	std::swap(mInputDomain, other.mInputDomain);
	std::swap(mOutputDomain, other.mOutputDomain);
	std::swap(mResolutionFunction, other.mResolutionFunction);
	std::swap(mStateFunction, mStateFunction);
	std::swap(mEventList, mEventList);
}

} // namespace device
} // namespace tamgef 

#endif
