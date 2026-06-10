#include <QtTest>

#include <QLabel>

#include "ui/widgets/CanvasView.h"

class CanvasViewTest : public QObject
{
    Q_OBJECT

private slots:
    void localizesPlaceholderText();
};

void CanvasViewTest::localizesPlaceholderText()
{
    CanvasView view;

    auto* label = view.findChild<QLabel*>();
    QVERIFY(label != nullptr);
    QCOMPARE(label->text(), QString("Preview Canvas"));

    view.setPlaceholderText(QString::fromUtf8("预览画布"));
    view.setImage(QImage());

    QCOMPARE(label->text(), QString::fromUtf8("预览画布"));
}

int runCanvasViewTests(int argc, char* argv[])
{
    CanvasViewTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_canvas_view.moc"
