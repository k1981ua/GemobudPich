QT       += core gui printsupport serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogtableresult.cpp \
    main.cpp \
    mainwindow.cpp \
    analoginputchannel.cpp \
    qcustomplot.cpp \
    dialogcalibr.cpp \
    modbusreader.cpp \
    src_libmodbus/modbus.c \
    src_libmodbus/modbus-data.c \
    src_libmodbus/modbus-rtu.c \
    src_libmodbus/modbus-tcp.c \
    dialogconfig.cpp \
    fmdlg.cpp

HEADERS += \
    dialogtableresult.h \
    mainwindow.h \
    analoginputchannel.h \
    qcustomplot.h \
    dialogcalibr.h \
    modbusreader.h \
    autostopthread.h \
    dialogconfig.h \
    fmdlg.h

FORMS += \
    dialogtableresult.ui \
    mainwindow.ui \
    dialogcalibr.ui \
    dialogconfig.ui \
    fmdlg.ui

RESOURCES += \
    res.qrc \
    AC.qrc

LIBS += -lws2_32


#add ico to windows application
RC_FILE = myapp.rc

OTHER_FILES += \
    myapp.rc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
