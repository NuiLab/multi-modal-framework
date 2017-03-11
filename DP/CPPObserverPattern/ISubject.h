
namespace Observer {
class ISubject
{
public:
  ISubject();
  virtual ~ISubject();
  virtual void Subscribe(int message, IObserver *observer);
  virtual void Unsubscribe(int message, IObserver *observer);
virtual void Notify(int message); //Notify observers.

private:
  typedef std::vector<IObserver *> ObserverList;
  typedef std::map<int, ObserverList> ObserverMap;
  ObserverMap mObservers;
}; //class ISubject
} //namespace Observer
