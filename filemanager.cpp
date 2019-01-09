#include "filemanager.h"
#include "utils.h"

int FileManager::readFile(std::string filepath, std::string* contents)
{
    if (_cache.find(filepath) != _cache.end())
    {
        *contents = _cache[filepath];
        return _cache[filepath].length();
    }
    int r = readFileIntoString(filepath, contents);
    _cache.insert(std::make_pair(filepath, *contents));
    return r < 0 ? r : contents->length();
}