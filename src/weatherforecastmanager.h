#ifndef WEATHERFORECASTMANAGER_H
#define WEATHERFORECASTMANAGER_H

// ####### API MACRO #######
#define NORWEGIAN 0
#define OPENWEATHER 1

// #########################

#include <QObject>
#include <memory>
#include <random>
#include <vector>
class AbstractWeatherForecast;
class NMIWeatherAPI;
class OWMWeatherAPI;
class WeatherLocationListModel;
class QTimer;
class WeatherForecastManager : public QObject
{
    Q_OBJECT

public:
    static WeatherForecastManager& instance(WeatherLocationListModel& model);

signals:

    void updated();
private slots:
    void update();

protected:
    explicit WeatherForecastManager(WeatherLocationListModel& model, int defaultAPI = NORWEGIAN);

private:
    std::default_random_engine generator;
    std::uniform_int_distribution<int>* distribution;
    WeatherLocationListModel& model_;

    int api_ = NORWEGIAN;

    void writeToCache(const std::vector<AbstractWeatherForecast*>& data);
    QTimer* timer;

    void readFromCache(QString& url);
    WeatherForecastManager(const WeatherForecastManager&);
    WeatherForecastManager& operator=(const WeatherForecastManager&);
};

#endif // WEATHERFORECASTMANAGER_H
