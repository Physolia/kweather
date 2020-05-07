#include "weatherlocationmodel.h"
#include "abstractweatherapi.h"
#include "abstractweatherforecast.h"
#include "locationquerymodel.h"
#include "nmiweatherapi2.h"
#include "weatherdaymodel.h"
#include "weatherhourmodel.h"
#include <KConfigCore/KConfigGroup>
#include <KConfigCore/KSharedConfig>
#include <QQmlEngine>

const QString WEATHER_LOCATIONS_CFG_GROUP = "WeatherLocations";

/* ~~~ WeatherLocation ~~~ */
WeatherLocation::WeatherLocation(AbstractWeatherForecast *forecast)
{
    this->weatherDayListModel_ = new WeatherDayListModel(this);
    this->weatherHourListModel_ = new WeatherHourListModel(this);
    this->lastUpdated_ = QDateTime::currentDateTime();
    this->forecast_ = forecast;
    QQmlEngine::setObjectOwnership(this->weatherDayListModel_, QQmlEngine::CppOwnership); // prevent segfaults from js garbage collecting
    QQmlEngine::setObjectOwnership(this->weatherHourListModel_, QQmlEngine::CppOwnership);
}

WeatherLocation::WeatherLocation(AbstractWeatherAPI *weatherBackendProvider, QString locationId, QString locationName, float latitude, float longitude, AbstractWeatherForecast *forecast)
{
    this->weatherBackendProvider_ = weatherBackendProvider;
    this->locationId_ = locationId;
    this->locationName_ = locationName;
    this->latitude_ = latitude;
    this->longitude_ = longitude;
    this->forecast_ = forecast;
    this->weatherDayListModel_ = new WeatherDayListModel(this);
    this->weatherHourListModel_ = new WeatherHourListModel(this);
    this->lastUpdated_ = forecast == nullptr ? QDateTime::currentDateTime() : forecast->timeCreated();
    QQmlEngine::setObjectOwnership(this->weatherDayListModel_, QQmlEngine::CppOwnership); // prevent segfaults from js garbage collecting
    QQmlEngine::setObjectOwnership(this->weatherHourListModel_, QQmlEngine::CppOwnership);
    determineCurrentForecast();

    if (weatherBackendProvider != nullptr)
        weatherBackendProvider->setLocation(latitude, longitude);

    connect(this->weatherBackendProvider(), &AbstractWeatherAPI::updated, this, &WeatherLocation::updateData, Qt::UniqueConnection);
}

WeatherLocation* WeatherLocation::fromJson(const QString& json)
{
    QJsonObject obj = QJsonDocument::fromJson(json.toUtf8()).object();
    return new WeatherLocation(new NMIWeatherAPI2(obj["locationId"].toString()), obj["locationId"].toString(), obj["locationName"].toString(), obj["latitude"].toDouble(), obj["longitude"].toDouble());
}

QString WeatherLocation::toJson()
{
    QJsonObject obj;
    obj["locationId"] = locationId();
    obj["locationName"] = locationName();
    obj["latitude"] = latitude();
    obj["longitude"] = longitude();
    return QString(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

// save to config
void WeatherLocation::save()
{
    auto config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(WEATHER_LOCATIONS_CFG_GROUP);
    group.writeEntry(this->locationId(), this->toJson());
}

void WeatherLocation::updateData(AbstractWeatherForecast *fc)
{
    forecast_ = fc; // don't need to delete pointers, they were already deleted by api class
    determineCurrentForecast();
    this->lastUpdated_ = fc->timeCreated();

    emit weatherRefresh(fc);
}

void WeatherLocation::determineCurrentForecast()
{
    if (forecast() == nullptr || forecast()->hourlyForecasts().count() == 0) {
        currentWeather_ = new AbstractHourlyWeatherForecast(QDateTime::currentDateTime(), "Unknown", "weather-none-available", "weather-none-available", 0, 0, AbstractHourlyWeatherForecast::WindDirection::N, 0, 0, 0, 0, 0);
    } else {
        long long minSecs = -1;
        QDateTime current = QDateTime::currentDateTime();

        // get closest forecast to current time
        for (auto forecast : forecast()->hourlyForecasts()) {
            if (minSecs == -1 || minSecs > forecast->date().secsTo(current)) {
                currentWeather_ = forecast;
                minSecs = forecast->date().secsTo(current);
            }
        }
    }
    QQmlEngine::setObjectOwnership(currentWeather_, QQmlEngine::CppOwnership); // prevent segfaults from js garbage collecting
    emit currentForecastChange();
}

/* ~~~ WeatherLocationListModel ~~~ */
WeatherLocationListModel::WeatherLocationListModel(QObject *parent)
{
    load();
}

void WeatherLocationListModel::load()
{
    // load locations from kconfig
    auto config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(WEATHER_LOCATIONS_CFG_GROUP);
    for (QString key : group.keyList()) {
        QString json = group.readEntry(key, "");
        if (json != "") {
            locationsList.append(WeatherLocation::fromJson(json));
        }
    }
}

int WeatherLocationListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return locationsList.size();
}

QVariant WeatherLocationListModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

void WeatherLocationListModel::updateUi()
{
    emit dataChanged(createIndex(0, 0), createIndex(locationsList.count() - 1, 0));
}

void WeatherLocationListModel::insert(int index, WeatherLocation *weatherLocation)
{
    if ((index < 0) || (index > locationsList.count()))
        return;

    QQmlEngine::setObjectOwnership(weatherLocation, QQmlEngine::CppOwnership);
    emit beginInsertRows(QModelIndex(), index, index);

    // save to config
    weatherLocation->save();

    locationsList.insert(index, weatherLocation);
    emit endInsertRows();
}

void WeatherLocationListModel::remove(int index)
{
    if ((index < 0) || (index >= locationsList.count()))
        return;

    emit beginRemoveRows(QModelIndex(), index, index);

    // remove from config
    auto config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(WEATHER_LOCATIONS_CFG_GROUP);
    group.deleteEntry(locationsList.at(index)->locationId());

    locationsList.removeAt(index);
    emit endRemoveRows();
}

WeatherLocation *WeatherLocationListModel::get(int index)
{
    if ((index < 0) || (index >= locationsList.count()))
        return {};

    return locationsList.at(index);
}

void WeatherLocationListModel::move(int oldIndex, int newIndex)
{
    if (oldIndex < 0 || oldIndex >= locationsList.count() || newIndex < 0 || newIndex >= locationsList.count())
        return;
    locationsList.move(oldIndex, newIndex);
}

void WeatherLocationListModel::addLocation(LocationQueryResult *ret)
{
    qDebug() << "add location";
    auto api = new NMIWeatherAPI2(ret->name());
    qDebug() << "lat" << ret->latitude();
    qDebug() << "lgn" << ret->longitude();
    api->setLocation(ret->latitude(), ret->longitude());
    auto location = new WeatherLocation(api, ret->geonameId(), ret->name(), ret->latitude(), ret->longitude());
    api->update();
    insert(this->locationsList.count(), location);
}
