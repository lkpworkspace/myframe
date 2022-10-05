/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "MyCUtils.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#define NANOSEC 1000000000
#define MICROSEC 1000000

uint8_t my_random_num(int min, int max)
{
    static bool is_invoke = false;
    if(!is_invoke)
    {
        srand((unsigned int)time(NULL));
        is_invoke = true;
    }
    uint8_t temp = 0x00;

    if (max < min) return 0x00;
    temp = rand() % (max - min) + min;
    return temp;
}

double my_gettime_sec() {
    struct timespec ti;
    clock_gettime(CLOCK_MONOTONIC, &ti);

    int sec = ti.tv_sec;
    int nsec = ti.tv_nsec;

    return (double)sec + (double)nsec / NANOSEC;
}

void my_systime_ms(uint32_t *sec, uint32_t *ms)
{
    struct timespec ti;
    clock_gettime(CLOCK_REALTIME, &ti);
    *sec = (uint32_t)ti.tv_sec;
    *ms = (uint32_t)(ti.tv_nsec / 1000000);
}

uint64_t my_gettime_ms()
{
    uint64_t t;
    struct timespec ti;
    clock_gettime(CLOCK_MONOTONIC, &ti);
    t = (uint64_t)ti.tv_sec * 1000;
    t += ti.tv_nsec / 1000000;
    return t;
}

bool my_set_sock_recv_timeout(int fd, int timeout_ms) {
    struct timeval timeout = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    return 0 == setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
}

bool my_set_nonblock(int fd, bool b)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if(b) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    return fcntl(fd, F_SETFL, flags) != -1;
}

bool my_is_block(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return !(flags & O_NONBLOCK);
}

const char* my_get_error()
{
    return strerror(errno);
}
