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


bool my_set_nonblock(int fd, bool b)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if(b)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) != -1;
}

const char* my_get_error()
{
    return strerror(errno);
}

/*
头文件:
    #include <dlfcn.h>

mode参数：
    RTLD_LAZY 暂缓决定，等有需要时再解出符号;
    RTLD_NOW 立即决定，返回前解除所有未决定的符号。
    RTLD_LOCAL 与 RTLD_GLOBAL 作用相反，动态库中定义的符号不能被其后打开的其它库重定位。
    如果没有指明是RTLD_GLOBAL还是RTLD_LOCAL，则缺省为RTLD_LOCAL。
    RTLD_GLOBAL 动态库中定义的符号可被其后打开的其它库重定位。

返回值:
    打开错误返回NULL，成功，返回库引用。

编译:
     -ldl (指定dl库)：例如　gcc test.c -o test -ldl

说明:
    如果一个函数库里面有一个输出的函数名字为_init,那么_init就会在dlopen（）这个函数返回前被执行。
    我们可以利用这个函数在我的函数库里面做一些初始化的工作。
*/
void* my_dll_open(const char* dl_path)
{
    void* handle;
    void* error;

    handle = dlopen(dl_path, RTLD_NOW | RTLD_GLOBAL);
    error = dlerror();
    if(error)
    {
        printf("Open dll %s failed, %s\n",dl_path, (char*)error);
        return NULL;
    }
    return handle;
}

void* my_dll_use(void *handle, const char *symbol)
{
    void* func;
    void* error;

    func = dlsym(handle, symbol);
    error = dlerror();
    if(error)
    {
        printf("Use dll sym failed, %s\n", (char*)error);
        return NULL;
    }
    return func;
}

int my_dll_close(void *handle)
{
    int res;
    void* error;
    res = dlclose(handle);
    if(res != 0)
    {
        error = dlerror();
        if(error)
        {
            printf("DLL close failed, %s\n", (char*)error);
            return -1;
        }
    }
    return res;
}
