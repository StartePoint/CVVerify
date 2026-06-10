#include "ui/mainwindow/MainWindow.h"

#include <QDir>
#include <QDesktopServices>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QAbstractItemView>
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
#include <QPushButton>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStatusBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#include <functional>

#include "app/AppSettings.h"
#include "core/detection/DetectionExportService.h"
#include "core/detection/DetectionRenderComposer.h"
#include "core/detection/LabelProvider.h"
#include "core/detection/ModelPackageLoader.h"
#include "core/detection/YoloDetectionStep.h"
#include "core/detection/YoloOnnxImportService.h"
#include "core/media/ImageFileSource.h"
#include "core/media/VideoFileSource.h"
#include "core/operators/BuiltinOperatorBootstrap.h"
#include "core/pipeline/PipelineEngine.h"
#include "core/pipeline/PipelineJsonSerializer.h"
#include "infra/opencv/OpenCvQtImageConverter.h"
#include "ui/dialogs/SystemSettingsDialog.h"
#include "ui/dialogs/OperatorParameterDialog.h"
#include "ui/dialogs/YoloModelImportDialog.h"
#include "ui/panels/ExportResultsPanel.h"
#include "ui/panels/MediaPanel.h"
#include "ui/panels/ParameterPanel.h"
#include "ui/panels/PipelinePanel.h"
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

}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    resize(1920, 1080);
    m_appSettings = AppSettingsStore::load();

    ui->verticalLayout->setContentsMargins(10, 10, 10, 10);
    ui->verticalLayout->setSpacing(8);
    ui->previewLayout->setSpacing(10);
    ui->previewLayout->setStretch(0, 1);
    ui->previewLayout->setStretch(1, 1);
    ui->sourcePanel->setFrameShape(QFrame::StyledPanel);
    ui->resultPanel->setFrameShape(QFrame::StyledPanel);
    ui->statusPanel->setFrameShape(QFrame::StyledPanel);
    ui->statusPanel->setMinimumHeight(150);
    ui->statusPanel->setMaximumHeight(220);
    ui->statusPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

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
    )qss");

    registerBuiltinOperators(m_operatorRegistry);

    auto* sourceLayout = new QVBoxLayout(ui->sourcePanel);
    sourceLayout->setContentsMargins(10, 10, 10, 10);
    sourceLayout->setSpacing(8);
    m_sourcePreviewTitleLabel = new QLabel(ui->sourcePanel);
    QFont sectionTitleFont = m_sourcePreviewTitleLabel->font();
    sectionTitleFont.setPointSize(11);
    sectionTitleFont.setBold(true);
    m_sourcePreviewTitleLabel->setFont(sectionTitleFont);
    sourceLayout->addWidget(m_sourcePreviewTitleLabel);
    m_sourceCanvasView = new CanvasView(ui->sourcePanel);
    sourceLayout->addWidget(m_sourceCanvasView, 1);
    connect(m_sourceCanvasView, &CanvasView::imagePointClicked, this, [this](const QPointF& imagePoint) {
        handleSourceCanvasPointClicked(imagePoint);
    });

    auto* resultLayout = new QVBoxLayout(ui->resultPanel);
    resultLayout->setContentsMargins(10, 10, 10, 10);
    resultLayout->setSpacing(8);
    m_resultPreviewTitleLabel = new QLabel(ui->resultPanel);
    m_resultPreviewTitleLabel->setFont(sectionTitleFont);
    resultLayout->addWidget(m_resultPreviewTitleLabel);
    m_canvasView = new CanvasView(ui->resultPanel);
    resultLayout->addWidget(m_canvasView, 1);

    auto* statusLayout = new QVBoxLayout(ui->statusPanel);
    statusLayout->setContentsMargins(10, 6, 10, 6);
    statusLayout->setSpacing(4);
    m_previewTitleLabel = new QLabel(ui->statusPanel);
    m_previewTitleLabel->setFont(sectionTitleFont);
    statusLayout->addWidget(m_previewTitleLabel);
    m_previewInfoLabel = new QLabel(ui->statusPanel);
    m_previewInfoLabel->setWordWrap(true);
    statusLayout->addWidget(m_previewInfoLabel);
    m_previewDetailsLabel = new QLabel(ui->statusPanel);
    m_previewDetailsLabel->setObjectName("previewDetailsLabel");
    m_previewDetailsLabel->setWordWrap(true);
    m_previewDetailsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_previewDetailsLabel->setMaximumHeight(34);
    statusLayout->addWidget(m_previewDetailsLabel);

    m_videoPlaybackTimer = new QTimer(this);
    m_videoPlaybackTimer->setSingleShot(false);

    createWorkbenchDialogs();
    createEmbeddedWorkbench();
    createMenus();

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

void MainWindow::createWorkbenchDialogs()
{
    m_operatorParameterDialog = new OperatorParameterDialog(this);
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

        auto clonedStep = m_operatorRegistry.create(operatorId);
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

    m_mediaWorkbenchDialog = new QDialog(this);
    m_mediaWorkbenchDialog->setObjectName("mediaWorkbenchDialog");
    m_mediaWorkbenchDialog->setModal(false);
    m_mediaWorkbenchDialog->resize(560, 420);
    auto* mediaLayout = new QVBoxLayout(m_mediaWorkbenchDialog);
    mediaLayout->setContentsMargins(0, 0, 0, 0);
    m_mediaPanel = new MediaPanel(m_mediaWorkbenchDialog);
    mediaLayout->addWidget(m_mediaPanel);

    m_pipelineWorkbenchDialog = new QDialog(this);
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

    m_exportResultsDialog = new QDialog(this);
    m_exportResultsDialog->setObjectName("exportResultsDialog");
    m_exportResultsDialog->setModal(false);
    m_exportResultsDialog->resize(560, 720);
    auto* exportLayout = new QVBoxLayout(m_exportResultsDialog);
    exportLayout->setContentsMargins(0, 0, 0, 0);
    m_exportResultsPanel = new ExportResultsPanel(m_exportResultsDialog);
    exportLayout->addWidget(m_exportResultsPanel);

    m_pipelinePanel->setOperators(m_operatorRegistry.allOperators());
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
    const QList<OperatorDescriptor> descriptors = m_operatorRegistry.allOperators();
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
    ui->verticalLayout->setStretch(1, 1);
    ui->verticalLayout->setStretch(2, 0);

    if (m_parameterPanel) {
        m_parameterPanel->setMinimumHeight(90);
        m_parameterPanel->setMaximumHeight(120);
        m_parameterPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        if (auto* statusLayout = qobject_cast<QVBoxLayout*>(ui->statusPanel->layout())) {
            statusLayout->insertWidget(1, m_parameterPanel, 0);
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

    const QList<OperatorDescriptor> descriptors = m_operatorRegistry.allOperators();
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

void MainWindow::loadImageFile()
{
    const QString dialogTitle = localizedText(m_appSettings.languageCode, "Open Image", QStringLiteral("打开图片"));
    const QString filePath = QFileDialog::getOpenFileName(this, dialogTitle, QString(), "Images (*.png *.jpg *.bmp)");
    if (filePath.isEmpty()) {
        return;
    }

    cacheCurrentImageForWorkspaceUse(filePath);

    ImageFileSource source(filePath);
    FrameReadResult result = source.read({});
    if (!result.success) {
        statusBar()->showMessage(result.errorMessage);
        return;
    }

    m_currentFrame = result.frame;
    syncWorkspaceCachedImageArtifact();
    m_perspectivePoints.clear();
    m_affinePoints.clear();
    m_floodFillSeeds.clear();
    m_grabCutRectPoints.clear();
    m_currentMediaInfo.kind = MediaSourceKind::ImageFile;
    m_currentMediaInfo.sourceId = filePath;
    m_currentMediaInfo.width = m_currentFrame.originalMat.cols;
    m_currentMediaInfo.height = m_currentFrame.originalMat.rows;
    m_currentMediaInfo.frameCount = 1;
    m_currentMediaInfo.fps = 0.0;
    m_currentVideoFrameIndex = 0;
    m_mediaPanel->setVideoNavigationState(false, 0, 0);
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Loaded image: %1").arg(filePath),
        QStringLiteral("已加载图片: %1").arg(filePath)
    ));
    rerunPreview();
}

void MainWindow::loadVideoFile()
{
    const QString dialogTitle = localizedText(m_appSettings.languageCode, "Open Video", QStringLiteral("打开视频"));
    const QString filePath = QFileDialog::getOpenFileName(this, dialogTitle, QString(), "Videos (*.avi *.mp4 *.mov *.mkv)");
    if (filePath.isEmpty()) {
        return;
    }

    cacheCurrentImageForWorkspaceUse(filePath);

    VideoFileSource source(filePath);
    m_currentMediaInfo = source.mediaInfo();

    FrameReadResult result = source.read({0});
    if (!result.success) {
        statusBar()->showMessage(result.errorMessage);
        return;
    }

    m_currentFrame = result.frame;
    syncWorkspaceCachedImageArtifact();
    m_perspectivePoints.clear();
    m_affinePoints.clear();
    m_floodFillSeeds.clear();
    m_grabCutRectPoints.clear();
    m_currentVideoFrameIndex = 0;
    m_mediaPanel->setVideoPlaybackState(false);
    m_mediaPanel->setVideoNavigationState(true, m_currentMediaInfo.frameCount, 0);
    rebuildMenuTexts();
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Loaded video: %1 (%2 frames)").arg(filePath).arg(m_currentMediaInfo.frameCount),
        QStringLiteral("已加载视频: %1 (%2 帧)").arg(filePath).arg(m_currentMediaInfo.frameCount)
    ));
    rerunPreview();
}

void MainWindow::requestVideoFrame(int frameIndex)
{
    if (m_currentMediaInfo.kind != MediaSourceKind::VideoFile || m_currentMediaInfo.sourceId.isEmpty()) {
        return;
    }

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
    rerunPreview();
}

void MainWindow::requestPreviousVideoFrame()
{
    if (m_currentMediaInfo.kind != MediaSourceKind::VideoFile) {
        return;
    }
    if (m_currentVideoFrameIndex <= 0) {
        return;
    }

    requestVideoFrame(m_currentVideoFrameIndex - 1);
}

void MainWindow::requestNextVideoFrame()
{
    if (m_currentMediaInfo.kind != MediaSourceKind::VideoFile) {
        return;
    }
    if (m_currentVideoFrameIndex >= std::max(0, m_currentMediaInfo.frameCount - 1)) {
        return;
    }

    requestVideoFrame(m_currentVideoFrameIndex + 1);
}

void MainWindow::toggleVideoPlayback(bool playing)
{
    if (m_currentMediaInfo.kind != MediaSourceKind::VideoFile) {
        m_mediaPanel->setVideoPlaybackState(false);
        rebuildMenuTexts();
        return;
    }

    if (!playing) {
        m_videoPlaybackTimer->stop();
        m_mediaPanel->setVideoPlaybackState(false);
        rebuildMenuTexts();
        return;
    }

    const int intervalMs = m_currentMediaInfo.fps > 0.0
        ? std::max(1, static_cast<int>(1000.0 / m_currentMediaInfo.fps))
        : 40;

    m_videoPlaybackTimer->start(intervalMs);
    m_mediaPanel->setVideoPlaybackState(true);
    rebuildMenuTexts();
}

void MainWindow::advanceVideoPlayback()
{
    if (m_currentMediaInfo.kind != MediaSourceKind::VideoFile) {
        m_videoPlaybackTimer->stop();
        m_mediaPanel->setVideoPlaybackState(false);
        rebuildMenuTexts();
        return;
    }

    const int nextFrame = m_currentVideoFrameIndex + 1;
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
    YoloModelImportDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const YoloImportRequest request = dialog.buildRequest();
    if (request.modelName.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Model name is required",
            QStringLiteral("模型名称不能为空")
        ));
        return;
    }

    const YoloImportResult importResult = YoloOnnxImportService::buildImportDescriptor(request);
    if (!importResult.success) {
        statusBar()->showMessage(importResult.errorMessage);
        return;
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
    if (!copyImportedFile(request.labelsPath, packagedLabelsPath, &errorMessage)) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    const LabelLoadResult labels = LabelProvider::loadLabels(packagedLabelsPath);
    if (!labels.success) {
        statusBar()->showMessage(labels.errorMessage);
        return;
    }

    m_activeDetectionModel = importResult.descriptor;
    m_activeDetectionModelPath = QFileInfo(packagedModelPath).absoluteFilePath();
    m_activeDetectionLabels = labels.labels;
    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Imported YOLO model: %1").arg(request.modelName),
        QStringLiteral("已导入 YOLO 模型: %1").arg(request.modelName)
    ));
    previewDetectionOnCurrentImage();
}

void MainWindow::addOperatorToPipeline(const QString& operatorId)
{
    if (!ensureMediaSelectedForOperatorAdd()) {
        return;
    }

    auto step = m_operatorRegistry.create(operatorId);
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
    rerunPreview();
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

    QMessageBox::information(
        this,
        localizedText(
            m_appSettings.languageCode,
            "Add Operator",
            QStringLiteral("\u6dfb\u52a0\u7b97\u5b50")
        ),
        localizedText(
            m_appSettings.languageCode,
            "Please select an image or video first",
            QStringLiteral("\u8bf7\u5148\u9009\u62e9\u56fe\u7247\u6216\u89c6\u9891")
        )
    );
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

        if (m_sourceCanvasView) {
            m_sourceCanvasView->setImage(OpenCvQtImageConverter::toQImage(m_currentFrame.originalMat));
            refreshSourceCanvasOverlay();
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

        if (!frameResult.boxes.isEmpty()) {
            DetectionRenderComposer::drawDetections(preview.workingMat, frameResult);
        }

        const QVariant detectionCountVariant = preview.annotations.value("detections");
        const int detectionCount = detectionCountVariant.isValid()
            ? detectionCountVariant.toInt()
            : frameResult.boxes.size();

        if (detectionCountVariant.isValid()) {
            statusBar()->showMessage(localizedText(
                m_appSettings.languageCode,
                QString("Detection preview complete: %1 boxes").arg(detectionCount),
                QStringLiteral("\u68c0\u6d4b\u9884\u89c8\u5b8c\u6210\uff1a%1 \u4e2a\u6846").arg(detectionCount)
            ));
        } else if (m_currentMediaInfo.kind == MediaSourceKind::VideoFile) {
            statusBar()->showMessage(localizedText(
                m_appSettings.languageCode,
                QString("Previewing video frame %1 / %2").arg(m_currentVideoFrameIndex + 1).arg(std::max(1, m_currentMediaInfo.frameCount)),
                QStringLiteral("\u6b63\u5728\u9884\u89c8\u89c6\u9891\u5e27 %1 / %2").arg(m_currentVideoFrameIndex + 1).arg(std::max(1, m_currentMediaInfo.frameCount))
            ));
        }

        const QString mediaName = QFileInfo(m_currentFrame.sourceId).fileName();
        const QString mediaKind = m_currentMediaInfo.kind == MediaSourceKind::VideoFile
            ? localizedText(m_appSettings.languageCode, "Video", QStringLiteral("\u89c6\u9891"))
            : localizedText(m_appSettings.languageCode, "Image", QStringLiteral("\u56fe\u7247"));
        const QString framePart = m_currentMediaInfo.kind == MediaSourceKind::VideoFile
            ? localizedText(
                m_appSettings.languageCode,
                QString("Frame %1 / %2").arg(m_currentVideoFrameIndex + 1).arg(std::max(1, m_currentMediaInfo.frameCount)),
                QStringLiteral("\u5e27 %1 / %2").arg(m_currentVideoFrameIndex + 1).arg(std::max(1, m_currentMediaInfo.frameCount)))
            : localizedText(m_appSettings.languageCode, "Single Frame", QStringLiteral("\u5355\u5e27"));
        const QString detectionPart = localizedText(
            m_appSettings.languageCode,
            QString("Detections: %1").arg(detectionCount),
            QStringLiteral("\u68c0\u6d4b\u6570\uff1a%1").arg(detectionCount)
        );
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

    m_appSettings = dialog.settings();
    AppSettingsStore::save(m_appSettings);
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

    auto step = m_operatorRegistry.create(operatorId);
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
            "Import a YOLO model before exporting detection results",
            QStringLiteral("请先导入 YOLO 模型，再导出检测结果")
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
    if (m_currentMediaInfo.kind != MediaSourceKind::VideoFile || m_currentMediaInfo.sourceId.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Load a video before exporting detection video",
            QStringLiteral("请先加载视频，再导出检测视频")
        ));
        return;
    }

    if (m_activeDetectionModelPath.isEmpty()) {
        statusBar()->showMessage(localizedText(
            m_appSettings.languageCode,
            "Import a YOLO model before exporting detection video",
            QStringLiteral("请先导入 YOLO 模型，再导出检测视频")
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

    QString errorMessage;
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
        exportContext
    );

    if (!success) {
        statusBar()->showMessage(errorMessage);
        return;
    }

    m_appSettings.defaultExportDirectory = outputDir;
    AppSettingsStore::save(m_appSettings);

    statusBar()->showMessage(localizedText(
        m_appSettings.languageCode,
        QString("Exported detection video to: %1").arg(outputDir),
        QStringLiteral("已导出检测视频到: %1").arg(outputDir)
    ));

    ExportResultSummary summary;
    summary.title = localizedText(m_appSettings.languageCode, "Video Export", QStringLiteral("视频导出"));
    summary.summaryText = localizedText(
        m_appSettings.languageCode,
        QString("Exported detection video with %1 frame(s)").arg(std::max(1, m_currentMediaInfo.frameCount)),
        QStringLiteral("已导出包含 %1 帧的视频检测结果").arg(std::max(1, m_currentMediaInfo.frameCount))
    );
    summary.outputDirectory = outputDir;
    summary.artifacts = QStringList()
        << exportFileNameFromSource(m_currentMediaInfo.sourceId, "_overlay.avi")
        << exportFileNameFromSource(m_currentMediaInfo.sourceId, ".json")
        << exportFileNameFromSource(m_currentMediaInfo.sourceId, ".csv")
        << "pipeline_snapshot.json"
        << "model_config_snapshot.json";
    recordExportSummary(summary);

    if (m_appSettings.autoOpenExportDirectory) {
        openExportOutputDirectory(outputDir);
    }
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

    if (!m_pipelineSteps.empty()) {
        PipelineEngine engine;
        std::vector<std::shared_ptr<IPipelineStep>> steps;
        steps.reserve(m_pipelineSteps.size());
        for (const PipelineStepState& state : m_pipelineSteps) {
            steps.push_back(state.step);
        }
        engine.setSteps(std::move(steps));
        const StepResult result = engine.run(*frame, RunContext{true});
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
            *errorMessage = "No active YOLO model is available";
        }
        return false;
    }

    auto detectionStep = std::make_shared<YoloDetectionStep>(
        m_activeDetectionModel,
        m_activeDetectionModelPath,
        m_activeDetectionLabels
    );

    const StepResult detectionStepResult = detectionStep->execute(*previewFrame, RunContext{true});
    if (!detectionStepResult.success) {
        if (errorMessage) {
            *errorMessage = detectionStepResult.errorMessage;
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

    const PipelineLoadResult result = PipelineJsonSerializer::loadFromFile(filePath);
    if (!result.success) {
        statusBar()->showMessage(result.errorMessage);
        return;
    }

    std::vector<PipelineStepState> loadedSteps;
    loadedSteps.reserve(static_cast<std::size_t>(result.definition.steps.size()));

    for (const PipelineStepDefinition& stepDefinition : result.definition.steps) {
        std::shared_ptr<IPipelineStep> step = m_operatorRegistry.create(stepDefinition.stepId);
        if (!step) {
            statusBar()->showMessage(localizedText(
                m_appSettings.languageCode,
                QString("Unknown pipeline step: %1").arg(stepDefinition.stepId),
                QStringLiteral("未知流程步骤: %1").arg(stepDefinition.stepId)
            ));
            return;
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
        QString("Loaded pipeline: %1").arg(filePath),
        QStringLiteral("已加载流程: %1").arg(filePath)
    ));
    rerunPreview();
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
    m_pipelineMenu = menuBar()->addMenu("Pipeline");
    m_pipelineMenu->menuAction()->setObjectName("pipelineMenuAction");
    m_exportMenu = menuBar()->addMenu("Export");
    m_exportMenu->menuAction()->setObjectName("exportMenuAction");
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
    m_openVideoAction = m_mediaMenu->addAction("Open Video", this, [this] { loadVideoFile(); });
    m_importYoloModelAction = m_mediaMenu->addAction("Import YOLO Model", this, [this] { importYoloModel(); });

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

    m_showExportResultsAction = m_exportMenu->addAction("Export Results", this, [this] { showExportResultsWorkbench(); });
    m_showExportResultsAction->setObjectName("showExportResultsAction");
    m_exportMenu->setDefaultAction(m_showExportResultsAction);
    m_exportMenu->addSeparator();
    m_exportDetectionImagesAction = m_exportMenu->addAction("Export Detection Images", this, [this] { exportDetectionImages(); });
    m_exportDetectionVideoAction = m_exportMenu->addAction("Export Detection Video", this, [this] { exportDetectionVideo(); });

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

    const QList<OperatorDescriptor> descriptors = m_operatorRegistry.allOperators();
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
}

void MainWindow::rebuildMenuTexts()
{
    {
        const QString languageCode = m_appSettings.languageCode;
        const QList<OperatorDescriptor> descriptors = m_operatorRegistry.allOperators();

        if (m_mediaMenu) m_mediaMenu->setTitle(localizedText(languageCode, "Media", QStringLiteral("\u5a92\u4f53")));
        if (m_pipelineMenu) m_pipelineMenu->setTitle(localizedText(languageCode, "Pipeline", QStringLiteral("\u6d41\u7a0b")));
        if (m_exportMenu) m_exportMenu->setTitle(localizedText(languageCode, "Export", QStringLiteral("\u5bfc\u51fa")));
        if (m_previewMenu) m_previewMenu->setTitle(localizedText(languageCode, "Preview", QStringLiteral("\u9884\u89c8")));
        if (m_operatorMenu) m_operatorMenu->setTitle(localizedText(languageCode, "Operators", QStringLiteral("\u7b97\u5b50")));
        if (m_playbackMenu) m_playbackMenu->setTitle(localizedText(languageCode, "Playback", QStringLiteral("\u64ad\u653e")));
        if (m_settingsMenu) m_settingsMenu->setTitle(localizedText(languageCode, "Settings", QStringLiteral("\u8bbe\u7f6e")));
        if (m_languageMenu) m_languageMenu->setTitle(localizedText(languageCode, "Language", QStringLiteral("\u8bed\u8a00")));

        if (m_showMediaWorkbenchAction) m_showMediaWorkbenchAction->setText(localizedText(languageCode, "Media & Model Workbench", QStringLiteral("\u5a92\u4f53\u4e0e\u6a21\u578b\u5de5\u4f5c\u53f0")));
        if (m_openImageAction) m_openImageAction->setText(localizedText(languageCode, "Open Image", QStringLiteral("\u6253\u5f00\u56fe\u7247")));
        if (m_openVideoAction) m_openVideoAction->setText(localizedText(languageCode, "Open Video", QStringLiteral("\u6253\u5f00\u89c6\u9891")));
        if (m_importYoloModelAction) m_importYoloModelAction->setText(localizedText(languageCode, "Import YOLO Model", QStringLiteral("\u5bfc\u5165 YOLO \u6a21\u578b")));

        if (m_showPipelineWorkbenchAction) m_showPipelineWorkbenchAction->setText(localizedText(languageCode, "Show Operator Workbench", QStringLiteral("\u663e\u793a\u7b97\u5b50\u5de5\u4f5c\u5e26")));
        if (m_addOperatorMenu) m_addOperatorMenu->setTitle(localizedText(languageCode, "Add Operator", QStringLiteral("\u6dfb\u52a0\u7b97\u5b50")));
        if (m_removeSelectedPipelineStepAction) m_removeSelectedPipelineStepAction->setText(localizedText(languageCode, "Remove Selected Step", QStringLiteral("\u79fb\u9664\u9009\u4e2d\u6b65\u9aa4")));
        if (m_clearPipelineAction) m_clearPipelineAction->setText(localizedText(languageCode, "Clear Pipeline", QStringLiteral("\u6e05\u7a7a\u6d41\u7a0b")));
        if (m_savePipelineAction) m_savePipelineAction->setText(localizedText(languageCode, "Save Pipeline", QStringLiteral("\u4fdd\u5b58\u6d41\u7a0b")));
        if (m_loadPipelineAction) m_loadPipelineAction->setText(localizedText(languageCode, "Load Pipeline", QStringLiteral("\u52a0\u8f7d\u6d41\u7a0b")));

        if (m_showExportResultsAction) m_showExportResultsAction->setText(localizedText(languageCode, "Export Results", QStringLiteral("\u5bfc\u51fa\u7ed3\u679c")));
        if (m_exportDetectionImagesAction) m_exportDetectionImagesAction->setText(localizedText(languageCode, "Export Detection Images", QStringLiteral("\u5bfc\u51fa\u68c0\u6d4b\u56fe\u7247")));
        if (m_exportDetectionVideoAction) m_exportDetectionVideoAction->setText(localizedText(languageCode, "Export Detection Video", QStringLiteral("\u5bfc\u51fa\u68c0\u6d4b\u89c6\u9891")));

        if (m_fullPipelinePreviewAction) m_fullPipelinePreviewAction->setText(localizedText(languageCode, "Full Pipeline Preview", QStringLiteral("\u5168\u6d41\u7a0b\u9884\u89c8")));
        if (m_singleNodePreviewAction) m_singleNodePreviewAction->setText(localizedText(languageCode, "Single Node Preview", QStringLiteral("\u5355\u8282\u70b9\u9884\u89c8")));

        if (m_previousFrameAction) m_previousFrameAction->setText(localizedText(languageCode, "Previous Frame", QStringLiteral("\u4e0a\u4e00\u5e27")));
        if (m_playPauseAction) {
            m_playPauseAction->setText(m_videoPlaybackTimer && m_videoPlaybackTimer->isActive()
                ? localizedText(languageCode, "Pause", QStringLiteral("\u6682\u505c"))
                : localizedText(languageCode, "Play", QStringLiteral("\u64ad\u653e")));
        }
        if (m_nextFrameAction) m_nextFrameAction->setText(localizedText(languageCode, "Next Frame", QStringLiteral("\u4e0b\u4e00\u5e27")));

        if (m_systemSettingsAction) m_systemSettingsAction->setText(localizedText(languageCode, "System Settings", QStringLiteral("\u7cfb\u7edf\u8bbe\u7f6e")));
        if (m_englishLanguageAction) m_englishLanguageAction->setText("English");
        if (m_chineseLanguageAction) m_chineseLanguageAction->setText(QStringLiteral("\u7b80\u4f53\u4e2d\u6587"));

        localizeOperatorMenuTree(m_addOperatorMenu, descriptors, languageCode);
        localizeOperatorMenuTree(m_operatorMenu, descriptors, languageCode);
        rebuildEmbeddedWorkbenchTexts();
        return;
    }
}
