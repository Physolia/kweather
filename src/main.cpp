/*
 * SPDX-FileCopyrightText: 2020 Han Young <hanyoung@protonmail.com>
 * SPDX-FileCopyrightText: 2020 Devin Lin <espidev@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QCommandLineParser>
#include <QIcon>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QUrl>
#include <QtQml>

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <KAboutData>
#include <KConfig>
#include <KLocalizedContext>
#include <KLocalizedString>

#include "kweathersettings.h"
#include "version.h"
#include "weatherlocation.h"

class AbstractHourlyWeatherForecast;
class AbstractDailyWeatherForecast;

Q_DECL_EXPORT int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
#else
    // set default style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
    // if using org.kde.desktop, ensure we use kde style if possible
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORMTHEME")) {
        qputenv("QT_QPA_PLATFORMTHEME", "kde");
    }

    QApplication app(argc, argv);
#endif
    QQmlApplicationEngine engine;

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kweather"));
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    KAboutData aboutData(QStringLiteral("kweather"),
                         i18n("Weather"),
                         QStringLiteral(KWEATHER_VERSION_STRING),
                         i18n("A convergent weather application for Plasma"),
                         KAboutLicense::GPL,
                         i18n("© 2020-2022 Plasma Development Team"));
    aboutData.setBugAddress("https://bugs.kde.org/describecomponents.cgi?product=kweather");
    aboutData.addAuthor(i18n("Han Young"), QString(), QStringLiteral("hanyoung@protonmail.com"));
    aboutData.addAuthor(i18n("Devin Lin"), QString(), QStringLiteral("espidev@gmail.com"), QStringLiteral("https://espi.dev"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    qmlRegisterSingletonType("kweather", 1, 0, "AboutData", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
        return engine->toScriptValue(KAboutData::applicationData());
    });

    engine.rootContext()->setContextProperty(QStringLiteral("settingsModel"), KWeatherSettings::self());

    WeatherLocation *emptyWeatherLocation = new WeatherLocation();
    engine.rootContext()->setContextProperty(QStringLiteral("emptyWeatherLocation"), emptyWeatherLocation);
#ifdef Q_OS_ANDROID
    engine.rootContext()->setContextProperty(QStringLiteral("KWEATHER_IS_ANDROID"), true);
#else
    engine.rootContext()->setContextProperty(QStringLiteral("KWEATHER_IS_ANDROID"), false);
#endif

    qRegisterMetaType<KWeatherCore::HourlyWeatherForecast>();
    qRegisterMetaType<KWeatherCore::DailyWeatherForecast>();
    qRegisterMetaType<QList<WeatherLocation *>>();

    // load setup wizard if first launch
    engine.loadFromModule("org.kde.kweather", "Main");

    // required for X11
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.kweather")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
