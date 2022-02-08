#ifndef OPT_STRINGMANIP_H
#define OPT_STRINGMANIP_H

#include <string>

namespace opt {

inline std::string toUpper(const std::string& s)
{
	std::string res = s;
	for (auto& c : res) c = toupper(c);
	return res;
}

} // opt

#endif // OPT_STRINGMANIP_H