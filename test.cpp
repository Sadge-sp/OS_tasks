#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <stdlib.h>
#include <sstream> 
#ifdef _WIN32
#include <windows.h>
#include <process.h> 
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif
void createChildProcess1() {
system ("copy1.exe");
}
main(){
    createChildProcess1();
}