#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <sstream> 
#include <cmath>

using namespace std;
mutex log_mutex;

void logTemp(float temp) {
    lock_guard<mutex> lock(log_mutex);
    ofstream logF("temperature_log.txt", ios::app);
    if (logF.is_open()) {
        auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        logF << put_time(localtime(&now), "%Y-%m-%d %H:%M:%S") << " - " << temp << "\n"; // Fixed newline character
    }
}

void cleanRecord(const string& file, chrono::hours period) {
    ifstream inFile(file);
    ofstream tempFile("temp_log.txt");
    string line;
    auto now = chrono::system_clock::now();

    while (getline(inFile, line)) {
        tm tm = {};
        istringstream ss(line.substr(0, 19)); 
        ss >> get_time(&tm, "%Y-%m-%d %H:%M:%S");
        auto recordTime = chrono::system_clock::from_time_t(mktime(&tm));

        if (now - recordTime < period) {
            tempFile << line << "\n"; 
        }
    }

    inFile.close();
    tempFile.close();
    remove(file.c_str()); 
    rename("temp_log.txt", file.c_str()); 
}

void hour_average(vector<float>& hourtemp) {
    lock_guard<mutex> lock(log_mutex);
    if (!hourtemp.empty()) {
        float sum = 0;
        for (float temp : hourtemp) {
            sum += temp;
        }
        float average = sum / hourtemp.size();
        ofstream hourlyLog("hourly_average_log.txt", ios::app);
        if (hourlyLog.is_open()) {
            auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
            hourlyLog << put_time(localtime(&now), "%Y-%m-%d %H:%M:%S") << " - " << average << "\n"; // Fixed newline character
        }
        hourtemp.clear();
    }
}

void Day_average(vector<float>& dailytemp) {
    lock_guard<mutex> lock(log_mutex);
    if (!dailytemp.empty()) {
        float sum = 0;
        for (float temp : dailytemp) {
            sum += temp;
        }
        float average = sum / dailytemp.size();
        ofstream dailyLog("daily_average_log.txt", ios::app);
        if (dailyLog.is_open()) {
            auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
            dailyLog << put_time(localtime(&now), "%Y-%m-%d") << " - " << average << "\n"; 
        }
        dailytemp.clear();
    }
}

float read_detector(const string& filename) {
    ifstream file(filename);
    float temperature;
    file >> temperature;
    file.close();
    return temperature;
}

int main() {
    vector<float> hour_temp;
    vector<float> daily_temp;

    auto start_time = chrono::system_clock::now();
    auto last_h_check = start_time;
    auto last_d_check = start_time;

    while (true) {
        float temperature = read_detector("detector.txt");
        logTemp(temperature);
        
        if (!isnan(temperature)) {  
            hour_temp.push_back(temperature);
            daily_temp.push_back(temperature);
        }

        auto current_time = chrono::system_clock::now();
        if (chrono::duration_cast<chrono::hours>(current_time - last_h_check).count() >= 1) {
            hour_average(hour_temp);
            cleanRecord("hourly_average_log.txt", chrono::hours(720));
            last_h_check = current_time;
        }

        if (chrono::duration_cast<chrono::hours>(current_time - last_d_check).count() >= 24) {
            Day_average(daily_temp);
            cleanRecord("daily_average_log.txt", chrono::hours(24 * 365));
            last_d_check = current_time;
        }

        this_thread::sleep_for(chrono::seconds(30)); 
    }

    return 0; 
}
