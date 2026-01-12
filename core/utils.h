#ifndef UTILS_H
#define UTILS_H

#include <string>

int SetNonBlock(int fd);

std::string LTrim(const std::string &str);
std::string RTrim(const std::string &str);
std::string Trim(const std::string &str);

#endif // UTILS_H