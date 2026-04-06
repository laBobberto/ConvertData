#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <random>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <sstream>
#ifdef __linux__
#include <sys/resource.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif
struct BenchmarkConfig {
    size_t testDataSize = 1000000;
    size_t iterationCount = 1000000;
    int minYear = 1800;
    int maxYear = 3000;
    bool includeEdgeCases = true;
    bool verboseOutput = true;
    bool trackMemory = true;
};
struct MemoryStats {
    size_t peakRssKb = 0;
    size_t currentRssKb = 0;
    void capture() {
#ifdef __linux__
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);
        peakRssKb = usage.ru_maxrss;
        std::ifstream status("/proc/self/status");
        std::string line;
        while (std::getline(status, line)) {
            if (line.substr(0, 6) == "VmRSS:") {
                std::istringstream iss(line.substr(6));
                iss >> currentRssKb;
                break;
            }
        }
#elif defined(_WIN32)
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            peakRssKb = pmc.PeakWorkingSetSize / 1024;
            currentRssKb = pmc.WorkingSetSize / 1024;
        }
#endif
    }
};
struct TestCase {
    struct tm timeStruct;
    int year;
    int dayOfYear;
    std::string description;
};
std::vector<TestCase> generateTestData(const BenchmarkConfig& config) {
    std::vector<TestCase> testCases;
    testCases.reserve(config.testDataSize + 200);
    std::mt19937 gen(42);
    std::uniform_int_distribution<> year_dist(config.minYear, config.maxYear);
    std::uniform_int_distribution<> day_dist(1, 365);
    for (size_t i = 0; i < config.testDataSize; ++i) {
        TestCase tc;
        int year = year_dist(gen);
        int dayOfYear = day_dist(gen);
        int maxDay = (!(year & 3) && ((year % 100) || !(year % 400))) ? 366 : 365;
        if (dayOfYear > maxDay) dayOfYear = maxDay;
        std::memset(&tc.timeStruct, 0, sizeof(struct tm));
        tc.timeStruct.tm_year = year - 1900;
        tc.timeStruct.tm_yday = dayOfYear - 1;
        tc.year = year;
        tc.dayOfYear = dayOfYear;
        tc.description = "Random";
        testCases.push_back(tc);
    }
    return testCases;
}
template<typename Func>
void runBenchmark(const std::string& name, const std::string& opt_level, Func func, const std::vector<TestCase>& testData, const BenchmarkConfig& config) {
    MemoryStats memoryBefore, memoryAfter;
    memoryBefore.capture();
    for (size_t i = 0; i < 10000; ++i) {
        volatile int week = func(testData[i % testData.size()].timeStruct);
        (void)week;
    }
    std::vector<double> times;
    times.reserve(config.iterationCount);
    for (size_t i = 0; i < config.iterationCount; ++i) {
        const auto& testCase = testData[i % testData.size()];
        auto start = std::chrono::high_resolution_clock::now();
        volatile int week = func(testCase.timeStruct);
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double, std::nano>(end - start).count());
        (void)week;
    }
    memoryAfter.capture();
    std::sort(times.begin(), times.end());
    double sum = 0.0;
    for (double t : times) sum += t;
    double avg = sum / times.size();
    std::cout << name << "," << opt_level << "," << avg << "," << times[times.size()/2] << "," 
              << times.front() << "," << times.back() << ","
              << times[static_cast<size_t>(times.size() * 0.95)] << ","
              << times[static_cast<size_t>(times.size() * 0.99)] << "\n";
}
