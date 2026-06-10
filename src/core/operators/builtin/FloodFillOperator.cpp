#include "core/operators/builtin/FloodFillOperator.h"

#include <QPoint>
#include <QRandomGenerator>

#include <opencv2/imgproc.hpp>

namespace {
cv::Scalar scalarFromHexColorFloodFill(const QString& hex)
{
    QString value = hex.trimmed();
    if (value.startsWith('#')) {
        value.remove(0, 1);
    }
    if (value.size() != 6) {
        return cv::Scalar(153, 255, 0);
    }

    const int r = value.mid(0, 2).toInt(nullptr, 16);
    const int g = value.mid(2, 2).toInt(nullptr, 16);
    const int b = value.mid(4, 2).toInt(nullptr, 16);
    return cv::Scalar(b, g, r);
}

cv::Mat ensureColorForFloodFill(const cv::Mat& source)
{
    if (source.channels() == 3) {
        return source.clone();
    }

    cv::Mat color;
    cv::cvtColor(source, color, cv::COLOR_GRAY2BGR);
    return color;
}

bool loadFloodFillSeedsFromArtifacts(const QVariantMap& artifacts, QVector<QPoint>* seeds)
{
    if (!seeds) {
        return false;
    }

    const QVariantList seedList = artifacts.value("flood_fill_seeds").toList();
    if (seedList.isEmpty()) {
        return false;
    }

    seeds->clear();
    for (const QVariant& seedValue : seedList) {
        const QVariantMap map = seedValue.toMap();
        if (!map.contains("x") || !map.contains("y")) {
            return false;
        }

        seeds->append(QPoint(
            qRound(map.value("x").toDouble()),
            qRound(map.value("y").toDouble())
        ));
    }
    return !seeds->isEmpty();
}

int floodFillFlags(int connectivity, bool fixedRange)
{
    int flags = (connectivity == 8) ? 8 : 4;
    if (fixedRange) {
        flags |= cv::FLOODFILL_FIXED_RANGE;
    }
    return flags;
}
}

QString FloodFillOperator::id() const
{
    return "builtin.flood_fill";
}

QString FloodFillOperator::displayName() const
{
    return "Flood Fill";
}

StepSchema FloodFillOperator::schema() const
{
    return {
        "builtin.flood_fill",
        "Flood Fill",
        {
            {"seedMode", "Seed Mode", "交互设置", StepParameterType::Choice, "manual_click", {}, {}, {}, {}, {}, {
                {"manual_click", "Manual Click"},
                {"auto_random", "Auto Random"},
            }},
            {"connectivity", "Connectivity", "填充设置", StepParameterType::Choice, "4", {}, {}, {}, {}, {}, {
                {"4", "4"},
                {"8", "8"},
            }},
            {"loDiff", "Lo Diff", "填充设置", StepParameterType::Integer, 20, 0, 255, 1, {}, {}, {}},
            {"upDiff", "Up Diff", "填充设置", StepParameterType::Integer, 20, 0, 255, 1, {}, {}, {}},
            {"fixedRange", "Fixed Range", "填充设置", StepParameterType::Boolean, false, {}, {}, {}, {}, {}, {}},
            {"outputMode", "Output Mode", "输出设置", StepParameterType::Choice, "overlay", {}, {}, {}, {}, {}, {
                {"overlay", "Overlay"},
                {"fill_color", "Fill Color"},
                {"mask_only", "Mask Only"},
            }},
            {"fillColor", "Fill Color", "输出设置", StepParameterType::String, "#00ff99", {}, {}, {}, "outputMode!=mask_only", {}, {}},
        }
    };
}

QVariantMap FloodFillOperator::parameterValues() const
{
    return {
        {"seedMode", m_seedMode},
        {"connectivity", m_connectivity},
        {"loDiff", m_loDiff},
        {"upDiff", m_upDiff},
        {"fixedRange", m_fixedRange},
        {"outputMode", m_outputMode},
        {"fillColor", m_fillColor},
    };
}

void FloodFillOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("seedMode")) m_seedMode = values.value("seedMode").toString();
    if (values.contains("connectivity")) m_connectivity = values.value("connectivity").toInt();
    if (values.contains("loDiff")) m_loDiff = values.value("loDiff").toInt();
    if (values.contains("upDiff")) m_upDiff = values.value("upDiff").toInt();
    if (values.contains("fixedRange")) m_fixedRange = values.value("fixedRange").toBool();
    if (values.contains("outputMode")) m_outputMode = values.value("outputMode").toString();
    if (values.contains("fillColor")) m_fillColor = values.value("fillColor").toString();
}

StepResult FloodFillOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    QVector<QPoint> seeds;
    if (m_seedMode == "auto_random") {
        if (frame.workingMat.cols <= 0 || frame.workingMat.rows <= 0) {
            return {false, "Flood fill requires a non-empty frame for random seed mode"};
        }
        seeds = {
            QPoint(
                QRandomGenerator::global()->bounded(frame.workingMat.cols),
                QRandomGenerator::global()->bounded(frame.workingMat.rows)
            )
        };
    } else if (!loadFloodFillSeedsFromArtifacts(frame.artifacts, &seeds)) {
        return {false, "Flood fill requires seed points in frame.artifacts[flood_fill_seeds]"};
    }

    const cv::Scalar fillColor = scalarFromHexColorFloodFill(m_fillColor);
    const int flags = floodFillFlags(m_connectivity, m_fixedRange);
    const cv::Scalar loDiff(m_loDiff, m_loDiff, m_loDiff);
    const cv::Scalar upDiff(m_upDiff, m_upDiff, m_upDiff);

    if (m_outputMode == "mask_only") {
        cv::Mat mask(frame.workingMat.rows + 2, frame.workingMat.cols + 2, CV_8UC1, cv::Scalar(0));
        cv::Mat source = frame.workingMat.clone();
        for (const QPoint& seed : seeds) {
            cv::floodFill(
                source,
                mask,
                cv::Point(seed.x(), seed.y()),
                fillColor,
                nullptr,
                loDiff,
                upDiff,
                flags | cv::FLOODFILL_MASK_ONLY | (255 << 8)
            );
        }

        frame.workingMat = mask(cv::Rect(1, 1, frame.workingMat.cols, frame.workingMat.rows)).clone();
        frame.metrics.insert("flood_fill_seed_count", seeds.size());
        return {};
    }

    cv::Mat colorImage = ensureColorForFloodFill(frame.workingMat);
    cv::Mat original = colorImage.clone();
    for (const QPoint& seed : seeds) {
        cv::floodFill(
            colorImage,
            cv::Point(seed.x(), seed.y()),
            fillColor,
            nullptr,
            loDiff,
            upDiff,
            flags
        );
    }

    if (m_outputMode == "overlay") {
        cv::Mat blended;
        cv::addWeighted(original, 0.45, colorImage, 0.55, 0.0, blended);
        frame.workingMat = blended;
    } else {
        frame.workingMat = colorImage;
    }

    frame.metrics.insert("flood_fill_seed_count", seeds.size());
    return {};
}
