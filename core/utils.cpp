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