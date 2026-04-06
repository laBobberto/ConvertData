#include "common.h"
int convertGregorianDateToWeekDate_V4_Mask(const struct tm& times) noexcept
{
    const int y = times.tm_year + 1900;
    const int dayOfYear = times.tm_yday + 1;
    const int y_1 = y - 1;
    const int jan1Weekday = 1 + ((y_1 + (y_1 / 4) - (y_1 / 100) + (y_1 / 400)) % 7);
    const int weekday = 1 + ((dayOfYear + jan1Weekday - 2) % 7);
    const int isPrevYear = (dayOfYear <= (8 - jan1Weekday)) & (jan1Weekday > 4);
    const int prevYearLeap = ((y_1 & 3) == 0) & (((y_1 % 100) != 0) | ((y_1 % 400) == 0));
    const int prevIs53 = (jan1Weekday == 5) | ((jan1Weekday == 6) & prevYearLeap);
    const int prevYearWeek = 52 + prevIs53;
    const int isLeap = ((y & 3) == 0) & (((y % 100) != 0) | ((y % 400) == 0));
    const int daysInYear = 365 + isLeap;
    const int isNextYear = (daysInYear - dayOfYear) < (4 - weekday);
    const int j = dayOfYear + (7 - weekday) + (jan1Weekday - 1);
    const int currYearWeek = (j / 7) - (jan1Weekday > 4);
    return (isPrevYear * prevYearWeek) + (isNextYear * 1) + (((isPrevYear | isNextYear) ^ 1) * currYearWeek);
}
#ifndef OPT_LEVEL
#define OPT_LEVEL "Unknown"
#endif
int main() {
    BenchmarkConfig config;
    auto testData = generateTestData(config);
    runBenchmark("V4_MathMask", OPT_LEVEL, convertGregorianDateToWeekDate_V4_Mask, testData, config);
    return 0;
}
