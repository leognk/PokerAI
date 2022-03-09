#ifndef OPT_IO_H
#define OPT_IO_H

#include <fstream>

namespace opt {

// Return a fstream object but throw an exception
// if the file could not be opened.
inline std::fstream fstream(const std::string& filename,
    std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
{
    std::fstream fs(filename, mode);
    if (fs.fail())
        throw std::runtime_error("File could not be opened.");
    return fs;
}

} // opt

#endif // OPT_IO_H