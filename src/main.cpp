#include <QApplication>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QUrl>
#include <QtQml>

#include <KAboutData>
#include <KConfig>
#include <KLocalizedContext>
#include <KLocalizedString>

#include "abstractdailyweatherforecast.h"
#include "abstracthourlyweatherforecast.h"
#include "abstractweatherforecast.h"
#include "locationquerymodel.h"
#include "nmiweatherapi2.h"
#include "settingsmodel.h"
#include "weatherdaymodel.h"
#include "weatherforecastmanager.h"
#include "weatherhourmodel.h"
#include "weatherlocationmodel.h"

class AbstractHourlyWeatherForecast;
class AbstractDailyWeatherForecast;

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

    KLocalizedString::setApplicationDomain("kweather");
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    KAboutData aboutData("kweather", "Weather", "0.1", "Weather application in Kirigami", KAboutLicense::GPL, i18n("© 2020 KDE Community"));
    KAboutData::setApplicationData(aboutData);

    // initialize models in context
    auto *weatherLocationListModel = new WeatherLocationListModel();
    auto *locationQueryModel = new LocationQueryModel();
    auto *settingsModel = new SettingsModel();
    WeatherForecastManager::instance(*weatherLocationListModel);

    engine.rootContext()->setContextProperty("weatherLocationListModel", weatherLocationListModel);
    engine.rootContext()->setContextProperty("locationQueryModel", locationQueryModel);
    engine.rootContext()->setContextProperty("settingsModel", settingsModel);
    // the longer the merrier, this add locations
    QObject::connect(locationQueryModel, &LocationQueryModel::appendLocation, [weatherLocationListModel, locationQueryModel] { weatherLocationListModel->addLocation(locationQueryModel->get(locationQueryModel->index_)); });

    // register QML types
    qmlRegisterType<WeatherLocation>("kweather", 1, 0, "WeatherLocation");
    qmlRegisterType<WeatherDay>("kweather", 1, 0, "WeatherDay");
    qmlRegisterType<WeatherHour>("kweather", 1, 0, "WeatherHour");
    qmlRegisterType<AbstractWeatherForecast>("kweather", 1, 0, "AbstractWeatherForecast");
    qmlRegisterType<AbstractHourlyWeatherForecast>("kweather", 1, 0, "AbstractHourlyWeatherForecast");
    qmlRegisterType<AbstractDailyWeatherForecast>("kweather", 1, 0, "AbstractDailyWeatherForecast");
    qmlRegisterType<AbstractWeatherForecast>("kweather", 1, 0, "AbstractWeatherForecast");
    qmlRegisterType<WeatherHourListModel>("kweather", 1, 0, "WeatherHourListModel");
    qmlRegisterType<WeatherDayListModel>("kweather", 1, 0, "WeatherDayListModel");

    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
