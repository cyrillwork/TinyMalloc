#include "malloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct HookChunk HookChunk;

extern void *__libc_malloc(size_t size);
extern void __libc_free(void *ptr);
extern void *__libc_realloc(void * ptr, size_t size);
extern void *__libc_memalign(size_t boundary, size_t size);

void* my_malloc_hook (size_t size, void *caller);
void my_free_hook (void *ptr, void *caller);
void* my_realloc_hook (void *ptr, size_t size, void *caller);

size_t malloc_hook_active = 0;
size_t total_alloc_mem;
size_t count_alloc_chunks;


void *ptr_base_heap = 0;
void *last_valid_addr = 0;

size_t count_free_mem = 0;

void* memalign(size_t boundary, size_t size){
    return __libc_memalign(boundary, size);
}

void free (void *ptr)
{
    void *caller;

    if(!ptr) {
        //printf("!!!!!!!!!!!!!!! Error free null\n");
        return;
    }

    if(/*!malloc_hook_active ||*/ !mutex_malloc) {
        __libc_free(ptr);
        return;
    }

    pthread_mutex_lock(mutex_malloc);

    caller = __builtin_return_address(0);

    if (malloc_hook_active &&
        ( (ptr_base_heap <= ptr) && (ptr <= ptr_base_heap + TOTAL_MEM_HOOK_MALLOC) )
            ) {
        my_free_hook(ptr, caller);
    } else {
        __libc_free(ptr);
    }

    pthread_mutex_unlock(mutex_malloc);
}

void my_free_hook (void *ptr, void *caller)
{
    // deactivate hooks for logging
    malloc_hook_active = 0;

    HookChunk *ptr1 = (HookChunk *)(ptr) - 1;
    ptr1->is_available = 0;

    // reactivate hooks
    malloc_hook_active = 1;
}

void* realloc (void* ptr, size_t size)
{
    void *result = 0;
    void *caller;

//    if((784700 < size) && (size < 784800)){
//        printf("realloc\n");
//    }

    if(/*!malloc_hook_active ||*/ !mutex_malloc) {
        return __libc_realloc(ptr, size);
    }

    pthread_mutex_lock(mutex_malloc);

    caller =  __builtin_return_address(0);

    if (malloc_hook_active)
    {
        if(size >= SIZE_MEM_HOOK_MALLOC) {
            if(!ptr) {
                result =  my_malloc_hook(size, caller);
            } else if(! ((ptr_base_heap <= ptr) && (ptr <= ptr_base_heap + TOTAL_MEM_HOOK_MALLOC))  ) {
                result = __libc_realloc(ptr, size);
            } else {
                result =  my_realloc_hook(ptr, size, caller);
            }
        } else {
            if( (ptr_base_heap <= ptr) && (ptr <= ptr_base_heap + TOTAL_MEM_HOOK_MALLOC) ) {
                result = ptr;
            } else {
                result = __libc_realloc(ptr, size);
            }
        }

    } else {
        result = __libc_realloc(ptr, size);
    }

    pthread_mutex_unlock(mutex_malloc);

    return result;
}

void* my_realloc_hook (void *ptr, size_t size, void *caller)
{
    void *result = 0;

    malloc_hook_active = 0;

    HookChunk *curr_ptr = 0;

    if(ptr)
    {
        //first trying used exist block
        HookChunk *ptr1 = (HookChunk *)ptr - 1;
        {
            if(ptr1->size >= size)
            {
                result = ptr1 + 1;
                ptr1->is_available = 1;
            }
            curr_ptr = ptr1;
        }
    }

    if(!result){
        HookChunk *ptr_next = ptr_base_heap;
        while(ptr_next != last_valid_addr) {
            //look for allocated block
            if((ptr_next->is_available == 0)&&(ptr_next->size >= size)) {
                result = ptr_next + 1;
                ptr_next->is_available = 1;
                //ptr1->curr_size = size;
                if(curr_ptr){
                    curr_ptr->is_available = 0;
                }
                break;
            }
            ptr_next = (HookChunk*)((char*)(ptr_next + 1) + ptr_next->size);
        }
    }

    if(!result){
        //allocate new block
        if(total_alloc_mem + size <= TOTAL_MEM_HOOK_MALLOC)
        {
            ++count_alloc_chunks;
            result = (char*)ptr_base_heap + total_alloc_mem;
            {
                HookChunk *chunk = (HookChunk*)result;
                chunk->size = size;
                //chunk->curr_size = size;
                chunk->is_available = 1;
                result = chunk + 1;

            }
            total_alloc_mem += size + sizeof(HookChunk)*count_alloc_chunks;

            if(curr_ptr){
                curr_ptr->is_available = 0;
            }

        } else {
            printf("!!!!!!!!!!!!!!! Error realloc size =%ld\n",
                   size + total_alloc_mem);
        }

    }


    malloc_hook_active = 1;

    return result;
}


void* malloc (size_t size)
{
    void *result;
    void *caller;

    if(/*!malloc_hook_active ||*/ !mutex_malloc) {
        return __libc_malloc(size);
    }

    pthread_mutex_lock(mutex_malloc);

    caller =  __builtin_return_address(0);

    if (malloc_hook_active && (size >= SIZE_MEM_HOOK_MALLOC) /*&& ((total_alloc_mem + size) < TOTAL_MEM_HOOK_MALLOC)*/) {
        result =  my_malloc_hook(size, caller);
    } else {
        result = __libc_malloc(size);
    }

    pthread_mutex_unlock(mutex_malloc);

    return result;
}

const size_t _align_malloc = 16; //sizeof(void*);

void* my_malloc_hook (size_t size, void *caller)
{
    void *result = 0;

    malloc_hook_active = 0;

    long diff_size = -1;

    size_t _align = size % _align_malloc;
    if(_align) {
        size += _align_malloc - _align;
    }

    HookChunk *ptr_next = (HookChunk*)ptr_base_heap;
    HookChunk *find_ptr = 0;

    while(ptr_next != last_valid_addr)
    {
        if((ptr_next->is_available == 0) && (size <= ptr_next->size))
        {
            if( (diff_size == -1) || ((ptr_next->size - size) < (size_t)diff_size) )
            {
                diff_size = ptr_next->size - size;
                find_ptr = ptr_next;
            }
        }
        ptr_next = (HookChunk *)((char*)(ptr_next + 1) + ptr_next->size);
    }

    if(find_ptr) {
        result = find_ptr + 1;
        //find_ptr->curr_size = size;
        find_ptr->is_available = 1;
    } else {
        if(total_alloc_mem + size <= TOTAL_MEM_HOOK_MALLOC)
        {
            ++count_alloc_chunks;
            result = (char*)ptr_base_heap + total_alloc_mem;
            {
                HookChunk *chunk = (HookChunk*)(result);
                chunk->size = size;
                //chunk->curr_size = size;
                chunk->is_available = 1;
                last_valid_addr = (char*)result + sizeof(HookChunk) + size;
                result = (HookChunk*)(result) + 1;                
            }            
            total_alloc_mem += size + sizeof(HookChunk);
        } else {
            printf("!!!!!!!!!!!!!!! Error alloc size =%ld\n", size);
        }
    }

    //    printf("!!!!!!!!!!!!!!! malloc(size=%ld) total=%ld reused=%d\n",
    //           size, total_alloc_mem, reused);

    malloc_hook_active = 1;
    return result;
}

void InitTinyMalloc(size_t size_pool, size_t size_chunk) {

    if(malloc_hook_active) {
        printf("!!!!!!!!!!!!!!! Error InitHookMalloc has already been executed.\n");
        return;
    }

    TOTAL_MEM_HOOK_MALLOC = size_pool;
    SIZE_MEM_HOOK_MALLOC = size_chunk;

    total_alloc_mem = 0;
    count_alloc_chunks = 0;

    ptr_base_heap = __libc_malloc(TOTAL_MEM_HOOK_MALLOC);
    if(!ptr_base_heap){
        printf("!!!!!!!!!!!!!!! Error InitHookMalloc\n");
        return;
    }
    last_valid_addr = ptr_base_heap;

    memset(ptr_base_heap, 0, TOTAL_MEM_HOOK_MALLOC);

    mutex_malloc = (pthread_mutex_t*)__libc_malloc(sizeof (pthread_mutex_t));
    memset(mutex_malloc, 0, sizeof(pthread_mutex_t));

    malloc_hook_active = 1;
}

void PackTinyMalloc()
{
    HookChunk *ptr_next;

    pthread_mutex_lock(mutex_malloc);

    ptr_next = (HookChunk*)ptr_base_heap;

    while(ptr_next != last_valid_addr) {
        HookChunk *ptr_next2 = (HookChunk*)((char*)(ptr_next + 1) + ptr_next->size);

        if( (!ptr_next->is_available) && (ptr_next2 != last_valid_addr) && (!ptr_next2->is_available) ) {
            ptr_next->size += ptr_next2->size + sizeof(HookChunk);
        } else {
            ptr_next = (HookChunk*)((char*)(ptr_next + 1) + ptr_next->size);
        }
    }

    pthread_mutex_unlock(mutex_malloc);

}


void DumpTinyMalloc(const char* name)
{
    FILE* fd;

    //pthread_mutex_lock(&mutex);
    malloc_hook_active = 0;

    fd = fopen(name, "w");

    if(fd) {
        HookChunk *ptr1 = (HookChunk*)ptr_base_heap;
        size_t _total = 0;
        size_t _used = 0;
        int counter = 1;

        fprintf(fd, "Num \t addr\t\t used \t size\n");

        while(ptr1) {

            fprintf(fd, "%d\t%p\t%d\t%ld\n" ,
                    counter, ptr1, ptr1->is_available, ptr1->size);

            _total += ptr1->size;
            if(ptr1->is_available) {
                _used += ptr1->size;
            }
            ++counter;

            ptr1 = (HookChunk*)((char*)ptr1 + ptr1->size + sizeof(HookChunk));
            if(!ptr1->size) {
                ptr1 = 0;
            }

        }

        fprintf(fd, "----------------------------------------\n");
        fprintf(fd, "total alloc mem %ld used %ld free %ld\n" ,
                _total, _used, _total - _used);
        fprintf(fd, "free mem %ld\n", TOTAL_MEM_HOOK_MALLOC - _total);

        fclose(fd);
    }

    //pthread_mutex_unlock(&mutex);
    malloc_hook_active = 1;
}


void StopTinyMalloc()
{
    malloc_hook_active = 0;
}

void StartTinyMalloc()
{
    malloc_hook_active = 1;
}
