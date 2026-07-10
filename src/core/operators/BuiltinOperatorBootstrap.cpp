#include "core/operators/BuiltinOperatorBootstrap.h"

#include "core/operators/builtin/AdaptiveThresholdOperator.h"
#include "core/operators/builtin/AffineTransformOperator.h"
#include "core/operators/builtin/ArtEffectOperator.h"
#include "core/operators/builtin/BilateralFilterOperator.h"
#include "core/operators/builtin/BoxFilterOperator.h"
#include "core/operators/builtin/BrightnessContrastOperator.h"
#include "core/operators/builtin/CannyOperator.h"
#include "core/operators/builtin/ColorConvertOperator.h"
#include "core/operators/builtin/ColorMapOperator.h"
#include "core/operators/builtin/ConnectedComponentsOperator.h"
#include "core/operators/builtin/ContourDetectOperator.h"
#include "core/operators/builtin/CornerDetectOperator.h"
#include "core/operators/builtin/CropOperator.h"
#include "core/operators/builtin/EmbossOperator.h"
#include "core/operators/builtin/FiberGlowOperator.h"
#include "core/operators/builtin/FloodFillOperator.h"
#include "core/operators/builtin/FlipOperator.h"
#include "core/operators/builtin/GaussianBlurOperator.h"
#include "core/operators/builtin/GrabCutOperator.h"
#include "core/operators/builtin/GrayscaleOperator.h"
#include "core/operators/builtin/HistogramBarOperator.h"
#include "core/operators/builtin/HistogramEqualizeOperator.h"
#include "core/operators/builtin/HistogramLineOperator.h"
#include "core/operators/builtin/HoughCirclesOperator.h"
#include "core/operators/builtin/HoughLinesOperator.h"
#include "core/operators/builtin/KMeansSegmentationOperator.h"
#include "core/operators/builtin/LaplacianOperator.h"
#include "core/operators/builtin/LinearAbsTransformOperator.h"
#include "core/operators/builtin/MeanBlurOperator.h"
#include "core/operators/builtin/MeanShiftSegmentationOperator.h"
#include "core/operators/builtin/MedianBlurOperator.h"
#include "core/operators/builtin/MorphologyOperator.h"
#include "core/operators/builtin/OtsuThresholdOperator.h"
#include "core/operators/builtin/PencilSketchOperator.h"
#include "core/operators/builtin/PolarTransformOperator.h"
#include "core/operators/builtin/PerspectiveTransformOperator.h"
#include "core/operators/builtin/ResizeOperator.h"
#include "core/operators/builtin/RotateOperator.h"
#include "core/operators/builtin/ScharrOperator.h"
#include "core/operators/builtin/SiftKeypointsOperator.h"
#include "core/operators/builtin/SobelOperator.h"
#include "core/operators/builtin/SpecialEffectOperator.h"
#include "core/operators/builtin/SurfKeypointsPlaceholderOperator.h"
#include "core/operators/builtin/SurfMatchPlaceholderOperator.h"
#include "core/operators/builtin/TemplateMatchOperator.h"
#include "core/operators/builtin/ThresholdOperator.h"
#include "core/operators/builtin/TriangleThresholdOperator.h"
#include "core/operators/builtin/WatershedSegmentationOperator.h"

namespace {

const QString kCategoryMorphology = QStringLiteral("\u5f62\u6001\u5b66\u5904\u7406");
const QString kCategoryFiltering = QStringLiteral("\u6ee4\u6ce2");
const QString kCategoryEnhancementTransform = QStringLiteral("\u56fe\u50cf\u589e\u5f3a\u4e0e\u53d8\u6362");
const QString kCategoryThresholding = QStringLiteral("\u4e8c\u503c\u5316");
const QString kCategoryEdgeDetection = QStringLiteral("\u8fb9\u7f18\u68c0\u6d4b");
const QString kCategoryFilterEffects = QStringLiteral("\u6ee4\u955c\u6548\u679c");
const QString kCategoryImageDetection = QStringLiteral("\u56fe\u50cf\u68c0\u6d4b");
const QString kCategorySegmentation = QStringLiteral("\u56fe\u50cf\u5206\u5272");
const QString kCategoryFeatureMatching = QStringLiteral("\u7279\u5f81\u5339\u914d");

}

void registerBuiltinOperators(OperatorRegistry& registry)
{
    registry.registerOperator({"builtin.brightness_contrast", "Brightness Contrast", kCategoryEnhancementTransform,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<BrightnessContrastOperator>();
    }});

    registry.registerOperator({"builtin.linear_abs_transform", "Linear Abs Transform", kCategoryEnhancementTransform,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<LinearAbsTransformOperator>();
    }});

    registry.registerOperator({"builtin.hist_equalize", "Histogram Equalize", kCategoryEnhancementTransform,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<HistogramEqualizeOperator>();
    }});

    registry.registerOperator({"builtin.histogram_line", "Histogram Line", kCategoryEnhancementTransform,
        OperatorCapabilityType::MultiResultOrMultiInput,
        OperatorInteractionType::None,
        OperatorResultType::ImageAndChart, [] {
        return std::make_shared<HistogramLineOperator>();
    }});

    registry.registerOperator({"builtin.histogram_bar", "Histogram Bar", kCategoryEnhancementTransform,
        OperatorCapabilityType::MultiResultOrMultiInput,
        OperatorInteractionType::None,
        OperatorResultType::ImageAndChart, [] {
        return std::make_shared<HistogramBarOperator>();
    }});

    registry.registerOperator({"builtin.mean_blur", "Mean Blur", kCategoryFiltering,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<MeanBlurOperator>();
    }});

    registry.registerOperator({"builtin.box_filter", "Box Filter", kCategoryFiltering,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<BoxFilterOperator>();
    }});

    registry.registerOperator({"builtin.bilateral_filter", "Bilateral Filter", kCategoryFiltering,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<BilateralFilterOperator>();
    }});

    registry.registerOperator({"builtin.gaussian_blur", "Gaussian Blur", kCategoryFiltering,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<GaussianBlurOperator>();
    }});

    registry.registerOperator({"builtin.median_blur", "Median Blur", kCategoryFiltering,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<MedianBlurOperator>();
    }});

    registry.registerOperator({"builtin.affine_transform", "Affine Transform", kCategoryEnhancementTransform,
        OperatorCapabilityType::InteractiveSingleInput,
        OperatorInteractionType::PointsOnSource,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<AffineTransformOperator>();
    }});

    registry.registerOperator({"builtin.polar_transform", "Polar Transform", kCategoryEnhancementTransform,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<PolarTransformOperator>();
    }});

    registry.registerOperator({"builtin.perspective_transform", "Perspective Transform", kCategoryEnhancementTransform,
        OperatorCapabilityType::InteractiveSingleInput,
        OperatorInteractionType::PointsOnSource,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<PerspectiveTransformOperator>();
    }});

    registry.registerOperator({"builtin.resize", "Resize", kCategoryEnhancementTransform,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<ResizeOperator>();
    }});

    registry.registerOperator({"builtin.crop", "Crop", kCategoryEnhancementTransform,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<CropOperator>();
    }});

    registry.registerOperator({"builtin.rotate", "Rotate", kCategoryEnhancementTransform,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<RotateOperator>();
    }});

    registry.registerOperator({"builtin.flip", "Flip", kCategoryEnhancementTransform,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<FlipOperator>();
    }});

    registry.registerOperator({"builtin.threshold", "Threshold", kCategoryThresholding,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<ThresholdOperator>();
    }});

    registry.registerOperator({"builtin.threshold_otsu", "Otsu Threshold", kCategoryThresholding,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<OtsuThresholdOperator>();
    }});

    registry.registerOperator({"builtin.threshold_triangle", "Triangle Threshold", kCategoryThresholding,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<TriangleThresholdOperator>();
    }});

    registry.registerOperator({"builtin.threshold_adaptive", "Adaptive Threshold", kCategoryThresholding,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<AdaptiveThresholdOperator>();
    }});

    registry.registerOperator({"builtin.canny", "Canny", kCategoryEdgeDetection,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<CannyOperator>();
    }});

    registry.registerOperator({"builtin.edge_sobel", "Sobel", kCategoryEdgeDetection,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<SobelOperator>();
    }});

    registry.registerOperator({"builtin.edge_laplacian", "Laplacian", kCategoryEdgeDetection,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<LaplacianOperator>();
    }});

    registry.registerOperator({"builtin.edge_scharr", "Scharr", kCategoryEdgeDetection,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<ScharrOperator>();
    }});

    registry.registerOperator({"builtin.grayscale", "Grayscale", kCategoryFilterEffects,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<GrayscaleOperator>();
    }});

    registry.registerOperator({"builtin.color_convert", "Color Convert", kCategoryFilterEffects,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<ColorConvertOperator>();
    }});

    registry.registerOperator({"builtin.color_map", "Color Map", kCategoryFilterEffects,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<ColorMapOperator>();
    }});

    registry.registerOperator({"builtin.pencil_sketch", "Pencil Sketch", kCategoryFilterEffects,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<PencilSketchOperator>();
    }});

    registry.registerOperator({"builtin.art_effect", "Art Effect", kCategoryFilterEffects,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<ArtEffectOperator>();
    }});

    registry.registerOperator({"builtin.fiber_glow", "Fiber Glow", kCategoryFilterEffects,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<FiberGlowOperator>();
    }});

    registry.registerOperator({"builtin.emboss", "Emboss", kCategoryFilterEffects,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<EmbossOperator>();
    }});

    registry.registerOperator({"builtin.special_effect", "Special Effect", kCategoryFilterEffects,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<SpecialEffectOperator>();
    }});

    registry.registerOperator({"builtin.contour_detect", "Contour Detect", kCategoryImageDetection,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageWithStats, [] {
        return std::make_shared<ContourDetectOperator>();
    }});

    registry.registerOperator({"builtin.hough_lines", "Hough Lines", kCategoryImageDetection,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageWithStats, [] {
        return std::make_shared<HoughLinesOperator>();
    }});

    registry.registerOperator({"builtin.hough_circles", "Hough Circles", kCategoryImageDetection,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageWithStats, [] {
        return std::make_shared<HoughCirclesOperator>();
    }});

    registry.registerOperator({"builtin.corner_detect", "Corner Detect", kCategoryImageDetection,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageWithStats, [] {
        return std::make_shared<CornerDetectOperator>();
    }});

    registry.registerOperator({"builtin.connected_components", "Connected Components", kCategoryImageDetection,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageWithStats, [] {
        return std::make_shared<ConnectedComponentsOperator>();
    }});

    registry.registerOperator({"builtin.template_match", "Template Match", kCategoryImageDetection,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageWithStats, [] {
        return std::make_shared<TemplateMatchOperator>();
    }});

    registry.registerOperator({"builtin.surf_keypoints", "SURF Keypoints", kCategoryImageDetection,
        OperatorCapabilityType::MultiResultOrMultiInput,
        OperatorInteractionType::None,
        OperatorResultType::ImageAndKeypoints, [] {
        return std::make_shared<SurfKeypointsPlaceholderOperator>();
    }});

    registry.registerOperator({"builtin.sift_keypoints", "SIFT Keypoints", kCategoryImageDetection,
        OperatorCapabilityType::MultiResultOrMultiInput,
        OperatorInteractionType::None,
        OperatorResultType::ImageAndKeypoints, [] {
        return std::make_shared<SiftKeypointsOperator>();
    }});

    registry.registerOperator({"builtin.segment_kmeans", "K-Means Segmentation", kCategorySegmentation,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<KMeansSegmentationOperator>();
    }});

    registry.registerOperator({"builtin.flood_fill", "Flood Fill", kCategorySegmentation,
        OperatorCapabilityType::InteractiveSingleInput,
        OperatorInteractionType::SeedPointsOnSource,
        OperatorResultType::ImageAndMask, [] {
        return std::make_shared<FloodFillOperator>();
    }});

    registry.registerOperator({"builtin.segment_watershed", "Watershed Segmentation", kCategorySegmentation,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageWithStats, [] {
        return std::make_shared<WatershedSegmentationOperator>();
    }});

    registry.registerOperator({"builtin.grabcut", "GrabCut", kCategorySegmentation,
        OperatorCapabilityType::InteractiveSingleInput,
        OperatorInteractionType::RectOnSource,
        OperatorResultType::ImageAndMask, [] {
        return std::make_shared<GrabCutOperator>();
    }});

    registry.registerOperator({"builtin.segment_meanshift", "MeanShift Segmentation", kCategorySegmentation,
        OperatorCapabilityType::AdvancedSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<MeanShiftSegmentationOperator>();
    }});

    registry.registerOperator({"builtin.surf_match", "SURF Match", kCategoryFeatureMatching,
        OperatorCapabilityType::MultiResultOrMultiInput,
        OperatorInteractionType::SecondImageRequired,
        OperatorResultType::ImageAndMatches, [] {
        return std::make_shared<SurfMatchPlaceholderOperator>();
    }});

    registry.registerOperator({"builtin.morphology", "Morphology", kCategoryMorphology,
        OperatorCapabilityType::BasicSingleIO,
        OperatorInteractionType::None,
        OperatorResultType::ImageOnly, [] {
        return std::make_shared<MorphologyOperator>();
    }});
}
