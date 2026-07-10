#pragma once

#include "app/AppSettings.h"
#include "app/CommandDispatcher.h"
#include "core/models/ModelRegistry.h"
#include "core/operators/OperatorRegistry.h"
#include "core/tasks/TaskScheduler.h"

class DiagnosticsLog;

class ApplicationContext
{
public:
    static ApplicationContext& instance();

    OperatorRegistry& operatorRegistry();
    ModelRegistry& modelRegistry();
    TaskScheduler& taskScheduler();
    DiagnosticsLog& diagnosticsLog();
    AppSettings& appSettings();
    CommandDispatcher& commandDispatcher();

    void setAppSettings(const AppSettings& settings);

private:
    ApplicationContext();

    OperatorRegistry m_operatorRegistry;
    ModelRegistry m_modelRegistry;
    TaskScheduler m_taskScheduler;
    CommandDispatcher m_commandDispatcher;
    AppSettings m_appSettings;
};
