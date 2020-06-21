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

#include "e-node.h"
#include "simulator.h"
#include "e-element.h"


eNode::eNode( QString id )
{
    m_id = id;
    m_nodeNum = 0;
    m_numCons = 0;
    m_volt    = 0;
    m_isBus = false;

    initialize();
    //qDebug() << "+eNode" << m_id;

    Simulator::self()->addToEnodeList( this );
}
eNode::~eNode(){ /*qDebug() << "~eNode" << m_id;Simulator::self()->remFromEnodeList( this );*/ }

void eNode::pinChanged( ePin* epin, int enodeComp ) // Add node at other side of pin
{
    //qDebug() << "eNode::pinChanged" << m_id << epin << enodeComp;
    m_nodeList[epin] = enodeComp;
}

void eNode::initialize()
{
    m_switched     = false;
    m_single       = false;
    m_changed      = false;
    m_currChanged  = false;
    m_admitChanged = false;

    m_changedFast.clear();
    m_nonLinear.clear();
    m_reactiveList.clear();
    m_admitList.clear();
    m_currList.clear();
    m_nodeList.clear();

    m_volt = 0;

    if( m_isBus )
    {
        m_eBusPinList.clear();
        m_eNodeList.clear();
        //qDebug() << "\neNode::initialize"<<this << m_eBusPinList.size();
    }
}

void eNode::stampCurrent( ePin* epin, double data )
{
    if( m_nodeList[epin] == m_nodeNum  ) return; // Be sure msg doesn't come from this node

    m_currList[epin] = data;

    //qDebug()<< m_nodeNum << epin << data << m_totalCurr;

    m_currChanged = true;

    if( !m_changed )
    {
        m_changed = true;
        Simulator::self()->addToChangedNodeList( this );
    }
}

void eNode::stampAdmitance( ePin* epin, double data )
{
    if( m_nodeList[epin] == m_nodeNum  ) return; // Be sure msg doesn't come from this node

    m_admitList[epin] = data;

    m_admitChanged = true;

    if( !m_changed )
    {
        m_changed = true;
        Simulator::self()->addToChangedNodeList( this );
    }
}

void eNode::setNodeNumber( int n ) { m_nodeNum = n; }

void eNode::stampMatrix()
{
    if( m_nodeNum == 0 ) return;

    m_changed = false;

    if( m_admitChanged )
    {
        m_admit.clear();
        m_totalAdmit = 0;

        QHashIterator<ePin*, double> i(m_admitList); // ePin-Admit
        while ( i.hasNext() )
        {
            i.next();

            double adm = i.value();

            ePin* epin = i.key();
            int enode = m_nodeList[epin];

            m_admit[enode] += adm;
            m_totalAdmit   += adm;
        }
        if( !m_single || m_switched ) stampAdmit();

        m_admitChanged = false;
    }

    if( m_currChanged )
    {
        m_totalCurr  = 0;
        for( double current : m_currList ) m_totalCurr += current;

        if( !m_single || m_switched ) stampCurr();

        m_currChanged  = false;
    }
    if( m_single ) solveSingle();
}

void eNode::stampAdmit()
{
    int nonCero = 0;
    QHashIterator<int, double> ai(m_admit); // iterate admitance hash: eNode-Admit
    while ( ai.hasNext() )
    {
        ai.next();
        int enode = ai.key();
        double admit = ai.value();
        if( enode>0 ) CircMatrix::self()->stampMatrix( m_nodeNum, enode, -admit );

        if( m_switched )                       // Find open/close events
        {
            if( admit > 0 ) nonCero++;
            double admitP = m_admitPrev[enode];

            if(( admit != admitP )
              &&((admit==0)||(admitP==0))) CircMatrix::self()->setCircChanged();
        }
    }
    if( m_switched )
    {
        m_admitPrev = m_admit;
        if( nonCero < 2 ) m_totalAdmit += 1e-12; //pnpBias example error
    }
    CircMatrix::self()->stampMatrix( m_nodeNum, m_nodeNum, m_totalAdmit );
}

void eNode::stampCurr()
{
    CircMatrix::self()->stampCoef( m_nodeNum, m_totalCurr );
}

void eNode::solveSingle()
{
    double volt = 0;

    if( m_totalAdmit > 0 ) volt = m_totalCurr/m_totalAdmit;
    setVolt( volt );
}

QList<int> eNode::getConnections()
{
    QList<int> cons;
    for( int nodeNum : m_nodeList )
    {
        if( m_admit[nodeNum] > 0 ) cons.append( nodeNum );
    }
    return cons;
}

void  eNode::setVolt( double v )
{
    //qDebug() << m_id << m_volt << v;
    if( fabs(m_volt-v) > 1e-9 ) //( m_volt != v )
    {
        //qDebug() << m_id << "setVChanged";
        m_voltChanged = true;
        m_volt = v;

        for( eElement* el : m_changedFast ) Simulator::self()->addToChangedFast( el ); // el->setVChanged();
        for( eElement* el : m_reactiveList ) Simulator::self()->addToReactiveList( el );
        for( eElement* el : m_nonLinear ) Simulator::self()->addToNoLinList( el );
    }
}
double eNode::getVolt() { return m_volt; }

void eNode::setIsBus( bool bus )
{
    m_isBus = bus;

    Simulator::self()->remFromEnodeList( this, /*delete=*/ false );
    Simulator::self()->addToEnodeBusList( this );
}

bool eNode::isBus()
{
    return m_isBus;
}

void eNode::createBus()
{
    int busSize = m_eBusPinList.size();

    //qDebug()<<"\neNode::createBus"<< this <<busSize << m_eBusPinList;

    m_eNodeList.clear();
    for( int i=0; i<busSize; i++ )
    {
        QList<ePin*> pinList = m_eBusPinList.at( i );

        eNode* enode = 0l;

        if( !pinList.isEmpty() )
        {
            enode = new eNode( m_id+"-eNode-"+QString::number( i ) );

            for( ePin* epin : pinList )
            {
                //qDebug() <<"Pin eNode"<< QString::fromStdString(epin->getId())<<enode;
                epin->setEnode( enode );
            }
        }
        m_eNodeList.append( enode );
    }
}

void eNode::addBusPinList( QList<ePin*> list, int line )
{
    int size = line+1;
    int busSize = m_eBusPinList.size();

    if( size > busSize )
    {
        for( int i=0; i<size-busSize; i++ )
        {
            QList<ePin*> newList;
            m_eBusPinList.append( newList );
        }
    }

    QList<ePin*> pinList = m_eBusPinList.at( line );
    for( ePin* epin : list )
    {
        if( !pinList.contains( epin ))
        {
            pinList.append( epin );
            //epin->setEnode( this );
        }
    }
    //qDebug() << "eNode::addBusPinList" <<this<< line << busSize<<"\n"<<pinList;
    m_eBusPinList.replace( line, pinList );
}

QList<ePin*> eNode::getEpins()    { return m_ePinList; }

void eNode::addEpin( ePin* epin )
{
    //qDebug() << "eNode::addEpin" << m_id << QString::fromStdString(epin->getId());
    if( !m_ePinList.contains(epin)) m_ePinList.append(epin);
}

void eNode::remEpin( ePin* epin )
{
    //qDebug() << "eNode::remEpin" << m_id << QString::fromStdString(epin->getId());
    if( m_ePinList.contains(epin) ) m_ePinList.removeOne( epin );

//qDebug() << "eNode::remEpin" << m_id << QString::fromStdString(epin->getId())<<m_ePinList.size();

    // If No epins then remove this enode
    if( m_ePinList.isEmpty() )
    {
        if( m_isBus ) Simulator::self()->remFromEnodeBusList( this, true );
        else          Simulator::self()->remFromEnodeList( this, true );
    }
}

void eNode::addToChangedFast( eElement* el )
{
    if( !m_changedFast.contains(el) ) m_changedFast.append(el);
}

void eNode::remFromChangedFast( eElement* el )
{
    m_changedFast.removeOne(el);
}

void eNode::addToReactiveList( eElement* el )
{
    if( !m_reactiveList.contains(el) ) m_reactiveList.append(el);
}

void eNode::remFromReactiveList( eElement* el )
{
    m_reactiveList.removeOne(el);
}

void eNode::addToNoLinList( eElement* el )
{
    if( !m_nonLinear.contains(el) ) m_nonLinear.append(el);
}

void eNode::remFromNoLinList( eElement* el )
{
    m_nonLinear.removeOne(el);
}

void eNode::setSingle( bool single ){ m_single = single; }      // This eNode can calculate it's own Volt
bool eNode::isSingle(){ return m_single; }

void eNode::setSwitched( bool switched ){ m_switched = switched; } // This eNode has switches attached
bool eNode::isSwitched(){ return m_switched; }

int eNode::getNodeNumber() { return m_nodeNum; }

QString eNode::itemId() { return m_id; }
