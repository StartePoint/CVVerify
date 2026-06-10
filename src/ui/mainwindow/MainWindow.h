#pragma once

#include <memory>
#include <vector>

#include <QActionGroup>
#include <QMainWindow>
#include <QMenu>
#include <QPointF>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantMap>

#include "app/AppSettings.h"
#include "core/detection/DetectionModelDescriptor.h"
#include "core/detection/DetectionTypes.h"
#include "core/frame/FramePacket.h"
#include "core/media/MediaTypes.h"
#include "core/operators/OperatorRegistry.h"
#include "core/pipeline/PipelineDefinition.h"

class MediaPanel;
class PipelinePanel;
class ParameterPanel;
class ExportResultsPanel;
class CanvasView;
class OperatorParameterDialog;
class IPipelineStep;
struct ExportResultSummary;
class QLabel;
class QDialog;
class QListWidget;
class QPushButton;
class QFrame;
class QToolButton;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class MainWindowMenuTest;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    struct PipelineStepState
    {
        QString displayName;
        std::shared_ptr<IPipelineStep> step;
    };

    enum class PreviewMode
    {
        FullPipeline,
        SingleNode,
    };

    void loadImageFile();
    void loadVideoFile();
    void requestVideoFrame(int frameIndex);
    void requestPreviousVideoFrame();
    void requestNextVideoFrame();
    void toggleVideoPlayback(bool playing);
    void advanceVideoPlayback();
    void importYoloModel();
    void showSystemSettings();
    void showMediaWorkbench();
    void showPipelineWorkbench();
    void showExportResultsWorkbench();
    void setOperatorWorkbenchVisible(bool visible);
    void setLanguage(const QString& languageCode);
    void previewDetectionOnCurrentImage();
    void exportDetectionImages();
    void exportDetectionVideo();
    void openOperatorParameterDialog(const QString& operatorId, const QString& displayName);
    void addOperatorToPipeline(const QString& operatorId);
    void selectPipelineStep(int index);
    void removePipelineStep(int index);
    void clearPipeline();
    void renamePipelineStep(int index, const QString& name);
    void movePipelineStep(int from, int to);
    void savePipelineToFile();
    void loadPipelineFromFile();
    void updateOperatorParameters(const QVariantMap& values);
    void rerunPreview();
    void createWorkbenchDialogs();
    void createEmbeddedWorkbench();
    void presentDialog(QDialog* dialog);
    void rebuildMenuTexts();
    void rebuildEmbeddedWorkbenchTexts();
    void rebuildPipelineStrip();
    void updateEmbeddedWorkbenchState();
    void setPreviewMode(PreviewMode mode);
    void createMenus();
    void openExportOutputDirectory(const QString& outputDirectory);
    void openExportArtifact(const QString& outputDirectory, const QString& artifactName);
    bool hasSelectedMedia() const;
    bool ensureMediaSelectedForOperatorAdd();
    void cacheCurrentImageForWorkspaceUse(const QString& nextSourceId);
    void syncWorkspaceCachedImageArtifact();
    bool applyPipelineSteps(FramePacket* frame, QString* errorMessage) const;
    bool runDetectionOnCurrentFrame(FramePacket* previewFrame, DetectionFrameResult* detectionResult, QString* errorMessage) const;
    bool processFrame(FramePacket* frame, DetectionFrameResult* detectionResult, QString* errorMessage) const;
    void recordExportSummary(const ExportResultSummary& summary);
    QStringList pipelineStepNames() const;
    PipelineDefinition buildPipelineDefinition() const;
    QString previewModeSummary() const;
    QString pipelineDetailsSummary() const;
    QString formatParameterMap(const std::shared_ptr<IPipelineStep>& step) const;
    QString formatVariantValue(const QVariant& value) const;
    void handleSourceCanvasPointClicked(const QPointF& imagePoint);
    bool currentStepUsesPerspectiveInteraction() const;
    bool currentStepUsesAffineInteraction() const;
    bool currentStepUsesFloodFillInteraction() const;
    bool currentStepUsesGrabCutInteraction() const;
    void syncPerspectivePointsToCurrentFrameArtifacts();
    void syncAffinePointsToCurrentFrameArtifacts();
    void syncFloodFillSeedsToCurrentFrameArtifacts();
    void syncGrabCutRectToCurrentFrameArtifacts();
    void refreshSourceCanvasOverlay();

    Ui::MainWindow* ui;
    MediaPanel* m_mediaPanel = nullptr;
    PipelinePanel* m_pipelinePanel = nullptr;
    ParameterPanel* m_parameterPanel = nullptr;
    ExportResultsPanel* m_exportResultsPanel = nullptr;
    OperatorParameterDialog* m_operatorParameterDialog = nullptr;
    QDialog* m_mediaWorkbenchDialog = nullptr;
    QDialog* m_pipelineWorkbenchDialog = nullptr;
    QDialog* m_exportResultsDialog = nullptr;
    CanvasView* m_sourceCanvasView = nullptr;
    CanvasView* m_canvasView = nullptr;
    QLabel* m_previewInfoLabel = nullptr;
    QLabel* m_previewDetailsLabel = nullptr;
    QLabel* m_previewTitleLabel = nullptr;
    QLabel* m_sourcePreviewTitleLabel = nullptr;
    QLabel* m_resultPreviewTitleLabel = nullptr;
    QFrame* m_operatorWorkbenchPanel = nullptr;
    QLabel* m_operatorWorkbenchTitleLabel = nullptr;
    QLabel* m_pipelineStripTitleLabel = nullptr;
    QListWidget* m_pipelineStripList = nullptr;
    QPushButton* m_removeCurrentStepButton = nullptr;
    QPushButton* m_clearPipelineInlineButton = nullptr;
    QPushButton* m_savePipelineInlineButton = nullptr;
    QPushButton* m_loadPipelineInlineButton = nullptr;
    QPushButton* m_fullPipelinePreviewButton = nullptr;
    QPushButton* m_singleNodePreviewButton = nullptr;
    OperatorRegistry m_operatorRegistry;
    MediaInfo m_currentMediaInfo;
    FramePacket m_currentFrame;
    QTimer* m_videoPlaybackTimer = nullptr;
    std::vector<PipelineStepState> m_pipelineSteps;
    std::vector<QToolButton*> m_operatorQuickAddButtons;
    bool m_updatingPipelineStrip = false;
    int m_selectedPipelineStepIndex = -1;
    int m_currentVideoFrameIndex = 0;
    PreviewMode m_previewMode = PreviewMode::FullPipeline;
    std::shared_ptr<IPipelineStep> m_singlePreviewStep;
    QString m_singlePreviewDisplayName;
    QVector<QPointF> m_perspectivePoints;
    QVector<QPointF> m_affinePoints;
    QVector<QPointF> m_floodFillSeeds;
    QVector<QPointF> m_grabCutRectPoints;
    DetectionModelDescriptor m_activeDetectionModel;
    QString m_activeDetectionModelPath;
    QStringList m_activeDetectionLabels;
    QString m_workspaceCachedImagePath;
    AppSettings m_appSettings;
    QMenu* m_mediaMenu = nullptr;
    QMenu* m_pipelineMenu = nullptr;
    QMenu* m_addOperatorMenu = nullptr;
    QMenu* m_exportMenu = nullptr;
    QMenu* m_playbackMenu = nullptr;
    QMenu* m_previewMenu = nullptr;
    QMenu* m_operatorMenu = nullptr;
    QMenu* m_settingsMenu = nullptr;
    QMenu* m_languageMenu = nullptr;
    QAction* m_openImageAction = nullptr;
    QAction* m_openVideoAction = nullptr;
    QAction* m_importYoloModelAction = nullptr;
    QAction* m_showMediaWorkbenchAction = nullptr;
    QAction* m_showPipelineWorkbenchAction = nullptr;
    QAction* m_showExportResultsAction = nullptr;
    QAction* m_exportDetectionImagesAction = nullptr;
    QAction* m_exportDetectionVideoAction = nullptr;
    QAction* m_fullPipelinePreviewAction = nullptr;
    QAction* m_singleNodePreviewAction = nullptr;
    QAction* m_previousFrameAction = nullptr;
    QAction* m_nextFrameAction = nullptr;
    QAction* m_playPauseAction = nullptr;
    QAction* m_savePipelineAction = nullptr;
    QAction* m_loadPipelineAction = nullptr;
    QAction* m_clearPipelineAction = nullptr;
    QAction* m_removeSelectedPipelineStepAction = nullptr;
    QAction* m_systemSettingsAction = nullptr;
    QAction* m_englishLanguageAction = nullptr;
    QAction* m_chineseLanguageAction = nullptr;
    QActionGroup* m_languageActionGroup = nullptr;
    QActionGroup* m_previewModeActionGroup = nullptr;
};
