#include "ui/mainwindow/MainWindow.h"

#include <QColor>
#include <QDir>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QTextBrowser>
#include <QDockWidget>
#include <QInputDialog>
#include <QKeySequence>
#include <QShortcut>
#include <QTabWidget>
#include <QTabBar>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QAbstractItemView>
#include <QElapsedTimer>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QListView>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QShowEvent>
#include <QSizePolicy>
#include <QStatusBar>
#include <QThread>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <functional>

#include "app/ApplicationContext.h"
#include "app/AppSettings.h"
#include "app/AppVersion.h"
#include "core/detection/DetectionExportService.h"
#include "core/detection/OpenCvDnnRunner.h"
#include "core/detection/DetectionRenderComposer.h"
#include "core/detection/LabelProvider.h"
#include "core/detection/ModelPackageLoader.h"
#include "core/detection/ModelImportService.h"
#include "core/detection/ModelTemplateLoader.h"
#include "core/detection/OnnxModelStepFactory.h"
#include "core/detection/YoloOnnxImportService.h"
#include "core/diagnostics/DiagnosticsLog.h"
#include "core/diagnostics/DnnRuntimeInfo.h"
#include "core/pipeline/BatchPipelineRunner.h"
#include "core/project/ParameterPresetStore.h"
#include "core/media/ImageFileSource.h"
#include "core/media/ImageFolderSource.h"
#include "core/media/VideoFileSource.h"
#include "core/models/ModelRegistry.h"
#include "core/project/ProjectDefinition.h"
#include "core/project/ProjectService.h"
#include "core/tasks/TaskTypes.h"
#include "core/pipeline/PipelineJsonSerializer.h"
#include "infra/opencv/OpenCvImageIO.h"
#include "infra/opencv/OpenCvQtImageConverter.h"
#include "infra/platform/PlatformPaths.h"
#include "ui/dialogs/ModelConfigEditorDialog.h"
#include "ui/dialogs/SystemSettingsDialog.h"
#include "ui/dialogs/OperatorParameterDialog.h"
#include "ui/dialogs/YoloModelImportDialog.h"
#include "ui/panels/ExportResultsPanel.h"
#include "ui/panels/DiagnosticsPanel.h"
#include "ui/panels/ModelListPanel.h"
#include "ui/panels/StatisticsPanel.h"
#include "ui/widgets/PreviewCanvasHost.h"
#include "ui/widgets/TimelineWidget.h"
#include "ui/panels/MediaPanel.h"
#include "ui/panels/ParameterPanel.h"
#include "ui/panels/PipelinePanel.h"
#include "ui/panels/TaskConsolePanel.h"
#include "ui/panels/TaskHistoryPanel.h"
#include "ui/widgets/CanvasView.h"
#include "ui_MainWindow.h"

namespace {

struct OperatorCategoryDefinition
{
    const char* key;
    QStringList operatorIds;
};

bool isChineseLanguage(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

QString localizedText(const QString& languageCode, const QString& englishText, const QString& chineseText)
{
    return isChineseLanguage(languageCode) ? chineseText : englishText;
}

QString quickStartGuideText(const QString& languageCode)
{
    if (isChineseLanguage(languageCode)) {
        return QStringLiteral(
            "CVVerify 快速入门\n"
            "================\n\n"
            "1. 打开媒体\n"
            "   使用「媒体 → 打开图片 / 打开视频 / 打开图片文件夹」，"
            "或把文件拖拽到主窗口。\n\n"
            "2. 搭建处理流程\n"
            "   在「流程 → 添加算子」中选择算子；"
            "底部参数区可调整当前步骤参数；"
            "按 F5 或点击预览相关按钮重新运行。\n\n"
            "3. 查看结果\n"
            "   中间区域左侧为原图，右侧为处理结果。"
            "「预览」菜单可切换双画面、拆分对比、四宫格等布局。\n\n"
            "4. 模型与检测\n"
            "   通过「媒体 → 导入 ONNX 模型」加载模型；"
            "可在「流程 → 添加当前模型步骤」加入检测步骤。"
            "需要模型列表时，打开「视图 → 模型与任务面板」。\n\n"
            "5. 播放与导出\n"
            "   视频可使用「播放」菜单或空格键控制播放；"
            "「导出」菜单支持导出检测结果、流程快照等。\n\n"
            "6. 常用快捷键\n"
            "   Ctrl+O  打开图片\n"
            "   F5      重新预览\n"
            "   空格    播放 / 暂停\n"
            "   Ctrl+,  系统设置\n\n"
            "提示：左右侧边面板默认隐藏，可在「视图」菜单中按需显示。");
    }

    return QStringLiteral(
        "CVVerify Quick Start\n"
        "====================\n\n"
        "1. Open media\n"
        "   Use Media -> Open Image / Open Video / Open Image Folder,\n"
        "   or drag files onto the main window.\n\n"
        "2. Build a pipeline\n"
        "   Choose operators from Pipeline -> Add Operator.\n"
        "   Adjust step parameters in the bottom panel.\n"
        "   Press F5 to rerun preview.\n\n"
        "3. Inspect results\n"
        "   The center area shows source on the left and result on the right.\n"
        "   Use Preview menu layouts such as dual view, wipe, or four grid.\n\n"
        "4. Models and detection\n"
        "   Import ONNX models from Media -> Import ONNX Model.\n"
        "   Add the active model step from Pipeline -> Add Active Model Step.\n"
        "   Open View -> Model && Tasks Panel when you need the model list.\n\n"
        "5. Playback and export\n"
        "   Control video playback from the Playback menu or Space key.\n"
        "   Export detections, snapshots, and batch results from the Export menu.\n\n"
        "6. Useful shortcuts\n"
        "   Ctrl+O  Open image\n"
        "   F5      Rerun preview\n"
        "   Space   Play / pause\n"
        "   Ctrl+,  System settings\n\n"
        "Tip: side panels are hidden by default. Show them from the View menu when needed.");
}

QString mediaSourceKindToString(MediaSourceKind kind)
{
    switch (kind) {
    case MediaSourceKind::ImageFile:
        return QStringLiteral("ImageFile");
    case MediaSourceKind::VideoFile:
        return QStringLiteral("VideoFile");
    case MediaSourceKind::ImageFolder:
        return QStringLiteral("ImageFolder");
    }
    return QStringLiteral("ImageFile");
}

MediaSourceKind mediaSourceKindFromString(const QString& value)
{
    if (value == QStringLiteral("VideoFile")) {
        return MediaSourceKind::VideoFile;
    }
    if (value == QStringLiteral("ImageFolder")) {
        return MediaSourceKind::ImageFolder;
    }
    return MediaSourceKind::ImageFile;
}

struct SampleResourceEntry
{
    const char* relativePath;
    const char* englishLabel;
    const char* chineseLabel;
};

const SampleResourceEntry kSamplePipelines[] = {
    {"samples/pipelines/basic_blur.json", "Basic Gaussian Blur", "\u57fa\u7840\u9ad8\u65af\u6a21\u7cca"},
    {"samples/pipelines/edge_canny.json", "Grayscale + Canny", "\u7070\u5ea6 + Canny \u8fb9\u7f18"},
    {"samples/pipelines/morphology_open.json", "Threshold + Morphology Open", "\u9608\u503c + \u5f00\u8fd0\u7b97"},
    {"samples/pipelines/histogram_sidecar.json", "Histogram Sidecar", "\u76f4\u65b9\u56fe Sidecar"},
};

const SampleResourceEntry kSampleProjects[] = {
    {"samples/projects/demo_project.json", "Demo Blur Project", "\u6a21\u7cca\u6f14\u793a\u9879\u76ee"},
    {"samples/projects/edge_canny_project.json", "Edge Canny Project", "Canny \u8fb9\u7f18\u9879\u76ee"},
};

bool isImageFilePath(const QString& path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == "png" || suffix == "jpg" || suffix == "jpeg" || suffix == "bmp"
        || suffix == "tif" || suffix == "tiff" || suffix == "webp";
}

bool isVideoFilePath(const QString& path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == "avi" || suffix == "mp4" || suffix == "mov" || suffix == "mkv"
        || suffix == "wmv" || suffix == "webm";
}

bool isOnnxModelPath(const QString& path)
{
    return QFileInfo(path).suffix().compare(QStringLiteral("onnx"), Qt::CaseInsensitive) == 0;
}

QString localizedParameterNameForSummary(const QString& key, const QString& displayName, const QString& languageCode)
{
    if (!isChineseLanguage(languageCode)) {
        return displayName;
    }

    if (key == "kernelSize") return QStringLiteral("\u6838\u5927\u5c0f");
    if (key == "sigmaX" || key == "sigma") return QStringLiteral("Sigma");
    if (key == "width") return QStringLiteral("\u5bbd\u5ea6");
    if (key == "height") return QStringLiteral("\u9ad8\u5ea6");
    if (key == "threshold") return QStringLiteral("\u9608\u503c");
    if (key == "maxValue") return QStringLiteral("\u6700\u5927\u503c");
    if (key == "type") return QStringLiteral("\u7c7b\u578b");
    if (key == "operation") return QStringLiteral("\u64cd\u4f5c");
    if (key == "kernelWidth") return QStringLiteral("\u6838\u5bbd");
    if (key == "kernelHeight") return QStringLiteral("\u6838\u9ad8");
    if (key == "iterations") return QStringLiteral("\u8fed\u4ee3\u6b21\u6570");
    if (key == "threshold1") return QStringLiteral("\u9608\u503c 1");
    if (key == "threshold2") return QStringLiteral("\u9608\u503c 2");
    if (key == "apertureSize") return QStringLiteral("\u5b54\u5f84\u5927\u5c0f");
    if (key == "useL2Gradient") return QStringLiteral("L2 \u68af\u5ea6");
    if (key == "enabled") return QStringLiteral("\u542f\u7528");
    if (key == "mode") return QStringLiteral("\u6a21\u5f0f");
    if (key == "secondaryImagePath") return QStringLiteral("\u7b2c\u4e8c\u5f20\u56fe\u8def\u5f84");
    if (key == "templateImagePath") return QStringLiteral("\u6a21\u677f\u56fe\u7247\u8def\u5f84");
    if (key == "direction") return QStringLiteral("\u65b9\u5411");
    if (key == "polarMode") return QStringLiteral("\u6781\u5750\u6807\u6a21\u5f0f");
    if (key == "centerMode") return QStringLiteral("\u4e2d\u5fc3\u6a21\u5f0f");
    if (key == "centerX") return QStringLiteral("\u4e2d\u5fc3 X");
    if (key == "centerY") return QStringLiteral("\u4e2d\u5fc3 Y");
    if (key == "radiusMode") return QStringLiteral("\u534a\u5f84\u6a21\u5f0f");
    if (key == "maxRadius") return QStringLiteral("\u6700\u5927\u534a\u5f84");
    if (key == "outputWidth") return QStringLiteral("\u8f93\u51fa\u5bbd\u5ea6");
    if (key == "outputHeight") return QStringLiteral("\u8f93\u51fa\u9ad8\u5ea6");
    if (key == "interpolation") return QStringLiteral("\u63d2\u503c\u65b9\u5f0f");
    if (key == "preset") return QStringLiteral("\u9884\u8bbe\u7c7b\u578b");
    if (key == "strength") return QStringLiteral("\u5f3a\u5ea6");
    if (key == "radius") return QStringLiteral("\u534a\u5f84");
    if (key == "angleDeg") return QStringLiteral("\u65b9\u5411\u89d2\u5ea6");
    if (key == "streakLength") return QStringLiteral("\u62d6\u5f71\u957f\u5ea6");
    if (key == "intensity") return QStringLiteral("\u53d1\u5149\u5f3a\u5ea6");
    if (key == "blendAlpha") return QStringLiteral("\u6df7\u5408\u900f\u660e\u5ea6");
    return displayName;
}

QString localizedChoiceLabelForSummary(
    const QString& parameterKey,
    const QString& choiceValue,
    const QString& choiceLabel,
    const QString& languageCode)
{
    if (!isChineseLanguage(languageCode)) {
        return choiceLabel;
    }

    if (parameterKey == "type") {
        if (choiceValue == "binary") return QStringLiteral("\u4e8c\u503c");
        if (choiceValue == "binary_inv") return QStringLiteral("\u53cd\u5411\u4e8c\u503c");
        if (choiceValue == "trunc") return QStringLiteral("\u622a\u65ad");
        if (choiceValue == "tozero") return QStringLiteral("\u7f6e\u96f6");
        if (choiceValue == "tozero_inv") return QStringLiteral("\u53cd\u5411\u7f6e\u96f6");
        if (choiceValue == "otsu") return QStringLiteral("\u5927\u6d25\u6cd5");
    }

    if (parameterKey == "operation") {
        if (choiceValue == "erode") return QStringLiteral("\u8150\u8680");
        if (choiceValue == "dilate") return QStringLiteral("\u81a8\u80c0");
        if (choiceValue == "open") return QStringLiteral("\u5f00\u8fd0\u7b97");
        if (choiceValue == "close") return QStringLiteral("\u95ed\u8fd0\u7b97");
    }

    if (parameterKey == "mode") {
        if (choiceValue == "binary") return QStringLiteral("\u4e8c\u503c");
        if (choiceValue == "otsu") return QStringLiteral("\u5927\u6d25\u6cd5");
    }

    return choiceLabel;
}

QString localizedOperatorDisplayName(const QString& operatorId, const QString& fallbackName, const QString& languageCode)
{
    if (isChineseLanguage(languageCode)) {
        if (operatorId == "builtin.mean_blur") return QStringLiteral("\u5747\u503c\u6ee4\u6ce2");
        if (operatorId == "builtin.box_filter") return QStringLiteral("\u65b9\u6846\u6ee4\u6ce2");
        if (operatorId == "builtin.bilateral_filter") return QStringLiteral("\u53cc\u8fb9\u6ee4\u6ce2");
        if (operatorId == "builtin.gaussian_blur") return QStringLiteral("\u9ad8\u65af\u6a21\u7cca");
        if (operatorId == "builtin.median_blur") return QStringLiteral("\u4e2d\u503c\u6a21\u7cca");
        if (operatorId == "builtin.brightness_contrast") return QStringLiteral("\u4eae\u5ea6\u4e0e\u5bf9\u6bd4\u5ea6");
        if (operatorId == "builtin.hist_equalize") return QStringLiteral("\u76f4\u65b9\u56fe\u5747\u8861\u5316");
        if (operatorId == "builtin.histogram_line") return QStringLiteral("\u76f4\u65b9\u56fe\uff08\u7ebf\u72b6\uff09");
        if (operatorId == "builtin.histogram_bar") return QStringLiteral("\u76f4\u65b9\u56fe\uff08\u67f1\u72b6\uff09");
        if (operatorId == "builtin.affine_transform") return QStringLiteral("\u4eff\u5c04\u53d8\u6362");
        if (operatorId == "builtin.polar_transform") return QStringLiteral("\u6781\u5750\u6807\u53d8\u6362");
        if (operatorId == "builtin.linear_abs_transform") return QStringLiteral("\u7ebf\u6027\u53d8\u6362");
        if (operatorId == "builtin.perspective_transform") return QStringLiteral("\u900f\u89c6\u53d8\u6362");
        if (operatorId == "builtin.resize") return QStringLiteral("\u7f29\u653e");
        if (operatorId == "builtin.morphology") return QStringLiteral("\u5f62\u6001\u5b66");
        if (operatorId == "builtin.threshold") return QStringLiteral("\u9608\u503c");
        if (operatorId == "builtin.threshold_otsu") return QStringLiteral("Otsu \u4e8c\u503c\u5316");
        if (operatorId == "builtin.threshold_triangle") return QStringLiteral("Triangle \u4e8c\u503c\u5316");
        if (operatorId == "builtin.threshold_adaptive") return QStringLiteral("\u81ea\u9002\u5e94\u4e8c\u503c\u5316");
        if (operatorId == "builtin.canny") return QStringLiteral("Canny \u8fb9\u7f18");
        if (operatorId == "builtin.edge_sobel") return QStringLiteral("Sobel \u8fb9\u7f18");
        if (operatorId == "builtin.edge_laplacian") return QStringLiteral("Laplacian \u8fb9\u7f18");
        if (operatorId == "builtin.edge_scharr") return QStringLiteral("Scharr \u8fb9\u7f18");
        if (operatorId == "builtin.grayscale") return QStringLiteral("\u7070\u5ea6\u5316");
        if (operatorId == "builtin.color_convert") return QStringLiteral("\u989c\u8272\u8f6c\u6362");
        if (operatorId == "builtin.color_map") return QStringLiteral("\u5f69\u8272\u6620\u5c04");
        if (operatorId == "builtin.pencil_sketch") return QStringLiteral("\u624b\u5de5\u753b\u6548\u679c");
        if (operatorId == "builtin.art_effect") return QStringLiteral("\u827a\u672f\u6548\u679c");
        if (operatorId == "builtin.fiber_glow") return QStringLiteral("\u5149\u7ea4\u6548\u679c");
        if (operatorId == "builtin.emboss") return QStringLiteral("\u96d5\u523b\u6548\u679c");
        if (operatorId == "builtin.special_effect") return QStringLiteral("\u7279\u6b8a\u6548\u679c");
        if (operatorId == "builtin.contour_detect") return QStringLiteral("\u8f6e\u5ed3\u68c0\u6d4b");
        if (operatorId == "builtin.connected_components") return QStringLiteral("\u8fde\u901a\u57df\u68c0\u6d4b");
        if (operatorId == "builtin.hough_lines") return QStringLiteral("\u970d\u592b\u76f4\u7ebf\u68c0\u6d4b");
        if (operatorId == "builtin.hough_circles") return QStringLiteral("\u970d\u592b\u5706\u68c0\u6d4b");
        if (operatorId == "builtin.corner_detect") return QStringLiteral("\u89d2\u70b9\u68c0\u6d4b");
        if (operatorId == "builtin.sift_keypoints") return QStringLiteral("SIFT \u7279\u5f81\u70b9\u68c0\u6d4b");
        if (operatorId == "builtin.surf_keypoints") return QStringLiteral("SURF \u7279\u5f81\u70b9\u68c0\u6d4b");
        if (operatorId == "builtin.segment_kmeans") return QStringLiteral("K-\u5747\u503c\u805a\u7c7b\u5206\u5272");
        if (operatorId == "builtin.segment_meanshift") return QStringLiteral("MeanShift \u5747\u503c\u6f02\u79fb\u5206\u5272");
        if (operatorId == "builtin.segment_watershed") return QStringLiteral("\u5206\u6c34\u5cad\u5206\u5272");
        if (operatorId == "builtin.flood_fill") return QStringLiteral("\u6d78\u6c34\u586b\u5145");
        if (operatorId == "builtin.grabcut") return QStringLiteral("GrabCut \u62a0\u56fe");
        if (operatorId == "builtin.surf_match") return QStringLiteral("SURF \u7279\u5f81\u5339\u914d");
        return fallbackName;
    }

    return fallbackName;
}

const QList<OperatorCategoryDefinition>& operatorMenuCategories()
{
    static const QList<OperatorCategoryDefinition> categories = {
        {"morphology", {"builtin.morphology"}},
        {"filtering", {
            "builtin.mean_blur",
            "builtin.box_filter",
            "builtin.gaussian_blur",
            "builtin.median_blur",
            "builtin.bilateral_filter",
        }},
        {"enhancement_transform", {
            "builtin.brightness_contrast",
            "builtin.affine_transform",
            "builtin.polar_transform",
            "builtin.linear_abs_transform",
            "builtin.perspective_transform",
            "builtin.hist_equalize",
            "builtin.histogram_line",
            "builtin.histogram_bar",
            "builtin.resize",
        }},
        {"thresholding", {
            "builtin.threshold",
            "builtin.threshold_otsu",
            "builtin.threshold_triangle",
            "builtin.threshold_adaptive",
        }},
        {"edge_detection", {
            "builtin.edge_sobel",
            "builtin.canny",
            "builtin.edge_laplacian",
            "builtin.edge_scharr",
        }},
        {"filter_effects", {
            "builtin.grayscale",
            "builtin.color_convert",
            "builtin.color_map",
            "builtin.pencil_sketch",
            "builtin.art_effect",
            "builtin.fiber_glow",
            "builtin.emboss",
            "builtin.special_effect",
        }},
        {"image_detection", {
            "builtin.contour_detect",
            "builtin.hough_lines",
            "builtin.hough_circles",
            "builtin.corner_detect",
            "builtin.connected_components",
            "builtin.surf_keypoints",
            "builtin.sift_keypoints",
        }},
        {"segmentation", {
            "builtin.segment_kmeans",
            "builtin.flood_fill",
            "builtin.segment_watershed",
            "builtin.grabcut",
            "builtin.segment_meanshift",
        }},
        {"feature_matching", {
            "builtin.surf_match",
        }},
    };
    return categories;
}

QString localizedOperatorCategoryTitle(const QString& categoryKey, const QString& languageCode)
{
    if (categoryKey == "morphology") {
        return localizedText(languageCode, "Morphology", QStringLiteral("\u5f62\u6001\u5b66\u5904\u7406"));
    }
    if (categoryKey == "filtering") {
        return localizedText(languageCode, "Filtering", QStringLiteral("\u6ee4\u6ce2"));
    }
    if (categoryKey == "enhancement_transform") {
        return localizedText(languageCode, "Enhancement & Transform", QStringLiteral("\u56fe\u50cf\u589e\u5f3a\u4e0e\u53d8\u6362"));
    }
    if (categoryKey == "thresholding") {
        return localizedText(languageCode, "Thresholding", QStringLiteral("\u4e8c\u503c\u5316"));
    }
    if (categoryKey == "edge_detection") {
        return localizedText(languageCode, "Edge Detection", QStringLiteral("\u8fb9\u7f18\u68c0\u6d4b"));
    }
    if (categoryKey == "filter_effects") {
        return localizedText(languageCode, "Filter Effects", QStringLiteral("\u6ee4\u955c\u6548\u679c"));
    }
    if (categoryKey == "image_detection") {
        return localizedText(languageCode, "Image Detection", QStringLiteral("\u56fe\u50cf\u68c0\u6d4b"));
    }
    if (categoryKey == "segmentation") {
        return localizedText(languageCode, "Segmentation", QStringLiteral("\u56fe\u50cf\u5206\u5272"));
    }
    if (categoryKey == "feature_matching") {
        return localizedText(languageCode, "Feature Matching", QStringLiteral("\u7279\u5f81\u5339\u914d"));
    }
    return categoryKey;
}

const OperatorDescriptor* findOperatorDescriptorById(const QList<OperatorDescriptor>& descriptors, const QString& operatorId)
{
    for (const OperatorDescriptor& descriptor : descriptors) {
        if (descriptor.id == operatorId) {
            return &descriptor;
        }
    }
    return nullptr;
}

void localizeOperatorMenuTree(QMenu* menu, const QList<OperatorDescriptor>& descriptors, const QString& languageCode)
{
    if (!menu) {
        return;
    }

    const QString categoryKey = menu->property("operatorCategoryKey").toString();
    if (!categoryKey.isEmpty()) {
        menu->setTitle(localizedOperatorCategoryTitle(categoryKey, languageCode));
    }

    for (QAction* action : menu->actions()) {
        if (!action) {
            continue;
        }

        if (QMenu* childMenu = action->menu()) {
            localizeOperatorMenuTree(childMenu, descriptors, languageCode);
            continue;
        }

        const QString operatorId = action->data().toString();
        if (operatorId.isEmpty()) {
            continue;
        }

        const OperatorDescriptor* descriptor = findOperatorDescriptorById(descriptors, operatorId);
        const QString fallbackName = descriptor ? descriptor->displayName : operatorId;
        action->setText(localizedOperatorDisplayName(operatorId, fallbackName, languageCode));
    }
}

bool populateOperatorCategoryMenu(
    QMenu* categoryMenu,
    const QString& categoryKey,
    const QStringList& operatorIds,
    const QList<OperatorDescriptor>& descriptors,
    const QString& languageCode,
    const std::function<void(const OperatorDescriptor&)>& onTriggered)
{
    if (!categoryMenu) {
        return false;
    }

    categoryMenu->clear();
    categoryMenu->setProperty("operatorCategoryKey", categoryKey);
    categoryMenu->setTitle(localizedOperatorCategoryTitle(categoryKey, languageCode));

    bool hasActions = false;
    for (const QString& operatorId : operatorIds) {
        const OperatorDescriptor* descriptor = findOperatorDescriptorById(descriptors, operatorId);
        if (!descriptor) {
            continue;
        }

        QAction* action = categoryMenu->addAction(localizedOperatorDisplayName(
            descriptor->id,
            descriptor->displayName,
            languageCode
        ));
        QString actionObjectName = descriptor->id;
        actionObjectName.replace('.', '_');
        action->setObjectName(QString("operatorAction_%1").arg(actionObjectName));
        action->setData(descriptor->id);

        const OperatorDescriptor descriptorCopy = *descriptor;
        QObject::connect(action, &QAction::triggered, categoryMenu, [descriptorCopy, onTriggered] {
            onTriggered(descriptorCopy);
        });
        hasActions = true;
    }

    return hasActions;
}

void populateGroupedOperatorMenu(
    QMenu* rootMenu,
    const QList<OperatorDescriptor>& descriptors,
    const QString& languageCode,
    const QString& categoryMenuObjectPrefix,
    const std::function<void(const OperatorDescriptor&)>& onTriggered)
{
    if (!rootMenu) {
        return;
    }

    rootMenu->clear();
    for (const OperatorCategoryDefinition& category : operatorMenuCategories()) {
        const QString categoryKey = QString::fromLatin1(category.key);
        auto* categoryMenu = new QMenu(rootMenu);
        categoryMenu->setObjectName(QString("%1_%2").arg(categoryMenuObjectPrefix, categoryKey));
        if (!populateOperatorCategoryMenu(
                categoryMenu,
                categoryKey,
                category.operatorIds,
                descriptors,
                languageCode,
                onTriggered)) {
            delete categoryMenu;
            continue;
        }

        rootMenu->addMenu(categoryMenu);
    }
}

bool copyImportedFile(const QString& sourcePath, const QString& targetPath, QString* errorMessage)
{
    const QString absoluteSourcePath = QFileInfo(sourcePath).absoluteFilePath();
    const QString absoluteTargetPath = QFileInfo(targetPath).absoluteFilePath();
    if (absoluteSourcePath == absoluteTargetPath) {
        return true;
    }

    if (QFile::exists(targetPath) && !QFile::remove(targetPath)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to replace existing file: %1").arg(targetPath);
        }
        return false;
    }

    if (!QFile::copy(sourcePath, targetPath)) {
        if (errorMessage) {
            *errorMessage = QString("Failed to copy file to package: %1").arg(targetPath);
        }
        return false;
    }

    return true;
}

DetectionFrameResult detectionResultFromAnnotations(const QVariantMap& annotations)
{
    DetectionFrameResult result;
    const QVariantList boxes = annotations.value("detection_boxes").toList();
    for (const QVariant& boxValue : boxes) {
        const QVariantMap map = boxValue.toMap();

        DetectionBox box;
        box.classId = map.contains("class_id") ? map.value("class_id").toInt() : -1;
        box.label = map.value("label").toString();
        box.score = map.value("score").toFloat();
        box.box = QRectF(
            map.value("x").toDouble(),
            map.value("y").toDouble(),
            map.value("width").toDouble(),
            map.value("height").toDouble()
        );
        result.boxes.append(box);
    }

    return result;
}

cv::Mat applyPreviewScale(const cv::Mat& source, double scale)
{
    if (source.empty() || scale >= 0.99) {
        return source;
    }

    cv::Mat scaled;
    cv::resize(source, scaled, cv::Size(), scale, scale, cv::INTER_AREA);
    return scaled;
}

QString modelResultSummary(const FramePacket& frame, const DetectionFrameResult& detectionResult)
{
    if (!detectionResult.boxes.isEmpty()) {
        return QString("Detections: %1").arg(detectionResult.boxes.size());
    }

    const QVariantList topPredictions = frame.artifacts.value("classification_top_k").toList();
    if (!topPredictions.isEmpty()) {
        const QVariantMap first = topPredictions.first().toMap();
        return QString("Top-1: %1 (%2)")
            .arg(first.value("label").toString())
            .arg(first.value("score").toDouble(), 0, 'f', 3);
    }

    if (frame.artifacts.value("segmentation_applied").toBool()) {
        return QString("Segmentation classes: %1")
            .arg(frame.metrics.value("segmentation_class_count").toInt());
    }

    if (frame.artifacts.contains("ocr_text")) {
        return QString("OCR: %1").arg(frame.artifacts.value("ocr_text").toString());
    }

    if (frame.artifacts.contains("keypoints")) {
        return QString("Keypoints: %1").arg(frame.artifacts.value("keypoints").toList().size());
    }

    if (frame.artifacts.contains("custom_tensor_outputs")) {
        return QString("Tensor outputs: %1")
            .arg(frame.artifacts.value("custom_tensor_outputs").toList().size());
    }

    return QString();
}

QStringList collectSupportedImageFiles(const QString& directoryPath)
{
    QDir directory(directoryPath);
    const QFileInfoList imageFiles = directory.entryInfoList(
        QStringList() << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp",
        QDir::Files | QDir::Readable,
        QDir::Name
    );

    QStringList imagePaths;
    for (const QFileInfo& fileInfo : imageFiles) {
        imagePaths.append(fileInfo.absoluteFilePath());
    }
    return imagePaths;
}

QString exportFileNameFromSource(const QString& sourceId, const QString& suffix)
{
    const QFileInfo fileInfo(sourceId);
    const QString stem = fileInfo.completeBaseName().isEmpty() ? QString("result") : fileInfo.completeBaseName();
    return stem + suffix;
}

QString serializePipelineDefinition(const PipelineDefinition& definition)
{
    QJsonObject root;
    root.insert("pipelineId", definition.pipelineId);
    root.insert("displayName", definition.displayName);

    QJsonArray stepsArray;
    for (const PipelineStepDefinition& step : definition.steps) {
        QJsonObject stepObject;
        stepObject.insert("stepId", step.stepId);
        stepObject.insert("displayName", step.displayName);
        stepObject.insert("parameters", QJsonObject::fromVariantMap(step.parameters));
        stepsArray.append(stepObject);
    }
    root.insert("steps", stepsArray);

    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

QString serializeDetectionModelDescriptor(const DetectionModelDescriptor& descriptor)
{
    QJsonObject root;
    root.insert("model_name", descriptor.modelName);
    root.insert("task_type", descriptor.taskType);
    root.insert("framework_family", descriptor.frameworkFamily);
    root.insert("model_format", descriptor.modelFormat);
    root.insert("labels_path", descriptor.labelsPath);
    root.insert("input", QJsonObject{
        {"width", descriptor.input.width},
        {"height", descriptor.input.height},
        {"channels", descriptor.input.channels},
        {"color_order", descriptor.input.colorOrder},
        {"letterbox", descriptor.input.letterbox},
        {"keep_ratio", descriptor.input.keepRatio},
        {"scale", descriptor.input.scale}
    });
    root.insert("postprocess", QJsonObject{
        {"confidence_threshold", descriptor.postprocess.confidenceThreshold},
        {"nms_iou_threshold", descriptor.postprocess.nmsIouThreshold},
        {"multi_label", descriptor.postprocess.multiLabel},
        {"max_detections", descriptor.postprocess.maxDetections}
    });
    root.insert("output", QJsonObject{
        {"layout_type", descriptor.output.layoutType}
    });

    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

DetectionExportService::DetectionExportContext buildExportContext(
    const PipelineDefinition& pipelineDefinition,
    const DetectionModelDescriptor& descriptor)
{
    DetectionExportService::DetectionExportContext context;
    context.pipelineSnapshotJson = serializePipelineDefinition(pipelineDefinition);
    if (!descriptor.modelName.isEmpty()) {
        context.modelConfigJson = serializeDetectionModelDescriptor(descriptor);
    }
    return context;
}

void configureWorkbenchTabWidget(QTabWidget* tabs)
{
    if (!tabs) {
        return;
    }

    tabs->setDocumentMode(false);
    tabs->setUsesScrollButtons(true);
    tabs->tabBar()->setExpanding(true);
    tabs->tabBar()->setMinimumHeight(42);
    tabs->tabBar()->setDrawBase(false);
}

QWidget* wrapWorkbenchDockWidget(QTabWidget* tabs)
{
    configureWorkbenchTabWidget(tabs);

    auto* host = new QWidget();
    auto* layout = new QVBoxLayout(host);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(0);
    layout->addWidget(tabs);
    return host;
}

void suppressDockTitleBar(QDockWidget* dock)
{
    if (!dock) {
        return;
    }

    auto* placeholder = new QWidget(dock);
    dock->setTitleBarWidget(placeholder);
}

QTabWidget* findWorkbenchTabs(QDockWidget* dock)
{
    if (!dock || !dock->widget()) {
        return nullptr;
    }

    if (auto* tabs = qobject_cast<QTabWidget*>(dock->widget())) {
        return tabs;
    }

    return dock->widget()->findChild<QTabWidget*>();
}

}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1920, 1080);
    m_appSettings = AppSettingsStore::load();
    syncPreviewSettingsFromAppSettings();

    ui->verticalLayout->setContentsMargins(10, 10, 10, 10);
    ui->verticalLayout->setSpacing(8);
    ui->previewLayout->setSpacing(10);
    ui->previewLayout->setStretch(0, 1);
    ui->previewLayout->setStretch(1, 1);
    ui->sourcePanel->setFrameShape(QFrame::StyledPanel);
    ui->resultPanel->setFrameShape(QFrame::StyledPanel);
    ui->statusPanel->setFrameShape(QFrame::StyledPanel);
    ui->statusPanel->setMinimumHeight(200);
    ui->statusPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    setStyleSheet(R"qss(
        QMainWindow,
        QWidget#centralwidget {
            background: #26282c;
            color: #e7e9ed;
        }

        QDialog {
            background: #2d3036;
            color: #e7e9ed;
        }

        QMenuBar {
            background: #1f2125;
            color: #dfe2e7;
            border-bottom: 1px solid #393c42;
            padding: 2px 6px;
        }

        QMenuBar::item {
            background: transparent;
            padding: 5px 9px;
            border-radius: 4px;
        }

        QMenuBar::item:selected {
            background: #353942;
        }

        QMenu {
            background: #2f3238;
            color: #edf0f5;
            border: 1px solid #4a4f58;
            padding: 6px;
        }

        QMenu::item {
            min-width: 160px;
            padding: 7px 26px 7px 12px;
            border-radius: 4px;
        }

        QMenu::item:selected {
            background: #3774d6;
            color: #ffffff;
        }

        QMenu::separator {
            height: 1px;
            background: #464a52;
            margin: 5px 8px;
        }

        QFrame#sourcePanel,
        QFrame#resultPanel,
        QFrame#statusPanel,
        QFrame#operatorWorkbenchPanel {
            background: #303238;
            border: 1px solid #474b54;
            border-radius: 10px;
        }

        QFrame#operatorWorkbenchPanel {
            background: #2d3036;
            border-top: 1px solid #555b66;
        }

        QLabel {
            color: #e9ebef;
        }

        QLabel:disabled {
            color: #777c86;
        }

        QLabel#previewDetailsLabel {
            color: #b8bec9;
            background: #272a30;
            border: 1px solid #3f444d;
            border-radius: 6px;
            padding: 6px 8px;
        }

        QFrame#statusPanel QPushButton {
            min-height: 28px;
            padding: 4px 12px;
        }

        QSlider#timelineSlider {
            min-height: 24px;
        }

        QProgressBar#taskConsoleProgressBar {
            min-height: 22px;
            max-height: 22px;
        }

        QWidget#parameterFormHost {
            background: transparent;
        }

        QScrollArea#parameterScrollArea {
            background: transparent;
            border: 0;
        }

        QScrollArea#parameterScrollArea::viewport {
            background: transparent;
        }

        QScrollArea#parameterScrollArea > QWidget > QWidget {
            background: transparent;
        }

        QLabel[parameterRole="rowLabel"] {
            color: #bfc5cf;
            font-size: 12px;
            padding: 2px 0;
        }

        QLabel[parameterRole="groupLabel"] {
            color: #f0f3f7;
            font-weight: 700;
            padding: 8px 0 2px 0;
        }

        QFrame[parameterRole="divider"] {
            border: none;
            border-top: 1px solid #4b505a;
            background: transparent;
            min-height: 1px;
            max-height: 1px;
        }

        QPushButton,
        QToolButton {
            min-height: 30px;
            padding: 4px 12px;
            color: #e9edf4;
            background: #3a3e46;
            border: 1px solid #555b66;
            border-radius: 6px;
        }

        QPushButton:hover,
        QToolButton:hover {
            background: #454b55;
            border-color: #6b7480;
        }

        QPushButton:pressed,
        QToolButton:pressed {
            background: #2a67bf;
            border-color: #3f86ed;
        }

        QPushButton:checked,
        QToolButton:checked {
            background: #1f5fab;
            border-color: #57a2ff;
            color: #ffffff;
        }

        QPushButton:disabled,
        QToolButton:disabled {
            color: #7b8089;
            background: #30333a;
            border-color: #42464f;
        }

        QToolButton::menu-indicator {
            subcontrol-origin: padding;
            subcontrol-position: center right;
            width: 16px;
            border-left: 1px solid #50565f;
        }

        QListWidget,
        QLineEdit,
        QSpinBox,
        QDoubleSpinBox,
        QComboBox {
            color: #f0f2f6;
            background: #25282e;
            border: 1px solid #4d535e;
            border-radius: 5px;
            selection-background-color: #2f73d6;
            selection-color: #ffffff;
        }

        QLineEdit,
        QSpinBox,
        QDoubleSpinBox,
        QComboBox {
            min-height: 26px;
            padding: 2px 7px;
        }

        QLineEdit:focus,
        QSpinBox:focus,
        QDoubleSpinBox:focus,
        QComboBox:focus {
            border: 1px solid #5aa2ff;
            background: #20242a;
        }

        QLineEdit:disabled,
        QSpinBox:disabled,
        QDoubleSpinBox:disabled,
        QComboBox:disabled {
            color: #757b85;
            background: #30333a;
            border-color: #42464f;
        }

        QComboBox::drop-down {
            width: 24px;
            border-left: 1px solid #444a54;
            background: #313640;
            border-top-right-radius: 5px;
            border-bottom-right-radius: 5px;
        }

        QComboBox QAbstractItemView {
            color: #edf0f5;
            background: #2f3238;
            border: 1px solid #4a4f58;
            selection-background-color: #3774d6;
            selection-color: #ffffff;
            outline: 0;
        }

        QAbstractSpinBox::up-button,
        QAbstractSpinBox::down-button {
            width: 18px;
            background: #313640;
            border-left: 1px solid #444a54;
        }

        QAbstractSpinBox::up-button:hover,
        QAbstractSpinBox::down-button:hover {
            background: #3d4551;
        }

        QCheckBox {
            color: #e7e9ed;
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 15px;
            height: 15px;
            border-radius: 3px;
            border: 1px solid #5b626e;
            background: #25282e;
        }

        QCheckBox::indicator:hover {
            border-color: #6faeff;
        }

        QCheckBox::indicator:checked {
            background: #2f73d6;
            border-color: #6faeff;
        }

        QListWidget {
            padding: 5px;
            outline: 0;
        }

        QListWidget::item {
            color: #dfe4eb;
            border-radius: 5px;
            padding: 5px 8px;
        }

        QListWidget::item:hover {
            background: #3a404a;
        }

        QListWidget::item:selected {
            background: #2f73d6;
            color: #ffffff;
        }

        QListWidget#pipelineStripList {
            background: #25282e;
            border: 1px solid #434852;
            border-radius: 7px;
            padding: 5px;
        }

        QListWidget#pipelineStripList::item {
            background: #363b44;
            border: 1px solid #505762;
            margin: 2px;
            padding: 6px 10px;
        }

        QListWidget#pipelineStripList::item:selected {
            background: #2f73d6;
            border-color: #70aeff;
        }

        QScrollBar:vertical,
        QScrollBar:horizontal {
            background: #25282e;
            border: 0;
            margin: 0;
        }

        QScrollBar:vertical {
            width: 10px;
        }

        QScrollBar:horizontal {
            height: 10px;
        }

        QScrollBar::handle:vertical,
        QScrollBar::handle:horizontal {
            background: #555c67;
            border-radius: 5px;
            min-height: 24px;
            min-width: 24px;
        }

        QScrollBar::handle:vertical:hover,
        QScrollBar::handle:horizontal:hover {
            background: #6a7381;
        }

        QScrollBar::add-line,
        QScrollBar::sub-line {
            width: 0;
            height: 0;
        }

        QStatusBar {
            color: #b9c0cc;
            background: #1f2125;
            border-top: 1px solid #393c42;
        }

        QToolTip {
            color: #f5f7fb;
            background: #1d2025;
            border: 1px solid #5b6470;
            padding: 5px;
        }

        QDockWidget#modelListDock,
        QDockWidget#inspectorDock {
            background: #303238;
            color: #e7e9ed;
            border: 1px solid #474b54;
        }

        QDockWidget#modelListDock::title,
        QDockWidget#inspectorDock::title {
            background: #1f2125;
            color: #dfe2e7;
            padding: 6px 8px;
            border-bottom: 1px solid #393c42;
        }

        QTabWidget#workbenchLeftTabs,
        QTabWidget#workbenchInspectorTabs {
            background: #303238;
        }

        QTabWidget#workbenchLeftTabs QTabBar,
        QTabWidget#workbenchInspectorTabs QTabBar {
            background: #2a2d33;
            min-height: 42px;
        }

        QTabWidget#workbenchLeftTabs::pane,
        QTabWidget#workbenchInspectorTabs::pane {
            border: 1px solid #474b54;
            border-top: 0;
            background: #303238;
            border-bottom-left-radius: 6px;
            border-bottom-right-radius: 6px;
            padding: 10px 8px 8px 8px;
            margin-top: 2px;
        }

        QTabWidget#workbenchLeftTabs QTabBar::tab,
        QTabWidget#workbenchInspectorTabs QTabBar::tab {
            background: #2a2d33;
            color: #b8bec9;
            border: 1px solid #474b54;
            border-bottom: none;
            min-width: 88px;
            min-height: 34px;
            padding: 10px 20px;
            margin-top: 6px;
            margin-right: 4px;
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
        }

        QTabWidget#workbenchLeftTabs QTabBar::tab:selected,
        QTabWidget#workbenchInspectorTabs QTabBar::tab:selected {
            background: #303238;
            color: #f0f3f7;
            border-color: #555b66;
        }

        QTabWidget#workbenchLeftTabs QTabBar::tab:hover:!selected,
        QTabWidget#workbenchInspectorTabs QTabBar::tab:hover:!selected {
            background: #353942;
            color: #e7e9ed;
        }

        QPlainTextEdit#diagnosticsTextEdit {
            color: #dfe4eb;
            background: #25282e;
            border: 1px solid #4d535e;
            border-radius: 5px;
            padding: 6px;
            selection-background-color: #2f73d6;
            selection-color: #ffffff;
        }

        QLabel#statisticsMetricsLabel,
        QLabel#statisticsTimingLabel {
            color: #dfe4eb;
            background: #25282e;
            border: 1px solid #4d535e;
            border-radius: 5px;
            min-height: 52px;
            padding: 10px 12px;
        }

        QWidget#modelListPanel,
        QWidget#taskHistoryPanel,
        QWidget#statisticsPanel,
        QWidget#diagnosticsPanel {
            background: #303238;
        }

        QListWidget#modelListWidget,
        QListWidget#taskHistoryList {
            border: 1px solid #4d535e;
            border-radius: 5px;
            padding: 4px;
            min-height: 120px;
        }

        QLabel#statisticsTitleLabel,
        QLabel#taskHistoryTitleLabel,
        QLabel#modelListTitleLabel {
            color: #f0f3f7;
            font-weight: 700;
            padding-bottom: 2px;
        }
    )qss");

    ui->sourcePanel->hide();
    ui->resultPanel->hide();

    m_previewCanvasHost = new PreviewCanvasHost(ui->centralwidget);
    m_sourceCanvasView = m_previewCanvasHost->sourceCanvas();
    m_canvasView = m_previewCanvasHost->resultCanvas();
    ui->previewLayout->addWidget(m_previewCanvasHost, 1);
    if (ui->previewLayout->count() >= 2) {
        ui->previewLayout->setStretch(0, 1);
        ui->previewLayout->setStretch(1, 0);
    }

    connect(m_sourceCanvasView, &CanvasView::imagePointClicked, this, [this](const QPointF& imagePoint) {
        handleSourceCanvasPointClicked(imagePoint);
    });
    connect(m_sourceCanvasView, &CanvasView::cursorMoved, this, &MainWindow::updateCursorStatus);
    connect(m_canvasView, &CanvasView::cursorMoved, this, &MainWindow::updateCursorStatus);

    m_sourcePreviewTitleLabel = new QLabel(ui->sourcePanel);
    m_sourcePreviewTitleLabel->hide();
    m_resultPreviewTitleLabel = new QLabel(ui->resultPanel);
    m_resultPreviewTitleLabel->hide();

    auto* statusLayout = new QVBoxLayout(ui->statusPanel);
    statusLayout->setContentsMargins(12, 10, 12, 10);
    statusLayout->setSpacing(8);
    m_previewTitleLabel = new QLabel(ui->statusPanel);
    QFont sectionTitleFont = m_previewTitleLabel->font();
    sectionTitleFont.setPointSize(11);
    sectionTitleFont.setBold(true);
    m_previewTitleLabel->setFont(sectionTitleFont);
    statusLayout->addWidget(m_previewTitleLabel);
    m_previewInfoLabel = new QLabel(ui->statusPanel);
    m_previewInfoLabel->setWordWrap(true);
    m_previewInfoLabel->setMaximumHeight(44);
    statusLayout->addWidget(m_previewInfoLabel);
    m_previewDetailsLabel = new QLabel(ui->statusPanel);
    m_previewDetailsLabel->setObjectName("previewDetailsLabel");
    m_previewDetailsLabel->setWordWrap(true);
    m_previewDetailsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_previewDetailsLabel->setMaximumHeight(56);
    statusLayout->addWidget(m_previewDetailsLabel);

    m_timelineWidget = new TimelineWidget(ui->statusPanel);
    m_timelineWidget->setObjectName("timelineWidget");
    m_timelineWidget->setMinimumHeight(34);
    statusLayout->addWidget(m_timelineWidget);
    connect(m_timelineWidget, &TimelineWidget::frameChanged, this, [this](int frameIndex) {
        requestVideoFrame(frameIndex);
    });

    m_taskConsolePanel = new TaskConsolePanel(ui->statusPanel);
    statusLayout->addWidget(m_taskConsolePanel);

    m_statusDetailsLabel = new QLabel(this);
    m_statusDetailsLabel->setObjectName("statusDetailsLabel");
    m_cursorStatusLabel = new QLabel(this);
    m_cursorStatusLabel->setObjectName("cursorStatusLabel");
    m_appVersionLabel = new QLabel(this);
    m_appVersionLabel->setObjectName("appVersionLabel");
    statusBar()->addPermanentWidget(m_cursorStatusLabel);
    statusBar()->addPermanentWidget(m_statusDetailsLabel);
    statusBar()->addPermanentWidget(m_appVersionLabel);

    createWorkbenchDialogs();
    createEmbeddedWorkbench();
    createMenus();

    TaskScheduler& taskScheduler = ApplicationContext::instance().taskScheduler();
    connect(m_taskConsolePanel, &TaskConsolePanel::cancelRequested, &taskScheduler, &TaskScheduler::cancelTask);
    connect(m_taskConsolePanel, &TaskConsolePanel::pauseRequested, &taskScheduler, &TaskScheduler::pauseTask);
    connect(m_taskConsolePanel, &TaskConsolePanel::resumeRequested, &taskScheduler, &TaskScheduler::resumeTask);
    connect(&taskScheduler, &TaskScheduler::progressChanged, m_taskConsolePanel, &TaskConsolePanel::updateProgress);
    connect(&taskScheduler, &TaskScheduler::progressChanged, this, [this](const TaskProgress& progress) {
        updateLatestTaskHistory(progress);
    });
    connect(&taskScheduler, &TaskScheduler::taskFinished, this, [this](bool success) {
        if (!m_taskHistory.isEmpty()) {
            m_taskHistory[0].state = success ? TaskState::Completed : TaskState::Failed;
            m_taskHistory[0].finishedAt = QDateTime::currentDateTime();
            refreshTaskHistoryPanel();
        }
        if (m_taskConsolePanel) {
            m_taskConsolePanel->appendLogSummary(DiagnosticsLog::instance().summaryText());
        }
        if (!success) {
            statusBar()->showMessage(localizedText(
                m_appSettings.languageCode,
                "Background task failed",
                QStringLiteral("后台任务失败")
            ));
        }
    });

    setAcceptDrops(true);

    m_videoPlaybackTimer = new QTimer(this);
    m_videoPlaybackTimer->setSingleShot(false);

    connect(m_mediaPanel, &MediaPanel::openImageRequested, this, [this] { loadImageFile(); });
    connect(m_mediaPanel, &MediaPanel::openVideoRequested, this, [this] { loadVideoFile(); });
    connect(m_mediaPanel, &MediaPanel::importYoloModelRequested, this, [this] { importYoloModel(); });
    connect(m_mediaPanel, &MediaPanel::exportDetectionImagesRequested, this, [this] { exportDetectionImages(); });
    connect(m_mediaPanel, &MediaPanel::exportDetectionVideoRequested, this, [this] { exportDetectionVideo(); });
    connect(m_mediaPanel, &MediaPanel::videoFrameRequested, this, [this](int frameIndex) { requestVideoFrame(frameIndex); });
    connect(m_mediaPanel, &MediaPanel::previousVideoFrameRequested, this, [this] { requestPreviousVideoFrame(); });
    connect(m_mediaPanel, &MediaPanel::nextVideoFrameRequested, this, [this] { requestNextVideoFrame(); });
    connect(m_mediaPanel, &MediaPanel::videoPlaybackToggled, this, [this](bool playing) { toggleVideoPlayback(playing); });
    connect(m_videoPlaybackTimer, &QTimer::timeout, this, [this] { advanceVideoPlayback(); });
    connect(m_exportResultsPanel, &ExportResultsPanel::openOutputDirectoryRequested, this, [this](const QString& outputDirectory) {
        openExportOutputDirectory(outputDirectory);
    });
    connect(m_exportResultsPanel, &ExportResultsPanel::openArtifactRequested, this, [this](const QString& outputDirectory, const QString& artifactName) {
        openExportArtifact(outputDirectory, artifactName);
    });
    connect(m_pipelinePanel, &PipelinePanel::addOperatorRequested, this, [this](const QString& operatorId) {
        addOperatorToPipeline(operatorId);
    });
    connect(m_pipelinePanel, &PipelinePanel::pipelineStepSelected, this, [this](int index) {
        selectPipelineStep(index);
    });
    connect(m_pipelinePanel, &PipelinePanel::pipelineStepRenamed, this, [this](int index, const QString& name) {
        renamePipelineStep(index, name);
    });
    connect(m_pipelinePanel, &PipelinePanel::pipelineStepMoved, this, [this](int from, int to) {
        movePipelineStep(from, to);
    });
    connect(m_pipelinePanel, &PipelinePanel::removePipelineStepRequested, this, [this](int index) {
        removePipelineStep(index);
    });
    connect(m_pipelinePanel, &PipelinePanel::clearPipelineRequested, this, [this] {
        clearPipeline();
    });
    connect(m_pipelinePanel, &PipelinePanel::savePipelineRequested, this, [this] {
        savePipelineToFile();
    });
    connect(m_pipelinePanel, &PipelinePanel::loadPipelineRequested, this, [this] {
        loadPipelineFromFile();
    });
    connect(m_parameterPanel, &ParameterPanel::parameterValuesChanged, this, [this](const QVariantMap& values) {
        updateOperatorParameters(values);
    });
    connect(m_parameterPanel, &ParameterPanel::resetToDefaultsRequested, this, &MainWindow::handleParameterReset);
    connect(m_parameterPanel, &ParameterPanel::savePresetRequested, this, &MainWindow::handleParameterSavePreset);
    connect(m_parameterPanel, &ParameterPanel::loadPresetRequested, this, &MainWindow::handleParameterLoadPreset);

    registerApplicationCommands();
    DnnRuntimeInfo::logBackendInfo();
    const DnnRuntimeInfo::BackendInfo backendInfo = DnnRuntimeInfo::queryBackendInfo();
    DiagnosticsLog::instance().recordRuntimeBackend(backendInfo.preferredBackend, backendInfo.preferredTarget);
    if (m_appVersionLabel) {
        m_appVersionLabel->setText(QString("%1 | %2")
            .arg(AppVersion::displayName(), backendInfo.preferredBackend));
    }

    setLanguage(m_appSettings.languageCode);
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        "Ready: media and operator foundation",
        QStringLiteral("就绪: 媒体与算子基础功能已加载")
    ));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    if (!m_workbenchDocksInitialized) {
        initializeWorkbenchDocks();
        m_workbenchDocksInitialized = true;
        showWelcomeGuideIfNeeded();
    }
}

void MainWindow::initializeWorkbenchDocks()
{
    if (m_modelDock) {
        return;
    }

    m_modelListPanel = new ModelListPanel(this);
    m_taskHistoryPanel = new TaskHistoryPanel(this);
    auto* leftTabs = new QTabWidget(this);
    leftTabs->setObjectName("workbenchLeftTabs");
    leftTabs->addTab(m_modelListPanel, "Models");
    leftTabs->addTab(m_taskHistoryPanel, "Tasks");
    m_modelDock = new QDockWidget("Models", this);
    m_modelDock->setObjectName("modelListDock");
    m_modelDock->setWidget(wrapWorkbenchDockWidget(leftTabs));
    m_modelDock->setMinimumWidth(280);
    suppressDockTitleBar(m_modelDock);
    addDockWidget(Qt::LeftDockWidgetArea, m_modelDock);

    m_diagnosticsPanel = new DiagnosticsPanel(this);
    m_statisticsPanel = new StatisticsPanel(this);
    auto* inspectorTabs = new QTabWidget(this);
    inspectorTabs->setObjectName("workbenchInspectorTabs");
    inspectorTabs->addTab(m_statisticsPanel, "Statistics");
    inspectorTabs->addTab(m_diagnosticsPanel, "Diagnostics");
    m_inspectorDock = new QDockWidget("Inspector", this);
    m_inspectorDock->setObjectName("inspectorDock");
    m_inspectorDock->setWidget(wrapWorkbenchDockWidget(inspectorTabs));
    m_inspectorDock->setMinimumWidth(300);
    suppressDockTitleBar(m_inspectorDock);
    addDockWidget(Qt::RightDockWidgetArea, m_inspectorDock);

    connect(m_modelListPanel, &ModelListPanel::modelActivated, this, &MainWindow::activateModelByName);
    connect(m_modelListPanel, &ModelListPanel::editModelRequested, this, [this](const QString&) {
        editActiveModelConfig();
    });
    connect(m_taskHistoryPanel, &TaskHistoryPanel::openOutputDirectoryRequested, this, &MainWindow::openExportOutputDirectory);
    m_modelListPanel->setLanguage(m_appSettings.languageCode);
    m_taskHistoryPanel->setLanguage(m_appSettings.languageCode);
    if (m_diagnosticsPanel) {
        m_diagnosticsPanel->setLanguage(m_appSettings.languageCode);
    }
    if (m_statisticsPanel) {
        m_statisticsPanel->setLanguage(m_appSettings.languageCode);
    }
    m_modelListPanel->refreshModels();
    refreshTaskHistoryPanel();
    localizeWorkbenchDocks(m_appSettings.languageCode);
    setupWorkbenchViewMenu();
}

void MainWindow::setupWorkbenchViewMenu()
{
    if (!m_viewMenu || !m_modelDock || !m_inspectorDock || m_showModelDockAction) {
        return;
    }

    m_showModelDockAction = m_modelDock->toggleViewAction();
    m_showModelDockAction->setObjectName("showModelDockAction");
    m_showModelDockAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M));

    m_showInspectorDockAction = m_inspectorDock->toggleViewAction();
    m_showInspectorDockAction->setObjectName("showInspectorDockAction");
    m_showInspectorDockAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I));

    m_viewMenu->addAction(m_showModelDockAction);
    m_viewMenu->addAction(m_showInspectorDockAction);
    m_viewMenu->addSeparator();

    m_showAllWorkbenchPanelsAction = m_viewMenu->addAction(
        "Show All Side Panels",
        this,
        [this] {
            if (m_modelDock) {
                m_modelDock->show();
            }
            if (m_inspectorDock) {
                m_inspectorDock->show();
            }
        });
    m_showAllWorkbenchPanelsAction->setObjectName("showAllWorkbenchPanelsAction");

    m_hideAllWorkbenchPanelsAction = m_viewMenu->addAction(
        "Hide All Side Panels",
        this,
        [this] {
            if (m_modelDock) {
                m_modelDock->hide();
            }
            if (m_inspectorDock) {
                m_inspectorDock->hide();
            }
        });
    m_hideAllWorkbenchPanelsAction->setObjectName("hideAllWorkbenchPanelsAction");
    m_viewMenu->addSeparator();

    m_showDiagnosticsAction = m_viewMenu->addAction(
        "Show Diagnostics",
        this,
        [this] { showInspectorPanel(1); });
    m_showDiagnosticsAction->setObjectName("showDiagnosticsAction");
    m_showDiagnosticsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));

    m_modelDock->hide();
    m_inspectorDock->hide();

    rebuildMenuTexts();
}

void MainWindow::showInspectorPanel(int tabIndex)
{
    if (!m_inspectorDock) {
        return;
    }

    m_inspectorDock->show();
    m_inspectorDock->raise();
    if (auto* tabs = findWorkbenchTabs(m_inspectorDock)) {
        if (tabIndex >= 0 && tabIndex < tabs->count()) {
            tabs->setCurrentIndex(tabIndex);
        }
    }
}

void MainWindow::localizeWorkbenchDocks(const QString& languageCode)
{
    if (m_modelDock) {
        m_modelDock->setWindowTitle(localizedText(languageCode, "Models", QStringLiteral("\u6a21\u578b")));
        if (auto* tabs = findWorkbenchTabs(m_modelDock)) {
            tabs->setTabText(0, localizedText(languageCode, "Models", QStringLiteral("\u6a21\u578b")));
            tabs->setTabText(1, localizedText(languageCode, "Tasks", QStringLiteral("\u4efb\u52a1")));
        }
    }
    if (m_inspectorDock) {
        m_inspectorDock->setWindowTitle(localizedText(languageCode, "Inspector", QStringLiteral("\u68c0\u67e5\u5668")));
        if (auto* tabs = findWorkbenchTabs(m_inspectorDock)) {
            tabs->setTabText(0, localizedText(languageCode, "Statistics", QStringLiteral("\u7edf\u8ba1")));
            tabs->setTabText(1, localizedText(languageCode, "Diagnostics", QStringLiteral("\u8bca\u65ad")));
        }
    }
}

void MainWindow::setupKeyboardShortcuts()
{
    if (m_openImageAction) {
        m_openImageAction->setShortcut(QKeySequence::Open);
    }
    if (m_saveProjectAction) {
        m_saveProjectAction->setShortcut(QKeySequence::Save);
    }
    if (m_loadProjectAction) {
        m_loadProjectAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    }
    if (m_savePipelineAction) {
        m_savePipelineAction->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S));
    }
    if (m_systemSettingsAction) {
        m_systemSettingsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Comma));
    }

    auto* rerunShortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(rerunShortcut, &QShortcut::activated, this, [this] { rerunPreview(); });

    auto* playPauseShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(playPauseShortcut, &QShortcut::activated, this, [this] {
        if (m_currentMediaInfo.kind == MediaSourceKind::VideoFile
            || m_currentMediaInfo.kind == MediaSourceKind::ImageFolder) {
            toggleVideoPlayback(!m_videoPlaybackTimer->isActive());
        }
    });
}

void MainWindow::createWorkbenchDialogs()
{
    m_operatorParameterDialog = new OperatorParameterDialog(this);
    m_operatorParameterDialog->setWindowFlags(Qt::Window);
    m_operatorParameterDialog->setObjectName("operatorParameterDialog");
    m_operatorParameterDialog->setModal(false);
    connect(m_operatorParameterDialog, &OperatorParameterDialog::previewRequested, this, [this] {
        m_singlePreviewStep = m_operatorParameterDialog->step();
        m_singlePreviewDisplayName = m_operatorParameterDialog->operatorDisplayName();
        setPreviewMode(PreviewMode::SingleNode);
        rerunPreview();
    });
    connect(m_operatorParameterDialog, &OperatorParameterDialog::addToPipelineRequested, this, [this] {
        const QString operatorId = m_operatorParameterDialog->operatorId();
        const auto sourceStep = m_operatorParameterDialog->step();
        if (operatorId.isEmpty() || !sourceStep) {
            return;
        }

        auto clonedStep = ApplicationContext::instance().operatorRegistry().create(operatorId);
        if (!clonedStep) {
            return;
        }

        clonedStep->setParameterValues(sourceStep->parameterValues());
        m_pipelineSteps.push_back({m_operatorParameterDialog->operatorDisplayName(), clonedStep});
        m_selectedPipelineStepIndex = static_cast<int>(m_pipelineSteps.size()) - 1;
        m_pipelinePanel->setPipelineStepNames(pipelineStepNames());
        m_pipelinePanel->setCurrentPipelineStep(m_selectedPipelineStepIndex);
        m_parameterPanel->setStep(m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step);
        setPreviewMode(PreviewMode::FullPipeline);
        rebuildPipelineStrip();
        updateEmbeddedWorkbenchState();
        rerunPreview();
    });

    m_mediaWorkbenchDialog = new QDialog(this, Qt::Window);
    m_mediaWorkbenchDialog->setObjectName("mediaWorkbenchDialog");
    m_mediaWorkbenchDialog->setModal(false);
    m_mediaWorkbenchDialog->resize(560, 420);
    auto* mediaLayout = new QVBoxLayout(m_mediaWorkbenchDialog);
    mediaLayout->setContentsMargins(0, 0, 0, 0);
    m_mediaPanel = new MediaPanel(m_mediaWorkbenchDialog);
    mediaLayout->addWidget(m_mediaPanel);

    m_pipelineWorkbenchDialog = new QDialog(this, Qt::Window);
    m_pipelineWorkbenchDialog->setObjectName("pipelineWorkbenchDialog");
    m_pipelineWorkbenchDialog->setModal(false);
    m_pipelineWorkbenchDialog->resize(1180, 760);
    auto* pipelineLayout = new QHBoxLayout(m_pipelineWorkbenchDialog);
    pipelineLayout->setContentsMargins(0, 0, 0, 0);
    pipelineLayout->setSpacing(12);
    m_pipelinePanel = new PipelinePanel(m_pipelineWorkbenchDialog);
    m_parameterPanel = new ParameterPanel(m_pipelineWorkbenchDialog);
    pipelineLayout->addWidget(m_pipelinePanel, 5);
    pipelineLayout->addWidget(m_parameterPanel, 4);

    m_exportResultsDialog = new QDialog(this, Qt::Window);
    m_exportResultsDialog->setObjectName("exportResultsDialog");
    m_exportResultsDialog->setModal(false);
    m_exportResultsDialog->resize(560, 720);
    auto* exportLayout = new QVBoxLayout(m_exportResultsDialog);
    exportLayout->setContentsMargins(0, 0, 0, 0);
    m_exportResultsPanel = new ExportResultsPanel(m_exportResultsDialog);
    exportLayout->addWidget(m_exportResultsPanel);

    m_pipelinePanel->setOperators(ApplicationContext::instance().operatorRegistry().allOperators());
}

void MainWindow::createEmbeddedWorkbench()
{
    m_operatorWorkbenchPanel = new QFrame(ui->centralwidget);
    m_operatorWorkbenchPanel->setObjectName("operatorWorkbenchPanel");
    m_operatorWorkbenchPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    auto* workbenchLayout = new QVBoxLayout(m_operatorWorkbenchPanel);
    workbenchLayout->setContentsMargins(10, 8, 10, 8);
    workbenchLayout->setSpacing(6);

    m_operatorWorkbenchTitleLabel = new QLabel(m_operatorWorkbenchPanel);
    QFont titleFont = m_operatorWorkbenchTitleLabel->font();
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    m_operatorWorkbenchTitleLabel->setFont(titleFont);
    workbenchLayout->addWidget(m_operatorWorkbenchTitleLabel);

    auto* operatorButtonsGrid = new QGridLayout();
    operatorButtonsGrid->setHorizontalSpacing(6);
    operatorButtonsGrid->setVerticalSpacing(6);
    operatorButtonsGrid->setContentsMargins(0, 0, 0, 0);
    const QList<OperatorDescriptor> descriptors = ApplicationContext::instance().operatorRegistry().allOperators();
    int categoryButtonIndex = 0;
    for (const OperatorCategoryDefinition& category : operatorMenuCategories()) {
        const QString categoryKey = QString::fromLatin1(category.key);
        auto* button = new QToolButton(m_operatorWorkbenchPanel);
        button->setObjectName(QString("quickAddCategoryButton_%1").arg(categoryKey));
        button->setProperty("operatorCategoryKey", categoryKey);
        button->setPopupMode(QToolButton::InstantPopup);
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        button->setMinimumHeight(28);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        auto* menu = new QMenu(button);
        menu->setObjectName(QString("quickAddCategoryMenu_%1").arg(categoryKey));
        if (!populateOperatorCategoryMenu(
                menu,
                categoryKey,
                category.operatorIds,
                descriptors,
                m_appSettings.languageCode,
                [this](const OperatorDescriptor& descriptor) {
                    addOperatorToPipeline(descriptor.id);
                })) {
            delete menu;
            delete button;
            continue;
        }

        button->setMenu(menu);
        m_operatorQuickAddButtons.push_back(button);
        const int row = categoryButtonIndex / 3;
        const int column = categoryButtonIndex % 3;
        operatorButtonsGrid->addWidget(button, row, column);
        ++categoryButtonIndex;
    }
    operatorButtonsGrid->setColumnStretch(0, 1);
    operatorButtonsGrid->setColumnStretch(1, 1);
    operatorButtonsGrid->setColumnStretch(2, 1);
    workbenchLayout->addLayout(operatorButtonsGrid);

    m_pipelineStripTitleLabel = new QLabel(m_operatorWorkbenchPanel);
    QFont stripTitleFont = m_pipelineStripTitleLabel->font();
    stripTitleFont.setBold(true);
    m_pipelineStripTitleLabel->setFont(stripTitleFont);
    workbenchLayout->addWidget(m_pipelineStripTitleLabel);

    m_pipelineStripList = new QListWidget(m_operatorWorkbenchPanel);
    m_pipelineStripList->setObjectName("pipelineStripList");
    m_pipelineStripList->setViewMode(QListView::ListMode);
    m_pipelineStripList->setFlow(QListView::LeftToRight);
    m_pipelineStripList->setWrapping(false);
    m_pipelineStripList->setSpacing(4);
    m_pipelineStripList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pipelineStripList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pipelineStripList->setMaximumHeight(56);
    m_pipelineStripList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_pipelineStripList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (m_updatingPipelineStrip) {
            return;
        }
        if (row >= 0) {
            selectPipelineStep(row);
        }
    });
    workbenchLayout->addWidget(m_pipelineStripList);

    auto* actionsRow = new QHBoxLayout();
    actionsRow->setSpacing(6);
    m_removeCurrentStepButton = new QPushButton(m_operatorWorkbenchPanel);
    m_removeCurrentStepButton->setObjectName("removeCurrentStepButton");
    connect(m_removeCurrentStepButton, &QPushButton::clicked, this, [this] {
        removePipelineStep(m_selectedPipelineStepIndex);
    });
    actionsRow->addWidget(m_removeCurrentStepButton);

    m_clearPipelineInlineButton = new QPushButton(m_operatorWorkbenchPanel);
    m_clearPipelineInlineButton->setObjectName("clearPipelineInlineButton");
    connect(m_clearPipelineInlineButton, &QPushButton::clicked, this, [this] {
        clearPipeline();
    });
    actionsRow->addWidget(m_clearPipelineInlineButton);

    m_savePipelineInlineButton = new QPushButton(m_operatorWorkbenchPanel);
    m_savePipelineInlineButton->setObjectName("savePipelineInlineButton");
    connect(m_savePipelineInlineButton, &QPushButton::clicked, this, [this] {
        savePipelineToFile();
    });
    actionsRow->addWidget(m_savePipelineInlineButton);

    m_loadPipelineInlineButton = new QPushButton(m_operatorWorkbenchPanel);
    m_loadPipelineInlineButton->setObjectName("loadPipelineInlineButton");
    connect(m_loadPipelineInlineButton, &QPushButton::clicked, this, [this] {
        loadPipelineFromFile();
    });
    actionsRow->addWidget(m_loadPipelineInlineButton);

    actionsRow->addStretch();

    m_fullPipelinePreviewButton = new QPushButton(m_operatorWorkbenchPanel);
    m_fullPipelinePreviewButton->setObjectName("fullPipelinePreviewButton");
    m_fullPipelinePreviewButton->setCheckable(true);
    connect(m_fullPipelinePreviewButton, &QPushButton::clicked, this, [this] {
        setPreviewMode(PreviewMode::FullPipeline);
        rerunPreview();
    });
    actionsRow->addWidget(m_fullPipelinePreviewButton);

    m_singleNodePreviewButton = new QPushButton(m_operatorWorkbenchPanel);
    m_singleNodePreviewButton->setObjectName("singleNodePreviewButton");
    m_singleNodePreviewButton->setCheckable(true);
    connect(m_singleNodePreviewButton, &QPushButton::clicked, this, [this] {
        setPreviewMode(PreviewMode::SingleNode);
        rerunPreview();
    });
    actionsRow->addWidget(m_singleNodePreviewButton);

    workbenchLayout->addLayout(actionsRow);

    ui->verticalLayout->insertWidget(0, m_operatorWorkbenchPanel);
    ui->verticalLayout->setStretch(0, 0);
    ui->verticalLayout->setStretch(1, 4);
    ui->verticalLayout->setStretch(2, 1);

    if (m_parameterPanel) {
        m_parameterPanel->setCompactMode(true);
        m_parameterPanel->setMinimumHeight(64);
        m_parameterPanel->setMaximumHeight(140);
        m_parameterPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        if (auto* statusLayout = qobject_cast<QVBoxLayout*>(ui->statusPanel->layout())) {
            const int detailsIndex = statusLayout->indexOf(m_previewDetailsLabel);
            if (detailsIndex >= 0) {
                statusLayout->insertWidget(detailsIndex + 1, m_parameterPanel, 0);
            } else {
                statusLayout->insertWidget(3, m_parameterPanel, 0);
            }
        }
    }

    setPreviewMode(PreviewMode::FullPipeline);
    rebuildEmbeddedWorkbenchTexts();
    rebuildPipelineStrip();
    updateEmbeddedWorkbenchState();
    setOperatorWorkbenchVisible(false);
}

void MainWindow::presentDialog(QDialog* dialog)
{
    if (!dialog) {
        return;
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void MainWindow::rebuildEmbeddedWorkbenchTexts()
{
    const QString languageCode = m_appSettings.languageCode;

    if (m_operatorWorkbenchTitleLabel) {
        m_operatorWorkbenchTitleLabel->setText(localizedText(
            languageCode,
            "Operator Workbench",
            QStringLiteral("\u7b97\u5b50\u5de5\u4f5c\u5e26")
        ));
    }
    if (m_pipelineStripTitleLabel) {
        m_pipelineStripTitleLabel->setText(localizedText(
            languageCode,
            "Pipeline Strip",
            QStringLiteral("\u6d41\u7a0b\u6761")
        ));
    }
    if (m_removeCurrentStepButton) {
        m_removeCurrentStepButton->setText(localizedText(
            languageCode,
            "Remove Step",
            QStringLiteral("\u5220\u9664\u6b65\u9aa4")
        ));
    }
    if (m_clearPipelineInlineButton) {
        m_clearPipelineInlineButton->setText(localizedText(
            languageCode,
            "Clear Pipeline",
            QStringLiteral("\u6e05\u7a7a\u6d41\u7a0b")
        ));
    }
    if (m_savePipelineInlineButton) {
        m_savePipelineInlineButton->setText(localizedText(
            languageCode,
            "Save Pipeline",
            QStringLiteral("\u4fdd\u5b58\u6d41\u7a0b")
        ));
    }
    if (m_loadPipelineInlineButton) {
        m_loadPipelineInlineButton->setText(localizedText(
            languageCode,
            "Load Pipeline",
            QStringLiteral("\u52a0\u8f7d\u6d41\u7a0b")
        ));
    }
    if (m_fullPipelinePreviewButton) {
        m_fullPipelinePreviewButton->setText(localizedText(
            languageCode,
            "Full Pipeline",
            QStringLiteral("\u5168\u6d41\u7a0b")
        ));
    }
    if (m_singleNodePreviewButton) {
        m_singleNodePreviewButton->setText(localizedText(
            languageCode,
            "Single Node",
            QStringLiteral("\u5355\u8282\u70b9")
        ));
    }

    const QList<OperatorDescriptor> descriptors = ApplicationContext::instance().operatorRegistry().allOperators();
    for (QToolButton* button : m_operatorQuickAddButtons) {
        if (!button) {
            continue;
        }
        const QString categoryKey = button->property("operatorCategoryKey").toString();
        button->setText(localizedOperatorCategoryTitle(categoryKey, languageCode));
        localizeOperatorMenuTree(button->menu(), descriptors, languageCode);
    }

    rebuildPipelineStrip();
    updateEmbeddedWorkbenchState();
}

void MainWindow::rebuildPipelineStrip()
{
    if (!m_pipelineStripList) {
        return;
    }

    m_updatingPipelineStrip = true;
    m_pipelineStripList->clear();
    for (const PipelineStepState& state : m_pipelineSteps) {
        const QString displayName = localizedOperatorDisplayName(
            state.step ? state.step->id() : QString(),
            state.displayName,
            m_appSettings.languageCode
        );
        auto* item = new QListWidgetItem(displayName, m_pipelineStripList);
        item->setToolTip(formatParameterMap(state.step));
    }
    if (m_selectedPipelineStepIndex >= 0 && m_selectedPipelineStepIndex < m_pipelineStripList->count()) {
        m_pipelineStripList->setCurrentRow(m_selectedPipelineStepIndex);
    }
    m_updatingPipelineStrip = false;
}

void MainWindow::updateEmbeddedWorkbenchState()
{
    const bool hasSelection = m_selectedPipelineStepIndex >= 0
        && m_selectedPipelineStepIndex < static_cast<int>(m_pipelineSteps.size());
    const bool hasSteps = !m_pipelineSteps.empty();

    if (m_removeCurrentStepButton) {
        m_removeCurrentStepButton->setEnabled(hasSelection);
    }
    if (m_clearPipelineInlineButton) {
        m_clearPipelineInlineButton->setEnabled(hasSteps);
    }
    if (m_savePipelineInlineButton) {
        m_savePipelineInlineButton->setEnabled(hasSteps);
    }
    if (m_singleNodePreviewButton) {
        m_singleNodePreviewButton->setEnabled(hasSelection || m_singlePreviewStep);
    }
    if (m_pipelineStripList) {
        m_pipelineStripList->setEnabled(true);
    }
}

void MainWindow::setPreviewMode(PreviewMode mode)
{
    m_previewMode = mode;
    const bool fullPipeline = mode == PreviewMode::FullPipeline;

    if (m_fullPipelinePreviewAction) {
        m_fullPipelinePreviewAction->setChecked(fullPipeline);
    }
    if (m_singleNodePreviewAction) {
        m_singleNodePreviewAction->setChecked(!fullPipeline);
    }
    if (m_fullPipelinePreviewButton) {
        m_fullPipelinePreviewButton->setChecked(fullPipeline);
    }
    if (m_singleNodePreviewButton) {
        m_singleNodePreviewButton->setChecked(!fullPipeline);
    }
    updateEmbeddedWorkbenchState();
}

void MainWindow::loadImageFile(const QString& filePath)
{
    QString resolvedPath = filePath;
    if (resolvedPath.isEmpty()) {
        const QString dialogTitle = localizedText(m_appSettings.languageCode, "Open Image", QStringLiteral("打开图片"));
        resolvedPath = QFileDialog::getOpenFileName(this, dialogTitle, QString(), "Images (*.png *.jpg *.bmp)");
    }
    if (resolvedPath.isEmpty()) {
        return;
    }

    recordRecentMedia(resolvedPath);
    loadMediaFromPath(resolvedPath, MediaSourceKind::ImageFile, 0);
}

void MainWindow::loadVideoFile(const QString& filePath)
{
    QString resolvedPath = filePath;
    if (resolvedPath.isEmpty()) {
        const QString dialogTitle = localizedText(m_appSettings.languageCode, "Open Video", QStringLiteral("打开视频"));
        resolvedPath = QFileDialog::getOpenFileName(this, dialogTitle, QString(), "Videos (*.avi *.mp4 *.mov *.mkv)");
    }
    if (resolvedPath.isEmpty()) {
        return;
    }

    recordRecentMedia(resolvedPath);
    loadMediaFromPath(resolvedPath, MediaSourceKind::VideoFile, 0);
}

void MainWindow::loadImageFolder(const QString& folderPath)
{
    QString resolvedPath = folderPath;
    if (resolvedPath.isEmpty()) {
        const QString dialogTitle = localizedText(
            m_appSettings.languageCode,
            "Open Image Folder",
            QStringLiteral("打开图片文件夹")
        );
        resolvedPath = QFileDialog::getExistingDirectory(this, dialogTitle, QString());
    }
    if (resolvedPath.isEmpty()) {
        return;
    }

    recordRecentMedia(resolvedPath);
    loadMediaFromPath(resolvedPath, MediaSourceKind::ImageFolder, 0);
}

bool MainWindow::loadMediaFromPath(const QString& path, MediaSourceKind kind, int frameIndex)
{
    if (path.isEmpty()) {
        return false;
    }

    QString resolvedPath = path;
    if (!QFileInfo::exists(resolvedPath)) {
        const QString resourcePath = PlatformPaths::resolveResourcePath(path);
        if (QFileInfo::exists(resourcePath)) {
            resolvedPath = resourcePath;
        }
    }

    cacheCurrentImageForWorkspaceUse(resolvedPath);

    if (kind == MediaSourceKind::ImageFile) {
        ImageFileSource source(resolvedPath);
        FrameReadResult result = source.read({});
        if (!result.success) {
            statusBar()->showMessage(result.errorMessage);
            return false;
        }

        m_currentFrame = result.frame;
        syncWorkspaceCachedImageArtifact();
        m_perspectivePoints.clear();
        m_affinePoints.clear();
        m_floodFillSeeds.clear();
        m_grabCutRectPoints.clear();
        m_currentMediaInfo.kind = MediaSourceKind::ImageFile;
        m_currentMediaInfo.sourceId = resolvedPath;
        m_currentMediaInfo.width = m_currentFrame.originalMat.cols;
        m_currentMediaInfo.height = m_currentFrame.originalMat.rows;
        m_currentMediaInfo.frameCount = 1;
        m_currentMediaInfo.fps = 0.0;
        m_currentVideoFrameIndex = 0;
        m_mediaPanel->setVideoNavigationState(false, 0, 0);
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            QString("Loaded image: %1").arg(path),
            QStringLiteral("已加载图片: %1").arg(path)
        ));
        rerunPreview();
        return true;
    }

    if (kind == MediaSourceKind::VideoFile) {
        VideoFileSource source(resolvedPath);
        m_currentMediaInfo = source.mediaInfo();

        FrameReadResult result = source.read({frameIndex});
        if (!result.success) {
            statusBar()->showMessage(result.errorMessage);
            return false;
        }

        m_currentFrame = result.frame;
        syncWorkspaceCachedImageArtifact();
        m_perspectivePoints.clear();
        m_affinePoints.clear();
        m_floodFillSeeds.clear();
        m_grabCutRectPoints.clear();
        m_currentVideoFrameIndex = frameIndex;
        m_mediaPanel->setVideoPlaybackState(false);
        m_mediaPanel->setVideoNavigationState(true, m_currentMediaInfo.frameCount, frameIndex);
        rebuildMenuTexts();
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            QString("Loaded video: %1 (%2 frames)").arg(resolvedPath).arg(m_currentMediaInfo.frameCount),
            QStringLiteral("已加载视频: %1 (%2 帧)").arg(resolvedPath).arg(m_currentMediaInfo.frameCount)
        ));
        rerunPreview();
        return true;
    }

    ImageFolderSource source(resolvedPath);
    m_currentMediaInfo = source.mediaInfo();
    if (m_currentMediaInfo.frameCount <= 0) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            QString("Image folder is empty: %1").arg(resolvedPath),
            QStringLiteral("图片文件夹为空: %1").arg(resolvedPath)
        ));
        return false;
    }

    FrameReadResult result = source.read({frameIndex});
    if (!result.success) {
        statusBar()->showMessage(result.errorMessage);
        return false;
    }

    m_currentFrame = result.frame;
    syncWorkspaceCachedImageArtifact();
    m_perspectivePoints.clear();
    m_affinePoints.clear();
    m_floodFillSeeds.clear();
    m_grabCutRectPoints.clear();
    m_currentVideoFrameIndex = frameIndex;
    m_mediaPanel->setVideoPlaybackState(false);
    m_mediaPanel->setVideoNavigationState(true, m_currentMediaInfo.frameCount, frameIndex);
    rebuildMenuTexts();
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Loaded image folder: %1 (%2 images)").arg(resolvedPath).arg(m_currentMediaInfo.frameCount),
        QStringLiteral("已加载图片文件夹: %1 (%2 张图片)").arg(resolvedPath).arg(m_currentMediaInfo.frameCount)
    ));
    rerunPreview();
    return true;
}

void MainWindow::recordRecentMedia(const QString& path)
{
    const QString trimmedPath = path.trimmed();
    if (trimmedPath.isEmpty()) {
        return;
    }

    m_appSettings.recentMediaFiles.removeAll(trimmedPath);
    m_appSettings.recentMediaFiles.prepend(trimmedPath);
    trimRecentFileLists();

    AppSettingsStore::save(m_appSettings);
    rebuildRecentMediaMenu();
}

void MainWindow::trimRecentFileLists()
{
    while (m_appSettings.recentMediaFiles.size() > m_appSettings.maxRecentMediaFiles) {
        m_appSettings.recentMediaFiles.removeLast();
    }
    while (m_appSettings.recentProjectFiles.size() > m_appSettings.maxRecentMediaFiles) {
        m_appSettings.recentProjectFiles.removeLast();
    }
}

void MainWindow::rebuildRecentMediaMenu()
{
    if (!m_openRecentMenu) {
        return;
    }

    m_openRecentMenu->clear();
    for (const QString& path : m_appSettings.recentMediaFiles) {
        if (path.isEmpty()) {
            continue;
        }

        m_openRecentMenu->addAction(QFileInfo(path).fileName(), this, [this, path] {
            if (QFileInfo(path).isDir()) {
                loadImageFolder(path);
                return;
            }

            if (isVideoFilePath(path)) {
                loadVideoFile(path);
                return;
            }

            loadImageFile(path);
        });
    }

    if (m_openRecentMenu->isEmpty()) {
        QAction* emptyAction = m_openRecentMenu->addAction(localizedText(
            m_appSettings.languageCode,
            "No Recent Media",
            QStringLiteral("暂无最近媒体")
        ));
        emptyAction->setEnabled(false);
    }
}

void MainWindow::recordRecentProject(const QString& filePath)
{
    const QString trimmedPath = filePath.trimmed();
    if (trimmedPath.isEmpty()) {
        return;
    }

    m_appSettings.recentProjectFiles.removeAll(trimmedPath);
    m_appSettings.recentProjectFiles.prepend(trimmedPath);
    trimRecentFileLists();

    AppSettingsStore::save(m_appSettings);
    rebuildRecentProjectMenu();
}

void MainWindow::rebuildRecentProjectMenu()
{
    if (!m_openRecentProjectMenu) {
        return;
    }

    m_openRecentProjectMenu->clear();
    for (const QString& path : m_appSettings.recentProjectFiles) {
        if (path.isEmpty()) {
            continue;
        }

        m_openRecentProjectMenu->addAction(QFileInfo(path).fileName(), this, [this, path] {
            loadProjectFromPath(path);
        });
    }

    if (m_openRecentProjectMenu->isEmpty()) {
        QAction* emptyAction = m_openRecentProjectMenu->addAction(localizedText(
            m_appSettings.languageCode,
            "No Recent Projects",
            QStringLiteral("暂无最近项目")
        ));
        emptyAction->setEnabled(false);
    }
}

ProjectDefinition MainWindow::buildProjectDefinition() const
{
    ProjectDefinition project;
    project.projectId = "cvverify.project";
    project.displayName = m_currentProjectFilePath.isEmpty()
        ? localizedText(m_appSettings.languageCode, "Untitled Project", QStringLiteral("未命名项目"))
        : QFileInfo(m_currentProjectFilePath).completeBaseName();
    project.mediaSourcePath = m_currentMediaInfo.sourceId;
    project.mediaSourceKind = mediaSourceKindToString(m_currentMediaInfo.kind);
    project.currentVideoFrameIndex = m_currentVideoFrameIndex;
    project.pipeline = buildPipelineDefinition();
    project.activeDetectionModel = m_activeDetectionModel;
    project.activeDetectionModelPath = m_activeDetectionModelPath;
    project.activeDetectionModelPackageDir = m_activeDetectionModelPackageDir;
    project.appSettings = m_appSettings;
    project.recentMediaFiles = m_appSettings.recentMediaFiles;
    return project;
}

void MainWindow::applyProjectDefinition(const ProjectDefinition& project)
{
    m_appSettings = project.appSettings;
    m_appSettings.recentMediaFiles = project.recentMediaFiles;
    AppSettingsStore::save(m_appSettings);
    ApplicationContext::instance().setAppSettings(m_appSettings);
    setLanguage(m_appSettings.languageCode);

    m_activeDetectionModel = project.activeDetectionModel;
    m_activeDetectionModelPath = project.activeDetectionModelPath;
    m_activeDetectionModelPackageDir = project.activeDetectionModelPackageDir;
    m_activeDetectionLabels.clear();
    if (!m_activeDetectionModelPackageDir.isEmpty()) {
        const QString labelsPath = QDir(m_activeDetectionModelPackageDir).filePath("labels.txt");
        const LabelLoadResult labels = LabelProvider::loadLabels(labelsPath);
        if (labels.success) {
            m_activeDetectionLabels = labels.labels;
        }
    } else if (!m_activeDetectionModelPath.isEmpty()) {
        const QString labelsPath = QFileInfo(m_activeDetectionModelPath).absolutePath() + "/labels.txt";
        const LabelLoadResult labels = LabelProvider::loadLabels(labelsPath);
        if (labels.success) {
            m_activeDetectionLabels = labels.labels;
        }
    }

    if (!project.activeDetectionModel.modelName.isEmpty()) {
        ApplicationContext::instance().modelRegistry().setActiveModelName(project.activeDetectionModel.modelName);
    }

    if (!applyPipelineDefinition(project.pipeline, project.projectId)) {
        return;
    }

    rebuildRecentMediaMenu();

    if (!project.mediaSourcePath.isEmpty()) {
        loadMediaFromPath(
            project.mediaSourcePath,
            mediaSourceKindFromString(project.mediaSourceKind),
            project.currentVideoFrameIndex
        );
    } else {
        rerunPreview();
    }
}

void MainWindow::saveProjectToFile()
{
    QString filePath = m_currentProjectFilePath;
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getSaveFileName(
            this,
            localizedText(m_appSettings.languageCode, "Save Project", QStringLiteral("保存项目")),
            QString(),
            "CVVerify Project (*.cvproj.json)"
        );
    }
    if (filePath.isEmpty()) {
        return;
    }

    QString errorMessage;
    if (!ProjectService::saveToFile(buildProjectDefinition(), filePath, &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    m_currentProjectFilePath = filePath;
    recordRecentProject(filePath);
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Saved project: %1").arg(filePath),
        QStringLiteral("已保存项目: %1").arg(filePath)
    ));
}

void MainWindow::loadProjectFromFile()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        localizedText(m_appSettings.languageCode, "Load Project", QStringLiteral("加载项目")),
        QString(),
        "CVVerify Project (*.cvproj.json)"
    );
    if (filePath.isEmpty()) {
        return;
    }

    loadProjectFromPath(filePath);
}

void MainWindow::loadProjectFromPath(const QString& filePath)
{
    const ProjectLoadResult result = ProjectService::loadFromFile(filePath);
    if (!result.success) {
        statusBar()->showMessage(result.errorMessage);
        return;
    }

    m_currentProjectFilePath = filePath;
    recordRecentProject(filePath);
    applyProjectDefinition(result.project);
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Loaded project: %1").arg(filePath),
        QStringLiteral("已加载项目: %1").arg(filePath)
    ));
}

void MainWindow::loadPipelineFromPath(const QString& filePath)
{
    const PipelineLoadResult result = PipelineJsonSerializer::loadFromFile(filePath);
    if (!result.success) {
        statusBar()->showMessage(result.errorMessage);
        return;
    }

    applyPipelineDefinition(result.definition, filePath);
}

void MainWindow::editActiveModelConfig()
{
    if (m_activeDetectionModel.modelName.isEmpty() && m_activeDetectionModelPath.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Import an ONNX model before editing model configuration",
            QStringLiteral("请先导入 ONNX 模型，再编辑模型配置")
        ));
        return;
    }

    ModelConfigEditorDialog dialog(this);
    dialog.setLanguage(m_appSettings.languageCode);
    dialog.setDescriptor(m_activeDetectionModel);
    if (m_currentFrame.hasOriginalFrame()) {
        dialog.setPreviewContext(m_activeDetectionModelPath, m_activeDetectionLabels, m_currentFrame.originalMat);
    }
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    m_activeDetectionModel = dialog.descriptor();
    if (!m_activeDetectionModelPackageDir.isEmpty()) {
        QString errorMessage;
        if (!ModelPackageLoader::saveDescriptor(m_activeDetectionModel, m_activeDetectionModelPackageDir, &errorMessage)) {
            statusBar()->showMessage(errorMessage);
            return;
        }
    }

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Updated model configuration: %1").arg(m_activeDetectionModel.modelName),
        QStringLiteral("已更新模型配置: %1").arg(m_activeDetectionModel.modelName)
    ));
    previewDetectionOnCurrentImage();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (!event->mimeData()->hasUrls()) {
        return;
    }

    for (const QUrl& url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            event->acceptProposedAction();
            return;
        }
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        return;
    }

    bool handled = false;
    for (const QUrl& url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }

        const QString localPath = url.toLocalFile();
        if (localPath.isEmpty()) {
            continue;
        }

        const QFileInfo fileInfo(localPath);
        if (fileInfo.isDir()) {
            loadImageFolder(localPath);
            handled = true;
            break;
        }

        if (isVideoFilePath(localPath)) {
            loadVideoFile(localPath);
            handled = true;
            break;
        }

        if (isOnnxModelPath(localPath)) {
            importOnnxModelWithOptionalPath(localPath);
            handled = true;
            break;
        }

        if (isImageFilePath(localPath)) {
            loadImageFile(localPath);
            handled = true;
            break;
        }

        if (fileInfo.suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) == 0) {
            QFile jsonFile(localPath);
            if (jsonFile.open(QIODevice::ReadOnly)) {
                QJsonParseError parseError;
                const QJsonDocument document = QJsonDocument::fromJson(jsonFile.readAll(), &parseError);
                if (parseError.error == QJsonParseError::NoError && document.isObject()) {
                    const QJsonObject root = document.object();
                    if (root.contains(QStringLiteral("projectId"))) {
                        loadProjectFromPath(localPath);
                        handled = true;
                        break;
                    }
                    if (root.contains(QStringLiteral("steps"))) {
                        loadPipelineFromPath(localPath);
                        handled = true;
                        break;
                    }
                }
            }
        }
    }

    if (handled) {
        event->acceptProposedAction();
        return;
    }

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        "Unsupported dropped file type",
        QStringLiteral("不支持的拖放文件类型")
    ));
}

void MainWindow::requestVideoFrame(int frameIndex)
{
    if (m_currentMediaInfo.sourceId.isEmpty()) {
        return;
    }

    if (m_currentMediaInfo.kind == MediaSourceKind::VideoFile) {
        VideoFileSource source(m_currentMediaInfo.sourceId);
        FrameReadResult result = source.read({frameIndex});
        if (!result.success) {
            statusBar()->showMessage(result.errorMessage);
            return;
        }

        m_currentFrame = result.frame;
        syncWorkspaceCachedImageArtifact();
        m_currentVideoFrameIndex = frameIndex;
        m_mediaPanel->setVideoNavigationState(true, m_currentMediaInfo.frameCount, frameIndex);
        if (m_timelineWidget) {
            m_timelineWidget->setRange(0, std::max(0, m_currentMediaInfo.frameCount - 1));
            m_timelineWidget->updateFrameStatus(frameIndex, m_currentMediaInfo.frameCount);
        }
        rerunPreview();
        return;
    }

    if (m_currentMediaInfo.kind == MediaSourceKind::ImageFolder) {
        ImageFolderSource source(m_currentMediaInfo.sourceId);
        FrameReadResult result = source.read({frameIndex});
        if (!result.success) {
            statusBar()->showMessage(result.errorMessage);
            return;
        }

        m_currentFrame = result.frame;
        syncWorkspaceCachedImageArtifact();
        m_currentVideoFrameIndex = frameIndex;
        m_mediaPanel->setVideoNavigationState(true, m_currentMediaInfo.frameCount, frameIndex);
        if (m_timelineWidget) {
            m_timelineWidget->setRange(0, std::max(0, m_currentMediaInfo.frameCount - 1));
            m_timelineWidget->updateFrameStatus(frameIndex, m_currentMediaInfo.frameCount);
        }
        rerunPreview();
    }
}

void MainWindow::requestPreviousVideoFrame()
{
    if (m_currentMediaInfo.kind != MediaSourceKind::VideoFile
        && m_currentMediaInfo.kind != MediaSourceKind::ImageFolder) {
        return;
    }
    if (m_currentVideoFrameIndex <= 0) {
        return;
    }

    requestVideoFrame(m_currentVideoFrameIndex - 1);
}

void MainWindow::requestNextVideoFrame()
{
    if (m_currentMediaInfo.kind != MediaSourceKind::VideoFile
        && m_currentMediaInfo.kind != MediaSourceKind::ImageFolder) {
        return;
    }
    if (m_currentVideoFrameIndex >= std::max(0, m_currentMediaInfo.frameCount - 1)) {
        return;
    }

    requestVideoFrame(m_currentVideoFrameIndex + 1);
}

void MainWindow::toggleVideoPlayback(bool playing)
{
    if (m_currentMediaInfo.kind != MediaSourceKind::VideoFile
        && m_currentMediaInfo.kind != MediaSourceKind::ImageFolder) {
        m_mediaPanel->setVideoPlaybackState(false);
        rebuildMenuTexts();
        return;
    }

    if (m_currentMediaInfo.kind == MediaSourceKind::ImageFolder) {
        if (!playing) {
            m_videoPlaybackTimer->stop();
            m_mediaPanel->setVideoPlaybackState(false);
            rebuildMenuTexts();
            return;
        }

        m_videoPlaybackTimer->start(500);
        m_mediaPanel->setVideoPlaybackState(true);
        rebuildMenuTexts();
        return;
    }

    if (!playing) {
        m_videoPlaybackTimer->stop();
        m_mediaPanel->setVideoPlaybackState(false);
        rebuildMenuTexts();
        return;
    }

    const int nativeIntervalMs = m_currentMediaInfo.fps > 0.0
        ? std::max(1, static_cast<int>(1000.0 / m_currentMediaInfo.fps))
        : 40;
    const int cappedIntervalMs = m_maxPreviewFps > 0
        ? std::max(nativeIntervalMs, 1000 / m_maxPreviewFps)
        : nativeIntervalMs;

    m_videoPlaybackTimer->start(cappedIntervalMs);
    m_mediaPanel->setVideoPlaybackState(true);
    rebuildMenuTexts();
}

void MainWindow::advanceVideoPlayback()
{
    if (m_currentMediaInfo.kind != MediaSourceKind::VideoFile
        && m_currentMediaInfo.kind != MediaSourceKind::ImageFolder) {
        m_videoPlaybackTimer->stop();
        m_mediaPanel->setVideoPlaybackState(false);
        rebuildMenuTexts();
        return;
    }

    const int frameStep = m_appSettings.allowFrameSkip ? qMax(1, m_previewFrameStep) : 1;
    const int nextFrame = m_currentVideoFrameIndex + frameStep;
    if (nextFrame >= std::max(1, m_currentMediaInfo.frameCount)) {
        m_videoPlaybackTimer->stop();
        m_mediaPanel->setVideoPlaybackState(false);
        rebuildMenuTexts();
        return;
    }

    requestVideoFrame(nextFrame);
}

void MainWindow::importYoloModel()
{
    importOnnxModelWithOptionalPath(QString());
}

void MainWindow::importOnnxModelWithOptionalPath(const QString& modelPath)
{
    YoloModelImportDialog dialog(this);
    dialog.setLanguage(m_appSettings.languageCode);
    if (!modelPath.isEmpty()) {
        dialog.setInitialModelPath(modelPath);
    }
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    ModelImportRequest request = dialog.buildImportRequest();
    if (request.modelName.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Model name is required",
            QStringLiteral("模型名称不能为空")
        ));
        return;
    }

    if (request.modelPath.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Model path is required",
            QStringLiteral("模型路径不能为空")
        ));
        return;
    }

    ModelImportResult importResult = ModelImportService::importModel(request);
    if (!importResult.success) {
        statusBar()->showMessage(importResult.errorMessage);
        return;
    }

    const QString selectedTemplate = dialog.selectedTemplateName();
    if (!selectedTemplate.isEmpty()) {
        DetectionModelDescriptor templateDescriptor;
        QString templateError;
        if (ModelTemplateLoader::loadTemplate(selectedTemplate, &templateDescriptor, &templateError)) {
            ModelTemplateLoader::applyTemplateDefaults(importResult.descriptor, templateDescriptor);
            importResult.descriptor.modelName = request.modelName;
        } else if (!templateError.isEmpty()) {
            statusBar()->showMessage(templateError);
            return;
        }
    }

    const QString packageDir = QDir("models").filePath(request.modelName);
    QString errorMessage;
    if (!ModelPackageLoader::saveDescriptor(importResult.descriptor, packageDir, &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    const QString packagedModelPath = QDir(packageDir).filePath("model.onnx");
    const QString packagedLabelsPath = QDir(packageDir).filePath("labels.txt");

    if (!copyImportedFile(request.modelPath, packagedModelPath, &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }
    if (!request.labelsPath.isEmpty()) {
        if (!copyImportedFile(request.labelsPath, packagedLabelsPath, &errorMessage)) {
            statusBar()->showMessage(errorMessage);
            return;
        }
    }

    QStringList labels;
    if (QFileInfo::exists(packagedLabelsPath)) {
        const LabelLoadResult labelResult = LabelProvider::loadLabels(packagedLabelsPath);
        if (!labelResult.success) {
            statusBar()->showMessage(labelResult.errorMessage);
            return;
        }
        labels = labelResult.labels;
    }

    m_activeDetectionModel = importResult.descriptor;
    m_activeDetectionModelPath = QFileInfo(packagedModelPath).absoluteFilePath();
    m_activeDetectionModelPackageDir = QFileInfo(packageDir).absoluteFilePath();
    m_activeDetectionLabels = labels;

    RegisteredModelPackage registeredPackage;
    registeredPackage.packageDir = m_activeDetectionModelPackageDir;
    registeredPackage.modelPath = m_activeDetectionModelPath;
    registeredPackage.descriptor = m_activeDetectionModel;
    registeredPackage.labels = m_activeDetectionLabels;
    ApplicationContext::instance().modelRegistry().registerPackage(registeredPackage);
    ApplicationContext::instance().modelRegistry().setActiveModelName(m_activeDetectionModel.modelName);
    ModelImportService::writePreviewPlaceholder(m_activeDetectionModelPackageDir, &errorMessage);
    if (m_modelListPanel) {
        m_modelListPanel->refreshModels();
    }

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Imported ONNX model: %1 (%2)").arg(request.modelName, m_activeDetectionModel.taskType),
        QStringLiteral("已导入 ONNX 模型: %1 (%2)").arg(request.modelName, m_activeDetectionModel.taskType)
    ));
    previewDetectionOnCurrentImage();
}

void MainWindow::showAboutDialog()
{
    const DnnRuntimeInfo::BackendInfo backendInfo = DnnRuntimeInfo::queryBackendInfo();
    const QString aboutText = localizedText(
        m_appSettings.languageCode,
        QString(
            "%1\n\n"
            "Desktop validation workstation for OpenCV pipelines and ONNX inference.\n\n"
            "OpenCV: %2\n"
            "Qt: %3\n"
            "DNN backends: %4")
            .arg(
                AppVersion::displayName(),
                backendInfo.opencvVersion,
                QString::fromLatin1(qVersion()),
                backendInfo.availableBackends.join(", ")),
        QString(
            "%1\n\n"
            "面向 OpenCV 流程与 ONNX 推理的桌面验证工作站。\n\n"
            "OpenCV: %2\n"
            "Qt: %3\n"
            "DNN 后端: %4")
            .arg(
                AppVersion::displayName(),
                backendInfo.opencvVersion,
                QString::fromLatin1(qVersion()),
                backendInfo.availableBackends.join(", "))
    );

    QMessageBox::about(
        this,
        localizedText(m_appSettings.languageCode, "About CVVerify", QStringLiteral("关于 CVVerify")),
        aboutText);
}

void MainWindow::exportPipelineSnapshot()
{
    if (m_pipelineSteps.empty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Pipeline is empty",
            QStringLiteral("流程为空")
        ));
        return;
    }

    const QString initialDirectory = m_appSettings.defaultExportDirectory.isEmpty()
        ? QDir::currentPath()
        : m_appSettings.defaultExportDirectory;
    const QString outputDir = QFileDialog::getExistingDirectory(
        this,
        localizedText(m_appSettings.languageCode, "Export Pipeline Snapshot", QStringLiteral("导出流程快照")),
        initialDirectory
    );
    if (outputDir.isEmpty()) {
        return;
    }

    const QString filePath = QDir(outputDir).filePath(
        QStringLiteral("pipeline_snapshot_%1.json")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"))));
    QString errorMessage;
    if (!PipelineJsonSerializer::saveToFile(buildPipelineDefinition(), filePath, &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Exported pipeline snapshot: %1").arg(filePath),
        QStringLiteral("已导出流程快照: %1").arg(filePath)
    ));
}

void MainWindow::exportAppConfigSnapshot()
{
    const QString initialDirectory = m_appSettings.defaultExportDirectory.isEmpty()
        ? QDir::currentPath()
        : m_appSettings.defaultExportDirectory;
    const QString outputDir = QFileDialog::getExistingDirectory(
        this,
        localizedText(m_appSettings.languageCode, "Export App Config Snapshot", QStringLiteral("导出应用配置快照")),
        initialDirectory
    );
    if (outputDir.isEmpty()) {
        return;
    }

    QJsonObject preview;
    preview.insert("maxFps", m_appSettings.maxPreviewFps);
    preview.insert("allowFrameSkip", m_appSettings.allowFrameSkip);
    preview.insert("previewScale", m_appSettings.previewScale);
    preview.insert("frameStep", m_appSettings.previewFrameStep);

    QJsonObject task;
    task.insert("offlineVideoStartFrame", m_appSettings.offlineVideoStartFrame);
    task.insert("offlineVideoEndFrame", m_appSettings.offlineVideoEndFrame);

    QJsonObject dnn;
    dnn.insert("backend", m_appSettings.dnnBackend);

    QJsonObject exportObject;
    exportObject.insert("videoSideBySide", m_appSettings.exportVideoSideBySide);

    QJsonObject ui;
    ui.insert("showWelcomeGuide", m_appSettings.showWelcomeGuide);

    QJsonArray recentMedia;
    for (const QString& path : m_appSettings.recentMediaFiles) {
        recentMedia.append(path);
    }

    QJsonArray recentProjects;
    for (const QString& path : m_appSettings.recentProjectFiles) {
        recentProjects.append(path);
    }

    QJsonObject root;
    root.insert("appVersion", AppVersion::applicationVersion());
    root.insert("exportedAt", QDateTime::currentDateTime().toString(Qt::ISODate));
    root.insert("languageCode", m_appSettings.languageCode);
    root.insert("defaultExportDirectory", m_appSettings.defaultExportDirectory);
    root.insert("autoOpenExportDirectory", m_appSettings.autoOpenExportDirectory);
    root.insert("maxRecentMediaFiles", m_appSettings.maxRecentMediaFiles);
    root.insert("recentMediaFiles", recentMedia);
    root.insert("recentProjectFiles", recentProjects);
    root.insert("preview", preview);
    root.insert("task", task);
    root.insert("dnn", dnn);
    root.insert("export", exportObject);
    root.insert("ui", ui);

    const QString filePath = QDir(outputDir).filePath(
        QStringLiteral("app_config_snapshot_%1.json")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss"))));
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            QString("Failed to write app config snapshot: %1").arg(filePath),
            QStringLiteral("写入应用配置快照失败: %1").arg(filePath)
        ));
        return;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Exported app config snapshot: %1").arg(filePath),
        QStringLiteral("已导出应用配置快照: %1").arg(filePath)
    ));
}

void MainWindow::showWelcomeGuideIfNeeded()
{
    if (!m_appSettings.showWelcomeGuide) {
        return;
    }

    m_appSettings.showWelcomeGuide = false;
    AppSettingsStore::save(m_appSettings);
    presentQuickStartGuideDialog();
}

void MainWindow::presentQuickStartGuideDialog()
{
    QDialog dialog(this);
    dialog.setObjectName("quickStartGuideDialog");
    dialog.setWindowTitle(localizedText(
        m_appSettings.languageCode,
        "Quick Start",
        QStringLiteral("\u5feb\u901f\u5165\u95e8")
    ));
    dialog.resize(560, 480);

    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* guideText = new QTextBrowser(&dialog);
    guideText->setObjectName("quickStartGuideText");
    guideText->setOpenExternalLinks(false);
    guideText->setReadOnly(true);
    guideText->setPlainText(quickStartGuideText(m_appSettings.languageCode));
    layout->addWidget(guideText, 1);

    auto* buttons = new QDialogButtonBox(&dialog);
    QPushButton* demoButton = buttons->addButton(
        localizedText(m_appSettings.languageCode, "Load Demo Project", QStringLiteral("\u52a0\u8f7d\u6f14\u793a\u9879\u76ee")),
        QDialogButtonBox::ActionRole);
    buttons->addButton(QDialogButtonBox::Close);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::accept);
    connect(demoButton, &QPushButton::clicked, &dialog, [this, &dialog] {
        loadSampleProject(QStringLiteral("samples/projects/demo_project.json"));
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::addOperatorToPipeline(const QString& operatorId)
{
    if (!ensureMediaSelectedForOperatorAdd()) {
        return;
    }

    auto step = ApplicationContext::instance().operatorRegistry().create(operatorId);
    if (!step) {
        statusBar()->showMessage(QString("Failed to create operator: %1").arg(operatorId));
        return;
    }

    const QString displayName = localizedOperatorDisplayName(
        operatorId,
        step->displayName(),
        m_appSettings.languageCode
    );
    m_pipelineSteps.push_back({displayName, step});
    m_selectedPipelineStepIndex = static_cast<int>(m_pipelineSteps.size()) - 1;
    m_pipelinePanel->setPipelineStepNames(pipelineStepNames());
    m_pipelinePanel->setCurrentPipelineStep(m_selectedPipelineStepIndex);
    m_parameterPanel->setStep(m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step);
    setPreviewMode(PreviewMode::FullPipeline);
    rebuildPipelineStrip();
    updateEmbeddedWorkbenchState();
    QTimer::singleShot(0, this, [this] { rerunPreview(); });
}

bool MainWindow::hasSelectedMedia() const
{
    return !m_currentMediaInfo.sourceId.isEmpty() && m_currentFrame.hasOriginalFrame();
}

bool MainWindow::ensureMediaSelectedForOperatorAdd()
{
    if (hasSelectedMedia()) {
        return true;
    }

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        "Please select an image or video first",
        QStringLiteral("\u8bf7\u5148\u9009\u62e9\u56fe\u7247\u6216\u89c6\u9891")
    ));
    return false;
}

void MainWindow::cacheCurrentImageForWorkspaceUse(const QString& nextSourceId)
{
    if (m_currentMediaInfo.kind != MediaSourceKind::ImageFile) {
        return;
    }

    const QString currentPath = m_currentMediaInfo.sourceId.trimmed();
    if (currentPath.isEmpty()) {
        return;
    }

    if (QFileInfo(currentPath).absoluteFilePath() == QFileInfo(nextSourceId).absoluteFilePath()) {
        return;
    }

    m_workspaceCachedImagePath = currentPath;
}

void MainWindow::syncWorkspaceCachedImageArtifact()
{
    const QString cachedPath = m_workspaceCachedImagePath.trimmed();
    if (cachedPath.isEmpty()) {
        m_currentFrame.artifacts.remove("workspace_cached_image_path");
        return;
    }

    m_currentFrame.artifacts.insert("workspace_cached_image_path", cachedPath);
}

void MainWindow::updateOperatorParameters(const QVariantMap& values)
{
    if (m_selectedPipelineStepIndex < 0 || m_selectedPipelineStepIndex >= static_cast<int>(m_pipelineSteps.size())) {
        return;
    }

    m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step->setParameterValues(values);
    rebuildPipelineStrip();
    rerunPreview();
}

void MainWindow::handleSourceCanvasPointClicked(const QPointF& imagePoint)
{
    if (currentStepUsesPerspectiveInteraction()) {
        if (m_perspectivePoints.size() >= 4) {
            m_perspectivePoints.clear();
        }

        m_perspectivePoints.append(imagePoint);
        syncPerspectivePointsToCurrentFrameArtifacts();
        refreshSourceCanvasOverlay();
        if (m_perspectivePoints.size() >= 4) {
            rerunPreview();
        }
        return;
    }

    if (currentStepUsesAffineInteraction()) {
        if (m_affinePoints.size() >= 3) {
            m_affinePoints.clear();
        }

        m_affinePoints.append(imagePoint);
        syncAffinePointsToCurrentFrameArtifacts();
        refreshSourceCanvasOverlay();
        if (m_affinePoints.size() >= 3) {
            rerunPreview();
        }
        return;
    }

    if (currentStepUsesFloodFillInteraction()) {
        m_floodFillSeeds.append(imagePoint);
        syncFloodFillSeedsToCurrentFrameArtifacts();
        refreshSourceCanvasOverlay();
        rerunPreview();
        return;
    }

    if (currentStepUsesGrabCutInteraction()) {
        if (m_grabCutRectPoints.size() >= 2) {
            m_grabCutRectPoints.clear();
        }

        m_grabCutRectPoints.append(imagePoint);
        syncGrabCutRectToCurrentFrameArtifacts();
        refreshSourceCanvasOverlay();
        if (m_grabCutRectPoints.size() >= 2) {
            rerunPreview();
        }
    }
}

bool MainWindow::currentStepUsesPerspectiveInteraction() const
{
    if (m_selectedPipelineStepIndex < 0 || m_selectedPipelineStepIndex >= static_cast<int>(m_pipelineSteps.size())) {
        return false;
    }

    const auto& step = m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step;
    return step && step->id() == "builtin.perspective_transform";
}

bool MainWindow::currentStepUsesAffineInteraction() const
{
    if (m_selectedPipelineStepIndex < 0 || m_selectedPipelineStepIndex >= static_cast<int>(m_pipelineSteps.size())) {
        return false;
    }

    const auto& step = m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step;
    return step && step->id() == "builtin.affine_transform";
}

bool MainWindow::currentStepUsesFloodFillInteraction() const
{
    if (m_selectedPipelineStepIndex < 0 || m_selectedPipelineStepIndex >= static_cast<int>(m_pipelineSteps.size())) {
        return false;
    }

    const auto& step = m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step;
    return step && step->id() == "builtin.flood_fill";
}

bool MainWindow::currentStepUsesGrabCutInteraction() const
{
    if (m_selectedPipelineStepIndex < 0 || m_selectedPipelineStepIndex >= static_cast<int>(m_pipelineSteps.size())) {
        return false;
    }

    const auto& step = m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step;
    return step && step->id() == "builtin.grabcut";
}

void MainWindow::syncPerspectivePointsToCurrentFrameArtifacts()
{
    QVariantList pointList;
    for (const QPointF& point : m_perspectivePoints) {
        QVariantMap map;
        map.insert("x", point.x());
        map.insert("y", point.y());
        pointList.append(map);
    }

    m_currentFrame.artifacts.insert("perspective_points", pointList);
}

void MainWindow::syncAffinePointsToCurrentFrameArtifacts()
{
    QVariantList pointList;
    for (const QPointF& point : m_affinePoints) {
        QVariantMap map;
        map.insert("x", point.x());
        map.insert("y", point.y());
        pointList.append(map);
    }

    m_currentFrame.artifacts.insert("affine_points", pointList);
}

void MainWindow::syncFloodFillSeedsToCurrentFrameArtifacts()
{
    QVariantList seedList;
    for (const QPointF& point : m_floodFillSeeds) {
        QVariantMap map;
        map.insert("x", point.x());
        map.insert("y", point.y());
        seedList.append(map);
    }

    m_currentFrame.artifacts.insert("flood_fill_seeds", seedList);
}

void MainWindow::syncGrabCutRectToCurrentFrameArtifacts()
{
    if (m_grabCutRectPoints.size() < 2) {
        m_currentFrame.artifacts.remove("grabcut_rect");
        return;
    }

    const QPointF first = m_grabCutRectPoints.at(0);
    const QPointF second = m_grabCutRectPoints.at(1);
    const double x = std::min(first.x(), second.x());
    const double y = std::min(first.y(), second.y());
    const double width = std::abs(first.x() - second.x());
    const double height = std::abs(first.y() - second.y());

    QVariantMap rectMap;
    rectMap.insert("x", x);
    rectMap.insert("y", y);
    rectMap.insert("width", width);
    rectMap.insert("height", height);
    m_currentFrame.artifacts.insert("grabcut_rect", rectMap);
}

void MainWindow::refreshSourceCanvasOverlay()
{
    if (m_sourceCanvasView) {
        m_sourceCanvasView->setOverlayRectangle({}, false);
        if (currentStepUsesPerspectiveInteraction()) {
            m_sourceCanvasView->setOverlayPoints(m_perspectivePoints);
        } else if (currentStepUsesAffineInteraction()) {
            m_sourceCanvasView->setOverlayPoints(m_affinePoints);
        } else if (currentStepUsesFloodFillInteraction()) {
            m_sourceCanvasView->setOverlayPoints(m_floodFillSeeds);
        } else if (currentStepUsesGrabCutInteraction()) {
            m_sourceCanvasView->setOverlayPoints({});
            if (m_grabCutRectPoints.size() >= 2) {
                const QPointF first = m_grabCutRectPoints.at(0);
                const QPointF second = m_grabCutRectPoints.at(1);
                const QRectF rectangle(
                    QPointF(std::min(first.x(), second.x()), std::min(first.y(), second.y())),
                    QPointF(std::max(first.x(), second.x()), std::max(first.y(), second.y()))
                );
                m_sourceCanvasView->setOverlayRectangle(rectangle, true);
            } else if (m_grabCutRectPoints.size() == 1) {
                m_sourceCanvasView->setOverlayPoints(m_grabCutRectPoints);
            }
        } else {
            m_sourceCanvasView->setOverlayPoints({});
        }
    }
}

void MainWindow::rerunPreview()
{
    {
        if (!m_currentFrame.hasOriginalFrame()) {
            if (m_sourceCanvasView) {
                m_sourceCanvasView->setImage(QImage());
                m_sourceCanvasView->setOverlayPoints({});
            }
            if (m_canvasView) {
                m_canvasView->setImage(QImage());
            }
            if (m_previewInfoLabel) {
                m_previewInfoLabel->setText(localizedText(
                    m_appSettings.languageCode,
                    "Load an image or video to start previewing.",
                    QStringLiteral("\u52a0\u8f7d\u56fe\u7247\u6216\u89c6\u9891\u540e\u5373\u53ef\u5f00\u59cb\u9884\u89c8\u3002")
                ));
            }
            if (m_previewDetailsLabel) {
                m_previewDetailsLabel->setText(pipelineDetailsSummary());
            }
            return;
        }

        FramePacket preview = m_currentFrame;
        preview.workingMat = preview.originalMat.clone();
        preview.sidecarMat.release();
        preview.sidecarKind.clear();
        if (m_previewScale < 0.99) {
            preview.workingMat = applyPreviewScale(preview.workingMat, m_previewScale);
        }

        if (m_sourceCanvasView) {
            m_sourceCanvasView->setImage(OpenCvQtImageConverter::toQImage(m_currentFrame.originalMat));
            refreshSourceCanvasOverlay();
        }
        if (m_previewCanvasHost) {
            m_previewCanvasHost->setSourceImage(OpenCvQtImageConverter::toQImage(m_currentFrame.originalMat));
        }

        DetectionFrameResult frameResult;
        QString errorMessage;

        if (m_previewMode == PreviewMode::SingleNode) {
            std::shared_ptr<IPipelineStep> previewStep;
            if (m_selectedPipelineStepIndex >= 0
                && m_selectedPipelineStepIndex < static_cast<int>(m_pipelineSteps.size())) {
                previewStep = m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step;
            } else {
                previewStep = m_singlePreviewStep;
            }

            if (previewStep) {
                const StepResult singleStepResult = previewStep->execute(preview, RunContext{true});
                if (!singleStepResult.success) {
                    statusBar()->showMessage(singleStepResult.errorMessage);
                    return;
                }
            }

            if (!m_activeDetectionModelPath.isEmpty()) {
                frameResult.sourceId = preview.sourceId;
                frameResult.frameId = preview.frameId;
                frameResult.timestampMs = preview.timestampMs;
                if (!runDetectionOnCurrentFrame(&preview, &frameResult, &errorMessage)) {
                    statusBar()->showMessage(errorMessage);
                    return;
                }
            }
        } else if (!processFrame(&preview, &frameResult, &errorMessage)) {
            statusBar()->showMessage(errorMessage);
            return;
        }

        if (!frameResult.boxes.isEmpty() || preview.artifacts.contains("classification_top_k")) {
            DetectionRenderComposer::applyModelArtifactsOverlay(
                preview.workingMat,
                frameResult,
                preview.artifacts);
        }

        const QVariant detectionCountVariant = preview.annotations.value("detections");
        const int detectionCount = detectionCountVariant.isValid()
            ? detectionCountVariant.toInt()
            : frameResult.boxes.size();
        const QString modelSummary = modelResultSummary(preview, frameResult);

        if (detectionCountVariant.isValid()) {
            statusBar()->showMessage(localizedText(
                m_appSettings.languageCode,
                QString("Detection preview complete: %1 boxes").arg(detectionCount),
                QStringLiteral("\u68c0\u6d4b\u9884\u89c8\u5b8c\u6210\uff1a%1 \u4e2a\u6846").arg(detectionCount)
            ));
        } else if (!modelSummary.isEmpty()) {
            statusBar()->showMessage(modelSummary);
        } else if (m_currentMediaInfo.kind == MediaSourceKind::VideoFile
            || m_currentMediaInfo.kind == MediaSourceKind::ImageFolder) {
            statusBar()->showMessage(localizedText(
                m_appSettings.languageCode,
                QString("Previewing frame %1 / %2").arg(m_currentVideoFrameIndex + 1).arg(std::max(1, m_currentMediaInfo.frameCount)),
                QStringLiteral("\u6b63\u5728\u9884\u89c8\u5e27 %1 / %2").arg(m_currentVideoFrameIndex + 1).arg(std::max(1, m_currentMediaInfo.frameCount))
            ));
        }

        const QString mediaName = QFileInfo(m_currentFrame.sourceId).fileName();
        QString mediaKind;
        if (m_currentMediaInfo.kind == MediaSourceKind::VideoFile) {
            mediaKind = localizedText(m_appSettings.languageCode, "Video", QStringLiteral("\u89c6\u9891"));
        } else if (m_currentMediaInfo.kind == MediaSourceKind::ImageFolder) {
            mediaKind = localizedText(m_appSettings.languageCode, "Image Folder", QStringLiteral("\u56fe\u7247\u6587\u4ef6\u5939"));
        } else {
            mediaKind = localizedText(m_appSettings.languageCode, "Image", QStringLiteral("\u56fe\u7247"));
        }
        const QString framePart = (m_currentMediaInfo.kind == MediaSourceKind::VideoFile
            || m_currentMediaInfo.kind == MediaSourceKind::ImageFolder)
            ? localizedText(
                m_appSettings.languageCode,
                QString("Frame %1 / %2").arg(m_currentVideoFrameIndex + 1).arg(std::max(1, m_currentMediaInfo.frameCount)),
                QStringLiteral("\u5e27 %1 / %2").arg(m_currentVideoFrameIndex + 1).arg(std::max(1, m_currentMediaInfo.frameCount)))
            : localizedText(m_appSettings.languageCode, "Single Frame", QStringLiteral("\u5355\u5e27"));
        const QString detectionPart = modelSummary.isEmpty()
            ? localizedText(
                m_appSettings.languageCode,
                QString("Detections: %1").arg(detectionCount),
                QStringLiteral("\u68c0\u6d4b\u6570\uff1a%1").arg(detectionCount))
            : modelSummary;
        const QString stepPart = localizedText(
            m_appSettings.languageCode,
            QString("Pipeline Steps: %1").arg(m_pipelineSteps.size()),
            QStringLiteral("\u6d41\u7a0b\u6b65\u9aa4\uff1a%1").arg(m_pipelineSteps.size())
        );

        if (m_previewInfoLabel) {
            m_previewInfoLabel->setText(QString("%1 | %2 | %3 | %4 | %5 | %6")
                .arg(mediaKind)
                .arg(mediaName.isEmpty()
                    ? localizedText(m_appSettings.languageCode, "Unnamed Source", QStringLiteral("\u672a\u547d\u540d\u6e90"))
                    : mediaName)
                .arg(framePart)
                .arg(previewModeSummary())
                .arg(stepPart)
                .arg(detectionPart));
        }

        if (m_previewDetailsLabel) {
            m_previewDetailsLabel->setText(pipelineDetailsSummary());
        }

        if (m_canvasView) {
            m_canvasView->setImage(OpenCvQtImageConverter::toQImage(preview.workingMat));
        }
        if (m_previewCanvasHost) {
            const QImage sourceImage = OpenCvQtImageConverter::toQImage(m_currentFrame.originalMat);
            const QImage resultImage = OpenCvQtImageConverter::toQImage(preview.workingMat);
            if (m_comparisonBaselineActive
                && !m_comparisonBaselineMat.empty()
                && m_previewViewMode == PreviewViewMode::Wipe) {
                m_previewCanvasHost->setSourceImage(OpenCvQtImageConverter::toQImage(m_comparisonBaselineMat));
                m_previewCanvasHost->setResultImage(resultImage);
            } else {
                m_previewCanvasHost->setSourceImage(sourceImage);
                m_previewCanvasHost->setResultImage(resultImage);
            }
            if (!frameResult.boxes.isEmpty()) {
                cv::Mat overlayMat = preview.workingMat.clone();
                DetectionRenderComposer::applyModelArtifactsOverlay(
                    overlayMat,
                    frameResult,
                    preview.artifacts);
                m_previewCanvasHost->setOverlayImage(OpenCvQtImageConverter::toQImage(overlayMat));
            } else if (preview.hasSidecarFrame() && preview.sidecarKind == "histogram") {
                m_previewCanvasHost->setOverlayImage(OpenCvQtImageConverter::toQImage(preview.sidecarMat));
            } else {
                m_previewCanvasHost->setOverlayImage(QImage());
            }
            if (m_comparisonBaselineActive
                && !m_comparisonBaselineMat.empty()
                && m_previewViewMode == PreviewViewMode::FourGrid) {
                m_previewCanvasHost->setIntermediateImage(OpenCvQtImageConverter::toQImage(m_comparisonBaselineMat));
            } else if (preview.hasSidecarFrame()) {
                m_previewCanvasHost->setIntermediateImage(OpenCvQtImageConverter::toQImage(preview.sidecarMat));
            } else {
                m_previewCanvasHost->setIntermediateImage(resultImage);
            }
        }
        if (m_statisticsPanel) {
            m_statisticsPanel->setFrameMetrics(preview.metrics);
            m_statisticsPanel->setTimingSummary(DiagnosticsLog::instance().summaryText());
        }
        if (m_diagnosticsPanel) {
            m_diagnosticsPanel->refresh();
        }
        if (m_statusDetailsLabel) {
            m_statusDetailsLabel->setText(localizedText(
                m_appSettings.languageCode,
                QString("Size: %1x%2 | Frame: %3/%4")
                    .arg(m_currentMediaInfo.width)
                    .arg(m_currentMediaInfo.height)
                    .arg(m_currentVideoFrameIndex + 1)
                    .arg(std::max(1, m_currentMediaInfo.frameCount)),
                QStringLiteral("\u5c3a\u5bf8: %1x%2 | \u5e27: %3/%4")
                    .arg(m_currentMediaInfo.width)
                    .arg(m_currentMediaInfo.height)
                    .arg(m_currentVideoFrameIndex + 1)
                    .arg(std::max(1, m_currentMediaInfo.frameCount))
            ));
        }
        return;
    }
}

void MainWindow::showMediaWorkbench()
{
    presentDialog(m_mediaWorkbenchDialog);
}

void MainWindow::showPipelineWorkbench()
{
    setOperatorWorkbenchVisible(true);
    if (m_operatorWorkbenchPanel) {
        m_operatorWorkbenchPanel->setFocus(Qt::OtherFocusReason);
    }
    if (m_pipelineStripList) {
        m_pipelineStripList->setFocus(Qt::OtherFocusReason);
    }
    raise();
    activateWindow();
}

void MainWindow::setOperatorWorkbenchVisible(bool visible)
{
    if (m_operatorWorkbenchPanel) {
        m_operatorWorkbenchPanel->setVisible(visible);
    }

    if (m_showPipelineWorkbenchAction && m_showPipelineWorkbenchAction->isChecked() != visible) {
        const QSignalBlocker blocker(m_showPipelineWorkbenchAction);
        m_showPipelineWorkbenchAction->setChecked(visible);
    }
}

void MainWindow::showExportResultsWorkbench()
{
    presentDialog(m_exportResultsDialog);
}

void MainWindow::showSystemSettings()
{
    SystemSettingsDialog dialog(this);
    dialog.setSettings(m_appSettings);
    dialog.setLanguage(m_appSettings.languageCode);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const AppSettings previousSettings = m_appSettings;
    m_appSettings = dialog.settings();
    m_appSettings.recentMediaFiles = previousSettings.recentMediaFiles;
    m_appSettings.recentProjectFiles = previousSettings.recentProjectFiles;
    if (dialog.clearRecentFilesRequested()) {
        m_appSettings.recentMediaFiles.clear();
        m_appSettings.recentProjectFiles.clear();
    }
    trimRecentFileLists();
    AppSettingsStore::save(m_appSettings);
    syncPreviewSettingsFromAppSettings();
    rebuildRecentMediaMenu();
    rebuildRecentProjectMenu();
    setLanguage(m_appSettings.languageCode);
}

void MainWindow::setLanguage(const QString& languageCode)
{
    {
        m_appSettings.languageCode = languageCode;

        setWindowTitle(localizedText(
            languageCode,
            "CVVerify - OpenCV Validation Platform",
            QStringLiteral("CVVerify - OpenCV \u9a8c\u8bc1\u5e73\u53f0")
        ));

        if (m_previewTitleLabel) {
            m_previewTitleLabel->setText(localizedText(
                languageCode,
                "Preview Status",
                QStringLiteral("\u9884\u89c8\u72b6\u6001")
            ));
        }
        if (m_sourcePreviewTitleLabel) {
            m_sourcePreviewTitleLabel->setText(localizedText(
                languageCode,
                "Source Image / Video",
                QStringLiteral("\u539f\u56fe / \u539f\u89c6\u9891")
            ));
        }
        if (m_resultPreviewTitleLabel) {
            m_resultPreviewTitleLabel->setText(localizedText(
                languageCode,
                "Processed Image / Video",
                QStringLiteral("\u5904\u7406\u540e\u56fe / \u89c6\u9891")
            ));
        }
        if (m_sourceCanvasView) {
            m_sourceCanvasView->setPlaceholderText(localizedText(
                languageCode,
                "Source Preview",
                QStringLiteral("\u539f\u59cb\u9884\u89c8")
            ));
        }
        if (m_canvasView) {
            m_canvasView->setPlaceholderText(localizedText(
                languageCode,
                "Processed Preview",
                QStringLiteral("\u5904\u7406\u7ed3\u679c\u9884\u89c8")
            ));
        }
        if (m_previewInfoLabel && m_currentFrame.sourceId.isEmpty()) {
            m_previewInfoLabel->setText(localizedText(
                languageCode,
                "Load an image or video to start previewing.",
                QStringLiteral("\u52a0\u8f7d\u56fe\u7247\u6216\u89c6\u9891\u540e\u5373\u53ef\u5f00\u59cb\u9884\u89c8\u3002")
            ));
        }
        if (m_previewDetailsLabel && m_currentFrame.sourceId.isEmpty()) {
            m_previewDetailsLabel->setText(pipelineDetailsSummary());
        }

        if (m_mediaPanel) {
            m_mediaPanel->setLanguage(languageCode);
        }
        if (m_pipelinePanel) {
            m_pipelinePanel->setLanguage(languageCode);
        }
        if (m_parameterPanel) {
            m_parameterPanel->setLanguage(languageCode);
        }
        if (m_exportResultsPanel) {
            m_exportResultsPanel->setLanguage(languageCode);
        }
        if (m_taskConsolePanel) {
            m_taskConsolePanel->setLanguage(languageCode);
        }
        if (m_timelineWidget) {
            m_timelineWidget->setLanguage(languageCode);
        }
        if (m_operatorParameterDialog) {
            m_operatorParameterDialog->setLanguage(languageCode);
        }
        if (m_mediaWorkbenchDialog) {
            m_mediaWorkbenchDialog->setWindowTitle(localizedText(
                languageCode,
                "Media & Model Workbench",
                QStringLiteral("\u5a92\u4f53\u4e0e\u6a21\u578b\u5de5\u4f5c\u53f0")
            ));
        }
        if (m_pipelineWorkbenchDialog) {
            m_pipelineWorkbenchDialog->setWindowTitle(localizedText(
                languageCode,
                "Pipeline Workbench",
                QStringLiteral("\u6d41\u7a0b\u5de5\u4f5c\u53f0")
            ));
        }
        if (m_exportResultsDialog) {
            m_exportResultsDialog->setWindowTitle(localizedText(
                languageCode,
                "Export Results",
                QStringLiteral("\u5bfc\u51fa\u7ed3\u679c")
            ));
        }

        if (m_englishLanguageAction) {
            m_englishLanguageAction->setChecked(!isChineseLanguage(languageCode));
        }
        if (m_chineseLanguageAction) {
            m_chineseLanguageAction->setChecked(isChineseLanguage(languageCode));
        }

        rebuildMenuTexts();
        rebuildEmbeddedWorkbenchTexts();

        if (m_currentFrame.hasOriginalFrame()) {
            rerunPreview();
        } else {
            statusBar()->showMessage(localizedText(
                languageCode,
                "Ready: media and operator foundation",
                QStringLiteral("\u5c31\u7eea\uff1a\u5a92\u4f53\u4e0e\u7b97\u5b50\u57fa\u7840\u529f\u80fd\u5df2\u52a0\u8f7d")
            ));
        }

        AppSettingsStore::save(m_appSettings);
        return;
    }
}

void MainWindow::previewDetectionOnCurrentImage()
{
    rerunPreview();
}

void MainWindow::openOperatorParameterDialog(const QString& operatorId, const QString& displayName)
{
    if (!ensureMediaSelectedForOperatorAdd()) {
        return;
    }

    auto step = ApplicationContext::instance().operatorRegistry().create(operatorId);
    if (!step) {
        statusBar()->showMessage(QString("Failed to create operator: %1").arg(operatorId));
        return;
    }

    if (m_operatorParameterDialog) {
        const QString localizedDisplayName = localizedOperatorDisplayName(operatorId, displayName, m_appSettings.languageCode);
        m_operatorParameterDialog->setLanguage(m_appSettings.languageCode);
        m_operatorParameterDialog->setOperatorStep(operatorId, localizedDisplayName, step);
        presentDialog(m_operatorParameterDialog);
    }
}

void MainWindow::exportDetectionImages()
{
    if (m_activeDetectionModelPath.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Import an ONNX model before exporting detection results",
            QStringLiteral("请先导入 ONNX 模型，再导出检测结果")
        ));
        return;
    }

    const QString initialDirectory = m_appSettings.defaultExportDirectory.isEmpty()
        ? (m_currentMediaInfo.sourceId.isEmpty() ? QString() : QFileInfo(m_currentMediaInfo.sourceId).absolutePath())
        : m_appSettings.defaultExportDirectory;

    QStringList inputImagePaths = QFileDialog::getOpenFileNames(
        this,
        localizedText(m_appSettings.languageCode, "Select Images For Detection Export", QStringLiteral("选择需要导出检测结果的图片")),
        initialDirectory,
        "Images (*.png *.jpg *.jpeg *.bmp)"
    );

    if (inputImagePaths.isEmpty()) {
        const QString inputDirectory = QFileDialog::getExistingDirectory(
            this,
            localizedText(m_appSettings.languageCode, "Select Image Directory For Detection Export", QStringLiteral("选择需要导出检测结果的图片目录")),
            initialDirectory
        );

        if (!inputDirectory.isEmpty()) {
            inputImagePaths = collectSupportedImageFiles(inputDirectory);
        }
    }

    if (inputImagePaths.isEmpty()) {
        if (m_currentMediaInfo.kind == MediaSourceKind::ImageFile && !m_currentFrame.sourceId.isEmpty()) {
            inputImagePaths.append(m_currentFrame.sourceId);
        } else {
            statusBar()->showMessage(localizedText(
                m_appSettings.languageCode,
                "Select one or more images, or choose an image directory",
                QStringLiteral("请选择一张或多张图片，或选择一个图片目录")
            ));
            return;
        }
    }

    const QString outputDir = QFileDialog::getExistingDirectory(
        this,
        localizedText(m_appSettings.languageCode, "Export Detection Images", QStringLiteral("导出检测图片")),
        initialDirectory
    );
    if (outputDir.isEmpty()) {
        return;
    }

    const DetectionExportService::DetectionExportContext exportContext = buildExportContext(
        buildPipelineDefinition(),
        m_activeDetectionModel
    );

    QString errorMessage;
    if (!DetectionExportService::exportImageBatch(
        inputImagePaths,
        outputDir,
        [this](FramePacket* frame, DetectionFrameResult* result, QString* processErrorMessage) -> bool {
            return processFrame(frame, result, processErrorMessage);
        },
        &errorMessage,
        exportContext
    )) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    m_appSettings.defaultExportDirectory = outputDir;
    AppSettingsStore::save(m_appSettings);

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Exported %1 image detection result(s) to: %2").arg(inputImagePaths.size()).arg(outputDir),
        QStringLiteral("已导出 %1 张图片的检测结果到: %2").arg(inputImagePaths.size()).arg(outputDir)
    ));

    ExportResultSummary summary;
    summary.title = inputImagePaths.size() > 1
        ? localizedText(m_appSettings.languageCode, "Batch Image Export", QStringLiteral("批量图片导出"))
        : localizedText(m_appSettings.languageCode, "Image Export", QStringLiteral("图片导出"));
    summary.summaryText = localizedText(
        m_appSettings.languageCode,
        QString("Exported %1 image detection result(s)").arg(inputImagePaths.size()),
        QStringLiteral("已导出 %1 张图片的检测结果").arg(inputImagePaths.size())
    );
    summary.outputDirectory = outputDir;
    summary.artifacts = QStringList() << "detections.csv" << "pipeline_snapshot.json" << "model_config_snapshot.json";
    if (!inputImagePaths.isEmpty()) {
        summary.artifacts.prepend(exportFileNameFromSource(inputImagePaths.first(), "_overlay.png"));
        summary.artifacts.insert(1, exportFileNameFromSource(inputImagePaths.first(), ".json"));
    }
    recordExportSummary(summary);

    if (m_appSettings.autoOpenExportDirectory) {
        openExportOutputDirectory(outputDir);
    }
}

void MainWindow::exportDetectionVideo()
{
    if ((m_currentMediaInfo.kind != MediaSourceKind::VideoFile
            && m_currentMediaInfo.kind != MediaSourceKind::ImageFolder)
        || m_currentMediaInfo.sourceId.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Load a video or image folder before exporting detection video",
            QStringLiteral("请先加载视频或图片文件夹，再导出检测视频")
        ));
        return;
    }

    if (m_activeDetectionModelPath.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Import an ONNX model before exporting detection video",
            QStringLiteral("请先导入 ONNX 模型，再导出检测视频")
        ));
        return;
    }

    const QString initialDirectory = m_appSettings.defaultExportDirectory.isEmpty()
        ? QFileInfo(m_currentMediaInfo.sourceId).absolutePath()
        : m_appSettings.defaultExportDirectory;

    const QString outputDir = QFileDialog::getExistingDirectory(
        this,
        localizedText(m_appSettings.languageCode, "Export Detection Video", QStringLiteral("导出检测视频")),
        initialDirectory
    );
    if (outputDir.isEmpty()) {
        return;
    }

    const DetectionExportService::DetectionExportContext exportContext = buildExportContext(
        buildPipelineDefinition(),
        m_activeDetectionModel
    );

    const int frameCount = std::max(1, m_currentMediaInfo.frameCount);
    TaskScheduler& taskScheduler = ApplicationContext::instance().taskScheduler();
    if (taskScheduler.isRunning()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Another background task is already running",
            QStringLiteral("已有后台任务正在运行")
        ));
        return;
    }

    const QString inputVideoPath = m_currentMediaInfo.kind == MediaSourceKind::VideoFile
        ? m_currentMediaInfo.sourceId
        : QString();
    const QString languageCode = m_appSettings.languageCode;

    auto finishExport = [this, outputDir, frameCount, languageCode](bool success, const QString& errorMessage) {
        if (!success) {
            statusBar()->showMessage(errorMessage.isEmpty()
                ? localizedText(languageCode, "Video export failed", QStringLiteral("视频导出失败"))
                : errorMessage);
            return;
        }

        m_appSettings.defaultExportDirectory = outputDir;
        AppSettingsStore::save(m_appSettings);

        statusBar()->showMessage(localizedText(
            languageCode,
            QString("Exported detection video to: %1").arg(outputDir),
            QStringLiteral("已导出检测视频到: %1").arg(outputDir)
        ));

        ExportResultSummary summary;
        summary.title = localizedText(languageCode, "Video Export", QStringLiteral("视频导出"));
        summary.summaryText = localizedText(
            languageCode,
            QString("Exported detection video with %1 frame(s)").arg(frameCount),
            QStringLiteral("已导出包含 %1 帧的视频检测结果").arg(frameCount)
        );
        summary.outputDirectory = outputDir;
        summary.artifacts = QStringList()
            << (m_appSettings.exportVideoSideBySide
                ? exportFileNameFromSource(m_currentMediaInfo.sourceId, "_side_by_side.avi")
                : exportFileNameFromSource(m_currentMediaInfo.sourceId, "_overlay.avi"))
            << exportFileNameFromSource(m_currentMediaInfo.sourceId, ".json")
            << exportFileNameFromSource(m_currentMediaInfo.sourceId, ".csv")
            << "pipeline_snapshot.json"
            << "model_config_snapshot.json";
        recordExportSummary(summary);

        if (m_appSettings.autoOpenExportDirectory) {
            openExportOutputDirectory(outputDir);
        }
    };

    if (frameCount > 1 && m_currentMediaInfo.kind == MediaSourceKind::VideoFile) {
        TaskDefinition task;
        task.taskId = QString("offline_video_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        task.displayName = localizedText(languageCode, "Offline Video Export", QStringLiteral("离线视频导出"));
        task.kind = TaskKind::OfflineVideo;
        task.state = TaskState::Running;
        task.createdAt = QDateTime::currentDateTime();
        task.outputDirectory = outputDir;
        recordTaskHistory(task);

        DetectionExportService::VideoExportOptions videoOptions;
        videoOptions.startFrame = qMax(0, m_appSettings.offlineVideoStartFrame);
        videoOptions.endFrame = m_appSettings.offlineVideoEndFrame;
        videoOptions.sideBySide = m_appSettings.exportVideoSideBySide;

        const bool started = taskScheduler.startTask(TaskKind::OfflineVideo, 1, [this, inputVideoPath, outputDir, exportContext, videoOptions](
            int current,
            int total,
            bool* cancelRequestedPtr,
            bool* pauseRequested,
            QString* errorMessage) -> bool {
            Q_UNUSED(current)
            Q_UNUSED(total)

            return DetectionExportService::exportVideoResult(
                inputVideoPath,
                outputDir,
                [this, cancelRequestedPtr, pauseRequested](
                    FramePacket* frame,
                    DetectionFrameResult* result,
                    QString* processErrorMessage) -> bool {
                    if (cancelRequestedPtr && *cancelRequestedPtr) {
                        if (processErrorMessage) {
                            *processErrorMessage = "Export cancelled";
                        }
                        return false;
                    }

                    while (pauseRequested && *pauseRequested && cancelRequestedPtr && !*cancelRequestedPtr) {
                        QThread::msleep(50);
                    }

                    if (!frame) {
                        if (processErrorMessage) {
                            *processErrorMessage = "Video frame state is invalid";
                        }
                        return false;
                    }

                    return processFrame(frame, result, processErrorMessage);
                },
                errorMessage,
                exportContext,
                videoOptions
            );
        });

        if (!started) {
            statusBar()->showMessage(localizedText(
                m_appSettings.languageCode,
                "Failed to start background video export",
                QStringLiteral("无法启动后台视频导出")
            ));
            return;
        }

        auto exportConnection = std::make_shared<QMetaObject::Connection>();
        *exportConnection = connect(
            &ApplicationContext::instance().taskScheduler(),
            &TaskScheduler::taskFinished,
            this,
            [finishExport, exportConnection](bool success) {
                QObject::disconnect(*exportConnection);
                const TaskProgress progress = ApplicationContext::instance().taskScheduler().currentProgress();
                finishExport(success, progress.errorMessage);
            });
        return;
    }

    QString errorMessage;
    DetectionExportService::VideoExportOptions videoOptions;
    videoOptions.startFrame = qMax(0, m_appSettings.offlineVideoStartFrame);
    videoOptions.endFrame = m_appSettings.offlineVideoEndFrame;
    videoOptions.sideBySide = m_appSettings.exportVideoSideBySide;

    const bool success = DetectionExportService::exportVideoResult(
        m_currentMediaInfo.sourceId,
        outputDir,
        [this](FramePacket* frame, DetectionFrameResult* result, QString* processErrorMessage) -> bool {
            if (!frame) {
                if (processErrorMessage) {
                    *processErrorMessage = "Video frame state is invalid";
                }
                return false;
            }

            statusBar()->showMessage(localizedText(
                m_appSettings.languageCode,
                QString("Exporting video detections: frame %1").arg(frame->frameId + 1),
                QStringLiteral("正在导出视频检测结果: 第 %1 帧").arg(frame->frameId + 1)
            ));
            return processFrame(frame, result, processErrorMessage);
        },
        &errorMessage,
        exportContext,
        videoOptions
    );

    finishExport(success, errorMessage);
}

void MainWindow::openExportOutputDirectory(const QString& outputDirectory)
{
    if (outputDirectory.isEmpty()) {
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(outputDirectory));
}

void MainWindow::openExportArtifact(const QString& outputDirectory, const QString& artifactName)
{
    if (outputDirectory.isEmpty() || artifactName.isEmpty()) {
        return;
    }

    const QString filePath = QDir(outputDirectory).filePath(artifactName);
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

bool MainWindow::applyPipelineSteps(FramePacket* frame, QString* errorMessage) const
{
    if (!frame) {
        if (errorMessage) {
            *errorMessage = "Pipeline frame state is invalid";
        }
        return false;
    }

    DiagnosticsLog::instance().clearStepTimings();

    for (const PipelineStepState& state : m_pipelineSteps) {
        if (!state.step) {
            continue;
        }

        QElapsedTimer timer;
        timer.start();
        const StepResult result = state.step->execute(*frame, RunContext{true});
        DiagnosticsLog::instance().recordStepTiming(
            state.step->id(),
            state.displayName,
            timer.elapsed()
        );
        if (!result.success) {
            if (errorMessage) {
                *errorMessage = result.errorMessage;
            }
            return false;
        }
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

bool MainWindow::runDetectionOnCurrentFrame(FramePacket* previewFrame, DetectionFrameResult* detectionResult, QString* errorMessage) const
{
    if (!previewFrame || !detectionResult) {
        if (errorMessage) {
            *errorMessage = "Detection preview state is invalid";
        }
        return false;
    }

    if (!previewFrame->hasWorkingFrame()) {
        if (errorMessage) {
            *errorMessage = "No working frame available for detection";
        }
        return false;
    }

    if (m_activeDetectionModelPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "No active ONNX model is available";
        }
        return false;
    }

    auto modelStep = OnnxModelStepFactory::createStep(
        m_activeDetectionModel,
        m_activeDetectionModelPath,
        m_activeDetectionLabels
    );
    if (!modelStep) {
        if (errorMessage) {
            *errorMessage = "Failed to create ONNX model step";
        }
        return false;
    }

    const StepResult modelStepResult = modelStep->execute(*previewFrame, RunContext{true});
    if (!modelStepResult.success) {
        if (errorMessage) {
            *errorMessage = modelStepResult.errorMessage;
        }
        return false;
    }

    *detectionResult = detectionResultFromAnnotations(previewFrame->annotations);
    detectionResult->sourceId = previewFrame->sourceId;

    if (errorMessage) {
        errorMessage->clear();
    }

    return true;
}

bool MainWindow::processFrame(FramePacket* frame, DetectionFrameResult* detectionResult, QString* errorMessage) const
{
    if (!frame) {
        if (errorMessage) {
            *errorMessage = "Frame state is invalid";
        }
        return false;
    }

    if (!applyPipelineSteps(frame, errorMessage)) {
        return false;
    }

    if (detectionResult) {
        detectionResult->boxes.clear();
        detectionResult->sourceId = frame->sourceId;
        detectionResult->frameId = frame->frameId;
        detectionResult->timestampMs = frame->timestampMs;
    }

    if (m_activeDetectionModelPath.isEmpty()) {
        if (errorMessage) {
            errorMessage->clear();
        }
        return true;
    }

    return runDetectionOnCurrentFrame(frame, detectionResult, errorMessage);
}

void MainWindow::recordExportSummary(const ExportResultSummary& summary)
{
    if (!m_exportResultsPanel) {
        return;
    }

    m_exportResultsPanel->setLatestSummary(summary);
}

void MainWindow::selectPipelineStep(int index)
{
    if (index < 0 || index >= static_cast<int>(m_pipelineSteps.size())) {
        return;
    }

    m_selectedPipelineStepIndex = index;
    m_parameterPanel->setStep(m_pipelineSteps[static_cast<std::size_t>(index)].step);
    if (m_pipelinePanel) {
        m_pipelinePanel->setCurrentPipelineStep(index);
    }
    if (m_pipelineStripList && !m_updatingPipelineStrip && m_pipelineStripList->currentRow() != index) {
        m_updatingPipelineStrip = true;
        m_pipelineStripList->setCurrentRow(index);
        m_updatingPipelineStrip = false;
    }
    refreshSourceCanvasOverlay();
    updateEmbeddedWorkbenchState();
    if (m_previewMode == PreviewMode::SingleNode) {
        rerunPreview();
    }
}

void MainWindow::removePipelineStep(int index)
{
    if (index < 0 || index >= static_cast<int>(m_pipelineSteps.size())) {
        return;
    }

    m_pipelineSteps.erase(m_pipelineSteps.begin() + index);

    if (m_pipelineSteps.empty()) {
        m_selectedPipelineStepIndex = -1;
        m_perspectivePoints.clear();
        m_affinePoints.clear();
        m_floodFillSeeds.clear();
        m_grabCutRectPoints.clear();
        m_parameterPanel->clearStep();
        m_pipelinePanel->setCurrentPipelineStep(-1);
    } else {
        m_selectedPipelineStepIndex = std::min(index, static_cast<int>(m_pipelineSteps.size()) - 1);
        m_pipelinePanel->setCurrentPipelineStep(m_selectedPipelineStepIndex);
        m_parameterPanel->setStep(m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step);
    }

    m_pipelinePanel->setPipelineStepNames(pipelineStepNames());
    rebuildPipelineStrip();
    refreshSourceCanvasOverlay();
    updateEmbeddedWorkbenchState();
    rerunPreview();
}

void MainWindow::clearPipeline()
{
    m_pipelineSteps.clear();
    m_selectedPipelineStepIndex = -1;
    m_singlePreviewStep.reset();
    m_singlePreviewDisplayName.clear();
    m_perspectivePoints.clear();
    m_affinePoints.clear();
    m_floodFillSeeds.clear();
    m_grabCutRectPoints.clear();
    setPreviewMode(PreviewMode::FullPipeline);
    m_pipelinePanel->setPipelineStepNames({});
    m_pipelinePanel->setCurrentPipelineStep(-1);
    m_parameterPanel->clearStep();
    rebuildPipelineStrip();
    refreshSourceCanvasOverlay();
    updateEmbeddedWorkbenchState();
    rerunPreview();
}

QStringList MainWindow::pipelineStepNames() const
{
    QStringList names;
    for (const PipelineStepState& state : m_pipelineSteps) {
        names.append(state.displayName);
    }
    return names;
}

void MainWindow::renamePipelineStep(int index, const QString& name)
{
    if (index < 0 || index >= static_cast<int>(m_pipelineSteps.size())) {
        return;
    }

    m_pipelineSteps[static_cast<std::size_t>(index)].displayName = name;
    rebuildPipelineStrip();
}

void MainWindow::movePipelineStep(int from, int to)
{
    if (from < 0 || to < 0 || from >= static_cast<int>(m_pipelineSteps.size()) || to >= static_cast<int>(m_pipelineSteps.size()) || from == to) {
        return;
    }

    PipelineStepState state = m_pipelineSteps[static_cast<std::size_t>(from)];
    m_pipelineSteps.erase(m_pipelineSteps.begin() + from);
    m_pipelineSteps.insert(m_pipelineSteps.begin() + to, state);

    if (m_selectedPipelineStepIndex == from) {
        m_selectedPipelineStepIndex = to;
    } else if (from < m_selectedPipelineStepIndex && to >= m_selectedPipelineStepIndex) {
        --m_selectedPipelineStepIndex;
    } else if (from > m_selectedPipelineStepIndex && to <= m_selectedPipelineStepIndex) {
        ++m_selectedPipelineStepIndex;
    }

    if (m_selectedPipelineStepIndex >= 0) {
        m_pipelinePanel->setCurrentPipelineStep(m_selectedPipelineStepIndex);
        m_parameterPanel->setStep(m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step);
    }
    rebuildPipelineStrip();
    updateEmbeddedWorkbenchState();
    rerunPreview();
}

void MainWindow::savePipelineToFile()
{
    if (m_pipelineSteps.empty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Pipeline is empty",
            QStringLiteral("流程为空")
        ));
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        localizedText(m_appSettings.languageCode, "Save Pipeline", QStringLiteral("保存流程")),
        QString(),
        "Pipeline (*.json)"
    );
    if (filePath.isEmpty()) {
        return;
    }

    QString errorMessage;
    if (!PipelineJsonSerializer::saveToFile(buildPipelineDefinition(), filePath, &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Saved pipeline: %1").arg(filePath),
        QStringLiteral("已保存流程: %1").arg(filePath)
    ));
}

void MainWindow::loadPipelineFromFile()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        localizedText(m_appSettings.languageCode, "Load Pipeline", QStringLiteral("加载流程")),
        QString(),
        "Pipeline (*.json)"
    );
    if (filePath.isEmpty()) {
        return;
    }

    loadPipelineFromPath(filePath);
}

bool MainWindow::applyPipelineDefinition(const PipelineDefinition& definition, const QString& sourceLabel)
{
    std::vector<PipelineStepState> loadedSteps;
    loadedSteps.reserve(static_cast<std::size_t>(definition.steps.size()));

    for (const PipelineStepDefinition& stepDefinition : definition.steps) {
        std::shared_ptr<IPipelineStep> step = ApplicationContext::instance().operatorRegistry().create(stepDefinition.stepId);
        if (!step) {
            statusBar()->showMessage(localizedText(
                m_appSettings.languageCode,
                QString("Unknown pipeline step: %1").arg(stepDefinition.stepId),
                QStringLiteral("未知流程步骤: %1").arg(stepDefinition.stepId)
            ));
            return false;
        }

        step->setParameterValues(stepDefinition.parameters);
        loadedSteps.push_back({
            stepDefinition.displayName.isEmpty() ? step->displayName() : stepDefinition.displayName,
            step,
        });
    }

    m_pipelineSteps = std::move(loadedSteps);

    if (m_pipelineSteps.empty()) {
        m_selectedPipelineStepIndex = -1;
        m_singlePreviewStep.reset();
        m_singlePreviewDisplayName.clear();
        setPreviewMode(PreviewMode::FullPipeline);
        m_parameterPanel->clearStep();
        m_pipelinePanel->setCurrentPipelineStep(-1);
    } else {
        m_selectedPipelineStepIndex = 0;
        m_pipelinePanel->setCurrentPipelineStep(m_selectedPipelineStepIndex);
        m_parameterPanel->setStep(m_pipelineSteps.front().step);
    }

    m_pipelinePanel->setPipelineStepNames(pipelineStepNames());
    rebuildPipelineStrip();
    updateEmbeddedWorkbenchState();
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Loaded pipeline: %1").arg(sourceLabel),
        QStringLiteral("已加载流程: %1").arg(sourceLabel)
    ));
    rerunPreview();
    return true;
}

void MainWindow::loadSamplePipeline(const QString& resourceRelativePath)
{
    const QString filePath = PlatformPaths::resolveResourcePath(resourceRelativePath);
    if (!QFileInfo::exists(filePath)) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            QString("Sample pipeline not found: %1").arg(resourceRelativePath),
            QStringLiteral("未找到示例流程: %1").arg(resourceRelativePath)
        ));
        return;
    }

    const PipelineLoadResult result = PipelineJsonSerializer::loadFromFile(filePath);
    if (!result.success) {
        statusBar()->showMessage(result.errorMessage);
        return;
    }

    applyPipelineDefinition(result.definition, filePath);
}

void MainWindow::loadSampleProject(const QString& resourceRelativePath)
{
    const QString filePath = PlatformPaths::resolveResourcePath(resourceRelativePath);
    if (!QFileInfo::exists(filePath)) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            QString("Sample project not found: %1").arg(resourceRelativePath),
            QStringLiteral("未找到示例项目: %1").arg(resourceRelativePath)
        ));
        return;
    }

    const ProjectLoadResult result = ProjectService::loadFromFile(filePath);
    if (!result.success) {
        statusBar()->showMessage(result.errorMessage);
        return;
    }

    m_currentProjectFilePath = filePath;
    recordRecentProject(filePath);
    applyProjectDefinition(result.project);
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Loaded sample project: %1").arg(filePath),
        QStringLiteral("已加载示例项目: %1").arg(filePath)
    ));
}

void MainWindow::populateSamplePipelineMenu()
{
    if (!m_loadSamplePipelineMenu) {
        return;
    }

    m_loadSamplePipelineMenu->clear();
    const QString languageCode = m_appSettings.languageCode;
    for (const SampleResourceEntry& entry : kSamplePipelines) {
        QAction* action = m_loadSamplePipelineMenu->addAction(localizedText(
            languageCode,
            QString::fromUtf8(entry.englishLabel),
            QString::fromUtf8(entry.chineseLabel)
        ));
        const QString relativePath = QString::fromUtf8(entry.relativePath);
        connect(action, &QAction::triggered, this, [this, relativePath] {
            loadSamplePipeline(relativePath);
        });
    }
}

void MainWindow::populateSampleProjectMenu()
{
    if (!m_loadSampleProjectMenu) {
        return;
    }

    m_loadSampleProjectMenu->clear();
    const QString languageCode = m_appSettings.languageCode;
    for (const SampleResourceEntry& entry : kSampleProjects) {
        QAction* action = m_loadSampleProjectMenu->addAction(localizedText(
            languageCode,
            QString::fromUtf8(entry.englishLabel),
            QString::fromUtf8(entry.chineseLabel)
        ));
        const QString relativePath = QString::fromUtf8(entry.relativePath);
        connect(action, &QAction::triggered, this, [this, relativePath] {
            loadSampleProject(relativePath);
        });
    }
}

PipelineDefinition MainWindow::buildPipelineDefinition() const
{
    PipelineDefinition definition;
    definition.pipelineId = "cvverify.pipeline";
    definition.displayName = "Current Pipeline";

    for (const PipelineStepState& state : m_pipelineSteps) {
        definition.steps.append({
            state.step->id(),
            state.displayName,
            state.step->parameterValues(),
        });
    }

    return definition;
}

QString MainWindow::previewModeSummary() const
{
    {
        if (m_previewMode == PreviewMode::SingleNode) {
            QString stepName;
            if (m_selectedPipelineStepIndex >= 0
                && m_selectedPipelineStepIndex < static_cast<int>(m_pipelineSteps.size())) {
                const PipelineStepState& state = m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)];
                stepName = localizedOperatorDisplayName(state.step->id(), state.displayName, m_appSettings.languageCode);
            } else if (m_singlePreviewStep) {
                stepName = localizedOperatorDisplayName(
                    m_singlePreviewStep->id(),
                    m_singlePreviewDisplayName.isEmpty() ? m_singlePreviewStep->displayName() : m_singlePreviewDisplayName,
                    m_appSettings.languageCode
                );
            }

            if (stepName.isEmpty()) {
                return localizedText(
                    m_appSettings.languageCode,
                    "Preview Mode: Single Node",
                    QStringLiteral("\u9884\u89c8\u6a21\u5f0f\uff1a\u5355\u8282\u70b9")
                );
            }

            return localizedText(
                m_appSettings.languageCode,
                QString("Preview Mode: Single Node (%1)").arg(stepName),
                QStringLiteral("\u9884\u89c8\u6a21\u5f0f\uff1a\u5355\u8282\u70b9\uff08%1\uff09").arg(stepName)
            );
        }

        return localizedText(
            m_appSettings.languageCode,
            "Preview Mode: Full Pipeline",
            QStringLiteral("\u9884\u89c8\u6a21\u5f0f\uff1a\u5168\u6d41\u7a0b")
        );
    }
}

QString MainWindow::pipelineDetailsSummary() const
{
    {
        QStringList lines;
        for (std::size_t index = 0; index < m_pipelineSteps.size(); ++index) {
            const PipelineStepState& state = m_pipelineSteps[index];
            const QString displayName = localizedOperatorDisplayName(
                state.step->id(),
                state.displayName,
                m_appSettings.languageCode
            );
            lines.append(QString("[%1] %2  %3")
                .arg(index + 1)
                .arg(displayName)
                .arg(formatParameterMap(state.step)));
        }

        if (lines.isEmpty()) {
            lines.append(localizedText(
                m_appSettings.languageCode,
                "Pipeline Details: Empty",
                QStringLiteral("\u6d41\u7a0b\u8be6\u60c5\uff1a\u5f53\u524d\u4e3a\u7a7a")
            ));
        }

        if (m_previewMode == PreviewMode::SingleNode && m_singlePreviewStep) {
            const QString previewName = localizedOperatorDisplayName(
                m_singlePreviewStep->id(),
                m_singlePreviewDisplayName.isEmpty() ? m_singlePreviewStep->displayName() : m_singlePreviewDisplayName,
                m_appSettings.languageCode
            );
            lines.prepend(localizedText(
                m_appSettings.languageCode,
                QString("Current Single Node Preview: %1  %2").arg(previewName).arg(formatParameterMap(m_singlePreviewStep)),
                QStringLiteral("\u5f53\u524d\u5355\u8282\u70b9\u9884\u89c8\uff1a%1  %2").arg(previewName).arg(formatParameterMap(m_singlePreviewStep))
            ));
        }

        return lines.join("\n");
    }
}

QString MainWindow::formatParameterMap(const std::shared_ptr<IPipelineStep>& step) const
{
    {
        if (!step) {
            return QString();
        }

        const StepSchema schema = step->schema();
        const QVariantMap values = step->parameterValues();
        QStringList parts;

        for (const StepParameter& parameter : schema.parameters) {
            const QVariant value = values.value(parameter.key, parameter.defaultValue);
            QString displayValue = formatVariantValue(value);
            if (parameter.type == StepParameterType::Choice) {
                for (const StepParameterChoice& choice : parameter.choices) {
                    if (choice.value == value.toString()) {
                        displayValue = localizedChoiceLabelForSummary(
                            parameter.key,
                            choice.value,
                            choice.label,
                            m_appSettings.languageCode
                        );
                        break;
                    }
                }
            }

            parts.append(QString("%1=%2")
                .arg(localizedParameterNameForSummary(parameter.key, parameter.displayName, m_appSettings.languageCode))
                .arg(displayValue));
        }

        return parts.join(", ");
    }
}

QString MainWindow::formatVariantValue(const QVariant& value) const
{
    if (value.type() == QVariant::Bool) {
        if (isChineseLanguage(m_appSettings.languageCode)) {
            return value.toBool() ? QStringLiteral("\u662f") : QStringLiteral("\u5426");
        }
        return value.toBool() ? QString("true") : QString("false");
    }
    if (value.canConvert<double>() && value.type() == QVariant::Double) {
        return QString::number(value.toDouble(), 'f', 3);
    }
    return value.toString();
}

void MainWindow::createMenus()
{
    m_mediaMenu = menuBar()->addMenu("Media");
    m_mediaMenu->menuAction()->setObjectName("mediaMenuAction");
    m_projectMenu = menuBar()->addMenu("Project");
    m_projectMenu->menuAction()->setObjectName("projectMenuAction");
    m_pipelineMenu = menuBar()->addMenu("Pipeline");
    m_pipelineMenu->menuAction()->setObjectName("pipelineMenuAction");
    m_exportMenu = menuBar()->addMenu("Export");
    m_exportMenu->menuAction()->setObjectName("exportMenuAction");
    m_viewMenu = menuBar()->addMenu("View");
    m_viewMenu->menuAction()->setObjectName("viewMenuAction");
    m_previewMenu = menuBar()->addMenu("Preview");
    m_previewMenu->menuAction()->setObjectName("previewMenuAction");
    m_operatorMenu = menuBar()->addMenu("Operators");
    m_operatorMenu->menuAction()->setObjectName("operatorMenuAction");
    m_playbackMenu = menuBar()->addMenu("Playback");
    m_playbackMenu->menuAction()->setObjectName("playbackMenuAction");
    m_settingsMenu = menuBar()->addMenu("Settings");
    m_settingsMenu->menuAction()->setObjectName("settingsMenuAction");
    m_languageMenu = menuBar()->addMenu("Language");
    m_languageMenu->menuAction()->setObjectName("languageMenuAction");

    m_showMediaWorkbenchAction = m_mediaMenu->addAction("Media & Model Workbench", this, [this] { showMediaWorkbench(); });
    m_showMediaWorkbenchAction->setObjectName("showMediaWorkbenchAction");
    m_mediaMenu->setDefaultAction(m_showMediaWorkbenchAction);
    m_mediaMenu->addSeparator();
    m_openImageAction = m_mediaMenu->addAction("Open Image", this, [this] { loadImageFile(); });
    m_openImageAction->setObjectName("openImageAction");
    m_openVideoAction = m_mediaMenu->addAction("Open Video", this, [this] { loadVideoFile(); });
    m_openVideoAction->setObjectName("openVideoAction");
    m_openImageFolderAction = m_mediaMenu->addAction("Open Image Folder", this, [this] { loadImageFolder(); });
    m_openImageFolderAction->setObjectName("openImageFolderAction");
    m_openRecentMenu = m_mediaMenu->addMenu("Open Recent");
    m_openRecentMenu->menuAction()->setObjectName("openRecentMenuAction");
    m_importYoloModelAction = m_mediaMenu->addAction("Import ONNX Model", this, [this] { importYoloModel(); });
    m_importYoloModelAction->setObjectName("importYoloModelAction");
    m_editModelConfigAction = m_mediaMenu->addAction("Edit Model Configuration", this, [this] { editActiveModelConfig(); });
    m_editModelConfigAction->setObjectName("editModelConfigAction");

    m_saveProjectAction = m_projectMenu->addAction("Save Project", this, [this] { saveProjectToFile(); });
    m_saveProjectAction->setObjectName("saveProjectAction");
    m_loadProjectAction = m_projectMenu->addAction("Load Project", this, [this] { loadProjectFromFile(); });
    m_loadProjectAction->setObjectName("loadProjectAction");
    m_openRecentProjectMenu = m_projectMenu->addMenu("Open Recent Project");
    m_openRecentProjectMenu->setObjectName("openRecentProjectMenu");
    m_openRecentProjectMenu->menuAction()->setObjectName("openRecentProjectMenuAction");
    m_loadSampleProjectMenu = m_projectMenu->addMenu("Load Sample Project");
    m_loadSampleProjectMenu->setObjectName("loadSampleProjectMenu");
    populateSampleProjectMenu();

    m_showPipelineWorkbenchAction = m_pipelineMenu->addAction("Show Operator Workbench");
    m_showPipelineWorkbenchAction->setObjectName("showPipelineWorkbenchAction");
    m_showPipelineWorkbenchAction->setCheckable(true);
    m_pipelineMenu->setDefaultAction(m_showPipelineWorkbenchAction);
    connect(m_showPipelineWorkbenchAction, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            showPipelineWorkbench();
            return;
        }
        setOperatorWorkbenchVisible(false);
    });
    setOperatorWorkbenchVisible(false);
    m_pipelineMenu->addSeparator();
    m_addOperatorMenu = m_pipelineMenu->addMenu("Add Operator");
    m_removeSelectedPipelineStepAction = m_pipelineMenu->addAction("Remove Selected Step", this, [this] {
        removePipelineStep(m_selectedPipelineStepIndex);
    });
    m_clearPipelineAction = m_pipelineMenu->addAction("Clear Pipeline", this, [this] { clearPipeline(); });
    m_pipelineMenu->addSeparator();
    m_savePipelineAction = m_pipelineMenu->addAction("Save Pipeline", this, [this] { savePipelineToFile(); });
    m_loadPipelineAction = m_pipelineMenu->addAction("Load Pipeline", this, [this] { loadPipelineFromFile(); });
    m_loadSamplePipelineMenu = m_pipelineMenu->addMenu("Load Sample Pipeline");
    m_loadSamplePipelineMenu->setObjectName("loadSamplePipelineMenu");
    populateSamplePipelineMenu();
    m_addDetectionStepAction = m_pipelineMenu->addAction("Add Active Model Step", this, [this] { addActiveModelStepToPipeline(); });

    m_showExportResultsAction = m_exportMenu->addAction("Export Results", this, [this] { showExportResultsWorkbench(); });
    m_showExportResultsAction->setObjectName("showExportResultsAction");
    m_exportMenu->setDefaultAction(m_showExportResultsAction);
    m_exportMenu->addSeparator();
    m_exportDetectionImagesAction = m_exportMenu->addAction("Export Detection Images", this, [this] { exportDetectionImages(); });
    m_exportDetectionVideoAction = m_exportMenu->addAction("Export Detection Video", this, [this] { exportDetectionVideo(); });
    m_exportBatchPipelineAction = m_exportMenu->addAction("Export Batch Pipeline", this, [this] { exportBatchPipeline(); });
    m_exportPipelineSnapshotAction = m_exportMenu->addAction(
        "Export Pipeline Snapshot",
        this,
        [this] { exportPipelineSnapshot(); });
    m_exportAppConfigSnapshotAction = m_exportMenu->addAction(
        "Export App Config Snapshot",
        this,
        [this] { exportAppConfigSnapshot(); });
    m_exportComparisonAction = m_exportMenu->addAction("Export Comparison Image", this, [this] { exportComparisonResult(); });

    m_dualViewAction = m_previewMenu->addAction("Dual View");
    m_wipeViewAction = m_previewMenu->addAction("Wipe Comparison");
    m_fourGridViewAction = m_previewMenu->addAction("Four Grid View");
    m_dualViewAction->setCheckable(true);
    m_wipeViewAction->setCheckable(true);
    m_fourGridViewAction->setCheckable(true);
    auto* viewModeGroup = new QActionGroup(this);
    viewModeGroup->setExclusive(true);
    viewModeGroup->addAction(m_dualViewAction);
    viewModeGroup->addAction(m_wipeViewAction);
    viewModeGroup->addAction(m_fourGridViewAction);
    m_dualViewAction->setChecked(true);
    connect(m_dualViewAction, &QAction::triggered, this, [this] { setPreviewViewMode(PreviewViewMode::Dual); });
    connect(m_wipeViewAction, &QAction::triggered, this, [this] { setPreviewViewMode(PreviewViewMode::Wipe); });
    connect(m_fourGridViewAction, &QAction::triggered, this, [this] { setPreviewViewMode(PreviewViewMode::FourGrid); });
    m_previewMenu->addSeparator();
    m_captureComparisonBaselineAction = m_previewMenu->addAction(
        "Capture Comparison Baseline",
        this,
        [this] { captureComparisonBaseline(); });
    m_clearComparisonBaselineAction = m_previewMenu->addAction(
        "Clear Comparison Baseline",
        this,
        [this] { clearComparisonBaseline(); });
    m_clearComparisonBaselineAction->setEnabled(false);
    m_previewMenu->addSeparator();

    m_previewModeActionGroup = new QActionGroup(this);
    m_previewModeActionGroup->setExclusive(true);
    m_fullPipelinePreviewAction = m_previewMenu->addAction("Full Pipeline Preview");
    m_fullPipelinePreviewAction->setObjectName("fullPipelinePreviewAction");
    m_fullPipelinePreviewAction->setCheckable(true);
    m_singleNodePreviewAction = m_previewMenu->addAction("Single Node Preview");
    m_singleNodePreviewAction->setObjectName("singleNodePreviewAction");
    m_singleNodePreviewAction->setCheckable(true);
    m_previewModeActionGroup->addAction(m_fullPipelinePreviewAction);
    m_previewModeActionGroup->addAction(m_singleNodePreviewAction);
    m_fullPipelinePreviewAction->setChecked(true);

    connect(m_fullPipelinePreviewAction, &QAction::triggered, this, [this] {
        setPreviewMode(PreviewMode::FullPipeline);
        rerunPreview();
    });
    connect(m_singleNodePreviewAction, &QAction::triggered, this, [this] {
        setPreviewMode(PreviewMode::SingleNode);
        rerunPreview();
    });

    m_previousFrameAction = m_playbackMenu->addAction("Previous Frame", this, [this] { requestPreviousVideoFrame(); });
    m_playPauseAction = m_playbackMenu->addAction("Play", this, [this] { toggleVideoPlayback(!m_videoPlaybackTimer->isActive()); });
    m_nextFrameAction = m_playbackMenu->addAction("Next Frame", this, [this] { requestNextVideoFrame(); });

    m_systemSettingsAction = m_settingsMenu->addAction("System Settings", this, [this] { showSystemSettings(); });

    m_languageActionGroup = new QActionGroup(this);
    m_languageActionGroup->setExclusive(true);
    m_englishLanguageAction = m_languageMenu->addAction("English");
    m_englishLanguageAction->setObjectName("englishLanguageAction");
    m_englishLanguageAction->setCheckable(true);
    m_chineseLanguageAction = m_languageMenu->addAction("Simplified Chinese");
    m_chineseLanguageAction->setObjectName("chineseLanguageAction");
    m_chineseLanguageAction->setCheckable(true);
    m_languageActionGroup->addAction(m_englishLanguageAction);
    m_languageActionGroup->addAction(m_chineseLanguageAction);

    connect(m_englishLanguageAction, &QAction::triggered, this, [this] { setLanguage("en"); });
    connect(m_chineseLanguageAction, &QAction::triggered, this, [this] { setLanguage("zh-CN"); });

    m_helpMenu = menuBar()->addMenu("Help");
    m_helpMenu->menuAction()->setObjectName("helpMenuAction");
    m_openQuickStartGuideAction = m_helpMenu->addAction(
        "Quick Start",
        this,
        [this] { presentQuickStartGuideDialog(); });
    m_openQuickStartGuideAction->setObjectName("openQuickStartGuideAction");
    m_helpMenu->addSeparator();
    m_aboutAction = m_helpMenu->addAction("About CVVerify", this, [this] { showAboutDialog(); });
    m_aboutAction->setObjectName("aboutAction");

    rebuildRecentMediaMenu();

    const QList<OperatorDescriptor> descriptors = ApplicationContext::instance().operatorRegistry().allOperators();
    populateGroupedOperatorMenu(
        m_addOperatorMenu,
        descriptors,
        m_appSettings.languageCode,
        "addOperatorCategoryMenu",
        [this](const OperatorDescriptor& descriptor) {
            addOperatorToPipeline(descriptor.id);
        });
    populateGroupedOperatorMenu(
        m_operatorMenu,
        descriptors,
        m_appSettings.languageCode,
        "operatorCategoryMenu",
        [this](const OperatorDescriptor& descriptor) {
            addOperatorToPipeline(descriptor.id);
        });

    setupKeyboardShortcuts();
    rebuildRecentProjectMenu();
}

void MainWindow::rebuildMenuTexts()
{
    {
        const QString languageCode = m_appSettings.languageCode;
        const QList<OperatorDescriptor> descriptors = ApplicationContext::instance().operatorRegistry().allOperators();

        if (m_mediaMenu) m_mediaMenu->setTitle(localizedText(languageCode, "Media", QStringLiteral("\u5a92\u4f53")));
        if (m_projectMenu) m_projectMenu->setTitle(localizedText(languageCode, "Project", QStringLiteral("\u9879\u76ee")));
        if (m_pipelineMenu) m_pipelineMenu->setTitle(localizedText(languageCode, "Pipeline", QStringLiteral("\u6d41\u7a0b")));
        if (m_exportMenu) m_exportMenu->setTitle(localizedText(languageCode, "Export", QStringLiteral("\u5bfc\u51fa")));
        if (m_viewMenu) m_viewMenu->setTitle(localizedText(languageCode, "View", QStringLiteral("\u89c6\u56fe")));
        if (m_previewMenu) m_previewMenu->setTitle(localizedText(languageCode, "Preview", QStringLiteral("\u9884\u89c8")));
        if (m_operatorMenu) m_operatorMenu->setTitle(localizedText(languageCode, "Operators", QStringLiteral("\u7b97\u5b50")));
        if (m_playbackMenu) m_playbackMenu->setTitle(localizedText(languageCode, "Playback", QStringLiteral("\u64ad\u653e")));
        if (m_settingsMenu) m_settingsMenu->setTitle(localizedText(languageCode, "Settings", QStringLiteral("\u8bbe\u7f6e")));
        if (m_helpMenu) m_helpMenu->setTitle(localizedText(languageCode, "Help", QStringLiteral("\u5e2e\u52a9")));
        if (m_languageMenu) m_languageMenu->setTitle(localizedText(languageCode, "Language", QStringLiteral("\u8bed\u8a00")));

        if (m_showMediaWorkbenchAction) m_showMediaWorkbenchAction->setText(localizedText(languageCode, "Media & Model Workbench", QStringLiteral("\u5a92\u4f53\u4e0e\u6a21\u578b\u5de5\u4f5c\u53f0")));
        if (m_openImageAction) m_openImageAction->setText(localizedText(languageCode, "Open Image", QStringLiteral("\u6253\u5f00\u56fe\u7247")));
        if (m_openVideoAction) m_openVideoAction->setText(localizedText(languageCode, "Open Video", QStringLiteral("\u6253\u5f00\u89c6\u9891")));
        if (m_openImageFolderAction) {
            m_openImageFolderAction->setText(localizedText(
                languageCode,
                "Open Image Folder",
                QStringLiteral("\u6253\u5f00\u56fe\u7247\u6587\u4ef6\u5939")
            ));
        }
        if (m_openRecentMenu) {
            m_openRecentMenu->setTitle(localizedText(languageCode, "Open Recent", QStringLiteral("\u6253\u5f00\u6700\u8fd1")));
        }
        if (m_importYoloModelAction) m_importYoloModelAction->setText(localizedText(languageCode, "Import ONNX Model", QStringLiteral("\u5bfc\u5165 ONNX \u6a21\u578b")));
        if (m_editModelConfigAction) {
            m_editModelConfigAction->setText(localizedText(
                languageCode,
                "Edit Model Configuration",
                QStringLiteral("\u7f16\u8f91\u6a21\u578b\u914d\u7f6e")
            ));
        }

        if (m_saveProjectAction) m_saveProjectAction->setText(localizedText(languageCode, "Save Project", QStringLiteral("\u4fdd\u5b58\u9879\u76ee")));
        if (m_loadProjectAction) m_loadProjectAction->setText(localizedText(languageCode, "Load Project", QStringLiteral("\u52a0\u8f7d\u9879\u76ee")));
        if (m_openRecentProjectMenu) {
            m_openRecentProjectMenu->setTitle(localizedText(
                languageCode,
                "Open Recent Project",
                QStringLiteral("\u6253\u5f00\u6700\u8fd1\u9879\u76ee")
            ));
            rebuildRecentProjectMenu();
        }
        if (m_loadSampleProjectMenu) {
            m_loadSampleProjectMenu->setTitle(localizedText(
                languageCode,
                "Load Sample Project",
                QStringLiteral("\u52a0\u8f7d\u793a\u4f8b\u9879\u76ee")
            ));
            populateSampleProjectMenu();
        }

        if (m_showPipelineWorkbenchAction) m_showPipelineWorkbenchAction->setText(localizedText(languageCode, "Show Operator Workbench", QStringLiteral("\u663e\u793a\u7b97\u5b50\u5de5\u4f5c\u5e26")));
        if (m_addOperatorMenu) m_addOperatorMenu->setTitle(localizedText(languageCode, "Add Operator", QStringLiteral("\u6dfb\u52a0\u7b97\u5b50")));
        if (m_removeSelectedPipelineStepAction) m_removeSelectedPipelineStepAction->setText(localizedText(languageCode, "Remove Selected Step", QStringLiteral("\u79fb\u9664\u9009\u4e2d\u6b65\u9aa4")));
        if (m_clearPipelineAction) m_clearPipelineAction->setText(localizedText(languageCode, "Clear Pipeline", QStringLiteral("\u6e05\u7a7a\u6d41\u7a0b")));
        if (m_savePipelineAction) m_savePipelineAction->setText(localizedText(languageCode, "Save Pipeline", QStringLiteral("\u4fdd\u5b58\u6d41\u7a0b")));
        if (m_loadPipelineAction) m_loadPipelineAction->setText(localizedText(languageCode, "Load Pipeline", QStringLiteral("\u52a0\u8f7d\u6d41\u7a0b")));
        if (m_loadSamplePipelineMenu) {
            m_loadSamplePipelineMenu->setTitle(localizedText(
                languageCode,
                "Load Sample Pipeline",
                QStringLiteral("\u52a0\u8f7d\u793a\u4f8b\u6d41\u7a0b")
            ));
            populateSamplePipelineMenu();
        }

        if (m_showModelDockAction) {
            m_showModelDockAction->setText(localizedText(
                languageCode,
                "Model && Tasks Panel",
                QStringLiteral("\u6a21\u578b\u4e0e\u4efb\u52a1\u9762\u677f")
            ));
        }
        if (m_showInspectorDockAction) {
            m_showInspectorDockAction->setText(localizedText(
                languageCode,
                "Statistics && Diagnostics Panel",
                QStringLiteral("\u7edf\u8ba1\u4e0e\u8bca\u65ad\u9762\u677f")
            ));
        }
        if (m_showAllWorkbenchPanelsAction) {
            m_showAllWorkbenchPanelsAction->setText(localizedText(
                languageCode,
                "Show All Side Panels",
                QStringLiteral("\u663e\u793a\u5168\u90e8\u4fa7\u8fb9\u9762\u677f")
            ));
        }
        if (m_hideAllWorkbenchPanelsAction) {
            m_hideAllWorkbenchPanelsAction->setText(localizedText(
                languageCode,
                "Hide All Side Panels",
                QStringLiteral("\u9690\u85cf\u5168\u90e8\u4fa7\u8fb9\u9762\u677f")
            ));
        }
        if (m_showDiagnosticsAction) {
            m_showDiagnosticsAction->setText(localizedText(
                languageCode,
                "Show Diagnostics",
                QStringLiteral("\u663e\u793a\u8bca\u65ad")
            ));
        }

        if (m_showExportResultsAction) m_showExportResultsAction->setText(localizedText(languageCode, "Export Results", QStringLiteral("\u5bfc\u51fa\u7ed3\u679c")));
        if (m_exportDetectionImagesAction) m_exportDetectionImagesAction->setText(localizedText(languageCode, "Export Detection Images", QStringLiteral("\u5bfc\u51fa\u68c0\u6d4b\u56fe\u7247")));
        if (m_exportDetectionVideoAction) m_exportDetectionVideoAction->setText(localizedText(languageCode, "Export Detection Video", QStringLiteral("\u5bfc\u51fa\u68c0\u6d4b\u89c6\u9891")));
        if (m_exportPipelineSnapshotAction) {
            m_exportPipelineSnapshotAction->setText(localizedText(
                languageCode,
                "Export Pipeline Snapshot",
                QStringLiteral("\u5bfc\u51fa\u6d41\u7a0b\u5feb\u7167")
            ));
        }
        if (m_exportAppConfigSnapshotAction) {
            m_exportAppConfigSnapshotAction->setText(localizedText(
                languageCode,
                "Export App Config Snapshot",
                QStringLiteral("\u5bfc\u51fa\u5e94\u7528\u914d\u7f6e\u5feb\u7167")
            ));
        }

        if (m_fullPipelinePreviewAction) m_fullPipelinePreviewAction->setText(localizedText(languageCode, "Full Pipeline Preview", QStringLiteral("\u5168\u6d41\u7a0b\u9884\u89c8")));
        if (m_singleNodePreviewAction) m_singleNodePreviewAction->setText(localizedText(languageCode, "Single Node Preview", QStringLiteral("\u5355\u8282\u70b9\u9884\u89c8")));
        if (m_dualViewAction) m_dualViewAction->setText(localizedText(languageCode, "Dual View", QStringLiteral("\u53cc\u753b\u9762")));
        if (m_wipeViewAction) m_wipeViewAction->setText(localizedText(languageCode, "Wipe Comparison", QStringLiteral("\u62c6\u5206\u5bf9\u6bd4")));
        if (m_fourGridViewAction) m_fourGridViewAction->setText(localizedText(languageCode, "Four Grid View", QStringLiteral("\u56db\u5bab\u683c")));
        if (m_captureComparisonBaselineAction) {
            m_captureComparisonBaselineAction->setText(localizedText(
                languageCode,
                "Capture Comparison Baseline",
                QStringLiteral("\u6355\u83b7\u5bf9\u6bd4\u57fa\u7ebf")));
        }
        if (m_clearComparisonBaselineAction) {
            m_clearComparisonBaselineAction->setText(localizedText(
                languageCode,
                "Clear Comparison Baseline",
                QStringLiteral("\u6e05\u9664\u5bf9\u6bd4\u57fa\u7ebf")));
        }

        if (m_previousFrameAction) m_previousFrameAction->setText(localizedText(languageCode, "Previous Frame", QStringLiteral("\u4e0a\u4e00\u5e27")));
        if (m_playPauseAction) {
            m_playPauseAction->setText(m_videoPlaybackTimer && m_videoPlaybackTimer->isActive()
                ? localizedText(languageCode, "Pause", QStringLiteral("\u6682\u505c"))
                : localizedText(languageCode, "Play", QStringLiteral("\u64ad\u653e")));
        }
        if (m_nextFrameAction) m_nextFrameAction->setText(localizedText(languageCode, "Next Frame", QStringLiteral("\u4e0b\u4e00\u5e27")));

        if (m_systemSettingsAction) m_systemSettingsAction->setText(localizedText(languageCode, "System Settings", QStringLiteral("\u7cfb\u7edf\u8bbe\u7f6e")));
        if (m_openQuickStartGuideAction) {
            m_openQuickStartGuideAction->setText(localizedText(
                languageCode,
                "Quick Start",
                QStringLiteral("\u5feb\u901f\u5165\u95e8")
            ));
        }
        if (m_aboutAction) {
            m_aboutAction->setText(localizedText(languageCode, "About CVVerify", QStringLiteral("\u5173\u4e8e CVVerify")));
        }
        if (m_englishLanguageAction) m_englishLanguageAction->setText("English");
        if (m_chineseLanguageAction) m_chineseLanguageAction->setText(QStringLiteral("\u7b80\u4f53\u4e2d\u6587"));

        localizeOperatorMenuTree(m_addOperatorMenu, descriptors, languageCode);
        localizeOperatorMenuTree(m_operatorMenu, descriptors, languageCode);
        rebuildRecentMediaMenu();
        rebuildEmbeddedWorkbenchTexts();
        if (m_modelListPanel) {
            m_modelListPanel->setLanguage(languageCode);
        }
        if (m_taskHistoryPanel) {
            m_taskHistoryPanel->setLanguage(languageCode);
        }
        if (m_diagnosticsPanel) {
            m_diagnosticsPanel->setLanguage(languageCode);
        }
        if (m_statisticsPanel) {
            m_statisticsPanel->setLanguage(languageCode);
        }
        localizeWorkbenchDocks(languageCode);
        if (m_previewCanvasHost) {
            m_previewCanvasHost->setLanguage(languageCode);
        }
        return;
    }
}

void MainWindow::setPreviewViewMode(PreviewViewMode mode)
{
    m_previewViewMode = mode;
    if (!m_previewCanvasHost) {
        return;
    }

    switch (mode) {
    case PreviewViewMode::Wipe:
        m_previewCanvasHost->setMode(PreviewCanvasHost::Mode::Wipe);
        break;
    case PreviewViewMode::FourGrid:
        m_previewCanvasHost->setMode(PreviewCanvasHost::Mode::FourGrid);
        break;
    case PreviewViewMode::Dual:
    default:
        m_previewCanvasHost->setMode(PreviewCanvasHost::Mode::Dual);
        break;
    }
    rerunPreview();
}

void MainWindow::exportBatchPipeline()
{
    if (m_currentMediaInfo.kind != MediaSourceKind::ImageFolder) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Batch pipeline export requires an image folder",
            QStringLiteral("批量流程导出需要图片文件夹")));
        return;
    }

    ImageFolderSource source(m_currentMediaInfo.sourceId);
    const QStringList imagePaths = source.imagePaths();
    if (imagePaths.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "No images found in folder",
            QStringLiteral("文件夹中没有图片")));
        return;
    }

    const QString outputDir = QFileDialog::getExistingDirectory(
        this,
        localizedText(m_appSettings.languageCode, "Select Batch Output Directory", QStringLiteral("选择批量输出目录")),
        m_appSettings.defaultExportDirectory);
    if (outputDir.isEmpty()) {
        return;
    }

    TaskDefinition task;
    task.taskId = QString("batch_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    task.displayName = "Batch Pipeline";
    task.kind = TaskKind::BatchPipeline;
    task.state = TaskState::Running;
    task.createdAt = QDateTime::currentDateTime();
    task.outputDirectory = outputDir;
    recordTaskHistory(task);

    const bool started = ApplicationContext::instance().taskScheduler().startTask(
        TaskKind::BatchPipeline,
        imagePaths.size(),
        [this, imagePaths, outputDir](int current, int, bool* cancelRequested, bool*, QString* stepError) {
            if (cancelRequested && *cancelRequested) {
                return false;
            }

            FramePacket frame;
            const QString imagePath = imagePaths.at(current);
            cv::Mat image = cv::imread(OpenCvImageIO::toOpenCvFilePath(imagePath), cv::IMREAD_COLOR);
            if (image.empty()) {
                if (stepError) {
                    *stepError = QString("Failed to load %1").arg(imagePath);
                }
                return false;
            }

            frame.sourceId = imagePath;
            frame.originalMat = image;
            frame.workingMat = image.clone();
            DetectionFrameResult detectionResult;
            if (!processFrame(&frame, &detectionResult, stepError)) {
                return false;
            }

            const QString stem = QFileInfo(imagePath).completeBaseName();
            const QString outputPath = QDir(outputDir).filePath(stem + "_processed.png");
            return cv::imwrite(OpenCvImageIO::toOpenCvFilePath(outputPath), frame.workingMat);
        });

    if (!started) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Failed to start batch pipeline task",
            QStringLiteral("无法启动批量流程任务")));
        return;
    }

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Batch pipeline started (%1 images)").arg(imagePaths.size()),
        QStringLiteral("批量流程已开始（%1 张图片）").arg(imagePaths.size())));
}

void MainWindow::exportComparisonResult()
{
    if (!m_currentFrame.hasOriginalFrame()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Load media before exporting comparison",
            QStringLiteral("导出对比图前请先加载媒体")));
        return;
    }

    FramePacket preview = m_currentFrame;
    preview.workingMat = preview.originalMat.clone();
    DetectionFrameResult detectionResult;
    QString errorMessage;
    if (!processFrame(&preview, &detectionResult, &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    if (!detectionResult.boxes.isEmpty() || preview.artifacts.contains("classification_top_k")) {
        DetectionRenderComposer::applyModelArtifactsOverlay(
            preview.workingMat,
            detectionResult,
            preview.artifacts);
    }

    const QString outputDir = QFileDialog::getExistingDirectory(
        this,
        localizedText(m_appSettings.languageCode, "Select Comparison Output Directory", QStringLiteral("选择对比图输出目录")),
        m_appSettings.defaultExportDirectory);
    if (outputDir.isEmpty()) {
        return;
    }

    const cv::Mat leftImage = m_comparisonBaselineActive && !m_comparisonBaselineMat.empty()
        ? m_comparisonBaselineMat
        : m_currentFrame.originalMat;
    const QString outputPath = QDir(outputDir).filePath("comparison.png");
    if (!DetectionExportService::exportComparisonImage(
            leftImage,
            preview.workingMat,
            outputPath,
            &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    DetectionExportService::exportEnvironmentSummary(outputDir, &errorMessage);
    DetectionExportService::exportValidationReport(outputDir, DiagnosticsLog::instance().summaryText(), &errorMessage);
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Comparison exported to %1").arg(outputPath),
        QStringLiteral("对比图已导出到 %1").arg(outputPath)));
}

void MainWindow::captureComparisonBaseline()
{
    if (!m_currentFrame.hasOriginalFrame()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Load media before capturing comparison baseline",
            QStringLiteral("捕获对比基线前请先加载媒体")));
        return;
    }

    FramePacket preview = m_currentFrame;
    preview.workingMat = preview.originalMat.clone();
    if (m_previewScale < 0.99) {
        preview.workingMat = applyPreviewScale(preview.workingMat, m_previewScale);
    }

    DetectionFrameResult detectionResult;
    QString errorMessage;
    if (m_previewMode == PreviewMode::SingleNode) {
        std::shared_ptr<IPipelineStep> previewStep;
        if (m_selectedPipelineStepIndex >= 0
            && m_selectedPipelineStepIndex < static_cast<int>(m_pipelineSteps.size())) {
            previewStep = m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step;
        } else {
            previewStep = m_singlePreviewStep;
        }
        if (previewStep) {
            const StepResult singleStepResult = previewStep->execute(preview, RunContext{true});
            if (!singleStepResult.success) {
                statusBar()->showMessage(singleStepResult.errorMessage);
                return;
            }
        }
        if (!m_activeDetectionModelPath.isEmpty()) {
            detectionResult.sourceId = preview.sourceId;
            detectionResult.frameId = preview.frameId;
            detectionResult.timestampMs = preview.timestampMs;
            if (!runDetectionOnCurrentFrame(&preview, &detectionResult, &errorMessage)) {
                statusBar()->showMessage(errorMessage);
                return;
            }
        }
    } else if (!processFrame(&preview, &detectionResult, &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    if (!detectionResult.boxes.isEmpty() || preview.artifacts.contains("classification_top_k")) {
        DetectionRenderComposer::applyModelArtifactsOverlay(
            preview.workingMat,
            detectionResult,
            preview.artifacts);
    }

    m_comparisonBaselineMat = preview.workingMat.clone();
    m_comparisonBaselineActive = !m_comparisonBaselineMat.empty();
    if (m_clearComparisonBaselineAction) {
        m_clearComparisonBaselineAction->setEnabled(m_comparisonBaselineActive);
    }

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        "Comparison baseline captured",
        QStringLiteral("已捕获对比基线")));
    rerunPreview();
}

void MainWindow::clearComparisonBaseline()
{
    m_comparisonBaselineMat.release();
    m_comparisonBaselineActive = false;
    if (m_clearComparisonBaselineAction) {
        m_clearComparisonBaselineAction->setEnabled(false);
    }
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        "Comparison baseline cleared",
        QStringLiteral("已清除对比基线")));
    rerunPreview();
}

void MainWindow::addActiveModelStepToPipeline()
{
    if (m_activeDetectionModelPath.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Import or activate a model first",
            QStringLiteral("请先导入或激活模型")));
        return;
    }

    if (!ensureMediaSelectedForOperatorAdd()) {
        return;
    }

    auto modelStep = OnnxModelStepFactory::createStep(
        m_activeDetectionModel,
        m_activeDetectionModelPath,
        m_activeDetectionLabels);
    if (!modelStep) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Failed to create model step",
            QStringLiteral("无法创建模型步骤")));
        return;
    }

    m_pipelineSteps.push_back({m_activeDetectionModel.modelName, modelStep});
    m_selectedPipelineStepIndex = static_cast<int>(m_pipelineSteps.size()) - 1;
    m_pipelinePanel->setPipelineStepNames(pipelineStepNames());
    m_pipelinePanel->setCurrentPipelineStep(m_selectedPipelineStepIndex);
    m_parameterPanel->setStep(modelStep);
    rebuildPipelineStrip();
    rerunPreview();
}

void MainWindow::registerApplicationCommands()
{
    auto& dispatcher = ApplicationContext::instance().commandDispatcher();
    dispatcher.registerCommand("media.open_image", [this] { loadImageFile(); });
    dispatcher.registerCommand("media.open_video", [this] { loadVideoFile(); });
    dispatcher.registerCommand("project.save", [this] { saveProjectToFile(); });
    dispatcher.registerCommand("project.load", [this] { loadProjectFromFile(); });
    dispatcher.registerCommand("pipeline.load_sample", [this] {
        loadSamplePipeline(QStringLiteral("samples/pipelines/basic_blur.json"));
    });
    dispatcher.registerCommand("project.load_sample", [this] {
        loadSampleProject(QStringLiteral("samples/projects/demo_project.json"));
    });
    dispatcher.registerCommand("pipeline.rerun", [this] { rerunPreview(); });
    dispatcher.registerCommand("export.comparison", [this] { exportComparisonResult(); });
    dispatcher.registerCommand("settings.open", [this] { showSystemSettings(); });
    dispatcher.registerCommand("diagnostics.show", [this] { showInspectorPanel(1); });
    dispatcher.registerCommand("welcome.show", [this] { presentQuickStartGuideDialog(); });
}

void MainWindow::refreshWorkbenchPanels()
{
    if (m_modelListPanel) {
        m_modelListPanel->refreshModels();
    }
    if (m_diagnosticsPanel) {
        m_diagnosticsPanel->refresh();
    }
}

void MainWindow::handleParameterReset()
{
    if (m_selectedPipelineStepIndex < 0 || m_selectedPipelineStepIndex >= static_cast<int>(m_pipelineSteps.size())) {
        return;
    }

    const auto& step = m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step;
    if (!step) {
        return;
    }

    const StepSchema schema = step->schema();
    QVariantMap defaults;
    for (const StepParameter& parameter : schema.parameters) {
        defaults.insert(parameter.key, parameter.defaultValue);
    }
    step->setParameterValues(defaults);
    m_parameterPanel->setStep(step);
    rerunPreview();
}

void MainWindow::handleParameterSavePreset()
{
    if (m_selectedPipelineStepIndex < 0 || m_selectedPipelineStepIndex >= static_cast<int>(m_pipelineSteps.size())) {
        return;
    }

    const auto& step = m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step;
    if (!step) {
        return;
    }

    const QString presetName = QInputDialog::getText(
        this,
        localizedText(m_appSettings.languageCode, "Save Preset", QStringLiteral("保存预设")),
        localizedText(m_appSettings.languageCode, "Preset name", QStringLiteral("预设名称")));
    if (presetName.trimmed().isEmpty()) {
        return;
    }

    QString errorMessage;
    if (!ParameterPresetStore::savePreset(step->id(), presetName.trimmed(), step->parameterValues(), &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Saved preset: %1").arg(presetName),
        QStringLiteral("已保存预设: %1").arg(presetName)));
}

void MainWindow::handleParameterLoadPreset()
{
    if (m_selectedPipelineStepIndex < 0 || m_selectedPipelineStepIndex >= static_cast<int>(m_pipelineSteps.size())) {
        return;
    }

    const auto& step = m_pipelineSteps[static_cast<std::size_t>(m_selectedPipelineStepIndex)].step;
    if (!step) {
        return;
    }

    const QStringList presets = ParameterPresetStore::listPresets(step->id());
    if (presets.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "No presets found",
            QStringLiteral("未找到预设")));
        return;
    }

    bool ok = false;
    const QString presetName = QInputDialog::getItem(
        this,
        localizedText(m_appSettings.languageCode, "Load Preset", QStringLiteral("加载预设")),
        localizedText(m_appSettings.languageCode, "Preset", QStringLiteral("预设")),
        presets,
        0,
        false,
        &ok);
    if (!ok || presetName.isEmpty()) {
        return;
    }

    QVariantMap values;
    QString errorMessage;
    if (!ParameterPresetStore::loadPreset(step->id(), presetName, &values, &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    step->setParameterValues(values);
    m_parameterPanel->setStep(step);
    rerunPreview();
}

void MainWindow::updateCursorStatus(const QPointF& imagePoint, const QColor& pixelColor)
{
    if (!m_cursorStatusLabel) {
        return;
    }

    const QString zoomText = m_canvasView
        ? QString("Zoom %1%").arg(static_cast<int>(m_canvasView->zoomFactor() * 100.0))
        : QString();
    m_cursorStatusLabel->setText(QString("(%1, %2) RGB(%3, %4, %5) %6")
        .arg(static_cast<int>(imagePoint.x()))
        .arg(static_cast<int>(imagePoint.y()))
        .arg(pixelColor.red())
        .arg(pixelColor.green())
        .arg(pixelColor.blue())
        .arg(zoomText));
}

void MainWindow::activateModelByName(const QString& modelName)
{
    RegisteredModelPackage package;
    if (!ApplicationContext::instance().modelRegistry().findByName(modelName, &package)) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            QString("Model not found: %1").arg(modelName),
            QStringLiteral("未找到模型: %1").arg(modelName)));
        return;
    }

    m_activeDetectionModel = package.descriptor;
    m_activeDetectionModelPath = package.modelPath;
    m_activeDetectionModelPackageDir = package.packageDir;
    m_activeDetectionLabels = package.labels;
    ApplicationContext::instance().modelRegistry().setActiveModelName(modelName);
    if (m_modelListPanel) {
        m_modelListPanel->refreshModels();
    }
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Active model: %1").arg(modelName),
        QStringLiteral("当前模型: %1").arg(modelName)));
    rerunPreview();
}

void MainWindow::syncPreviewSettingsFromAppSettings()
{
    m_maxPreviewFps = qMax(0, m_appSettings.maxPreviewFps);
    m_previewScale = qBound(0.1, m_appSettings.previewScale, 1.0);
    m_previewFrameStep = qMax(1, m_appSettings.previewFrameStep);
    OpenCvDnnRunner::setPreferredBackend(m_appSettings.dnnBackend);
}

void MainWindow::recordTaskHistory(const TaskDefinition& task)
{
    m_taskHistory.prepend(task);
    while (m_taskHistory.size() > 20) {
        m_taskHistory.removeLast();
    }
    refreshTaskHistoryPanel();
}

void MainWindow::refreshTaskHistoryPanel()
{
    if (m_taskHistoryPanel) {
        m_taskHistoryPanel->setTasks(m_taskHistory);
    }
}

void MainWindow::updateLatestTaskHistory(const TaskProgress& progress)
{
    if (m_taskHistory.isEmpty()) {
        return;
    }

    m_taskHistory[0].state = progress.state;
    m_taskHistory[0].progress = progress;
    refreshTaskHistoryPanel();
}
