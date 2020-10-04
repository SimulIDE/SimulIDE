 ###########################################################################
 #   Copyright (C) 2012   by santiago González                             #
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

TEMPLATE = app

QT += core gui multimedia widgets serialport concurrent script xml svg

win32-g++{
    LIBS += -lws2_32
}

include(./SimulIDE.pri)

QMAKE_CXXFLAGS_DEBUG -= -O
QMAKE_CXXFLAGS_DEBUG -= -O1
QMAKE_CXXFLAGS_DEBUG -= -O2
QMAKE_CXXFLAGS_DEBUG -= -O3
QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE *= -O3

QMAKE_CFLAGS += --std=gnu11

QMAKE_LIBS += -lelf

CONFIG += qt 
CONFIG += warn_on
CONFIG += no_qml_debug
CONFIG *= c++11
CONFIG += lrelease

DEFINES += MAINMODULE_EXPORT=
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

TARGET_NAME = SimulIDE_$$VERSION$$

TARGET = simulide

runLrelease.commands = lrelease $$PWD/resources/translations/simulide.ts \
    $$PWD/resources/translations/simulide_en.ts \
    $$PWD/resources/translations/simulide_es.ts \
    $$PWD/resources/translations/simulide_fr.ts \
    $$PWD/resources/translations/simulide_ru.ts

QMAKE_EXTRA_TARGETS += runLrelease
POST_TARGETDEPS     += runLrelease

RESOURCES = ./src/application.qrc

QM_FILES_INSTALL_PATH = $$PREFIX/share/$$TARGET/translations

data.path = $$PREFIX/share/$$TARGET
data.files = ./resources/data
examples.path = $$PREFIX/share/$$TARGET
examples.files = ./resources/examples
icons.path = $$PREFIX/share
icons.files = ./resources/icons
target.path = $$PREFIX/bin
INSTALLS += data
INSTALLS += examples
INSTALLS += icons
INSTALLS += target
