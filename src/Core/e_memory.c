#include "Core/e_memory.h"

ChildStack *alloc_memory_head = NULL;

typedef enum{
    TIGOR_ALLOC_TYPE_DATA,
    TIGOR_ALLOC_TYPE_BUFFER,
    TIGOR_ALLOC_TYPE_PIPELINE,
    TIGOR_ALLOC_TYPE_DESCRIPTOR
} AllocType;

typedef struct{
    uint32_t size;
    uint32_t count;
    void *parent;
    char *name;
    void *ptr;
} AllocObj;

void InitMemory(){
    alloc_memory_head = (ChildStack *)calloc(1, sizeof(ChildStack));
}

int32_t alloc_counter = 0;


void *MakeAlloc(AllocObj *obj){
    
    void *data = calloc(obj->count, obj->size);

    if(alloc_memory_head == NULL)
        InitMemory();

    obj->ptr = data;

    if(alloc_memory_head->node == NULL){
        alloc_memory_head->next = calloc(1, sizeof(ChildStack));
        alloc_memory_head->node = obj;
    }
    else{

        ChildStack *child = alloc_memory_head->next;

        while(child->next != NULL)
        {
            child = child->next;
        }

        child->next = calloc(1, sizeof(ChildStack));
        child->node = obj;
    }

    alloc_counter ++;

    return data;
}

void *AllocateMemoryN(int32_t count, int32_t size, char *name){

    AllocObj *obj = calloc(1, sizeof(AllocObj));
    obj->size = size;
    obj->count = count;

    uint32_t len = strlen(name);
    obj->name = calloc(len + 1, sizeof(char));
    memcpy(obj->name, name, sizeof(char) * len);
    obj->name[len] = '\0';

    return MakeAlloc(obj);
}

void *AllocateMemoryP(int32_t count, int32_t size, void *parent){

    AllocObj *obj = calloc(1, sizeof(AllocObj));
    obj->size = size;
    obj->count = count;
    obj->parent = parent;

    return MakeAlloc(obj);
}

void *AllocateMemory(int32_t count, int32_t size){

    AllocObj *obj = calloc(1, sizeof(AllocObj));
    obj->size = size;
    obj->count = count;
    
    return MakeAlloc(obj);
}

uint32_t GetAllocatedMemoryCount(){
    
    ChildStack *child = alloc_memory_head;

    uint32_t count = 0;

    while(child->next != NULL)
    {
        child = child->next;

        count ++;
    }

    return count;
}

void FreeMemory(void *data){

    if(data == NULL)
        return;
    
    void *curr = NULL;

    ChildStack *child = alloc_memory_head;
    ChildStack *before = NULL;
    AllocObj *obj = NULL;

    while(child != NULL)
    {
        curr = child->node;
        
        if(curr != NULL){
            obj = curr;
            if(obj->ptr == data)
                break;
        }else
            break;

        before = child;
        child = child->next;
    }

    if(curr == NULL){
        printf("Can't find that memory : 0x%x\n", data);
        return;
    }

    if(child->next != NULL)
    {
        obj = child->node;
        free(obj->ptr);
        if(obj->name != NULL)
            free(obj->name);
        free(child->node);
        child->node = NULL;

        if(before != NULL)
            before->next = child->next;
        else
            alloc_memory_head = child->next;

        free(child);
        child = NULL;
    }
}

void ClearAllAllocatedMemory(){
    ChildStack *child = alloc_memory_head;
    ChildStack *before = NULL;
    AllocObj *obj = NULL;
    
    uint32_t counter = 0;
    
    if(child == NULL)
        return;

#ifndef NDEBUG
    printf("Allocating times : %i\n", alloc_counter);
    printf("Allocated count : %i\n", GetAllocatedMemoryCount()); 
#endif   

    while(child != NULL)
    {
        if(child->node != NULL)
            counter ++;

        if(child->node != NULL){
            obj = child->node;
            free(obj->ptr);
            if(obj->name != NULL){
                #ifndef NDEBUG
                printf("Object not free with name : %s\n", obj->name);
                #endif   

                free(obj->name);
            }
            free(child->node);
            child->node = NULL;
        }

        child = alloc_memory_head = child->next;

        free(before);
        before = NULL;
    }

    if(alloc_memory_head != NULL)
        free(alloc_memory_head);

    if(counter > 0)
        printf("Auto free allocated buffers count : %i\n", counter);
}