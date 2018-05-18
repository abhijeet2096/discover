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

#ifndef FWUPDBACKEND_H
#define FWUPDBACKEND_H

#include <resources/AbstractResourcesBackend.h>
#include <QVariantList>

extern "C" {
#include <fwupd.h>
}
#include <fcntl.h>
#include <gio/gio.h>
#include <gio-unix-2.0/gio/gunixfdlist.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>


#include <libsoup/soup-request-http.h>
#include <libsoup/soup-requester.h>
#include <libsoup/soup.h>



class QAction;
class FwupdReviewsBackend;
class StandardBackendUpdater;
class FwupdResource;
class FwupdBackend : public AbstractResourcesBackend
{
Q_OBJECT
Q_PROPERTY(int startElements MEMBER m_startElements)
public:
    explicit FwupdBackend(QObject* parent = nullptr);
    ~FwupdBackend();

    int updatesCount() const override;
    AbstractBackendUpdater* backendUpdater() const override;
    AbstractReviewsBackend* reviewsBackend() const override;
    ResultsStream* search(const AbstractResourcesBackend::Filters & search) override;
    ResultsStream * findResourceByPackageName(const QUrl& search) override;
    QHash<QString, FwupdResource*> resources() const { return m_resources; }
    bool isValid() const override { return true; } // No external file dependencies that could cause runtime errors

    Transaction* installApplication(AbstractResource* app) override;
    Transaction* installApplication(AbstractResource* app, const AddonList& addons) override;
    Transaction* removeApplication(AbstractResource* app) override;
    bool isFetching() const override { return m_fetching; }
    AbstractResource * resourceForFile(const QUrl & ) override;
    void checkForUpdates() override;
    QString displayName() const override;
    bool hasApplications() const override;
    FwupdClient *client;
    GPtrArray *to_download;
    GPtrArray *to_ignore;


public Q_SLOTS:
    void toggleFetching();

private:
    void populate(const QString& name);

    QHash<QString, FwupdResource*> m_resources;
    StandardBackendUpdater* m_updater;
    FwupdReviewsBackend* m_reviews;
    bool m_fetching;
    int m_startElements;

    
    g_autofree gchar *user_agent = NULL;
    g_autoptr(SoupSession) soup_session = NULL;

};

#endif // FWUPDBACKEND_H
