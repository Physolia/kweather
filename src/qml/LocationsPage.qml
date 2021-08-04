/*
 * SPDX-FileCopyrightText: 2020 Han Young <hanyoung@protonmail.com>
 * SPDX-FileCopyrightText: 2020 Devin Lin <espidev@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.12 as Kirigami

Kirigami.ScrollablePage {
    title: i18n("Locations")
    
    property int yTranslate: 0
    property int currentIndex: 0;

    mainAction: Kirigami.Action {
        iconName: "list-add"
        text: i18n("Add Location")
        onTriggered: appwindow.pageStack.push(Qt.resolvedUrl("AddLocationPage.qml"))
    }

    Connections {
        target: weatherLocationListModel
        onNetworkErrorCreating: showPassiveNotification(i18n("Unable to fetch timezone information"))
    }

    ListView {
        id: citiesList
        model: weatherLocationListModel
        transform: Translate { y: yTranslate }

        reuseItems: true
        currentIndex: -1 // no default highlight

        add: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: Kirigami.Units.shortDuration }
        }
        remove: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: Kirigami.Units.shortDuration }
        }
        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuad}
        }
        moveDisplaced: Transition {
            YAnimator {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Kirigami.Units.largeSpacing

            icon.name: "globe"
            text: i18n("Add a location")
            visible: citiesList.count == 0
        }

        delegate: Kirigami.DelegateRecycler {
            width: citiesList.width
            sourceComponent: delegateComponent
        }
        
        Component {
            id: delegateComponent
            
            Kirigami.SwipeListItem {
                id: listItem
                actions: [
                    Kirigami.Action {
                        iconName: "delete"
                        text: i18n("Remove")
                        onTriggered: {
                            weatherLocationListModel.remove(index);
                        }
                    }
                ]
                onClicked: {
                    appwindow.switchToPage(appwindow.getPage("Forecast"), 0);
                    appwindow.getPage("Forecast").pageIndex = index;
                }

                contentItem: Item {
                    implicitWidth: listItem.width
                    implicitHeight: delegateLayout.implicitHeight
                    RowLayout {
                        Layout.alignment: Qt.AlignLeft
                        id: delegateLayout
                        anchors {
                            left: parent.left
                            top: parent.top
                            right: parent.right
                        }
                        spacing: Kirigami.Units.largeSpacing * 2

                        Kirigami.ListItemDragHandle {
                            listItem: listItem
                            listView: citiesList
                            onMoveRequested: {
                                weatherLocationListModel.move(oldIndex, newIndex)
                            }
                        }
                        
                        ColumnLayout {
                            spacing: Kirigami.Units.smallSpacing
                            Kirigami.Icon {
                                Layout.alignment: Qt.AlignHCenter
                                source: location.todayForecast.weatherIcon
                                Layout.maximumHeight: Kirigami.Units.iconSizes.sizeForLabels * 2
                                Layout.preferredWidth: height
                                Layout.preferredHeight: Kirigami.Units.iconSizes.sizeForLabels * 2
                            }
                            Label {
                                Layout.alignment: Qt.AlignHCenter
                                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.3
                                text: location.todayForecast.maxTemp
                            }
                        }

                        Kirigami.Heading {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            
                            level: 2
                            text: location.name
                            elide: Text.ElideRight
                            maximumLineCount: 2
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }
    }
}
