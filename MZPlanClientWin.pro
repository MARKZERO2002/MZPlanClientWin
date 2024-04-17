QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DATA/datauntil.cpp \
    DESINER/imageswitch.cpp \
    NETWORK/networkuntil.cpp \
    SERVER/timerserver.cpp \
    SERVER/userserver.cpp \
    SHOW/mzplanclientwin.cpp \
    SHOW/plan.cpp \
    SHOW/plancalenderwidget.cpp \
    SHOW/plandetail.cpp \
    SHOW/planlist.cpp \
    SHOW/remind.cpp \
    main.cpp \

HEADERS += \
    DATA/datauntil.h \
    DATA/shortCut.h \
    DESINER/imageswitch.h \
    NETWORK/networkuntil.h \
    NETWORK/protocol.h \
    SERVER/timerserver.h \
    SERVER/userserver.h \
    SHOW/mzplanclientwin.h \
    SHOW/plan.h \
    SHOW/plancalenderwidget.h \
    SHOW/plandetail.h \
    SHOW/planlist.h \
    SHOW/remind.h

FORMS += \
    SHOW/mzplanclientwin.ui \
    SHOW/plan.ui \
    SHOW/plandetail.ui \
    SHOW/planlist.ui \
    SHOW/remind.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Readme_cn.md \
    MZPlanClientWin.rc

RC_FILE += \
    MZPlanClientWin.rc

RESOURCES += \
    icon.qrc \
    main.qrc
