/*
 * SPDX-FileCopyrightText: 2020 Han Young <hanyoung@protonmail.com>
 * SPDX-FileCopyrightText: 2020-2021 Devin Lin <espidev@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.12 as Kirigami

Kirigami.Page {
    id: page
    topPadding: 0; bottomPadding: 0; rightPadding: 0; leftPadding: 0

    title: {
        if (weatherLocationListModel.locations.count == 0) {
            return i18n("Forecast");
        } else if (settingsModel.forecastStyle === "Dynamic") {
            return ""; // provided by DynamicForecastPage
        } else if (weatherLocationListModel.locations[loader.item.currentIndex]) {
            return weatherLocationListModel.locations[loader.item.currentIndex].name;
        } else {
            return "";
        }
    }
    
    globalToolBarStyle: (settingsModel.forecastStyle === "Dynamic" && pageStack.layers.depth <= 1) ? Kirigami.ApplicationHeaderStyle.None : Kirigami.ApplicationHeaderStyle.ToolBar
    
    property int yTranslate: 0
    
    function switchPageIndex(pageIndex) {
        loader.item.currentIndex = pageIndex;
    }
    
    // actions (only shown in flat view since the toolbar is hidden in dynamic view)
    actions.contextualActions: [
        Kirigami.Action {
            visible: Kirigami.Settings.isMobile
            iconName: "globe"
            text: i18n("Locations")
            onTriggered: addPageLayer(getPage("Locations"), 0)
        },
        Kirigami.Action {
            visible: Kirigami.Settings.isMobile
            iconName: "settings-configure"
            text: i18n("Settings")
            displayHint: Kirigami.Action.IconOnly
            onTriggered: addPageLayer(getPage("Settings"), 0)
        },
        Kirigami.Action {
            visible: !Kirigami.Settings.isMobile
            iconName: "view-refresh"
            text: i18n("Refresh")
            displayHint: Kirigami.Action.IconOnly
            onTriggered: weatherLocationListModel.locations[loader.item.currentIndex].update()
        },
        Kirigami.Action {
            visible: !Kirigami.Settings.isMobile && weatherLocationListModel.count > 1
            iconName: "arrow-left"
            text: i18n("Left")
            displayHint: Kirigami.Action.IconOnly
            onTriggered: loader.item.moveLeft()
            enabled: loader.item.canGoLeft
        },
        Kirigami.Action {
            visible: !Kirigami.Settings.isMobile && weatherLocationListModel.count > 1
            iconName: "arrow-right"
            text: i18n("Right")
            displayHint: Kirigami.Action.IconOnly
            onTriggered: loader.item.moveRight()
            enabled: loader.item.canGoRight
        }
    ]
    
    Loader {
        id: loader
        transform: Translate { y: yTranslate }
        anchors.fill: parent
        
        Component.onCompleted: loadStyle()
        function loadStyle() {
            setSource(settingsModel.forecastStyle === "Dynamic" ? "DynamicForecastPage.qml" : "FlatForecastPage.qml");
        }
        
        Connections {
            target: settingsModel
            function onForecastStyleChanged() {
                loader.loadStyle();
            }
        }
    }
}


