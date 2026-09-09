QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    src/core/basecomponent.cpp \
    src/core/settingsmanager.cpp \
    src/core/thememanager.cpp \
    src/core/widgetconfigmanager.cpp \
    src/ui/imagecontrolpanel.cpp \
    src/ui/imagesettingsform.cpp \
    src/widgets/imagewidget.cpp

HEADERS += \
    src/core/basecomponent.h \
    src/core/settingsmanager.h \
    src/core/thememanager.h \
    src/core/widgetconfigmanager.h \
    src/ui/imagecontrolpanel.h \
    src/ui/imagesettingsform.h \
    src/widgets/imagewidget.h

FORMS += \
    src/ui/imagecontrolpanel.ui \
    src/ui/imagesettingsform.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    sources.qrc

RC_ICONS = app_icon.ico
