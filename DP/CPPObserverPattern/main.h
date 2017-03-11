#include "MySubject.h"
#include "MyObserver.h"

namespace Observer {

int main(int char **)
{
  MyObserver observer1("observer1");
  MyObserver observer2("observer2");
  MyObserver observer3("observer3");
  MySubject subject;

  subject.Subscribe(MySubject::ADD, &observer1);
  subject.Subscribe(MySubject::ADD, &observer2);
  subject.Subscribe(MySubject::REMOVE, &observer2);
  subject.Subscribe(MySubject::REMOVE, &observer3);

  subject.Notify(MySubject::ADD);
  subject.Notify(MySubject::REMOVE);

  return 0;
}
}//namespace observer
