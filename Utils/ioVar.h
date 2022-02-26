#ifndef OPT_IOVAR_H
#define OPT_IOVAR_H

#include <fstream>
#include <vector>
#include <string>
#include <regex>

namespace opt {

template<typename T>
inline void saveVar(const T& var, std::fstream& file)
{
	file.write((char*)&var, sizeof(var));
}

template<typename T>
inline void loadVar(T& var, std::fstream& file)
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

inline std::string extractVarValue(const std::string& var)
{
	static const std::regex rgx(R"(\S+ = (.+))");
	std::smatch match;
	if (std::regex_search(var, match, rgx))
		return match.str(1);
	else
		throw std::runtime_error("Variable's value not found.");
}

inline void skipLine(std::istream& is)
{
	std::string line;
	std::getline(is, line);
}

} // opt

#endif // OPT_IOVAR_H