#include "common.h"
int convertGregorianDateToWeekDate_Original(const struct tm& times) noexcept {
    const int y = times.tm_year + 1900;
    const int dayOfYearNumber = times.tm_yday + 1;
    const int yy = (y - 1) % 100;
    const int c = (y - 1) - yy;
    const int g = yy + yy / 4;
    const int jan1Weekday = 1 + (((((c / 100) % 4) * 5) + g) % 7);
    const int h = dayOfYearNumber + (jan1Weekday - 1);
    const int weekday = 1 + ((h - 1) % 7);
    int yearNumber = 0, weekNumber = 0;
    if ((dayOfYearNumber <= (8 - jan1Weekday)) && (jan1Weekday > 4)) {
        yearNumber = y - 1;
        weekNumber = ((jan1Weekday == 5) || ((jan1Weekday == 6) && (!(yearNumber & 3) && ((yearNumber % 100) || !(yearNumber % 400))))) ? 53 : 52;
    } else {
        yearNumber = y;
        const int daysInYear = (!(y & 3) && ((y % 100) || !(y % 400))) ? 366 : 365;
        if ((daysInYear - dayOfYearNumber) < (4 - weekday)) {
            yearNumber = y + 1;
            weekNumber = 1;
        }
    }
    if (yearNumber == y) {
        const int j = dayOfYearNumber + (7 - weekday) + (jan1Weekday - 1);
        weekNumber = j / 7;
        if (jan1Weekday > 4) weekNumber--;
    }
    return weekNumber;
}
#ifndef OPT_LEVEL
#define OPT_LEVEL "Unknown"
#endif
int main() {
    BenchmarkConfig config;
    auto testData = generateTestData(config);
    runBenchmark("Original", OPT_LEVEL, convertGregorianDateToWeekDate_Original, testData, config);
    return 0;
}
