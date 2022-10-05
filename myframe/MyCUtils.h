/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t my_random_num(int min, int max);

double my_gettime_sec();

void my_systime_ms(uint32_t *sec, uint32_t *ms);

uint64_t my_gettime_ms();

bool my_set_sock_recv_timeout(int fd, int timeout_ms);
bool my_set_nonblock(int fd, bool b);
bool my_is_block(int fd);

const char* my_get_error();

#ifdef __cplusplus
}
#endif

