/***************************************************************************
 *   Copyright © 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>       *
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

#include "FwupdSourcesBackend.h"
#include <QDebug>
#include <QAction>

FwupdSourcesBackend::FwupdSourcesBackend(AbstractResourcesBackend * parent)
    : AbstractSourcesBackend(parent)
    , m_sources(new QStandardItemModel(this))
    , m_testAction(new QAction(QIcon::fromTheme(QStringLiteral("kalgebra")), QStringLiteral("FwupdAction"), this))
{
    for (int i = 0; i<10; ++i)
        addSource(QStringLiteral("FwupdSource%1").arg(i));

    connect(m_testAction, &QAction::triggered, [](){ qDebug() << "action triggered!"; });
    connect(m_sources, &QStandardItemModel::itemChanged, this, [](QStandardItem* item) { qDebug() << "FwupdSource changed" << item << item->checkState(); });
}

QAbstractItemModel* FwupdSourcesBackend::sources()
{
    return m_sources;
}

bool FwupdSourcesBackend::addSource(const QString& id)
{
    if (id.isEmpty())
        return false;

    QStandardItem* it = new QStandardItem(id);
    it->setData(QVariant(id + QLatin1Char(' ') + id), Qt::ToolTipRole);
    it->setCheckable(true);
    it->setCheckState(Qt::Checked);
    m_sources->appendRow(it);
    return true;
}

bool FwupdSourcesBackend::removeSource(const QString& id)
{
    QList<QStandardItem*> items = m_sources->findItems(id);
    if (items.count()==1) {
        m_sources->removeRow(items.first()->row());
    } else {
        qWarning() << "couldn't find " << id  << items;
    }
    return items.count()==1;
}

QList<QAction*> FwupdSourcesBackend::actions() const
{
    return QList<QAction*>() << m_testAction;
}

