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
// НАСТРОЙКИ ТЕСТИРОВАНИЯ
// ============================================================================
struct BenchmarkConfig {
    size_t razmer_testovyh_dannyh = 50000000;           // Количество тестовых случаев
    size_t kolichestvo_iteracij = 1000000000;           // Количество итераций
    int minimalnyj_god = 1800;                          // Минимальный год
    int maksimalnyj_god = 3000;                         // Максимальный год
    bool vklyuchat_granichnye_sluchai = true;           // Включить граничные случаи
    bool podrobnyj_vyvod = true;                        // Подробный вывод
    bool otslezhivat_pamyat = true;                     // Отслеживание памяти
};

// ============================================================================
// ОТСЛЕЖИВАНИЕ ПАМЯТИ
// ============================================================================
struct MemoryStats {
    size_t pikovyj_rss_kb = 0;                          // Пиковый RSS (рабочий набор) (КБ)
    size_t tekushchij_rss_kb = 0;                       // Текущий RSS (КБ)
    size_t ispolzovanie_steka_bajty = 0;                // Оценка использования стека (байты)
    size_t ispolzovanie_kuchi_kb = 0;                   // Использование кучи (КБ)

    void capture() {
#ifdef __linux__
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);
        pikovyj_rss_kb = usage.ru_maxrss; // В линуксе КБ

        //  /proc/self/status для текущей памяти
        std::ifstream status("/proc/self/status");
        std::string line;
        while (std::getline(status, line)) {
            if (line.substr(0, 6) == "VmRSS:") {
                std::istringstream iss(line.substr(6));
                iss >> tekushchij_rss_kb;
                break;
            }
        }
#elif defined(_WIN32)
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            pikovyj_rss_kb = pmc.PeakWorkingSetSize / 1024;
            tekushchij_rss_kb = pmc.WorkingSetSize / 1024;
        }
#endif
    }
};


// ============================================================================
// ФУНКЦИЯ ВЫСОКОСНОГО ГОДА (я вынес отдельно)
// ============================================================================
// inline int isLeapYear(int year) noexcept
// {
//     return (!(year & 3) && ((year % 100) || !(year % 400)));
// }

// ============================================================================
// Оригинал
// ============================================================================
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
        // isLeapYear(yearNumber)
        weekNumber = ((jan1Weekday == 5) || ((jan1Weekday == 6) &&
            (!(yearNumber & 3) && ((yearNumber % 100) || !(yearNumber % 400))))) ? 53 : 52;
    }
    else
    {
        yearNumber = y;
        // isLeapYear(y)
        const int i = (!(y & 3) && ((y % 100) || !(y % 400))) ? 366 : 365;

        if ((i - dayOfYearNumber) < (4 - weekday))
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

// ============================================================================
// Ранний выход
// ============================================================================
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
        // isLeapYear(y - 1)
        const int y_1 = y - 1;
        return ((jan1Weekday == 5) || ((jan1Weekday == 6) && (!(y_1 & 3) && ((y_1 % 100) || !(y_1 % 400))))) ? 53 : 52;
    }

    // isLeapYear(y)
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

// ============================================================================
// Побитовые операции
// ============================================================================
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
        // isLeapYear(y_1)
        const int prevYearLeap = (!(y_1 & 3) && ((y_1 % 100) || !(y_1 % 400)));
        const int is53 = (jan1Weekday == 5) | ((jan1Weekday == 6) & prevYearLeap);
        return 52 + is53;
    }

    // isLeapYear(y)
    const int daysInYear = 365 + (!(y & 3) && ((y % 100) || !(y % 400)));
    if ((daysInYear - dayOfYear) < (4 - weekday))
    {
        return 1;
    }

    const int j = dayOfYear + (7 - weekday) + (jan1Weekday - 1);
    const int weekNumber = (j / 7) - (jan1Weekday > 4);

    return weekNumber;
}

// ============================================================================
// Разделение вычислений
// ============================================================================
int convertGregorianDateToWeekDate_V4(const struct tm& times) noexcept
{
    const int y = times.tm_year + 1900;
    const int dayOfYear = times.tm_yday + 1;
    const int y_1 = y - 1;

    // isLeapYear(y_1)
    const int prevYearLeap = (!(y_1 & 3) && ((y_1 % 100) || !(y_1 % 400)));
    const int yy = y_1 % 100;
    const int g = yy + (yy >> 2);
    const int c_div_100 = y_1 / 100;
    const int jan1Weekday = 1 + ((((c_div_100 & 3) * 5) + g) % 7);

    // isLeapYear(y)
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
// СТРУКТУРЫ ДЛЯ ТЕСТОВЫХ ДАННЫХ
// ============================================================================
struct TestCase {
    struct tm struktura_vremeni;
    int god;
    int den_goda;
    std::string opisanie;
};

// ============================================================================
// ГЕНЕРАТОР ТЕСТОВЫХ ДАННЫХ
// ============================================================================
std::vector<TestCase> generateTestData(const BenchmarkConfig& config) {
    std::vector<TestCase> testCases;
    testCases.reserve(config.razmer_testovyh_dannyh + 200);

    std::random_device rd;
    std::mt19937 gen(42);
    std::uniform_int_distribution<> year_dist(config.minimalnyj_god, config.maksimalnyj_god);
    std::uniform_int_distribution<> day_dist(1, 365);

    for (size_t i = 0; i < config.razmer_testovyh_dannyh; ++i) {
        TestCase tc;
        int year = year_dist(gen);
        int dayOfYear = day_dist(gen);

        int maxDay = (!(year & 3) && ((year % 100) || !(year % 400))) ? 366 : 365;
        if (dayOfYear > maxDay) {
            dayOfYear = maxDay;
        }

        std::memset(&tc.struktura_vremeni, 0, sizeof(struct tm));
        tc.struktura_vremeni.tm_year = year - 1900;
        tc.struktura_vremeni.tm_yday = dayOfYear - 1;
        tc.god = year;
        tc.den_goda = dayOfYear;
        tc.opisanie = "Random";

        testCases.push_back(tc);
    }

    if (config.vklyuchat_granichnye_sluchai) {
        std::vector<int> centuries = {1800, 1900, 2000, 2100, 2200};
        for (int cent : centuries) {
            if (cent >= config.minimalnyj_god && cent <= config.maksimalnyj_god) {
                TestCase tc1;
                std::memset(&tc1.struktura_vremeni, 0, sizeof(struct tm));
                tc1.struktura_vremeni.tm_year = cent - 1900;
                tc1.struktura_vremeni.tm_yday = 0;
                tc1.god = cent;
                tc1.den_goda = 1;
                tc1.opisanie = "Начало века: " + std::to_string(cent);
                testCases.push_back(tc1);
            }
        }

        for (int year = config.minimalnyj_god; year <= config.maksimalnyj_god; year += 4) {
            // ВСТРОЕНО: isLeapYear(year)
            if ((!(year & 3) && ((year % 100) || !(year % 400)))) {
                TestCase tc;
                std::memset(&tc.struktura_vremeni, 0, sizeof(struct tm));
                tc.struktura_vremeni.tm_year = year - 1900;
                tc.struktura_vremeni.tm_yday = 59;
                tc.god = year;
                tc.den_goda = 60;
                tc.opisanie = "Високосное 29 Фев: " + std::to_string(year);
                testCases.push_back(tc);
            }
        }
    }

    return testCases;
}

// ============================================================================
// СТРУКТУРА РЕЗУЛЬТАТОВ БЕНЧМАРКА
// ============================================================================
template<typename Func>
struct BenchmarkResult {
    std::string imya_versii;
    double srednee_vremya_ns;
    double min_vremya_ns;
    double maks_vremya_ns;
    double mediana_vremeni_ns;
    double percentil_95_ns;
    double percentil_99_ns;
    size_t iteracii;
    bool proverka_korrektnosti;
    size_t raskhozhdeniya;

    // Статистика памяти
    MemoryStats pamyat_do;
    MemoryStats pamyat_posle;
    size_t stek_funkcii_bajty;
};

// ============================================================================
// ФУНКЦИЯ ДЛЯ БЕНЧА
// ============================================================================
template<typename Func>
BenchmarkResult<Func> benchmarkFunction(
    const std::string& name,
    Func func,
    const std::vector<TestCase>& testData,
    const BenchmarkConfig& config
) {
    BenchmarkResult<Func> result;
    result.imya_versii = name;
    result.iteracii = config.kolichestvo_iteracij;
    result.raskhozhdeniya = 0;

    // Оцениваем использование стека
    result.stek_funkcii_bajty = sizeof(struct tm) + 64; // тУТ я примерно учел локальные переменные

    if (config.podrobnyj_vyvod) {
        std::cout << "  Тестируем " << name << "..." << std::flush;
    }

    // Захватываем память (до)
    if (config.otslezhivat_pamyat) {
        result.pamyat_do.capture();
    }

    // "Прогрев"
    for (size_t i = 0; i < 10000; ++i) {
        volatile int week = func(testData[i % testData.size()].struktura_vremeni);
        (void)week;
    }

    // Бенчмарк
    std::vector<double> times;
    times.reserve(config.kolichestvo_iteracij);

    for (size_t i = 0; i < config.kolichestvo_iteracij; ++i) {
        const auto& testCase = testData[i % testData.size()];

        auto start = std::chrono::high_resolution_clock::now();
        volatile int week = func(testCase.struktura_vremeni);
        auto end = std::chrono::high_resolution_clock::now();

        double elapsed_ns = std::chrono::duration<double, std::nano>(end - start).count();
        times.push_back(elapsed_ns);
        (void)week;
    }

    // Захватываем память (после)
    if (config.otslezhivat_pamyat) {
        result.pamyat_posle.capture();
    }

    // Считаем статистику
    std::sort(times.begin(), times.end());

    double sum = 0.0;
    for (double t : times) {
        sum += t;
    }

    result.srednee_vremya_ns = sum / times.size();
    result.min_vremya_ns = times.front();
    result.maks_vremya_ns = times.back();
    result.mediana_vremeni_ns = times[times.size() / 2];
    result.percentil_95_ns = times[static_cast<size_t>(times.size() * 0.95)];
    result.percentil_99_ns = times[static_cast<size_t>(times.size() * 0.99)];

    // Проверка корректности
    result.proverka_korrektnosti = true;
    for (const auto& testCase : testData) {
        int result1 = convertGregorianDateToWeekDate_Original(testCase.struktura_vremeni);
        int result2 = func(testCase.struktura_vremeni);
        if (result1 != result2) {
            result.proverka_korrektnosti = false;
            result.raskhozhdeniya++;
            if (config.podrobnyj_vyvod && result.raskhozhdeniya <= 5) {
                std::cout << "\n    РАСХОЖДЕНИЕ: " << testCase.opisanie
                          << " Год=" << testCase.god
                          << " День=" << testCase.den_goda
                          << " Original=" << result1
                          << " " << name << "=" << result2;
            }
        }
    }

    if (config.podrobnyj_vyvod) {
        std::cout << " Готово!" << std::endl;
    }

    return result;
}

int main(int argc, char* argv[]) {
    BenchmarkConfig config;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--test-size" && i + 1 < argc) {
            config.razmer_testovyh_dannyh = std::stoull(argv[++i]);
        } else if (arg == "--iterations" && i + 1 < argc) {
            config.kolichestvo_iteracij = std::stoull(argv[++i]);
        } else if (arg == "--year-min" && i + 1 < argc) {
            config.minimalnyj_god = std::stoi(argv[++i]);
        } else if (arg == "--year-max" && i + 1 < argc) {
            config.maksimalnyj_god = std::stoi(argv[++i]);
        } else if (arg == "--no-edge-cases") {
            config.vklyuchat_granichnye_sluchai = false;
        } else if (arg == "--quiet") {
            config.podrobnyj_vyvod = false;
        } else if (arg == "--help") {
            std::cout << "Использование: " << argv[0] << " [options]\n"
                      << "Опции:\n"
                      << "  --test-size N       Количество тестовых случаев (default: 100000)\n"
                      << "  --iterations N      Количество итераций (default: 10000000)\n"
                      << "  --year-min YEAR     Минимальный год (default: 1800)\n"
                      << "  --year-max YEAR     Максимальный год (default: 2200)\n"
                      << "  --no-edge-cases     Отключить граничные случаи\n"
                      << "  --quiet             Минимальный вывод\n"
                      << "  --help              Показать справку\n";
            return 0;
        }
    }

    std::cout << "=== ISO 8601 Week Date Conversion ===" << std::endl;
    std::cout << "Конфигурация:" << std::endl;
    std::cout << "  Размер тест. данных: " << config.razmer_testovyh_dannyh << std::endl;
    std::cout << "  Итераций бенча: " << config.kolichestvo_iteracij << std::endl;
    std::cout << "  Диапазон лет: " << config.minimalnyj_god << "-" << config.maksimalnyj_god << std::endl;
    std::cout << std::endl;

    std::cout << "Генерация тестовых данных..." << std::endl;
    auto testData = generateTestData(config);
    std::cout << "Сгенерировано " << testData.size() << " тестовых случаев" << std::endl;
    std::cout << std::endl;

    std::cout << "Запуск бенчмарков..." << std::endl;

    auto result_orig = benchmarkFunction("Original", convertGregorianDateToWeekDate_Original, testData, config);
    auto result_v1 = benchmarkFunction("V1_EarlyReturn", convertGregorianDateToWeekDate_V1, testData, config);
    auto result_v2 = benchmarkFunction("V2_BitOps_", convertGregorianDateToWeekDate_V2, testData, config);
    auto result_v4 = benchmarkFunction("V4_Precalculation", convertGregorianDateToWeekDate_V4, testData, config);

    std::cout << "\n=== РЕЗУЛЬТАТЫ ПРОИЗВОДИТЕЛЬНОСТИ ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);

    auto printResult = [](const auto& r, double baseline_ns) {
        std::cout << "\n" << r.imya_versii << ":" << std::endl;
        std::cout << "  Производительность:" << std::endl;
        std::cout << "    Avg (сред.): " << r.srednee_vremya_ns << " ns" << std::endl;
        std::cout << "    Median (мед.): " << r.mediana_vremeni_ns << " ns" << std::endl;
        std::cout << "    Min (мин.):  " << r.min_vremya_ns << " ns" << std::endl;
        std::cout << "    Max (макс.):  " << r.maks_vremya_ns << " ns" << std::endl;
        std::cout << "    95th:      " << r.percentil_95_ns << " ns" << std::endl;
        std::cout << "    99th:      " << r.percentil_99_ns << " ns" << std::endl;
        std::cout << "    Speedup (ускорение): " << (baseline_ns / r.srednee_vremya_ns) << "x" << std::endl;

        std::cout << "  Память:" << std::endl;
        std::cout << "    Стек:      ~" << r.stek_funkcii_bajty << " байт" << std::endl;
        std::cout << "    RSS До: " << r.pamyat_do.tekushchij_rss_kb << " KB" << std::endl;
        std::cout << "    RSS После:  " << r.pamyat_posle.tekushchij_rss_kb << " KB" << std::endl;
        std::cout << "    Пик RSS:   " << r.pamyat_posle.pikovyj_rss_kb << " KB" << std::endl;

        std::cout << "  Корректность: " << (r.proverka_korrektnosti ? "ПРОЙДЕНО" : "ОШИБКА");
        if (!r.proverka_korrektnosti) {
            std::cout << " (" << r.raskhozhdeniya << " расхождений)";
        }
        std::cout << std::endl;
    };

    printResult(result_orig, result_orig.srednee_vremya_ns);
    printResult(result_v1, result_orig.srednee_vremya_ns);
    printResult(result_v2, result_orig.srednee_vremya_ns);
    printResult(result_v4, result_orig.srednee_vremya_ns);

    std::cout << "\nЗапись результатов в benchmark_analysis.csv..." << std::endl;
    std::ofstream csv("benchmark_analysis.csv");
    csv << "Версия,Среднее_нс,Медиана_нс,Мин_нс,Макс_нс,P95_нс,P99_нс,Ускорение,"
            << "Стек_Байты,RSS_До_КБ,RSS_После_КБ,Пиковый_RSS_КБ,"
            << "Итерации,Корректность,Расхождения\n";

    auto writeCSV = [&csv](const auto& r, double baseline_ns) {
        csv << r.imya_versii << ","
            << r.srednee_vremya_ns << ","
            << r.mediana_vremeni_ns << ","
            << r.min_vremya_ns << ","
            << r.maks_vremya_ns << ","
            << r.percentil_95_ns << ","
            << r.percentil_99_ns << ","
            << (baseline_ns / r.srednee_vremya_ns) << ","
            << r.stek_funkcii_bajty << ","
            << r.pamyat_do.tekushchij_rss_kb << ","
            << r.pamyat_posle.tekushchij_rss_kb << ","
            << r.pamyat_posle.pikovyj_rss_kb << ","
            << r.iteracii << ","
            << (r.proverka_korrektnosti ? "PASS" : "FAIL") << ","
            << r.raskhozhdeniya << "\n";
    };

    writeCSV(result_orig, result_orig.srednee_vremya_ns);
    writeCSV(result_v1, result_orig.srednee_vremya_ns);
    writeCSV(result_v2, result_orig.srednee_vremya_ns);
    writeCSV(result_v4, result_orig.srednee_vremya_ns);

    csv.close();

    std::cout << "\nГотово!" << std::endl;

    return 0;
}