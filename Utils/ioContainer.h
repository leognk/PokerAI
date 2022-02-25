#ifndef OPT_IOCONTAINER_H
#define OPT_IOCONTAINER_H

#include <string>
#include <fstream>

namespace opt {

template<typename A>
inline void saveArray(const A& arr, std::fstream& file)
{
	file.write((char*)&arr[0], sizeof(arr));
}

template<typename A>
inline void saveArray(const A& arr, const std::string& path)
{
	auto file = std::fstream(path, std::ios::out | std::ios::binary);
	saveArray(arr, file);
	file.close();
}

template<typename A>
inline void loadArray(A& arr, std::fstream& file)
{
	file.read((char*)&arr[0], sizeof(arr));
}

template<typename A>
inline void loadArray(A& arr, const std::string& path)
{
	auto file = std::fstream(path, std::ios::in | std::ios::binary);
	loadArray(arr, file);
	file.close();
}

template<typename V>
inline void save1DVector(const V& v, std::fstream& file)
{
	file.write((char*)&v[0], v.size() * sizeof(v[0]));
}

template<typename V>
inline void save1DVector(const V& v, const std::string& path)
{
	auto file = std::fstream(path, std::ios::out | std::ios::binary);
	save1DVector(v, file);
	file.close();
}

template<typename V>
inline void load1DVector(V& v, std::fstream& file)
{
	file.read((char*)&v[0], v.size() * sizeof(v[0]));
}

template<typename V>
inline void load1DVector(V& v, const std::string& path)
{
	auto file = std::fstream(path, std::ios::in | std::ios::binary);
	load1DVector(v, file);
	file.close();
}

// Warning: 2D vectors are NOT in contiguous memory.
// 1D vectors are.
template<typename V>
inline void save2DVector(const V& v, std::fstream& file)
{
	for (const auto& w : v) save1DVector(w, file);
}

template<typename V>
inline void save2DVector(const V& v, const std::string& path)
{
	auto file = std::fstream(path, std::ios::out | std::ios::binary);
	save2DVector(v, file);
	file.close();
}

// Warning: 2D vectors are NOT in contiguous memory.
// 1D vectors are.
template<typename V>
inline void load2DVector(V& v, std::fstream& file)
{
	for (auto& w : v) load1DVector(w, file);
}

template<typename V>
inline void load2DVector(V& v, const std::string& path)
{
	auto file = std::fstream(path, std::ios::in | std::ios::binary);
	load2DVector(v, file);
	file.close();
}

template<typename V>
inline void save3DVector(const V& v, std::fstream& file)
{
	for (const auto& w : v) save2DVector(w, file);
}

template<typename V>
inline void save3DVector(const V& v, const std::string& path)
{
	auto file = std::fstream(path, std::ios::out | std::ios::binary);
	save3DVector(v, file);
	file.close();
}

template<typename V>
inline void load3DVector(V& v, std::fstream& file)
{
	for (auto& w : v) load2DVector(w, file);
}

template<typename V>
inline void load3DVector(V& v, const std::string& path)
{
	auto file = std::fstream(path, std::ios::in | std::ios::binary);
	load3DVector(v, file);
	file.close();
}

} // opt

#endif // OPT_IOCONTAINER_H