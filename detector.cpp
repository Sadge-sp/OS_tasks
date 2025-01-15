#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
using namespace std;

// генерация случайного числа (взята из интернета)
double generateRandomNumber(double min, double max) {
    return min + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (max - min)));
}

void updateFile(const string& file) {
    ifstream inFile(file);
    double cvalue;
    inFile >> cvalue;
    inFile.close();
    double newvalue;

    do {
        newvalue = cvalue + generateRandomNumber(-0.5, 0.5);
    } while (newvalue < -30 || newvalue > 30);

    ofstream outFile(file);
    outFile << newvalue;
    outFile.close();
    
}

int main() {
    srand(static_cast<unsigned int>(time(0))); 
    const string file = "detector.txt";

    double initialValue = generateRandomNumber(-30, 30);
    ofstream outputFile(file);
    outputFile << initialValue;
    outputFile.close();
    

    while (true) {
        this_thread::sleep_for(chrono::seconds(30));
        updateFile(file);
    }

    return 0;
}