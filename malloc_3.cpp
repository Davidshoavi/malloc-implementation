#include <unistd.h>
#include <cmath>
#include <string.h>
#include <sys/mman.h>
#define MAX_ORDER 10
#define ORDER_SIZE(i) (size_t)pow(2, i)*128

/*
//#include <stddef.h>
//#include <unistd.h>
//#include <sys/wait.h>
//#include <unistd.h>
size_t _num_free_bytes();
size_t _num_free_blocks();
size_t _num_allocated_bytes();
size_t _num_overall_bytes();
size_t _num_allocated_blocks();
size_t _num_meta_data_bytes();
size_t _size_meta_data();
#define MAX_ALLOCATION_SIZE (1e8)
#define MMAP_THRESHOLD (128 * 1024)
#define MIN_SPLIT_SIZE (128)
#define MAX_ELEMENT_SIZE (128*1024)
#include <assert.h>
#define REQUIRE(bool) assert(bool)
#define verify_blocks(allocated_blocks, allocated_bytes, free_blocks, free_bytes)                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        REQUIRE(_num_allocated_blocks() == allocated_blocks);                                                          \
        REQUIRE(_num_allocated_bytes() == (allocated_bytes));                                              \
        REQUIRE(_num_free_blocks() == free_blocks);                                                                    \
        REQUIRE(_num_free_bytes() == (free_bytes));                                                        \
        REQUIRE(_num_meta_data_bytes() == (_size_meta_data() * allocated_blocks));                         \
    } while (0)

#define verify_size(base)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        void *after = sbrk(0);                                                                                         \
        REQUIRE(_num_allocated_bytes() + aligned_size(_size_meta_data() * _num_allocated_blocks()) ==                  \
                (size_t)after - (size_t)base);                                                                         \
    } while (0)

#define verify_size_with_large_blocks(base, diff)                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        void *after = sbrk(0);                                                                                         \
        REQUIRE(diff == (size_t)after - (size_t)base);                                                                 \
    } while (0)

void verify_block_by_order(int order0free, int order0used, int order1free, int order1used, \
                                int order2free, int order2used,\
                                int order3free, int order3used, \
                                int order4free, int order4used, \
                                int order5free, int order5used, \
                                int order6free, int order6used, \
                                int order7free, int order7used, \
                                int order8free,int  order8used, \
                                int order9free,int  order9used, \
                                int order10free,int  order10used,
                                int big_blocks_count, long big_blocks_size  )\
                                                                                                                     \
    {                                                                                                                  \
        unsigned int __total_blocks = order0free + order0used+ order1free + order1used+ order2free + order2used+ order3free + order3used+ order4free + order4used+ order5free + order5used+ order6free + order6used+ order7free + order7used+ order8free + order8used+ order9free + order9used+ order10free + order10used + big_blocks_count       ;        \
        unsigned int __total_free_blocks = order0free+ order1free+ order2free+ order3free+ order4free+ order5free+ order6free+ order7free+ order8free+ order9free+ order10free ;                     \
        unsigned int __total_free_bytes_with_meta  = order0free*128*pow(2,0) +  order1free*128*pow(2,1) +  order2free*128*pow(2,2) +  order3free*128*pow(2,3) +  order4free*128*pow(2,4) +  order5free*128*pow(2,5) +  order6free*128*pow(2,6) +  order7free*128*pow(2,7) +  order8free*128*pow(2,8) +  order9free*128*pow(2,9)+  order10free*128*pow(2,10) ;                                                                     \
        unsigned int testing_allocated_bytes;
        if (__total_blocks==0) testing_allocated_bytes = 0;
        else testing_allocated_bytes = big_blocks_size+32 * MAX_ELEMENT_SIZE - (__total_blocks-big_blocks_count)*(_size_meta_data());
        verify_blocks(__total_blocks, testing_allocated_bytes, __total_free_blocks,__total_free_bytes_with_meta - __total_free_blocks*(_size_meta_data()));\
    }


*/



struct MallocMtadata{
    size_t size;
    bool is_free;
    struct MallocMtadata* next_order;
    struct MallocMtadata* prev_order;
    unsigned char order;
};


struct MallocMtadata* heap_ptr = nullptr;
struct MallocMtadata* orders[MAX_ORDER+1];
struct MallocMtadata* mmap_ptr = nullptr;


size_t num_free_blocks = 0;
size_t num_free_bytes = 0;
//size_t num_allocated_bytes = 0;
size_t num_allocated_blocks = 0;
size_t num_overall_bytes = 0;


size_t _size_meta_data(){
    return sizeof(struct MallocMtadata);
}

size_t _num_overall_bytes(){
    return num_overall_bytes;
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
    return _num_overall_bytes() - _size_meta_data() * _num_allocated_blocks();
}

size_t _num_meta_data_bytes(){
    return _num_allocated_blocks() * _size_meta_data();
}

void align_memory(){
    void* ptr = sbrk(0);
    size_t x = ((size_t)ptr) % (size_t)(32*ORDER_SIZE(MAX_ORDER));
    if (x != 0){
        sbrk((int)((size_t)(32*ORDER_SIZE(MAX_ORDER)) - x));
    }
}


void initialHeap(){
    align_memory();
    num_free_blocks = 32;
    num_allocated_blocks = 32;
    num_free_bytes = (size_t)32*(ORDER_SIZE(MAX_ORDER) - _size_meta_data());
    num_overall_bytes += ORDER_SIZE(MAX_ORDER)*32;
    heap_ptr = (struct MallocMtadata*)sbrk(32*128*1024);
    orders[10] = heap_ptr;
    struct MallocMtadata* temp = heap_ptr;
    struct MallocMtadata* tempPrev;
    for (int i = 0; i<31; i++){
        temp->next_order = (struct MallocMtadata*)((size_t)temp + (size_t)ORDER_SIZE(MAX_ORDER));
        temp->size = ORDER_SIZE(MAX_ORDER) - _size_meta_data();
        temp->is_free = true;
        tempPrev = temp;
        temp = temp->next_order;
        temp->prev_order = tempPrev;
        temp->order = (unsigned char) 10;
    }
    temp->size = ORDER_SIZE(MAX_ORDER) - _size_meta_data();
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
    while (temp && (size_t)temp < (size_t)block){
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
    if (block->prev_order){
        block->prev_order->next_order = block;
    }
    else{
        orders[order] = block;
    }
    temp->prev_order = block;
}


void split(int splitValue, int order, struct MallocMtadata* block){ // block is the first block on the list!
    int splitedBlocksize;
    struct MallocMtadata* buddy;
    orders[order] = block->next_order;
    if (block->next_order){
        block->next_order->prev_order = nullptr;
    }
    while (splitValue > 0){
        num_free_blocks++;
        num_allocated_blocks++;
        num_free_bytes -= _size_meta_data();

        splitedBlocksize = ORDER_SIZE(order-1);
        block->size = (size_t)splitedBlocksize - _size_meta_data();
        block->next_order = nullptr;
        buddy = (struct MallocMtadata*)((size_t)block + (size_t)splitedBlocksize);
        buddy->is_free = true;
        buddy->order = (unsigned char)order-1;
        addFreeBlockToOrders(order-1, buddy);
        buddy->size = (size_t)splitedBlocksize - _size_meta_data();
        order--;
        splitValue--;
    }
    //block->size = pow(2, order)*128;
}


struct MallocMtadata* getFreeBlock(size_t size){
    int order = getOrder(size + _size_meta_data());
    int orig_order = order;
    int splitValue = 0;
    while (!orders[order]){
        order++;
        splitValue++;
        if (order == 11){
            return NULL;
        }
    }
    struct MallocMtadata* block = orders[order];
    split(splitValue, order, block);
    block->order = (unsigned char)orig_order;
    block->is_free = false;
    num_free_blocks--;
    num_free_bytes -= block->size;
    return (struct MallocMtadata*)((size_t)block + _size_meta_data());
}


void addToMmap(struct MallocMtadata* meta){
    meta->next_order = mmap_ptr;
    mmap_ptr = meta;
    if (meta->next_order){
        meta->next_order->prev_order = meta;
    }
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
        num_allocated_blocks++;
        num_overall_bytes += size + _size_meta_data();
        return (void*)((size_t)meta + _size_meta_data());
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


void outOfOrder(struct MallocMtadata* meta, bool reg){ // gets meta and removes it from orders
    if (meta->prev_order){
        meta->prev_order->next_order = meta->next_order;
    }
    else{
        if (reg){
            orders[meta->order] = meta->next_order;
        }
        else{
            mmap_ptr = meta->next_order; // fix
        }
    }
    if (meta->next_order){
        meta->next_order->prev_order = meta->prev_order;
    }
}


void merge(struct MallocMtadata* meta){
    size_t size = meta->size + _size_meta_data();
    struct MallocMtadata* buddy = (struct MallocMtadata*)(void*)((size_t)meta^size);
    outOfOrder(meta, true);
    while (size < 128*1024 && size == buddy->size + _size_meta_data() && buddy->is_free){
        outOfOrder(buddy, true);
        if (meta >= buddy){ // p before buddy
            meta = buddy;
        }
        num_free_blocks--;
        num_allocated_blocks--;
        num_free_bytes += _size_meta_data();
        meta->size *= (size_t)2;
        meta->size += _size_meta_data();
        meta->is_free = true;
        meta->order = (unsigned char)((int)meta->order + 1);
        size = meta->size + _size_meta_data();
        buddy = (struct MallocMtadata*)((size_t)meta^size);
    }
    addFreeBlockToOrders(meta->order, meta);
}


void sfree(void* p){ //free mmap!
    if (!p){
        return;
    }
    struct MallocMtadata* meta = (struct MallocMtadata*)((size_t)p - _size_meta_data());
    if (meta->is_free){
        return;
    }
    if (meta->size + _size_meta_data() > (size_t)ORDER_SIZE(MAX_ORDER)){
        num_allocated_blocks--;
        num_overall_bytes -= meta->size + _size_meta_data();
        outOfOrder(meta, false);
        munmap(meta, meta->size + _size_meta_data());
        return;
    }
    meta->is_free = true;
    num_free_blocks++;
    num_free_bytes += meta->size;
    addFreeBlockToOrders(meta->order, meta);
    merge(meta);
}


bool checkMergeRealloc(struct MallocMtadata* meta, size_t target_size){
    size_t size = meta->size + _size_meta_data();
    struct MallocMtadata* buddy = (struct MallocMtadata*)((size_t)meta^size);
    while (size < ORDER_SIZE(MAX_ORDER) && size == buddy->size + _size_meta_data() && buddy->is_free){
        if (meta >= buddy){ // buddy before meta
            meta = buddy;
        }
        if (size >= target_size){
            return true;
        }
        size *= 2;
        buddy = (struct MallocMtadata*)((size_t)meta^size);
    }
    return false;
}


void* sreallocMerge(struct MallocMtadata* meta, size_t target_size){
    size_t size = meta->size + _size_meta_data();
    struct MallocMtadata* buddy = (struct MallocMtadata*)((size_t)meta^size);
    outOfOrder(meta, true);
    while (size < 128*1024 && size == buddy->size + _size_meta_data() && buddy->is_free && target_size > size){
        outOfOrder(buddy, true);
        if (meta >= buddy){ // buddy before meta
            meta = buddy;
        }
        num_free_blocks--;
        num_allocated_blocks--;
        num_free_bytes += _size_meta_data();
        meta->size *= 2;
        meta->size += _size_meta_data();
        meta->is_free = true;
        size = meta->size + _size_meta_data();
        buddy = (struct MallocMtadata*)((size_t)meta^size);
    }
    return meta + _size_meta_data();
}


void* srealloc(void* oldp, size_t size){
    if (size == 0 || size > pow(10, 8)){
        return NULL;
    }
    if (!oldp){
        return smalloc(size);
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
        return p;
    }
    p = smalloc(size);
    memcpy(p, oldp, oldMeta->size);
    sfree(oldp);
    return p;
}



/*
int main(){
    //std::vector<void*> allocations;
    void* allocations[64];
    int i=1;
    for (; i < 64; i++)
    {
        void* ptr = smalloc(MAX_ELEMENT_SIZE+100);
        REQUIRE(ptr != nullptr);
        allocations[i] = ptr;
        //allocations.push_back(ptr);
        verify_block_by_order(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32,0,i,i*(MAX_ELEMENT_SIZE+100));
    }

    for (i = 63; i >= 1; i--)
    {
        void* ptr = allocations[i];
        allocations[i] = nullptr;
        sfree(ptr);
        verify_block_by_order(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32,0,i-1,(i-1)*(MAX_ELEMENT_SIZE+100));
    }

    // Verify that all blocks are merged into a single large block
    verify_block_by_order(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0);


    for (i = 1; i < 64; i++)
    {
        void* ptr = smalloc(MAX_ELEMENT_SIZE+100);
        REQUIRE(ptr != nullptr);
        allocations[i] = ptr;
        //allocations.push_back(ptr);
        verify_block_by_order(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32,0,i,i*(MAX_ELEMENT_SIZE+100));
    }

    int j = 63;
    for (i = 1; i < 64; i++)
    {
        void* ptr = allocations[i];
        allocations[i] = nullptr;
        sfree(ptr);
        j--;
        verify_block_by_order(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32,0,j,j*(MAX_ELEMENT_SIZE+100));
    }
    verify_block_by_order(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0);
}

*/

