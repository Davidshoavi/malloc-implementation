#include <unistd.h>
#include <cmath>
#include <string.h>

/*
#include <stddef.h>
#include <assert.h>

#define REQUIRE(bool) assert(bool)
#define MAX_ALLOCATION_SIZE (1e8)

#define verify_blocks(allocated_blocks, allocated_bytes, free_blocks, free_bytes)                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        REQUIRE(_num_allocated_blocks() == allocated_blocks);                                                          \
        REQUIRE(_num_allocated_bytes() == allocated_bytes);                                                            \
        REQUIRE(_num_free_blocks() == free_blocks);                                                                    \
        REQUIRE(_num_free_bytes() == free_bytes);                                                                      \
        REQUIRE(_num_meta_data_bytes() == _size_meta_data() * allocated_blocks);                                       \
    } while (0)

#define verify_size(base)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        void *after = sbrk(0);                                                                                         \
        REQUIRE(_num_allocated_bytes() + _size_meta_data() * _num_allocated_blocks() == (size_t)after - (size_t)base); \
    } while (0)

*/



struct MallocMtadata{
    size_t size;
    bool is_free;
    struct MallocMtadata* next;
    struct MallocMtadata* prev;
};

struct MallocMtadata* ptr = nullptr;
size_t num_free_blocks = 0;
size_t num_free_bytes = 0;
size_t num_allocated_blocks = 0;
size_t num_allocated_bytes = 0;


size_t _size_meta_data(){
    return sizeof(struct MallocMtadata);
}

size_t _num_free_blocks(){
    return num_free_blocks;
}

size_t _num_free_bytes(){
    return num_free_bytes;
}

size_t _num_allocated_blocks(){
    return num_allocated_blocks;
}

size_t _num_allocated_bytes(){
    return num_allocated_bytes;
}

size_t _num_meta_data_bytes(){
    return num_allocated_blocks * _size_meta_data();
}


struct MallocMtadata* getFreeBlock(size_t size){
    struct MallocMtadata* temp = ptr;
    while (temp){
        if (temp->is_free && temp->size >= _size_meta_data()+size){
            return temp;
        }
        temp = temp->next;
    }
    return nullptr;
}


struct MallocMtadata* getLastBlock(){
    if (!ptr){
        return nullptr;
    }
    struct MallocMtadata* temp = ptr;
    while(!temp->next){
        temp = temp->next;
    }
    return temp;
}


void* smalloc(size_t size){
    if (size == 0 || size > pow(10, 8)){
        return NULL;
    }
    struct MallocMtadata* block;
    if (!ptr){
        block = (struct MallocMtadata*)sbrk(size + _size_meta_data());
        if (!block){
            return NULL;
        }
        num_allocated_blocks++;
        num_allocated_bytes += size;
        ptr = block;
        block->size = size;
    }
    else{
        block = getFreeBlock(size);
        if (!block){
            block = (struct MallocMtadata*)sbrk(size + _size_meta_data());
            if (!block){
                return NULL;
            }
            struct MallocMtadata* last = getLastBlock();
            last->next = block;
            block->prev =last;
            block->size = size;
            num_allocated_blocks++;
            num_allocated_bytes += size;
        }
        else{
            num_free_blocks--;
            num_free_bytes -= block->size;
        }
    }
    block->is_free = false;
    return (void *)((size_t)block + _size_meta_data());
}


void* scalloc(size_t num, size_t size){
    if (num == 0 || size == 0 || num*size > pow(10, 8)){
        return NULL;
    }
    void* p = smalloc(num*size);
    if (!p){
        return NULL;
    }
    memset(p, 0, size*num);
    return p;
}


void sfree(void* p){
    if (!p){
        return;
    }
    struct MallocMtadata* meta = (struct MallocMtadata*)((size_t)p - _size_meta_data());
    if (meta->is_free){
        return;
    }
    meta->is_free = true;
    num_free_blocks++;
    num_free_bytes += meta->size;
}


void* srealloc(void* oldp, size_t size){
    if (size == 0 || size > pow(10, 8)){
        return NULL;
    }
    if (!oldp){
        // handle
    }
    struct MallocMtadata* oldMeta = (struct MallocMtadata*)((size_t)oldp - _size_meta_data());
    if (oldMeta->size >= size){
        return oldp;
    }
    void* p = smalloc(size);
    if (!p){
        return NULL;
    }
    memcpy(p, oldp, oldMeta->size);
    sfree(oldp);
    return p;
}

/*

int main(){
    verify_blocks(0, 0, 0, 0);
    void *base = sbrk(0);
    char *a = (char *)smalloc(10);
    REQUIRE(a != nullptr);
    REQUIRE((size_t)base + _size_meta_data() == (size_t)a);
    verify_blocks(1, 10, 0, 0);
    verify_size(base);
    sfree(a);
    verify_blocks(1, 10, 1, 10);
    verify_size(base);
}

*/
