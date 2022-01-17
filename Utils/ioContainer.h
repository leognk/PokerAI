#ifndef OPT_IOCONTAINER_H
#define OPT_IOCONTAINER_H

#include <string>
#include <fstream>

namespace opt {

template<typename A>
static void saveArray(const A& arr, const std::string& path)
{
	auto file = std::fstream(path, std::ios::out | std::ios::binary);
	file.write((char*)&arr[0], sizeof(arr));
	file.close();
}

// Warning: 2D vectors are NOT in contiguous memory.
// 1D vectors are.
template<typename V>
static void save2DVector(const V& v, const std::string& path)
{
	auto file = std::fstream(path, std::ios::out | std::ios::binary);
	for (const auto& w : v)
		file.write((char*)&w[0], sizeof(w));
	file.close();
}

template<typename A>
static void loadArray(A& arr, const std::string& path)
{
	auto file = std::fstream(path, std::ios::in | std::ios::binary);
	file.read((char*)&arr[0], sizeof(arr));
	file.close();
}

// Warning: 2D vectors are NOT in contiguous memory.
// 1D vectors are.
template<typename V>
static void load2DVector(V& v, const std::string& path)
{
	auto file = std::fstream(path, std::ios::in | std::ios::binary);
	for (auto& w : v)
		file.read((char*)&w[0], sizeof(w));
	file.close();
}

} // opt

#endif // OPT_IOCONTAINER_H