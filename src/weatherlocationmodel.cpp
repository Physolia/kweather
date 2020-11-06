/*
 * Copyright 2020 Han Young <hanyoung@protonmail.com>
 * Copyright 2020 Devin Lin <espidev@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "weatherlocationmodel.h"
#include "weatherlocation.h"

#include <KConfigCore/KConfigGroup>
#include <KConfigCore/KSharedConfig>

#include <QJsonArray>
#include <QQmlEngine>

const QString WEATHER_LOCATIONS_CFG_GROUP = QStringLiteral("WeatherLocations");
const QString WEATHER_LOCATIONS_CFG_KEY = QStringLiteral("locationsVec");

/* ~~~ WeatherLocationListModel ~~~ */
WeatherLocationListModel::WeatherLocationListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    load();
}

void WeatherLocationListModel::load()
{
    // load locations from kconfig
    auto config = KSharedConfig::openConfig(QString(), KSharedConfig::FullConfig, QStandardPaths::AppConfigLocation);
    KConfigGroup group = config->group(WEATHER_LOCATIONS_CFG_GROUP);
    QJsonDocument doc = QJsonDocument::fromJson(group.readEntry(WEATHER_LOCATIONS_CFG_KEY, "{}").toUtf8());
    for (QJsonValueRef r : doc.array()) {
        QJsonObject obj = r.toObject();
        locationsVec.append(WeatherLocation::fromJson(obj));
    }
}

void WeatherLocationListModel::save()
{
    QJsonArray arr;
    for (auto lc : locationsVec) {
        arr.push_back(lc->toJson());
    }
    QJsonObject obj;
    obj["list"] = arr;

    auto config = KSharedConfig::openConfig(QString(), KSharedConfig::FullConfig, QStandardPaths::AppConfigLocation);
    KConfigGroup group = config->group(WEATHER_LOCATIONS_CFG_GROUP);
    group.writeEntry(WEATHER_LOCATIONS_CFG_KEY, QString(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
}

int WeatherLocationListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return locationsVec.size();
}

QVariant WeatherLocationListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= locationsVec.size() || index.row() < 0) {
        return {};
    }
    if (role == Roles::LocationRole) {
        return QVariant::fromValue(locationsVec.at(index.row()));
    }
    return {};
}

QHash<int, QByteArray> WeatherLocationListModel::roleNames() const
{
    return {{Roles::LocationRole, "location"}};
}

void WeatherLocationListModel::updateUi()
{
    Q_EMIT dataChanged(createIndex(0, 0), createIndex(locationsVec.size() - 1, 0));
    for (auto l : locationsVec) {
        Q_EMIT l->propertyChanged();
        l->weatherDayListModel()->updateUi();
        l->weatherHourListModel()->updateUi();
    }
}

void WeatherLocationListModel::insert(int index, WeatherLocation *weatherLocation)
{
    if ((index < 0) || (index > locationsVec.size()))
        return;

    QQmlEngine::setObjectOwnership(weatherLocation, QQmlEngine::CppOwnership);
    Q_EMIT beginInsertRows(QModelIndex(), index, index);
    locationsVec.insert(index, weatherLocation);
    Q_EMIT endInsertRows();

    save();
}

void WeatherLocationListModel::remove(int index)
{
    if ((index < 0) || (index >= locationsVec.size()))
        return;

    Q_EMIT beginRemoveRows(QModelIndex(), index, index);
    auto location = locationsVec.at(index);
    locationsVec.removeAt(index);
    delete location;
    Q_EMIT endRemoveRows();

    save();
}

WeatherLocation *WeatherLocationListModel::get(int index)
{
    if ((index < 0) || (index >= locationsVec.size()))
        return {};

    return locationsVec.at(index);
}

void WeatherLocationListModel::move(int oldIndex, int newIndex)
{
    if (oldIndex < 0 || oldIndex >= locationsVec.size() || newIndex < 0 || newIndex >= locationsVec.size())
        return;

    // to my surprise, we have to do this
    if (newIndex > oldIndex)
        std::swap(newIndex, oldIndex);

    Q_EMIT beginMoveRows(QModelIndex(), oldIndex, oldIndex, QModelIndex(), newIndex);
    locationsVec.move(oldIndex, newIndex);
    Q_EMIT endMoveRows();
}
int WeatherLocationListModel::count() const
{
    return locationsVec.size();
}
void WeatherLocationListModel::addLocation(KWeatherCore::LocationQueryResult *ret)
{
    qDebug() << "add location";
    auto locId = ret->geonameId(), locName = ret->toponymName();
    auto lat = ret->latitude(), lon = ret->longitude();

    // add location
    auto *location = new WeatherLocation(locId, locName, QString(), lat, lon, KWeatherCore::WeatherForecast());
    location->update();

    insert(this->locationsVec.size(), location);
}

// invoked by frontend
void WeatherLocationListModel::requestCurrentLocation()
{
    auto geoPtr = new KWeatherCore::LocationQuery(this);
    geoPtr->locate();
    //    // failure
    //    connect(geoPtr, &KWeatherCore::LocationQuery::, this, [this]() { Q_EMIT networkErrorCreatingDefault(); });
    //    // success
    connect(geoPtr, &KWeatherCore::LocationQuery::located, this, &WeatherLocationListModel::addCurrentLocation);
}

void WeatherLocationListModel::addCurrentLocation(KWeatherCore::LocationQueryResult ret)
{
    // default location, use timestamp as id
    long id = QDateTime::currentSecsSinceEpoch();

    auto location = new WeatherLocation(QString::number(id), ret.name(), QString(), ret.latitude(), ret.longitude(), KWeatherCore::WeatherForecast());
    location->update();

    insert(0, location);
    Q_EMIT successfullyCreatedDefault();
}
