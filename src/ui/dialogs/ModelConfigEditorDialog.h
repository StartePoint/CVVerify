#pragma once

#include <opencv2/core/mat.hpp>

#include <QDialog>

#include "core/detection/DetectionModelDescriptor.h"

class QTabWidget;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;
class QLabel;
class QTimer;
class CanvasView;

class ModelConfigEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModelConfigEditorDialog(QWidget* parent = nullptr);

    void setDescriptor(const DetectionModelDescriptor& descriptor);
    DetectionModelDescriptor descriptor() const;
    void setPreviewContext(const QString& modelPath, const QStringList& labels, const cv::Mat& previewImage);
    void setLanguage(const QString& languageCode);

private slots:
    void handleSaveTemplate();
    void handleLoadTemplate();
    void handleRunPreview();
    void schedulePreviewRefresh();

private:
    void rebuildFromDescriptor(const DetectionModelDescriptor& descriptor);
    bool saveDescriptorAsTemplate(const DetectionModelDescriptor& descriptor, QString* errorMessage) const;
    void runPreviewInference();
    void connectPreviewSignals();
    void rebuildTexts();

    QTabWidget* m_tabs = nullptr;
    QLineEdit* m_modelNameEdit = nullptr;
    QLineEdit* m_taskTypeEdit = nullptr;
    QLineEdit* m_frameworkEdit = nullptr;
    QLineEdit* m_labelsPathEdit = nullptr;
    QSpinBox* m_inputWidthSpin = nullptr;
    QSpinBox* m_inputHeightSpin = nullptr;
    QSpinBox* m_inputChannelsSpin = nullptr;
    QComboBox* m_colorOrderCombo = nullptr;
    QCheckBox* m_letterboxCheck = nullptr;
    QCheckBox* m_keepRatioCheck = nullptr;
    QDoubleSpinBox* m_scaleSpin = nullptr;
    QDoubleSpinBox* m_confidenceSpin = nullptr;
    QDoubleSpinBox* m_nmsSpin = nullptr;
    QSpinBox* m_maxDetectionsSpin = nullptr;
    QLineEdit* m_layoutTypeEdit = nullptr;
    QPushButton* m_saveTemplateButton = nullptr;
    QPushButton* m_loadTemplateButton = nullptr;
    CanvasView* m_previewCanvas = nullptr;
    QLabel* m_previewStatusLabel = nullptr;
    QPushButton* m_runPreviewButton = nullptr;
    QTimer* m_previewDebounceTimer = nullptr;
    QString m_previewModelPath;
    QStringList m_previewLabels;
    cv::Mat m_previewSourceImage;
    QString m_languageCode = "en";
};
