// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <iostream>
#include <fstream>
#include <atomic>

extern std::atomic<int> counter; // Counter variable
extern std::ofstream logFile;    // Log file stream

#endif // GLOBALS_H