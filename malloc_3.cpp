#include <unistd.h>
#include <cmath>
#include <string.h>
#include <sys/mman.h>
#define MAX_ORDER 10
#define ORDER_SIZE(i) (size_t)pow(2, i)*128


struct MallocMtadata{
    size_t size;
    bool is_free;
    struct MallocMtadata* next;
    struct MallocMtadata* prev;
    struct MallocMtadata* next_order;
    struct MallocMtadata* prev_order;
    unsigned char order;
};


struct MallocMtadata* heap_ptr = nullptr;
struct MallocMtadata* orders[MAX_ORDER+1];
struct MallocMtadata* mmap_ptr = nullptr;

size_t _size_meta_data(){
    return sizeof(struct MallocMtadata);
}


size_t _num_free_blocks(){
    return 0;
}

size_t _num_free_bytes(){
    return 0;
}

size_t _num_allocated_blocks(){
    return 0;
}

size_t _num_allocated_bytes(){
    return 0;
}

size_t _num_meta_data_bytes(){
    return 0 * _size_meta_data();
}


void align_memory(){
    void* ptr = sbrk(0);
    size_t x = ((size_t)ptr) % (32*128*1024);
    sbrk(32*128*1024 - x);
}


void initialHeap(){
    align_memory();
    heap_ptr = (struct MallocMtadata*)sbrk(ORDER_SIZE(MAX_ORDER)*32);
    orders[10] = heap_ptr;
    struct MallocMtadata* temp = heap_ptr;
    struct MallocMtadata* tempPrev;
    for (int i = 0; i<31; i++){
        temp->next = temp + ORDER_SIZE(MAX_ORDER);
        temp->next_order = temp->next;
        temp->size = ORDER_SIZE(MAX_ORDER);
        temp->is_free = true;
        tempPrev = temp;
        temp = temp->next;
        temp->prev = tempPrev;
        temp->prev_order = temp->prev;
    }
    temp->size = ORDER_SIZE(MAX_ORDER);
    temp->is_free = true;
}


unsigned char getOrder(size_t size){
    for (int i=0; i<=MAX_ORDER; i++){
        if (size <= ORDER_SIZE(i)){
            return (unsigned char)i;
        }
    }
    return (unsigned char)11;
}


void addFreeBlockToOrders(int order, struct MallocMtadata* block){
    if (!orders[order]){
        orders[order] = block;
        block->next_order = nullptr;
        block->prev_order = nullptr;
        return;
    }
    struct MallocMtadata* temp =  orders[order];
    struct MallocMtadata* temp_prev;
    while (temp && temp < block){
        temp_prev = temp;
        temp = temp->next_order;
    }
    if (!temp){
        temp_prev->next_order = block;
        block->prev_order = temp_prev;
        return;
    }
    block->prev_order = temp->prev_order;
    block->next_order = temp;
    block->prev_order->next_order = block;
    temp->prev_order = block;
}


void split(int splitValue, int order, struct MallocMtadata* block){ // block is the first block on the list!
    int splitedBlocksize;
    struct MallocMtadata* temp;
    orders[order] = block->next_order;
    block->next_order->prev_order = nullptr;
    while (splitValue > 0){
        splitedBlocksize = pow(2, order-1)*128;
        
        block->size = splitedBlocksize;
        block->next_order = nullptr;
        temp = block->next;
        block->next = (struct MallocMtadata*) block + splitedBlocksize;
        block->next->is_free = true;
        block->next->next = temp;
        addFreeBlockToOrders(order-1, block->next);
        block->next->prev = block;
        block->next->size = splitedBlocksize;
        order--;
        splitValue--;
    }
    block->size = pow(2, order)*128;
}


struct MallocMtadata* getFreeBlock(size_t size){
    int order = getOrder(size + _size_meta_data());
    int splitValue = 0;
    while (!orders[order]){ // should while stop? need to wait for piazza answer.
        order++;
        splitValue++;
        if (order == 11){
            return NULL;
        }
    }
    struct MallocMtadata* block = orders[order];
    split(splitValue, order, block);
    block->order = (unsigned char)order;
    block->is_free = false;
    return block + _size_meta_data();
}


void addToMmap(struct MallocMtadata* meta){
    meta->next_order = mmap_ptr;
    mmap_ptr = meta;
}


void* smalloc(size_t size){
    if (!heap_ptr){
        initialHeap();
    }
    if (size == 0 || size > pow(10, 8)){
        return NULL;
    }
    if (size + _size_meta_data() >= ORDER_SIZE(MAX_ORDER)){
        struct MallocMtadata* meta = (struct MallocMtadata*) mmap(NULL, size + _size_meta_data(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        meta->size = size; //check
        meta->is_free = false;
        addToMmap(meta);
        return meta + _size_meta_data();
    }
    return getFreeBlock(size);
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


/*
void outOfOrder(struct MallocMtadata* meta){ // gets meta and removes it from orders  // fix!
    struct MallocMtadata* temp = orders[meta->order];
    if (!temp){
        return;
    }
    while(temp && temp != meta){
        temp = temp->next_order;
    }
    if (!temp){
        return;
    }
    if (temp->prev_order && temp->next_order){
        temp->prev_order->next_order = temp->next_order;
        temp->next_order->prev_order = temp->prev_order;
        return;
    }
    if (temp->prev_order){
        temp->prev_order->next_order = nullptr;
        return;
    }
    if (temp->next_order){
        temp->next_order->prev_order = nullptr;
        return;
    }
    orders[meta->order] = nullptr;
}
*/


void outOfOrder(struct MallocMtadata* meta, bool reg){ // gets meta and removes it from orders  // fix!
    if (meta->prev_order){
        meta->prev_order->next_order = meta->next_order;
    }
    else{
        if (reg){
            orders[meta->order] = meta->next_order;
        }
        else{
            mmap_ptr = meta->next_order;
        }
    }
    if (meta->next_order){
        meta->next_order->prev_order = meta->prev_order;
    }
}


void merge(struct MallocMtadata* meta){
    size_t size = meta->size + _size_meta_data();
    struct MallocMtadata* buddy = (struct MallocMtadata*)((size_t)meta^size);
    while (size < 128*1024 && size == buddy->size + _size_meta_data() && buddy->is_free){
        outOfOrder(buddy, true);
        if (meta < buddy){ // p before buddy
            meta->next = buddy->next;
        }
        else{ // buddy before p
            buddy->next = meta->next;
            meta = buddy;
        }
        meta->size *= 2;
        meta->size += _size_meta_data();
        meta->is_free = true;
        size = meta->size + _size_meta_data();
        buddy = (struct MallocMtadata*)((size_t)meta^meta->size);
    }
}


void sfree(void* p){ //free mmap!
    if (!p){
        return;
    }
    struct MallocMtadata* meta = (struct MallocMtadata*)p - _size_meta_data();
    if (meta->is_free){
        return;
    }
    if (meta->size + _size_meta_data() >= ORDER_SIZE(MAX_ORDER)){
        outOfOrder(meta, false);
        munmap(meta, meta->size + _size_meta_data());
        return;
    }
    meta->is_free = true;
    outOfOrder(meta, true);
    merge(meta);
}


bool checkMergeRealloc(struct MallocMtadata* meta, size_t target_size){
    size_t size = meta->size + _size_meta_data();
    struct MallocMtadata* buddy = (struct MallocMtadata*)((size_t)meta^size);
    while (size < ORDER_SIZE(MAX_ORDER) && size == buddy->size + _size_meta_data() && buddy->is_free){
        if (meta >= buddy){ // buddy before p
            meta = buddy;
        }
        if (size >= target_size){
            return true;
        }
        size *= 2;
        buddy = (struct MallocMtadata*)((size_t)meta^meta->size);
    }
    return false;
}


void* sreallocMerge(struct MallocMtadata* meta, size_t target_size){
    size_t size = meta->size + _size_meta_data();
    struct MallocMtadata* buddy = (struct MallocMtadata*)((size_t)meta^size);
    while (size < 128*1024 && size == buddy->size + _size_meta_data() && buddy->is_free && target_size > size){
        outOfOrder(buddy, true);
        if (meta < buddy){ // p before buddy
            meta->next = buddy->next;
        }
        else{ // buddy before p
            buddy->next = meta->next;
            meta = buddy;
        }
        meta->size *= 2;
        meta->size += _size_meta_data();
        meta->is_free = true;
        size = meta->size + _size_meta_data();
        buddy = (struct MallocMtadata*)((size_t)meta^meta->size);
    }
    return meta + _size_meta_data();
}


void* srealloc(void* oldp, size_t size){
    if (size == 0 || size > pow(10, 8)){
        return NULL;
    }
    if (!oldp){
        // handle
    }
    void* p;
    struct MallocMtadata* oldMeta = (struct MallocMtadata*)((size_t)oldp - _size_meta_data());
    if (oldMeta->size >= size){
        return oldp;
    }
    if (checkMergeRealloc(oldMeta, size)){
        p = sreallocMerge(oldMeta, size);
        p = (void*)((size_t)p + _size_meta_data());
        memcpy(p, oldp, oldMeta->size);
        sfree(oldp);
        return p;
    }
    p = smalloc(size);
    memcpy(p, oldp, oldMeta->size);
    sfree(oldp);
    return p;
}
