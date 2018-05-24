/***************************************************************************
 *   Copyright ?? 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>       *
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

#ifndef RESOURCESMODEL_H
#define RESOURCESMODEL_H

#include <QSet>
#include <QVector>
#include <QTimer>

#include "discovercommon_export.h"
#include "AbstractResourcesBackend.h"

class QAction;

class DISCOVERCOMMON_EXPORT AggregatedResultsStream : public ResultsStream
{
Q_OBJECT
public:
    AggregatedResultsStream(const QSet<ResultsStream*>& streams);

Q_SIGNALS:
    void finished();

private:
    void addResults(const QVector<AbstractResource*>& res);
    void emitResults();
    void destruction(QObject* obj);
    void clear();

    QSet<QObject*> m_streams;
    QVector<AbstractResource*> m_results;
    QTimer m_delayedEmission;
};

class DISCOVERCOMMON_EXPORT ResourcesModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int updatesCount READ updatesCount NOTIFY updatesCountChanged)
    Q_PROPERTY(bool hasSecurityUpdates READ hasSecurityUpdates NOTIFY updatesCountChanged)
    Q_PROPERTY(bool isFetching READ isFetching NOTIFY fetchingChanged)
    Q_PROPERTY(QVariantList applicationBackends READ applicationBackendsVariant NOTIFY backendsChanged)
    Q_PROPERTY(AbstractResourcesBackend* currentApplicationBackend READ currentApplicationBackend WRITE setCurrentApplicationBackend NOTIFY currentApplicationBackendChanged)
    Q_PROPERTY(QAction* updateAction READ updateAction CONSTANT)
    public:
        /** This constructor should be only used by unit tests.
         *  @p backendName defines what backend will be loaded when the backend is constructed.
         */
        explicit ResourcesModel(const QString& backendName, QObject* parent = nullptr);
        static ResourcesModel* global();
        ~ResourcesModel() override;
        
        QVector< AbstractResourcesBackend* > backends() const;
        int updatesCount() const;
        bool hasSecurityUpdates() const;
        
        bool isBusy() const;
        bool isFetching() const;
        
        Q_SCRIPTABLE bool isExtended(const QString &id);

        AggregatedResultsStream* search(const AbstractResourcesBackend::Filters &search);
        AbstractResource* resourceForFile(const QUrl &/*url*/);
        void checkForUpdates();

        QVariantList applicationBackendsVariant() const;
        QVector<AbstractResourcesBackend*> applicationBackends() const;
        void setCurrentApplicationBackend(AbstractResourcesBackend* backend, bool writeConfig = true);
        AbstractResourcesBackend* currentApplicationBackend() const;

        QAction* updateAction() const { return m_updateAction; }

    public Q_SLOTS:
        void installApplication(AbstractResource* app, const AddonList& addons);
        void installApplication(AbstractResource* app);
        void removeApplication(AbstractResource* app);

    Q_SIGNALS:
        void fetchingChanged(bool isFetching);
        void allInitialized();
        void backendsChanged();
        void updatesCountChanged();
        void backendDataChanged(AbstractResourcesBackend* backend, const QVector<QByteArray>& properties);
        void resourceDataChanged(AbstractResource* resource, const QVector<QByteArray>& properties);
        void resourceRemoved(AbstractResource* resource);
        void passiveMessage(const QString &message);
        void currentApplicationBackendChanged(AbstractResourcesBackend* currentApplicationBackend);

    private Q_SLOTS:
        void callerFetchingChanged();
        void updateCaller(const QVector<QByteArray>& properties);
        void registerAllBackends();

    private:
        ///@p initialize tells if all backends load will be triggered on construction
        explicit ResourcesModel(QObject* parent=nullptr, bool load = true);
        void init(bool load);
        void addResourcesBackend(AbstractResourcesBackend* backend);
        void registerBackendByName(const QString& name);
        void initApplicationsBackend();
        void slotFetching();

        bool m_isFetching;
        QVector< AbstractResourcesBackend* > m_backends;
        int m_initializingBackends;
        QAction* m_updateAction = nullptr;
        AbstractResourcesBackend* m_currentApplicationBackend;

        static ResourcesModel* s_self;
};

#endif // RESOURCESMODEL_H
