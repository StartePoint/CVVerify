QT += core gui widgets
CONFIG += c++17

CVVERIFY_ROOT = $$clean_path($$PWD/..)
isEmpty(CVVERIFY_OPENCV_SDK_ROOT) {
    CVVERIFY_OPENCV_SDK_ROOT = $$clean_path($$CVVERIFY_ROOT/opencvsdk/windows/opencv4.12)
}

INCLUDEPATH += \
    $$CVVERIFY_ROOT/src \
    $$CVVERIFY_OPENCV_SDK_ROOT/include

DEPENDPATH += $$INCLUDEPATH

OPENCV_LIB_DIR = $$clean_path($$CVVERIFY_OPENCV_SDK_ROOT/x64/mingw/lib)

LIBS += \
    -L$$OPENCV_LIB_DIR \
    -lopencv_core4120 \
    -lopencv_imgproc4120 \
    -lopencv_imgcodecs4120 \
    -lopencv_videoio4120 \
    -lopencv_dnn4120 \
    -lopencv_photo4120 \
    -lopencv_features2d4120 \
    -lopencv_flann4120 \
    -lopencv_calib3d4120

OBJECTS_DIR = $$OUT_PWD/.obj
MOC_DIR = $$OUT_PWD/.moc
UI_DIR = $$OUT_PWD/.ui
RCC_DIR = $$OUT_PWD/.rcc
