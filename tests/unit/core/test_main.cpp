#include <QApplication>

int runCorePipelineTests(int argc, char* argv[]);
int runMediaSourcesTests(int argc, char* argv[]);
int runModelPackageLoaderTests(int argc, char* argv[]);
int runOperatorRegistryTests(int argc, char* argv[]);
int runPipelineSerializationTests(int argc, char* argv[]);
int runDetectionRenderComposerTests(int argc, char* argv[]);
int runDetectionExportServiceTests(int argc, char* argv[]);
int runYoloDecoderTests(int argc, char* argv[]);
int runYoloDetectionStepTests(int argc, char* argv[]);
int runOnnxModelStepFactoryTests(int argc, char* argv[]);
int runModelTemplateLoaderTests(int argc, char* argv[]);
int runOperatorSchemaLoaderTests(int argc, char* argv[]);
int runAppDefaultsLoaderTests(int argc, char* argv[]);
int runAppVersionTests(int argc, char* argv[]);
int runCommandDispatcherTests(int argc, char* argv[]);
int runDnnRuntimeInfoTests(int argc, char* argv[]);
int runProjectServiceTests(int argc, char* argv[]);
int runModelRegistryTests(int argc, char* argv[]);
int runMediaPanelTests(int argc, char* argv[]);
int runCanvasViewTests(int argc, char* argv[]);
int runExportResultsPanelTests(int argc, char* argv[]);
int runMainWindowMenuTests(int argc, char* argv[]);
int runSystemSettingsDialogTests(int argc, char* argv[]);
int runTimelineWidgetTests(int argc, char* argv[]);
int runParameterPanelTests(int argc, char* argv[]);
int runPipelinePanelTests(int argc, char* argv[]);
int runYoloModelImportDialogTests(int argc, char* argv[]);
int runIntegrationSmokeTests(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    qputenv("QT_PLUGIN_PATH", "C:/Qt/Qt5.14.2/5.14.2/mingw73_64/plugins");
    QApplication app(argc, argv);
    int status = 0;
    status |= runCorePipelineTests(argc, argv);
    status |= runMediaSourcesTests(argc, argv);
    status |= runModelPackageLoaderTests(argc, argv);
    status |= runOperatorRegistryTests(argc, argv);
    status |= runPipelineSerializationTests(argc, argv);
    status |= runDetectionRenderComposerTests(argc, argv);
    status |= runDetectionExportServiceTests(argc, argv);
    status |= runYoloDecoderTests(argc, argv);
    status |= runYoloDetectionStepTests(argc, argv);
    status |= runOnnxModelStepFactoryTests(argc, argv);
    status |= runModelTemplateLoaderTests(argc, argv);
    status |= runOperatorSchemaLoaderTests(argc, argv);
    status |= runAppDefaultsLoaderTests(argc, argv);
    status |= runAppVersionTests(argc, argv);
    status |= runCommandDispatcherTests(argc, argv);
    status |= runDnnRuntimeInfoTests(argc, argv);
    status |= runProjectServiceTests(argc, argv);
    status |= runModelRegistryTests(argc, argv);
    status |= runMediaPanelTests(argc, argv);
    status |= runCanvasViewTests(argc, argv);
    status |= runExportResultsPanelTests(argc, argv);
    status |= runMainWindowMenuTests(argc, argv);
    status |= runSystemSettingsDialogTests(argc, argv);
    status |= runTimelineWidgetTests(argc, argv);
    status |= runParameterPanelTests(argc, argv);
    status |= runPipelinePanelTests(argc, argv);
    status |= runYoloModelImportDialogTests(argc, argv);
    status |= runIntegrationSmokeTests(argc, argv);
    return status;
}
