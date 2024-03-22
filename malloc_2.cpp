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
size_t num_free_blocks = 0;
size_t num_free_bytes = 0;
size_t num_allocated_blocks = 0;
size_t num_allocated_bytes = 0;

struct MallocMtadata* getFreeBlock(size_t size){
    struct MallocMtadata* temp = ptr;
    while (temp){
        if (temp->is_free && temp->size <= _size_meta_data()+size){
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
    return block + sizeof(struct MallocMtadata);
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
    struct MallocMtadata* meta = (struct MallocMtadata*)p - _size_meta_data();
    meta->is_free = true;
    num_free_blocks++;
    num_free_bytes += meta->size;

}


void* srealloc(void* oldp, size_t size){
    if (size == 0 || size > pow(10, 8)){
        return NULL;
    }
    struct MallocMtadata* oldMeta = (struct MallocMtadata*)oldp - _size_meta_data();
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

size_t _size_meta_data(){
    return sizeof(struct MallocMtadata);
}
