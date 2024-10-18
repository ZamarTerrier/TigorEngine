#include <Tools/e_direct.h>

int main(){

    char buffer[256];
	_getcwd(buffer, 256);
    printf("%s\n", buffer);

    char *ext = DirectGetFileExt("Little_Tokyo.glb");

    printf("File extension : %s\n", ext);

    return 0;
}