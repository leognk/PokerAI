#ifndef OPT_FASTVECTOR_H
#define OPT_FASTVECTOR_H

#include <vector>

namespace opt {

// Data structure containing a vector allowing the program
// not to reallocate memory at every pop_back and push_back.
// T cannot be bool because vector<bool> behaviour is particular:
// https://stackoverflow.com/questions/63476664/what-does-the-error-return-cannot-convert-from-std-vb-referencestd-wrap
template<typename T>
class FastVector
{
public:
	FastVector() :
		v(), mSize(0)
	{
	}

	FastVector(size_t nElems) :
		v(nElems), mSize(nElems)
	{
	}

	T& operator[](size_t i)
	{
		return v[i];
	}

	T& back()
	{
		return v[mSize - 1];
	}

	size_t size() const
	{
		return mSize;
	}

	bool empty() const
	{
		return mSize == 0;
	}

	void clear()
	{
		mSize = 0;
	}

	void push_back(const T& x)
	{
		if (mSize == v.size())
			v.push_back(x);
		else
			v[mSize] = x;
		++mSize;
	}

	void pop_back()
	{
		--mSize;
	}

private:
	std::vector<T> v;
	size_t mSize;
};

} // opt

#endif // OPT_FASTVECTOR_H