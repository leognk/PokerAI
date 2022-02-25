#ifndef OPT_IOVAR_H
#define OPT_IOVAR_H

#include <ostream>
#include <vector>
#include <string>

namespace opt {

template<typename T>
inline void saveVar(const T& var, std::fstream& file)
{
	file.write((char*)&var, sizeof(var));
}

template<typename T>
inline void loadVar(T& var, const std::fstream& file)
{
	file.read((char*)&var, sizeof(var));
}

#define WRITE_VAR(os, var) opt::writeVar(os, var, #var)

inline std::ostream& operator<<(std::ostream& os, const uint8_t& n)
{
	return os << std::to_string(n);
}

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
	os << "{ ";
	for (size_t i = 0; i < v.size(); ++i) {
		os << v[i];
		if (i != v.size() - 1) os << ", ";
	}
	os << " }";
	return os;
}
	
template<typename T>
inline std::ostream& writeVar(std::ostream& os, const T& var, const char* varName)
{

	return os << varName << " = " << var << "\n";
}

} // opt

#endif // OPT_IOVAR_H