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

#include "FwupdBackend.h"
#include "FwupdResource.h"
#include "FwupdReviewsBackend.h"
#include "FwupdTransaction.h"
#include "FwupdSourcesBackend.h"
#include <resources/StandardBackendUpdater.h>
#include <resources/SourcesModel.h>
#include <Transaction/Transaction.h>

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QAction>

DISCOVER_BACKEND_PLUGIN(FwupdBackend)

#define STRING(s) #s
    
FwupdBackend::FwupdBackend(QObject* parent)
    : AbstractResourcesBackend(parent)
    , m_updater(new StandardBackendUpdater(this))
    , m_reviews(new FwupdReviewsBackend(this))
    , m_fetching(true)
    , m_startElements(120)
{

    QTimer::singleShot(500, this, &FwupdBackend::toggleFetching);
    connect(m_reviews, &FwupdReviewsBackend::ratingsReady, this, &AbstractResourcesBackend::emitRatingsReady);
    connect(m_updater, &StandardBackendUpdater::updatesCountChanged, this, &FwupdBackend::updatesCountChanged);


    client = fwupd_client_new ();
    to_download = g_ptr_array_new_with_free_func (g_free);
    to_ignore = g_ptr_array_new_with_free_func (g_free);

    /* use a custom user agent to provide the fwupd version */
    user_agent = fwupd_build_user_agent (STRING(PROJECT_NAME),STRING(PROJECT_VERSION));
    soup_session = soup_session_new_with_options (SOUP_SESSION_USER_AGENT, user_agent,
                                  SOUP_SESSION_TIMEOUT, 10,
                                  NULL);
    soup_session_remove_feature_by_type (soup_session,
                             SOUP_TYPE_CONTENT_DECODER);

    populate(QStringLiteral("Devices"));
    if (!m_fetching)
        m_reviews->initialize();

    SourcesModel::global()->addSourcesBackend(new FwupdSourcesBackend(this));
}

FwupdBackend::~FwupdBackend()
{
    g_object_unref (client);
    g_ptr_array_unref (to_download);
    g_ptr_array_unref (to_ignore);
}

void FwupdBackend::populate(const QString& n)
{
    g_autoptr(GPtrArray) remotes = NULL;
    g_autoptr(GCancellable) cancellable = g_cancellable_new();
    g_autoptr(GError) error = NULL;
    g_autoptr(GPtrArray) devices = NULL;

    /* get devices */
    devices = fwupd_client_get_devices (client, cancellable, &error);
    //releases = fwupd_client_get_releases (client,gs_fwupd_app_get_device_id (app),cancellable,&error_local);
    
    for (guint i = 0; i < devices->len; i++) {
        FwupdDevice *device = (FwupdDevice *)g_ptr_array_index (devices, i);
        const QString name = QLatin1String(fwupd_device_get_name(device));
        FwupdResource* res = new FwupdResource(name, false, this);
       // res->setSize(100+(m_startElements-i));
       // res->setState(AbstractResource::State(1+(i%3)));
        res->addCategories(n);
        res->setSummary(QLatin1String(fwupd_device_get_summary(device)));
        res->setVendor(QLatin1String(fwupd_device_get_vendor(device)));
        res->setVersion(QLatin1String(fwupd_device_get_version(device)));
        res->setDescription(QLatin1String(fwupd_device_get_description(device)));
        m_resources.insert(name.toLower(), res);
        
        connect(res, &FwupdResource::stateChanged, this, &FwupdBackend::updatesCountChanged);
    }

   /* const int start = m_resources.count();
    for(int i=start; i<start+m_startElements; i++) {
        const QString name = n+QLatin1Char(' ')+QString::number(i);
        FwupdResource* res = new FwupdResource(name, false, this);
        res->setSize(100+(m_startElements-i));
        res->setState(AbstractResource::State(1+(i%3)));
        m_resources.insert(name.toLower(), res);
        connect(res, &FwupdResource::stateChanged, this, &FwupdBackend::updatesCountChanged);
    }

    for(int i=start; i<start+m_startElements; i++) {
        const QString name = QStringLiteral("techie")+QString::number(i);
        FwupdResource* res = new FwupdResource(name, true, this);
        res->setState(AbstractResource::State(1+(i%3)));
        res->setSize(300+(m_startElements-i));
        m_resources.insert(name, res);
        connect(res, &FwupdResource::stateChanged, this, &FwupdBackend::updatesCountChanged);
    }*/
}

void FwupdBackend::toggleFetching()
{
    m_fetching = !m_fetching;
//     qDebug() << "fetching..." << m_fetching;
    emit fetchingChanged();
    if (!m_fetching)
        m_reviews->initialize();
}

int FwupdBackend::updatesCount() const
{
    return m_updater->updatesCount();
}

ResultsStream* FwupdBackend::search(const AbstractResourcesBackend::Filters& filter)
{
    QVector<AbstractResource*> ret;
    if (!filter.resourceUrl.isEmpty() && filter.resourceUrl.scheme() == QLatin1String("fwupd"))
        return findResourceByPackageName(filter.resourceUrl);
    else foreach(AbstractResource* r, m_resources) {
        if(r->name().contains(filter.search, Qt::CaseInsensitive) || r->comment().contains(filter.search, Qt::CaseInsensitive))
            ret += r;
    }
    return new ResultsStream(QStringLiteral("FwupdStream"), ret);
}

ResultsStream * FwupdBackend::findResourceByPackageName(const QUrl& search)
{
    auto res = search.scheme() == QLatin1String("fwupd") ? m_resources.value(search.host().replace(QLatin1Char('.'), QLatin1Char(' '))) : nullptr;
    if (!res) {
        return new ResultsStream(QStringLiteral("FwupdStream"), {});
    } else
        return new ResultsStream(QStringLiteral("FwupdStream"), { res });
}

AbstractBackendUpdater* FwupdBackend::backendUpdater() const
{
    return m_updater;
}

AbstractReviewsBackend* FwupdBackend::reviewsBackend() const
{
    return m_reviews;
}

Transaction* FwupdBackend::installApplication(AbstractResource* app, const AddonList& addons)
{
    return new FwupdTransaction(qobject_cast<FwupdResource*>(app), addons, Transaction::InstallRole);
}

Transaction* FwupdBackend::installApplication(AbstractResource* app)
{
	return new FwupdTransaction(qobject_cast<FwupdResource*>(app), Transaction::InstallRole);
}

Transaction* FwupdBackend::removeApplication(AbstractResource* app)
{
	return new FwupdTransaction(qobject_cast<FwupdResource*>(app), Transaction::RemoveRole);
}

void FwupdBackend::checkForUpdates()
{
    if(m_fetching)
        return;
    toggleFetching();
    populate(QStringLiteral("Fwupd"));
    QTimer::singleShot(500, this, &FwupdBackend::toggleFetching);
    qDebug() << "FwupdBackend::checkForUpdates";
}

AbstractResource * FwupdBackend::resourceForFile(const QUrl& path)
{
    FwupdResource* res = new FwupdResource(path.fileName(), true, this);
    res->setSize(666);
    res->setState(AbstractResource::None);
    m_resources.insert(res->packageName(), res);
    connect(res, &FwupdResource::stateChanged, this, &FwupdBackend::updatesCountChanged);
    return res;
}

QString FwupdBackend::displayName() const
{
    return QStringLiteral("Fwupd");
}

bool FwupdBackend::hasApplications() const
{
    return true;
}

#include "FwupdBackend.moc"
