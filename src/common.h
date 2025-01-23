// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMON_H
#define COMMON_H

#include <QString>

static const QString SYSTEMD1_SERVICE = "org.freedesktop.systemd1";
static const QString SYSTEMD1_MANAGER_PATH = "/org/freedesktop/systemd1";

static const QString ACTION_ID_CHECK_UPGRADE = "org.deepin.UpdateManager.check-upgrade";
static const QString ACTION_ID_UPGRADE = "org.deepin.UpdateManager.upgrade";

static const QByteArray OSTREE_DEFAULT_REMOTE_NAME = "default";


static const QString STATE_IDEL = "idle";
static const QString STATE_UPGRADING = "upgrading";
static const QString STATE_FAILED = "failed";
static const QString STATE_SUCCESS = "success";


#endif // COMMON_H
