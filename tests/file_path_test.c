#include <direct.h>

int main(){

    char buffer[256];
	_getcwd(buffer, 256);
    printf("%s\n", buffer);

    return 0;
}