#include "DurationLogger.h"
#include <fstream>
#include <iostream>

DurationLogger::DurationLogger(const std::string& filePath) : filePath(filePath) {}
DurationLogger::DurationLogger() {}

DurationLogger::~DurationLogger() {
    // Optionally, automatically write to file upon destruction
    writeToFile();
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
