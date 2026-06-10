#include <QtTest>

#include <QListWidget>
#include <QPushButton>
#include <QSignalSpy>

#include "core/operators/OperatorDescriptor.h"
#include "ui/panels/PipelinePanel.h"

namespace {

constexpr int kOperatorIdRole = Qt::UserRole;
constexpr int kIsCategoryHeaderRole = Qt::UserRole + 2;

QList<OperatorDescriptor> sampleGroupedOperators()
{
    return {
        {"builtin.gaussian_blur", "Gaussian Blur", QStringLiteral("\u6ee4\u6ce2"),
            OperatorCapabilityType::BasicSingleIO,
            OperatorInteractionType::None,
            OperatorResultType::ImageOnly, {}},
        {"builtin.resize", "Resize", QStringLiteral("\u56fe\u50cf\u589e\u5f3a\u4e0e\u53d8\u6362"),
            OperatorCapabilityType::BasicSingleIO,
            OperatorInteractionType::None,
            OperatorResultType::ImageOnly, {}},
    };
}

}

class PipelinePanelTest : public QObject
{
    Q_OBJECT

private slots:
    void emitsAddOperatorRequestedForSelectedAvailableOperator();
    void categoryHeadersCannotBeAdded();
    void localizesAvailableOperatorNamesAndCategoryHeaders();
    void emitsPipelineStepSelectedForSelectedPipelineStep();
    void emitsRemoveAndClearSignals();
    void emitsRenameSignalWhenPipelineStepTextChanges();
    void movePipelineStepReordersItemsAndEmitsSignal();
    void emitsSaveAndLoadSignals();
};

void PipelinePanelTest::emitsAddOperatorRequestedForSelectedAvailableOperator()
{
    PipelinePanel panel;
    panel.setOperators(sampleGroupedOperators());

    auto* availableList = panel.findChild<QListWidget*>("availableOperatorsList");
    auto* addButton = panel.findChild<QPushButton*>("addOperatorButton");
    QVERIFY(availableList != nullptr);
    QVERIFY(addButton != nullptr);
    QCOMPARE(availableList->count(), 4);

    QSignalSpy spy(&panel, SIGNAL(addOperatorRequested(QString)));

    availableList->setCurrentRow(3);
    QVERIFY(addButton->isEnabled());
    addButton->click();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QString("builtin.resize"));
}

void PipelinePanelTest::categoryHeadersCannotBeAdded()
{
    PipelinePanel panel;
    panel.setOperators(sampleGroupedOperators());

    auto* availableList = panel.findChild<QListWidget*>("availableOperatorsList");
    auto* addButton = panel.findChild<QPushButton*>("addOperatorButton");
    QVERIFY(availableList != nullptr);
    QVERIFY(addButton != nullptr);

    QSignalSpy spy(&panel, SIGNAL(addOperatorRequested(QString)));

    availableList->setCurrentRow(0);
    QVERIFY(availableList->item(0) != nullptr);
    QVERIFY(availableList->item(0)->data(kIsCategoryHeaderRole).toBool());
    QVERIFY(availableList->currentItem() == nullptr
        || !availableList->currentItem()->data(kIsCategoryHeaderRole).toBool());
    QVERIFY(!addButton->isEnabled());
    addButton->click();

    QCOMPARE(spy.count(), 0);
}

void PipelinePanelTest::localizesAvailableOperatorNamesAndCategoryHeaders()
{
    PipelinePanel panel;
    panel.setOperators(sampleGroupedOperators());

    auto* availableList = panel.findChild<QListWidget*>("availableOperatorsList");
    QVERIFY(availableList != nullptr);
    QCOMPARE(availableList->count(), 4);

    QCOMPARE(availableList->item(0)->text(), QString("Filtering"));
    QCOMPARE(availableList->item(1)->text(), QString("Gaussian Blur"));
    QCOMPARE(availableList->item(1)->data(kOperatorIdRole).toString(), QString("builtin.gaussian_blur"));
    QCOMPARE(availableList->item(2)->text(), QString("Enhancement & Transform"));
    QCOMPARE(availableList->item(3)->text(), QString("Resize"));

    panel.setLanguage("zh-CN");

    QCOMPARE(availableList->item(0)->text(), QStringLiteral("\u6ee4\u6ce2"));
    QCOMPARE(availableList->item(1)->text(), QStringLiteral("\u9ad8\u65af\u6a21\u7cca"));
    QCOMPARE(availableList->item(2)->text(), QStringLiteral("\u56fe\u50cf\u589e\u5f3a\u4e0e\u53d8\u6362"));
    QCOMPARE(availableList->item(3)->text(), QStringLiteral("\u7f29\u653e"));
}

void PipelinePanelTest::emitsPipelineStepSelectedForSelectedPipelineStep()
{
    PipelinePanel panel;
    panel.setPipelineStepNames({"Grayscale", "Resize"});

    auto* pipelineList = panel.findChild<QListWidget*>("pipelineStepsList");
    QVERIFY(pipelineList != nullptr);

    QSignalSpy spy(&panel, SIGNAL(pipelineStepSelected(int)));

    pipelineList->setCurrentRow(1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 1);
}

void PipelinePanelTest::emitsRemoveAndClearSignals()
{
    PipelinePanel panel;
    panel.setPipelineStepNames({"Grayscale", "Resize"});

    auto* pipelineList = panel.findChild<QListWidget*>("pipelineStepsList");
    auto* removeButton = panel.findChild<QPushButton*>("removeOperatorButton");
    auto* clearButton = panel.findChild<QPushButton*>("clearPipelineButton");
    QVERIFY(pipelineList != nullptr);
    QVERIFY(removeButton != nullptr);
    QVERIFY(clearButton != nullptr);

    QSignalSpy removeSpy(&panel, SIGNAL(removePipelineStepRequested(int)));
    QSignalSpy clearSpy(&panel, SIGNAL(clearPipelineRequested()));

    pipelineList->setCurrentRow(0);
    removeButton->click();
    clearButton->click();

    QCOMPARE(removeSpy.count(), 1);
    QCOMPARE(removeSpy.takeFirst().at(0).toInt(), 0);
    QCOMPARE(clearSpy.count(), 1);
}

void PipelinePanelTest::emitsRenameSignalWhenPipelineStepTextChanges()
{
    PipelinePanel panel;
    panel.setPipelineStepNames({"Grayscale", "Resize"});

    auto* pipelineList = panel.findChild<QListWidget*>("pipelineStepsList");
    QVERIFY(pipelineList != nullptr);
    QCOMPARE(pipelineList->count(), 2);

    QSignalSpy renameSpy(&panel, SIGNAL(pipelineStepRenamed(int,QString)));

    auto* item = pipelineList->item(0);
    QVERIFY(item != nullptr);
    item->setText("Preprocess Gray");

    QCOMPARE(renameSpy.count(), 1);
    const QList<QVariant> payload = renameSpy.takeFirst();
    QCOMPARE(payload.at(0).toInt(), 0);
    QCOMPARE(payload.at(1).toString(), QString("Preprocess Gray"));
}

void PipelinePanelTest::movePipelineStepReordersItemsAndEmitsSignal()
{
    PipelinePanel panel;
    panel.setPipelineStepNames({"Grayscale", "Resize", "Canny"});

    auto* pipelineList = panel.findChild<QListWidget*>("pipelineStepsList");
    QVERIFY(pipelineList != nullptr);

    QSignalSpy moveSpy(&panel, SIGNAL(pipelineStepMoved(int,int)));

    panel.movePipelineStep(0, 2);

    QCOMPARE(moveSpy.count(), 1);
    const QList<QVariant> payload = moveSpy.takeFirst();
    QCOMPARE(payload.at(0).toInt(), 0);
    QCOMPARE(payload.at(1).toInt(), 2);

    QCOMPARE(pipelineList->item(0)->text(), QString("Resize"));
    QCOMPARE(pipelineList->item(1)->text(), QString("Canny"));
    QCOMPARE(pipelineList->item(2)->text(), QString("Grayscale"));
}

void PipelinePanelTest::emitsSaveAndLoadSignals()
{
    PipelinePanel panel;

    auto* saveButton = panel.findChild<QPushButton*>("savePipelineButton");
    auto* loadButton = panel.findChild<QPushButton*>("loadPipelineButton");
    QVERIFY(saveButton != nullptr);
    QVERIFY(loadButton != nullptr);

    QSignalSpy saveSpy(&panel, SIGNAL(savePipelineRequested()));
    QSignalSpy loadSpy(&panel, SIGNAL(loadPipelineRequested()));

    saveButton->click();
    loadButton->click();

    QCOMPARE(saveSpy.count(), 1);
    QCOMPARE(loadSpy.count(), 1);
}

int runPipelinePanelTests(int argc, char* argv[])
{
    PipelinePanelTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_pipeline_panel.moc"
