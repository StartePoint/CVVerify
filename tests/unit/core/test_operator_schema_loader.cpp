#include <QtTest>

#include "app/ApplicationContext.h"
#include "core/operators/OperatorSchemaLoader.h"

class OperatorSchemaLoaderTest : public QObject
{
    Q_OBJECT

private slots:
    void listsBuiltInSchemas();
    void loadsGaussianBlurSchema();
    void validatesGaussianBlurAgainstRuntimeStep();
    void validatesIndexedSchemasAgainstRegistry();
};

void OperatorSchemaLoaderTest::listsBuiltInSchemas()
{
    const QStringList schemas = OperatorSchemaLoader::listSchemas();
    QCOMPARE(schemas.size(), 48);
    QVERIFY(schemas.contains("gaussian_blur"));
    QVERIFY(schemas.contains("template_match"));
    QVERIFY(schemas.contains("histogram_bar"));
    QVERIFY(schemas.contains("threshold"));
    QVERIFY(!schemas.contains("index"));

    const QStringList indexed = OperatorSchemaLoader::indexedSchemaNames();
    QCOMPARE(indexed.size(), 48);
}

void OperatorSchemaLoaderTest::loadsGaussianBlurSchema()
{
    OperatorSchemaDocument document;
    QString errorMessage;
    QVERIFY(OperatorSchemaLoader::loadSchema("gaussian_blur", &document, &errorMessage));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QCOMPARE(document.id, QString("builtin.gaussian_blur"));
    QCOMPARE(document.schema.parameters.size(), 4);
    QCOMPARE(document.schema.parameters.first().key, QString("kernelWidth"));
}

void OperatorSchemaLoaderTest::validatesGaussianBlurAgainstRuntimeStep()
{
    OperatorSchemaDocument document;
    QString errorMessage;
    QVERIFY(OperatorSchemaLoader::loadSchema("gaussian_blur", &document, &errorMessage));

    const auto step = ApplicationContext::instance().operatorRegistry().create("builtin.gaussian_blur");
    QVERIFY(step != nullptr);
    QVERIFY(OperatorSchemaLoader::validateAgainstStep(document, *step, &errorMessage));
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
}

void OperatorSchemaLoaderTest::validatesIndexedSchemasAgainstRegistry()
{
    QStringList failures;
    const int failureCount = OperatorSchemaLoader::validateIndexedSchemas(
        ApplicationContext::instance().operatorRegistry(),
        &failures);
    if (failureCount > 0) {
        qWarning() << "Schema validation failures:" << failures;
    }
    QCOMPARE(failureCount, 0);
}

int runOperatorSchemaLoaderTests(int argc, char* argv[])
{
    OperatorSchemaLoaderTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_operator_schema_loader.moc"
