TEMPLATE = app
TARGET = CVVerifyCoreTests

QT += testlib
CONFIG += console testcase

include(qmake/cvverify_common.pri)
include(qmake/cvverify_sources.pri)

SOURCES += $$CVVERIFY_APP_COMMON_SOURCES
SOURCES += $$CVVERIFY_TEST_SOURCES

HEADERS += $$CVVERIFY_APP_HEADERS
HEADERS += $$CVVERIFY_TEST_HEADERS
FORMS += $$CVVERIFY_APP_FORMS
