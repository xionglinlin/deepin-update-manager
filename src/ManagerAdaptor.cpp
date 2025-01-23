// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ManagerAdaptor.h"

#include "common.h"
#include "checkupgradetask.h"
#include "upgradetask.h"

QDBusArgument &operator<<(QDBusArgument &argument, const Progress &progress)
{
    argument.beginStructure();
    argument << progress.stage << progress.percent;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Progress &progress)
{
    argument.beginStructure();
    argument >> progress.stage;
    double d;
    argument >> d;
    progress.percent = d;
    argument.endStructure();

    return argument;
}

ManagerAdaptor::ManagerAdaptor(int listRemoteRefsFd,
                               int upgradeStdoutFd,
                               const QDBusConnection &bus,
                               QObject *parent)
    : QObject(parent)
    , m_bus(bus)
    , m_listRemoteRefsFd(listRemoteRefsFd)
    , m_upgradeStdoutFd(upgradeStdoutFd)
    , m_remoteBranch(QString())
    , m_upgradable(false)
    , m_state(STATE_IDEL)
{
    qRegisterMetaType<Progress>("Progress");
    qDBusRegisterMetaType<Progress>();

    connect(this, &ManagerAdaptor::stateChanged, this, [this](const QString &state) {
        sendPropertyChanged("state", state);
    });
    connect(this, &ManagerAdaptor::upgradableChanged, this, [this](bool upgradable) {
        sendPropertyChanged("upgradable", upgradable);
    });
}

ManagerAdaptor::~ManagerAdaptor() = default;

void ManagerAdaptor::checkUpgrade(const QDBusMessage &message)
{
    checkUpgradeTask *task = new checkUpgradeTask(m_bus, message, this);
    task->setTaskDate("dum-list-remote-refs.service", m_listRemoteRefsFd);

    connect(task, &checkUpgradeTask::checkUpgradeResult, this, [this](bool upgradable, const QString &remoteBranch) {
        m_remoteBranch = remoteBranch;
        if (m_upgradable != upgradable) {
            m_upgradable = upgradable;
            emit upgradableChanged(m_upgradable);
        }
   });

   task->run();
}

static QString systemdEscape(const QString &str)
{
    auto tmp = str;
    tmp.replace('-', "\\x2d");
    tmp.replace('/', "-");

    return tmp;
}

void ManagerAdaptor::upgrade(const QDBusMessage &message)
{
    if (!m_upgradable || m_remoteBranch.isEmpty()) {
        m_bus.send(message.createErrorReply(QDBusError::AccessDenied, "No upgrade available"));
        return;
    }

    QString version = OSTREE_DEFAULT_REMOTE_NAME + ':' + m_remoteBranch;
    QString unit = QString("dum-upgrade@%1.service").arg(systemdEscape(version));

    UpgradeTask *task = new UpgradeTask(m_bus, message, this);
    task->setTaskDate("dum-upgrade.service", m_upgradeStdoutFd);

    connect(task, &UpgradeTask::progressChanged, this, [this](const QString &stage, float percent) {
        emit progress({ stage, percent });
    });

    connect(task, &UpgradeTask::upgradableChanged, this, [this](bool upgradable) {
        m_upgradable = upgradable;
        emit upgradableChanged(m_upgradable);
    });

    connect(task, &UpgradeTask::stateChanged, this, [this](const QString &state) {
        m_state = state;
        emit stateChanged(m_state);
    });

    task->run();
}

bool ManagerAdaptor::upgradable() const
{
    return m_upgradable;
}

QString ManagerAdaptor::state() const
{
    return m_state;
}

void ManagerAdaptor::sendPropertyChanged(const QString &property, const QVariant &value)
{
    auto msg = QDBusMessage::createSignal(ADAPTOR_PATH,
                                          "org.freedesktop.DBus.Properties",
                                          "PropertiesChanged");
    msg << "org.deepin.UpdateManager1";
    msg << QVariantMap{ { property, value } };
    msg << QStringList{};

    auto res = m_bus.send(msg);
    if (!res) {
        qWarning() << "sendPropertyChanged failed";
    }
}
