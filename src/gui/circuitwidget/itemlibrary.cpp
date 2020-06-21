/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/
 
#include "itemlibrary.h"
#include "circuit.h"
#include "simuapi_apppath.h"
#include "appiface.h"

//BEGIN Item includes
#include "amperimeter.h"
#include "adc.h"
#include "arduino.h"
#include "audio_out.h"
#include "avrcomponent.h"
#include "bcdto7s.h"
#include "bcdtodec.h"
#include "bincounter.h"
#include "bjt.h"
#include "buffer.h"
#include "bus.h"
#include "capacitor.h"
#include "clock.h"
#include "currsource.h"
#include "dac.h"
#include "dectobcd.h"
#include "demux.h"
#include "diode.h"
#include "elcapacitor.h"
#include "ellipse.h"
#include "flipflopd.h"
#include "flipflopjk.h"
#include "frequencimeter.h"
#include "fulladder.h"
#include "function.h"
#include "gate_and.h"
#include "gate_or.h"
#include "gate_xor.h"
#include "ground.h"
#include "hd44780.h"
#include "image.h"
#include "i2cram.h"
#include "i2ctoparallel.h"
//#include "inbus.h"
#include "inductor.h"
#include "keypad.h"
#include "ks0108.h"
#include "latchd.h"
#include "led.h"
#include "ledbar.h"
#include "ledmatrix.h"
#include "line.h"
#include "lm555.h"
#include "logicinput.h"
#include "memory.h"
#include "mosfet.h"
#include "mux.h"
#include "mux_analog.h"
#include "op_amp.h"
#include "oscope.h"
//#include "outbus.h"
#include "piccomponent.h"
#include "pcd8544.h"
#include "probe.h"
#include "potentiometer.h"
#include "push.h"
#include "rail.h"
#include "rectangle.h"
#include "relay-spst.h"
#include "resistor.h"
#include "resistordip.h"
#include "serialport.h"
#include "serialterm.h"
#include "servo.h"
#include "sevensegment.h"
#include "sevensegment_bcd.h"
#include "shiftreg.h"
#include "sr04.h"
#include "ssd1306.h"
#include "stepper.h"
#include "subcircuit.h"
#include "subpackage.h"
#include "switch.h"
#include "switchdip.h"
#include "textcomponent.h"
#include "voltimeter.h"
#include "volt_reg.h"
#include "voltsource.h"
#include "wavegen.h"
#include "ws2812.h"
//END Item includes

ItemLibrary* ItemLibrary::m_pSelf = 0l;

ItemLibrary::ItemLibrary()
{
    m_pSelf = this;

    loadItems();
    //loadPlugins();
}
ItemLibrary::~ItemLibrary()
{
    for( LibraryItem* item : m_items ) delete item;
}

void ItemLibrary::loadItems()
{
    m_items.clear();
    // Meters
    addItem( Probe::libraryItem() );
    addItem( Voltimeter::libraryItem() );
    addItem( Amperimeter::libraryItem() );
    addItem( Frequencimeter::libraryItem() );
    addItem( Oscope::libraryItem() );
    // Sources
    addItem( LogicInput::libraryItem() );
    addItem( Clock::libraryItem() );
    addItem( WaveGen::libraryItem() );
    addItem( VoltSource::libraryItem() );
    addItem( CurrSource::libraryItem() );
    addItem( Rail::libraryItem() );
    addItem( Ground::libraryItem() );
    // Switches
    addItem( Push::libraryItem() );
    addItem( Switch::libraryItem() );
    //addItem( ToggleSwitch::libraryItem() );
    addItem( SwitchDip::libraryItem() );
    addItem( RelaySPST::libraryItem() );
    // Passive
    addItem( Resistor::libraryItem() );
    addItem( ResistorDip::libraryItem() );
    addItem( Potentiometer::libraryItem() );
    addItem( Capacitor::libraryItem() );
    addItem( elCapacitor::libraryItem() );
    addItem( Inductor::libraryItem() );
    // Active
    addItem( Diode::libraryItem() );
    addItem( VoltReg::libraryItem() );
    addItem( OpAmp::libraryItem() );
    addItem( Mosfet::libraryItem() );
    addItem( BJT::libraryItem() );
    addItem( MuxAnalog::libraryItem() );
    // Outputs
    addItem( Led::libraryItem() );
    addItem( LedBar::libraryItem() );
    addItem( LedMatrix::libraryItem() );
    addItem( WS2812::libraryItem() );
    addItem( SevenSegment::libraryItem() );
    addItem( KeyPad::libraryItem() );
    addItem( Hd44780::libraryItem() );
    addItem( Pcd8544::libraryItem() );
    addItem( Ks0108::libraryItem() );
    addItem( Ssd1306::libraryItem() );
    addItem( Stepper::libraryItem() );
    addItem( Servo::libraryItem() );
    addItem( AudioOut::libraryItem() );
    // Micro
    addItem( PICComponent::libraryItem() );
    addItem( AVRComponent::libraryItem() );
    addItem( Arduino::libraryItem() );
    addItem( new LibraryItem( tr("Sensors"),tr("Micro"), "1to2.png","", 0l ) );
    addItem( SR04::libraryItem() );
    addItem( SerialPort::libraryItem() );
    addItem( SerialTerm::libraryItem() );
    // Logic
    addItem( new LibraryItem( tr("Gates"),tr("Logic"), "gates.png","", 0l ) );
    addItem( new LibraryItem( tr("Arithmetic"),tr("Logic"), "2to2.png","", 0l ) );
    addItem( new LibraryItem( tr("Memory"),tr("Logic"), "subc.png","", 0l ) );
    addItem( new LibraryItem( tr("Converters"),tr("Logic"), "1to2.png","", 0l ) );
    addItem( new LibraryItem( tr("Other Logic"),tr("Logic"), "2to3.png","", 0l ) );
    addItem( Buffer::libraryItem() );
    addItem( AndGate::libraryItem() );
    addItem( OrGate::libraryItem() );
    addItem( XorGate::libraryItem() );
    addItem( Function::libraryItem() );
    addItem( FlipFlopD::libraryItem() );
    addItem( FlipFlopJK::libraryItem() );
    addItem( BinCounter::libraryItem() );
    addItem( FullAdder::libraryItem() );
    addItem( LatchD::libraryItem() );
    addItem( ShiftReg::libraryItem() );
    addItem( Mux::libraryItem() );
    addItem( Demux::libraryItem() );
    addItem( BcdToDec::libraryItem() );
    addItem( DecToBcd::libraryItem() );
    addItem( BcdTo7S::libraryItem() );
    addItem( ADC::libraryItem() );
    addItem( DAC::libraryItem() );
    addItem( Bus::libraryItem() );
    addItem( SevenSegmentBCD::libraryItem() );
    addItem( Memory::libraryItem() );
    addItem( I2CRam::libraryItem() );
    addItem( I2CToParallel::libraryItem() );
    addItem( Lm555::libraryItem() );
    // Subcircuits
    addItem( SubCircuit::libraryItem() );
    // Other
    addItem( Image::libraryItem() );
    addItem( TextComponent::libraryItem() );
    addItem( Rectangle::libraryItem() );
    addItem( Ellipse::libraryItem() );
    addItem( Line::libraryItem() );

    addItem( SubPackage::libraryItem() );
}

void ItemLibrary::addItem( LibraryItem* item )
{
    if (!item) return;
    m_items.append(item);
}

/*void ItemLibrary::loadPlugins()
{
    m_plugins.clear();
    QDir pluginsDir( qApp->applicationDirPath() );

    pluginsDir.cd( "data/plugins" );

    qDebug() << "\n    Loading App plugins at:\n"<<pluginsDir.absolutePath()<<"\n";

    QString pluginName = "*plugin.*";
    pluginsDir.setNameFilters( QStringList(pluginName) );

    for( pluginName : pluginsDir.entryList( QDir::Files ) )
    {
        QPluginLoader pluginLoader( pluginsDir.absoluteFilePath( pluginName ) );
        QObject *plugin = pluginLoader.instance();

        pluginName = pluginName.split(".").first().remove("lib").remove("plugin").toUpper();

        if( plugin )
        {
            AppIface* item = qobject_cast<AppIface*>( plugin );

            item->initialize();
            if( item && !(m_plugins.contains(pluginName)) )
            {
                m_plugins.append(pluginName);
                qDebug()<< "        Loaded plugin\t" << pluginName;
            }
        }
        else
        {
            QString errorMsg = pluginLoader.errorString();
            qDebug()<< "        " << pluginName << "\tplugin FAILED: " << errorMsg;

            if( errorMsg.contains( "libQt5SerialPort" ) )
                errorMsg = " Qt5SerialPort is not installed in your system\n\n    Mcu SerialPort will not work\n    Just Install libQt5SerialPort package\n    To have Mcu Serial Port Working";

            QMessageBox::warning( 0,"App Plugin Error:", errorMsg );
        }
    }
    qDebug() << "\n";
}*/


const QList<LibraryItem *> ItemLibrary::items() const
{
    return m_items;
}

LibraryItem *ItemLibrary::itemByName(const QString name) const
{
    for( LibraryItem* item : m_items )
    {
        if( item->name() == name ) return item;
    }
    return 0l;
}

LibraryItem* ItemLibrary::libraryItem(const QString type ) const
{
    for( LibraryItem* item : m_items )
    {
        if( item->type() == type ) return item;
    }
    return 0l;
}


// CLASS LIBRAYITEM *********************************************************

LibraryItem::LibraryItem( const QString &name,
                          const QString &category,
                          const QString &iconName,
                          const QString type,
                          createItemPtr _createItem )
{
    m_name      = name;
    m_category  = category;
    m_iconfile  = iconName;
    m_type      = type;
    m_help      = "Sorry... no Help Available";
    createItem  = _createItem;

}

LibraryItem::~LibraryItem() { }

QString* LibraryItem::help() 
{
    if( m_help == "Sorry... no Help Available" )
    {
        m_help = getHelpFile( m_type );
    }
    return &m_help; 
}

QString LibraryItem::getHelpFile( QString name )
{
    QString help = "";

    //QString locale   = "_"+QLocale::system().name().split("_").first();
    QString locale = "_"+Circuit::self()->loc();

    name= name.toLower().replace( " ", "" );
    QString dfPath = SIMUAPI_AppPath::self()->availableDataFilePath( "help/"+locale+"/"+name+locale+".txt" );

    if( dfPath == "" )
        dfPath = SIMUAPI_AppPath::self()->availableDataFilePath( "help/"+name+".txt" );

    if( dfPath != "" )
    {
        QFile file( dfPath );

        if( file.open(QFile::ReadOnly | QFile::Text) ) // Get Text from Help File
        {
            QTextStream s1( &file );
            s1.setCodec("UTF-8");

            help = s1.readAll();

            file.close();
        }
        else qDebug() << "LibraryItem::getHelpFile ERROR"<<dfPath;
    }
    return help;
}
