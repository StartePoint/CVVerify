# CVVerify Operator System Expansion Design

## Goal

将当前仅覆盖少量基础 OpenCV 算子的算子体系，升级为覆盖 9 大类能力的统一算子系统，并在保持现有 `OperatorDescriptor + IPipelineStep + ParameterPanel` 架构可演进的前提下，补齐复杂参数、交互输入、多结果输出与 `SURF` 占位策略。

## Confirmed Scope

用户已确认按独立审阅页 `docs/operator-parameter-review.html` 中的算子目录与参数提案继续实现。

已确认的分类目录如下：

1. 形态学处理
2. 滤波
3. 图像增强与变换
4. 二值化
5. 边缘检测
6. 滤镜效果
7. 图像检测
8. 图像分割
9. 特征匹配

## Core Design

### 1. Operator Catalog Layer

每个算子都必须具备以下统一元数据：

- `id`
- `displayName`
- `category`
- `capabilityType`
- `interactionType`
- `resultType`

当前 `OperatorDescriptor` 仅包含 `id / displayName / category / create`，需要扩展。

### 2. Capability Type Layer

能力类型固定分为 4 档：

- `basic_single_io`
  - 单图输入，单图输出，纯静态参数
- `advanced_single_io`
  - 单图输入，单图输出，但参数更复杂或结果包含额外统计
- `interactive_single_input`
  - 需要在原图上执行点选、框选、种子点等交互
- `multi_result_or_multi_input`
  - 需要第二张图，或除处理图外还需要图表、关键点、匹配结果等附加视图

### 3. Interaction Type Layer

交互类型固定分为：

- `none`
- `points_on_source`
- `rect_on_source`
- `seed_points_on_source`
- `second_image_required`
- `mask_edit_on_source`

首阶段只需落地前 5 种，`mask_edit_on_source` 作为 GrabCut 后续增强保留。

### 4. Result Type Layer

结果类型统一抽象为：

- `image_only`
- `image_with_stats`
- `chart_only`
- `image_and_chart`
- `image_and_keypoints`
- `image_and_matches`
- `image_and_mask`

当前主界面只支持原图 / 处理图双预览，因此：

- `chart_only`
- `image_and_chart`
- `image_and_keypoints`
- `image_and_matches`
- `image_and_mask`

都需要后续补充附加结果展示区域或工作台。

## Parameter Schema Evolution

### Existing Model

当前 `StepParameter` 只有：

- `key`
- `displayName`
- `type`
- `defaultValue`
- `minimumValue`
- `maximumValue`
- `choices`

### Expanded Model

需补充以下字段：

- `group`
  - 用于参数分组，如“核设置 / 阈值设置 / 输出设置”
- `step`
  - 数值步进
- `visibleWhen`
  - 条件显示表达式
- `tooltip`
  - 参数解释

### UI Strategy

参数输入继续沿用 `ParameterPanel` 的动态表单路线，但实现分层：

- 第一层：纯表单参数
  - 形态学、滤波、增强、二值化、边缘、部分检测/分割
- 第二层：表单 + 预览画布交互
  - 仿射、透视、GrabCut、浸水填充
- 第三层：表单 + 第二输入区
  - 特征匹配
- 第四层：表单 + 附加结果区
  - 直方图、关键点检测、匹配结果、分割掩码

## SURF Strategy

用户已确认采用以下策略：

- 规范层保留 `SURF` 和 `SURF 特征匹配`
- 当前 SDK 落地允许 `SIFT / ORB / AKAZE` 占位
- 后续若升级到包含 `opencv_contrib/xfeatures2d` 的 SDK，再补真正的 `SURF`

## Category-Level Decisions

### Morphology

保留为单一“家族算子”：

- 操作类型：`erode / dilate / open / close / gradient / tophat / blackhat`
- 核形状：`rect / ellipse / cross`
- 参数：核宽、核高、迭代次数、边界模式、边界值

### Filtering

拆分为独立算子：

- Mean Blur
- Box Filter
- Gaussian Blur
- Median Blur
- Bilateral Filter

### Enhancement & Transform

包含：

- Brightness / Contrast
- Affine Transform
- Polar Transform
- Linear Absolute Transform `|y = kx + b|`
- Perspective Transform
- Histogram Equalization
- Histogram Line
- Histogram Bar

其中：

- `Affine Transform` 和 `Perspective Transform` 为交互型
- `Histogram Line / Bar` 为多结果型

### Thresholding

拆分为独立算子：

- Basic Threshold
- Otsu Threshold
- Triangle Threshold
- Adaptive Threshold

### Edge Detection

拆分为独立算子：

- Sobel
- Canny
- Laplacian
- Scharr

### Filter Effects

包含：

- Color Convert
- Color Map
- Pencil Sketch
- Art Effect
- Fiber Glow
- Emboss
- Special Effect

其中：

- `Fiber Glow`
- `Special Effect`

当前仍带业务定义确认风险，但已形成首版参数提案。

### Detection

包含：

- Contour Detect
- Hough Lines
- Hough Circles
- Corner Detect
- Connected Components
- SURF Keypoints Placeholder
- SIFT Keypoints

### Segmentation

包含：

- K-Means Segmentation
- Flood Fill
- Watershed
- GrabCut
- MeanShift Segmentation

其中：

- `Flood Fill`
- `GrabCut`

为交互型。

### Feature Matching

当前先实现：

- SURF Match Placeholder

实际落地后端先走 `SIFT / ORB / AKAZE`。

## UI Implications

### Main Window / Embedded Workbench

现有“快捷按钮平铺”不再适合所有算子，后续需要逐步转向：

- 分类化可用算子列表
- 支持更多算子而非只显示少量快捷按钮
- 支持交互态提示
- 支持第二输入与附加结果工作区

### Pipeline Panel

当前 `PipelinePanel::setOperators()` 只平铺显示名称，后续建议：

- 按分类分组
- 支持按名称 / 分类搜索
- 支持展示能力标签

## Delivery Strategy

采用“统一规范 + 分三期连续实现”：

### Phase 1

基础能力层：

- 扩展算子元数据
- 扩展参数模型
- 升级现有算子定义
- 实现形态学、滤波、基础增强、基础二值化、基础边缘检测

### Phase 2

分析与可视化层：

- 直方图结果展示
- 轮廓、霍夫、角点、连通域
- SIFT / SURF 占位关键点检测
- 主界面附加结果视图

### Phase 3

交互与高级能力层：

- 仿射 / 透视点选
- Flood Fill 交互
- GrabCut 交互
- K-Means / Watershed / MeanShift
- 特征匹配第二输入工作流

## Risks

### 1. Current UI is not enough for all result types

双图预览结构不够承载：

- 直方图图表
- 关键点附加统计
- 匹配结果双图拼接
- 分割掩码 / 前景图 / 叠加图

### 2. Current parameter model is too weak

不扩展 `StepParameter` 就无法稳定表达：

- 条件显示
- 参数分组
- 步进
- 复杂提示

### 3. Interaction tools do not exist yet

当前主画布不支持：

- 点选控制点
- 框选
- 种子点
- 第二张图输入协同

### 4. SURF cannot be directly implemented in current SDK

当前 SDK 下必须按“规范保留，后端占位”执行。

## Source of Truth

这份设计文档是后续正式实施的规范基线；
具体参数清单以 `docs/operator-parameter-review.html` 为审阅稿来源；
如用户后续逐项修改参数，以最新确认结果更新本设计文档与对应实现计划。
