/*
 *   Copyright (C) 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls 2.1 as QQC2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.1
import "navigation.js" as Navigation
import org.kde.kirigami 2.4 as Kirigami

Kirigami.AbstractCard
{
    id: delegateArea
    property alias application: installButton.application
    property bool compact: false
    property bool showRating: true
    showClickFeedback: true

    function trigger() {
        if (ListView.view)
            ListView.view.currentIndex = index
        Navigation.openApplication(application)
    }
    highlighted: ListView.isCurrentItem
    Keys.onReturnPressed: trigger()
    onClicked: trigger()

    contentItem: Item {
        implicitHeight: delegateArea.compact ? Kirigami.Units.gridUnit * 2 : Kirigami.Units.gridUnit * 4

        Kirigami.Icon {
            id: resourceIcon
            source: application.icon
            readonly property real contHeight: delegateArea.compact ? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit * 5
            height: contHeight
            width: contHeight
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
            }
        }

        ColumnLayout {
            spacing: delegateArea.compact ? 0 : 5
            anchors {
                verticalCenter: parent.verticalCenter
                right: parent.right
                left: resourceIcon.right
                leftMargin: Kirigami.Units.largeSpacing
            }

            Kirigami.Heading {
                id: head
                level: delegateArea.compact ? 3 : 2
                Layout.fillWidth: true
                Layout.rightMargin: installButton.width
                elide: Text.ElideRight
                text: delegateArea.application.name
                maximumLineCount: 1

                InstallApplicationButton {
                    id: installButton
                    anchors {
                        verticalCenter: delegateArea.compact ? parent.bottom: parent.verticalCenter
                        left: parent.right
                    }
                }

            }

            RowLayout {
                visible: showRating
                spacing: Kirigami.Units.largeSpacing
                Rating {
                    rating: delegateArea.application.rating ? delegateArea.application.rating.sortableRating : 0
                    starSize: delegateArea.compact ? summary.font.pointSize : head.font.pointSize
                }
                QQC2.Label {
                    text: delegateArea.application.rating ? i18np("%1 rating", "%1 ratings", delegateArea.application.rating.ratingCount) : i18n("No ratings yet")
                    opacity: 0.5
                }
            }

            Layout.fillWidth: true
            QQC2.Label {
                id: summary
                Layout.fillWidth: true
                Layout.rightMargin: delegateArea.compact ? installButton.width : 0

                bottomPadding: Kirigami.Units.smallSpacing
                elide: Text.ElideRight
                text: delegateArea.application.comment
                maximumLineCount: 1
                textFormat: Text.PlainText
            }
        }
    }
}
