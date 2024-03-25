#include <unistd.h>
#include <cmath>
#include <string.h>


struct MallocMtadata{
    size_t size;
    bool is_free;
    struct MallocMtadata* next;
    struct MallocMtadata* prev;
};


struct MallocMtadata* heap_ptr = nullptr;


void initialHeap(){
    heap_ptr = (struct MallocMtadata*)sbrk(13172);
    heap_ptr->size = 13172;
    heap_ptr->is_free = true;
    struct MallocMtadata* temp = heap_ptr;
    struct MallocMtadata* tempPrev;
    for (int i = 1; i<32; i++){
        temp->next = (struct MallocMtadata*)sbrk(13172);
        temp->size = 13172;
        temp->is_free = true;
        tempPrev = temp;
        temp = temp->next;
        temp->prev = tempPrev;
    }
}