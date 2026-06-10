#include <QtTest>

#include <memory>
#include <vector>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPointF>
#include <QPushButton>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QSpinBox>
#include <QTimer>
#include <QToolButton>
#include <QVariantMap>
#include <QVector>
#include <QWidget>

#include <opencv2/core/mat.hpp>

#include "ui/mainwindow/MainWindow.h"
#include "ui/widgets/CanvasView.h"

namespace {

QAction* findActionInMenu(QMenu* menu, const QString& objectName)
{
    if (!menu) {
        return nullptr;
    }

    for (QAction* action : menu->actions()) {
        if (action && action->objectName() == objectName) {
            return action;
        }
    }
    return nullptr;
}

bool hasVisibleOperatorDialog()
{
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        auto* dialog = qobject_cast<QDialog*>(widget);
        if (dialog && dialog->objectName() == "operatorParameterDialog" && dialog->isVisible()) {
            return true;
        }
    }
    return false;
}

}

class MainWindowMenuTest : public QObject
{
    Q_OBJECT

private slots:
    void showsSettingsAndLanguageMenus();
    void showsDualPreviewShell();
    void defaultsToLargeWindowWithHiddenOperatorWorkbench();
    void opensWorkbenchDialogsFromMenus();
    void localizesPreviewAndOperatorMenusAndParameters();
    void blocksAddingOperatorWithoutSelectedMedia();
    void operatorMenuAddsOperatorIntoMainWindow();
    void operatorMenuDoesNotOpenParameterDialog();
    void addingOperatorShowsStepAndParameterDetails();

private:
    void seedLoadedMedia(MainWindow& window);
};

void MainWindowMenuTest::seedLoadedMedia(MainWindow& window)
{
    window.m_currentMediaInfo.sourceId = "test.png";
    window.m_currentFrame.sourceId = "test.png";
    window.m_currentFrame.originalMat = cv::Mat::zeros(32, 32, CV_8UC3);
    window.m_currentFrame.workingMat = window.m_currentFrame.originalMat.clone();
}

void MainWindowMenuTest::showsSettingsAndLanguageMenus()
{
    MainWindow window;

    auto* menuBar = window.menuBar();
    QVERIFY(menuBar != nullptr);

    QAction* settingsAction = nullptr;
    QAction* languageAction = nullptr;

    const QList<QAction*> actions = menuBar->actions();
    for (QAction* action : actions) {
        if (!action) {
            continue;
        }

        if (action->objectName() == "settingsMenuAction") {
            settingsAction = action;
        } else if (action->objectName() == "languageMenuAction") {
            languageAction = action;
        }
    }

    QVERIFY(settingsAction != nullptr);
    QVERIFY(languageAction != nullptr);
}

void MainWindowMenuTest::showsDualPreviewShell()
{
    MainWindow window;

    const QList<CanvasView*> canvases = window.findChildren<CanvasView*>();
    QCOMPARE(canvases.count(), 2);

    QLabel* sourceLabel = nullptr;
    QLabel* resultLabel = nullptr;
    const QList<QLabel*> labels = window.findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (!label) {
            continue;
        }

        if (label->text() == QStringLiteral("\u539f\u56fe / \u539f\u89c6\u9891")
            || label->text() == QString("Source Image / Video")) {
            sourceLabel = label;
        } else if (label->text() == QStringLiteral("\u5904\u7406\u540e\u56fe / \u89c6\u9891")
            || label->text() == QString("Processed Image / Video")) {
            resultLabel = label;
        }
    }

    QVERIFY(sourceLabel != nullptr);
    QVERIFY(resultLabel != nullptr);
}

void MainWindowMenuTest::defaultsToLargeWindowWithHiddenOperatorWorkbench()
{
    MainWindow window;

    QCOMPARE(window.size(), QSize(1920, 1080));

    auto* operatorWorkbenchPanel = window.findChild<QFrame*>("operatorWorkbenchPanel");
    auto* pipelineWorkbenchAction = window.findChild<QAction*>("showPipelineWorkbenchAction");
    QVERIFY(operatorWorkbenchPanel != nullptr);
    QVERIFY(pipelineWorkbenchAction != nullptr);
    QVERIFY(operatorWorkbenchPanel->isHidden());
    QVERIFY(!pipelineWorkbenchAction->isChecked());
}

void MainWindowMenuTest::opensWorkbenchDialogsFromMenus()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto* mediaWorkbenchAction = window.findChild<QAction*>("showMediaWorkbenchAction");
    auto* pipelineWorkbenchAction = window.findChild<QAction*>("showPipelineWorkbenchAction");
    auto* exportResultsAction = window.findChild<QAction*>("showExportResultsAction");
    auto* operatorWorkbenchPanel = window.findChild<QFrame*>("operatorWorkbenchPanel");
    QVERIFY(mediaWorkbenchAction != nullptr);
    QVERIFY(pipelineWorkbenchAction != nullptr);
    QVERIFY(exportResultsAction != nullptr);
    QVERIFY(operatorWorkbenchPanel != nullptr);

    auto countTopLevelDialogs = []() {
        int count = 0;
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget* widget : widgets) {
            if (qobject_cast<QDialog*>(widget) != nullptr && widget->isVisible()) {
                ++count;
            }
        }
        return count;
    };

    const int initialDialogs = countTopLevelDialogs();
    mediaWorkbenchAction->trigger();
    QTRY_COMPARE(countTopLevelDialogs(), initialDialogs + 1);
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        if (auto* dialog = qobject_cast<QDialog*>(widget)) {
            if (dialog->isVisible()) {
                dialog->close();
            }
        }
    }

    pipelineWorkbenchAction->trigger();
    QTRY_VERIFY(operatorWorkbenchPanel->isVisible());
    QVERIFY(pipelineWorkbenchAction->isChecked());
    QCOMPARE(countTopLevelDialogs(), initialDialogs);

    pipelineWorkbenchAction->trigger();
    QTRY_VERIFY(!operatorWorkbenchPanel->isVisible());
    QVERIFY(!pipelineWorkbenchAction->isChecked());
    QCOMPARE(countTopLevelDialogs(), initialDialogs);

    exportResultsAction->trigger();
    QTRY_COMPARE(countTopLevelDialogs(), initialDialogs + 1);
}

void MainWindowMenuTest::localizesPreviewAndOperatorMenusAndParameters()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto* localizedChineseAction = window.findChild<QAction*>("chineseLanguageAction");
    QVERIFY(localizedChineseAction != nullptr);
    localizedChineseAction->trigger();

    auto* previewMenuAction = window.findChild<QAction*>("previewMenuAction");
    auto* operatorMenuAction = window.findChild<QAction*>("operatorMenuAction");
    auto* showWorkbenchAction = window.findChild<QAction*>("showPipelineWorkbenchAction");
    auto* fullPipelinePreviewAction = window.findChild<QAction*>("fullPipelinePreviewAction");
    auto* singleNodePreviewAction = window.findChild<QAction*>("singleNodePreviewAction");
    QVERIFY(previewMenuAction != nullptr);
    QVERIFY(operatorMenuAction != nullptr);
    QVERIFY(showWorkbenchAction != nullptr);
    QVERIFY(fullPipelinePreviewAction != nullptr);
    QVERIFY(singleNodePreviewAction != nullptr);

    QCOMPARE(previewMenuAction->text(), QStringLiteral("\u9884\u89c8"));
    QCOMPARE(operatorMenuAction->text(), QStringLiteral("\u7b97\u5b50"));
    QCOMPARE(showWorkbenchAction->text(), QStringLiteral("\u663e\u793a\u7b97\u5b50\u5de5\u4f5c\u5e26"));
    QCOMPARE(fullPipelinePreviewAction->text(), QStringLiteral("\u5168\u6d41\u7a0b\u9884\u89c8"));
    QCOMPARE(singleNodePreviewAction->text(), QStringLiteral("\u5355\u8282\u70b9\u9884\u89c8"));

    auto* filteringButton = window.findChild<QToolButton*>("quickAddCategoryButton_filtering");
    auto* filteringMenu = window.findChild<QMenu*>("quickAddCategoryMenu_filtering");
    QVERIFY(filteringButton != nullptr);
    QVERIFY(filteringMenu != nullptr);
    QCOMPARE(filteringButton->text(), QStringLiteral("\u6ee4\u6ce2"));
    QCOMPARE(filteringMenu->title(), QStringLiteral("\u6ee4\u6ce2"));

    QAction* gaussianAction = findActionInMenu(filteringMenu, "operatorAction_builtin_gaussian_blur");
    QVERIFY(gaussianAction != nullptr);
    QCOMPARE(gaussianAction->text(), QStringLiteral("\u9ad8\u65af\u6a21\u7cca"));

    seedLoadedMedia(window);
    gaussianAction->trigger();
    QVERIFY(!hasVisibleOperatorDialog());

    auto* pipelineStripList = window.findChild<QListWidget*>("pipelineStripList");
    QVERIFY(pipelineStripList != nullptr);
    QTRY_COMPARE(pipelineStripList->count(), 1);

    bool foundKernelLabel = false;
    const auto labels = window.findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label && label->text() == QStringLiteral("\u6838\u5bbd")) {
            foundKernelLabel = true;
            break;
        }
    }
    QVERIFY(foundKernelLabel);
}

void MainWindowMenuTest::blocksAddingOperatorWithoutSelectedMedia()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto* transformMenu = window.findChild<QMenu*>("operatorCategoryMenu_enhancement_transform");
    QVERIFY(transformMenu != nullptr);
    auto* resizeAction = findActionInMenu(transformMenu, "operatorAction_builtin_resize");
    QVERIFY(resizeAction != nullptr);

    auto* pipelineStripList = window.findChild<QListWidget*>("pipelineStripList");
    QVERIFY(pipelineStripList != nullptr);
    QCOMPARE(pipelineStripList->count(), 0);

    QString shownMessage;
    QTimer::singleShot(0, [&shownMessage] {
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            auto* candidate = qobject_cast<QMessageBox*>(widget);
            if (candidate && candidate->isVisible()) {
                shownMessage = candidate->text();
                candidate->accept();
                break;
            }
        }
    });

    resizeAction->trigger();

    QCOMPARE(pipelineStripList->count(), 0);
    QCOMPARE(shownMessage, QStringLiteral("\u8bf7\u5148\u9009\u62e9\u56fe\u7247\u6216\u89c6\u9891"));
}

void MainWindowMenuTest::operatorMenuAddsOperatorIntoMainWindow()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    seedLoadedMedia(window);

    auto* transformMenu = window.findChild<QMenu*>("operatorCategoryMenu_enhancement_transform");
    QVERIFY(transformMenu != nullptr);
    auto* resizeAction = findActionInMenu(transformMenu, "operatorAction_builtin_resize");
    QVERIFY(resizeAction != nullptr);
    resizeAction->trigger();
    QVERIFY(!hasVisibleOperatorDialog());

    auto* pipelineStripList = window.findChild<QListWidget*>("pipelineStripList");
    QVERIFY(pipelineStripList != nullptr);
    QTRY_COMPARE(pipelineStripList->count(), 1);
    QCOMPARE(pipelineStripList->currentRow(), 0);

    auto* spinBox = window.findChild<QSpinBox*>();
    QVERIFY(spinBox != nullptr);
}

void MainWindowMenuTest::operatorMenuDoesNotOpenParameterDialog()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    seedLoadedMedia(window);

    auto* transformMenu = window.findChild<QMenu*>("operatorCategoryMenu_enhancement_transform");
    QVERIFY(transformMenu != nullptr);
    auto* resizeAction = findActionInMenu(transformMenu, "operatorAction_builtin_resize");
    QVERIFY(resizeAction != nullptr);
    resizeAction->trigger();

    QVERIFY(!hasVisibleOperatorDialog());
    auto* pipelineStripList = window.findChild<QListWidget*>("pipelineStripList");
    QVERIFY(pipelineStripList != nullptr);
    QTRY_COMPARE(pipelineStripList->count(), 1);

    auto* singlePreviewAction = window.findChild<QAction*>("singleNodePreviewAction");
    QVERIFY(singlePreviewAction != nullptr);
    QVERIFY(!singlePreviewAction->isChecked());
}

void MainWindowMenuTest::addingOperatorShowsStepAndParameterDetails()
{
    MainWindow window;
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto* englishAction = window.findChild<QAction*>("englishLanguageAction");
    QVERIFY(englishAction != nullptr);
    englishAction->trigger();

    seedLoadedMedia(window);

    auto* transformMenu = window.findChild<QMenu*>("operatorCategoryMenu_enhancement_transform");
    QVERIFY(transformMenu != nullptr);
    auto* resizeAction = findActionInMenu(transformMenu, "operatorAction_builtin_resize");
    QVERIFY(resizeAction != nullptr);
    resizeAction->trigger();
    QVERIFY(!hasVisibleOperatorDialog());

    auto* pipelineStripList = window.findChild<QListWidget*>("pipelineStripList");
    QVERIFY(pipelineStripList != nullptr);
    QTRY_COMPARE(pipelineStripList->count(), 1);
    QCOMPARE(pipelineStripList->currentRow(), 0);

    auto* spinBox = window.findChild<QSpinBox*>();
    QVERIFY(spinBox != nullptr);

    auto* detailsLabel = window.findChild<QLabel*>("previewDetailsLabel");
    QVERIFY(detailsLabel != nullptr);
    QVERIFY(detailsLabel->text().contains("Resize"));
    QVERIFY(detailsLabel->text().contains("Width=640"));
}

int runMainWindowMenuTests(int argc, char* argv[])
{
    MainWindowMenuTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_mainwindow_menus.moc"
