cmake_minimum_required(VERSION 3.21)

project(CVVerify VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

option(CVVERIFY_BUILD_TESTS "Build CVVerify tests" ON)

set(CVVERIFY_QT_RUNTIME_BIN "" CACHE PATH "Optional Qt runtime bin directory for local execution")
set(CVVERIFY_OPENCV_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/opencvsdk/windows/opencv4.12" CACHE PATH "Bundled OpenCV SDK root")
set(_CVVERIFY_DEFAULT_OPENCV_DIR "${CVVERIFY_OPENCV_SDK_ROOT}/x64/mingw/lib")
set(_CVVERIFY_DEFAULT_OPENCV_RUNTIME_BIN "${CVVERIFY_OPENCV_SDK_ROOT}/x64/mingw/bin")

if(NOT DEFINED CVVERIFY_OPENCV_DIR OR CVVERIFY_OPENCV_DIR STREQUAL "" OR CVVERIFY_OPENCV_DIR MATCHES "^C:/msys64/mingw64")
    set(CVVERIFY_OPENCV_DIR "${_CVVERIFY_DEFAULT_OPENCV_DIR}" CACHE PATH "OpenCVConfig.cmake directory" FORCE)
else()
    set(CVVERIFY_OPENCV_DIR "${CVVERIFY_OPENCV_DIR}" CACHE PATH "OpenCVConfig.cmake directory")
endif()

if(NOT DEFINED CVVERIFY_OPENCV_RUNTIME_BIN OR CVVERIFY_OPENCV_RUNTIME_BIN STREQUAL "" OR CVVERIFY_OPENCV_RUNTIME_BIN MATCHES "^C:/msys64/mingw64")
    set(CVVERIFY_OPENCV_RUNTIME_BIN "${_CVVERIFY_DEFAULT_OPENCV_RUNTIME_BIN}" CACHE PATH "OpenCV runtime bin directory" FORCE)
else()
    set(CVVERIFY_OPENCV_RUNTIME_BIN "${CVVERIFY_OPENCV_RUNTIME_BIN}" CACHE PATH "OpenCV runtime bin directory")
endif()

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Target platform: Windows first, cross-platform ready")

if(CVVERIFY_OPENCV_DIR AND EXISTS "${CVVERIFY_OPENCV_DIR}/OpenCVConfig.cmake")
    set(OpenCV_DIR "${CVVERIFY_OPENCV_DIR}" CACHE PATH "Directory containing OpenCVConfig.cmake" FORCE)
    list(PREPEND CMAKE_PREFIX_PATH "${CVVERIFY_OPENCV_DIR}")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Test)
find_package(OpenCV REQUIRED COMPONENTS
    core
    imgproc
    imgcodecs
    videoio
    dnn
    photo
    features2d
    flann
    calib3d
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_standard_project_setup()
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(CVVerify
        src/main.cpp
        src/app/AppSettings.h
        src/app/AppSettings.cpp
        src/core/detection/DetectionTypes.h
        src/core/detection/DetectionModelDescriptor.h
        src/core/detection/ModelPackageLoader.h
        src/core/detection/ModelPackageLoader.cpp
        src/core/detection/LabelProvider.h
        src/core/detection/LabelProvider.cpp
        src/core/detection/OpenCvDnnRunner.h
        src/core/detection/OpenCvDnnRunner.cpp
        src/core/detection/YoloOnnxImportService.h
        src/core/detection/YoloOnnxImportService.cpp
        src/core/detection/DetectionRenderComposer.h
        src/core/detection/DetectionRenderComposer.cpp
        src/core/detection/DetectionExportService.h
        src/core/detection/DetectionExportService.cpp
        src/core/detection/YoloDetectionDecoder.h
        src/core/detection/YoloDetectionDecoder.cpp
        src/core/detection/YoloDetectionStep.h
        src/core/detection/YoloDetectionStep.cpp
        src/ui/dialogs/YoloModelImportDialog.h
        src/ui/dialogs/YoloModelImportDialog.cpp
        src/ui/dialogs/YoloModelImportDialog.ui
        src/ui/dialogs/SystemSettingsDialog.h
        src/ui/dialogs/SystemSettingsDialog.cpp
        src/ui/dialogs/SystemSettingsDialog.ui
        src/ui/dialogs/OperatorParameterDialog.h
        src/ui/dialogs/OperatorParameterDialog.cpp
        src/ui/dialogs/OperatorParameterDialog.ui
        src/core/frame/FramePacket.h
        src/core/media/MediaTypes.h
        src/core/media/IMediaSource.h
        src/core/media/ImageFileSource.h
        src/core/media/ImageFileSource.cpp
        src/core/media/VideoFileSource.h
        src/core/media/VideoFileSource.cpp
        src/core/operators/OperatorDescriptor.h
        src/core/operators/OperatorRegistry.h
        src/core/operators/OperatorRegistry.cpp
        src/core/operators/BuiltinOperatorBootstrap.h
        src/core/operators/BuiltinOperatorBootstrap.cpp
        src/core/operators/builtin/AdaptiveThresholdOperator.h
        src/core/operators/builtin/AdaptiveThresholdOperator.cpp
        src/core/operators/builtin/AffineTransformOperator.h
        src/core/operators/builtin/AffineTransformOperator.cpp
        src/core/operators/builtin/BilateralFilterOperator.h
        src/core/operators/builtin/BilateralFilterOperator.cpp
        src/core/operators/builtin/BoxFilterOperator.h
        src/core/operators/builtin/BoxFilterOperator.cpp
        src/core/operators/builtin/BrightnessContrastOperator.h
        src/core/operators/builtin/BrightnessContrastOperator.cpp
        src/core/operators/builtin/GaussianBlurOperator.h
        src/core/operators/builtin/GaussianBlurOperator.cpp
        src/core/operators/builtin/GrayscaleOperator.h
        src/core/operators/builtin/GrayscaleOperator.cpp
        src/core/operators/builtin/ColorConvertOperator.h
        src/core/operators/builtin/ColorConvertOperator.cpp
        src/core/operators/builtin/ColorMapOperator.h
        src/core/operators/builtin/ColorMapOperator.cpp
        src/core/operators/builtin/ConnectedComponentsOperator.h
        src/core/operators/builtin/ConnectedComponentsOperator.cpp
        src/core/operators/builtin/ContourDetectOperator.h
        src/core/operators/builtin/ContourDetectOperator.cpp
        src/core/operators/builtin/CornerDetectOperator.h
        src/core/operators/builtin/CornerDetectOperator.cpp
        src/core/operators/builtin/EmbossOperator.h
        src/core/operators/builtin/EmbossOperator.cpp
        src/core/operators/builtin/FiberGlowOperator.h
        src/core/operators/builtin/FiberGlowOperator.cpp
        src/core/operators/builtin/FloodFillOperator.h
        src/core/operators/builtin/FloodFillOperator.cpp
        src/core/operators/builtin/GrabCutOperator.h
        src/core/operators/builtin/GrabCutOperator.cpp
        src/core/operators/builtin/HistogramBarOperator.h
        src/core/operators/builtin/HistogramBarOperator.cpp
        src/core/operators/builtin/HistogramEqualizeOperator.h
        src/core/operators/builtin/HistogramEqualizeOperator.cpp
        src/core/operators/builtin/HistogramLineOperator.h
        src/core/operators/builtin/HistogramLineOperator.cpp
        src/core/operators/builtin/HoughCirclesOperator.h
        src/core/operators/builtin/HoughCirclesOperator.cpp
        src/core/operators/builtin/HoughLinesOperator.h
        src/core/operators/builtin/HoughLinesOperator.cpp
        src/core/operators/builtin/KMeansSegmentationOperator.h
        src/core/operators/builtin/KMeansSegmentationOperator.cpp
        src/core/operators/builtin/LaplacianOperator.h
        src/core/operators/builtin/LaplacianOperator.cpp
        src/core/operators/builtin/LinearAbsTransformOperator.h
        src/core/operators/builtin/LinearAbsTransformOperator.cpp
        src/core/operators/builtin/MedianBlurOperator.h
        src/core/operators/builtin/MedianBlurOperator.cpp
        src/core/operators/builtin/MeanBlurOperator.h
        src/core/operators/builtin/MeanBlurOperator.cpp
        src/core/operators/builtin/MeanShiftSegmentationOperator.h
        src/core/operators/builtin/MeanShiftSegmentationOperator.cpp
        src/core/operators/builtin/SurfKeypointsPlaceholderOperator.h
        src/core/operators/builtin/SurfKeypointsPlaceholderOperator.cpp
        src/core/operators/builtin/SurfMatchPlaceholderOperator.h
        src/core/operators/builtin/SurfMatchPlaceholderOperator.cpp
        src/core/operators/builtin/ThresholdOperator.h
        src/core/operators/builtin/ThresholdOperator.cpp
        src/core/operators/builtin/CannyOperator.h
        src/core/operators/builtin/CannyOperator.cpp
        src/core/operators/builtin/MorphologyOperator.h
        src/core/operators/builtin/MorphologyOperator.cpp
        src/core/operators/builtin/OtsuThresholdOperator.h
        src/core/operators/builtin/OtsuThresholdOperator.cpp
        src/core/operators/builtin/ArtEffectOperator.h
        src/core/operators/builtin/ArtEffectOperator.cpp
        src/core/operators/builtin/PencilSketchOperator.h
        src/core/operators/builtin/PencilSketchOperator.cpp
        src/core/operators/builtin/PolarTransformOperator.h
        src/core/operators/builtin/PolarTransformOperator.cpp
        src/core/operators/builtin/PerspectiveTransformOperator.h
        src/core/operators/builtin/PerspectiveTransformOperator.cpp
        src/core/operators/builtin/ResizeOperator.h
        src/core/operators/builtin/ResizeOperator.cpp
        src/core/operators/builtin/ScharrOperator.h
        src/core/operators/builtin/ScharrOperator.cpp
        src/core/operators/builtin/SiftKeypointsOperator.h
        src/core/operators/builtin/SiftKeypointsOperator.cpp
        src/core/operators/builtin/SobelOperator.h
        src/core/operators/builtin/SobelOperator.cpp
        src/core/operators/builtin/SpecialEffectOperator.h
        src/core/operators/builtin/SpecialEffectOperator.cpp
        src/core/operators/builtin/TriangleThresholdOperator.h
        src/core/operators/builtin/TriangleThresholdOperator.cpp
        src/core/operators/builtin/WatershedSegmentationOperator.h
        src/core/operators/builtin/WatershedSegmentationOperator.cpp
        src/core/pipeline/IPipelineStep.h
        src/core/pipeline/PipelineDefinition.h
        src/core/pipeline/PipelineEngine.h
        src/core/pipeline/PipelineEngine.cpp
        src/core/pipeline/PipelineJsonSerializer.h
        src/core/pipeline/PipelineJsonSerializer.cpp
        src/infra/opencv/OpenCvImageIO.h
        src/infra/opencv/OpenCvImageIO.cpp
        src/infra/opencv/OpenCvQtImageConverter.h
        src/infra/opencv/OpenCvQtImageConverter.cpp
        src/ui/mainwindow/MainWindow.h
        src/ui/mainwindow/MainWindow.cpp
        src/ui/mainwindow/MainWindow.ui
        src/ui/panels/ExportResultsPanel.h
        src/ui/panels/ExportResultsPanel.cpp
        src/ui/panels/ExportResultsPanel.ui
        src/ui/panels/MediaPanel.h
        src/ui/panels/MediaPanel.cpp
        src/ui/panels/PipelinePanel.h
        src/ui/panels/PipelinePanel.cpp
        src/ui/panels/ParameterPanel.h
        src/ui/panels/ParameterPanel.cpp
        src/ui/widgets/CanvasView.h
        src/ui/widgets/CanvasView.cpp
    )
else()
    add_executable(CVVerify
        src/main.cpp
        src/app/AppSettings.h
        src/app/AppSettings.cpp
        src/core/detection/DetectionTypes.h
        src/core/detection/DetectionModelDescriptor.h
        src/core/detection/ModelPackageLoader.h
        src/core/detection/ModelPackageLoader.cpp
        src/core/detection/LabelProvider.h
        src/core/detection/LabelProvider.cpp
        src/core/detection/OpenCvDnnRunner.h
        src/core/detection/OpenCvDnnRunner.cpp
        src/core/detection/YoloOnnxImportService.h
        src/core/detection/YoloOnnxImportService.cpp
        src/core/detection/DetectionRenderComposer.h
        src/core/detection/DetectionRenderComposer.cpp
        src/core/detection/DetectionExportService.h
        src/core/detection/DetectionExportService.cpp
        src/core/detection/YoloDetectionDecoder.h
        src/core/detection/YoloDetectionDecoder.cpp
        src/core/detection/YoloDetectionStep.h
        src/core/detection/YoloDetectionStep.cpp
        src/ui/dialogs/YoloModelImportDialog.h
        src/ui/dialogs/YoloModelImportDialog.cpp
        src/ui/dialogs/YoloModelImportDialog.ui
        src/ui/dialogs/SystemSettingsDialog.h
        src/ui/dialogs/SystemSettingsDialog.cpp
        src/ui/dialogs/SystemSettingsDialog.ui
        src/ui/dialogs/OperatorParameterDialog.h
        src/ui/dialogs/OperatorParameterDialog.cpp
        src/ui/dialogs/OperatorParameterDialog.ui
        src/core/frame/FramePacket.h
        src/core/media/MediaTypes.h
        src/core/media/IMediaSource.h
        src/core/media/ImageFileSource.h
        src/core/media/ImageFileSource.cpp
        src/core/media/VideoFileSource.h
        src/core/media/VideoFileSource.cpp
        src/core/operators/OperatorDescriptor.h
        src/core/operators/OperatorRegistry.h
        src/core/operators/OperatorRegistry.cpp
        src/core/operators/BuiltinOperatorBootstrap.h
        src/core/operators/BuiltinOperatorBootstrap.cpp
        src/core/operators/builtin/AdaptiveThresholdOperator.h
        src/core/operators/builtin/AdaptiveThresholdOperator.cpp
        src/core/operators/builtin/AffineTransformOperator.h
        src/core/operators/builtin/AffineTransformOperator.cpp
        src/core/operators/builtin/BilateralFilterOperator.h
        src/core/operators/builtin/BilateralFilterOperator.cpp
        src/core/operators/builtin/BoxFilterOperator.h
        src/core/operators/builtin/BoxFilterOperator.cpp
        src/core/operators/builtin/BrightnessContrastOperator.h
        src/core/operators/builtin/BrightnessContrastOperator.cpp
        src/core/operators/builtin/GaussianBlurOperator.h
        src/core/operators/builtin/GaussianBlurOperator.cpp
        src/core/operators/builtin/GrayscaleOperator.h
        src/core/operators/builtin/GrayscaleOperator.cpp
        src/core/operators/builtin/ColorConvertOperator.h
        src/core/operators/builtin/ColorConvertOperator.cpp
        src/core/operators/builtin/ColorMapOperator.h
        src/core/operators/builtin/ColorMapOperator.cpp
        src/core/operators/builtin/ConnectedComponentsOperator.h
        src/core/operators/builtin/ConnectedComponentsOperator.cpp
        src/core/operators/builtin/ContourDetectOperator.h
        src/core/operators/builtin/ContourDetectOperator.cpp
        src/core/operators/builtin/CornerDetectOperator.h
        src/core/operators/builtin/CornerDetectOperator.cpp
        src/core/operators/builtin/EmbossOperator.h
        src/core/operators/builtin/EmbossOperator.cpp
        src/core/operators/builtin/FiberGlowOperator.h
        src/core/operators/builtin/FiberGlowOperator.cpp
        src/core/operators/builtin/FloodFillOperator.h
        src/core/operators/builtin/FloodFillOperator.cpp
        src/core/operators/builtin/GrabCutOperator.h
        src/core/operators/builtin/GrabCutOperator.cpp
        src/core/operators/builtin/HistogramBarOperator.h
        src/core/operators/builtin/HistogramBarOperator.cpp
        src/core/operators/builtin/HistogramEqualizeOperator.h
        src/core/operators/builtin/HistogramEqualizeOperator.cpp
        src/core/operators/builtin/HistogramLineOperator.h
        src/core/operators/builtin/HistogramLineOperator.cpp
        src/core/operators/builtin/HoughCirclesOperator.h
        src/core/operators/builtin/HoughCirclesOperator.cpp
        src/core/operators/builtin/HoughLinesOperator.h
        src/core/operators/builtin/HoughLinesOperator.cpp
        src/core/operators/builtin/KMeansSegmentationOperator.h
        src/core/operators/builtin/KMeansSegmentationOperator.cpp
        src/core/operators/builtin/LaplacianOperator.h
        src/core/operators/builtin/LaplacianOperator.cpp
        src/core/operators/builtin/LinearAbsTransformOperator.h
        src/core/operators/builtin/LinearAbsTransformOperator.cpp
        src/core/operators/builtin/MedianBlurOperator.h
        src/core/operators/builtin/MedianBlurOperator.cpp
        src/core/operators/builtin/MeanBlurOperator.h
        src/core/operators/builtin/MeanBlurOperator.cpp
        src/core/operators/builtin/MeanShiftSegmentationOperator.h
        src/core/operators/builtin/MeanShiftSegmentationOperator.cpp
        src/core/operators/builtin/SurfKeypointsPlaceholderOperator.h
        src/core/operators/builtin/SurfKeypointsPlaceholderOperator.cpp
        src/core/operators/builtin/SurfMatchPlaceholderOperator.h
        src/core/operators/builtin/SurfMatchPlaceholderOperator.cpp
        src/core/operators/builtin/ThresholdOperator.h
        src/core/operators/builtin/ThresholdOperator.cpp
        src/core/operators/builtin/CannyOperator.h
        src/core/operators/builtin/CannyOperator.cpp
        src/core/operators/builtin/MorphologyOperator.h
        src/core/operators/builtin/MorphologyOperator.cpp
        src/core/operators/builtin/OtsuThresholdOperator.h
        src/core/operators/builtin/OtsuThresholdOperator.cpp
        src/core/operators/builtin/ArtEffectOperator.h
        src/core/operators/builtin/ArtEffectOperator.cpp
        src/core/operators/builtin/PencilSketchOperator.h
        src/core/operators/builtin/PencilSketchOperator.cpp
        src/core/operators/builtin/PolarTransformOperator.h
        src/core/operators/builtin/PolarTransformOperator.cpp
        src/core/operators/builtin/PerspectiveTransformOperator.h
        src/core/operators/builtin/PerspectiveTransformOperator.cpp
        src/core/operators/builtin/ResizeOperator.h
        src/core/operators/builtin/ResizeOperator.cpp
        src/core/operators/builtin/ScharrOperator.h
        src/core/operators/builtin/ScharrOperator.cpp
        src/core/operators/builtin/SiftKeypointsOperator.h
        src/core/operators/builtin/SiftKeypointsOperator.cpp
        src/core/operators/builtin/SobelOperator.h
        src/core/operators/builtin/SobelOperator.cpp
        src/core/operators/builtin/SpecialEffectOperator.h
        src/core/operators/builtin/SpecialEffectOperator.cpp
        src/core/operators/builtin/TriangleThresholdOperator.h
        src/core/operators/builtin/TriangleThresholdOperator.cpp
        src/core/operators/builtin/WatershedSegmentationOperator.h
        src/core/operators/builtin/WatershedSegmentationOperator.cpp
        src/core/pipeline/IPipelineStep.h
        src/core/pipeline/PipelineDefinition.h
        src/core/pipeline/PipelineEngine.h
        src/core/pipeline/PipelineEngine.cpp
        src/core/pipeline/PipelineJsonSerializer.h
        src/core/pipeline/PipelineJsonSerializer.cpp
        src/infra/opencv/OpenCvImageIO.h
        src/infra/opencv/OpenCvImageIO.cpp
        src/infra/opencv/OpenCvQtImageConverter.h
        src/infra/opencv/OpenCvQtImageConverter.cpp
        src/ui/mainwindow/MainWindow.h
        src/ui/mainwindow/MainWindow.cpp
        src/ui/mainwindow/MainWindow.ui
        src/ui/panels/ExportResultsPanel.h
        src/ui/panels/ExportResultsPanel.cpp
        src/ui/panels/ExportResultsPanel.ui
        src/ui/panels/MediaPanel.h
        src/ui/panels/MediaPanel.cpp
        src/ui/panels/PipelinePanel.h
        src/ui/panels/PipelinePanel.cpp
        src/ui/panels/ParameterPanel.h
        src/ui/panels/ParameterPanel.cpp
        src/ui/widgets/CanvasView.h
        src/ui/widgets/CanvasView.cpp
    )
endif()

target_link_libraries(CVVerify PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    ${OpenCV_LIBS}
)

target_sources(CVVerify PRIVATE
    resources/app_resources.qrc
)

if(WIN32)
    target_sources(CVVerify PRIVATE
        resources/windows/app_icon.rc
    )
endif()

target_include_directories(CVVerify PRIVATE
    src
)

include(CTest)

if(CVVERIFY_BUILD_TESTS)
    enable_testing()

    add_executable(CVVerifyCoreTests
        tests/unit/core/test_main.cpp
        tests/unit/core/test_core_pipeline.cpp
        tests/unit/core/test_media_sources.cpp
        tests/unit/core/test_model_package_loader.cpp
        tests/unit/core/test_operator_registry.cpp
        tests/unit/core/test_pipeline_serialization.cpp
        tests/unit/core/test_detection_render_composer.cpp
        tests/unit/core/test_detection_export_service.cpp
        tests/unit/core/test_yolo_decoder.cpp
        tests/unit/core/test_yolo_detection_step.cpp
        tests/unit/ui/test_pipeline_panel.cpp
        tests/unit/ui/test_parameter_panel.cpp
        tests/unit/ui/test_media_panel.cpp
        tests/unit/ui/test_canvas_view.cpp
        tests/unit/ui/test_export_results_panel.cpp
        tests/unit/ui/test_mainwindow_menus.cpp
        tests/unit/ui/test_system_settings_dialog.cpp
        tests/unit/ui/test_yolo_model_import_dialog.cpp
        src/app/AppSettings.cpp
        src/core/detection/ModelPackageLoader.cpp
        src/core/detection/LabelProvider.cpp
        src/core/detection/OpenCvDnnRunner.cpp
        src/core/detection/YoloOnnxImportService.cpp
        src/core/detection/DetectionRenderComposer.cpp
        src/core/detection/DetectionExportService.cpp
        src/core/detection/YoloDetectionDecoder.cpp
        src/core/detection/YoloDetectionStep.cpp
        src/ui/dialogs/YoloModelImportDialog.h
        src/ui/dialogs/YoloModelImportDialog.cpp
        src/ui/dialogs/YoloModelImportDialog.ui
        src/ui/dialogs/SystemSettingsDialog.h
        src/ui/dialogs/SystemSettingsDialog.cpp
        src/ui/dialogs/SystemSettingsDialog.ui
        src/ui/dialogs/OperatorParameterDialog.h
        src/ui/dialogs/OperatorParameterDialog.cpp
        src/ui/dialogs/OperatorParameterDialog.ui
        src/ui/mainwindow/MainWindow.cpp
        src/ui/mainwindow/MainWindow.h
        src/ui/mainwindow/MainWindow.ui
        src/ui/panels/ExportResultsPanel.cpp
        src/ui/panels/ExportResultsPanel.h
        src/ui/panels/ExportResultsPanel.ui
        src/ui/panels/MediaPanel.cpp
        src/core/media/ImageFileSource.cpp
        src/core/media/VideoFileSource.cpp
        src/core/operators/OperatorRegistry.cpp
        src/core/operators/BuiltinOperatorBootstrap.cpp
        src/core/operators/builtin/AdaptiveThresholdOperator.cpp
        src/core/operators/builtin/AffineTransformOperator.cpp
        src/core/operators/builtin/BilateralFilterOperator.cpp
        src/core/operators/builtin/BoxFilterOperator.cpp
        src/core/operators/builtin/BrightnessContrastOperator.cpp
        src/core/operators/builtin/ColorConvertOperator.cpp
        src/core/operators/builtin/ColorMapOperator.cpp
        src/core/operators/builtin/ConnectedComponentsOperator.cpp
        src/core/operators/builtin/ContourDetectOperator.cpp
        src/core/operators/builtin/CornerDetectOperator.cpp
        src/core/operators/builtin/EmbossOperator.cpp
        src/core/operators/builtin/FiberGlowOperator.cpp
        src/core/operators/builtin/FloodFillOperator.cpp
        src/core/operators/builtin/GrabCutOperator.cpp
        src/core/operators/builtin/HistogramBarOperator.cpp
        src/core/operators/builtin/GaussianBlurOperator.cpp
        src/core/operators/builtin/GrayscaleOperator.cpp
        src/core/operators/builtin/HistogramEqualizeOperator.cpp
        src/core/operators/builtin/HistogramLineOperator.cpp
        src/core/operators/builtin/HoughCirclesOperator.cpp
        src/core/operators/builtin/HoughLinesOperator.cpp
        src/core/operators/builtin/KMeansSegmentationOperator.cpp
        src/core/operators/builtin/LaplacianOperator.cpp
        src/core/operators/builtin/LinearAbsTransformOperator.cpp
        src/core/operators/builtin/MedianBlurOperator.cpp
        src/core/operators/builtin/MeanBlurOperator.cpp
        src/core/operators/builtin/MeanShiftSegmentationOperator.cpp
        src/core/operators/builtin/SurfKeypointsPlaceholderOperator.cpp
        src/core/operators/builtin/SurfMatchPlaceholderOperator.cpp
        src/core/operators/builtin/ThresholdOperator.cpp
        src/core/operators/builtin/CannyOperator.cpp
        src/core/operators/builtin/MorphologyOperator.cpp
        src/core/operators/builtin/OtsuThresholdOperator.cpp
        src/core/operators/builtin/ArtEffectOperator.cpp
        src/core/operators/builtin/PencilSketchOperator.cpp
        src/core/operators/builtin/PolarTransformOperator.cpp
        src/core/operators/builtin/PerspectiveTransformOperator.cpp
        src/core/operators/builtin/ResizeOperator.cpp
        src/core/operators/builtin/ScharrOperator.cpp
        src/core/operators/builtin/SiftKeypointsOperator.cpp
        src/core/operators/builtin/SobelOperator.cpp
        src/core/operators/builtin/SpecialEffectOperator.cpp
        src/core/operators/builtin/TriangleThresholdOperator.cpp
        src/core/operators/builtin/WatershedSegmentationOperator.cpp
        src/core/pipeline/PipelineJsonSerializer.cpp
        src/core/pipeline/PipelineEngine.cpp
        src/infra/opencv/OpenCvImageIO.cpp
        src/infra/opencv/OpenCvQtImageConverter.cpp
        src/ui/panels/PipelinePanel.cpp
        src/ui/panels/ParameterPanel.cpp
        src/ui/widgets/CanvasView.cpp
    )

    target_include_directories(CVVerifyCoreTests PRIVATE
        src
    )

    target_link_libraries(CVVerifyCoreTests PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        ${OpenCV_LIBS}
    )

    get_filename_component(CVVERIFY_TOOLCHAIN_RUNTIME_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(CVVERIFY_TEST_RUNTIME_PATHS "")
    if(CVVERIFY_OPENCV_RUNTIME_BIN)
        list(APPEND CVVERIFY_TEST_RUNTIME_PATHS "${CVVERIFY_OPENCV_RUNTIME_BIN}")
    endif()
    if(CVVERIFY_QT_RUNTIME_BIN)
        list(APPEND CVVERIFY_TEST_RUNTIME_PATHS "${CVVERIFY_QT_RUNTIME_BIN}")
    endif()
    if(CVVERIFY_TOOLCHAIN_RUNTIME_BIN)
        list(APPEND CVVERIFY_TEST_RUNTIME_PATHS "${CVVERIFY_TOOLCHAIN_RUNTIME_BIN}")
    endif()

    if(CVVERIFY_TEST_RUNTIME_PATHS)
        list(REMOVE_DUPLICATES CVVERIFY_TEST_RUNTIME_PATHS)
        string(JOIN ";" CVVERIFY_TEST_RUNTIME_PATH ${CVVERIFY_TEST_RUNTIME_PATHS})
        add_test(
            NAME CVVerifyCoreTests
            COMMAND ${CMAKE_COMMAND} -E env "PATH=${CVVERIFY_TEST_RUNTIME_PATH};$ENV{PATH}" $<TARGET_FILE:CVVerifyCoreTests>
        )
    else()
        add_test(NAME CVVerifyCoreTests COMMAND $<TARGET_FILE:CVVerifyCoreTests>)
    endif()
endif()
