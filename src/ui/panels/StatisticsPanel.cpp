#include "ui/panels/StatisticsPanel.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QFont>
#include <QSizePolicy>

namespace {

bool isChineseLanguage(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

QString localizedText(const QString& languageCode, const QString& englishText, const QString& chineseText)
{
    return isChineseLanguage(languageCode) ? chineseText : englishText;
}

}

StatisticsPanel::StatisticsPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("statisticsPanel");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 6, 4, 4);
    layout->setSpacing(10);

    m_titleLabel = new QLabel("Statistics", this);
    m_titleLabel->setObjectName("statisticsTitleLabel");
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->hide();
    layout->addWidget(m_titleLabel);

    m_metricsLabel = new QLabel(this);
    m_metricsLabel->setObjectName("statisticsMetricsLabel");
    m_metricsLabel->setWordWrap(true);
    m_metricsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_metricsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_metricsLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    layout->addWidget(m_metricsLabel);

    m_timingLabel = new QLabel(this);
    m_timingLabel->setObjectName("statisticsTimingLabel");
    m_timingLabel->setWordWrap(true);
    m_timingLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_timingLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_timingLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    layout->addWidget(m_timingLabel);

    layout->addStretch();
    clear();
}

void StatisticsPanel::setFrameMetrics(const QVariantMap& metrics)
{
    m_lastMetrics = metrics;
    if (metrics.isEmpty()) {
        refreshEmptyStateLabels();
        return;
    }

    QStringList lines;
    for (auto it = metrics.constBegin(); it != metrics.constEnd(); ++it) {
        lines << QString("%1: %2").arg(it.key(), it.value().toString());
    }
    m_metricsLabel->setText(lines.join('\n'));
}

void StatisticsPanel::setTimingSummary(const QString& summary)
{
    m_lastTimingSummary = summary;
    if (summary.isEmpty()) {
        m_timingLabel->setText(localizedText(
            m_languageCode,
            "No timing data.",
            QStringLiteral("\u6682\u65e0\u8017\u65f6\u6570\u636e\u3002")
        ));
        return;
    }

    m_timingLabel->setText(summary);
}

void StatisticsPanel::clear()
{
    m_lastMetrics.clear();
    m_lastTimingSummary.clear();
    refreshEmptyStateLabels();
}

void StatisticsPanel::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    m_titleLabel->setText(localizedText(
        languageCode,
        "Statistics",
        QStringLiteral("\u7edf\u8ba1")
    ));
    setFrameMetrics(m_lastMetrics);
    setTimingSummary(m_lastTimingSummary);
}

void StatisticsPanel::refreshEmptyStateLabels()
{
    m_metricsLabel->setText(localizedText(
        m_languageCode,
        "No frame metrics.",
        QStringLiteral("\u6682\u65e0\u5e27\u6307\u6807\u3002")
    ));
    m_timingLabel->setText(localizedText(
        m_languageCode,
        "No timing data.",
        QStringLiteral("\u6682\u65e0\u8017\u65f6\u6570\u636e\u3002")
    ));
}
