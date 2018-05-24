/***************************************************************************
 *   Copyright © 2013 Lukas Appelhans <l.appelhans@gmx.de>                 *
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
#ifndef PACKAGEKITUPDATER_H
#define PACKAGEKITUPDATER_H

#include <resources/AbstractBackendUpdater.h>
#include "PackageKitBackend.h"
#include <PackageKit/Transaction>

class PackageKitUpdater : public AbstractBackendUpdater
{
    Q_OBJECT
    public:
        explicit PackageKitUpdater(PackageKitBackend * parent = nullptr);
        ~PackageKitUpdater() override;
        
        void prepare() override;
        
        bool hasUpdates() const override;
        qreal progress() const override;

        void setProgressing(bool progressing);
        
        void removeResources(const QList<AbstractResource*>& apps) override;
        void addResources(const QList<AbstractResource*>& apps) override;
        QList<AbstractResource*> toUpdate() const override;
        bool isMarked(AbstractResource* res) const override;
        QDateTime lastUpdate() const override;
        bool isCancelable() const override;
        bool isProgressing() const override;
        void fetchChangelog() const override;
        double updateSize() const override;
        quint64 downloadSpeed() const override;

        void proceed() override;

    public Q_SLOTS:
        ///must be implemented if ever isCancelable is true
        void cancel() override;
        void start() override;
    
    private Q_SLOTS:
        void errorFound(PackageKit::Transaction::Error err, const QString& error);
        void mediaChange(PackageKit::Transaction::MediaType media, const QString& type, const QString& text);
        void requireRestart(PackageKit::Transaction::Restart restart, const QString& p);
        void eulaRequired(const QString &eulaID, const QString &packageID, const QString &vendor, const QString &licenseAgreement);
        void finished(PackageKit::Transaction::Exit exit, uint);
        void cancellableChanged();
        void percentageChanged();
        void updateDetail(const QString& packageID, const QStringList& updates, const QStringList& obsoletes, const QStringList& vendorUrls,
                                      const QStringList& bugzillaUrls, const QStringList& cveUrls, PackageKit::Transaction::Restart restart, const QString& updateText,
                                      const QString& changelog, PackageKit::Transaction::UpdateState state, const QDateTime& issued, const QDateTime& updated);
        void packageResolved(PackageKit::Transaction::Info info, const QString& packageId);
        void repoSignatureRequired(const QString &packageID,
                                    const QString &repoName,
                                    const QString &keyUrl,
                                    const QString &keyUserid,
                                    const QString &keyId,
                                    const QString &keyFingerprint,
                                    const QString &keyTimestamp,
                                    PackageKit::Transaction::SigType type);

    private:
        void processProceedFunction();
        void itemProgress(const QString &itemID, PackageKit::Transaction::Status status, uint percentage);
        void fetchLastUpdateTime();
        void lastUpdateTimeReceived(QDBusPendingCallWatcher* w);
        void setupTransaction(PackageKit::Transaction::TransactionFlags flags);
        QSet<QString> involvedPackages(const QSet<AbstractResource*>& packages) const;
        QSet<AbstractResource*> packagesForPackageId(const QSet<QString>& packages) const;

        QPointer<PackageKit::Transaction> m_transaction;
        PackageKitBackend * const m_backend;
        QSet<AbstractResource*> m_toUpgrade;
        QSet<AbstractResource*> m_allUpgradeable;
        bool m_isCancelable;
        bool m_isProgressing;
        int m_percentage;
        QDateTime m_lastUpdate;
        QStringList m_packagesRemoved;
        QVector<std::function<PackageKit::Transaction*()>> m_proceedFunctions;
};


#endif
