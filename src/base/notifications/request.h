/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2016 Eugene Shalygin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#ifndef NOTIFICATIONS_REQUEST_H
#define NOTIFICATIONS_REQUEST_H

#include <QMap>
#include <QVariant>
#include "base/bittorrent/infohash.h"

namespace BitTorrent
{
    class TorrentHandle;
}

namespace Notifications
{
    enum class Category
    {
        Generic,
        Download,
        Network
    };

    enum class Urgency
    {
        Low,
        Normal,
        High
    };

    enum class Severity
    {
        No,
        Information,
        Warning,
        Error
    };

    class Request
    {
    public:
        typedef Request this_type;

        static const QString defaultActionName;
        typedef QMap<QString,QString> ActionsMap;

        Request();

        QString title() const;
        QString message() const;
        Category category() const;
        Urgency urgency() const;
        Severity severity() const;
        BitTorrent::InfoHash torrent() const;
        const ActionsMap& actions() const;
        int timeout() const;
        const QVariant& userData() const;

        this_type& setTitle(const QString& title);
        this_type& setMessage(const QString& message);
        this_type& setCategory(Category category);
        this_type& setUrgency(Urgency urgency);
        this_type& setSeverity(Severity severity);
        this_type& setTorrent(BitTorrent::InfoHash& infoHash);
        this_type& setTorrent(BitTorrent::TorrentHandle* torrent);
        //! Timeout for notification, ms
        //! There are two special values:
        //! 0: infinite
        //! -1: default timeout
        this_type& setTimeout(int timeout);
        this_type& setUserData(const QVariant &data);

        this_type& addAction(const QString& id, const QString& label);

        void exec() const &;
        void exec() &&;

    private:
        QString m_title;
        QString m_message;
        Category m_category;
        Urgency m_urgency;
        Severity m_severity;
        BitTorrent::InfoHash m_torrentHash;
        ActionsMap m_actions;
        int m_timeout;
        QVariant m_userData;
    };
}

#endif // NOTIFICATIONS_REQUEST_H
