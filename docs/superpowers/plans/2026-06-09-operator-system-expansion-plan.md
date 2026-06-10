# Operator System Expansion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将 CVVerify 当前少量基础算子升级为覆盖 9 大类能力的统一算子系统，并为复杂参数、交互输入、多结果输出和 SURF 占位策略建立可演进基础。

**Architecture:** 先扩展算子元数据与参数模型，再逐步替换和补齐现有内建算子。实现顺序遵循“基础层先稳住，交互层后接入”的原则：第一阶段只落能直接接进现有单图处理链的能力，同时为后续多结果与交互型算子预埋元数据和 UI 扩展点。

**Tech Stack:** C++17, Qt Widgets 5.14.2, OpenCV 4.12, Qt Test, CMake/qmake 并行维护

---

## File Structure

### Existing files to modify

- `src/core/pipeline/IPipelineStep.h`
  - 扩展参数模型，增加参数分组、步进、条件显示、提示文案
- `src/core/operators/OperatorDescriptor.h`
  - 扩展算子元数据，加入能力类型、交互类型、结果类型
- `src/core/operators/OperatorRegistry.cpp`
  - 后续如需稳定排序或分类支持，在此扩展
- `src/core/operators/BuiltinOperatorBootstrap.cpp`
  - 注册第一阶段新旧算子与分类元数据
- `src/ui/panels/PipelinePanel.cpp`
  - 至少先兼容扩展后的算子元数据和名称本地化
- `src/ui/mainwindow/MainWindow.cpp`
  - 后续接交互提示和多结果扩展
- `tests/unit/core/test_operator_registry.cpp`
  - 补充第一阶段算子能力验证
- `tests/unit/ui/test_pipeline_panel.cpp`
  - 如分类展示或元数据读取变化，补测试

### Existing operator files likely to modify in Phase 1

- `src/core/operators/builtin/MorphologyOperator.h/.cpp`
- `src/core/operators/builtin/GaussianBlurOperator.h/.cpp`
- `src/core/operators/builtin/MedianBlurOperator.h/.cpp`
- `src/core/operators/builtin/ThresholdOperator.h/.cpp`
- `src/core/operators/builtin/CannyOperator.h/.cpp`

### New operator files planned for Phase 1

- `src/core/operators/builtin/MeanBlurOperator.h/.cpp`
- `src/core/operators/builtin/BoxFilterOperator.h/.cpp`
- `src/core/operators/builtin/BilateralFilterOperator.h/.cpp`
- `src/core/operators/builtin/BrightnessContrastOperator.h/.cpp`
- `src/core/operators/builtin/LinearAbsTransformOperator.h/.cpp`
- `src/core/operators/builtin/OtsuThresholdOperator.h/.cpp`
- `src/core/operators/builtin/TriangleThresholdOperator.h/.cpp`
- `src/core/operators/builtin/AdaptiveThresholdOperator.h/.cpp`
- `src/core/operators/builtin/SobelOperator.h/.cpp`
- `src/core/operators/builtin/LaplacianOperator.h/.cpp`
- `src/core/operators/builtin/ScharrOperator.h/.cpp`

### New files planned for later phases

- 直方图结果视图组件
- 交互型画布输入状态组件
- 第二输入媒体工作流组件
- 检测 / 分割 / 匹配高级算子实现文件

---

## Phase Breakdown

### Phase 1: Foundation + Basic Operator Families

目标：

- 扩展参数与算子元数据模型
- 升级现有基础算子参数定义
- 实现第一批可直接落地的基础算子

覆盖范围：

- 形态学处理
- 滤波
- 亮度/对比度与线性变换
- 基础二值化、Otsu、Triangle、自适应二值化
- Sobel、Canny、Laplacian、Scharr

### Phase 2: Visualization + Analysis

目标：

- 增加附加结果视图
- 实现直方图、轮廓、霍夫、角点、连通域、SIFT / SURF 占位关键点

### Phase 3: Interaction + Advanced Segmentation / Matching

目标：

- 增加点选、框选、种子点、第二图输入工作流
- 实现仿射、透视、Flood Fill、GrabCut、K-Means、Watershed、MeanShift、SURF 匹配占位

---

## Immediate Execution Tasks

### Task 1: 扩展参数模型与算子元数据

**Files:**
- Modify: `src/core/pipeline/IPipelineStep.h`
- Modify: `src/core/operators/OperatorDescriptor.h`
- Test: `tests/unit/core/test_operator_registry.cpp`

- [ ] 扩展 `StepParameter`，加入 `group / step / visibleWhen / tooltip`
- [ ] 增加算子能力枚举或字符串常量，覆盖能力类型、交互类型、结果类型
- [ ] 扩展 `OperatorDescriptor` 以承载这些新元数据
- [ ] 补一组最小测试，确保 registry 注册后可读取这些元数据

### Task 2: 升级现有基础算子参数定义

**Files:**
- Modify: `src/core/operators/builtin/MorphologyOperator.h/.cpp`
- Modify: `src/core/operators/builtin/GaussianBlurOperator.h/.cpp`
- Modify: `src/core/operators/builtin/MedianBlurOperator.h/.cpp`
- Modify: `src/core/operators/builtin/ThresholdOperator.h/.cpp`
- Modify: `src/core/operators/builtin/CannyOperator.h/.cpp`
- Test: `tests/unit/core/test_operator_registry.cpp`

- [ ] 形态学算子加入核形状、梯度、顶帽、黑帽
- [ ] 高斯滤波升级为核宽 / 核高 / sigmaX / sigmaY
- [ ] 阈值算子收口为“基础二值化”
- [ ] Canny 参数继续沿用，但补充分组与元数据
- [ ] 更新对应测试，保证新参数仍能正确执行

### Task 3: 实现第一批新增基础算子

**Files:**
- Create: `src/core/operators/builtin/MeanBlurOperator.h/.cpp`
- Create: `src/core/operators/builtin/BoxFilterOperator.h/.cpp`
- Create: `src/core/operators/builtin/BilateralFilterOperator.h/.cpp`
- Create: `src/core/operators/builtin/BrightnessContrastOperator.h/.cpp`
- Create: `src/core/operators/builtin/LinearAbsTransformOperator.h/.cpp`
- Create: `src/core/operators/builtin/OtsuThresholdOperator.h/.cpp`
- Create: `src/core/operators/builtin/TriangleThresholdOperator.h/.cpp`
- Create: `src/core/operators/builtin/AdaptiveThresholdOperator.h/.cpp`
- Create: `src/core/operators/builtin/SobelOperator.h/.cpp`
- Create: `src/core/operators/builtin/LaplacianOperator.h/.cpp`
- Create: `src/core/operators/builtin/ScharrOperator.h/.cpp`
- Modify: `src/core/operators/BuiltinOperatorBootstrap.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/core/test_operator_registry.cpp`

- [ ] 为每个新增基础算子先写最小 failing test
- [ ] 最小实现通过测试
- [ ] 注册到 builtin bootstrap
- [ ] 保持分类与命名对齐已确认审阅页

### Task 4: 让 PipelinePanel 兼容扩展后的算子目录

**Files:**
- Modify: `src/ui/panels/PipelinePanel.cpp`
- Modify: `src/ui/panels/PipelinePanel.h`
- Test: `tests/unit/ui/test_pipeline_panel.cpp`

- [ ] 确保扩展后的 `OperatorDescriptor` 不破坏现有列表渲染
- [ ] 至少保留当前平铺展示可用
- [ ] 为后续分类展示预留数据读取路径

### Task 5: 补正式验证与文档同步

**Files:**
- Modify: `README.md`（如需要）
- Modify: `CHANGELOG.md`
- Update: `docs/operator-parameter-review.html`（若实现中出现已确认偏差）
- Test: `tests/unit/core/test_operator_registry.cpp`
- Test: `tests/unit/ui/test_pipeline_panel.cpp`

- [ ] 更新变更说明
- [ ] 明确当前已实现算子范围
- [ ] 记录未进入 Phase 1 的交互型与多结果型能力

---

## Verification Strategy

优先验证：

- `tests/unit/core/test_operator_registry.cpp`
  - 新算子可创建
  - 关键参数可生效
  - 输出类型基本符合预期
- `tests/unit/ui/test_pipeline_panel.cpp`
  - 算子目录仍可显示和添加

若 `build-mingw` 继续受现有 AutoMoc 问题影响，则需明确记录为“环境验证受限”，但不阻塞代码层级自检与最小构建检查。

---

## Execution Choice

用户已经明确希望按已确认文档继续实现，因此当前线程默认采用 **Inline Execution**，直接从 Phase 1 开始落地，不停留在纯计划阶段。
