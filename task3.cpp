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
using namespace std;
ofstream logF;
bool running = false;

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
void logCurrentTime() {
    while (running) {
        this_thread::sleep_for(chrono::seconds(1));
        time_t now = time(nullptr);
        tm* time = localtime(&now);
        int counterV = counter();
        logF << put_time(time, "%Y-%m-%d %H:%M:%S") << " PID: " 
                << getpid() << " Counter: " << counterV << endl;
    }
}

void timer() {
    while (running) {
        this_thread::sleep_for(chrono::seconds(3));
        int Counter = counter();
        wcounter(Counter + 1);
    }
}



int main() {
    wcounter(0);

    logF.open("log.txt", ios::app);

    time_t sTime = time(nullptr);
    logF << "MainProcess ID: " << getpid() << " Start Time: " 
            << ctime(&sTime);

    running = true;

    thread tThread(timer);
    thread lThread(logCurrentTime);

    string input;
    while (running) {
        cout << "Enter a new counter value (or 'exit' to quit): ";
        cin >> input;
        if (input == "exit") {
            running = false;
        } else {
            stringstream ss(input);
            int newValue;
            if (ss >> newValue) {
                wcounter(newValue);
            } else {
                cout << "Invalid input. Please enter a number." << endl;
            }
        }

        static auto lastSpawnTime = chrono::steady_clock::now();
        if (chrono::steady_clock::now() - lastSpawnTime >= chrono::seconds(3)) {
        system ("copy1.exe");
        system ("copy2.exe");
        }
    }

    running = false; 
    tThread.join();
    lThread.join();

    logF.close();
    return 0;
}
