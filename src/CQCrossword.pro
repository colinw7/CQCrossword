APPNAME = CQCrossword

include($$(MAKE_DIR)/qt_app.mk)

QT += svg

SOURCES += \
CQCrossword.cpp \

HEADERS += \
CQCrossword.h \

INCLUDEPATH += \

PRE_TARGETDEPS += \

unix:LIBS += \
