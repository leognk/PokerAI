#ifndef OPT_IOARRAY_H
#define OPT_IOARRAY_H

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

template<typename A>
static void loadArray(A& arr, const std::string& path)
{
	auto file = std::fstream(path, std::ios::in | std::ios::binary);
	file.read((char*)&arr[0], sizeof(arr));
	file.close();
}

} // opt

#endif // OPT_IOARRAY_H