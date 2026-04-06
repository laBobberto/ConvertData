#include "common.h"
int convertGregorianDateToWeekDate_V2(const struct tm& times) noexcept {
    const int y = times.tm_year + 1900;
    const int dayOfYear = times.tm_yday + 1;
    const int y_1 = y - 1;
    const int yy = y_1 % 100;
    const int c = y_1 - yy;
    const int g = yy + (yy >> 2);
    const int jan1Weekday = 1 + (((((c / 100) & 3) * 5) + g) % 7);
    const int weekday = 1 + ((dayOfYear + jan1Weekday - 2) % 7);
    if ((dayOfYear <= (8 - jan1Weekday)) & (jan1Weekday > 4)) {
        const int prevYearLeap = (!(y_1 & 3) && ((y_1 % 100) || !(y_1 % 400)));
        const int is53 = (jan1Weekday == 5) | ((jan1Weekday == 6) & prevYearLeap);
        return 52 + is53;
    }
    const int daysInYear = 365 + (!(y & 3) && ((y % 100) || !(y % 400)));
    if ((daysInYear - dayOfYear) < (4 - weekday)) return 1;
    const int j = dayOfYear + (7 - weekday) + (jan1Weekday - 1);
    const int weekNumber = (j / 7) - (jan1Weekday > 4);
    return weekNumber;
}
#ifndef OPT_LEVEL
#define OPT_LEVEL "Unknown"
#endif
int main() {
    BenchmarkConfig config;
    auto testData = generateTestData(config);
    runBenchmark("V2_BitOps", OPT_LEVEL, convertGregorianDateToWeekDate_V2, testData, config);
    return 0;
}
