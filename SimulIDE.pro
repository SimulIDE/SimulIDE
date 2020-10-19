 ###########################################################################
 #   Copyright (C) 2012   by Santiago González                             #
 #   santigoro@gmail.com                                                   #
 #                                                                         #
 #   This program is free software; you can redistribute it and/or modify  #
 #   it under the terms of the GNU General Public License as published by  #
 #   the Free Software Foundation; either version 3 of the License, or     #
 #   (at your option) any later version.                                   #
 #                                                                         #
 #   This program is distributed in the hope that it will be useful,       #
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
 #   GNU General Public License for more details.                          #
 #                                                                         #
 #   You should have received a copy of the GNU General Public License     #
 #   along with this program; if not, see <http://www.gnu.org/licenses/>.  #
 #                                                                         #
 ###########################################################################
 
VERSION     = "0.4.13"
RELEASE = "-SR5-CE"

TEMPLATE = app

TARGET = simulide

QT += core gui multimedia widgets serialport concurrent script xml svg

win32-g++{
    OS = Windows
    LIBS += -lws2_32
}
linux {
    OS = Linux
    QMAKE_LFLAGS += -no-pie
}
macx {
    OS = MacOS
    INCLUDEPATH += \
        /usr/local/Cellar/libelf/0.8.13_1/include \
        /usr/local/Cellar/libelf/0.8.13_1/include/libelf
    
    LIBS += /usr/local/lib/libelf.a
    QMAKE_LFLAGS += -no-pie
}

include(./SimulIDE.pri)

QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-missing-field-initializers
QMAKE_CXXFLAGS += -Wno-implicit-fallthrough
QMAKE_CXXFLAGS -= -fPIC
QMAKE_CXXFLAGS += -fno-pic

QMAKE_CFLAGS += --std=gnu11
QMAKE_CFLAGS += -Wno-unused-result
QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-missing-field-initializers
QMAKE_CFLAGS += -Wno-implicit-function-declaration
QMAKE_CFLAGS += -Wno-implicit-fallthrough
QMAKE_CFLAGS += -Wno-int-conversion
QMAKE_CFLAGS += -Wno-sign-compare
QMAKE_CFLAGS += -O2
QMAKE_CFLAGS -= -fPIC
QMAKE_CFLAGS += -fno-pic

QMAKE_LIBS += -lelf

CONFIG += qt 
CONFIG += warn_on
CONFIG += no_qml_debug
CONFIG *= c++11


DEFINES += MAINMODULE_EXPORT=
DEFINES += APP_VERSION=\\\"$$VERSION$$RELEASE\\\"

TARGET_NAME = SimulIDE_$$VERSION$$RELEASE$$

CONFIG( release, debug|release ) {
        TARGET_PREFIX = $$OUT_PWD/release/$$TARGET_NAME
        _OBJECTS_DIR  = $$OUT_PWD/release
}

CONFIG( debug, debug|release ) {
        TARGET_PREFIX = $$OUT_PWD/debug/$$TARGET_NAME
        _OBJECTS_DIR  = $$OUT_PWD/debug
}

OBJECTS_DIR *= $$_OBJECTS_DIR
MOC_DIR     *= $$OUT_PWD/build
INCLUDEPATH += $$MOC_DIR

runLrelease.commands = lrelease $$PWD/resources/translations/simulide.ts \
    $$PWD/resources/translations/simulide_en.ts \
    $$PWD/resources/translations/simulide_es.ts \
    $$PWD/resources/translations/simulide_fr.ts \
    $$PWD/resources/translations/simulide_ru.ts \
    $$PWD/resources/translations/simulide_de.ts \
    $$PWD/resources/translations/simulide_pt_BR.ts

QMAKE_EXTRA_TARGETS += runLrelease
POST_TARGETDEPS     += runLrelease

RESOURCES = ./src/application.qrc

win32 | linux {
    DESTDIR = $$TARGET_PREFIX/bin
    mkpath( $$TARGET_PREFIX/bin )
    copy2dest.commands = \
        $(MKDIR)    $$TARGET_PREFIX/share/simulide/data ; \
        $(MKDIR)    $$TARGET_PREFIX/share/simulide/examples ; \
        $(MKDIR)    $$TARGET_PREFIX/share/simulide/translations ; \
        $(COPY_DIR) $$PWD/resources/data              $$TARGET_PREFIX/share/simulide ; \
        $(COPY_DIR) $$PWD/resources/examples          $$TARGET_PREFIX/share/simulide ; \
        $(COPY_DIR) $$PWD/resources/icons             $$TARGET_PREFIX/share ; \
        $(MOVE)     $$PWD/resources/translations/*.qm $$TARGET_PREFIX/share/simulide/translations ;
}
macx {
    DESTDIR = $$TARGET_PREFIX
    mkpath( $$TARGET_PREFIX/simulide.app )
    copy2dest.commands = \
        $(MKDIR)    $$TARGET_PREFIX/simulide.app/Contents/share/simulide/data ; \
        $(MKDIR)    $$TARGET_PREFIX/simulide.app/Contents/share/simulide/examples ; \
        $(MKDIR)    $$TARGET_PREFIX/simulide.app/Contents/share/simulide/translations ; \
        $(COPY_DIR) $$PWD/resources/data              $$TARGET_PREFIX/simulide.app/Contents/share/simulide ; \
        $(COPY_DIR) $$PWD/resources/examples          $$TARGET_PREFIX/simulide.app/Contents/share/simulide ; \
        $(COPY_DIR) $$PWD/resources/icons             $$TARGET_PREFIX/simulide.app/Contents/share ; \
        $(MOVE)     $$PWD/resources/translations/*.qm $$TARGET_PREFIX/simulide.app/Contents/share/simulide/translations ;
}

QMAKE_EXTRA_TARGETS += copy2dest
POST_TARGETDEPS     += copy2dest

message( "-----------------------------------")
message( "    "                               )
message( "    "$$TARGET_NAME for $$OS         )
message( "    "                               )
message( "      Qt version: "$$QT_VERSION     )
message( "    "                               )
message( "    Destination Folder:"            )
message( $$TARGET_PREFIX                      )
message( "-----------------------------------")
