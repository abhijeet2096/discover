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

#ifndef DUMMYRESOURCE_H
#define DUMMYRESOURCE_H

#include <resources/AbstractResource.h>

class AddonList;
class FwupdResource : public AbstractResource
{
Q_OBJECT
public:
    explicit FwupdResource(QString  name, bool isTechnical, AbstractResourcesBackend* parent);

    QList<PackageState> addonsInformation() override;
    QString section() override;
    QString origin() const override;
    QString longDescription() override;
    QString availableVersion() const override;
    QString installedVersion() const override;
    QString license() override;
    int size() override;
    QUrl homepage() override;
    QUrl helpURL() override;
    QUrl bugURL() override;
    QUrl donationURL() override;
    QStringList categories() override;
    AbstractResource::State state() override;
    QVariant icon() const override;
    QString comment() override;
    QString name() const override;
    QString packageName() const override;
    bool isTechnical() const override { return m_isTechnical; }
    bool canExecute() const override { return true; }
    void invokeApplication() const override;
    void fetchChangelog() override;
    void fetchScreenshots() override;
    QUrl url() const override;
    void setState(State state);
    void setSize(int size) { m_size = size; }
    void setAddons(const AddonList& addons);

    void setAddonInstalled(const QString& addon, bool installed);
    QString sourceIcon() const override { return QStringLiteral("player-time"); }
    QDate releaseDate() const override { return {}; }

public:
    QString m_name;
    AbstractResource::State m_state;
    QList<QUrl> m_screenshots;
    QList<QUrl> m_screenshotThumbnails;
    QString m_iconName;
    QList<PackageState> m_addons;
    bool m_isTechnical;
    int m_size;
};

#endif // DUMMYRESOURCE_H
