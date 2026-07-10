#pragma once

#include <QVariantMap>
#include <QWidget>

class QLabel;

class StatisticsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsPanel(QWidget* parent = nullptr);

    void setFrameMetrics(const QVariantMap& metrics);
    void setTimingSummary(const QString& summary);
    void clear();
    void setLanguage(const QString& languageCode);

private:
    void refreshEmptyStateLabels();

    QLabel* m_titleLabel = nullptr;
    QLabel* m_metricsLabel = nullptr;
    QLabel* m_timingLabel = nullptr;
    QString m_languageCode = "en";
    QVariantMap m_lastMetrics;
    QString m_lastTimingSummary;
};
