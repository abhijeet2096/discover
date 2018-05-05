import QtQuick 2.4
import QtQuick.Controls 1.1
import QtQuick.Controls 2.1 as QQC2
import QtQuick.Layouts 1.1
import org.kde.discover 2.0
import org.kde.discover.app 1.0
import org.kde.kirigami 2.2 as Kirigami
import "navigation.js" as Navigation

DiscoverPage {
    id: page
    clip: true
    title: i18n("Settings")
    property string search: ""

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
    }

    contextualActions: [
        KirigamiActionBridge { action: app.action("help_about_app") },
        KirigamiActionBridge { action: app.action("help_report_bug") }
    ]

    mainItem: ListView {
        id: sourcesView
        model: QSortFilterProxyModel {
            filterRegExp: new RegExp(page.search, 'i')
            dynamicSortFilter: false //We don't want to sort, as sorting can have some semantics on some backends
            sourceModel: SourcesModel
        }
        currentIndex: -1

        Component {
            id: sourceBackendDelegate
            Kirigami.AbstractListItem {
                id: backendItem
                hoverEnabled: false
                supportsMouseEvents: false
                readonly property QtObject backend: sourcesBackend
                readonly property bool isDefault: ResourcesModel.currentApplicationBackend == resourcesBackend
                RowLayout {
                    Connections {
                        target: backendItem.backend
                        onPassiveMessage: window.showPassiveNotification(message)
                    }

                    anchors {
                        right: parent.right
                        left: parent.left
                        rightMargin: parent.rightPadding
                        leftMargin: parent.leftPadding
                    }
                    Kirigami.Heading {
                        Layout.fillWidth: true
                        text: backendItem.isDefault ? i18n("%1 (Default)", resourcesBackend.displayName) : resourcesBackend.displayName
                    }
                    Button {
                        Layout.rightMargin: Kirigami.Units.smallSpacing
                        iconName: "preferences-other"

                        visible: resourcesBackend && resourcesBackend.hasApplications
                        Component {
                            id: dialogComponent
                            AddSourceDialog {
                                source: backendItem.backend
                                onVisibleChanged: if (!visible) {
                                    destroy()
                                }
                            }
                        }

                        menu: Menu {
                            id: settingsMenu
                            MenuItem {
                                enabled: !backendItem.isDefault
                                text: i18n("Make default")
                                onTriggered: ResourcesModel.currentApplicationBackend = backendItem.backend.resourcesBackend
                            }

                            MenuItem {
                                text: i18n("Add Source...")
                                visible: backendItem.backend && backendItem.backend.supportsAdding

                                onTriggered: {
                                    var addSourceDialog = dialogComponent.createObject(null, {displayName: backendItem.backend.resourcesBackend.displayName })
                                    addSourceDialog.open()
                                }
                            }

                            MenuSeparator {
                                visible: backendActionsInst.count>0
                            }

                            Instantiator {
                                id: backendActionsInst
                                model: ActionsModel {
                                    actions: backendItem.backend ? backendItem.backend.actions : undefined
                                }
                                delegate: MenuItem {
                                    action: ActionBridge { action: modelData.action }
                                }
                                onObjectAdded: {
                                    settingsMenu.insertItem(index, object)
                                }
                                onObjectRemoved: {
                                    object.destroy()
                                }
                            }
                        }
                    }
                }
            }
        }

        delegate: ConditionalLoader {
            anchors {
                right: parent.right
                left: parent.left
            }
            readonly property variant resourcesBackend: model.resourcesBackend
            readonly property variant sourcesBackend: model.sourcesBackend
            readonly property variant display: model.display
            readonly property variant checked: model.checked
            readonly property variant statusTip: model.statusTip
            readonly property variant toolTip: model.toolTip
            readonly property variant sourceId: model.sourceId
            readonly property variant modelIndex: sourcesView.model.index(index, 0)

            condition: resourcesBackend != null
            componentTrue: sourceBackendDelegate
            componentFalse: sourceDelegate
        }

        Component {
            id: sourceDelegate
            Kirigami.SwipeListItem {
                Layout.fillWidth: true
                enabled: display.length>0
                highlighted: ListView.isCurrentItem
                onClicked: Navigation.openApplicationListSource(sourceId)

                Keys.onReturnPressed: clicked()
                actions: [
                    Kirigami.Action {
                        iconName: "go-up"
                        enabled: sourcesBackend.firstSourceId !== sourceId
                        visible: sourcesBackend.canMoveSources
                        onTriggered: {
                             var ret = sourcesBackend.moveSource(sourceId, -1)
                             if (!ret)
                                 window.showPassiveNotification(i18n("Failed to increase '%1' preference", display))
                        }
                    },
                    Kirigami.Action {
                        iconName: "go-down"
                        enabled: sourcesBackend.lastSourceId !== sourceId
                        visible: sourcesBackend.canMoveSources
                        onTriggered: {
                            var ret = sourcesBackend.moveSource(sourceId, +1)
                            if (!ret)
                                 window.showPassiveNotification(i18n("Failed to decrease '%1' preference", display))
                        }
                    },
                    Kirigami.Action {
                        iconName: "edit-delete"
                        tooltip: i18n("Delete the origin")
                        onTriggered: {
                            var backend = sourcesBackend
                            if (!backend.removeSource(sourceId)) {
                                window.showPassiveNotification(i18n("Failed to remove the source '%1'", display))
                            }
                        }
                    }
                ]

                RowLayout {
                    CheckBox {
                        id: enabledBox

                        readonly property variant modelChecked: sourcesView.model.data(modelIndex, Qt.CheckStateRole)
                        checked: modelChecked != Qt.Unchecked
                        enabled: modelChecked !== undefined
                        onClicked: {
                            sourcesView.model.setData(modelIndex, checkedState, Qt.CheckStateRole)
                        }
                    }
                    QQC2.Label {
                        text: display + " - <i>" + toolTip + "</i>"
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
            }
        }

        footer: ColumnLayout {
            id: foot
            anchors {
                right: parent.right
                left: parent.left
                margins: Kirigami.Units.smallSpacing
            }
            Kirigami.Heading {
                Layout.fillWidth: true
                text: i18n("Missing Backends")
                visible: back.count>0
            }
            Repeater {
                id: back
                model: ResourcesProxyModel {
                    extending: "org.kde.discover.desktop"
                }
                delegate: RowLayout {
                    visible: !model.application.isInstalled
                    QQC2.Label {
                        Layout.fillWidth: true
                        text: name
                    }
                    InstallApplicationButton {
                        application: model.application
                    }
                }
            }
        }
    }
}
