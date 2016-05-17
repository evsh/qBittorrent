/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2015  Vladimir Golovnev <glassez@yandex.ru>
 * Copyright (C) 2011  Christophe Dumez <chris@qbittorrent.org>
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

#include "guiiconprovider.h"
#include "base/preferences.h"

#include <QIcon>
#if (defined(Q_OS_UNIX) && !defined(Q_OS_MAC))
#include <QDir>
#include <QFile>
#endif

GuiIconProvider::GuiIconProvider(QObject *parent)
    : IconProvider(parent)
{
    configure();
    connect(Preferences::instance(), SIGNAL(changed()), SLOT(configure()));
}

GuiIconProvider::~GuiIconProvider() {}

void GuiIconProvider::initInstance()
{
    if (!m_instance)
        m_instance = new GuiIconProvider;
}

GuiIconProvider *GuiIconProvider::instance()
{
    return static_cast<GuiIconProvider *>(m_instance);
}

QIcon GuiIconProvider::getIcon(const QString &iconId)
{
#if (defined(Q_OS_UNIX) && !defined(Q_OS_MAC))
    if (m_useSystemTheme) {
        return QIcon::fromTheme(iconId, QIcon(IconProvider::getIconPath(iconId)));
    }
#endif
    return QIcon(IconProvider::getIconPath(iconId));
}

QIcon GuiIconProvider::getFlagIcon(const QString &countryIsoCode)
{
    if (countryIsoCode.isEmpty()) return QIcon();
    return QIcon(":/icons/flags/" + countryIsoCode.toLower() + ".png");
}

QString GuiIconProvider::getIconPath(const QString &iconId)
{
#if (defined(Q_OS_UNIX) && !defined(Q_OS_MAC))
    if (m_useSystemTheme) {
        QString path = QDir::temp().absoluteFilePath(iconId + ".png");
        if (!QFile::exists(path)) {
            const QIcon icon = QIcon::fromTheme(iconId);
            if (!icon.isNull())
                icon.pixmap(32).save(path);
            else
                path = IconProvider::getIconPath(iconId);
        }

        return path;
    }
#endif
    return IconProvider::getIconPath(iconId);
}


void GuiIconProvider::configure()
{
#if (defined(Q_OS_UNIX) && !defined(Q_OS_MAC))
    m_useSystemTheme = Preferences::instance()->useSystemIconTheme();
#endif
}
