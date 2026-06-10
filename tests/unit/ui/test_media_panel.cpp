#include <QtTest>

#include <QPushButton>
#include <QSignalSpy>
#include <QSpinBox>

#include "ui/panels/MediaPanel.h"

class MediaPanelTest : public QObject
{
    Q_OBJECT

private slots:
    void emitsVideoFrameRequestedWhenFrameChanges();
    void emitsPlaybackControlSignals();
};

void MediaPanelTest::emitsVideoFrameRequestedWhenFrameChanges()
{
    MediaPanel panel;
    panel.setVideoNavigationState(true, 8, 0);

    auto* spinBox = panel.findChild<QSpinBox*>("videoFrameSpinBox");
    QVERIFY(spinBox != nullptr);

    QSignalSpy spy(&panel, SIGNAL(videoFrameRequested(int)));

    spinBox->setValue(3);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 3);
}

void MediaPanelTest::emitsPlaybackControlSignals()
{
    MediaPanel panel;
    panel.setVideoNavigationState(true, 8, 2);

    auto* previousButton = panel.findChild<QPushButton*>("previousVideoFrameButton");
    auto* nextButton = panel.findChild<QPushButton*>("nextVideoFrameButton");
    auto* playPauseButton = panel.findChild<QPushButton*>("playPauseVideoButton");
    QVERIFY(previousButton != nullptr);
    QVERIFY(nextButton != nullptr);
    QVERIFY(playPauseButton != nullptr);

    QSignalSpy previousSpy(&panel, SIGNAL(previousVideoFrameRequested()));
    QSignalSpy nextSpy(&panel, SIGNAL(nextVideoFrameRequested()));
    QSignalSpy playSpy(&panel, SIGNAL(videoPlaybackToggled(bool)));

    previousButton->click();
    nextButton->click();
    playPauseButton->click();

    QCOMPARE(previousSpy.count(), 1);
    QCOMPARE(nextSpy.count(), 1);
    QCOMPARE(playSpy.count(), 1);
    QCOMPARE(playSpy.takeFirst().at(0).toBool(), true);
    QCOMPARE(playPauseButton->text(), QString("Pause"));

    panel.setVideoPlaybackState(false);
    QCOMPARE(playPauseButton->text(), QString("Play"));
}

int runMediaPanelTests(int argc, char* argv[])
{
    MediaPanelTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_media_panel.moc"
