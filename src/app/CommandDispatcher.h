#pragma once

#include <functional>

#include <QHash>
#include <QString>

class CommandDispatcher
{
public:
    using CommandHandler = std::function<void()>;

    void registerCommand(const QString& commandId, CommandHandler handler);
    bool dispatch(const QString& commandId) const;
    bool hasCommand(const QString& commandId) const;
    QStringList commandIds() const;

private:
    QHash<QString, CommandHandler> m_handlers;
};
