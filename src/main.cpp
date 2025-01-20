// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dumdbusadaptor.h"
#include "updatemanager.h"

#include <systemd/sd-daemon.h>

#include <QCoreApplication>
#include <QDBusConnection>

#include <string>
#include <unordered_map>

const std::string DUM_LIST_REMOTE_REFS_STDOUT = "dum-list-remote-refs-stdout";
const std::string DUM_UPGRADE_STDOUT = "dum-upgrade-stdout";

static std::unordered_map<std::string, int> getFds()
{
    std::unordered_map<std::string, int> fds;
    char **names;
    int count = sd_listen_fds_with_names(true, &names);
    for (int i = 0; i < count; i++) {
        fds.emplace(names[i], SD_LISTEN_FDS_START + i);
        free(names[i]);
    }

    return fds;
}

int main(int argc, char *argv[])
{
    auto fds = getFds();
    if (!fds.contains(DUM_LIST_REMOTE_REFS_STDOUT)) {
        qWarning() << DUM_LIST_REMOTE_REFS_STDOUT << " not found";
        return 1;
    }
    if (!fds.contains(DUM_UPGRADE_STDOUT)) {
        qWarning() << DUM_UPGRADE_STDOUT << " not found";
        return 1;
    }

    int dumListRemoteRefsStdoutFd = fds[DUM_LIST_REMOTE_REFS_STDOUT];
    int dumUpgradeStdoutFd = fds[DUM_UPGRADE_STDOUT];

    QCoreApplication a(argc, argv);

    QDBusConnection connection = QDBusConnection::systemBus();

    auto dum = new UpdateManager(dumListRemoteRefsStdoutFd,dumUpgradeStdoutFd);
    DumDbusAdaptor adaptor(dum);

    bool dumRegister = connection.registerService(DBUS_SERVICE_NAME);
    if (!dumRegister) {
        qWarning() << "deepin-update-manager dbus service register failed:" << connection.lastError();
        return EXIT_FAILURE;
    }
    bool dumObjRegister = connection.registerObject(DBUS_PATH,DBUS_INTERFACE_NAME, dum);
    if (!dumObjRegister) {
        qWarning() << "deepin-update-manager dbus object register failed:" << connection.lastError();
        return EXIT_FAILURE;
    }
    return a.exec();
}
