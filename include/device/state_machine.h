#ifndef STATES_H
#define STATES_H

#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <internal/concurrentqueue/concurrentqueue.h>

namespace tamgef {
namespace device {

template <typename T>
class StateMachine
{
public:
	StateMachine();
	StateMachine(StateMachine const&) = delete;
	StateMachine(StateMachine&&);
	StateMachine& operator=(StateMachine const&);
	StateMachine& operator=(StateMachine&&);
	~StateMachine();

	// appends the state name and event function to state map
	// event function is added to event thread pool on transistion
	// returns true if insert is successful, false otherwise
	bool addState(std::string const& stateName,
		 	std::function<T> const& stateEvent);

	// appends state name and empty function to state map
	// returns true if insert is successful, false otherwise
	bool addState(std::string const& stateName);

	// appends transtition between states to transition map
	// transition between states is made when predicate returns true
	// returns true if insert is successful, false otherwise
	bool addTransition(std::string const& fromStateName,
		 	std::string const& toStateName,
		 	std::function<bool()> const& transitionPredicate);
	
	// appends transtition between states to transition map
	// transition is always made between states
	// returns true if insert is successful, false otherwise
	bool addTransition(std::string const& fromStateName,
		 	std::string const& toStateName);

	// designates existing state as initial state,
	// does not change present state.
	// returns true if designation was made, false if state not found
	bool setInitialState(std::string const& stateName);

	// designates existing state as final state, 
	// does not change present state.
	// returns true if designation was made, false if state not found
	bool setFinalState(std::string const& stateName);

	// returns name of present state
	std::string getPresentState() const;

	// starts state machine 
	// initializes handler thread and event thread pool
	// pre: initial state has been set
	bool start();

	// start state machine, specifying initial state, see start()
	bool start(std::string const& initialStateName);

	// stops state machine, joins handler thread and even thread pool
	bool stop();

//private:
	std::map<std::string, std::function<T>> mStateMap;
	std::map<std::string, std::map<std::string, 
		std::function<bool()>>> mTransitionMap;

	std::string mInitialStateName;
	std::string mPresentStateName;
	std::string mFinalStateName;

	std::atomic<bool> mIsActive;
	std::shared_ptr<std::thread> pStateHandlerThread;
	std::vector<std::thread> mEventThreadPool;
	moodycamel::ConcurrentQueue<std::function<T>> mEventQueue;
	
	// state machine logic launched on state handler thread
	void stateHandler();
};

template <typename T>
StateMachine<T>::StateMachine() :
	mInitialStateName("__null__"),
	mPresentStateName("__null__"),
	mFinalStateName("__null__"),
	mIsActive(false),
	pStateHandlerThread(nullptr)
{}

template <typename T>
StateMachine<T>::~StateMachine()
{
	stop();
}

template <typename T>
bool StateMachine<T>::addState(std::string const& stateName)
{
	return addState(stateName, [](){});
}

template <typename T>
bool StateMachine<T>::addState(std::string const& stateName, 
		std::function<T> const& stateEvent)
{
	// function target not valid
	if (!stateEvent)
		return false;

	return mStateMap.emplace(std::make_pair(stateName, stateEvent)).second;
}

template <typename T>
bool StateMachine<T>::addTransition(std::string const& fromState, 
		std::string const& toState,
	 	std::function<bool()> const& transitionPredicate)
{
	// can only add transitions to existing events
	if (mStateMap.find(fromState) == mStateMap.end() || 
			mStateMap.find(toState) == mStateMap.end())
		return false;

	if (!transitionPredicate)
		return false;

	mTransitionMap[fromState][toState] = transitionPredicate;

	return true;
}

template <typename T>
bool StateMachine<T>::addTransition(std::string const& fromState, 
		std::string const& toState)
{
	// transistion predicate always returns true; always moves to next state.
	return addTransition(fromState, toState, [](){ return true; });
}

template <typename T>
bool StateMachine<T>::setInitialState(std::string const& stateName)
{
	// can only set an existing state
	if (mStateMap.find(stateName) != mStateMap.end())
		return false;

	mInitialStateName = stateName;
}

template <typename T>
bool StateMachine<T>::setFinalState(std::string const& stateName)
{
	if (mStateMap.find(stateName) != mStateMap.end())
		return false;
	
	mFinalStateName = stateName;
}

template <typename T>
std::string StateMachine<T>::getPresentState() const
{
	return mPresentStateName;
}

template <typename T>
bool StateMachine<T>::start()
{
	// protect handler thread from duplicate calls
	if (mIsActive.load(std::memory_order_acquire))
		return true;

	// initial state was not set
	if (mInitialStateName == "__null__")
		return false;

	auto const kThreadPoolCount = std::thread::hardware_concurrency();

	// hardware concurrency not defined or computable
	if (!kThreadPoolCount)
		return false;

	// initialize thread pool
	try {
		for (auto count = 0; count < kThreadPoolCount; count++) {
			mEventThreadPool.push_back(std::thread([&]() {
				while (mIsActive.load(std::memory_order_acquire)) {
					std::function<T> event;
					if (mEventQueue.try_dequeue(event)) {
						try {
							event();
						}
						catch (std::bad_function_call const& e) {
							std::cerr << "ERROR: Invalid state event function" 
								<< e.what() << std::endl;
						}
						catch (...) {
							std::cerr << "ERROR: Unexpected exception in event thread pool " 
								<< std::this_thread::get_id << std::endl;
						}
					}
					else {
						std::this_thread::yield();
					}
				}
			}));
		}
	}
	catch (std::bad_alloc const& e) {
		std::cerr << "ERROR: Cannot start event thread pool" << e.what();
		return false;
	}

	// initialize state handler thread
	try {
		assert(!pStateHandlerThread);

		pStateHandlerThread = std::make_shared<std::thread>(&StateMachine<T>::stateHandler, this);
	}
	catch (std::bad_alloc const& e) {
		std::cerr << "ERROR: Cannot start state handler thread"
		 << e.what();
		return false;
	}

	mIsActive.store(true, std::memory_order_release);

	//stateHandler();

	return true;
}

template <typename T>
bool StateMachine<T>::start(std::string const& initialStateName)
{
	mInitialStateName = initialStateName;

	return start();
}

template <typename T>
bool StateMachine<T>::stop()
{
	mIsActive.store(false, std::memory_order_release);

	// protect thread from stopping without starting
	if (!pStateHandlerThread)
		return false;

	if (!pStateHandlerThread->joinable())
		return false;

	pStateHandlerThread->join();
	pStateHandlerThread = nullptr;

	// mIsActive is false, worker threads should be dormant
	for (auto& thread : mEventThreadPool) {
		if (thread.joinable())
			thread.join();
	}

	return true;
}

template <typename T>
void StateMachine<T>::stateHandler()
{
	auto presentName = mInitialStateName;
	//auto presentName = std::string("up");

	while (mIsActive.load(std::memory_order_acquire)) {
		// stateEvent is handled by event thread pool
		mEventQueue.enqueue(mStateMap[presentName]);

		// if state has transitions
		if (mTransitionMap.find(presentName) != mTransitionMap.end()) {
			for (auto& transition : mTransitionMap[presentName]) {
				try {
					if (transition.second()) { // condition satisfied, change state
						presentName = transition.first;
						break; // chooses first path found
					}
				}
				catch (std::bad_function_call const& e) {
					std::cerr << "ERROR: Failed to execute transition predicate" 
						<< e.what() << std::endl;
				}
				catch (...) {
					std::cerr << "ERROR: Unexpected exception calling transition function" 
						<< std::this_thread::get_id << std::endl;
				}
			}
		}
	}
}

} // namespace device
} // namespace tamgef

#endif
