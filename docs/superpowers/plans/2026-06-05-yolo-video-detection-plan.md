# YOLO Video Detection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extend the current YOLO ONNX image-first slice into a real video-processing slice with first-frame preview, reusable frame processing, and offline overlay video export.

**Architecture:** Reuse the existing `FramePacket` pipeline and detection helpers instead of creating a parallel video-only flow. Keep UI orchestration in `MainWindow`, push frame-by-frame export into `DetectionExportService`, and centralize reusable per-frame processing in `MainWindow` helpers so image preview, image export, video preview, and video export all use the same logic.

**Tech Stack:** C++17, Qt Widgets, Qt Test, OpenCV 4.12, OpenCV VideoIO, OpenCV DNN, CMake, MinGW 7.3.0

---

## File Structure Map

- `docs/superpowers/plans/2026-06-05-yolo-video-detection-plan.md`
  - Short execution plan for the video slice
- `src/core/detection/DetectionExportService.h`
  - Add frame-by-frame video export contract
- `src/core/detection/DetectionExportService.cpp`
  - Add overlay video writer and per-frame JSON export
- `src/ui/mainwindow/MainWindow.h`
  - Add reusable frame-processing helpers and video load/export entrypoints
- `src/ui/mainwindow/MainWindow.cpp`
  - Wire real video load, shared processing, and offline video export
- `tests/unit/core/test_detection_export_service.cpp`
  - Add overlay video export coverage
- `tests/unit/core/test_main.cpp`
  - Keep test runner aligned if new entrypoints are added
- `CMakeLists.txt`
  - Ensure new code stays wired into app and test targets

## Task 1: Add failing video export coverage

**Files:**
- Modify: `tests/unit/core/test_detection_export_service.cpp`

- [ ] **Step 1: Add a failing video export test**
- [ ] **Step 2: Run `CVVerifyCoreTests` build and confirm failure because the video export API does not exist yet**

## Task 2: Extend export service for overlay video output

**Files:**
- Modify: `src/core/detection/DetectionExportService.h`
- Modify: `src/core/detection/DetectionExportService.cpp`

- [ ] **Step 1: Add a callback-based video export contract that processes one `FramePacket` at a time**
- [ ] **Step 2: Implement frame loop, overlay writer, and frame-result JSON serialization**
- [ ] **Step 3: Rebuild tests and confirm the new video export test passes**

## Task 3: Wire real video load and shared processing into `MainWindow`

**Files:**
- Modify: `src/ui/mainwindow/MainWindow.h`
- Modify: `src/ui/mainwindow/MainWindow.cpp`

- [ ] **Step 1: Replace the placeholder video-open status message with actual first-frame loading**
- [ ] **Step 2: Centralize per-frame operator + detection processing into shared helpers**
- [ ] **Step 3: Make image export use the shared processing path so exported results match preview behavior**

## Task 4: Wire offline video export from the UI

**Files:**
- Modify: `src/ui/mainwindow/MainWindow.cpp`

- [ ] **Step 1: Replace the video-export placeholder message with a real export flow**
- [ ] **Step 2: Reuse the shared processing helper for each video frame**
- [ ] **Step 3: Surface progress/result status in the status bar**

## Task 5: Verify and commit

**Files:**
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Run `cmake --build build-mingw --target CVVerify CVVerifyCoreTests -- -j4`**
- [ ] **Step 2: Run `.\build-mingw\CVVerifyCoreTests.exe`**
- [ ] **Step 3: Commit focused video-slice changes**
