TEMPLATE = app
TARGET = CVVerify

include(qmake/cvverify_common.pri)
include(qmake/cvverify_sources.pri)

SOURCES += \
    $$CVVERIFY_APP_COMMON_SOURCES \
    $$CVVERIFY_APP_ENTRY_SOURCE
HEADERS += $$CVVERIFY_APP_HEADERS
FORMS += $$CVVERIFY_APP_FORMS
RESOURCES += resources/app_resources.qrc

win32 {
    RC_FILE += resources/windows/app_icon.rc
}
