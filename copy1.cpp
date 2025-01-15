#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <cstdlib>
#include <thread>

#ifdef _WIN32
#include <process.h> 
#else
#include <unistd.h> 
#endif
using namespace std;
int counter() {
    ifstream counterFile("counter.txt");
    int Value = 0;
    if (counterFile.is_open()) {
        counterFile >> Value;
        counterFile.close();
    }
    return Value;
}

void wcounter(int value) {
    ofstream counterFile("counter.txt");
    if (counterFile.is_open()) {
        counterFile << value;
        counterFile.close();
    }
}


int main() {
    
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);
    ofstream log("log.txt", ios::app);
    log << "Copy_1 ID: " << getpid() << " start sime: " 
                 << put_time(localTime, "%Y-%m-%d %H:%M:%S") << endl;
    log.close();

    int ncounter = counter();
    wcounter(ncounter + 10);

    log.open("log.txt", ios::app);
    log << "Copy_1 counter: " << counter() << "  Copy_1 exiting" << endl;
    log.close();
}