#include <Core/e_memory.h>


int main(){

    void *data1 = AllocateMemory(2, sizeof(uint32_t));
    void *data2 = AllocateMemory(3, sizeof(uint32_t));
    void *data3 = AllocateMemory(4, sizeof(uint32_t));
    void *data4 = AllocateMemory(5, sizeof(uint32_t));
    void *data5 = AllocateMemory(6, sizeof(uint32_t));

    FreeMemory(data2);
    FreeMemory(data4);

    ClearAllAllocatedMemory();

    return 0;
}