#include "Variabels/engine_includes.h"

void InitMemory();
void *AllocateMemory(int32_t count, int32_t size);
void *AllocateMemoryP(int32_t count, int32_t size, void *parent);
uint32_t GetAllocatedMemoryCount();
void FreeMemory(void *data);
void ClearAllAllocatedMemory();