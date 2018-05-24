import QtQuick.Controls 1.2
import QtQuick.Controls 2.1 as QQC2
import QtQuick.Layouts 1.1
import QtQuick 2.4
import org.kde.discover 2.0
import org.kde.discover.app 1.0
import "navigation.js" as Navigation
import org.kde.kirigami 2.1 as Kirigami

DiscoverPage
{
    id: page
    title: i18n("Updates")

    property string footerLabel: ""

    ResourcesUpdatesModel {
        id: resourcesUpdatesModel
        onPassiveMessage: window.showPassiveNotification(message)
        onIsProgressingChanged: {
            if (!isProgressing) {
                resourcesUpdatesModel.prepare()
            }
        }

        Component.onCompleted: {
            if (!isProgressing) {
                resourcesUpdatesModel.prepare()
            }
        }
    }

    UpdateModel {
        id: updateModel
        backend: resourcesUpdatesModel
    }

    Kirigami.Action
    {
        id: updateAction
        text: page.unselected>0 ? i18n("Update Selected") : i18n("Update All")
        visible: updateModel.toUpdateCount
        iconName: "update-none"
        enabled: !resourcesUpdatesModel.isProgressing
        onTriggered: resourcesUpdatesModel.updateAll()
    }

    footer:  TextArea {
        width: parent.width
        height: Kirigami.Units.gridUnit * 10
        text: log.contents
        visible: text.length > 0

        onTextChanged: flickableItem.contentY = flickableItem.contentHeight - flickableItem.height
        font.family: "monospace"

        ReadFile {
            id: log
            filter: ".*ALPM-SCRIPTLET\\] .*"
            path: "/var/log/pacman.log"
        }
    }

    Kirigami.Action
    {
        id: cancelUpdateAction
        iconName: "dialog-cancel"
        text: i18n("Cancel")
        enabled: resourcesUpdatesModel.transaction && resourcesUpdatesModel.transaction.isCancellable
        onTriggered: resourcesUpdatesModel.transaction.cancel()
    }

    readonly property int unselected: (updateModel.totalUpdatesCount - updateModel.toUpdateCount)
    readonly property QtObject currentAction: resourcesUpdatesModel.isProgressing ? cancelUpdateAction : updateAction
    actions {
        left: refreshAction
        main: currentAction
    }

    header: QQC2.ToolBar {
        visible: (updateModel.totalUpdatesCount > 0 && resourcesUpdatesModel.isProgressing) || updateModel.hasUpdates

        RowLayout {
            anchors.fill: parent
            LabelBackground {
                Layout.leftMargin: Kirigami.Units.gridUnit
                text: updateModel.toUpdateCount + " (" + updateModel.updateSize+")"
            }
            QQC2.Label {
                text: i18n("updates selected")
            }
            LabelBackground {
                id: unselectedItem
                text: page.unselected
                visible: page.unselected>0
            }
            QQC2.Label {
                text: i18n("updates not selected")
                visible: unselectedItem.visible
            }
            Item {
                Layout.fillWidth: true
            }
                enabled: page.currentAction.enabled
        }
    }

    supportsRefreshing: true
    onRefreshingChanged: {
        showPassiveNotification("Fetching updates...")
        ResourcesModel.updateAction.triggered()
        refreshing = false
    }
    ListView
    {
        id: updatesView
        currentIndex: -1

        footer: ColumnLayout {
            anchors.right: parent.right
            anchors.left: parent.left
            Kirigami.Heading {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                visible: page.footerLabel !== ""
                text: page.footerLabel
            }
            Kirigami.Icon {
                Layout.alignment: Qt.AlignHCenter
                visible: page.footerLabel !== ""
                source: "update-none"
                opacity: 0.3
                width: 200
                height: 200
            }
            Item {
                visible: page.footerLabel === ""
                height: Kirigami.Units.gridUnit
                width: 1
            }
        }

        model: updateModel

        section {
            property: "section"
            delegate: Kirigami.Heading {
                x: Kirigami.Units.gridUnit
                level: 2
                text: section
            }
        }

        spacing: Kirigami.Units.smallSpacing

        delegate: Kirigami.AbstractListItem {
            backgroundColor: Kirigami.Theme.viewBackgroundColor
            x: Kirigami.Units.gridUnit
            width: ListView.view.width - Kirigami.Units.gridUnit * 2
            highlighted: ListView.isCurrentItem
            onEnabledChanged: if (!enabled) {
                layout.extended = false;
            }

            Keys.onReturnPressed: {
                itemChecked.clicked()
            }
            Keys.onPressed: if (event.key===Qt.Key_Alt) layout.extended = true
            Keys.onReleased: if (event.key===Qt.Key_Alt)  layout.extended = false

            ColumnLayout {
                id: layout
                property bool extended: false
                onExtendedChanged: if (extended) {
                    updateModel.fetchChangelog(index)
                }
                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    CheckBox {
                        id: itemChecked
                        Layout.alignment: Qt.AlignVCenter
                        checked: model.checked == Qt.Checked
                        onClicked: model.checked = (model.checked==Qt.Checked ? Qt.Unchecked : Qt.Checked)
                    }

                    Kirigami.Icon {
                        Layout.fillHeight: true
                        Layout.preferredWidth: height
                        source: decoration
                        smooth: true
                    }

                    QQC2.Label {
                        Layout.fillWidth: true
                        text: i18n("%1 (%2)", display, version)
                        elide: Text.ElideRight
                    }

                    LabelBackground {
                        Layout.minimumWidth: Kirigami.Units.gridUnit * 6
                        text: size

                        progress: resourceProgress/100
                    }
                }

                QQC2.Frame {
                    Layout.fillWidth: true
                    implicitHeight: view.contentHeight
                    visible: layout.extended && changelog.length>0
                    QQC2.Label {
                        id: view
                        anchors {
                            right: parent.right
                            left: parent.left
                        }
                        text: changelog
                        textFormat: Text.StyledText
                        wrapMode: Text.WordWrap
                        onLinkActivated: Qt.openUrlExternally(link)

                    }

                    //This saves a binding loop on implictHeight, as the Label
                    //height is updated twice (first time with the wrong value)
                    Behavior on implicitHeight
                    { PropertyAnimation { duration: Kirigami.Units.shortDuration } }
                }

                Button {
                    Layout.alignment: Qt.AlignRight
                    text: i18n("More Information...")
                    visible: layout.extended
                    enabled: !resourcesUpdatesModel.isProgressing
                    onClicked: Navigation.openApplication(resource)
                }
            }

            onClicked: {
                layout.extended = !layout.extended
            }
        }
    }

    readonly property alias secSinceUpdate: resourcesUpdatesModel.secsToLastUpdate
    state:  ( updateModel.hasUpdates                     ? "has-updates"
            : ResourcesModel.isFetching                  ? "fetching"
            : resourcesUpdatesModel.isProgressing        ? "progressing"
            : secSinceUpdate < 0                         ? "unknown"
            : secSinceUpdate === 0                       ? "now-uptodate"
            : secSinceUpdate < 1000 * 60 * 60 * 24       ? "uptodate"
            : secSinceUpdate < 1000 * 60 * 60 * 24 * 7   ? "medium"
            :                                              "low"
            )

    states: [
        State {
            name: "fetching"
            PropertyChanges { target: page; title: i18nc("@info", "Fetching...") }
            PropertyChanges { target: page; footerLabel: i18nc("@info", "Looking for updates") }
        },
        State {
            name: "progressing"
            PropertyChanges { target: page; title: i18nc("@info", "Updating...") }
            PropertyChanges { target: page; footerLabel: resourcesUpdatesModel.progress<=0 ? i18nc("@info", "Fetching updates") : "" }
        },
        State {
            name: "has-updates"
            PropertyChanges { target: page; title: i18nc("@info", "Updates") }
        },
        State {
            name: "now-uptodate"
            PropertyChanges { target: page; title: i18nc("@info", "The system is up to date") }
            PropertyChanges { target: page; footerLabel: i18nc("@info", "No updates") }
        },
        State {
            name: "uptodate"
            PropertyChanges { target: page; title: i18nc("@info", "The system is up to date") }
            PropertyChanges { target: page; footerLabel: i18nc("@info", "No updates") }
        },
        State {
            name: "medium"
            PropertyChanges { target: page; title: i18nc("@info", "No updates are available") }
        },
        State {
            name: "low"
            PropertyChanges { target: page; title: i18nc("@info", "Should check for updates") }
        },
        State {
            name: "unknown"
            PropertyChanges { target: page; title: i18nc("@info", "It is unknown when the last check for updates was") }
        }
    ]
}
