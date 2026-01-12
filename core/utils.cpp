#include "utils.h"
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <stdexcept>
#include "logger.h"

int SetNonBlock(int fd)
{
    int oldFlag = fcntl(fd, F_GETFL);
    if (0 > oldFlag)
    {
        error("fcntl failed, err: {}", strerror(errno));
        return -1;
    }

    if (0 > fcntl(fd, F_SETFL, oldFlag | O_NONBLOCK))
    {
        error("fcntl failed, err: {}", strerror(errno));
        return -1;
    }
    return oldFlag;
}

std::string LTrim(const std::string &str)
{
    auto pos = str.find_first_not_of(" \t\n");
    if (pos == std::string::npos)
    {
        return "";
    }
    else
    {
        return str.substr(pos);
    }
}

std::string RTrim(const std::string &str)
{
    auto pos = str.find_last_not_of(" \t\n");
    if (pos == std::string::npos)
    {
        return "";
    }
    else
    {
        return str.substr(0, pos + 1);
    }
}

std::string Trim(const std::string &str)
{
    return LTrim(RTrim(str));
}