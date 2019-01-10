#include "utils.h"
#include <string.h>
#include <fstream>

int readFileIntoString(std::string filename, std::string* contents)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in)
    {
        in.seekg(0, std::ios::end);
        contents->resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&(*contents)[0], contents->size());
        in.close();
        return 0;
    } 
    return -1;
}

int readFileStat(std::string filename, struct stat* statp)
{
    struct stat attr;
    int r = stat(filename.c_str(), &attr);
    if (r == -1) {
        return r;
    }
    memcpy(statp, &attr, sizeof(attr));
    return 0;
}

std::string extractFileExtension(std::string* filename)
{
    if(filename->find_last_of(".") != std::string::npos)
        return filename->substr(filename->find_last_of(".") + 1);
    return "";
}

