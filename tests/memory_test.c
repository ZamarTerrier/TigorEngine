#include <Core/e_memory.h>
#include <stdio.h>

#define NUM_PTRS  10
#define NUM_BYTES 16

void heapdump(void)
{
    _HEAPINFO hinfo;
    int heapstatus;
    int numLoops;
    hinfo._pentry = NULL;
    numLoops = 0;
    while((heapstatus = _heapwalk(&hinfo)) == _HEAPOK &&
          numLoops < 100)
    {
        printf("%8s block at %Fp of size %d\n",
               (hinfo._useflag == _USEDENTRY ? "USED" : "FREE"),
               hinfo._pentry, hinfo._size);
        numLoops++;
    }

    switch(heapstatus)
    {
    case _HEAPEMPTY:
        printf("OK - empty heap\n");
        break;
    case _HEAPEND:
        printf("OK - end of heap\n");
        break;
    case _HEAPBADPTR:
        printf("ERROR - bad pointer to heap\n");
        break;
    case _HEAPBADBEGIN:
        printf("ERROR - bad start of heap\n");
        break;
    case _HEAPBADNODE:
        printf("ERROR - bad node in heap\n");
        break;
    }
}

int main(){


    char *buffer;

    heapdump();
    if((buffer = (char *)malloc(59)) != NULL)
    {
        heapdump();
        free(buffer);
    }
    heapdump();

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