#include <stdio.h>
#include <stdlib.h>
#include "launch_process.h" 

#ifdef _WIN32
#define process L"hello_world.exe" 
#else
#define process "hello_world.exe" 
#endif

int main() {
#ifdef _WIN32
    printf("Starting  ");
    printf("%ls...\n", process); 
    int exit_code = launch(process);
#else
    printf("Starting %s...\n", process);
    int exit_code = launch(process);
#endif
printf("Exit code received: %d\n", exit_code); 
    getchar(); 
    return 0;
}
