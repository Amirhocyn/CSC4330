#ifndef FILEPARSE_H
#define FILEPARSE_H

#include <string>

namespace TextExtraction
{
    struct FileParse
    {
        std::string fileName;
        std::string fileText;
    };

    static std::string readWholeFile(const std::string &filename) {};

};

#endif
