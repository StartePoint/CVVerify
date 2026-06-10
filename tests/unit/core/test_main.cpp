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
int runMediaPanelTests(int argc, char* argv[]);
int runCanvasViewTests(int argc, char* argv[]);
int runExportResultsPanelTests(int argc, char* argv[]);
int runMainWindowMenuTests(int argc, char* argv[]);
int runSystemSettingsDialogTests(int argc, char* argv[]);
int runParameterPanelTests(int argc, char* argv[]);
int runPipelinePanelTests(int argc, char* argv[]);
int runYoloModelImportDialogTests(int argc, char* argv[]);

int main(int argc, char* argv[])
{
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
    status |= runMediaPanelTests(argc, argv);
    status |= runCanvasViewTests(argc, argv);
    status |= runExportResultsPanelTests(argc, argv);
    status |= runMainWindowMenuTests(argc, argv);
    status |= runSystemSettingsDialogTests(argc, argv);
    status |= runParameterPanelTests(argc, argv);
    status |= runPipelinePanelTests(argc, argv);
    status |= runYoloModelImportDialogTests(argc, argv);
    return status;
}
