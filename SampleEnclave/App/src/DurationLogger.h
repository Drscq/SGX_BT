#ifndef DURATIONLOGGER_H
#define DURATIONLOGGER_H

#include <string>
#include <chrono>
#include <map>

class DurationLogger {
public:
    DurationLogger(const std::string& filePath);
    DurationLogger();
    ~DurationLogger();

    void startTiming(const std::string& name);
    void stopTiming(const std::string& name);
    void writeToFile();

private:
    std::string filePath;
    std::map<std::string, std::chrono::steady_clock::duration> durations;
    std::map<std::string, std::chrono::time_point<std::chrono::steady_clock>> startTimes;
};

#endif // DURATIONLOGGER_H
