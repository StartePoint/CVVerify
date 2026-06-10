#include <QtTest>

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSignalSpy>
#include <QSpinBox>

#include "core/pipeline/IPipelineStep.h"
#include "ui/panels/ParameterPanel.h"

class FakeSchemaStep : public IPipelineStep
{
public:
    QString id() const override
    {
        return "fake.schema";
    }

    QString displayName() const override
    {
        return "Fake Schema";
    }

    StepSchema schema() const override
    {
        return {
            "fake.schema",
            "Fake Schema",
            {
                {"kernelSize", "Kernel Size", "Basic", StepParameterType::Integer, 3, 1, 31, 1, {}, {}, {}},
                {"sigma", "Sigma", "Basic", StepParameterType::Double, 1.5, 0.0, 10.0, 0.1, {}, {}, {}},
                {"enabled", "Enabled", "Basic", StepParameterType::Boolean, true, {}, {}, {}, {}, {}, {}},
                {"mode", "Mode", "Basic", StepParameterType::Choice, "binary", {}, {}, {}, {}, {}, {
                    {"binary", "Binary"},
                    {"otsu", "Otsu"},
                }},
                {"boundaryColor", "Boundary Color", "Advanced", StepParameterType::String, "#ff2d55", {}, {}, {}, "mode!=otsu", {}, {}},
            }
        };
    }

    QVariantMap parameterValues() const override
    {
        return m_values;
    }

    void setParameterValues(const QVariantMap& values) override
    {
        m_values = values;
    }

    StepResult execute(FramePacket&, const RunContext&) override
    {
        return {};
    }

private:
    QVariantMap m_values{
        {"kernelSize", 3},
        {"sigma", 1.5},
        {"enabled", true},
        {"mode", "binary"},
        {"boundaryColor", "#ff2d55"},
    };
};

class ParameterPanelTest : public QObject
{
    Q_OBJECT

private slots:
    void createsEditorsForMultipleParameterTypes();
    void emitsUpdatedValuesWhenIntegerChanges();
    void emitsUpdatedValuesWhenStringChanges();
    void hidesStringEditorWhenVisibleWhenIsFalse();
};

void ParameterPanelTest::createsEditorsForMultipleParameterTypes()
{
    auto step = std::make_shared<FakeSchemaStep>();

    ParameterPanel panel;
    panel.setStep(step);

    QVERIFY(panel.findChild<QSpinBox*>() != nullptr);
    QVERIFY(panel.findChild<QDoubleSpinBox*>() != nullptr);
    QVERIFY(panel.findChild<QCheckBox*>() != nullptr);
    QVERIFY(panel.findChild<QComboBox*>() != nullptr);
    QVERIFY(panel.findChild<QLineEdit*>() != nullptr);
}

void ParameterPanelTest::emitsUpdatedValuesWhenIntegerChanges()
{
    auto step = std::make_shared<FakeSchemaStep>();

    ParameterPanel panel;
    panel.setStep(step);

    QSignalSpy spy(&panel, &ParameterPanel::parameterValuesChanged);
    auto* spinBox = panel.findChild<QSpinBox*>();
    QVERIFY(spinBox != nullptr);

    spinBox->setValue(7);

    QCOMPARE(spy.count(), 1);
    const QVariantMap values = spy.takeFirst().at(0).toMap();
    QCOMPARE(values.value("kernelSize").toInt(), 7);
}

void ParameterPanelTest::emitsUpdatedValuesWhenStringChanges()
{
    auto step = std::make_shared<FakeSchemaStep>();

    ParameterPanel panel;
    panel.setStep(step);

    QSignalSpy spy(&panel, &ParameterPanel::parameterValuesChanged);
    const auto lineEdits = panel.findChildren<QLineEdit*>();
    QVERIFY(!lineEdits.isEmpty());

    lineEdits.constFirst()->setText("#00ff99");

    QVERIFY(spy.count() >= 1);
    const QVariantMap values = spy.takeLast().at(0).toMap();
    QCOMPARE(values.value("boundaryColor").toString(), QString("#00ff99"));
}

void ParameterPanelTest::hidesStringEditorWhenVisibleWhenIsFalse()
{
    auto step = std::make_shared<FakeSchemaStep>();
    step->setParameterValues({
        {"kernelSize", 3},
        {"sigma", 1.5},
        {"enabled", true},
        {"mode", "otsu"},
        {"boundaryColor", "#ff2d55"},
    });

    ParameterPanel panel;
    panel.setStep(step);

    const auto lineEdits = panel.findChildren<QLineEdit*>();
    QVERIFY(!lineEdits.isEmpty());
    QVERIFY(!lineEdits.constFirst()->isVisible());
}

int runParameterPanelTests(int argc, char* argv[])
{
    ParameterPanelTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_parameter_panel.moc"
