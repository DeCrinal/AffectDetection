TEMPLATE = app
CONFIG += console c++20
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += /usr/include/opencv4
LIBS += -L/usr/lib/x86_64-linux-gnu -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
SOURCES += \
        areascontainer.cpp \
        imageprocesser.cpp \
        main.cpp

HEADERS += \
    areascontainer.h \
    imageprocesser.h
