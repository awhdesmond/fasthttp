#pragma once

#include <sys/stat.h>
#include <string>

#define FILE_EXT_JSON "json"
#define FILE_EXT_XML "xml"
#define FILE_EXT_HTML "html"
#define FILE_EXT_CSV "csv"

int readFileStat(std::string filename, struct stat* mtime);
int readFileIntoString(std::string filename, std::string* contents);
std::string extractFileExtension(std::string* filename);