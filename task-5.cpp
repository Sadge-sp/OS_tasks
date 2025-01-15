#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <sstream>
#include <cmath>
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Thread.h"
#include "Poco/Mutex.h"
#include "Poco/Exception.h"
#include <sqlite3.h>
#include <fstream>

using namespace std;
using namespace Poco;
using namespace Poco::Net;

mutex log_mutex;
sqlite3* db; // Указатель на базу данных

// Функция для логирования температуры в базу данных
void logTemp(float temp) {
    lock_guard<mutex> lock(log_mutex);
    char* errMsg;
    string sql = "INSERT INTO temperature_logs (temperature, timestamp) VALUES (" + to_string(temp) + ", datetime('now'));";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (errMsg) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
    }
}

// Функция для вычисления средней температуры за час
void hour_average(vector<float>& hourtemp) {
    lock_guard<mutex> lock(log_mutex);
    if (!hourtemp.empty()) {
        float sum = 0;
        for (float temp : hourtemp) {
            sum += temp;
        }
        float average = sum / hourtemp.size();
        char* errMsg;
        string sql = "INSERT INTO hourly_average (average_temperature, timestamp) VALUES (" + to_string(average) + ", datetime('now'));";
        sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
        if (errMsg) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
        hourtemp.clear();
    }
}

// Функция для вычисления средней температуры за день
void Day_average(vector<float>& dailytemp) {
    lock_guard<mutex> lock(log_mutex);
    if (!dailytemp.empty()) {
        float sum = 0;
        for (float temp : dailytemp) {
            sum += temp;
        }
        float average = sum / dailytemp.size();
        char* errMsg;
        string sql = "INSERT INTO daily_average (average_temperature, timestamp) VALUES (" + to_string(average) + ", datetime('now'));";
        sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
        if (errMsg) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
        dailytemp.clear();
    }
}

// Функция для чтения температуры из файла
float read_detector(const string& filename) {
    ifstream file(filename);
    float temperature;
    file >> temperature;
    file.close();
    return temperature;
}

// Обработчик HTTP-запросов
class TemperatureRequestHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        string responseBody;
        if (request.getURI() == "/temperature") {
            lock_guard<mutex> lock(log_mutex);
            // Получаем последнюю температуру из базы данных
            string sql = "SELECT temperature FROM temperature_logs ORDER BY timestamp DESC LIMIT 1;";
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                float current_temp = sqlite3_column_double(stmt, 0);
                responseBody = "Current Temperature: " + to_string(current_temp);
                response.setStatus(HTTPResponse::HTTP_OK);
            } else {
                responseBody = "No data available.";
                response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            }
            sqlite3_finalize(stmt);
        } else {
            responseBody = "Invalid Request.";
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
        }

        response.setContentType("text/plain");
        response.setContentLength(responseBody.size());
        response.sendBuffer(responseBody.data(), responseBody.size());
    }
};

// Фабрика обработчиков HTTP-запросов
class TemperatureRequestHandlerFactory : public HTTPRequestHandlerFactory {
public:
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request) override {
        return new TemperatureRequestHandler;
    }
};

// Функция для запуска сетевого сервера
void startServer(void*) {
    ServerSocket svs(8080); // Порт 8080
    HTTPServer srv(new TemperatureRequestHandlerFactory, svs, new HTTPServerParams);
    srv.start();
    cout << "Server started on port 8080." << endl;

    // Ожидание завершения работы сервера
    while (true) {
        this_thread::sleep_for(chrono::seconds(1));
    }

    srv.stop();
}

int main() {
    // Инициализация базы данных
    sqlite3_open("temperature.db", &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS temperature_logs (id INTEGER PRIMARY KEY, temperature REAL, timestamp TEXT);", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS hourly_average (id INTEGER PRIMARY KEY, average_temperature REAL, timestamp TEXT);", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS daily_average (id INTEGER PRIMARY KEY, average_temperature REAL, timestamp TEXT);", nullptr, nullptr, nullptr);

    // Запуск сетевого сервера в отдельном потоке
    Thread serverThread;
    serverThread.start(startServer, nullptr); // Передаем nullptr как аргумент

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
            last_h_check = current_time;
        }

        if (chrono::duration_cast<chrono::hours>(current_time - last_d_check).count() >= 24) {
            Day_average(daily_temp);
            last_d_check = current_time;
        }

        this_thread::sleep_for(chrono::seconds(30));
    }

    sqlite3_close(db);
    return 0;
}
