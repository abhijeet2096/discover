/***************************************************************************
 *   Copyright © 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "SnapResource.h"
#include "SnapBackend.h"
#include <QDebug>
#include <QProcess>
#include <QBuffer>
#include <QImageReader>
#include <QStandardItemModel>
#include <KLocalizedString>
#include <utils.h>
#include <QProcess>

QDebug operator<<(QDebug debug, const QSnapdPlug& plug)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "QSnapdPlug(";
    debug.nospace() << "name:" << plug.name() << ',';
    debug.nospace() << "snap:" << plug.snap() << ',';
    debug.nospace() << "label:" << plug.label() << ',';
    debug.nospace() << "interface:" << plug.interface() << ',';
    debug.nospace() << "connectionCount:" << plug.connectionCount();
    debug.nospace() << ')';
    return debug;
}

QDebug operator<<(QDebug debug, const QSnapdSlot& slot)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "QSnapdSlot(";
    debug.nospace() << "name:" << slot.name() << ',';
    debug.nospace() << "label:" << slot.label() << ',';
    debug.nospace() << "snap:" << slot.snap() << ',';
    debug.nospace() << "interface:" << slot.interface() << ',';
    debug.nospace() << "connectionCount:" << slot.connectionCount();
    debug.nospace() << ')';
    return debug;
}

QDebug operator<<(QDebug debug, const QSnapdPlug* plug)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "*" << *plug;
    return debug;
}

QDebug operator<<(QDebug debug, const QSnapdSlot* slot)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "*" << *slot;
    return debug;
}

SnapResource::SnapResource(QSharedPointer<QSnapdSnap> snap, AbstractResource::State state, SnapBackend* backend)
    : AbstractResource(backend)
    , m_state(state)
    , m_snap(snap)
    , m_objects({ QStringLiteral("qrc:/snapui/PermissionsButton.qml") })
{
    setObjectName(snap->name());
}

QString SnapResource::availableVersion() const
{
    return installedVersion();
}

QStringList SnapResource::categories()
{
    return { QStringLiteral("Application") };
}

QString SnapResource::comment()
{
    return m_snap->summary();
}

int SnapResource::size()
{
//     return isInstalled() ? m_snap->installedSize() : m_snap->downloadSize();
    return m_snap->downloadSize();
}

QVariant SnapResource::icon() const
{
    if (m_icon.isNull()) {
        m_icon = [this]() -> QVariant {
            const auto iconPath = m_snap->icon();
            if (iconPath.isEmpty())
                return QStringLiteral("package-x-generic");

            if (!iconPath.startsWith(QLatin1Char('/')))
                return QUrl(iconPath);

            auto backend = qobject_cast<SnapBackend*>(parent());
            auto req = backend->client()->getIcon(packageName());
            connect(req, &QSnapdGetIconRequest::complete, this, &SnapResource::gotIcon);
            req->runAsync();
            return {};
        }();
    }
    return m_icon;
}

void SnapResource::gotIcon()
{
    auto req = qobject_cast<QSnapdGetIconRequest*>(sender());
    if (req->error()) {
        qWarning() << "icon error" << req->errorString();
        return;
    }

    auto icon = req->icon();

    QBuffer buffer;
    buffer.setData(icon->data());
    QImageReader reader(&buffer);

    auto theIcon = QVariant::fromValue<QImage>(reader.read());
    if (theIcon != m_icon) {
        m_icon = theIcon;
        iconChanged();
    }
}

QString SnapResource::installedVersion() const
{
    return m_snap->version();
}

QString SnapResource::license()
{
    return m_snap->license();
}

QString SnapResource::longDescription()
{
    return m_snap->description();
}

QString SnapResource::name() const
{
    return m_snap->title().isEmpty() ? m_snap->name() : m_snap->title();
}

QString SnapResource::origin() const
{
    return QStringLiteral("snappy:") + m_snap->channel();
}

QString SnapResource::packageName() const
{
    return m_snap->name();
}

QString SnapResource::section()
{
    return QStringLiteral("snap");
}

AbstractResource::State SnapResource::state()
{
    return m_state;
}

void SnapResource::setState(AbstractResource::State state)
{
    if (m_state != state) {
        m_state = state;
        Q_EMIT stateChanged();
    }
}

void SnapResource::fetchChangelog()
{
    QString log;
    emit changelogFetched(log);
}

void SnapResource::fetchScreenshots()
{
    QList<QUrl> screenshots;
    for(int i = 0, c = m_snap->screenshotCount(); i<c; ++i) {
        QScopedPointer<QSnapdScreenshot> screenshot(m_snap->screenshot(i));
        screenshots << QUrl(screenshot->url());
    }
    Q_EMIT screenshotsFetched(screenshots, screenshots);
}

void SnapResource::invokeApplication() const
{
    QProcess::startDetached(QStringLiteral("snap"), {QStringLiteral("run"), packageName()});
}

bool SnapResource::isTechnical() const
{
    return m_snap->snapType() != QLatin1String("app");
}

QUrl SnapResource::url() const
{
    //FIXME interim, until it has an appstreamId
    return QUrl(QStringLiteral("snap://") + packageName());
}

void SnapResource::setSnap(const QSharedPointer<QSnapdSnap>& snap)
{
    Q_ASSERT(snap->name() == m_snap->name());
    if (m_snap == snap)
        return;

    const bool newSize = m_snap->installedSize() != snap->installedSize() || m_snap->downloadSize() != snap->downloadSize();
    m_snap = snap;
    if (newSize)
        Q_EMIT sizeChanged();
}

QDate SnapResource::releaseDate() const
{
    return {};
}

class PlugsModel : public QStandardItemModel
{
public:
    enum Roles {
        PlugNameRole = Qt::UserRole + 1,
        SlotSnapRole,
        SlotNameRole
    };

    PlugsModel(QSnapdSnap* snap, SnapBackend* backend, QObject* parent)
        : QStandardItemModel(parent)
        , m_snap(snap)
        , m_backend(backend)
    {
        setItemRoleNames(roleNames().unite(
            { {Qt::CheckStateRole, "checked"} }
        ));

        auto req = backend->client()->getInterfaces();
        req->runSync();

        QHash<QString, QVector<QSnapdSlot*>> slotsForInterface;
        for (int i = 0; i<req->slotCount(); ++i) {
            const auto slot = req->slot(i);
            slot->setParent(this);
            slotsForInterface[slot->interface()].append(slot);

        }

        for (int i = 0; i<req->plugCount(); ++i) {
            const QScopedPointer<QSnapdPlug> plug(req->plug(i));
            if (plug->snap() == m_snap->name()) {
                for (auto slot: slotsForInterface[plug->interface()]) {
                    auto item = new QStandardItem;
                    if (plug->label().isEmpty())
                        item->setText(plug->name());
                    else
                        item->setText(i18n("%1 - %2", plug->name(), plug->label()));

                    item->setCheckable(true);
                    item->setCheckState(plug->connectionCount()>0 ? Qt::Checked : Qt::Unchecked);
                    item->setData(plug->name(), PlugNameRole);
                    item->setData(slot->snap(), SlotSnapRole);
                    item->setData(slot->name(), SlotNameRole);
                    appendRow(item);
                }
            }
        }
    }

private:
    bool setData(const QModelIndex & index, const QVariant & value, int role) override {
        if (role != Qt::CheckStateRole)
            return QStandardItemModel::setData(index, value, role);

        auto item = itemFromIndex(index);
        Q_ASSERT(item);
        const QString plugName = item->data(PlugNameRole).toString();
        const QString slotSnap = item->data(SlotSnapRole).toString();
        const QString slotName = item->data(SlotNameRole).toString();

        QSnapdRequest* req;

        if (item->checkState() == Qt::Checked) {
            req = m_backend->client()->connectInterface(m_snap->name(), plugName, slotSnap, slotName);
        } else {
            req = m_backend->client()->disconnectInterface(m_snap->name(), plugName, slotSnap, slotName);
        }
        req->runSync();
        if (req->error()) {
            qWarning() << "snapd error" << req->errorString();
        }
        return req->error() == QSnapdRequest::NoError;
    }

    QSnapdSnap* const m_snap;
    SnapBackend* const m_backend;
};

QAbstractItemModel* SnapResource::plugs(QObject* p)
{
    if (!isInstalled())
        return new QStandardItemModel(p);


    return new PlugsModel(m_snap.data(), qobject_cast<SnapBackend*>(parent()), p);
}
