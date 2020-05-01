#ifndef MALLOC_HPP
#define MALLOC_HPP

#include <unistd.h>
#include <pthread.h>

#define TOTAL_MEM_HOOK_MALLOC 3000000000

//#define SIZE_MEM_HOOK_MALLOC      256000
#define SIZE_MEM_HOOK_MALLOC      131072
//#define SIZE_MEM_HOOK_MALLOC      65536
//#define SIZE_MEM_HOOK_MALLOC      4096
//#define SIZE_MEM_HOOK_MALLOC      2048
//#define SIZE_MEM_HOOK_MALLOC      65536
//#define SIZE_MEM_HOOK_MALLOC     32768

pthread_mutex_t *mutex_malloc = 0;

struct HookChunk;

struct __attribute__ ((packed)) HookChunk
{
    size_t  size;
    int     is_available;
    int     unused1;
    size_t  unused2;
};


void InitHookMalloc();

void PackHookMalloc();

void DumpHookMalloc(const char* name);

void StopHookMalloc();

void StartHookMalloc();


#endif
