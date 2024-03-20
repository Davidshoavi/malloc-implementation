#include <unistd.h>
#include <cmath>


void* smalloc(size_t size){
    if (size == 0 || size > pow(10, 8)){
        return NULL;
    }
    void* ptr = sbrk(size);
    if (ptr == (void*)-1){
        return NULL;
    }
    return ptr;
}