#include <map>
#include <vector>

namespace observer {
class IObserver
{
public:
  virtual ~IObserver() = default;

  //This will receive the latest message
  //It will return true if Observer successfully receives message
  //otherwise, it will return false.
  virtual void Update(int message) 0;
}; //class IObserver
} //namespace observer
