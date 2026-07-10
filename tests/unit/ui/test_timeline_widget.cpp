#include <QtTest>

#include <QPushButton>
#include <QLabel>
#include <QSpinBox>

#include "app/AppDefaultsLoader.h"
#include "app/AppSettings.h"
#include "ui/widgets/TimelineWidget.h"

class TimelineWidgetTest : public QObject
{
    Q_OBJECT

private slots:
    void localizesIdleAndFrameStatus();
};

void TimelineWidgetTest::localizesIdleAndFrameStatus()
{
    TimelineWidget widget;
    widget.setLanguage("en");
    widget.setIdleState();
    QCOMPARE(widget.findChild<QLabel*>("timelineStatusLabel")->text(), QString("Timeline"));

    widget.setRange(0, 9);
    widget.updateFrameStatus(2, 10);
    QCOMPARE(widget.findChild<QLabel*>("timelineStatusLabel")->text(), QString("Frame 3 / 10"));

    widget.setLanguage("zh-CN");
    QCOMPARE(widget.findChild<QLabel*>("timelineStatusLabel")->text(), QStringLiteral("\u5e27 3 / 10"));

    widget.setIdleState();
    QCOMPARE(widget.findChild<QLabel*>("timelineStatusLabel")->text(), QStringLiteral("\u65f6\u95f4\u8f74"));
}

int runTimelineWidgetTests(int argc, char* argv[])
{
    TimelineWidgetTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_timeline_widget.moc"
