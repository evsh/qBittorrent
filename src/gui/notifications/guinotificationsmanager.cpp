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

#include "guinotificationsmanager.h"

#include <QDesktopServices>

#include "base/notifications/notifier.h"
#include "app/application.h"

#if (defined(Q_OS_UNIX) && !defined(Q_OS_MAC)) && defined(QT_DBUS_LIB)
#include "base/notifications/dbusnotifier.h"
#else
#include "gui/notifications/systemtraynotifier.h"
#include "gui/mainwindow.h"
#endif

Notifications::GuiManager::GuiManager(QObject *parent)
    : Manager(GuiManager::createNotifier(), parent)
{
}

void Notifications::GuiManager::openUrl(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}

Notifications::Notifier* Notifications::GuiManager::createNotifier()
{
#if (defined(Q_OS_UNIX) && !defined(Q_OS_MAC)) && defined(QT_DBUS_LIB)
    return new Notifications::DBusNotifier(nullptr);
#else
    return new Notifications::SystemTrayNotifier(nullptr, static_cast<Application*>(
                                                     QCoreApplication::instance())->mainWindow()->getSystemTrayIcon());
#endif
}
