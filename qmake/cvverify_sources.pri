CVVERIFY_ROOT = $$clean_path($$PWD/..)

CVVERIFY_APP_COMMON_SOURCES = \
    $$CVVERIFY_ROOT/src/app/AppSettings.cpp \
    $$CVVERIFY_ROOT/src/core/detection/DetectionExportService.cpp \
    $$CVVERIFY_ROOT/src/core/detection/DetectionRenderComposer.cpp \
    $$CVVERIFY_ROOT/src/core/detection/LabelProvider.cpp \
    $$CVVERIFY_ROOT/src/core/detection/ModelPackageLoader.cpp \
    $$CVVERIFY_ROOT/src/core/detection/OpenCvDnnRunner.cpp \
    $$CVVERIFY_ROOT/src/core/detection/YoloDetectionDecoder.cpp \
    $$CVVERIFY_ROOT/src/core/detection/YoloDetectionStep.cpp \
    $$CVVERIFY_ROOT/src/core/detection/YoloOnnxImportService.cpp \
    $$CVVERIFY_ROOT/src/core/media/ImageFileSource.cpp \
    $$CVVERIFY_ROOT/src/core/media/VideoFileSource.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/AdaptiveThresholdOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/AffineTransformOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ArtEffectOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/BilateralFilterOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/BoxFilterOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/BrightnessContrastOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/CannyOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ColorConvertOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ColorMapOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ConnectedComponentsOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ContourDetectOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/CornerDetectOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/EmbossOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/FiberGlowOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/FloodFillOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/GaussianBlurOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/GrabCutOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/GrayscaleOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/HistogramBarOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/HistogramEqualizeOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/HistogramLineOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/HoughCirclesOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/HoughLinesOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/KMeansSegmentationOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/LaplacianOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/LinearAbsTransformOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/MeanBlurOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/MeanShiftSegmentationOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/MedianBlurOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/MorphologyOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/OtsuThresholdOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/PencilSketchOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/PolarTransformOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/PerspectiveTransformOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ResizeOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ScharrOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/SiftKeypointsOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/SobelOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/SpecialEffectOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/SurfKeypointsPlaceholderOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/SurfMatchPlaceholderOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ThresholdOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/TriangleThresholdOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/builtin/WatershedSegmentationOperator.cpp \
    $$CVVERIFY_ROOT/src/core/operators/BuiltinOperatorBootstrap.cpp \
    $$CVVERIFY_ROOT/src/core/operators/OperatorRegistry.cpp \
    $$CVVERIFY_ROOT/src/core/pipeline/PipelineEngine.cpp \
    $$CVVERIFY_ROOT/src/core/pipeline/PipelineJsonSerializer.cpp \
    $$CVVERIFY_ROOT/src/infra/opencv/OpenCvImageIO.cpp \
    $$CVVERIFY_ROOT/src/infra/opencv/OpenCvQtImageConverter.cpp \
    $$CVVERIFY_ROOT/src/ui/dialogs/OperatorParameterDialog.cpp \
    $$CVVERIFY_ROOT/src/ui/dialogs/SystemSettingsDialog.cpp \
    $$CVVERIFY_ROOT/src/ui/dialogs/YoloModelImportDialog.cpp \
    $$CVVERIFY_ROOT/src/ui/mainwindow/MainWindow.cpp \
    $$CVVERIFY_ROOT/src/ui/panels/ExportResultsPanel.cpp \
    $$CVVERIFY_ROOT/src/ui/panels/MediaPanel.cpp \
    $$CVVERIFY_ROOT/src/ui/panels/ParameterPanel.cpp \
    $$CVVERIFY_ROOT/src/ui/panels/PipelinePanel.cpp \
    $$CVVERIFY_ROOT/src/ui/widgets/CanvasView.cpp

CVVERIFY_APP_ENTRY_SOURCE = \
    $$CVVERIFY_ROOT/src/main.cpp

CVVERIFY_APP_SOURCES = \
    $$CVVERIFY_APP_COMMON_SOURCES \
    $$CVVERIFY_APP_ENTRY_SOURCE

CVVERIFY_APP_HEADERS = \
    $$CVVERIFY_ROOT/src/app/AppSettings.h \
    $$CVVERIFY_ROOT/src/core/detection/DetectionExportService.h \
    $$CVVERIFY_ROOT/src/core/detection/DetectionModelDescriptor.h \
    $$CVVERIFY_ROOT/src/core/detection/DetectionRenderComposer.h \
    $$CVVERIFY_ROOT/src/core/detection/DetectionTypes.h \
    $$CVVERIFY_ROOT/src/core/detection/LabelProvider.h \
    $$CVVERIFY_ROOT/src/core/detection/ModelPackageLoader.h \
    $$CVVERIFY_ROOT/src/core/detection/OpenCvDnnRunner.h \
    $$CVVERIFY_ROOT/src/core/detection/YoloDetectionDecoder.h \
    $$CVVERIFY_ROOT/src/core/detection/YoloDetectionStep.h \
    $$CVVERIFY_ROOT/src/core/detection/YoloOnnxImportService.h \
    $$CVVERIFY_ROOT/src/core/frame/FramePacket.h \
    $$CVVERIFY_ROOT/src/core/media/ImageFileSource.h \
    $$CVVERIFY_ROOT/src/core/media/IMediaSource.h \
    $$CVVERIFY_ROOT/src/core/media/MediaTypes.h \
    $$CVVERIFY_ROOT/src/core/media/VideoFileSource.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/AdaptiveThresholdOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/AffineTransformOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ArtEffectOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/BilateralFilterOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/BoxFilterOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/BrightnessContrastOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/CannyOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ColorConvertOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ColorMapOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ConnectedComponentsOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ContourDetectOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/CornerDetectOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/EmbossOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/FiberGlowOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/FloodFillOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/GaussianBlurOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/GrabCutOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/GrayscaleOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/HistogramBarOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/HistogramEqualizeOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/HistogramLineOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/HoughCirclesOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/HoughLinesOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/KMeansSegmentationOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/LaplacianOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/LinearAbsTransformOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/MeanBlurOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/MeanShiftSegmentationOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/MedianBlurOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/MorphologyOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/OtsuThresholdOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/PencilSketchOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/PolarTransformOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/PerspectiveTransformOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ResizeOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ScharrOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/SiftKeypointsOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/SobelOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/SpecialEffectOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/SurfKeypointsPlaceholderOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/SurfMatchPlaceholderOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/ThresholdOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/TriangleThresholdOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/builtin/WatershedSegmentationOperator.h \
    $$CVVERIFY_ROOT/src/core/operators/BuiltinOperatorBootstrap.h \
    $$CVVERIFY_ROOT/src/core/operators/OperatorDescriptor.h \
    $$CVVERIFY_ROOT/src/core/operators/OperatorRegistry.h \
    $$CVVERIFY_ROOT/src/core/pipeline/IPipelineStep.h \
    $$CVVERIFY_ROOT/src/core/pipeline/PipelineDefinition.h \
    $$CVVERIFY_ROOT/src/core/pipeline/PipelineEngine.h \
    $$CVVERIFY_ROOT/src/core/pipeline/PipelineJsonSerializer.h \
    $$CVVERIFY_ROOT/src/infra/opencv/OpenCvImageIO.h \
    $$CVVERIFY_ROOT/src/infra/opencv/OpenCvQtImageConverter.h \
    $$CVVERIFY_ROOT/src/ui/dialogs/OperatorParameterDialog.h \
    $$CVVERIFY_ROOT/src/ui/dialogs/SystemSettingsDialog.h \
    $$CVVERIFY_ROOT/src/ui/dialogs/YoloModelImportDialog.h \
    $$CVVERIFY_ROOT/src/ui/mainwindow/MainWindow.h \
    $$CVVERIFY_ROOT/src/ui/panels/ExportResultsPanel.h \
    $$CVVERIFY_ROOT/src/ui/panels/MediaPanel.h \
    $$CVVERIFY_ROOT/src/ui/panels/ParameterPanel.h \
    $$CVVERIFY_ROOT/src/ui/panels/PipelinePanel.h \
    $$CVVERIFY_ROOT/src/ui/widgets/CanvasView.h

CVVERIFY_APP_FORMS = \
    $$CVVERIFY_ROOT/src/ui/dialogs/OperatorParameterDialog.ui \
    $$CVVERIFY_ROOT/src/ui/dialogs/SystemSettingsDialog.ui \
    $$CVVERIFY_ROOT/src/ui/dialogs/YoloModelImportDialog.ui \
    $$CVVERIFY_ROOT/src/ui/mainwindow/MainWindow.ui \
    $$CVVERIFY_ROOT/src/ui/panels/ExportResultsPanel.ui

CVVERIFY_TEST_SOURCES = \
    $$CVVERIFY_ROOT/tests/unit/core/test_core_pipeline.cpp \
    $$CVVERIFY_ROOT/tests/unit/core/test_detection_export_service.cpp \
    $$CVVERIFY_ROOT/tests/unit/core/test_detection_render_composer.cpp \
    $$CVVERIFY_ROOT/tests/unit/core/test_main.cpp \
    $$CVVERIFY_ROOT/tests/unit/core/test_media_sources.cpp \
    $$CVVERIFY_ROOT/tests/unit/core/test_model_package_loader.cpp \
    $$CVVERIFY_ROOT/tests/unit/core/test_operator_registry.cpp \
    $$CVVERIFY_ROOT/tests/unit/core/test_pipeline_serialization.cpp \
    $$CVVERIFY_ROOT/tests/unit/core/test_yolo_decoder.cpp \
    $$CVVERIFY_ROOT/tests/unit/core/test_yolo_detection_step.cpp \
    $$CVVERIFY_ROOT/tests/unit/ui/test_canvas_view.cpp \
    $$CVVERIFY_ROOT/tests/unit/ui/test_export_results_panel.cpp \
    $$CVVERIFY_ROOT/tests/unit/ui/test_mainwindow_menus.cpp \
    $$CVVERIFY_ROOT/tests/unit/ui/test_media_panel.cpp \
    $$CVVERIFY_ROOT/tests/unit/ui/test_parameter_panel.cpp \
    $$CVVERIFY_ROOT/tests/unit/ui/test_pipeline_panel.cpp \
    $$CVVERIFY_ROOT/tests/unit/ui/test_system_settings_dialog.cpp \
    $$CVVERIFY_ROOT/tests/unit/ui/test_yolo_model_import_dialog.cpp

CVVERIFY_TEST_HEADERS =
