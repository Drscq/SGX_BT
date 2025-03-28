#include "DurationLogger.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

DurationLogger::DurationLogger(const std::string& filePath) : filePath(filePath) {
    // check if the directory exists, if not create it
    std::string dirPath = filePath.substr(0, filePath.find_last_of('/'));
    if (mkdir(dirPath.c_str(), 0777) == -1 && errno != EEXIST) {
        std::cerr << "Error creating directory: " << dirPath << std::endl;
    }
    // check the file exists, if not create it
    std::ofstream file(filePath, std::ios::out | std::ios::app);
    if (!file) {
        std::cerr << "Error creating file: " << filePath << std::endl;
    }
    file.close();
}
DurationLogger::DurationLogger() {}

DurationLogger::~DurationLogger() {
    // Optionally, automatically write to file upon destruction
    // writeToFile();
}

void DurationLogger::startTiming(const std::string& name) {
    startTimes[name] = std::chrono::steady_clock::now();
}

void DurationLogger::stopTiming(const std::string& name) {
    auto stopTime = std::chrono::steady_clock::now();
    auto startTimeIt = startTimes.find(name);
    if (startTimeIt != startTimes.end()) {
        durations[name] = stopTime - startTimeIt->second;
    } else {
        std::cerr << "Timing for " << name << " was not started!" << std::endl;
    }
}

void DurationLogger::writeToFile() {
    std::ofstream file(filePath, std::ios::out | std::ios::app);
    if (file.is_open()) {
        for (const auto& pair : durations) {
            auto durationInNanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(pair.second).count();
            file << pair.first << "," << durationInNanoseconds << "ns ";
        }
        file.close();
        durations.clear();
    } else {
        std::cerr << "Failed to open file: " << filePath << std::endl;
    }
}
