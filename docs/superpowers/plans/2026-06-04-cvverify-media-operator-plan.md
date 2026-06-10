# CVVerify Media And Operator Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add the first real image-processing vertical slice: OpenCV SDK integration on the MinGW toolchain, image and video media access, a configurable operator registry, and a schema-driven parameter panel wired into preview rendering.

**Architecture:** Keep the modular monolith shape established in the foundation phase. Introduce an `infra/opencv` layer for SDK-bound utilities, a `core/media` layer that turns files into `FramePacket` instances, a `core/operators` layer that produces configurable `IPipelineStep` implementations, and thin UI adapters that render a single-image preview through the existing main window.

**Tech Stack:** C++17, Qt Widgets, Qt Test, CMake, OpenCV 4.x, Qt 5.14.2 + MinGW 7.3.0 64-bit, Git

---

## File Structure Map

- `CMakeLists.txt`
  - Add OpenCV discovery, runtime path support, media/operator sources, and updated test targets
- `docs/build-windows-mingw.md`
  - Record MinGW + OpenCV configuration and runtime path requirements
- `src/core/frame/FramePacket.h`
  - Restore concrete `cv::Mat` storage and frame convenience helpers
- `src/core/pipeline/IPipelineStep.h`
  - Extend configurable step schema and parameter mutation contract
- `src/core/media/MediaTypes.h`
  - Shared media enums and DTOs
- `src/core/media/IMediaSource.h`
  - Interface for frame-producing media sources
- `src/core/media/ImageFileSource.h`
  - Single-image media source declaration
- `src/core/media/ImageFileSource.cpp`
  - Single-image media source implementation
- `src/core/media/VideoFileSource.h`
  - Video media source declaration
- `src/core/media/VideoFileSource.cpp`
  - Video media source implementation
- `src/infra/opencv/OpenCvImageIO.h`
  - OpenCV-backed image/video loading declarations
- `src/infra/opencv/OpenCvImageIO.cpp`
  - OpenCV-backed image/video loading implementation
- `src/infra/opencv/OpenCvQtImageConverter.h`
  - `cv::Mat` to `QImage` conversion declaration
- `src/infra/opencv/OpenCvQtImageConverter.cpp`
  - `cv::Mat` to `QImage` conversion implementation
- `src/core/operators/OperatorDescriptor.h`
  - Operator descriptor and factory contracts
- `src/core/operators/OperatorRegistry.h`
  - Registry declaration
- `src/core/operators/OperatorRegistry.cpp`
  - Registry implementation
- `src/core/operators/BuiltinOperatorBootstrap.h`
  - Builtin operator registration entrypoint
- `src/core/operators/BuiltinOperatorBootstrap.cpp`
  - Builtin operator registration implementation
- `src/core/operators/builtin/GrayscaleOperator.h`
  - Grayscale operator declaration
- `src/core/operators/builtin/GrayscaleOperator.cpp`
  - Grayscale operator implementation
- `src/core/operators/builtin/ResizeOperator.h`
  - Resize operator declaration
- `src/core/operators/builtin/ResizeOperator.cpp`
  - Resize operator implementation
- `src/ui/panels/MediaPanel.h`
  - Add image/video open signals
- `src/ui/panels/MediaPanel.cpp`
  - Add buttons and media actions
- `src/ui/panels/PipelinePanel.h`
  - Add operator list and selection signal
- `src/ui/panels/PipelinePanel.cpp`
  - Display builtin operators
- `src/ui/panels/ParameterPanel.h`
  - Add dynamic schema editor API
- `src/ui/panels/ParameterPanel.cpp`
  - Generate Qt editors from `StepSchema`
- `src/ui/widgets/CanvasView.h`
  - Add `setImage()` API
- `src/ui/widgets/CanvasView.cpp`
  - Render preview pixmap
- `src/ui/mainwindow/MainWindow.h`
  - Add preview workflow state and slots
- `src/ui/mainwindow/MainWindow.cpp`
  - Wire media load, operator selection, parameter changes, and preview refresh
- `tests/unit/core/test_core_pipeline.cpp`
  - Expand core coverage for parameterized operators
- `tests/unit/core/test_media_sources.cpp`
  - Add image and video media access tests
- `tests/unit/core/test_operator_registry.cpp`
  - Add registry and operator execution tests

## Task 1: Integrate OpenCV into the MinGW build and restore concrete frame storage

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `docs/build-windows-mingw.md`
- Modify: `src/core/frame/FramePacket.h`

- [ ] **Step 1: Add OpenCV cache paths and runtime path support**

Replace the build-cache section at the top of `CMakeLists.txt` with:

```cmake
option(CVVERIFY_BUILD_TESTS "Build CVVerify tests" ON)

set(CVVERIFY_QT_RUNTIME_BIN "" CACHE PATH "Qt runtime bin directory for local execution")
set(CVVERIFY_OPENCV_DIR "C:/msys64/mingw64/lib/cmake/opencv4" CACHE PATH "OpenCVConfig.cmake directory")
set(CVVERIFY_OPENCV_RUNTIME_BIN "C:/msys64/mingw64/bin" CACHE PATH "OpenCV runtime bin directory")

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Target platform: Windows first, cross-platform ready")

if(CVVERIFY_OPENCV_DIR)
    list(PREPEND CMAKE_PREFIX_PATH "${CVVERIFY_OPENCV_DIR}")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Test)
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs videoio)
```

- [ ] **Step 2: Link OpenCV to the application and test targets**

Update the target sections in `CMakeLists.txt` to include:

```cmake
target_link_libraries(CVVerify PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    ${OpenCV_LIBS}
)
```

and later for tests:

```cmake
target_link_libraries(CVVerifyCoreTests PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    ${OpenCV_LIBS}
)
```

- [ ] **Step 3: Restore `FramePacket` to concrete `cv::Mat` storage**

Replace `src/core/frame/FramePacket.h` with:

```cpp
#pragma once

#include <QString>
#include <QVariantMap>

#include <opencv2/core/mat.hpp>

struct FramePacket
{
    qint64 frameId = -1;
    qint64 timestampMs = 0;
    QString sourceId;
    cv::Mat originalMat;
    cv::Mat workingMat;
    QVariantMap artifacts;
    QVariantMap tensorOutputs;
    QVariantMap annotations;
    QVariantMap metrics;
    QVariantMap debugTrace;

    bool hasOriginalFrame() const
    {
        return !originalMat.empty();
    }

    bool hasWorkingFrame() const
    {
        return !workingMat.empty();
    }
};
```

- [ ] **Step 4: Teach the MinGW build guide about OpenCV**

Append this configuration command to `docs/build-windows-mingw.md`:

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles" `
  -DCMAKE_PREFIX_PATH=C:/Qt/Qt5.14.2/5.14.2/mingw73_64 `
  -DCMAKE_C_COMPILER=C:/Qt/Qt5.14.2/Tools/mingw730_64/bin/gcc.exe `
  -DCMAKE_CXX_COMPILER=C:/Qt/Qt5.14.2/Tools/mingw730_64/bin/g++.exe `
  -DCMAKE_MAKE_PROGRAM=C:/Qt/Qt5.14.2/Tools/mingw730_64/bin/mingw32-make.exe `
  -DCVVERIFY_QT_RUNTIME_BIN=C:/Qt/Qt5.14.2/5.14.2/mingw73_64/bin `
  -DCVVERIFY_OPENCV_DIR=C:/msys64/mingw64/lib/cmake/opencv4 `
  -DCVVERIFY_OPENCV_RUNTIME_BIN=C:/msys64/mingw64/bin
```

- [ ] **Step 5: Reconfigure and build the current targets**

Run:

```powershell
$env:PATH='C:\Qt\Qt5.14.2\Tools\mingw730_64\bin;C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin;C:\msys64\mingw64\bin;'+$env:PATH
cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/Qt/Qt5.14.2/5.14.2/mingw73_64 -DCMAKE_C_COMPILER=C:/Qt/Qt5.14.2/Tools/mingw730_64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/Qt/Qt5.14.2/Tools/mingw730_64/bin/g++.exe -DCMAKE_MAKE_PROGRAM=C:/Qt/Qt5.14.2/Tools/mingw730_64/bin/mingw32-make.exe -DCVVERIFY_QT_RUNTIME_BIN=C:/Qt/Qt5.14.2/5.14.2/mingw73_64/bin -DCVVERIFY_OPENCV_DIR=C:/msys64/mingw64/lib/cmake/opencv4 -DCVVERIFY_OPENCV_RUNTIME_BIN=C:/msys64/mingw64/bin
cmake --build build-mingw --target CVVerify -- -j4
cmake --build build-mingw --target CVVerifyCoreTests -- -j4
```

Expected:

- Configure succeeds with OpenCV found
- App target builds
- Test target builds

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt docs/build-windows-mingw.md src/core/frame/FramePacket.h
git commit -m "build: integrate opencv into mingw toolchain"
```

## Task 2: Add image and video media access using `FramePacket`

**Files:**
- Create: `src/core/media/MediaTypes.h`
- Create: `src/core/media/IMediaSource.h`
- Create: `src/core/media/ImageFileSource.h`
- Create: `src/core/media/ImageFileSource.cpp`
- Create: `src/core/media/VideoFileSource.h`
- Create: `src/core/media/VideoFileSource.cpp`
- Create: `src/infra/opencv/OpenCvImageIO.h`
- Create: `src/infra/opencv/OpenCvImageIO.cpp`
- Modify: `CMakeLists.txt`
- Create: `tests/unit/core/test_media_sources.cpp`

- [ ] **Step 1: Add media and OpenCV IO files to the application target**

Append these files inside the `CVVerify` source list in `CMakeLists.txt`:

```cmake
        src/core/media/MediaTypes.h
        src/core/media/IMediaSource.h
        src/core/media/ImageFileSource.h
        src/core/media/ImageFileSource.cpp
        src/core/media/VideoFileSource.h
        src/core/media/VideoFileSource.cpp
        src/infra/opencv/OpenCvImageIO.h
        src/infra/opencv/OpenCvImageIO.cpp
```

- [ ] **Step 2: Define shared media DTOs**

Create `src/core/media/MediaTypes.h`:

```cpp
#pragma once

#include <QString>

#include "core/frame/FramePacket.h"

enum class MediaSourceKind
{
    ImageFile,
    VideoFile
};

struct MediaInfo
{
    MediaSourceKind kind = MediaSourceKind::ImageFile;
    QString sourceId;
    int width = 0;
    int height = 0;
    int frameCount = 0;
    double fps = 0.0;
};

struct FrameReadRequest
{
    int frameIndex = 0;
};

struct FrameReadResult
{
    bool success = false;
    QString errorMessage;
    FramePacket frame;
};
```

- [ ] **Step 3: Add the media source interface**

Create `src/core/media/IMediaSource.h`:

```cpp
#pragma once

#include "core/media/MediaTypes.h"

class IMediaSource
{
public:
    virtual ~IMediaSource() = default;

    virtual MediaInfo mediaInfo() const = 0;
    virtual FrameReadResult read(const FrameReadRequest& request) = 0;
};
```

- [ ] **Step 4: Implement OpenCV-backed image and video IO**

Create `src/infra/opencv/OpenCvImageIO.h`:

```cpp
#pragma once

#include <QString>

#include "core/media/MediaTypes.h"

namespace OpenCvImageIO {

FrameReadResult loadImageFile(const QString& filePath);
MediaInfo probeVideoFile(const QString& filePath);
FrameReadResult readVideoFrame(const QString& filePath, int frameIndex);

}
```

Create `src/infra/opencv/OpenCvImageIO.cpp`:

```cpp
#include "infra/opencv/OpenCvImageIO.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

namespace OpenCvImageIO {

FrameReadResult loadImageFile(const QString& filePath)
{
    FrameReadResult result;
    const std::string nativePath = filePath.toStdString();

    cv::Mat image = cv::imread(nativePath, cv::IMREAD_COLOR);
    if (image.empty()) {
        result.errorMessage = QString("Failed to load image: %1").arg(filePath);
        return result;
    }

    result.success = true;
    result.frame.sourceId = filePath;
    result.frame.originalMat = image;
    result.frame.workingMat = image.clone();
    return result;
}

MediaInfo probeVideoFile(const QString& filePath)
{
    MediaInfo info;
    info.kind = MediaSourceKind::VideoFile;
    info.sourceId = filePath;

    cv::VideoCapture capture(filePath.toStdString());
    if (!capture.isOpened()) {
        return info;
    }

    info.width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
    info.height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    info.frameCount = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
    info.fps = capture.get(cv::CAP_PROP_FPS);
    return info;
}

FrameReadResult readVideoFrame(const QString& filePath, int frameIndex)
{
    FrameReadResult result;
    cv::VideoCapture capture(filePath.toStdString());
    if (!capture.isOpened()) {
        result.errorMessage = QString("Failed to open video: %1").arg(filePath);
        return result;
    }

    capture.set(cv::CAP_PROP_POS_FRAMES, frameIndex);

    cv::Mat frame;
    if (!capture.read(frame) || frame.empty()) {
        result.errorMessage = QString("Failed to read frame %1 from %2").arg(frameIndex).arg(filePath);
        return result;
    }

    result.success = true;
    result.frame.frameId = frameIndex;
    result.frame.sourceId = filePath;
    result.frame.originalMat = frame;
    result.frame.workingMat = frame.clone();
    return result;
}

}
```

- [ ] **Step 5: Implement image and video source wrappers**

Create `src/core/media/ImageFileSource.h`:

```cpp
#pragma once

#include "core/media/IMediaSource.h"

class ImageFileSource : public IMediaSource
{
public:
    explicit ImageFileSource(QString filePath);

    MediaInfo mediaInfo() const override;
    FrameReadResult read(const FrameReadRequest& request) override;

private:
    QString m_filePath;
};
```

Create `src/core/media/ImageFileSource.cpp`:

```cpp
#include "core/media/ImageFileSource.h"

#include "infra/opencv/OpenCvImageIO.h"

ImageFileSource::ImageFileSource(QString filePath)
    : m_filePath(std::move(filePath))
{
}

MediaInfo ImageFileSource::mediaInfo() const
{
    MediaInfo info;
    info.kind = MediaSourceKind::ImageFile;
    info.sourceId = m_filePath;
    info.frameCount = 1;
    return info;
}

FrameReadResult ImageFileSource::read(const FrameReadRequest&)
{
    return OpenCvImageIO::loadImageFile(m_filePath);
}
```

Create `src/core/media/VideoFileSource.h`:

```cpp
#pragma once

#include "core/media/IMediaSource.h"

class VideoFileSource : public IMediaSource
{
public:
    explicit VideoFileSource(QString filePath);

    MediaInfo mediaInfo() const override;
    FrameReadResult read(const FrameReadRequest& request) override;

private:
    QString m_filePath;
    MediaInfo m_info;
};
```

Create `src/core/media/VideoFileSource.cpp`:

```cpp
#include "core/media/VideoFileSource.h"

#include "infra/opencv/OpenCvImageIO.h"

VideoFileSource::VideoFileSource(QString filePath)
    : m_filePath(std::move(filePath))
    , m_info(OpenCvImageIO::probeVideoFile(m_filePath))
{
}

MediaInfo VideoFileSource::mediaInfo() const
{
    return m_info;
}

FrameReadResult VideoFileSource::read(const FrameReadRequest& request)
{
    return OpenCvImageIO::readVideoFrame(m_filePath, request.frameIndex);
}
```

- [ ] **Step 6: Add a media-source test binary entry**

Append `tests/unit/core/test_media_sources.cpp` inside the `CVVerifyCoreTests` source list in `CMakeLists.txt`.

Create `tests/unit/core/test_media_sources.cpp`:

```cpp
#include <QtTest>

#include <QTemporaryDir>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "core/media/ImageFileSource.h"
#include "core/media/VideoFileSource.h"

class MediaSourcesTest : public QObject
{
    Q_OBJECT

private slots:
    void imageSourceLoadsGeneratedImage();
    void videoSourceReportsErrorForMissingFile();
};

void MediaSourcesTest::imageSourceLoadsGeneratedImage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString filePath = dir.filePath("sample.png");

    cv::Mat image(6, 8, CV_8UC3, cv::Scalar(10, 20, 30));
    QVERIFY(cv::imwrite(filePath.toStdString(), image));

    ImageFileSource source(filePath);
    FrameReadResult result = source.read({});

    QVERIFY(result.success);
    QCOMPARE(result.frame.originalMat.cols, 8);
    QCOMPARE(result.frame.originalMat.rows, 6);
    QVERIFY(result.frame.hasWorkingFrame());
}

void MediaSourcesTest::videoSourceReportsErrorForMissingFile()
{
    VideoFileSource source("missing-file.avi");
    FrameReadResult result = source.read({0});

    QVERIFY(!result.success);
    QVERIFY(!result.errorMessage.isEmpty());
}
```

- [ ] **Step 7: Rebuild tests and verify they pass**

Run:

```powershell
$env:PATH='C:\Qt\Qt5.14.2\Tools\mingw730_64\bin;C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin;C:\msys64\mingw64\bin;'+$env:PATH
cmake --build build-mingw --target CVVerifyCoreTests -- -j4
ctest --test-dir build-mingw --output-on-failure
```

Expected:

- `CVVerifyCoreTests` rebuilds successfully
- Media tests pass

- [ ] **Step 8: Commit**

```bash
git add CMakeLists.txt src/core/media/MediaTypes.h src/core/media/IMediaSource.h src/core/media/ImageFileSource.h src/core/media/ImageFileSource.cpp src/core/media/VideoFileSource.h src/core/media/VideoFileSource.cpp src/infra/opencv/OpenCvImageIO.h src/infra/opencv/OpenCvImageIO.cpp tests/unit/core/test_media_sources.cpp
git commit -m "feat: add image and video media access layer"
```

## Task 3: Build a configurable operator registry with grayscale and resize operators

**Files:**
- Modify: `src/core/pipeline/IPipelineStep.h`
- Modify: `tests/unit/core/test_core_pipeline.cpp`
- Modify: `CMakeLists.txt`
- Create: `src/core/operators/OperatorDescriptor.h`
- Create: `src/core/operators/OperatorRegistry.h`
- Create: `src/core/operators/OperatorRegistry.cpp`
- Create: `src/core/operators/BuiltinOperatorBootstrap.h`
- Create: `src/core/operators/BuiltinOperatorBootstrap.cpp`
- Create: `src/core/operators/builtin/GrayscaleOperator.h`
- Create: `src/core/operators/builtin/GrayscaleOperator.cpp`
- Create: `src/core/operators/builtin/ResizeOperator.h`
- Create: `src/core/operators/builtin/ResizeOperator.cpp`
- Create: `tests/unit/core/test_operator_registry.cpp`

- [ ] **Step 1: Extend `IPipelineStep` with parameter metadata and value mutation**

Replace `src/core/pipeline/IPipelineStep.h` with:

```cpp
#pragma once

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

#include "core/frame/FramePacket.h"

enum class StepParameterType
{
    Integer,
    Double,
    Boolean,
    String,
    Choice
};

struct StepParameterChoice
{
    QString value;
    QString label;
};

struct StepParameter
{
    QString key;
    QString displayName;
    StepParameterType type = StepParameterType::String;
    QVariant defaultValue;
    QVariant minimumValue;
    QVariant maximumValue;
    QVector<StepParameterChoice> choices;
};

struct StepSchema
{
    QString id;
    QString displayName;
    QVector<StepParameter> parameters;
};

struct RunContext
{
    bool isPreview = true;
};

struct StepResult
{
    bool success = true;
    QString errorMessage;
};

class IPipelineStep
{
public:
    virtual ~IPipelineStep() = default;

    virtual QString id() const = 0;
    virtual QString displayName() const = 0;
    virtual StepSchema schema() const = 0;
    virtual QVariantMap parameterValues() const = 0;
    virtual void setParameterValues(const QVariantMap& values) = 0;
    virtual StepResult execute(FramePacket& frame, const RunContext& context) = 0;
};
```

- [ ] **Step 2: Add the operator registry contracts**

Create `src/core/operators/OperatorDescriptor.h`:

```cpp
#pragma once

#include <functional>
#include <memory>

#include "core/pipeline/IPipelineStep.h"

struct OperatorDescriptor
{
    QString id;
    QString displayName;
    QString category;
    std::function<std::shared_ptr<IPipelineStep>()> create;
};
```

Create `src/core/operators/OperatorRegistry.h`:

```cpp
#pragma once

#include <memory>

#include <QHash>
#include <QList>

#include "core/operators/OperatorDescriptor.h"

class OperatorRegistry
{
public:
    void registerOperator(const OperatorDescriptor& descriptor);
    QList<OperatorDescriptor> allOperators() const;
    std::shared_ptr<IPipelineStep> create(const QString& id) const;

private:
    QHash<QString, OperatorDescriptor> m_descriptors;
};
```

Create `src/core/operators/OperatorRegistry.cpp`:

```cpp
#include "core/operators/OperatorRegistry.h"

void OperatorRegistry::registerOperator(const OperatorDescriptor& descriptor)
{
    m_descriptors.insert(descriptor.id, descriptor);
}

QList<OperatorDescriptor> OperatorRegistry::allOperators() const
{
    return m_descriptors.values();
}

std::shared_ptr<IPipelineStep> OperatorRegistry::create(const QString& id) const
{
    auto it = m_descriptors.find(id);
    if (it == m_descriptors.end()) {
        return {};
    }

    return it->create();
}
```

- [ ] **Step 3: Implement the first builtin operators**

Create `src/core/operators/builtin/GrayscaleOperator.h`:

```cpp
#pragma once

#include "core/pipeline/IPipelineStep.h"

class GrayscaleOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;
};
```

Create `src/core/operators/builtin/GrayscaleOperator.cpp`:

```cpp
#include "core/operators/builtin/GrayscaleOperator.h"

#include <opencv2/imgproc.hpp>

QString GrayscaleOperator::id() const
{
    return "builtin.grayscale";
}

QString GrayscaleOperator::displayName() const
{
    return "Grayscale";
}

StepSchema GrayscaleOperator::schema() const
{
    return {"builtin.grayscale", "Grayscale", {}};
}

QVariantMap GrayscaleOperator::parameterValues() const
{
    return {};
}

void GrayscaleOperator::setParameterValues(const QVariantMap&)
{
}

StepResult GrayscaleOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    if (frame.workingMat.channels() == 1) {
        return {};
    }

    cv::Mat gray;
    cv::cvtColor(frame.workingMat, gray, cv::COLOR_BGR2GRAY);
    frame.workingMat = gray;
    return {};
}
```

Create `src/core/operators/builtin/ResizeOperator.h`:

```cpp
#pragma once

#include "core/pipeline/IPipelineStep.h"

class ResizeOperator : public IPipelineStep
{
public:
    QString id() const override;
    QString displayName() const override;
    StepSchema schema() const override;
    QVariantMap parameterValues() const override;
    void setParameterValues(const QVariantMap& values) override;
    StepResult execute(FramePacket& frame, const RunContext& context) override;

private:
    int m_width = 640;
    int m_height = 480;
};
```

Create `src/core/operators/builtin/ResizeOperator.cpp`:

```cpp
#include "core/operators/builtin/ResizeOperator.h"

#include <opencv2/imgproc.hpp>

QString ResizeOperator::id() const
{
    return "builtin.resize";
}

QString ResizeOperator::displayName() const
{
    return "Resize";
}

StepSchema ResizeOperator::schema() const
{
    return {
        "builtin.resize",
        "Resize",
        {
            {"width", "Width", StepParameterType::Integer, 640, 1, 8192, {}},
            {"height", "Height", StepParameterType::Integer, 480, 1, 8192, {}},
        }
    };
}

QVariantMap ResizeOperator::parameterValues() const
{
    return {
        {"width", m_width},
        {"height", m_height},
    };
}

void ResizeOperator::setParameterValues(const QVariantMap& values)
{
    if (values.contains("width")) {
        m_width = values.value("width").toInt();
    }
    if (values.contains("height")) {
        m_height = values.value("height").toInt();
    }
}

StepResult ResizeOperator::execute(FramePacket& frame, const RunContext&)
{
    if (frame.workingMat.empty()) {
        return {false, "No working frame available"};
    }

    cv::Mat resized;
    cv::resize(frame.workingMat, resized, cv::Size(m_width, m_height));
    frame.workingMat = resized;
    return {};
}
```

- [ ] **Step 4: Add builtin registration**

Create `src/core/operators/BuiltinOperatorBootstrap.h`:

```cpp
#pragma once

#include "core/operators/OperatorRegistry.h"

void registerBuiltinOperators(OperatorRegistry& registry);
```

Create `src/core/operators/BuiltinOperatorBootstrap.cpp`:

```cpp
#include "core/operators/BuiltinOperatorBootstrap.h"

#include "core/operators/builtin/GrayscaleOperator.h"
#include "core/operators/builtin/ResizeOperator.h"

void registerBuiltinOperators(OperatorRegistry& registry)
{
    registry.registerOperator({"builtin.grayscale", "Grayscale", "Color", [] {
        return std::make_shared<GrayscaleOperator>();
    }});

    registry.registerOperator({"builtin.resize", "Resize", "Geometry", [] {
        return std::make_shared<ResizeOperator>();
    }});
}
```

- [ ] **Step 5: Add operator code to the build and tests**

Append these files to the `CVVerify` source list:

```cmake
        src/core/operators/OperatorDescriptor.h
        src/core/operators/OperatorRegistry.h
        src/core/operators/OperatorRegistry.cpp
        src/core/operators/BuiltinOperatorBootstrap.h
        src/core/operators/BuiltinOperatorBootstrap.cpp
        src/core/operators/builtin/GrayscaleOperator.h
        src/core/operators/builtin/GrayscaleOperator.cpp
        src/core/operators/builtin/ResizeOperator.h
        src/core/operators/builtin/ResizeOperator.cpp
```

Append these files to the `CVVerifyCoreTests` source list:

```cmake
        tests/unit/core/test_operator_registry.cpp
        src/core/operators/OperatorRegistry.cpp
        src/core/operators/BuiltinOperatorBootstrap.cpp
        src/core/operators/builtin/GrayscaleOperator.cpp
        src/core/operators/builtin/ResizeOperator.cpp
```

- [ ] **Step 6: Expand tests for operator behavior**

Create `tests/unit/core/test_operator_registry.cpp`:

```cpp
#include <QtTest>

#include <opencv2/core.hpp>

#include "core/operators/BuiltinOperatorBootstrap.h"

class OperatorRegistryTest : public QObject
{
    Q_OBJECT

private slots:
    void registryCreatesBuiltinOperators();
    void grayscaleOperatorConvertsColorFrame();
    void resizeOperatorResizesWorkingFrame();
};

void OperatorRegistryTest::registryCreatesBuiltinOperators()
{
    OperatorRegistry registry;
    registerBuiltinOperators(registry);

    QVERIFY(registry.create("builtin.grayscale") != nullptr);
    QVERIFY(registry.create("builtin.resize") != nullptr);
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
```

- [ ] **Step 7: Rebuild tests and fix the old fake step**

Update `tests/unit/core/test_core_pipeline.cpp` so `FakeStep` implements:

```cpp
QVariantMap parameterValues() const override
{
    return {};
}

void setParameterValues(const QVariantMap&) override
{
}
```

Then run:

```powershell
$env:PATH='C:\Qt\Qt5.14.2\Tools\mingw730_64\bin;C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin;C:\msys64\mingw64\bin;'+$env:PATH
cmake --build build-mingw --target CVVerifyCoreTests -- -j4
ctest --test-dir build-mingw --output-on-failure
```

Expected:

- Registry tests pass
- Existing core pipeline tests still pass

- [ ] **Step 8: Commit**

```bash
git add CMakeLists.txt src/core/pipeline/IPipelineStep.h src/core/operators/OperatorDescriptor.h src/core/operators/OperatorRegistry.h src/core/operators/OperatorRegistry.cpp src/core/operators/BuiltinOperatorBootstrap.h src/core/operators/BuiltinOperatorBootstrap.cpp src/core/operators/builtin/GrayscaleOperator.h src/core/operators/builtin/GrayscaleOperator.cpp src/core/operators/builtin/ResizeOperator.h src/core/operators/builtin/ResizeOperator.cpp tests/unit/core/test_core_pipeline.cpp tests/unit/core/test_operator_registry.cpp
git commit -m "feat: add configurable operator registry"
```

## Task 4: Wire media loading, operator selection, parameter editing, and preview rendering into the UI

**Files:**
- Create: `src/infra/opencv/OpenCvQtImageConverter.h`
- Create: `src/infra/opencv/OpenCvQtImageConverter.cpp`
- Modify: `src/ui/widgets/CanvasView.h`
- Modify: `src/ui/widgets/CanvasView.cpp`
- Modify: `src/ui/panels/MediaPanel.h`
- Modify: `src/ui/panels/MediaPanel.cpp`
- Modify: `src/ui/panels/PipelinePanel.h`
- Modify: `src/ui/panels/PipelinePanel.cpp`
- Modify: `src/ui/panels/ParameterPanel.h`
- Modify: `src/ui/panels/ParameterPanel.cpp`
- Modify: `src/ui/mainwindow/MainWindow.h`
- Modify: `src/ui/mainwindow/MainWindow.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Add the `cv::Mat` to `QImage` converter**

Create `src/infra/opencv/OpenCvQtImageConverter.h`:

```cpp
#pragma once

#include <QImage>

#include <opencv2/core/mat.hpp>

namespace OpenCvQtImageConverter {

QImage toQImage(const cv::Mat& mat);

}
```

Create `src/infra/opencv/OpenCvQtImageConverter.cpp`:

```cpp
#include "infra/opencv/OpenCvQtImageConverter.h"

#include <opencv2/imgproc.hpp>

namespace OpenCvQtImageConverter {

QImage toQImage(const cv::Mat& mat)
{
    if (mat.empty()) {
        return {};
    }

    if (mat.type() == CV_8UC1) {
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8).copy();
    }

    if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    }

    return {};
}

}
```

- [ ] **Step 2: Teach `CanvasView` to render preview images**

Replace `src/ui/widgets/CanvasView.h` with:

```cpp
#pragma once

#include <QImage>
#include <QWidget>

class QLabel;

class CanvasView : public QWidget
{
    Q_OBJECT

public:
    explicit CanvasView(QWidget* parent = nullptr);
    void setImage(const QImage& image);

private:
    QLabel* m_label = nullptr;
};
```

Replace `src/ui/widgets/CanvasView.cpp` with:

```cpp
#include "ui/widgets/CanvasView.h"

#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>

CanvasView::CanvasView(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    m_label = new QLabel("Preview Canvas", this);
    m_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_label);
}

void CanvasView::setImage(const QImage& image)
{
    if (image.isNull()) {
        m_label->setText("Preview Canvas");
        m_label->setPixmap({});
        return;
    }

    m_label->setPixmap(QPixmap::fromImage(image));
}
```

- [ ] **Step 3: Turn `MediaPanel` into an action source**

Replace `src/ui/panels/MediaPanel.h` with:

```cpp
#pragma once

#include <QWidget>

class QPushButton;

class MediaPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MediaPanel(QWidget* parent = nullptr);

signals:
    void openImageRequested();
    void openVideoRequested();

private:
    QPushButton* m_openImageButton = nullptr;
    QPushButton* m_openVideoButton = nullptr;
};
```

Replace `src/ui/panels/MediaPanel.cpp` with:

```cpp
#include "ui/panels/MediaPanel.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

MediaPanel::MediaPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Media Sources", this));

    m_openImageButton = new QPushButton("Open Image", this);
    m_openVideoButton = new QPushButton("Open Video", this);

    connect(m_openImageButton, &QPushButton::clicked, this, &MediaPanel::openImageRequested);
    connect(m_openVideoButton, &QPushButton::clicked, this, &MediaPanel::openVideoRequested);

    layout->addWidget(m_openImageButton);
    layout->addWidget(m_openVideoButton);
    layout->addStretch();
}
```

- [ ] **Step 4: Turn `PipelinePanel` into an operator selector**

Replace `src/ui/panels/PipelinePanel.h` with:

```cpp
#pragma once

#include <QWidget>

class QListWidget;
class QListWidgetItem;

struct OperatorDescriptor;

class PipelinePanel : public QWidget
{
    Q_OBJECT

public:
    explicit PipelinePanel(QWidget* parent = nullptr);
    void setOperators(const QList<OperatorDescriptor>& descriptors);

signals:
    void operatorSelected(const QString& operatorId);

private:
    QListWidget* m_listWidget = nullptr;
};
```

Replace `src/ui/panels/PipelinePanel.cpp` with:

```cpp
#include "ui/panels/PipelinePanel.h"

#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

#include "core/operators/OperatorDescriptor.h"

PipelinePanel::PipelinePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Pipeline Steps", this));

    m_listWidget = new QListWidget(this);
    layout->addWidget(m_listWidget);

    connect(m_listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        emit operatorSelected(item->data(Qt::UserRole).toString());
    });
}

void PipelinePanel::setOperators(const QList<OperatorDescriptor>& descriptors)
{
    m_listWidget->clear();

    for (const auto& descriptor : descriptors) {
        auto* item = new QListWidgetItem(descriptor.displayName, m_listWidget);
        item->setData(Qt::UserRole, descriptor.id);
    }
}
```

- [ ] **Step 5: Build the schema-driven `ParameterPanel`**

Replace `src/ui/panels/ParameterPanel.h` with:

```cpp
#pragma once

#include <memory>

#include <QVariantMap>
#include <QWidget>

class QFormLayout;

class IPipelineStep;

class ParameterPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterPanel(QWidget* parent = nullptr);
    void setStep(const std::shared_ptr<IPipelineStep>& step);

signals:
    void parameterValuesChanged(const QVariantMap& values);

private:
    void rebuildEditors();

    std::shared_ptr<IPipelineStep> m_step;
    QFormLayout* m_formLayout = nullptr;
};
```

Replace `src/ui/panels/ParameterPanel.cpp` with:

```cpp
#include "ui/panels/ParameterPanel.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

#include "core/pipeline/IPipelineStep.h"

ParameterPanel::ParameterPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->addWidget(new QLabel("Parameters", this));

    auto* formHost = new QWidget(this);
    m_formLayout = new QFormLayout(formHost);
    rootLayout->addWidget(formHost);
    rootLayout->addStretch();
}

void ParameterPanel::setStep(const std::shared_ptr<IPipelineStep>& step)
{
    m_step = step;
    rebuildEditors();
}

void ParameterPanel::rebuildEditors()
{
    while (m_formLayout->rowCount() > 0) {
        m_formLayout->removeRow(0);
    }

    if (!m_step) {
        return;
    }

    const StepSchema schema = m_step->schema();
    QVariantMap currentValues = m_step->parameterValues();

    for (const StepParameter& parameter : schema.parameters) {
        if (parameter.type == StepParameterType::Integer) {
            auto* editor = new QSpinBox(this);
            editor->setMinimum(parameter.minimumValue.toInt());
            editor->setMaximum(parameter.maximumValue.toInt());
            editor->setValue(currentValues.value(parameter.key, parameter.defaultValue).toInt());
            connect(editor, qOverload<int>(&QSpinBox::valueChanged), this, [this, parameter](int value) {
                if (!m_step) return;
                QVariantMap values = m_step->parameterValues();
                values.insert(parameter.key, value);
                emit parameterValuesChanged(values);
            });
            m_formLayout->addRow(parameter.displayName, editor);
        }
    }
}
```

- [ ] **Step 6: Wire `MainWindow` into a working single-image preview loop**

Replace `src/ui/mainwindow/MainWindow.h` with:

```cpp
#pragma once

#include <memory>

#include <QMainWindow>

#include "core/frame/FramePacket.h"
#include "core/operators/OperatorRegistry.h"

class MediaPanel;
class PipelinePanel;
class ParameterPanel;
class CanvasView;
class IPipelineStep;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void loadImageFile();
    void selectOperator(const QString& operatorId);
    void updateOperatorParameters(const QVariantMap& values);
    void rerunPreview();

    Ui::MainWindow* ui;
    MediaPanel* m_mediaPanel = nullptr;
    PipelinePanel* m_pipelinePanel = nullptr;
    ParameterPanel* m_parameterPanel = nullptr;
    CanvasView* m_canvasView = nullptr;
    OperatorRegistry m_operatorRegistry;
    FramePacket m_currentFrame;
    std::shared_ptr<IPipelineStep> m_activeStep;
};
```

Replace `src/ui/mainwindow/MainWindow.cpp` with:

```cpp
#include "ui/mainwindow/MainWindow.h"

#include <QFileDialog>
#include <QVBoxLayout>

#include "core/media/ImageFileSource.h"
#include "core/operators/BuiltinOperatorBootstrap.h"
#include "core/pipeline/PipelineEngine.h"
#include "infra/opencv/OpenCvQtImageConverter.h"
#include "ui/panels/MediaPanel.h"
#include "ui/panels/ParameterPanel.h"
#include "ui/panels/PipelinePanel.h"
#include "ui/widgets/CanvasView.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("CVVerify - OpenCV Validation Platform");
    statusBar()->showMessage("Ready: media and operator foundation");

    registerBuiltinOperators(m_operatorRegistry);

    auto* leftLayout = new QVBoxLayout(ui->leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    m_mediaPanel = new MediaPanel(ui->leftPanel);
    m_pipelinePanel = new PipelinePanel(ui->leftPanel);
    leftLayout->addWidget(m_mediaPanel);
    leftLayout->addWidget(m_pipelinePanel);

    auto* centerLayout = new QVBoxLayout(ui->centerPanel);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    m_canvasView = new CanvasView(ui->centerPanel);
    centerLayout->addWidget(m_canvasView);

    auto* rightLayout = new QVBoxLayout(ui->rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    m_parameterPanel = new ParameterPanel(ui->rightPanel);
    rightLayout->addWidget(m_parameterPanel);

    m_pipelinePanel->setOperators(m_operatorRegistry.allOperators());

    connect(m_mediaPanel, &MediaPanel::openImageRequested, this, [this] { loadImageFile(); });
    connect(m_pipelinePanel, &PipelinePanel::operatorSelected, this, [this](const QString& id) { selectOperator(id); });
    connect(m_parameterPanel, &ParameterPanel::parameterValuesChanged, this, [this](const QVariantMap& values) { updateOperatorParameters(values); });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadImageFile()
{
    const QString filePath = QFileDialog::getOpenFileName(this, "Open Image", QString(), "Images (*.png *.jpg *.bmp)");
    if (filePath.isEmpty()) {
        return;
    }

    ImageFileSource source(filePath);
    FrameReadResult result = source.read({});
    if (!result.success) {
        statusBar()->showMessage(result.errorMessage);
        return;
    }

    m_currentFrame = result.frame;
    m_canvasView->setImage(OpenCvQtImageConverter::toQImage(m_currentFrame.workingMat));
    statusBar()->showMessage(QString("Loaded image: %1").arg(filePath));
}

void MainWindow::selectOperator(const QString& operatorId)
{
    m_activeStep = m_operatorRegistry.create(operatorId);
    m_parameterPanel->setStep(m_activeStep);
    rerunPreview();
}

void MainWindow::updateOperatorParameters(const QVariantMap& values)
{
    if (!m_activeStep) {
        return;
    }

    m_activeStep->setParameterValues(values);
    rerunPreview();
}

void MainWindow::rerunPreview()
{
    if (!m_currentFrame.hasOriginalFrame()) {
        return;
    }

    FramePacket preview = m_currentFrame;
    preview.workingMat = preview.originalMat.clone();

    if (m_activeStep) {
        PipelineEngine engine;
        engine.setSteps({m_activeStep});
        StepResult result = engine.run(preview, RunContext{true});
        if (!result.success) {
            statusBar()->showMessage(result.errorMessage);
            return;
        }
    }

    m_canvasView->setImage(OpenCvQtImageConverter::toQImage(preview.workingMat));
}
```

- [ ] **Step 7: Add new UI files to the build**

Append `src/infra/opencv/OpenCvQtImageConverter.h` and `src/infra/opencv/OpenCvQtImageConverter.cpp` to the `CVVerify` source list in `CMakeLists.txt`.

- [ ] **Step 8: Build and manually validate the vertical slice**

Run:

```powershell
$env:PATH='C:\Qt\Qt5.14.2\Tools\mingw730_64\bin;C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin;C:\msys64\mingw64\bin;'+$env:PATH
cmake --build build-mingw --target CVVerify -- -j4
.\build-mingw\CVVerify.exe
```

Manual verification checklist:

- App launches under MinGW
- `Open Image` opens a local image file
- Preview canvas shows the original image
- Selecting `Grayscale` converts preview to grayscale
- Selecting `Resize` shows width/height integer controls
- Changing width/height reruns the preview

- [ ] **Step 9: Commit**

```bash
git add CMakeLists.txt src/infra/opencv/OpenCvQtImageConverter.h src/infra/opencv/OpenCvQtImageConverter.cpp src/ui/widgets/CanvasView.h src/ui/widgets/CanvasView.cpp src/ui/panels/MediaPanel.h src/ui/panels/MediaPanel.cpp src/ui/panels/PipelinePanel.h src/ui/panels/PipelinePanel.cpp src/ui/panels/ParameterPanel.h src/ui/panels/ParameterPanel.cpp src/ui/mainwindow/MainWindow.h src/ui/mainwindow/MainWindow.cpp
git commit -m "feat: wire preview workflow for media and operators"
```

## Task 5: Update docs for the new OpenCV-backed vertical slice

**Files:**
- Modify: `README.md`
- Modify: `CHANGELOG.md`
- Modify: `docs/build-windows-mingw.md`
- Modify: `docs/superpowers/specs/2026-06-04-opencv-validation-platform-design.md`

- [ ] **Step 1: Update README current status**

Replace the `Available now` and `Planned next` sections in `README.md` with:

```markdown
Available now:

- product specification document
- foundation implementation plan and stage-two implementation plan
- runnable Qt application shell
- frame-stream core contracts
- image file loading and video file probing
- builtin grayscale and resize operators
- schema-driven integer parameter editing
- single-image preview workflow

Planned next:

- richer operator set
- full video preview loop
- operator chaining and pipeline persistence
- model import and ONNX inference workflow
```

- [ ] **Step 2: Update changelog**

Append under `## [Unreleased]` in `CHANGELOG.md`:

```markdown
- OpenCV SDK integrated into the MinGW toolchain
- Image and video media access layer added
- Builtin operator registry added
- Single-image preview workflow added
```

- [ ] **Step 3: Extend the MinGW build guide**

Add this runtime note to `docs/build-windows-mingw.md`:

```markdown
## Runtime Note

Before running the app or tests from a fresh shell, make sure `PATH` includes:

- `C:\Qt\Qt5.14.2\Tools\mingw730_64\bin`
- `C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin`
- `C:\msys64\mingw64\bin`
```

- [ ] **Step 4: Sync the spec with concrete implementation choices**

Add this sentence under `## 0. Local Build Baseline` in the design spec:

```markdown
The current validated local SDK pairing is Qt 5.14.2 MinGW with an OpenCV package exposed through `C:/msys64/mingw64/lib/cmake/opencv4`.
```

- [ ] **Step 5: Commit**

```bash
git add README.md CHANGELOG.md docs/build-windows-mingw.md docs/superpowers/specs/2026-06-04-opencv-validation-platform-design.md
git commit -m "docs: document media and operator foundation"
```

## Self-Review

### Spec coverage

- OpenCV-based image processing foundation: covered by Tasks 1 and 2
- Image and video handling: covered by Task 2
- Traditional operator access and parameter exposure: covered by Tasks 3 and 4
- UI preview loop for validation workflow: covered by Task 4
- MinGW-first local developer workflow: covered by Tasks 1 and 5

### Placeholder scan

- No TODO or TBD placeholders remain in executable steps
- Build and test commands are explicit
- UI verification checklist names concrete behaviors

### Type consistency

- `FramePacket`, `FrameReadResult`, `IPipelineStep`, `OperatorRegistry`, and `ParameterPanel` use consistent names across tasks
- `build-mingw` remains the only documented build output directory

## Notes

This plan intentionally stops at a single-image preview vertical slice plus video-file probing. Full video playback, multi-step pipelines, and ONNX inference should be planned after these pieces are stable.
