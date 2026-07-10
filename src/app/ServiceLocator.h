#pragma once

class ApplicationContext;
class DiagnosticsLog;
class ModelRegistry;
class OperatorRegistry;
class TaskScheduler;
struct AppSettings;

class ServiceLocator
{
public:
    static ServiceLocator& instance();

    ApplicationContext& applicationContext();
    OperatorRegistry& operatorRegistry();
    ModelRegistry& modelRegistry();
    TaskScheduler& taskScheduler();
    DiagnosticsLog& diagnosticsLog();
    AppSettings& appSettings();

private:
    ServiceLocator() = default;
};
