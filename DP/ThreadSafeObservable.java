/*
* Class Description - This is a basic implementation of the Observer Pattern in a Thread-Safe context
* and is not intended for production, but rather for educational purposes to get a better "feel" for the
* observer pattern and how it works by using an OOP language such as Java.
*
* For the purpose of Interactive Paint we will think of "Observers" as the input devices (RealSense, Leapmotion, etc.) of Interactive paint.
*
* Author: Camilo Alejandro Riviere
* Contact: (786) 447-8080
*/

public class ThreadSafeObservable {

    public interface Observer {
        void observableChanged();
    }

    // there is no thread-safe implementation of Set, therefore we must pass a thread-safe Map
    // (cont) to Collections.newSetFromMap(Map) in order to obtain thread-safe Set
    private Set<Observer> mappedObservers = Collections.newSetFromMap(new ConcurrentHashMap<Observer, Boolean>(0));
}

/**
* This method registers a new Observer - Once registered, it will be notified when the Subject/Observable is modified.
*/
public void registerObserver(Observer observer) {
    if(observer == null) return;
    mappedObservers.add(observer); // This is thread-safe, because we made a Thread-Safe set when we declared and initialized mappedObservers.
}

/**
* This method removes an Observer - Once removed, it will no longer be modified when Observable changes.
*/
public void unregisterObserver(Observer observer) {
    if (observer != null) {
        mappedObservers.remove(observer); // This is thread-safe, because we made a Thread-Safe set when we declared and initialized mappedObservers.
        observer.observableChanged();
    }
}