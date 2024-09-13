#include "Tools/e_direct.h"

#include "Core/e_memory.h"

char* DirectGetCurrectFilePath(){
    
    char *currPath = (char *)AllocateMemory(256, sizeof(char));
	_getcwd(currPath, 256);

    return currPath;
}

int DirectIsFileExist(const char *path){
    FILE *file = fopen(path, "r");
    if (file) {
        fclose(file);
        return true;
    } else {
        #ifndef NDEBUG
        printf("Error! File does not exist : %s\n", path);
        #endif
        return false;
    }
}
