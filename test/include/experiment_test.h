#pragma once

#include <initializer_list>

#include <gtest/gtest.h>

#include <experiment/experiment.h>

class ExperimentTest : public ::testing::Test
{
protected:
  template<class> struct Treatment;

  template<typename R, typename... Args>
  struct Treatment<R(Args...)>
  {
    Treatment(std::initializer_list<std::function<R(Args...)>> tasks)
      : m_tasks(tasks),
        m_tasks_left(tasks.size())
    {}

    void start()
    {
      for (auto & task : m_tasks)
      {
        task();
      }
    }

    bool state()
    {
      return (m_tasks_left == 0);
    }

  private:
    std::vector<std::function<R(Args...)>> m_tasks;
    int m_tasks_left;
  };
};