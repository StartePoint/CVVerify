#include "app/ApplicationContext.h"

#include "core/diagnostics/DiagnosticsLog.h"
#include "core/diagnostics/DnnRuntimeInfo.h"
#include "core/operators/BuiltinOperatorBootstrap.h"
#include "core/operators/OperatorSchemaLoader.h"

ApplicationContext& ApplicationContext::instance()
{
    static ApplicationContext context;
    return context;
}

ApplicationContext::ApplicationContext()
{
    registerBuiltinOperators(m_operatorRegistry);

    const QStringList schemaNames = OperatorSchemaLoader::listSchemas();
    DiagnosticsLog::instance().info(
        "config",
        QString("Loaded %1 operator schema file(s)").arg(schemaNames.size()));

    QStringList schemaFailures;
    const int schemaFailureCount = OperatorSchemaLoader::validateIndexedSchemas(
        m_operatorRegistry,
        &schemaFailures);
    if (schemaFailureCount > 0) {
        for (const QString& failure : schemaFailures) {
            DiagnosticsLog::instance().warning("config", failure);
        }
    } else {
        DiagnosticsLog::instance().info(
            "config",
            QString("Validated %1 indexed operator schema(s)")
                .arg(OperatorSchemaLoader::indexedSchemaNames().size()));
    }

    DnnRuntimeInfo::logBackendInfo();
}

OperatorRegistry& ApplicationContext::operatorRegistry()
{
    return m_operatorRegistry;
}

ModelRegistry& ApplicationContext::modelRegistry()
{
    return m_modelRegistry;
}

TaskScheduler& ApplicationContext::taskScheduler()
{
    return m_taskScheduler;
}

DiagnosticsLog& ApplicationContext::diagnosticsLog()
{
    return DiagnosticsLog::instance();
}

AppSettings& ApplicationContext::appSettings()
{
    return m_appSettings;
}

void ApplicationContext::setAppSettings(const AppSettings& settings)
{
    m_appSettings = settings;
}

CommandDispatcher& ApplicationContext::commandDispatcher()
{
    return m_commandDispatcher;
}
