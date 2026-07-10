#include <QtTest>

#include <QFileInfo>

#include "core/diagnostics/DnnRuntimeInfo.h"

class DnnRuntimeInfoTest : public QObject
{
    Q_OBJECT

private slots:
    void reportsCpuBackend();
};

void DnnRuntimeInfoTest::reportsCpuBackend()
{
    const DnnRuntimeInfo::BackendInfo info = DnnRuntimeInfo::queryBackendInfo();
    QVERIFY(info.availableBackends.contains(QStringLiteral("cpu")));
    QVERIFY(!info.opencvVersion.isEmpty());
}

int runDnnRuntimeInfoTests(int argc, char* argv[])
{
    DnnRuntimeInfoTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_dnn_runtime_info.moc"
