#include <unistd.h>
#include <cmath>
#include <string.h>
#define MAX_ORDER 10


struct MallocMtadata{
    size_t size;
    bool is_free;
    struct MallocMtadata* next;
    struct MallocMtadata* prev;
    struct MallocMtadata* next_order;
    struct MallocMtadata* prev_order;
};


struct MallocMtadata* heap_ptr = nullptr;
struct MallocMtadata* orders[MAX_ORDER+1];



void align_memory(){
    void* ptr = sbrk(0);
    size_t x = ((size_t)ptr) % (32*128*1024);
    sbrk(32*128*1024 - x);
}


void initialHeap(){

    heap_ptr = (struct MallocMtadata*)sbrk(131072*32);
    orders[10] = heap_ptr;
    struct MallocMtadata* temp = heap_ptr;
    struct MallocMtadata* tempPrev;
    for (int i = 0; i<31; i++){
        temp->next = temp + 131072;
        temp->next_order = temp->next;
        temp->size = 131072;
        temp->is_free = true;
        tempPrev = temp;
        temp = temp->next;
        temp->prev = tempPrev;
        temp->prev_order = temp->prev;
    }
    temp->size = 131072;
    temp->is_free = true;
}


int getOrder(size_t size){}


void addFreeBlockToOrders(int order, struct MallocMtadata* block){}


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
    int order = getOrder(size);
    int splitValue = 0;
    while (!orders[order]){ // should while stop? need to wait for piazza answer.
        order++;
        splitValue++;
    }
    struct MallocMtadata* block = orders[order];
    split(splitValue, order, block);
    block->is_free = false;
    return block;
}


void* smalloc(size_t size){
    if (!heap_ptr){
        initialHeap();
    }
    if (size == 0 || size > pow(10, 8)){
        return NULL;
    }
    if (size >= pow(2, MAX_ORDER)*128){
        //use mmap
        return; // return a block!
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


void outOfOrder(struct MallocMtadata* meta){} // gets meta and removes it from orders


void merge(void* p){
    struct MallocMtadata* meta = (struct MallocMtadata*)p;
    size_t size = meta->size;
    struct MallocMtadata* buddy = (struct MallocMtadata*)((size_t)p^size);
    while (size < 128*1024 && size == buddy->size && buddy->is_free){
        outOfOrder(buddy);
        if (p < buddy){ // p before buddy
            meta->next = buddy->next;
        }
        else{ // buddy before p
            buddy->next = meta->next;
            meta = buddy;
        }
        meta->size *= 2;
        meta->is_free = true;
        buddy = (struct MallocMtadata*)((size_t)meta^meta->size);
    }
}


void sfree(void* p){
    if(!p){
        return;
    }
    struct MallocMtadata* meta = (struct MallocMtadata*)p;
    meta->is_free = true;
    outOfOrder(meta);
    merge(p);
}
