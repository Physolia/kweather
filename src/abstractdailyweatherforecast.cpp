#include "abstractdailyweatherforecast.h"

abstractdailyweatherforecast::abstractdailyweatherforecast()
{
}

abstractdailyweatherforecast::abstractdailyweatherforecast(int maxTemp, int minTemp, float precipitation, QString weatherIcon, QString weatherDescription, QDate date)
    : maxTemp_(maxTemp)
    , minTemp_(minTemp)
    , precipitation_(precipitation)
    , weatherIcon_(weatherIcon)
    , weatherDescription_(weatherDescription)
    , date_(date)
{
}
