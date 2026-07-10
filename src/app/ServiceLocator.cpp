#include "app/ServiceLocator.h"

#include "app/ApplicationContext.h"

ServiceLocator& ServiceLocator::instance()
{
    static ServiceLocator locator;
    return locator;
}

ApplicationContext& ServiceLocator::applicationContext()
{
    return ApplicationContext::instance();
}

OperatorRegistry& ServiceLocator::operatorRegistry()
{
    return ApplicationContext::instance().operatorRegistry();
}

ModelRegistry& ServiceLocator::modelRegistry()
{
    return ApplicationContext::instance().modelRegistry();
}

TaskScheduler& ServiceLocator::taskScheduler()
{
    return ApplicationContext::instance().taskScheduler();
}

DiagnosticsLog& ServiceLocator::diagnosticsLog()
{
    return ApplicationContext::instance().diagnosticsLog();
}

AppSettings& ServiceLocator::appSettings()
{
    return ApplicationContext::instance().appSettings();
}
