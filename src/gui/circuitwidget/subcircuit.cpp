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

#include "subcircuit.h"
#include "componentselector.h"
#include "circuit.h"
#include "utils.h"
#include "pin.h"
#include "connector.h"
#include "mainwindow.h"
#include "itemlibrary.h"
#include "simuapi_apppath.h"

#include "ledsmd.h"
#include "e-bcdto7s.h"
#include "e-bcdtodec.h"
#include "e-bincounter.h"
#include "e-bjt.h"
#include "e-bus.h"
#include "e-capacitor.h"
#include "e-clock.h"
#include "e-dectobcd.h"
#include "e-demux.h"
#include "e-diode.h"
#include "e-flipflopd.h"
#include "e-flipflopjk.h"
#include "e-fulladder.h"
#include "e-function.h"
#include "e-gate_or.h"
#include "e-gate_xor.h"
#include "e-gate_xor.h"
#include "e-latch_d.h"
#include "e-lm555.h"
#include "e-logic_device.h"
#include "e-mosfet.h"
#include "e-mux.h"
#include "e-mux_analog.h"
#include "e-op_amp.h"
#include "e-resistor.h"
#include "e-resistordip.h"
#include "e-shiftreg.h"
#include "e-source.h"
#include "e-volt_reg.h"

//#include "mcucomponent.h"
//#include "hd44780.h"

Component* SubCircuit::construct( QObject* parent, QString type, QString id )
{ 
    SubCircuit* subCircuit = new SubCircuit( parent, type,  id ); 
    if( m_error > 0 )
    {
        Circuit::self()->compList()->removeOne( subCircuit );
        subCircuit->deleteLater();
        subCircuit = 0l;
        m_error = 0;
    }
    return subCircuit;
}

LibraryItem* SubCircuit::libraryItem()
{
    return new LibraryItem(
        tr("Subcircuit"),
        tr(""),         // Not dispalyed
        "",
        "Subcircuit",
        SubCircuit::construct );
}

SubCircuit::SubCircuit( QObject* parent, QString type, QString id )
          : Chip( parent, type, id )
{
    m_numItems = 0;

    QString compName = m_id.split("-").first(); // for example: "atmega328-1" to: "atmega328"
    QString dataFile = ComponentSelector::self()->getXmlFile( compName );

    qDebug()<<"SubCircuit::SubCircuit"<<dataFile;

    if( dataFile == "" )
    {
        if     ( compName.startsWith( "74XX") ) compName.replace( "XX", "HC" );
        else if( compName.startsWith( "74HC") ) compName.replace( "HC", "XX" );

        dataFile = ComponentSelector::self()->getXmlFile( compName );
    }
    if( dataFile == "" )
    {
          MessageBoxNB( "SubCircuit::SubCircuit", "                               \n"+
                    tr( "There are no data files for " )+compName+"    ");
          m_error = 23;
          return;
    }

    QFile file( dataFile );
    if( !file.open(QFile::ReadOnly | QFile::Text) )
    {
          MessageBoxNB( "SubCircuit::SubCircuit", "                               \n"+
                    compName+" "+ tr("Cannot read file %1:\n%2.").arg(dataFile).arg(file.errorString()));
          m_error = 21;
          return;
    }
    QDomDocument domDoc;
    if( !domDoc.setContent(&file) )
    {
         MessageBoxNB( "SubCircuit::SubCircuit", "                               \n"+
                   tr( "Cannot set file %1\nto DomDocument") .arg(dataFile));
         file.close();
         m_error = 22;
         return;
    }
    file.close();

    QDomElement root  = domDoc.documentElement();
    QDomNode    rNode = root.firstChild();

    while( !rNode.isNull() )
    {
        QDomElement element = rNode.toElement();
        QDomNode    node    = element.firstChild();

        while( !node.isNull() )         // Find the "package", for example 628A is package: 627A, Same pins
        {
            QDomElement element = node.toElement();
            
            if( element.attribute("name") == compName )
            {
                QDir dataDir(  dataFile );
                dataDir.cdUp();             // Indeed it doesn't cd, just take out file name
                
                m_pkgeFile = dataDir.filePath( element.attribute( "package" ) );
                if( !m_pkgeFile.endsWith( ".package" ) ) m_pkgeFile += ".package" ;
                
                m_subcFile = dataDir.filePath( element.attribute( "subcircuit" ) );
                if( !m_subcFile.endsWith( ".subcircuit" ) ) m_subcFile += ".subcircuit" ;
                //qDebug() << "SubCircuit::SubCircuit"<<m_pkgeFile <<m_subcFile ;
                break;
            }
            node = node.nextSibling();
        }
        rNode = rNode.nextSibling();
    }
    //initChip();
}
SubCircuit::~SubCircuit()
{
}

void SubCircuit::initChip()
{
    Chip::initChip();
    initSubcircuit();
}

void SubCircuit::initSubcircuit()
{
    //qDebug() << "SubCircuit::initSubcircuit datafile: " << m_subcFile;
    QFile file( m_subcFile );
    if( !file.open(QFile::ReadOnly | QFile::Text) )
    {
          MessageBoxNB( "SubCircuit::initSubcircuit",
                    tr("Cannot read file %1:\n%2.").arg(m_subcFile).arg(file.errorString()));
          m_error = 23;
          return;
    }
    QDomDocument domDoc;
    if( !domDoc.setContent(&file) )
    {
         MessageBoxNB( "SubCircuit::initSubcircuit",
                   tr("Cannot set file %1\nto DomDocument") .arg(m_subcFile));
         file.close();
         m_error = 24;
         return;
    }
    file.close();

    QDomElement root  = domDoc.documentElement();
    QDomNode    rNode = root.firstChild();

    if( root.tagName()!="subcircuit" )
    {
        MessageBoxNB( "SubCircuit::initSubcircuit",
                  tr("Error reading Subcircuit file: %1\n") .arg(m_subcFile));
        m_error = 25;
        return;
    }
    if( root.hasAttribute("enodes") )                    // Create eNodes & add to enodList
    {
        int enodes = root.attribute( "enodes" ).toInt(); // Number of eNodes to be created
        for( int i=0; i<enodes; i++ )
        {
            QString eNodeid = m_id;
            eNodeid.append( "-eNode_" ).append( QString::number(i));
            m_internal_eNode.append( new eNode(eNodeid) );
        }
    }
    m_pinConections.resize( m_numpins );

    while( !rNode.isNull() )
    {
        QDomElement element = rNode.toElement();
        QString     tagName = element.tagName();

        if( tagName=="item" )
        {
            QString type = element.attribute( "itemtype"  );
            QString id = m_id+"-"+type+"-"+QString::number(m_numItems);
            m_numItems++;

            //qDebug() << "\nSubCircuit::initSubcircuit" << id << type;

            eElement* ecomponent = 0l;

            /*if( type == "eHd44780" )
            {
                Hd44780* hd = new Hd44780( this, "Hd44780", "Hd44780-"+Circuit::self()->newSceneId() );
                hd->setParentItem( this );
                hd->setPos( 0,0 );
                Circuit::self()->addItem( hd );
                Circuit::self()->compList()->removeOne( hd );
                ecomponent = hd;

            }*/
            if( type == "eResistor" )  
            {
                eResistor* eresistor = new eResistor( id.toStdString() );
                if( element.hasAttribute("resistance") ) eresistor->setRes( element.attribute( "resistance" ).toDouble() );
                ecomponent = eresistor;
            }
            else if( type == "eResistorDip" )  
            {
                int size = 8;
                eResistorDip* eresistordip = new eResistorDip( id.toStdString() );
                if( element.hasAttribute("size") ) size = element.attribute( "size" ).toInt();
                eresistordip->setSize( size );
                if( element.hasAttribute("resistance") ) eresistordip->setRes( element.attribute( "resistance" ).toDouble() );
                ecomponent = eresistordip;
            }
            else if( type == "eCapacitor" ) 
            {
                ecomponent = new eCapacitor( id.toStdString() );
            }
            else if( type == "eDiode" )     
            {
                eDiode* ediode = new eDiode( id.toStdString() );
                if( element.hasAttribute("threshold") )
                {
                    ediode->setThreshold( element.attribute( "threshold" ).toDouble() );
                }
                ecomponent = ediode;
            }
            else if( type == "eAndGate" )
            {
                int numInputs = 2;
                if( element.hasAttribute("numInputs") ) numInputs  = element.attribute( "numInputs" ).toInt();
                eGate* egate = new eGate( id.toStdString(), numInputs );
                egate->createPins( numInputs, 1 );
                ecomponent = egate;
            }
            else if( type == "eBuffer" )
            {
                eGate* egate = new eGate( id.toStdString(), 1 );
                egate->createPins( 1, 1 );
                ecomponent = egate;
                
                if( element.attribute( "tristate" ) == "true" )
                    egate->setTristate( true );
            }
            else if( type == "eOrGate" )
            {
                int numInputs = 2;
                if( element.hasAttribute("numInputs") ) numInputs  = element.attribute( "numInputs" ).toInt();
                eOrGate* egate = new eOrGate( id.toStdString(), numInputs );
                egate->createPins( numInputs, 1 );
                ecomponent = egate;
            }
            else if( type == "eXorGate" )
            {
                int numInputs = 2;
                if( element.hasAttribute("numInputs") ) numInputs  = element.attribute( "numInputs" ).toInt();
                eXorGate* egate = new eXorGate( id.toStdString(), numInputs );
                egate->createPins( numInputs, 1 );
                ecomponent = egate;
            }
            else if( type == "eFunction" )
            {
                eFunction* efunction = new eFunction( id.toStdString() );
                ecomponent = efunction;
                
                int inputs  = 0;
                int outputs = 0;
                if( element.hasAttribute("numInputs") )  inputs  = element.attribute( "numInputs" ).toInt();
                if( element.hasAttribute("numOutputs") ) outputs = element.attribute( "numOutputs" ).toInt();
                efunction->createPins( inputs, outputs );
                
                if( element.hasAttribute("functions") ) efunction->setFunctions( element.attribute( "functions" ) );
            }
            else if( type.startsWith( "eLatchD" ) )
            {
                int channels = 1;
                if( element.hasAttribute("channels") ) channels = element.attribute( "channels" ).toInt();
                eLatchD* elatchd = new eLatchD( id.toStdString() );
                elatchd->setNumChannels( channels );
                
                if( element.hasAttribute("trigger") )
                {
                    int t = element.attribute( "trigger" ).toInt();
                    if     ( t == 1 ) elatchd->createClockPin();
                    else if( t == 2 ) elatchd->createInEnablePin();
                }
                ecomponent = elatchd;
            }
            else if( (type == "eCounter") || (type == "eBinCounter") )
            {
                int maxValue = 1;
                if( element.hasAttribute("maxValue") ) maxValue  = element.attribute( "maxValue" ).toInt();
                eBinCounter* ecounter = new eBinCounter( id.toStdString() );
                ecounter->setTopValue( maxValue );
                ecounter->createPins();
                ecomponent = ecounter;
            }
            else if( type == "eFullAdder" )
            {
                eFullAdder* efulladder = new eFullAdder( id.toStdString() );
                efulladder->createPins();
                ecomponent = efulladder;
            }
            else if( type == "eFlipFlopD" )
            {
                eFlipFlopD* eFFD = new eFlipFlopD( id.toStdString() );
                eFFD->createPins();
                ecomponent = eFFD;
                
                bool srInv = true;
                if( element.hasAttribute("sRInverted" ) )
                {
                    if( element.attribute( "sRInverted" ) == "false" ) srInv = false;
                }
                eFFD->setSrInv( srInv );
            }
            else if( type == "eFlipFlopJK" )
            {
                eFlipFlopJK* eFFJK = new eFlipFlopJK( id.toStdString() );
                eFFJK->createPins();
                ecomponent = eFFJK;
                
                bool srInv = true;
                if( element.hasAttribute("sRInverted" ) )
                {
                    if( element.attribute( "sRInverted" ) == "false" ) srInv = false;
                }
                eFFJK->setSrInv( srInv );
            }
            else if( type == "eShiftReg" )  
            {
                int latchClk = 0;
                int serOut   = 0;
                if( element.hasAttribute("latchClock") ) latchClk = element.attribute( "latchClock" ).toInt();
                if( element.hasAttribute("serialOut") )  serOut   = element.attribute( "serialOut" ).toInt();
                ecomponent = new eShiftReg( id.toStdString(), latchClk, serOut );
            }
            else if( type == "eMux" )
            {
                eMux* emux = new eMux( id.toStdString() );
                emux->createPins();
                ecomponent = emux;
            }
            else if( type == "eDemux" )
            {
                eDemux* edemux = new eDemux( id.toStdString() );
                edemux->createPins();
                ecomponent = edemux;
            }
            else if( type == "eBcdTo7S" )
            {
                eBcdTo7S* ebcdto7s = new eBcdTo7S( id.toStdString() );
                ebcdto7s->createPins();
                ecomponent = ebcdto7s;
            }
            else if( type == "eBcdToDec" )
            {
                eBcdToDec* ebcdtodec = new eBcdToDec( id.toStdString() );
                ebcdtodec->createPins();
                ecomponent = ebcdtodec;
            }
            else if( type == "eDecToBcd" )
            {
                eDecToBcd* edectobcd = new eDecToBcd( id.toStdString() );
                edectobcd->createPins();
                ecomponent = edectobcd;
            }
            else if( type == "eClock" )
            {
                double freq = 1000;
                double volt = 5;
                if( element.hasAttribute("freq") ) freq = element.attribute( "freq" ).toDouble();
                if( element.hasAttribute("voltage") ) volt = element.attribute( "voltage" ).toDouble();
                eClock* eclock = new eClock( id.toStdString() );
                eclock->setFreq( freq );
                eclock->setVolt( volt );
                ecomponent = eclock;
            }
            else if(( type == "eBus" )
                  ||( type == "eOutBus" )
                  ||( type == "eInBus" ) )
            {
                int numbits = 8;
                //int startBit = 0;
                if( element.hasAttribute("numBits") )  numbits = element.attribute( "numBits" ).toInt();
                //if( element.hasAttribute("startBit") ) startBit = element.attribute( "startBit" ).toInt();
                eBus* ebus = new eBus( id.toStdString() );
                ebus->setNumLines( numbits );
                //ebus->setStartBit( startBit );
                ecomponent = ebus;
            }
            else if( (type == "eRail")||(type == "eGround") )
            {
                double volt = 0;
                if( element.hasAttribute("voltage") ) volt = element.attribute( "voltage" ).toDouble();
                eSource* esource = new eSource( id.toStdString(), 0l );
                esource->createPin();
                esource->setVoltHigh( volt );
                esource->setOut( true );
                ecomponent = esource;
            }
            else if( type == "eMosfet" )
            {
                double threshold = 3;
                double rDSon     = 1;
                if( element.hasAttribute("threshold") ) threshold = element.attribute( "threshold" ).toDouble();
                if( element.hasAttribute("rDSon") )     rDSon = element.attribute( "rDSon" ).toDouble();
                eMosfet* emosfet = new eMosfet( id.toStdString() );
                emosfet->setThreshold( threshold );
                emosfet->setRDSon( rDSon );
                if( element.hasAttribute("pChannel") )
                {
                    if( element.attribute( "pChannel" ) == "true" ) emosfet->setPchannel( true ); 
                }
                if( element.hasAttribute("Depletion") )
                {
                    if( element.attribute( "Depletion" ) == "true" ) emosfet->setDepletion( true );
                }
                ecomponent = emosfet;
            }
            else if( type == "eBJT" )
            {
                double threshold = 0.7;
                double gain     = 100;
                if( element.hasAttribute("threshold") ) threshold = element.attribute( "threshold" ).toDouble();
                if( element.hasAttribute("gain") )      gain      = element.attribute( "gain" ).toDouble();
                eBJT* ebjt = new eBJT( id.toStdString() );
                ebjt->setBEthr( threshold );
                ebjt->setGain( gain );
                if( element.hasAttribute("pNP") )
                {
                    if( element.attribute( "pNP" ) == "true" ) { ebjt->setPnp( true ); }
                }
                if( element.hasAttribute("bCdiode") )
                {
                    if( element.attribute( "bCdiode" ) == "true" ) { ebjt->setBCd( true ); }
                }
                ecomponent = ebjt;
            }
            else if( type == "eVoltReg" )
            {
                double volts = 1.2;
                if( element.hasAttribute("Volts") ) volts = element.attribute( "Volts" ).toDouble();
                eVoltReg* evoltreg = new eVoltReg( id.toStdString() );
                evoltreg->setNumEpins(3);
                evoltreg->setVRef( volts );
                ecomponent = evoltreg;
            }
            else if( type == "eopAmp" )
            {
                double gain = 1000;
                if( element.hasAttribute("Gain") ) gain = element.attribute( "Gain" ).toDouble();
                bool powerPins = false;
                if( element.hasAttribute("Power_Pins" ) )
                {
                    if( element.attribute( "Power_Pins" ) == "true" ) powerPins = true;
                }
                eOpAmp* eopamp = new eOpAmp( id.toStdString() );
                eopamp->setGain( gain );
                eopamp->setPowerPins( powerPins );
                ecomponent = eopamp;
            }
            else if( type == "eMuxAnalog" )
            {
                eMuxAnalog* muxAn = new eMuxAnalog( id.toStdString() );
                double imp = 1;
                if( element.hasAttribute("impedance") ) imp = element.attribute( "impedance" ).toDouble();
                muxAn->setResist( imp );
                int bits = 3;
                if( element.hasAttribute("addressBits") ) bits = element.attribute( "addressBits" ).toInt();
                muxAn->setBits( bits );
                ecomponent = muxAn;
            }
            else if( type == "LedSmd" )
            {
                int width = 8;
                int height = 8;
                if( element.hasAttribute("width") )  width  = element.attribute( "width" ).toDouble();
                if( element.hasAttribute("height") ) height = element.attribute( "height" ).toDouble();
                ecomponent = new LedSmd( this, "LEDSMD", id, QRectF( 0, 0, width, height )  );
            }
            else if( type == "eLm555" )
            {
                ecomponent = new eLm555( id.toStdString() );
            }
            
            if( ecomponent )
            {
                m_elementList.append( ecomponent );
                ecomponent->initEpins();

                // Get properties
                if( element.hasAttribute("maxcurrent") )
                {
                    eLed* eled = static_cast<eLed*>(ecomponent);
                    eled->setMaxCurrent( element.attribute( "maxcurrent" ).toDouble() );
                }
                if( element.hasAttribute("capacitance") )
                {
                    eCapacitor* ecapacitor = static_cast<eCapacitor*>(ecomponent);
                    ecapacitor->setCap( element.attribute( "capacitance" ).toDouble() );
                }
                if( element.hasAttribute("inputHighV") )
                {
                    eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                    elogicdevice->setInputHighV( element.attribute( "inputHighV" ).toDouble() );
                }
                if( element.hasAttribute("inputLowV") )
                {
                    eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                    elogicdevice->setInputLowV( element.attribute( "inputLowV" ).toDouble() );
                }
                if( element.hasAttribute("outHighV") )
                {
                    eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                    elogicdevice->setOutHighV( element.attribute( "outHighV" ).toDouble() );
                }
                if( element.hasAttribute("outLowV") )
                {
                    eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                    elogicdevice->setOutLowV( element.attribute( "outLowV" ).toDouble() );
                }
                if( element.hasAttribute("inputImped") )
                {
                    eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                    elogicdevice->setInputImp( element.attribute( "inputImped" ).toDouble() );
                }
                if( element.hasAttribute("outImped") )
                {
                    eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                    elogicdevice->setOutImp( element.attribute( "outImped" ).toDouble() );
                }
                if( element.hasAttribute("tristate") )
                {
                    if( element.attribute( "tristate" ) == "true" )
                    {
                        eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                        elogicdevice->createOutEnablePin();
                    }
                }
                if( element.hasAttribute("openCollector") )
                {
                    if( element.attribute( "openCollector" ) == "true" )
                    {
                        eGate* egate = static_cast<eGate*>(ecomponent);
                        egate->setOpenCol( true );
                    }
                }
                if( element.hasAttribute("inputEnable") )
                {
                    if( element.attribute( "inputEnable" ) == "true" )
                    {
                        eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                        elogicdevice->createInEnablePin();
                    }
                }
                if( element.hasAttribute("clocked") )
                {
                    if( element.attribute( "clocked" ) == "true" )
                    {
                        eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                        elogicdevice->createClockPin();
                    }
                }
                if( element.hasAttribute("clockInverted") )
                {
                    if( element.attribute( "clockInverted" ) == "true" )
                    {
                        eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                        elogicdevice->setClockInv( true );
                    }
                }
                if( element.hasAttribute("inverted") )
                {
                    if( element.attribute( "inverted" ) == "true" )
                    {
                        eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                        elogicdevice->setInverted( true );
                    }
                }
                if( element.hasAttribute("invertInputs") )
                {
                    if( element.attribute( "invertInputs" ) == "true" )
                    {
                        eLogicDevice* elogicdevice = static_cast<eLogicDevice*>(ecomponent);
                        elogicdevice->setInvertInps( true );
                    }
                }

                QStringList connectionList = element.attribute( "connections" ).split(" ");
                //qDebug() << "connectionList" << connectionList;

                for( QString connection : connectionList )   // Get the connection points for each connection
                {
                    if( !(connection.contains("-")) ) continue;
                    QStringList pins = connection.split("-");

                    QString pin = pins.first();

                    //qDebug() << "SubCircuit::initSubcircuit connecting:"<<element.attribute( "itemtype" ) << pins.first() << pins.last();
                    ePin* epin = 0l;

                    if( pin.startsWith("componentPin"))
                    {
                        int pinNum = pin.remove("componentPin").toInt();
                        epin = ecomponent->getEpin( pinNum );
                    }
                    else epin = ecomponent->getEpin( pin );

                    if( epin ) connectEpin( epin, pins.last().replace( "\n", "" ).replace( "\r", "" ) );   // Connect points (ePin to Pin or eNode)
                    else 
                    {
                        qDebug() << "SubCircuit::initSubcircuit Pin Doesn't Exist:" << pin;
                        m_error = 31;
                        return;
                    }
                }
                ecomponent->resetState();
            }
            else 
            {
                qDebug() << "SubCircuit::initSubcircuit Error creating: " << id;
                m_error = 32;
                return;
            }
        }
        rNode = rNode.nextSibling();
    }
}

void SubCircuit::connectEpin( ePin* epin, QString connetTo )
{
    if( connetTo.startsWith("eNode") )
    {
        int eNodeNum = connetTo.remove("eNode").toInt();
        epin->setEnode( m_internal_eNode.at(eNodeNum) );
        //qDebug() << "SubCircuit::connectEpin to eNode "<< QString::fromStdString( epin->getId() ) << connetTo << eNodeNum;
    }
    else if( connetTo.startsWith("packagePin") )
    {
        int pinNum = connetTo.remove("packagePin").toInt()-1;
        //qDebug() << "SubCircuit::connectEpin to Pin " << connetTo << pinNum;
        m_pinConections[pinNum].append( epin );
    }
    else if( connetTo.startsWith("Package_") )          // Find pin by id
    {
        connetTo = connetTo.replace( "Package_", "" );
        QString ctName;
        QString ctNumber;

        for( int i=0; i<connetTo.size(); i++ )
        {
            QChar ch = connetTo[i];
            if( ch.isDigit() ) ctNumber.append( ch );
            else               ctName.append( ch );
        }

        bool found = false;
        for( int i=0; i<m_numpins; i++ )
        {
            Pin* pin = m_pin[i];
            QString pinId = pin->pinId().split("-").last().replace( " ", "" );
            //qDebug() << "SubCircuit::connectEpin search"<< pinId << connetTo << i;

            QString piName;
            QString piNumber;

            for( int i=0; i<pinId.size(); i++ )
            {
                QChar ch = pinId[i];
                if( ch.isDigit() ) piNumber.append( ch );
                else               piName.append( ch );
            }
            
            if( (piName==ctName) && (piNumber==ctNumber) )
            {
                found = true;
                m_pinConections[i].append( epin );
                //qDebug() << "SubCircuit::connectEpin Found "<< pinId << connetTo << i;
                break;
            }
        }
        if( !found ) qDebug() << "SubCircuit::connectEpin ERROR Pin Not Found"<< connetTo;
    }
}

void SubCircuit::initialize()
{
    //qDebug() << "SubCircuit::initialize()";
    for( int i=0; i<m_numpins; i++ )        // create internal enodes for not connected package pins
    {
        eNode* enod = m_ePin[i]->getEnode();
        
        if( enod ) continue;

        QList<ePin*> ePinList = m_pinConections[i];
        int size = ePinList.size();

        if( size > 1 )
        {
            enod = ePinList.first()->getEnode();
            if( !enod )
            {
                QString eNodeid = m_id;
                eNodeid.append( "-eNode_I_" ).append( QString::number(i));
                enod = new eNode( eNodeid );
                for( ePin* epin : m_pinConections[i] ) epin->setEnode(enod);
                //qDebug() << "SubCircuit::initialize() New eNode:" << QString::fromStdString( m_ePin[i]->getId() );
            }
        }
        //qDebug() << "SubCircuit::initialize() Pin"<< QString::fromStdString( m_ePin[i]->getId() );//<< enod->itemId();
    }
}

void SubCircuit::attach()
{
    qDebug() << "SubCircuit::attach()";
    for( int i=0; i<m_numpins; i++ )        // get eNode for each package pin
    {                                       // and assing to connected subcircuit ePins
        eNode* enod = m_ePin[i]->getEnode();
        if( !enod ) continue;

        qDebug() << "SubCircuit::attach() Pin"<< QString::fromStdString( m_ePin[i]->getId() )<< enod->itemId();
        for( ePin* epin : m_pinConections[i] )
        {
            //if( epin->getEnode() ) break;
            qDebug() << "SubCircuit::attach()"<< QString::fromStdString( epin->getId() )<<enod;
            epin->setEnode(enod);
        }
    }
}

void SubCircuit::setLogicSymbol( bool ls )
{
    if( m_initialized && (m_isLS == ls) ) return;
    //qDebug() <<"SubCircuit::setLogicSymbol"<<ls<<m_pkgeFile;
    
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();
    
    Circuit::self()->saveState();
    
    clear();
    
    Chip::setLogicSymbol( ls );
    
    initSubcircuit();
    
    if( pauseSim ) Simulator::self()->runContinuous();
}

void SubCircuit::clear()
{
    for( int i=0; i<m_pinConections.size(); i++ )
    {
        for( ePin* epin : m_pinConections[i] ) epin->setEnode(0l);
    }
    m_pinConections.clear();
    for( eNode* node : m_internal_eNode )
    {
        Simulator::self()->remFromEnodeList( node, true );
        //delete node;
    }
    m_internal_eNode.clear();
    for( eElement* el : m_elementList )
    {
        //qDebug() << "deleting" << QString::fromStdString( el->getId() );
        delete el;
    }
    m_elementList.clear();
    m_numItems = 0;
}

void SubCircuit::remove()
{
    clear();
    Chip::remove();
}

void SubCircuit::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    event->accept();
    QMenu *menu = new QMenu();

    //menu->addSeparator();

    Component::contextMenu( event, menu );
    menu->deleteLater();
}

#include "moc_subcircuit.cpp"

