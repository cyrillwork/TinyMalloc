#ifndef MALLOC_HPP
#define MALLOC_HPP

#include <unistd.h>
#include <pthread.h>

size_t TOTAL_MEM_HOOK_MALLOC = 3000000000;

//mmap
//#define SIZE_MEM_HOOK_MALLOC      256000
//#define SIZE_MEM_HOOK_MALLOC      131072

//Heap
//#define SIZE_MEM_HOOK_MALLOC      65536
//#define SIZE_MEM_HOOK_MALLOC      32768
//#define SIZE_MEM_HOOK_MALLOC      4096
//#define SIZE_MEM_HOOK_MALLOC      2048

size_t SIZE_MEM_HOOK_MALLOC = 16;

pthread_mutex_t *mutex_malloc = 0;

struct HookChunk;

struct __attribute__ ((packed)) HookChunk
{
    size_t  size;
    int     is_available;
    int     unused1;
    size_t  unused2;
};



void InitTinyMalloc(size_t size_pool, size_t size_chunk);

void PackTinyMalloc();

void DumpTinyMalloc(const char* name);

void StopTinyMalloc();

void StartTinyMalloc();


#endif
