# CVVerify V1 Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first runnable foundation of CVVerify, including repository skeleton, basic CMake/Qt application structure, core frame-stream interfaces, and the initial validation workflow scaffolding needed for later operator and DNN features.

**Architecture:** Use a modular monolith structure with clear separation between UI, application orchestration, core processing contracts, and infrastructure helpers. Start by implementing the smallest vertical slice that proves the Windows-first but cross-platform-ready desktop architecture, while keeping all processing centered around the frame-stream abstraction.

**Tech Stack:** C++17, Qt Widgets, CMake, OpenCV, OpenCV DNN, GitHub Actions, Qt 5.14.2 + MinGW 7.3.0 (current local baseline)

---

## File Structure Map

These files and directories are expected in the first implementation phase.

- `CMakeLists.txt`
  - Root build configuration for the repository
- `src/main.cpp`
  - Application entry point
- `src/app/ApplicationContext.h`
  - Shared runtime dependency container declarations
- `src/app/ApplicationContext.cpp`
  - Shared runtime dependency container implementation
- `src/core/frame/FramePacket.h`
  - Unified frame-stream carrier type
- `src/core/pipeline/IPipelineStep.h`
  - Core pipeline step contract
- `src/core/pipeline/PipelineDefinition.h`
  - Serializable pipeline definition structures
- `src/core/pipeline/PipelineEngine.h`
  - Pipeline execution engine interface
- `src/core/pipeline/PipelineEngine.cpp`
  - Initial pipeline engine implementation
- `src/ui/mainwindow/MainWindow.h`
  - Main window declaration
- `src/ui/mainwindow/MainWindow.cpp`
  - Main window implementation
- `src/ui/mainwindow/MainWindow.ui`
  - Main window layout file
- `src/ui/panels/MediaPanel.h`
  - Media panel declaration
- `src/ui/panels/MediaPanel.cpp`
  - Media panel implementation
- `src/ui/panels/PipelinePanel.h`
  - Pipeline panel declaration
- `src/ui/panels/PipelinePanel.cpp`
  - Pipeline panel implementation
- `src/ui/panels/ParameterPanel.h`
  - Parameter panel declaration
- `src/ui/panels/ParameterPanel.cpp`
  - Parameter panel implementation
- `src/ui/widgets/CanvasView.h`
  - Core preview canvas declaration
- `src/ui/widgets/CanvasView.cpp`
  - Core preview canvas implementation
- `tests/unit/core/test_frame_packet.cpp`
  - Initial frame packet unit coverage
- `tests/unit/core/test_pipeline_definition.cpp`
  - Initial pipeline definition unit coverage

## Task 1: Establish repository directories and CMake targets

**Files:**
- Modify: `CMakeLists.txt`
- Create: `src/main.cpp`
- Create: `src/app/.gitkeep`
- Create: `src/core/frame/.gitkeep`
- Create: `src/core/pipeline/.gitkeep`
- Create: `src/ui/mainwindow/.gitkeep`
- Create: `src/ui/panels/.gitkeep`
- Create: `src/ui/widgets/.gitkeep`
- Create: `src/infra/.gitkeep`

- [ ] **Step 1: Add the first real CMake target layout**

```cmake
cmake_minimum_required(VERSION 3.21)

project(CVVerify VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CVVERIFY_BUILD_TESTS "Build CVVerify tests" ON)

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Target platform: Windows first, cross-platform ready")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

add_executable(CVVerify
    src/main.cpp
)

target_link_libraries(CVVerify PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
```

- [ ] **Step 2: Add the minimal application entry point**

```cpp
#include <QApplication>
#include <QLabel>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QLabel label("CVVerify bootstrap");
    label.resize(320, 80);
    label.show();

    return app.exec();
}
```

- [ ] **Step 3: Create the source subdirectories tracked by git**

Create these files as empty placeholders:

```text
src/app/.gitkeep
src/core/frame/.gitkeep
src/core/pipeline/.gitkeep
src/ui/mainwindow/.gitkeep
src/ui/panels/.gitkeep
src/ui/widgets/.gitkeep
src/infra/.gitkeep
```

- [ ] **Step 4: Configure the project**

Run: `cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\Qt5.14.2\5.14.2\mingw73_64 -DCMAKE_C_COMPILER=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\gcc.exe -DCMAKE_CXX_COMPILER=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\g++.exe -DCMAKE_MAKE_PROGRAM=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\mingw32-make.exe`

Expected:

- CMake configure succeeds
- Qt6 packages resolve successfully
- `build-mingw/` is generated

- [ ] **Step 5: Build the bootstrap target**

Run: `cmake --build build-mingw`

Expected:

- `CVVerify` target builds successfully

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt src/main.cpp src/app/.gitkeep src/core/frame/.gitkeep src/core/pipeline/.gitkeep src/ui/mainwindow/.gitkeep src/ui/panels/.gitkeep src/ui/widgets/.gitkeep src/infra/.gitkeep
git commit -m "chore: bootstrap Qt application target"
```

## Task 2: Add the main window shell and three-column workspace

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `src/main.cpp`
- Create: `src/ui/mainwindow/MainWindow.h`
- Create: `src/ui/mainwindow/MainWindow.cpp`
- Create: `src/ui/mainwindow/MainWindow.ui`
- Create: `src/ui/panels/MediaPanel.h`
- Create: `src/ui/panels/MediaPanel.cpp`
- Create: `src/ui/panels/PipelinePanel.h`
- Create: `src/ui/panels/PipelinePanel.cpp`
- Create: `src/ui/panels/ParameterPanel.h`
- Create: `src/ui/panels/ParameterPanel.cpp`
- Create: `src/ui/widgets/CanvasView.h`
- Create: `src/ui/widgets/CanvasView.cpp`

- [ ] **Step 1: Update CMake to include UI shell files**

```cmake
qt_add_executable(CVVerify
    src/main.cpp
    src/ui/mainwindow/MainWindow.h
    src/ui/mainwindow/MainWindow.cpp
    src/ui/mainwindow/MainWindow.ui
    src/ui/panels/MediaPanel.h
    src/ui/panels/MediaPanel.cpp
    src/ui/panels/PipelinePanel.h
    src/ui/panels/PipelinePanel.cpp
    src/ui/panels/ParameterPanel.h
    src/ui/panels/ParameterPanel.cpp
    src/ui/widgets/CanvasView.h
    src/ui/widgets/CanvasView.cpp
)
```

- [ ] **Step 2: Replace the label bootstrap with a main window**

```cpp
#include <QApplication>

#include "ui/mainwindow/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
```

- [ ] **Step 3: Create the main window declaration**

```cpp
#pragma once

#include <QMainWindow>

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
    Ui::MainWindow* ui;
};
```

- [ ] **Step 4: Create the main window implementation**

```cpp
#include "ui/mainwindow/MainWindow.h"

#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("CVVerify");
}

MainWindow::~MainWindow()
{
    delete ui;
}
```

- [ ] **Step 5: Create the initial main window UI**

```xml
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>MainWindow</class>
 <widget class="QMainWindow" name="MainWindow">
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>1440</width>
    <height>900</height>
   </rect>
  </property>
  <property name="windowTitle">
   <string>CVVerify</string>
  </property>
  <widget class="QWidget" name="centralwidget">
   <layout class="QHBoxLayout" name="horizontalLayout">
    <item>
     <widget class="QFrame" name="leftPanel"/>
    </item>
    <item>
     <widget class="QFrame" name="centerPanel"/>
    </item>
    <item>
     <widget class="QFrame" name="rightPanel"/>
    </item>
   </layout>
  </widget>
  <widget class="QMenuBar" name="menubar"/>
  <widget class="QStatusBar" name="statusbar"/>
 </widget>
 <resources/>
 <connections/>
</ui>
```

- [ ] **Step 6: Add simple panel/widget classes**

Use this repeated pattern for `MediaPanel`, `PipelinePanel`, `ParameterPanel`, and `CanvasView`:

```cpp
#pragma once

#include <QWidget>

class MediaPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MediaPanel(QWidget* parent = nullptr);
};
```

```cpp
#include "ui/panels/MediaPanel.h"

#include <QVBoxLayout>
#include <QLabel>

MediaPanel::MediaPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Media", this));
    layout->addStretch();
}
```

- [ ] **Step 7: Configure and build again**

Run: `cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\Qt5.14.2\5.14.2\mingw73_64 -DCMAKE_C_COMPILER=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\gcc.exe -DCMAKE_CXX_COMPILER=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\g++.exe -DCMAKE_MAKE_PROGRAM=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\mingw32-make.exe`

Run: `cmake --build build-mingw`

Expected:

- Main window builds successfully
- UI source generation succeeds

- [ ] **Step 8: Launch and verify the shell manually**

Run: `build-mingw\\CVVerify.exe`

Expected:

- Main window opens
- Three-column workspace shell is visible

- [ ] **Step 9: Commit**

```bash
git add CMakeLists.txt src/main.cpp src/ui/mainwindow/MainWindow.h src/ui/mainwindow/MainWindow.cpp src/ui/mainwindow/MainWindow.ui src/ui/panels/MediaPanel.h src/ui/panels/MediaPanel.cpp src/ui/panels/PipelinePanel.h src/ui/panels/PipelinePanel.cpp src/ui/panels/ParameterPanel.h src/ui/panels/ParameterPanel.cpp src/ui/widgets/CanvasView.h src/ui/widgets/CanvasView.cpp
git commit -m "feat: add initial main window shell"
```

## Task 3: Define frame-stream core contracts

**Files:**
- Modify: `CMakeLists.txt`
- Create: `src/core/frame/FramePacket.h`
- Create: `src/core/pipeline/IPipelineStep.h`
- Create: `src/core/pipeline/PipelineDefinition.h`
- Create: `src/core/pipeline/PipelineEngine.h`
- Create: `src/core/pipeline/PipelineEngine.cpp`

- [ ] **Step 1: Add the core files to CMake**

```cmake
target_sources(CVVerify PRIVATE
    src/core/frame/FramePacket.h
    src/core/pipeline/IPipelineStep.h
    src/core/pipeline/PipelineDefinition.h
    src/core/pipeline/PipelineEngine.h
    src/core/pipeline/PipelineEngine.cpp
)
```

- [ ] **Step 2: Create the initial `FramePacket` contract**

```cpp
#pragma once

#include <QString>
#include <QVariant>
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
};
```

- [ ] **Step 3: Create the pipeline step contract**

```cpp
#pragma once

#include <QString>
#include <QVariant>
#include <QVector>

#include "core/frame/FramePacket.h"

struct StepParameter
{
    QString key;
    QString displayName;
    QString type;
    QVariant defaultValue;
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
    virtual StepResult execute(FramePacket& frame, const RunContext& context) = 0;
};
```

- [ ] **Step 4: Create the pipeline definition structures**

```cpp
#pragma once

#include <QString>
#include <QVariantMap>
#include <QVector>

struct PipelineStepDefinition
{
    QString stepId;
    QVariantMap parameters;
};

struct PipelineDefinition
{
    QString pipelineId;
    QString displayName;
    QVector<PipelineStepDefinition> steps;
};
```

- [ ] **Step 5: Create the initial pipeline engine**

```cpp
#pragma once

#include <memory>
#include <vector>

#include "core/pipeline/IPipelineStep.h"

class PipelineEngine
{
public:
    void setSteps(std::vector<std::shared_ptr<IPipelineStep>> steps);
    StepResult run(FramePacket& frame, const RunContext& context) const;

private:
    std::vector<std::shared_ptr<IPipelineStep>> m_steps;
};
```

```cpp
#include "core/pipeline/PipelineEngine.h"

void PipelineEngine::setSteps(std::vector<std::shared_ptr<IPipelineStep>> steps)
{
    m_steps = std::move(steps);
}

StepResult PipelineEngine::run(FramePacket& frame, const RunContext& context) const
{
    for (const auto& step : m_steps) {
        StepResult result = step->execute(frame, context);
        if (!result.success) {
            return result;
        }
    }

    return {};
}
```

- [ ] **Step 6: Build to verify contracts compile cleanly**

Run: `cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\Qt5.14.2\5.14.2\mingw73_64 -DCMAKE_C_COMPILER=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\gcc.exe -DCMAKE_CXX_COMPILER=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\g++.exe -DCMAKE_MAKE_PROGRAM=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\mingw32-make.exe`

Run: `cmake --build build-mingw`

Expected:

- Core contract headers and pipeline engine compile successfully

- [ ] **Step 7: Commit**

```bash
git add CMakeLists.txt src/core/frame/FramePacket.h src/core/pipeline/IPipelineStep.h src/core/pipeline/PipelineDefinition.h src/core/pipeline/PipelineEngine.h src/core/pipeline/PipelineEngine.cpp
git commit -m "feat: add frame-stream core contracts"
```

## Task 4: Introduce test infrastructure for the core contracts

**Files:**
- Modify: `CMakeLists.txt`
- Create: `tests/unit/core/test_frame_packet.cpp`
- Create: `tests/unit/core/test_pipeline_engine.cpp`

- [ ] **Step 1: Extend CMake with a simple test target**

```cmake
include(CTest)

if(CVVERIFY_BUILD_TESTS)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)

    add_executable(CVVerifyCoreTests
        tests/unit/core/test_frame_packet.cpp
        tests/unit/core/test_pipeline_engine.cpp
        src/core/pipeline/PipelineEngine.cpp
    )

    target_include_directories(CVVerifyCoreTests PRIVATE src)

    target_link_libraries(CVVerifyCoreTests PRIVATE
        Qt6::Core
        Qt6::Test
    )

    add_test(NAME CVVerifyCoreTests COMMAND CVVerifyCoreTests)
endif()
```

- [ ] **Step 2: Add the failing frame packet test**

```cpp
#include <QtTest>

#include "core/frame/FramePacket.h"

class FramePacketTest : public QObject
{
    Q_OBJECT

private slots:
    void defaultsAreInitialized();
};

void FramePacketTest::defaultsAreInitialized()
{
    FramePacket frame;

    QCOMPARE(frame.frameId, -1);
    QCOMPARE(frame.timestampMs, 0);
    QVERIFY(frame.sourceId.isEmpty());
    QVERIFY(frame.artifacts.isEmpty());
    QVERIFY(frame.tensorOutputs.isEmpty());
}

QTEST_APPLESS_MAIN(FramePacketTest)
#include "test_frame_packet.moc"
```

- [ ] **Step 3: Add the failing pipeline engine test**

```cpp
#include <QtTest>

#include <memory>

#include "core/pipeline/PipelineEngine.h"

class FakeStep : public IPipelineStep
{
public:
    explicit FakeStep(bool success)
        : m_success(success)
    {
    }

    QString id() const override { return "fake"; }
    QString displayName() const override { return "Fake"; }
    StepSchema schema() const override { return {}; }

    StepResult execute(FramePacket&, const RunContext&) override
    {
        if (m_success) {
            return {};
        }

        return {false, "step failed"};
    }

private:
    bool m_success = true;
};

class PipelineEngineTest : public QObject
{
    Q_OBJECT

private slots:
    void returnsSuccessWhenAllStepsPass();
    void stopsWhenAStepFails();
};

void PipelineEngineTest::returnsSuccessWhenAllStepsPass()
{
    PipelineEngine engine;
    FramePacket frame;

    engine.setSteps({std::make_shared<FakeStep>(true), std::make_shared<FakeStep>(true)});

    StepResult result = engine.run(frame, RunContext{});

    QVERIFY(result.success);
    QVERIFY(result.errorMessage.isEmpty());
}

void PipelineEngineTest::stopsWhenAStepFails()
{
    PipelineEngine engine;
    FramePacket frame;

    engine.setSteps({std::make_shared<FakeStep>(true), std::make_shared<FakeStep>(false)});

    StepResult result = engine.run(frame, RunContext{});

    QVERIFY(!result.success);
    QCOMPARE(result.errorMessage, QString("step failed"));
}

QTEST_APPLESS_MAIN(PipelineEngineTest)
#include "test_pipeline_engine.moc"
```

- [ ] **Step 4: Run tests to verify the target builds and executes**

Run: `cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:\Qt\Qt5.14.2\5.14.2\mingw73_64 -DCMAKE_C_COMPILER=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\gcc.exe -DCMAKE_CXX_COMPILER=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\g++.exe -DCMAKE_MAKE_PROGRAM=C:\Qt\Qt5.14.2\Tools\mingw730_64\bin\mingw32-make.exe`

Run: `cmake --build build-mingw --target CVVerifyCoreTests`

Run: `ctest --test-dir build-mingw --output-on-failure`

Expected:

- Test binary builds
- Tests pass

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt tests/unit/core/test_frame_packet.cpp tests/unit/core/test_pipeline_engine.cpp
git commit -m "test: add core pipeline contract coverage"
```

## Task 5: Wire the UI shell to the frame-stream architecture vocabulary

**Files:**
- Modify: `src/ui/mainwindow/MainWindow.cpp`
- Modify: `src/ui/panels/MediaPanel.cpp`
- Modify: `src/ui/panels/PipelinePanel.cpp`
- Modify: `src/ui/panels/ParameterPanel.cpp`
- Modify: `src/ui/widgets/CanvasView.cpp`

- [ ] **Step 1: Update the main window title and initial status text**

```cpp
#include "ui/mainwindow/MainWindow.h"

#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("CVVerify - OpenCV Validation Platform");
    statusBar()->showMessage("Ready: frame-stream foundation");
}
```

- [ ] **Step 2: Add placeholder labels that reflect actual platform concepts**

Use this pattern in the panel/widget implementations:

```cpp
layout->addWidget(new QLabel("Media Sources", this));
layout->addWidget(new QLabel("Pipeline Steps", this));
layout->addWidget(new QLabel("Parameters", this));
layout->addWidget(new QLabel("Preview Canvas", this));
```

- [ ] **Step 3: Rebuild and manually verify the UI language**

Run: `cmake --build build-mingw`

Run: `build-mingw\\CVVerify.exe`

Expected:

- Main window launches
- Workspace labels reflect product vocabulary instead of generic placeholders

- [ ] **Step 4: Commit**

```bash
git add src/ui/mainwindow/MainWindow.cpp src/ui/panels/MediaPanel.cpp src/ui/panels/PipelinePanel.cpp src/ui/panels/ParameterPanel.cpp src/ui/widgets/CanvasView.cpp
git commit -m "refactor: align ui shell with platform vocabulary"
```

## Task 6: Strengthen repository documentation for implementation kickoff

**Files:**
- Modify: `README.md`
- Modify: `CHANGELOG.md`
- Modify: `docs/superpowers/specs/2026-06-04-opencv-validation-platform-design.md`

- [ ] **Step 1: Update README current status**

Add this section under `## Current Status`:

```markdown
Available now:

- product specification document
- open-source repository skeleton
- local Git repository initialization

Planned next:

- implementation plan execution
- CMake and Qt application bootstrap
- frame-stream core contracts
```

- [ ] **Step 2: Update changelog**

Append this under `## [Unreleased]`:

```markdown
- Added implementation plan for V1 foundation work
```

- [ ] **Step 3: Keep the spec in sync if structure changes during setup**

Re-read:

`docs/superpowers/specs/2026-06-04-opencv-validation-platform-design.md`

Verify that the repository structure section still matches the real repository direction. If a mismatch is found, edit the spec before implementation starts.

- [ ] **Step 4: Commit**

```bash
git add README.md CHANGELOG.md docs/superpowers/specs/2026-06-04-opencv-validation-platform-design.md
git commit -m "docs: prepare repository for implementation kickoff"
```

## Self-Review

### Spec coverage

- Repository skeleton and build system: covered by Tasks 1 and 6
- Unified frame-stream abstraction: covered by Task 3
- R&D-oriented UI shell: covered by Tasks 2 and 5
- Test foundation: covered by Task 4
- Windows-first, cross-platform-ready structure: covered by Tasks 1, 2, and 6

### Placeholder scan

- No TODO/TBD placeholders appear in the actionable task steps
- All code-edit steps contain explicit code blocks
- All verification steps contain exact commands

### Type consistency

- `FramePacket`, `RunContext`, `StepSchema`, `StepResult`, `IPipelineStep`, and `PipelineEngine` use consistent naming across tasks
- Paths and class names are reused consistently across all tasks

## Notes

This plan intentionally covers only the first foundation slice. Traditional operators, ONNX model import, schema-driven parameter editing, batch tasks, and export systems should be planned in later documents once the base shell compiles and runs.
