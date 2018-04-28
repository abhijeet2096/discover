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

#ifndef DUMMYREVIEWSBACKEND_H
#define DUMMYREVIEWSBACKEND_H

#include <ReviewsBackend/AbstractReviewsBackend.h>
#include <ReviewsBackend/ReviewsModel.h>
#include <QMap>

class FwupdBackend;
class FwupdReviewsBackend : public AbstractReviewsBackend
{
Q_OBJECT
public:
    explicit FwupdReviewsBackend(FwupdBackend* parent = nullptr);

    QString userName() const override { return QStringLiteral("fwupd"); }
    void login() override {}
    void logout() override {}
    void registerAndLogin() override {}

    Rating* ratingForApplication(AbstractResource* app) const override;
    bool hasCredentials() const override { return false; }
    void deleteReview(Review*) override {}
    void fetchReviews(AbstractResource* app, int page = 1) override;
    bool isFetching() const override { return false; }
    void submitReview(AbstractResource*, const QString&, const QString&, const QString&) override;
    void flagReview(Review*, const QString&, const QString&) override {}
    void submitUsefulness(Review*, bool) override;

    void initialize();
    bool isResourceSupported(AbstractResource * res) const override;

Q_SIGNALS:
    void ratingsReady();

private:
    QHash<AbstractResource*, Rating*> m_ratings;
};

#endif // DUMMYREVIEWSBACKEND_H
