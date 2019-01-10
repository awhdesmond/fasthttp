#pragma once

#include <sys/stat.h>
#include <string>

#define CONTENT_TYPE_JSON "json"
#define CONTENT_TYPE_XML "xml"
#define CONTENT_TYPE_HTML "html"
#define CONTENT_TYPE_PLAIN "plain"
#define CONTENT_TYPE_CSV "csv"

int readFileStat(std::string filename, struct stat* mtime);
int readFileIntoString(std::string filename, std::string* contents);
std::string extractFileExtension(std::string* filename);