/***************************************************************************
 *   Copyright © 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>       *
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

#include "ResourcesModel.h"

#include "AbstractResource.h"
#include "resources/AbstractResourcesBackend.h"
#include "resources/AbstractBackendUpdater.h"
#include <ReviewsBackend/Rating.h>
#include <ReviewsBackend/AbstractReviewsBackend.h>
#include <Transaction/Transaction.h>
#include <DiscoverBackendsFactory.h>
#include "Transaction/TransactionModel.h"
#include "Category/CategoryModel.h"
#include "utils.h"
#include <QDebug>
#include <QCoreApplication>
#include <QThread>
#include <QAction>
#include <QMetaProperty>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

ResourcesModel *ResourcesModel::s_self = nullptr;

ResourcesModel *ResourcesModel::global()
{
    if(!s_self)
        s_self = new ResourcesModel;
    return s_self;
}

ResourcesModel::ResourcesModel(QObject* parent, bool load)
    : QObject(parent)
    , m_isFetching(false)
    , m_initializingBackends(0)
    , m_currentApplicationBackend(nullptr)
{
    init(load);
    connect(this, &ResourcesModel::allInitialized, this, &ResourcesModel::slotFetching);
    connect(this, &ResourcesModel::backendsChanged, this, &ResourcesModel::initApplicationsBackend);
}

void ResourcesModel::init(bool load)
{
    Q_ASSERT(!s_self);
    Q_ASSERT(QCoreApplication::instance()->thread()==QThread::currentThread());

    if(load)
        QMetaObject::invokeMethod(this, "registerAllBackends", Qt::QueuedConnection);


    m_updateAction = new QAction(this);
    m_updateAction->setIcon(QIcon::fromTheme(QStringLiteral("system-software-update")));
    m_updateAction->setText(i18nc("@action Checks the Internet for updates", "Check for Updates"));
    m_updateAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    connect(this, &ResourcesModel::fetchingChanged, m_updateAction, [this](bool fetching) {
        m_updateAction->setEnabled(!fetching);
    });
    connect(m_updateAction, &QAction::triggered, this, &ResourcesModel::checkForUpdates);

    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &QObject::deleteLater);
}

ResourcesModel::ResourcesModel(const QString& backendName, QObject* parent)
    : ResourcesModel(parent, false)
{
    s_self = this;
    registerBackendByName(backendName);
}

ResourcesModel::~ResourcesModel()
{
    s_self = nullptr;
    qDeleteAll(m_backends);
}

void ResourcesModel::addResourcesBackend(AbstractResourcesBackend* backend)
{
    Q_ASSERT(!m_backends.contains(backend));
    if(!backend->isValid()) {
        qWarning() << "Discarding invalid backend" << backend->name();
        CategoryModel::global()->blacklistPlugin(backend->name());
        backend->deleteLater();
        return;
    }

    m_backends += backend;
    if(!backend->isFetching()) {
        if (backend->updatesCount() > 0)
            emit updatesCountChanged();
    } else {
        m_initializingBackends++;
    }

    connect(backend, &AbstractResourcesBackend::fetchingChanged, this, &ResourcesModel::callerFetchingChanged);
    connect(backend, &AbstractResourcesBackend::allDataChanged, this, &ResourcesModel::updateCaller);
    connect(backend, &AbstractResourcesBackend::resourcesChanged, this, &ResourcesModel::resourceDataChanged);
    connect(backend, &AbstractResourcesBackend::updatesCountChanged, this, &ResourcesModel::updatesCountChanged);
    connect(backend, &AbstractResourcesBackend::resourceRemoved, this, &ResourcesModel::resourceRemoved);
    connect(backend, &AbstractResourcesBackend::passiveMessage, this, &ResourcesModel::passiveMessage);
    connect(backend->backendUpdater(), &AbstractBackendUpdater::progressingChanged, this, &ResourcesModel::slotFetching);

    if(m_initializingBackends==0)
        emit allInitialized();
    else
        slotFetching();
}

void ResourcesModel::callerFetchingChanged()
{
    AbstractResourcesBackend* backend = qobject_cast<AbstractResourcesBackend*>(sender());

    if (!backend->isValid()) {
        qWarning() << "Discarding invalid backend" << backend->name();
        int idx = m_backends.indexOf(backend);
        Q_ASSERT(idx>=0);
        m_backends.removeAt(idx);
        Q_EMIT backendsChanged();
        CategoryModel::global()->blacklistPlugin(backend->name());
        backend->deleteLater();
        return;
    }

    if(backend->isFetching()) {
        m_initializingBackends++;
        slotFetching();
    } else {
        m_initializingBackends--;
        if(m_initializingBackends==0)
            emit allInitialized();
        else
            slotFetching();
    }
}

void ResourcesModel::updateCaller(const QVector<QByteArray>& properties)
{
    AbstractResourcesBackend* backend = qobject_cast<AbstractResourcesBackend*>(sender());
    
    Q_EMIT backendDataChanged(backend, properties);
}

QVector< AbstractResourcesBackend* > ResourcesModel::backends() const
{
    return m_backends;
}

int ResourcesModel::updatesCount() const
{
    int ret = 0;

    foreach(AbstractResourcesBackend* backend, m_backends) {
        ret += backend->updatesCount();
    }

    return ret;
}

bool ResourcesModel::hasSecurityUpdates() const
{
    bool ret = false;

    foreach(AbstractResourcesBackend* backend, m_backends) {
        ret |= backend->hasSecurityUpdates();
    }

    return ret;
}

void ResourcesModel::installApplication(AbstractResource* app)
{
    TransactionModel::global()->addTransaction(app->backend()->installApplication(app));
}

void ResourcesModel::installApplication(AbstractResource* app, const AddonList& addons)
{
    TransactionModel::global()->addTransaction(app->backend()->installApplication(app, addons));
}

void ResourcesModel::removeApplication(AbstractResource* app)
{
    TransactionModel::global()->addTransaction(app->backend()->removeApplication(app));
}

void ResourcesModel::registerAllBackends()
{
    DiscoverBackendsFactory f;
    const auto backends = f.allBackends();
    if(m_initializingBackends==0 && backends.isEmpty()) {
        qWarning() << "Couldn't find any backends";
        emit allInitialized();
    } else {
        foreach(AbstractResourcesBackend* b, backends) {
            addResourcesBackend(b);
        }
        emit backendsChanged();
    }
}

void ResourcesModel::registerBackendByName(const QString& name)
{
    DiscoverBackendsFactory f;
    for(auto b : f.backend(name))
        addResourcesBackend(b);

    emit backendsChanged();
}

bool ResourcesModel::isFetching() const
{
    return m_isFetching;
}

void ResourcesModel::slotFetching()
{
    bool newFetching = false;
    foreach(AbstractResourcesBackend* b, m_backends) {
        // isFetching should sort of be enough. However, sometimes the backend itself
        // will still be operating on things, which from a model point of view would
        // still mean something going on. So, interpret that as fetching as well, for
        // the purposes of this property.
        if(b->isFetching() || (b->backendUpdater() && b->backendUpdater()->isProgressing())) {
            newFetching = true;
            break;
        }
    }
    if (newFetching != m_isFetching) {
        m_isFetching = newFetching;
        Q_EMIT fetchingChanged(m_isFetching);
    }
}

bool ResourcesModel::isBusy() const
{
    return TransactionModel::global()->rowCount() > 0;
}

bool ResourcesModel::isExtended(const QString& id)
{
    bool ret = true;
    foreach (AbstractResourcesBackend* backend, m_backends) {
        ret = backend->extends().contains(id);
        if (ret)
            break;
    }
    return ret;
}

AggregatedResultsStream::AggregatedResultsStream(const QSet<ResultsStream*>& streams)
    : ResultsStream(QStringLiteral("AggregatedResultsStream"))
{
    Q_ASSERT(!streams.contains(nullptr));
    if (streams.isEmpty()) {
        qWarning() << "no streams to aggregate!!";
        QTimer::singleShot(0, this, &AggregatedResultsStream::clear);
    }

    for (auto stream: streams) {
        connect(stream, &ResultsStream::resourcesFound, this, &AggregatedResultsStream::addResults);
        connect(stream, &QObject::destroyed, this, &AggregatedResultsStream::destruction);
        m_streams << stream;
    }

    m_delayedEmission.setInterval(0);
    connect(&m_delayedEmission, &QTimer::timeout, this, &AggregatedResultsStream::emitResults);
}

void AggregatedResultsStream::addResults(const QVector<AbstractResource *>& res)
{
    m_results += res;

    m_delayedEmission.start();
}

void AggregatedResultsStream::emitResults()
{
    if (!m_results.isEmpty()) {
        Q_EMIT resourcesFound(m_results);
        m_results.clear();
    }
    m_delayedEmission.setInterval(m_delayedEmission.interval() + 100);
    m_delayedEmission.stop();
}

void AggregatedResultsStream::destruction(QObject* obj)
{
    m_streams.remove(obj);
    clear();
}

void AggregatedResultsStream::clear()
{
    if (m_streams.isEmpty()) {
        emitResults();
        Q_EMIT finished();
        deleteLater();
    }
}

AggregatedResultsStream* ResourcesModel::search(const AbstractResourcesBackend::Filters& search)
{
    if (search.isEmpty()) {
        return new AggregatedResultsStream ({new ResultsStream(QStringLiteral("emptysearch"), {})});
    }

    auto streams = kTransform<QSet<ResultsStream*>>(m_backends, [search](AbstractResourcesBackend* backend){ return backend->search(search); });
    return new AggregatedResultsStream(streams);
}

AbstractResource* ResourcesModel::resourceForFile(const QUrl& file)
{
    AbstractResource* ret = nullptr;
    foreach(auto backend, m_backends) {
        ret = backend->resourceForFile(file);
        if (ret)
            break;
    }
    return ret;
}

void ResourcesModel::checkForUpdates()
{
    for(auto backend: qAsConst(m_backends))
        backend->checkForUpdates();
}

QVector<AbstractResourcesBackend *> ResourcesModel::applicationBackends() const
{
    return kFilter<QVector<AbstractResourcesBackend*>>(m_backends, [](AbstractResourcesBackend* b){ return b->hasApplications(); });
}

QVariantList ResourcesModel::applicationBackendsVariant() const
{
    return kTransform<QVariantList>(applicationBackends(), [](AbstractResourcesBackend* b) {return QVariant::fromValue<QObject*>(b);});
}

AbstractResourcesBackend* ResourcesModel::currentApplicationBackend() const
{
    return m_currentApplicationBackend;
}

void ResourcesModel::setCurrentApplicationBackend(AbstractResourcesBackend* backend, bool write)
{
    if (backend != m_currentApplicationBackend) {
        if (write) {
            KConfigGroup settings(KSharedConfig::openConfig(), "ResourcesModel");
            if (backend)
                settings.writeEntry("currentApplicationBackend", backend->name());
            else
                settings.deleteEntry("currentApplicationBackend");
        }

        qDebug() << "setting currentApplicationBackend" << backend;
        m_currentApplicationBackend = backend;
        Q_EMIT currentApplicationBackendChanged(backend);
    }
}

void ResourcesModel::initApplicationsBackend()
{
    KConfigGroup settings(KSharedConfig::openConfig(), "ResourcesModel");
    const QString name = settings.readEntry<QString>("currentApplicationBackend", QStringLiteral("packagekit-backend"));

    const auto backends = applicationBackends();
    auto idx = kIndexOf(backends, [name](AbstractResourcesBackend* b) { return b->name() == name; });
    if (idx<0) {
        idx = kIndexOf(backends, [](AbstractResourcesBackend* b) { return b->hasApplications(); });
        qDebug() << "falling back applications backend to" << idx;
    }
    setCurrentApplicationBackend(backends.value(idx, nullptr), false);
}
