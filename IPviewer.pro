QT       += core gui


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Global.cpp \
    drawLines.cpp \
    insertPicture.cpp \
    main.cpp \
    mainwindow.cpp \
    resize.cpp \
    slots.cpp

HEADERS += \
    Global.h \
    graphics.h \
    imagelabel.h \
    mainwindow.h

FORMS += \
    mainwindow.ui
    




INCLUDEPATH += $$PWD/include
LIBS += -L$$PWD/bin
LIBS += -lopencv_core440d \
 -lopencv_core440 \
 -lopencv_highgui440 \
 -lopencv_imgcodecs440d \
 -lopencv_imgcodecs440 \
 -lopencv_imgproc440d \
 -lopencv_imgproc440 \
 -lopencv_videoio440d \
 -lopencv_videoio440 \





# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    pictures.qrc



RC_FILE = app.rc # Добавить файл, где прописана вставка иконки
