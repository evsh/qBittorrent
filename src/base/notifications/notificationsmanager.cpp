#include "notificationsmanager.h"

#include "base/settingsstorage.h"
#include "base/bittorrent/session.h"
#include "base/bittorrent/torrenthandle.h"
#include "dummynotifier.h"
#include "request.h"

#include <QDir>
#include <QDebug>
#include <QUrl>
#include <QProcess>

#if (defined(Q_OS_UNIX) && !defined(Q_OS_MAC)) && defined(QT_DBUS_LIB)
#include "base/notifications/dbusnotifier.h"
#endif

namespace
{
#define SETTINGS_KEY(name) "Notifications/" name
    // Notifications properties keys
    const QString KEY_NOTIFICATIONS_ENABLED = SETTINGS_KEY("Enabled");
    const QString KEY_NOTIFICATIONS_TORRENTADDED = SETTINGS_KEY("TorrentAdded");


    const QLatin1String ACTION_NAME_OPEN_FINISHED_TORRENT("document-open"); // it must be named as the corresponding FDO icon

    QUrl getUrlForTorrentOpen(const BitTorrent::TorrentHandle *h)
    {
        if (h->filesCount() == 1)   // we open the single torrent file
            return QUrl::fromLocalFile(QDir(h->savePath()).absoluteFilePath(h->filePath(0)));
        else   // otherwise we open top directory
            return QUrl::fromLocalFile(h->rootPath());
    }
}

Notifications::Manager*Notifications::Manager::m_instance = nullptr;

Notifications::Manager::Manager(Notifier *notifier, QObject *parent)
    : QObject {parent}
{
    resetNotifier(notifier);
    connectSlots();
}

void Notifications::Manager::setInstance(Notifications::Manager::this_type *ptr)
{
    m_instance = ptr;
}

Notifications::Manager::~Manager()
{
}

Notifications::Manager* Notifications::Manager::instance()
{
    return m_instance;
}

void Notifications::Manager::handleAddTorrentFailure(const QString &error) const
{
    Request()
    .setTitle(tr("Error"))
    .setMessage(tr("Failed to add torrent: %1").arg(error))
    .setCategory(Category::Generic)
    .setSeverity(Severity::Error)
    .setUrgency(Urgency::High)
    .setTimeout(0)
    .exec();
}

void Notifications::Manager::handleTorrentFinished(BitTorrent::TorrentHandle*const torrent) const
{
    Request()
    .setTitle(tr("Download completion"))
    .setMessage(tr("%1 has finished downloading.", "e.g: xxx.avi has finished downloading.")
                .arg(torrent->name()))
    .setCategory(Category::Download)
    .setTorrent(torrent)
    .setSeverity(Severity::Information)
    .setTimeout(0)
    .addAction(ACTION_NAME_OPEN_FINISHED_TORRENT, tr("Open", "Open donwloaded torrent"))
    .addAction(ACTION_NAME_DEFAULT, tr("View", "View torrent"))
    .exec();
}

void Notifications::Manager::handleFullDiskError(BitTorrent::TorrentHandle*const torrent, QString msg) const
{
    Request()
    .setTitle(tr("I/O Error", "i.e: Input/Output Error"))
    .setMessage(tr("An I/O error occurred for torrent %1.\n Reason: %2",
                   "e.g: An error occurred for torrent xxx.avi.\n Reason: disk is full.")
                .arg(torrent->name()).arg(msg))
    .setCategory(Category::Download)
    .setTorrent(torrent)
    .setSeverity(Severity::Error)
    .setUrgency(Urgency::High)
    .setTimeout(0)
    .exec();
}

void Notifications::Manager::handleDownloadFromUrlFailure(QString url, QString reason) const
{
    Request()
    .setTitle(tr("Url download error"))
    .setMessage(tr("Couldn't download file at url: %1, reason: %2.").arg(url).arg(reason))
    .setCategory(Category::Download)
    .setSeverity(Severity::Error)
    .setUrgency(Urgency::High)
    .setTimeout(0)
    .exec();
}

void Notifications::Manager::notificationActionTriggered(const Notifications::Request &request, const QString &actionId)
{
    if (actionId == ACTION_NAME_OPEN_FINISHED_TORRENT) {
        const BitTorrent::TorrentHandle *h = BitTorrent::Session::instance()->findTorrent(request.torrent());
        // if there is only a single file in the torrent, we open it
        if (h)
            openUrl(getUrlForTorrentOpen(h));
    }
#ifdef PORT_TO_GUI
    else if (actionId == ACTION_NAME_DEFAULT) {
        if (notification.widget())
            notification.widget()->activateWindow();
        // we could select the finished torrent...
    }
#endif
}

void Notifications::Manager::notificationClosed(const Notifications::Request &request, Notifications::CloseReason reason)
{
    Q_UNUSED(request)
    Q_UNUSED(reason)
}

void Notifications::Manager::connectSlots()
{
    using BitTorrent::Session;
    connect(Session::instance(), SIGNAL(fullDiskError(BitTorrent::TorrentHandle * const,QString)),
            this, SLOT(handleFullDiskError(BitTorrent::TorrentHandle * const,QString)));
    connect(Session::instance(), SIGNAL(addTorrentFailed(QString)),
            this, SLOT(handleAddTorrentFailure(QString)));
    connect(Session::instance(), SIGNAL(torrentFinished(BitTorrent::TorrentHandle * const)),
            this, SLOT(handleTorrentFinished(BitTorrent::TorrentHandle * const)));
    connect(Session::instance(), SIGNAL(downloadFromUrlFailed(QString,QString)),
            this, SLOT(handleDownloadFromUrlFailure(QString,QString)));
}

bool Notifications::Manager::areNotificationsEnabled()
{
    return SettingsStorage::instance()->loadValue(KEY_NOTIFICATIONS_ENABLED, true).toBool();
}

void Notifications::Manager::setNotificationsEnabled(bool value)
{
    SettingsStorage::instance()->storeValue(KEY_NOTIFICATIONS_ENABLED, value);
}

void Notifications::Manager::openUrl(const QUrl &url)
{
#if defined Q_OS_MAC
    QProcess::execute(QLatin1String("open"), QStringList(url.toString()));
#elif defined(Q_OS_UNIX)
    QProcess::execute(QLatin1String("xdg-open"), QStringList(url.toString()));
#elif defined Q_OS_WIN
    QProcess::execute(QLatin1String("start"), QStringList(url.toString()));
#endif
}

Notifications::Notifier* Notifications::Manager::createNotifier()
{
#if (defined(Q_OS_UNIX) && !defined(Q_OS_MAC)) && defined(QT_DBUS_LIB)
    if (SettingsStorage::instance()->loadValue(KEY_NOTIFICATIONS_ENABLED).toBool())
        return new Notifications::DBusNotifier(nullptr);
#endif

    return new DummyNotifier(nullptr);
}

void Notifications::Manager::resetNotifier(Notifier *notifier)
{
    if (m_notifier)
        delete m_notifier.data();

    if (notifier) {
        m_notifier = QPointer<Notifier>(notifier);
    }
    else {
        m_notifier = QPointer<Notifier>(createNotifier());

        if (!m_notifier) {     // Ups...
            qDebug() << "createNotifier() returned null pointer. Setting dummy notifier object";
            m_notifier = QPointer<Notifier>(new DummyNotifier(this));
        }
    }

    m_notifier->setParent(this);
    connect(m_notifier.data(), SIGNAL(notificationActionTriggered(const Request&,const QString&)),
            this, SLOT(notificationActionTriggered(const Request&,const QString&)));
}

const Notifications::Notifier* Notifications::Manager::notifier() const
{
    return m_notifier.data();
}
