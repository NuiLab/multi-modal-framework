#ifndef DOMAIN_H
#define DOMAIN_H

#include <functional>

namespace tamgef {
namespace device { 

template<typename T>
class Domain
{
public:
	Domain() = default;
	Domain(Domain<T> const&);
	Domain(Domain<T> &&);
	Domain<T> operator=(Domain<T> const&);
	Domain<T> operator=(Domain<T> &&);
	virtual ~Domain() = default;

	Domain(std::function<bool(T)>);

	bool operator()(T) const;
	Domain<T> operator+(Domain<T> const&) const;
	Domain<T> operator*(Domain<T> const&) const;

private:
	std::function<bool(T)> mPredicate;
};

template<typename T>
Domain<T>::Domain(Domain<T> const& other) :
	mPredicate(other.mPredicate)
{}

template<typename T>
Domain<T>::Domain(Domain<T> && other) :
	mPredicate(std::move(other.mPredicate))
{}

template<typename T>
Domain<T> Domain<T>::operator=(Domain<T> const& other)
{
	mPredicate = other.mPredicate;
	return *this;
}

template<typename T>
Domain<T> Domain<T>::operator=(Domain<T> && other)
{
	mPredicate = std::move(other.mPredicate);
	return *this;
}

template<typename T>
Domain<T>::Domain(std::function<bool(T)> predicate) :
	mPredicate(predicate)
{}

template<typename T>
bool Domain<T>::operator()(T element) const
{
	return mPredicate(element);
}

template<typename T>
Domain<T> Domain<T>::operator+(Domain<T> const& other) const
{
	Domain<T> compositeDomain(
			[=](T const& element) -> bool
			{
				return mPredicate(element) || other.mPredicate(element);
			});

	return compositeDomain;
}

template<typename T>
Domain<T> Domain<T>::operator*(Domain<T> const& other) const
{
	Domain<T> compositeDomain(
			[=](T const& element) -> bool
			{
				return mPredicate(element) && other.mPredicate(element);
			});

	return compositeDomain;
}

} // namespace device
} // namespace tamgef

#endif
