#include <QtTest>

#include <QTemporaryDir>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "core/operators/BuiltinOperatorBootstrap.h"
#include "core/pipeline/PipelineEngine.h"

class OperatorRegistryTest : public QObject
{
    Q_OBJECT

private slots:
    void registryCreatesBuiltinOperators();
    void registryExposesExpandedOperatorMetadata();
    void grayscaleOperatorConvertsColorFrame();
    void resizeOperatorResizesWorkingFrame();
    void registryCreatesExtendedBuiltinOperators();
    void registryCreatesExpandedFilterOperators();
    void registryCreatesExpandedEnhancementOperators();
    void registryCreatesExpandedThresholdOperators();
    void registryCreatesExpandedEdgeOperators();
    void registryCreatesExpandedColorOperators();
    void registryCreatesExpandedFilterEffectOperators();
    void registryCreatesExpandedDetectionOperators();
    void registryCreatesExpandedGeometryDetectionOperators();
    void registryCreatesExpandedSegmentationOperators();
    void registryCreatesExpandedHistogramOperators();
    void registryCreatesExpandedPlaceholderFeatureOperators();
    void registryCreatesExpandedInteractiveGeometryOperators();
    void registryCreatesExpandedInteractiveSegmentationOperators();
    void gaussianBlurOperatorPreservesFrameSize();
    void thresholdOperatorProducesBinaryOutput();
    void cannyOperatorProducesSingleChannelOutput();
    void morphologyOperatorPreservesFrameSize();
    void meanBlurOperatorPreservesFrameSize();
    void boxFilterOperatorPreservesFrameSize();
    void bilateralFilterOperatorPreservesFrameSize();
    void brightnessContrastOperatorPreservesFrameSize();
    void linearAbsTransformOperatorPreservesFrameSize();
    void histogramEqualizeOperatorPreservesFrameSize();
    void otsuThresholdOperatorProducesBinaryOutput();
    void triangleThresholdOperatorProducesBinaryOutput();
    void adaptiveThresholdOperatorProducesBinaryOutput();
    void sobelOperatorProducesSingleChannelOutput();
    void laplacianOperatorProducesSingleChannelOutput();
    void scharrOperatorProducesSingleChannelOutput();
    void colorConvertOperatorChangesChannelLayout();
    void colorMapOperatorProducesColorOutput();
    void pencilSketchOperatorProducesSketchOutput();
    void artEffectOperatorPreservesFrameSize();
    void contourDetectOperatorProducesColorOverlay();
    void connectedComponentsOperatorProducesColorLabels();
    void houghLinesOperatorProducesColorOverlay();
    void houghCirclesOperatorProducesColorOverlay();
    void cornerDetectOperatorProducesColorOverlay();
    void siftKeypointsOperatorProducesKeypointOverlay();
    void kmeansSegmentationOperatorProducesColorOutput();
    void meanShiftSegmentationOperatorProducesColorOutput();
    void watershedSegmentationOperatorProducesColorOutput();
    void histogramLineOperatorProducesChartImage();
    void histogramBarOperatorProducesChartImage();
    void surfKeypointsPlaceholderProducesOverlay();
    void surfMatchPlaceholderUsesSecondaryImagePath();
    void surfMatchPlaceholderUsesWorkspaceCachedImage();
    void thresholdOperatorSupportsToZeroInverted();
    void polarTransformOperatorProducesOutput();
    void fiberGlowOperatorProducesColorOutput();
    void embossOperatorProducesOutput();
    void specialEffectOperatorProducesColorOutput();
    void perspectiveTransformOperatorRequiresFourPointsAndWarpsImage();
    void affineTransformOperatorRequiresThreePointsAndWarpsImage();
    void floodFillOperatorRequiresSeedsAndProducesOutput();
    void floodFillOperatorSupportsAutoRandomSeedMode();
    void grabCutOperatorRequiresRectangleAndProducesOutput();
    void grabCutOperatorSupportsMaskInitMode();
    void pipelineRunsMultipleOperatorsInSequence();
};

void OperatorRegistryTest::registryCreatesBuiltinOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.grayscale") != nullptr);
    QVERIFY(registry.create("builtin.resize") != nullptr);
}

void OperatorRegistryTest::registryExposesExpandedOperatorMetadata()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    const QList<OperatorDescriptor> descriptors = registry.allOperators();
    QVERIFY(!descriptors.isEmpty());

    bool foundMorphology = false;
    for (const OperatorDescriptor& descriptor : descriptors) {
        if (descriptor.id != "builtin.morphology") {
            continue;
        }

        foundMorphology = true;
        QCOMPARE(descriptor.category, QStringLiteral("\u5f62\u6001\u5b66\u5904\u7406"));
        QCOMPARE(descriptor.capabilityType, OperatorCapabilityType::BasicSingleIO);
        QCOMPARE(descriptor.interactionType, OperatorInteractionType::None);
        QCOMPARE(descriptor.resultType, OperatorResultType::ImageOnly);
    }

    QVERIFY(foundMorphology);
}

void OperatorRegistryTest::grayscaleOperatorConvertsColorFrame()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.grayscale");
    QVERIFY(step != nullptr);

    FramePacket frame;
    frame.workingMat = cv::Mat(4, 4, CV_8UC3, cv::Scalar(10, 20, 30));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
}

void OperatorRegistryTest::resizeOperatorResizesWorkingFrame()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.resize");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"width", 5}, {"height", 3}});

    FramePacket frame;
    frame.workingMat = cv::Mat(10, 8, CV_8UC1, cv::Scalar(255));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.cols, 5);
    QCOMPARE(frame.workingMat.rows, 3);
}

void OperatorRegistryTest::registryCreatesExtendedBuiltinOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.gaussian_blur") != nullptr);
    QVERIFY(registry.create("builtin.median_blur") != nullptr);
    QVERIFY(registry.create("builtin.threshold") != nullptr);
    QVERIFY(registry.create("builtin.canny") != nullptr);
    QVERIFY(registry.create("builtin.morphology") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedFilterOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.mean_blur") != nullptr);
    QVERIFY(registry.create("builtin.box_filter") != nullptr);
    QVERIFY(registry.create("builtin.bilateral_filter") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedEnhancementOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.brightness_contrast") != nullptr);
    QVERIFY(registry.create("builtin.linear_abs_transform") != nullptr);
    QVERIFY(registry.create("builtin.polar_transform") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedThresholdOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.threshold_otsu") != nullptr);
    QVERIFY(registry.create("builtin.threshold_triangle") != nullptr);
    QVERIFY(registry.create("builtin.threshold_adaptive") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedEdgeOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.edge_sobel") != nullptr);
    QVERIFY(registry.create("builtin.edge_laplacian") != nullptr);
    QVERIFY(registry.create("builtin.edge_scharr") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedColorOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.color_convert") != nullptr);
    QVERIFY(registry.create("builtin.color_map") != nullptr);
    QVERIFY(registry.create("builtin.hist_equalize") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedFilterEffectOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.pencil_sketch") != nullptr);
    QVERIFY(registry.create("builtin.art_effect") != nullptr);
    QVERIFY(registry.create("builtin.fiber_glow") != nullptr);
    QVERIFY(registry.create("builtin.emboss") != nullptr);
    QVERIFY(registry.create("builtin.special_effect") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedDetectionOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.contour_detect") != nullptr);
    QVERIFY(registry.create("builtin.connected_components") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedGeometryDetectionOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.hough_lines") != nullptr);
    QVERIFY(registry.create("builtin.hough_circles") != nullptr);
    QVERIFY(registry.create("builtin.corner_detect") != nullptr);
    QVERIFY(registry.create("builtin.sift_keypoints") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedSegmentationOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.segment_kmeans") != nullptr);
    QVERIFY(registry.create("builtin.segment_meanshift") != nullptr);
    QVERIFY(registry.create("builtin.segment_watershed") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedInteractiveSegmentationOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.flood_fill") != nullptr);
    QVERIFY(registry.create("builtin.grabcut") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedHistogramOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.histogram_line") != nullptr);
    QVERIFY(registry.create("builtin.histogram_bar") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedPlaceholderFeatureOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.surf_keypoints") != nullptr);
    QVERIFY(registry.create("builtin.surf_match") != nullptr);
}

void OperatorRegistryTest::registryCreatesExpandedInteractiveGeometryOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.perspective_transform") != nullptr);
    QVERIFY(registry.create("builtin.affine_transform") != nullptr);
}

void OperatorRegistryTest::gaussianBlurOperatorPreservesFrameSize()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.gaussian_blur");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"kernelWidth", 5}, {"kernelHeight", 7}, {"sigmaX", 1.2}, {"sigmaY", 0.0}});

    FramePacket frame;
    frame.workingMat = cv::Mat(16, 12, CV_8UC3, cv::Scalar(10, 20, 30));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.cols, 12);
    QCOMPARE(frame.workingMat.rows, 16);
}

void OperatorRegistryTest::thresholdOperatorProducesBinaryOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.threshold");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"threshold", 127.0}, {"maxValue", 255.0}, {"type", "binary"}});

    FramePacket frame;
    frame.workingMat = cv::Mat(4, 4, CV_8UC1, cv::Scalar(200));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.type(), CV_8UC1);
    QCOMPARE(frame.workingMat.at<uchar>(0, 0), static_cast<uchar>(255));
}

void OperatorRegistryTest::cannyOperatorProducesSingleChannelOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.canny");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"threshold1", 50.0}, {"threshold2", 150.0}, {"apertureSize", 3}, {"useL2Gradient", false}});

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 8, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::rectangle(frame.workingMat, cv::Rect(2, 2, 4, 4), cv::Scalar(255, 255, 255), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
}

void OperatorRegistryTest::morphologyOperatorPreservesFrameSize()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.morphology");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"operation", "gradient"},
        {"kernelShape", "ellipse"},
        {"kernelWidth", 3},
        {"kernelHeight", 3},
        {"iterations", 1},
        {"borderType", "reflect101"}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(10, 10, CV_8UC1, cv::Scalar(0));
    frame.workingMat.at<uchar>(5, 5) = 255;

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.cols, 10);
    QCOMPARE(frame.workingMat.rows, 10);
}

void OperatorRegistryTest::meanBlurOperatorPreservesFrameSize()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.mean_blur");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"kernelWidth", 5}, {"kernelHeight", 3}});

    FramePacket frame;
    frame.workingMat = cv::Mat(9, 11, CV_8UC3, cv::Scalar(40, 50, 60));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.cols, 11);
    QCOMPARE(frame.workingMat.rows, 9);
}

void OperatorRegistryTest::boxFilterOperatorPreservesFrameSize()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.box_filter");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"kernelWidth", 5}, {"kernelHeight", 5}, {"normalize", true}});

    FramePacket frame;
    frame.workingMat = cv::Mat(9, 11, CV_8UC3, cv::Scalar(40, 50, 60));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.cols, 11);
    QCOMPARE(frame.workingMat.rows, 9);
}

void OperatorRegistryTest::bilateralFilterOperatorPreservesFrameSize()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.bilateral_filter");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"diameter", 9}, {"sigmaColor", 50.0}, {"sigmaSpace", 50.0}});

    FramePacket frame;
    frame.workingMat = cv::Mat(9, 11, CV_8UC3, cv::Scalar(40, 50, 60));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.cols, 11);
    QCOMPARE(frame.workingMat.rows, 9);
}

void OperatorRegistryTest::brightnessContrastOperatorPreservesFrameSize()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.brightness_contrast");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"alpha", 1.2}, {"beta", 10.0}});

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 10, CV_8UC3, cv::Scalar(10, 20, 30));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.cols, 10);
    QCOMPARE(frame.workingMat.rows, 8);
}

void OperatorRegistryTest::linearAbsTransformOperatorPreservesFrameSize()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.linear_abs_transform");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"k", -1.0}, {"b", 255.0}, {"applyAbs", true}});

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 10, CV_8UC1, cv::Scalar(50));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.cols, 10);
    QCOMPARE(frame.workingMat.rows, 8);
}

void OperatorRegistryTest::histogramEqualizeOperatorPreservesFrameSize()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.hist_equalize");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"mode", "global"}, {"colorMode", "gray_or_luminance"}});

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 10, CV_8UC3, cv::Scalar(10, 20, 30));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.cols, 10);
    QCOMPARE(frame.workingMat.rows, 8);
}

void OperatorRegistryTest::otsuThresholdOperatorProducesBinaryOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.threshold_otsu");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"maxValue", 255.0}, {"thresholdType", "binary"}});

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 8, CV_8UC1, cv::Scalar(180));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
}

void OperatorRegistryTest::triangleThresholdOperatorProducesBinaryOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.threshold_triangle");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"maxValue", 255.0}, {"thresholdType", "binary"}});

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 8, CV_8UC1, cv::Scalar(180));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
}

void OperatorRegistryTest::adaptiveThresholdOperatorProducesBinaryOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.threshold_adaptive");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"maxValue", 255.0},
        {"adaptiveMethod", "gaussian"},
        {"thresholdType", "binary"},
        {"blockSize", 11},
        {"cValue", 2.0}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 8, CV_8UC1, cv::Scalar(180));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
}

void OperatorRegistryTest::sobelOperatorProducesSingleChannelOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.edge_sobel");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"outputMode", "magnitude"},
        {"dx", 1},
        {"dy", 1},
        {"kernelSize", 3},
        {"scale", 1.0},
        {"delta", 0.0}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 8, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::rectangle(frame.workingMat, cv::Rect(2, 2, 4, 4), cv::Scalar(255, 255, 255), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
}

void OperatorRegistryTest::laplacianOperatorProducesSingleChannelOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.edge_laplacian");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"kernelSize", 3},
        {"scale", 1.0},
        {"delta", 0.0},
        {"convertAbs", true}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 8, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::rectangle(frame.workingMat, cv::Rect(2, 2, 4, 4), cv::Scalar(255, 255, 255), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
}

void OperatorRegistryTest::scharrOperatorProducesSingleChannelOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.edge_scharr");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"outputMode", "magnitude"},
        {"scale", 1.0},
        {"delta", 0.0}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 8, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::rectangle(frame.workingMat, cv::Rect(2, 2, 4, 4), cv::Scalar(255, 255, 255), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
}

void OperatorRegistryTest::colorConvertOperatorChangesChannelLayout()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.color_convert");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"conversion", "bgr_to_gray"}});

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 8, CV_8UC3, cv::Scalar(30, 40, 50));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
}

void OperatorRegistryTest::colorMapOperatorProducesColorOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.color_map");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"colorMap", "turbo"}, {"convertToGray", true}, {"invertGray", false}});

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 8, CV_8UC1, cv::Scalar(100));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::pencilSketchOperatorProducesSketchOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.pencil_sketch");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"outputMode", "gray"},
        {"sigmaS", 60.0},
        {"sigmaR", 0.07},
        {"shadeFactor", 0.02}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 8, CV_8UC3, cv::Scalar(80, 100, 120));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
}

void OperatorRegistryTest::artEffectOperatorPreservesFrameSize()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.art_effect");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"styleMode", "stylization"},
        {"sigmaS", 60.0},
        {"sigmaR", 0.45}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 8, CV_8UC3, cv::Scalar(80, 100, 120));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.cols, 8);
    QCOMPARE(frame.workingMat.rows, 8);
}

void OperatorRegistryTest::contourDetectOperatorProducesColorOverlay()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.contour_detect");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"retrievalMode", "external"},
        {"approxMode", "simple"},
        {"minArea", 1.0},
        {"maxArea", 0.0},
        {"drawMode", "outline"},
        {"thickness", 2}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(16, 16, CV_8UC1, cv::Scalar(0));
    cv::rectangle(frame.workingMat, cv::Rect(4, 4, 6, 6), cv::Scalar(255), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::connectedComponentsOperatorProducesColorLabels()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.connected_components");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"connectivity", 8},
        {"minArea", 1},
        {"maxArea", 0},
        {"colorize", true},
        {"drawBBox", true}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(16, 16, CV_8UC1, cv::Scalar(0));
    cv::rectangle(frame.workingMat, cv::Rect(2, 2, 4, 4), cv::Scalar(255), -1);
    cv::rectangle(frame.workingMat, cv::Rect(10, 10, 4, 4), cv::Scalar(255), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::houghLinesOperatorProducesColorOverlay()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.hough_lines");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"mode", "probabilistic"},
        {"rho", 1.0},
        {"thetaDeg", 1.0},
        {"threshold", 30},
        {"minLineLength", 5.0},
        {"maxLineGap", 2.0}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(32, 32, CV_8UC1, cv::Scalar(0));
    cv::line(frame.workingMat, cv::Point(4, 4), cv::Point(28, 4), cv::Scalar(255), 1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::houghCirclesOperatorProducesColorOverlay()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.hough_circles");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"dp", 1.0},
        {"minDist", 10.0},
        {"param1", 100.0},
        {"param2", 10.0},
        {"minRadius", 3},
        {"maxRadius", 10}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(32, 32, CV_8UC1, cv::Scalar(0));
    cv::circle(frame.workingMat, cv::Point(16, 16), 6, cv::Scalar(255), 1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::cornerDetectOperatorProducesColorOverlay()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.corner_detect");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"method", "shi_tomasi"},
        {"maxCorners", 20},
        {"qualityLevel", 0.01},
        {"minDistance", 2.0},
        {"blockSize", 3}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(32, 32, CV_8UC1, cv::Scalar(0));
    cv::rectangle(frame.workingMat, cv::Rect(8, 8, 12, 12), cv::Scalar(255), 1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::siftKeypointsOperatorProducesKeypointOverlay()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.sift_keypoints");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"nFeatures", 0},
        {"nOctaveLayers", 3},
        {"contrastThreshold", 0.04},
        {"edgeThreshold", 10.0},
        {"sigma", 1.6},
        {"drawRichKeypoints", true},
        {"maxDisplayCount", 50}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(64, 64, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::circle(frame.workingMat, cv::Point(20, 20), 6, cv::Scalar(255, 255, 255), -1);
    cv::rectangle(frame.workingMat, cv::Rect(36, 36, 12, 12), cv::Scalar(255, 255, 255), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::kmeansSegmentationOperatorProducesColorOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.segment_kmeans");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"clusterCount", 3},
        {"colorSpace", "lab"},
        {"includeSpatialXY", false},
        {"maxIterations", 10},
        {"epsilon", 1.0},
        {"attempts", 1}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(32, 32, CV_8UC3, cv::Scalar(20, 60, 120));
    cv::rectangle(frame.workingMat, cv::Rect(8, 8, 8, 8), cv::Scalar(200, 200, 30), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::meanShiftSegmentationOperatorProducesColorOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.segment_meanshift");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"spatialRadius", 10.0},
        {"colorRadius", 20.0},
        {"maxLevel", 1},
        {"termMaxCount", 5},
        {"termEpsilon", 1.0}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(32, 32, CV_8UC3, cv::Scalar(20, 60, 120));
    cv::rectangle(frame.workingMat, cv::Rect(8, 8, 8, 8), cv::Scalar(200, 200, 30), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::watershedSegmentationOperatorProducesColorOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.segment_watershed");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"markerMode", "distance_transform_auto"},
        {"openingKernel", 3},
        {"dilateIterations", 2},
        {"distanceThresholdRatio", 0.35},
        {"minMarkerArea", 8},
        {"boundaryColor", "#ff2d55"}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(32, 32, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::rectangle(frame.workingMat, cv::Rect(4, 4, 10, 10), cv::Scalar(255, 255, 255), -1);
    cv::rectangle(frame.workingMat, cv::Rect(18, 18, 10, 10), cv::Scalar(180, 180, 180), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::histogramLineOperatorProducesChartImage()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.histogram_line");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"channels", "gray"},
        {"bins", 64},
        {"rangeMin", 0.0},
        {"rangeMax", 255.0},
        {"normalize", true},
        {"yScale", "linear"}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(64, 64, CV_8UC1, cv::Scalar(80));
    cv::rectangle(frame.workingMat, cv::Rect(16, 16, 16, 16), cv::Scalar(200), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
    QCOMPARE(frame.workingMat.cols, 640);
    QCOMPARE(frame.workingMat.rows, 360);
}

void OperatorRegistryTest::histogramBarOperatorProducesChartImage()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.histogram_bar");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"channels", "gray"},
        {"bins", 64},
        {"rangeMin", 0.0},
        {"rangeMax", 255.0},
        {"normalize", true},
        {"barStyle", "filled"}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(64, 64, CV_8UC1, cv::Scalar(80));
    cv::rectangle(frame.workingMat, cv::Rect(16, 16, 16, 16), cv::Scalar(200), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
    QCOMPARE(frame.workingMat.cols, 640);
    QCOMPARE(frame.workingMat.rows, 360);
}

void OperatorRegistryTest::surfKeypointsPlaceholderProducesOverlay()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.surf_keypoints");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"backend", "sift"},
        {"hessianThreshold", 400.0},
        {"nOctaves", 4},
        {"nOctaveLayers", 3},
        {"extended", false},
        {"upright", false},
        {"maxDisplayCount", 50}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(64, 64, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::circle(frame.workingMat, cv::Point(20, 20), 6, cv::Scalar(255, 255, 255), -1);
    cv::rectangle(frame.workingMat, cv::Rect(36, 36, 12, 12), cv::Scalar(255, 255, 255), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::surfMatchPlaceholderUsesSecondaryImagePath()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.surf_match");
    QVERIFY(step != nullptr);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString secondaryPath = dir.filePath("secondary.png");

    FramePacket frame;
    frame.workingMat = cv::Mat(128, 128, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::circle(frame.workingMat, cv::Point(32, 32), 10, cv::Scalar(255, 255, 255), -1);
    cv::rectangle(frame.workingMat, cv::Rect(72, 72, 24, 24), cv::Scalar(255, 255, 255), -1);

    cv::Mat secondary = frame.workingMat.clone();
    QVERIFY(cv::imwrite(secondaryPath.toStdString(), secondary));
    step->setParameterValues({
        {"secondaryImagePath", secondaryPath},
        {"backend", "sift"},
        {"matcherType", "bf"},
        {"crossCheck", false},
        {"maxMatches", 20},
        {"useRansac", false},
    });

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::surfMatchPlaceholderUsesWorkspaceCachedImage()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.surf_match");
    QVERIFY(step != nullptr);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString secondaryPath = dir.filePath("workspace-cache.png");

    FramePacket frame;
    frame.workingMat = cv::Mat(128, 128, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::circle(frame.workingMat, cv::Point(36, 36), 10, cv::Scalar(255, 255, 255), -1);
    cv::rectangle(frame.workingMat, cv::Rect(72, 68, 24, 24), cv::Scalar(255, 255, 255), -1);

    cv::Mat secondary = frame.workingMat.clone();
    QVERIFY(cv::imwrite(secondaryPath.toStdString(), secondary));
    frame.artifacts.insert("workspace_cached_image_path", secondaryPath);

    step->setParameterValues({
        {"secondImageSource", "current_workspace_cache"},
        {"backend", "sift"},
        {"matcherType", "bf"},
        {"crossCheck", false},
        {"maxMatches", 20},
        {"useRansac", false}
    });

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
    QCOMPARE(frame.annotations.value("secondary_image_path").toString(), secondaryPath);
}

void OperatorRegistryTest::thresholdOperatorSupportsToZeroInverted()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.threshold");
    QVERIFY(step != nullptr);
    step->setParameterValues({{"threshold", 127.0}, {"maxValue", 255.0}, {"type", "tozero_inv"}});

    FramePacket frame;
    frame.workingMat = cv::Mat(1, 2, CV_8UC1);
    frame.workingMat.at<uchar>(0, 0) = 100;
    frame.workingMat.at<uchar>(0, 1) = 200;

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.at<uchar>(0, 0), static_cast<uchar>(100));
    QCOMPARE(frame.workingMat.at<uchar>(0, 1), static_cast<uchar>(0));
}

void OperatorRegistryTest::polarTransformOperatorProducesOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.polar_transform");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"direction", "cart_to_polar"},
        {"polarMode", "linear"},
        {"centerMode", "image_center"},
        {"radiusMode", "auto"},
        {"outputWidth", 64},
        {"outputHeight", 64},
        {"interpolation", "linear"},
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(32, 32, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::circle(frame.workingMat, cv::Point(16, 16), 8, cv::Scalar(255, 255, 255), 1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.cols, 64);
    QCOMPARE(frame.workingMat.rows, 64);
}

void OperatorRegistryTest::fiberGlowOperatorProducesColorOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.fiber_glow");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"angleDeg", 45.0},
        {"streakLength", 11},
        {"intensity", 1.5},
        {"threshold", 120.0},
        {"blendAlpha", 0.6},
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(32, 32, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::line(frame.workingMat, cv::Point(4, 4), cv::Point(28, 28), cv::Scalar(255, 255, 255), 2);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::embossOperatorProducesOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.emboss");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"direction", "north_west"},
        {"strength", 1.0},
        {"bias", 128.0},
        {"convertToGray", false},
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(16, 16, CV_8UC3, cv::Scalar(50, 80, 120));
    cv::rectangle(frame.workingMat, cv::Rect(4, 4, 6, 6), cv::Scalar(220, 220, 220), -1);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::specialEffectOperatorProducesColorOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.special_effect");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"preset", "negative"},
        {"strength", 1.0},
        {"radius", 5},
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(4, 4, CV_8UC3, cv::Scalar(10, 20, 30));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
    QCOMPARE(frame.workingMat.at<cv::Vec3b>(0, 0)[0], static_cast<uchar>(245));
}

void OperatorRegistryTest::perspectiveTransformOperatorRequiresFourPointsAndWarpsImage()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.perspective_transform");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"outputWidth", 128},
        {"outputHeight", 96},
        {"interpolation", "linear"},
        {"borderType", "constant"},
        {"borderValue", 0}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(64, 64, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::rectangle(frame.workingMat, cv::Rect(10, 10, 30, 30), cv::Scalar(255, 255, 255), -1);

    QVariantList pointList;
    for (const QPointF& point : {QPointF(10, 10), QPointF(40, 10), QPointF(40, 40), QPointF(10, 40)}) {
        QVariantMap map;
        map.insert("x", point.x());
        map.insert("y", point.y());
        pointList.append(map);
    }
    frame.artifacts.insert("perspective_points", pointList);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
    QCOMPARE(frame.workingMat.cols, 128);
    QCOMPARE(frame.workingMat.rows, 96);
}

void OperatorRegistryTest::affineTransformOperatorRequiresThreePointsAndWarpsImage()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.affine_transform");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"outputWidth", 128},
        {"outputHeight", 96},
        {"interpolation", "linear"},
        {"borderType", "constant"},
        {"borderValue", 0}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(64, 64, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::rectangle(frame.workingMat, cv::Rect(10, 10, 30, 30), cv::Scalar(255, 255, 255), -1);

    QVariantList pointList;
    for (const QPointF& point : {QPointF(10, 10), QPointF(40, 10), QPointF(10, 40)}) {
        QVariantMap map;
        map.insert("x", point.x());
        map.insert("y", point.y());
        pointList.append(map);
    }
    frame.artifacts.insert("affine_points", pointList);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
    QCOMPARE(frame.workingMat.cols, 128);
    QCOMPARE(frame.workingMat.rows, 96);
}

void OperatorRegistryTest::floodFillOperatorRequiresSeedsAndProducesOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.flood_fill");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"seedMode", "manual_click"},
        {"connectivity", 4},
        {"loDiff", 20},
        {"upDiff", 20},
        {"fixedRange", false},
        {"outputMode", "overlay"},
        {"fillColor", "#00ff99"}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(32, 32, CV_8UC3, cv::Scalar(10, 10, 10));
    cv::rectangle(frame.workingMat, cv::Rect(8, 8, 12, 12), cv::Scalar(200, 200, 200), -1);

    QVariantList seedList;
    QVariantMap seed;
    seed.insert("x", 10.0);
    seed.insert("y", 10.0);
    seedList.append(seed);
    frame.artifacts.insert("flood_fill_seeds", seedList);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::floodFillOperatorSupportsAutoRandomSeedMode()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.flood_fill");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"seedMode", "auto_random"},
        {"connectivity", 4},
        {"loDiff", 15},
        {"upDiff", 15},
        {"fixedRange", false},
        {"outputMode", "mask_only"},
        {"fillColor", "#00ff99"}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(24, 24, CV_8UC3, cv::Scalar(80, 80, 80));

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
    QCOMPARE(frame.metrics.value("flood_fill_seed_count").toInt(), 1);
}

void OperatorRegistryTest::grabCutOperatorRequiresRectangleAndProducesOutput()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.grabcut");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"initMode", "rect"},
        {"iterCount", 3},
        {"outputMode", "overlay"},
        {"featherRadius", 0}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(48, 48, CV_8UC3, cv::Scalar(10, 10, 10));
    cv::rectangle(frame.workingMat, cv::Rect(12, 12, 20, 20), cv::Scalar(220, 220, 220), -1);

    QVariantMap rectMap;
    rectMap.insert("x", 8.0);
    rectMap.insert("y", 8.0);
    rectMap.insert("width", 28.0);
    rectMap.insert("height", 28.0);
    frame.artifacts.insert("grabcut_rect", rectMap);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 3);
}

void OperatorRegistryTest::grabCutOperatorSupportsMaskInitMode()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto step = registry.create("builtin.grabcut");
    QVERIFY(step != nullptr);
    step->setParameterValues({
        {"initMode", "mask"},
        {"iterCount", 2},
        {"outputMode", "mask"},
        {"featherRadius", 0}
    });

    FramePacket frame;
    frame.workingMat = cv::Mat(48, 48, CV_8UC3, cv::Scalar(10, 10, 10));
    cv::rectangle(frame.workingMat, cv::Rect(12, 12, 20, 20), cv::Scalar(220, 220, 220), -1);

    QVariantMap rectMap;
    rectMap.insert("x", 8.0);
    rectMap.insert("y", 8.0);
    rectMap.insert("width", 28.0);
    rectMap.insert("height", 28.0);
    frame.artifacts.insert("grabcut_rect", rectMap);

    StepResult result = step->execute(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
    QVERIFY(cv::countNonZero(frame.workingMat) > 0);
}

void OperatorRegistryTest::pipelineRunsMultipleOperatorsInSequence()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    auto grayscale = registry.create("builtin.grayscale");
    auto resize = registry.create("builtin.resize");
    QVERIFY(grayscale != nullptr);
    QVERIFY(resize != nullptr);

    resize->setParameterValues({{"width", 6}, {"height", 4}});

    PipelineEngine engine;
    engine.setSteps({grayscale, resize});

    FramePacket frame;
    frame.workingMat = cv::Mat(8, 10, CV_8UC3, cv::Scalar(10, 20, 30));

    StepResult result = engine.run(frame, {});

    QVERIFY(result.success);
    QCOMPARE(frame.workingMat.channels(), 1);
    QCOMPARE(frame.workingMat.cols, 6);
    QCOMPARE(frame.workingMat.rows, 4);
}

int runOperatorRegistryTests(int argc, char* argv[])
{
    OperatorRegistryTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_operator_registry.moc"
