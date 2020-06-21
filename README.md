# SimulIDE 0.3.10

Electronic Circuit Simulator


SimulIDE is a simple real time electronic circuit simulator.

It's intended for general purpose electronics and microcontroller simulation, supporting PIC, AVR and Arduino.

PIC simulation is provided by gpsim and avr simulation by simavr.

This is not an accurate simulator for circuit analisis, it aims to be the fast, simple and easy to use, so this means simple and not very accurate electronic models and limited features.

Intended for hobbist or students to learn and experiment with simple circuits.


SimulIDE also features a code Editor and Debugger for GcBasic, Arduino, PIC asm and AVR asm.
Editor/Debugger is still in it's firsts stages of development, with basic functionalities, but it is possible to write, compile and basic debugging with breakpoints, watch registers and global variables.


## Building SimulIDE:

Build dependencies:

 - Qt5 dev packages
 - Qt5Core
 - Qt5Gui
 - Qt5Xml
 - Qt5Widgets
 - Qt5Concurrent
 - Qt5svg dev
 - Qt5 Multimedia dev
 - Qt5 Serialport dev
 - Qt5 Script
 - Qt5 qmake
 - libelf dev
 - gcc-avr
 - avr-libc

 
Once installed go to build_XX folder, then:

```
$ qmake
$ make
```

In folder build_XX/release/SimulIDE_x.x.x you will find executable and all files needed to run SimulIDE.



## Running SimulIDE:

Run time dependencies:

 - Qt5Core
 - Qt5Gui
 - Qt5Xml
 - Qt5svg
 - Qt5Widgets
 - Qt5Concurrent
 - Qt5 Multimedia
 - Qt5 Multimedia Plugins
 - Qt5 Serialport
 - Qt5 Script
 - libelf


SimuliDE executable is in bin folder.
No need for installation, place SimulIDE folder wherever you want and run the executable.
