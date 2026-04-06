#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <random>
#include <ctime>
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

// ============================================================================
// TEST CONFIGURATION
// ============================================================================
struct BenchmarkConfig {
#ifdef __linux__
    size_t testDataSize = 50000000;           // Number of test cases
    size_t iterationCount = 1000000000;
#endif
#ifdef _WIN32
    size_t testDataSize = 1000000;
    size_t iterationCount = 1000000;
#endif

    int minYear = 1800;                          // Minimum year
    int maxYear = 3000;                         // Maximum year
    bool includeEdgeCases = true;               // Include boundary cases
    bool verboseOutput = true;                  // Detailed output
    bool trackMemory = true;                    // Memory tracking
};

// ============================================================================
// MEMORY TRACKING
// ============================================================================
struct MemoryStats {
    size_t peakRssKb = 0;                          // Peak RSS (working set) (KB)
    size_t currentRssKb = 0;                       // Current RSS (KB)
    size_t stackUsageBytes = 0;                    // Estimated stack usage (bytes)
    size_t heapUsageKb = 0;                        // Heap usage (KB)

    void capture() {
#ifdef __linux__
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);
        peakRssKb = usage.ru_maxrss; // KB on Linux

        // /proc/self/status for current memory
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

// ============================================================================
// ISO 8601 WEEK DATE CONVERSION FUNCTIONS
// ============================================================================

// Original version
int convertGregorianDateToWeekDate_Original(const struct tm& times) noexcept
{
    const int y = times.tm_year + 1900;
    const int dayOfYearNumber = times.tm_yday + 1;

    const int yy = (y - 1) % 100;
    const int c = (y - 1) - yy;
    const int g = yy + yy / 4;
    const int jan1Weekday = 1 + (((((c / 100) % 4) * 5) + g) % 7);

    const int h = dayOfYearNumber + (jan1Weekday - 1);
    const int weekday = 1 + ((h - 1) % 7);

    int yearNumber = 0, weekNumber = 0;

    if ((dayOfYearNumber <= (8 - jan1Weekday)) && (jan1Weekday > 4))
    {
        yearNumber = y - 1;
        weekNumber = ((jan1Weekday == 5) || ((jan1Weekday == 6) &&
            (!(yearNumber & 3) && ((yearNumber % 100) || !(yearNumber % 400))))) ? 53 : 52;
    }
    else
    {
        yearNumber = y;
        const int daysInYear = (!(y & 3) && ((y % 100) || !(y % 400))) ? 366 : 365;

        if ((daysInYear - dayOfYearNumber) < (4 - weekday))
        {
            yearNumber = y + 1;
            weekNumber = 1;
        }
    }

    if (yearNumber == y)
    {
        const int j = dayOfYearNumber + (7 - weekday) + (jan1Weekday - 1);
        weekNumber = j / 7;
        if (jan1Weekday > 4)
            weekNumber--;
    }

    return weekNumber;
}

// V1: Early Return
int convertGregorianDateToWeekDate_V1(const struct tm& times) noexcept
{
    const int y = times.tm_year + 1900;
    const int dayOfYear = times.tm_yday + 1;

    const int yy = (y - 1) % 100;
    const int c = (y - 1) - yy;
    const int g = yy + yy / 4;
    const int jan1Weekday = 1 + (((((c / 100) % 4) * 5) + g) % 7);

    const int h = dayOfYear + (jan1Weekday - 1);
    const int weekday = 1 + ((h - 1) % 7);

    if ((dayOfYear <= (8 - jan1Weekday)) && (jan1Weekday > 4))
    {
        const int y_1 = y - 1;
        return ((jan1Weekday == 5) || ((jan1Weekday == 6) && (!(y_1 & 3) && ((y_1 % 100) || !(y_1 % 400))))) ? 53 : 52;
    }

    const int daysInYear = (!(y & 3) && ((y % 100) || !(y % 400))) ? 366 : 365;
    if ((daysInYear - dayOfYear) < (4 - weekday))
    {
        return 1;
    }

    const int j = dayOfYear + (7 - weekday) + (jan1Weekday - 1);
    int weekNumber = j / 7;
    if (jan1Weekday > 4)
        weekNumber--;

    return weekNumber;
}

// V2: Bitwise Operations
int convertGregorianDateToWeekDate_V2(const struct tm& times) noexcept
{
    const int y = times.tm_year + 1900;
    const int dayOfYear = times.tm_yday + 1;

    const int y_1 = y - 1;
    const int yy = y_1 % 100;
    const int c = y_1 - yy;
    const int g = yy + (yy >> 2);
    const int jan1Weekday = 1 + (((((c / 100) & 3) * 5) + g) % 7);

    const int weekday = 1 + ((dayOfYear + jan1Weekday - 2) % 7);

    if ((dayOfYear <= (8 - jan1Weekday)) & (jan1Weekday > 4))
    {
        const int prevYearLeap = (!(y_1 & 3) && ((y_1 % 100) || !(y_1 % 400)));
        const int is53 = (jan1Weekday == 5) | ((jan1Weekday == 6) & prevYearLeap);
        return 52 + is53;
    }

    const int daysInYear = 365 + (!(y & 3) && ((y % 100) || !(y % 400)));
    if ((daysInYear - dayOfYear) < (4 - weekday))
    {
        return 1;
    }

    const int j = dayOfYear + (7 - weekday) + (jan1Weekday - 1);
    const int weekNumber = (j / 7) - (jan1Weekday > 4);

    return weekNumber;
}

// V3: Calculation Splitting
int convertGregorianDateToWeekDate_V3(const struct tm& times) noexcept
{
    const int y = times.tm_year + 1900;
    const int dayOfYear = times.tm_yday + 1;
    const int y_1 = y - 1;

    const int prevYearLeap = (!(y_1 & 3) && ((y_1 % 100) || !(y_1 % 400)));
    const int yy = y_1 % 100;
    const int g = yy + (yy >> 2);
    const int c_div_100 = y_1 / 100;
    const int jan1Weekday = 1 + ((((c_div_100 & 3) * 5) + g) % 7);

    const int currYearLeap = (!(y & 3) && ((y % 100) || !(y % 400)));
    const int daysInYear = 365 + currYearLeap;

    const int weekday = 1 + ((dayOfYear + jan1Weekday - 2) % 7);

    if ((dayOfYear <= (8 - jan1Weekday)) & (jan1Weekday > 4))
    {
        const int is53 = (jan1Weekday == 5) | ((jan1Weekday == 6) & prevYearLeap);
        return 52 + is53;
    }

    if ((daysInYear - dayOfYear) < (4 - weekday))
    {
        return 1;
    }

    const int j = dayOfYear + (7 - weekday) + (jan1Weekday - 1);
    const int weekNumber = (j / 7) - (jan1Weekday > 4);

    return weekNumber;
}

// ============================================================================
// TEST DATA STRUCTURES
// ============================================================================
struct TestCase {
    struct tm timeStruct;
    int year;
    int dayOfYear;
    std::string description;
};

// ============================================================================
// TEST DATA GENERATOR
// ============================================================================
std::vector<TestCase> generateTestData(const BenchmarkConfig& config) {
    std::vector<TestCase> testCases;
    testCases.reserve(config.testDataSize + 200);

    std::random_device rd;
    std::mt19937 gen(42);
    std::uniform_int_distribution<> year_dist(config.minYear, config.maxYear);
    std::uniform_int_distribution<> day_dist(1, 365);

    for (size_t i = 0; i < config.testDataSize; ++i) {
        TestCase tc;
        int year = year_dist(gen);
        int dayOfYear = day_dist(gen);

        int maxDay = (!(year & 3) && ((year % 100) || !(year % 400))) ? 366 : 365;
        if (dayOfYear > maxDay) {
            dayOfYear = maxDay;
        }

        std::memset(&tc.timeStruct, 0, sizeof(struct tm));
        tc.timeStruct.tm_year = year - 1900;
        tc.timeStruct.tm_yday = dayOfYear - 1;
        tc.year = year;
        tc.dayOfYear = dayOfYear;
        tc.description = "Random";

        testCases.push_back(tc);
    }

    if (config.includeEdgeCases) {
        std::vector<int> centuries = {1800, 1900, 2000, 2100, 2200};
        for (int cent : centuries) {
            if (cent >= config.minYear && cent <= config.maxYear) {
                TestCase tc1;
                std::memset(&tc1.timeStruct, 0, sizeof(struct tm));
                tc1.timeStruct.tm_year = cent - 1900;
                tc1.timeStruct.tm_yday = 0;
                tc1.year = cent;
                tc1.dayOfYear = 1;
                tc1.description = "Century start: " + std::to_string(cent);
                testCases.push_back(tc1);
            }
        }

        for (int year = config.minYear; year <= config.maxYear; year += 4) {
            if ((!(year & 3) && ((year % 100) || !(year % 400)))) {
                TestCase tc;
                std::memset(&tc.timeStruct, 0, sizeof(struct tm));
                tc.timeStruct.tm_year = year - 1900;
                tc.timeStruct.tm_yday = 59;
                tc.year = year;
                tc.dayOfYear = 60;
                tc.description = "Leap year Feb 29: " + std::to_string(year);
                testCases.push_back(tc);
            }
        }
    }

    return testCases;
}

// ============================================================================
// BENCHMARK RESULT STRUCTURE
// ============================================================================
template<typename Func>
struct BenchmarkResult {
    std::string versionName;
    double averageTimeNs;
    double minTimeNs;
    double maxTimeNs;
    double medianTimeNs;
    double percentile95Ns;
    double percentile99Ns;
    size_t iterations;
    bool correctnessCheck;
    size_t discrepancies;

    // Memory stats
    MemoryStats memoryBefore;
    MemoryStats memoryAfter;
    size_t functionStackBytes;
};

// ============================================================================
// BENCHMARK FUNCTION
// ============================================================================
template<typename Func>
BenchmarkResult<Func> benchmarkFunction(
    const std::string& name,
    Func func,
    const std::vector<TestCase>& testData,
    const BenchmarkConfig& config
) {
    BenchmarkResult<Func> result;
    result.versionName = name;
    result.iterations = config.iterationCount;
    result.discrepancies = 0;

    // Estimate stack usage
    result.functionStackBytes = sizeof(struct tm) + 64; // rough estimate for local variables

    if (config.verboseOutput) {
        std::cout << "  Testing " << name << "..." << std::flush;
    }

    // Capture memory (before)
    if (config.trackMemory) {
        result.memoryBefore.capture();
    }

    // Warm-up
    for (size_t i = 0; i < 10000; ++i) {
        volatile int week = func(testData[i % testData.size()].timeStruct);
        (void)week;
    }

    // Benchmark
    std::vector<double> times;
    times.reserve(config.iterationCount);

#ifdef _WIN32
    // ===============================================================
    // WINDOWS VERSION (Batching due to low timer resolution)
    // ===============================================================
    const int BATCH_SIZE = 1000;
    size_t testDataIndex = 0;

    for (size_t i = 0; i < config.iterationCount; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        for(int j = 0; j < BATCH_SIZE; ++j) {
            const auto& testCase = testData[testDataIndex];
            volatile int week = func(testCase.timeStruct);
            (void)week;

            testDataIndex++;
            if (testDataIndex >= testData.size()) {
                testDataIndex = 0;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        double elapsed_ns = std::chrono::duration<double, std::nano>(end - start).count() / BATCH_SIZE;
        times.push_back(elapsed_ns);
    }
#else
    // ===============================================================
    // LINUX VERSION
    // ===============================================================
    for (size_t i = 0; i < config.iterationCount; ++i) {
        const auto& testCase = testData[i % testData.size()];

        auto start = std::chrono::high_resolution_clock::now();
        volatile int week = func(testCase.timeStruct);
        auto end = std::chrono::high_resolution_clock::now();

        double elapsed_ns = std::chrono::duration<double, std::nano>(end - start).count();
        times.push_back(elapsed_ns);
        (void)week;
    }
#endif

    // Capture memory (after)
    if (config.trackMemory) {
        result.memoryAfter.capture();
    }

    // Statistics
    std::sort(times.begin(), times.end());

    double sum = 0.0;
    for (double t : times) {
        sum += t;
    }

    result.averageTimeNs = sum / times.size();
    result.minTimeNs = times.front();
    result.maxTimeNs = times.back();
    result.medianTimeNs = times[times.size() / 2];
    result.percentile95Ns = times[static_cast<size_t>(times.size() * 0.95)];
    result.percentile99Ns = times[static_cast<size_t>(times.size() * 0.99)];

    // Correctness check
    result.correctnessCheck = true;
    for (const auto& testCase : testData) {
        int res1 = convertGregorianDateToWeekDate_Original(testCase.timeStruct);
        int res2 = func(testCase.timeStruct);
        if (res1 != res2) {
            result.correctnessCheck = false;
            result.discrepancies++;
            if (config.verboseOutput && result.discrepancies <= 5) {
                std::cout << "\n    DISCREPANCY: " << testCase.description
                          << " Year=" << testCase.year
                          << " Day=" << testCase.dayOfYear
                          << " Original=" << res1
                          << " " << name << "=" << res2;
            }
        }
    }

    if (config.verboseOutput) {
        std::cout << " Done!" << std::endl;
    }

    return result;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    BenchmarkConfig config;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--test-size" && i + 1 < argc) {
            config.testDataSize = std::stoull(argv[++i]);
        } else if (arg == "--iterations" && i + 1 < argc) {
            config.iterationCount = std::stoull(argv[++i]);
        } else if (arg == "--year-min" && i + 1 < argc) {
            config.minYear = std::stoi(argv[++i]);
        } else if (arg == "--year-max" && i + 1 < argc) {
            config.maxYear = std::stoi(argv[++i]);
        } else if (arg == "--no-edge-cases") {
            config.includeEdgeCases = false;
        } else if (arg == "--quiet") {
            config.verboseOutput = false;
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  --test-size N       Number of test cases (default: " << config.testDataSize << ")\n"
                      << "  --iterations N      Number of benchmark iterations (default: " << config.iterationCount << ")\n"
                      << "  --year-min YEAR     Minimum year (default: 1800)\n"
                      << "  --year-max YEAR     Maximum year (default: 3000)\n"
                      << "  --no-edge-cases     Disable boundary cases\n"
                      << "  --quiet             Minimal output\n"
                      << "  --help              Show this help\n";
            return 0;
        }
    }

    std::cout << "=== ISO 8601 Week Date Conversion ===" << std::endl;
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Test data size: " << config.testDataSize << std::endl;
    std::cout << "  Benchmark iterations: " << config.iterationCount << std::endl;
    std::cout << "  Year range: " << config.minYear << "-" << config.maxYear << std::endl;
    std::cout << std::endl;

    std::cout << "Generating test data..." << std::endl;
    auto testData = generateTestData(config);
    std::cout << "Generated " << testData.size() << " test cases" << std::endl;
    std::cout << std::endl;

    std::cout << "Running benchmarks..." << std::endl;

    auto result_orig = benchmarkFunction("Original", convertGregorianDateToWeekDate_Original, testData, config);
    auto result_v1 = benchmarkFunction("V1_EarlyReturn", convertGregorianDateToWeekDate_V1, testData, config);
    auto result_v2 = benchmarkFunction("V2_BitOps_", convertGregorianDateToWeekDate_V2, testData, config);
    auto result_v3 = benchmarkFunction("V3_Precalculation", convertGregorianDateToWeekDate_V3, testData, config);

    std::cout << "\n=== PERFORMANCE RESULTS ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);

    auto printResult = [](const auto& r, double baseline_ns) {
        std::cout << "\n" << r.versionName << ":" << std::endl;
        std::cout << "  Performance:" << std::endl;
        std::cout << "    Avg: " << r.averageTimeNs << " ns" << std::endl;
        std::cout << "    Median: " << r.medianTimeNs << " ns" << std::endl;
        std::cout << "    Min:  " << r.minTimeNs << " ns" << std::endl;
        std::cout << "    Max:  " << r.maxTimeNs << " ns" << std::endl;
        std::cout << "    95th:      " << r.percentile95Ns << " ns" << std::endl;
        std::cout << "    99th:      " << r.percentile99Ns << " ns" << std::endl;
        std::cout << "    Speedup: " << (baseline_ns / r.averageTimeNs) << "x" << std::endl;

        std::cout << "  Memory:" << std::endl;
        std::cout << "    Stack:      ~" << r.functionStackBytes << " bytes" << std::endl;
        std::cout << "    RSS Before: " << r.memoryBefore.currentRssKb << " KB" << std::endl;
        std::cout << "    RSS After:  " << r.memoryAfter.currentRssKb << " KB" << std::endl;
        std::cout << "    Peak RSS:   " << r.memoryAfter.peakRssKb << " KB" << std::endl;

        std::cout << "  Correctness: " << (r.correctnessCheck ? "PASS" : "FAIL");
        if (!r.correctnessCheck) {
            std::cout << " (" << r.discrepancies << " discrepancies)";
        }
        std::cout << std::endl;
    };

    printResult(result_orig, result_orig.averageTimeNs);
    printResult(result_v1, result_orig.averageTimeNs);
    printResult(result_v2, result_orig.averageTimeNs);
    printResult(result_v3, result_orig.averageTimeNs);

    std::cout << "\nWriting results to benchmark_analysis.csv..." << std::endl;
    std::ofstream csv("benchmark_analysis.csv");
    csv << "Version,Average_ns,Median_ns,Min_ns,Max_ns,P95_ns,P99_ns,Speedup,"
            << "Stack_Bytes,RSS_Before_KB,RSS_After_KB,Peak_RSS_KB,"
            << "Iterations,Correctness,Discrepancies\n";

    auto writeCSV = [&csv](const auto& r, double baseline_ns) {
        csv << r.versionName << ","
            << r.averageTimeNs << ","
            << r.medianTimeNs << ","
            << r.minTimeNs << ","
            << r.maxTimeNs << ","
            << r.percentile95Ns << ","
            << r.percentile99Ns << ","
            << (baseline_ns / r.averageTimeNs) << ","
            << r.functionStackBytes << ","
            << r.memoryBefore.currentRssKb << ","
            << r.memoryAfter.currentRssKb << ","
            << r.memoryAfter.peakRssKb << ","
            << r.iterations << ","
            << (r.correctnessCheck ? "PASS" : "FAIL") << ","
            << r.discrepancies << "\n";
    };

    writeCSV(result_orig, result_orig.averageTimeNs);
    writeCSV(result_v1, result_orig.averageTimeNs);
    writeCSV(result_v2, result_orig.averageTimeNs);
    writeCSV(result_v3, result_orig.averageTimeNs);

    csv.close();

    std::cout << "\nDone!" << std::endl;

    return 0;
}