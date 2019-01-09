#pragma once

#include <string>
#include <map>

class FileManager
{
    public:
        FileManager() {};
        ~FileManager() {};

        int readFile(std::string filepath, std::string* contents);
    
    private:
        std::map<std::string, std::string> _cache;
        // int refreshCache();
};