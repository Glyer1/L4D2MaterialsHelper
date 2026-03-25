QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    componentsetting.cpp \
    diffconfigdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    materialsetting.cpp \
    processors/texturepreprocessor.cpp \
    processors/vmtgenerator.cpp \
    processors/vpkgenerator.cpp \
    processors/vtfgenerator.cpp \
    variantconfigdialog.cpp

HEADERS += \
    componentsetting.h \
    datastructures.h \
    diffconfigdialog.h \
    mainwindow.h \
    materialsetting.h \
    processors/texturepreprocessor.h \
    processors/vmtgenerator.h \
    processors/vpkgenerator.h \
    processors/vtfgenerator.h \
    utils/fileutils.h \
    variantconfigdialog.h

FORMS += \
    componentsetting.ui \
    diffconfigdialog.ui \
    mainwindow.ui \
    materialsetting.ui \
    variantconfigdialog.ui

TRANSLATIONS += \
    L4D2MaterialsHelper_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# ===== FreeImage 库配置 =====
INCLUDEPATH += $$PWD/thirdparty/FreeImage
LIBS += -L$$PWD/thirdparty/FreeImage -lFreeImage

RC_ICONS = icon.ico

RESOURCES += \
    Resources.qrc
