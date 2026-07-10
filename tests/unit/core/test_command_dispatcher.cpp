#include <QtTest>

#include "app/CommandDispatcher.h"

class CommandDispatcherTest : public QObject
{
    Q_OBJECT

private slots:
    void registersAndDispatchesCommands();
    void ignoresEmptyRegistration();
    void reportsUnknownCommands();
};

void CommandDispatcherTest::registersAndDispatchesCommands()
{
    CommandDispatcher dispatcher;
    int counter = 0;
    dispatcher.registerCommand("test.increment", [&counter] { ++counter; });

    QVERIFY(dispatcher.hasCommand("test.increment"));
    QVERIFY(dispatcher.commandIds().contains("test.increment"));
    QVERIFY(dispatcher.dispatch("test.increment"));
    QCOMPARE(counter, 1);
}

void CommandDispatcherTest::ignoresEmptyRegistration()
{
    CommandDispatcher dispatcher;
    dispatcher.registerCommand(QString(), [] {});
    dispatcher.registerCommand("valid", CommandDispatcher::CommandHandler());

    QVERIFY(!dispatcher.hasCommand(QString()));
    QVERIFY(!dispatcher.hasCommand("valid"));
    QCOMPARE(dispatcher.commandIds().size(), 0);
}

void CommandDispatcherTest::reportsUnknownCommands()
{
    CommandDispatcher dispatcher;
    QVERIFY(!dispatcher.dispatch("missing.command"));
    QVERIFY(!dispatcher.hasCommand("missing.command"));
}

int runCommandDispatcherTests(int argc, char* argv[])
{
    CommandDispatcherTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_command_dispatcher.moc"
