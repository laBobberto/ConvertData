#include "common.h"
int convertGregorianDateToWeekDate_V1(const struct tm& times) noexcept {
    const int y = times.tm_year + 1900;
    const int dayOfYear = times.tm_yday + 1;
    const int yy = (y - 1) % 100;
    const int c = (y - 1) - yy;
    const int g = yy + yy / 4;
    const int jan1Weekday = 1 + (((((c / 100) % 4) * 5) + g) % 7);
    const int h = dayOfYear + (jan1Weekday - 1);
    const int weekday = 1 + ((h - 1) % 7);
    if ((dayOfYear <= (8 - jan1Weekday)) && (jan1Weekday > 4)) {
        const int y_1 = y - 1;
        return ((jan1Weekday == 5) || ((jan1Weekday == 6) && (!(y_1 & 3) && ((y_1 % 100) || !(y_1 % 400))))) ? 53 : 52;
    }
    const int daysInYear = (!(y & 3) && ((y % 100) || !(y % 400))) ? 366 : 365;
    if ((daysInYear - dayOfYear) < (4 - weekday)) return 1;
    const int j = dayOfYear + (7 - weekday) + (jan1Weekday - 1);
    int weekNumber = j / 7;
    if (jan1Weekday > 4) weekNumber--;
    return weekNumber;
}
#ifndef OPT_LEVEL
#define OPT_LEVEL "Unknown"
#endif
int main() {
    BenchmarkConfig config;
    auto testData = generateTestData(config);
    runBenchmark("V1_EarlyReturn", OPT_LEVEL, convertGregorianDateToWeekDate_V1, testData, config);
    return 0;
}
