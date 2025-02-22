// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IDLE_H
#define IDLE_H

#include <QTimer>
#include <QObject>

#define DUM_AUTO_IDLE_TIMEOUT 60000

class Idle: public QObject
{
    Q_OBJECT
public:
    explicit Idle(QObject *parent = nullptr);
    void Inhibit(const QString& task);
    void UnInhibit(const QString& task);

private:
    void handleInhibit() const;

public Q_SLOTS:
    void onTimeout() const;

private:
    QTimer *m_timer{};
    QStringList m_reasons;
};



#endif //IDLE_H
