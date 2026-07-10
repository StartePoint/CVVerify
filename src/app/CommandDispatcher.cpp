#include "app/CommandDispatcher.h"

void CommandDispatcher::registerCommand(const QString& commandId, CommandHandler handler)
{
    if (commandId.isEmpty() || !handler) {
        return;
    }

    m_handlers.insert(commandId, std::move(handler));
}

bool CommandDispatcher::dispatch(const QString& commandId) const
{
    const auto iterator = m_handlers.constFind(commandId);
    if (iterator == m_handlers.constEnd()) {
        return false;
    }

    iterator.value()();
    return true;
}

bool CommandDispatcher::hasCommand(const QString& commandId) const
{
    return m_handlers.contains(commandId);
}

QStringList CommandDispatcher::commandIds() const
{
    return m_handlers.keys();
}
