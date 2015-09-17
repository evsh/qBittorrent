#ifndef DUMMYNOTIFIER_H
#define DUMMYNOTIFIER_H

#include "notifier.h"

namespace Notifications
{
    class DummyNotifier: public Notifications::Notifier
    {
    public:
        DummyNotifier(QObject* parent);
        // Notifier interface
        virtual void showNotification(const Request &) const override;
    };
}

#endif // DUMMYNOTIFIER_H
