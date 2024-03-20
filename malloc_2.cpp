#include <unistd.h>
#include <cmath>
#include <string.h>

struct MallocMtadata{
    size_t size;
    bool is_free;
    struct MallocMtadata* next;
    struct MallocMtadata* prev;
};

struct MallocMtadata* ptr = nullptr;

struct MallocMtadata* getFreeBlock(size_t size){
    struct MallocMtadata* temp = ptr;
    while (temp){
        if (temp->is_free && temp->size <= sizeof(MallocMtadata)+size){
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
        block = (struct MallocMtadata*)sbrk(size);
        if (!block){
            return NULL;
        }
        ptr = block;
        block->size = size;
    }
    else{
        block = getFreeBlock(size);
        if (!block){
            block = (struct MallocMtadata*)sbrk(size);
            if (!block){
                return NULL;
            }
            struct MallocMtadata* last = getLastBlock();
            last->next = block;
            block->prev =last;
            block->size = size;
        }
    }
    block->is_free = false;
    return block + sizeof(MallocMtadata);
}


void* scalloc(size_t num, size_t size){
    if (num == 0 || size == 0 || num*size > pow(10, 8)){
        return NULL;
    }
    void* ptr = smalloc(num*size);
    if (!ptr){
        return NULL;
    }
    memset(ptr, 0, size*num);
    return ptr;
}


void sfree(void* p){
    if (!p){
        return;
    }
    struct MallocMtadata* meta = (struct MallocMtadata*)p - sizeof(struct MallocMtadata);
    meta->is_free = true;
}


void* srealloc(void* oldp, size_t size){
    
}
