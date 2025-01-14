// SPDX-FileCopyrightText: 2022 Devin Lin <espidev@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.0
import QtQuick.Controls 2.2 as Controls
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.19 as Kirigami

Kirigami.Dialog {
    id: root
    title: i18n("Settings")
    standardButtons: Kirigami.Dialog.NoButton
    
    preferredWidth: Kirigami.Units.gridUnit * 35
    
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    
    Controls.Control {
        id: control
        leftPadding: 0
        rightPadding: 0
        topPadding: Kirigami.Units.gridUnit
        bottomPadding: Kirigami.Units.gridUnit
        
        background: Rectangle {
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            color: Kirigami.Theme.backgroundColor
        }
        
        contentItem: SettingsComponent {
            dialog: root
            width: control.width
        }
    }
}
