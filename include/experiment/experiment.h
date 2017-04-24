#ifndef TAMGEF_EXPERIMENT_EXPERIMENT_H
#define TAMGEF_EXPERIMENT_EXPERIMENT_H

#include <algorithm>
#include <chrono>
#include <cassert>
#include <ctime>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <random>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <queue>

namespace tamgef {
namespace experiment {

template<typename Treatment>
struct Experiment
{ 
  enum class ExperimentState
  {
    Initial,
    Started,
    Running,
    Paused,
    Stopped
  };

  enum class ExperimentOrder
  {
    Random,
    Sequential
  };

  enum class ExperimentMode
  {
    Training,
    Treatment
  };

  Experiment(std::string const &,
             std::string const &,
             ExperimentOrder);

  virtual ~Experiment() = default;

  // append treatments to treatement list
  // treatments are run in the mode specified
  // only valid in initial state
  void append(Treatment const &, 
              ExperimentMode = ExperimentMode::Treatment);

  // return current experiment mode
  ExperimentMode mode() const;

  // returns name of experiment
  std::string name() const;

  // returns experiment order
  ExperimentOrder order() const;

  // sets current treatment to next in queue 
  // only valid instart, running, paused states
  // returns false if queue is empty
  // changes state to running
  bool next();

  // changes state to paused
  // only valid in running state 
  void pause();

  // passes current treatment to function handler
  // only valid in running, paused states
  // dispatches asynch task with function handler
  // returns future of type TreatmentState
  template<typename TreatmentState>
  auto process(std::function<TreatmentState(std::shared_ptr<Treatment>)>); 
  
  // shuffles treatment queue
  void randomize();

  // changes state to running
  // only valid in paused state
  void resume();

  // calls stop() and clears treatment list 
  // changes state to initial
  void reset();

  // return size of treatment queue
  std::size_t size() const;

  // returns current state of experiment
  ExperimentState state() const;

  // initializes treatment queue according to order
  // only valid in initial, stopped states
  // throws runtime_error if treatment list is empty
  // changes state to started
  void start();

  // resets treatment queue
  // changes state to stopped
  void stop();

  // return subject name
  std::string subject() const;

  // return std::ctime of experiment time
  std::string time() const;

  // returns current treatment
  // only valid in running, paused states
  std::shared_ptr<Treatment> treatment() const;
  
  std::ostream & operator << (std::ostream & out) const;
  friend std::ostream & operator << (std::ostream &, ExperimentState const &);
  friend std::ostream & operator << (std::ostream &, ExperimentOrder const &);

  ExperimentState mExperimentState;
  ExperimentOrder mExperimentOrder;
  ExperimentMode mExperimentMode;
  std::string mExperimentName;
  std::string mSubjectName;
  std::vector<std::pair<Treatment, ExperimentMode>> mTreatmentList;
  std::queue<std::pair<Treatment, ExperimentMode>> mTreatmentQueue;
  std::chrono::system_clock::time_point mExperimentTime;
  std::chrono::system_clock::time_point mTreatmentTime;
  std::shared_ptr<Treatment> pCurrentTreatment;

};// class Experiment

template<typename Treatment>
Experiment<Treatment>::Experiment(std::string const & experimentName,
                                  std::string const & subjectName,
                                  ExperimentOrder experimentMode)
  : mExperimentState(ExperimentState::Initial),
    mExperimentMode(ExperimentMode::Training),
    mExperimentOrder(experimentMode),
    mExperimentName(experimentName),
    mSubjectName(subjectName),
    mTreatmentProcessor(treatmentProcessor)
{}

template<typename Treatment>
void Experiment<Treatment>::append(Treatment const & treatment,
                                   ExperimentMode mode)
{
  if (state() != ExperimentState::Initial)
  {
    throw std::runtime_error("attempt to append after started");
  }
  mTreatmentList.push_back(std::make_pair<Treatment, ExperimentMode>(treatment, mode));
}

template<typename Treatment>
typename Experiment<Treatment>::ExperimentOrder 
Experiment<Treatment>::order() const
{
  return mExperimentOrder;
}

template<typename Treatment>
typename Experiment<Treatment>::ExperimentMode
Experiment<Treatment>::mode() const
{
  return mExperimentMode;
}

template<typename Treatment>
std::string Experiment<Treatment>::name() const
{
  return mExperimentName;
}

template<typename Treatment>
bool Experiment<Treatment>::next()
{
  if (state() != ExperimentState::Started &&
      state() != ExperimentState::Paused  &&
      state() != ExperimentState::Running)
  {
    throw std::runtime_error("attempt to get next while inactive");
  }

  if (mTreatmentQueue.empty()) 
  {
    stop();
    return false;
  }

  pCurrentTreatment = std::make_shared<Treatment>(mTreatmentQueue.front().first);
  mExperimentMode = mTreatmentQueue.front().second;
  mTreatmentQueue.pop();

  mExperimentState = ExperimentState::Running;
  
  return true;
}

template<typename Treatment>
void Experiment<Treatment>::pause()
{
  if (state() != ExperimentState::Running)
  {
    std::runtime_error("attempt to pause inactive experiment");
  }

  mExperimentState = ExperimentState::Paused;
}

template<typename Treatment>
template<typename TreatmentState>
auto Experiment<Treatment>::process(std::function<TreatmentState(std::shared_ptr<Treatment>)> treatmentProcessor)
{
  if (state() != ExperimentState::Running &&
      state() != ExperimentState::Paused)
  {
    throw std::runtime_error("attempt to process while inactive");
  }

  assert(pCurrentTreatment);

  return std::async(treatmentProcessor(pCurrentTreatment)); 
}

template<typename Treatment>
void Experiment<Treatment>::randomize()
{
  if (mTreatmentList.empty())
    return;

  std::random_device rd;
  std::mt19937 generator(rd);
  std::shuffle(mTreatmentList.begin(), 
               mTreatmentList.end(), 
               generator);

  // put all training treatments first
  std::partition(mTreatmentList.begin(),
                 mTreatmentList.end(),
                 [](auto const& treatment)
                 {
                   return treatment.second == ExperimentMode::Training; 
                 });
}

template<typename Treatment>
void Experiment<Treatment>::reset()
{
  stop();
  mTreatmentList.clear();

  mExperimentState = ExperimentState::Initial;
}

template<typename Treatment>
void Experiment<Treatment>::resume()
{
  if (state() != ExperimentState::Paused)
  {
    std::runtime_error("attempt to resume before pausing.");
  }

  mExperimentState = ExperimentState::Running;
}

template<typename Treatment>
void Experiment<Treatment>::start()
{
  if (state() != ExperimentState::Initial ||
      state() != ExperimentState::Stopped)
  {
    throw std::runtime_error("attempt to start before stop");
  }

  if (mTreatmentList.empty())
  {
    throw std::runtime_error("no treatments have been added");
  }

  if (order() == ExperimentOrder::Random)
  {
    randomize();
  }

  for (auto const & treatment : mTreatmentList)
    mTreatmentQueue.push(treatment);

  mExperimentTime = std::chrono::system_clock::now();

  mExperimentState = ExperimentState::Started;
}

template<typename Treatment>
typename Experiment<Treatment>::ExperimentState 
Experiment<Treatment>::state() const
{
  return mExperimentState;
}

template<typename Treatment>
std::size_t Experiment<Treatment>::size() const
{
  return mTreatmentQueue.size();
}

template<typename Treatment>
void Experiment<Treatment>::stop()
{
  pCurrentTreatment.reset();
  mTreatmentQueue.clear();

  mExperimentState = ExperimentState::Stopped;
}

template<typename Treatment>
std::string Experiment<Treatment>::subject() const
{
  return mSubjectName;
}

template<typename Treatment>
std::string Experiment<Treatment>::time() const
{
  auto experimentTime = std::chrono::system_clock::to_time_t(mExperimentTime);
  return std::string(std::ctime(experimentTime));
}

template<typename Treatment>
std::shared_ptr<Treatment> 
Experiment<Treatment>::treatment() const
{
  return pCurrentTreatment;
}

template<typename Treatment>
std::ostream & 
Experiment<Treatment>::operator << (std::ostream & out) const
{
  out << "{Experiment}: "
      << " Experiment Name:  " << name()
      << " Subject Name:     " << subject()
      << " Exeriment Time:   " << time()
      << " Experiment State: " << state()
      << " Experiment Order: " << order();
  
  return out;
}

template<typename Treatment>
std::ostream & 
operator << (std::ostream & out, 
             typename Experiment<Treatment>::ExperimentState const & state)
{
  switch (state)
  {
  case Experiment<Treatment>::ExperimentState::Initial:
    out << "Initial"; 
    break;
  case Experiment<Treatment>::ExperimentState::Started:
    out << "Started"; 
    break;
  case Experiment<Treatment>::ExperimentState::Running:
    out << "Running"; 
    break;
  case Experiment<Treatment>::ExperimentState::Paused:
    out << "Paused"; 
    break;
  case Experiment<Treatment>::ExperimentState::Stopped:
    out << "Stopped"; 
    break;
  }
    
  return out;
}

template<typename Treatment>
std::ostream & 
operator << (std::ostream & out, 
             typename Experiment<Treatment>::ExperimentOrder const & order)
{
  switch (order)
  {
  case Experiment<Treatment>::ExperimentOrder::Random:
    out << "Random"; 
    break;
  case Experiment<Treatment>::ExperimentOrder::Sequential:
    out << "Sequential"; 
    break;
  }

  return out;
}

} // namespace experiment
} // namespace tamgef

#endif
