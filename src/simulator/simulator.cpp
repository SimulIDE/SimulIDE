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

#include <iostream>

#include "simulator.h"
#include "circuit.h"
#include "matrixsolver.h"
#include "e-element.h"
#include "oscopewidget.h"
#include "plotterwidget.h"
#include "outpaneltext.h"
#include "mcucomponent.h"
#include "mainwindow.h"
#include "circuitwidget.h"
#include "baseprocessor.h"

Simulator* Simulator::m_pSelf = 0l;

Simulator::Simulator( QObject* parent )
         : QObject(parent)
{
    m_pSelf = this;

    m_isrunning = false;
    m_debugging = false;
    m_paused    = false;
    m_runMcu    = false;

    m_step       = 0;
    m_numEnodes  = 0;
    m_timerId    = 0;
    m_lastStep   = 0;
    m_lastRefTime = 0;
    m_timerSc    = 1;
    m_timerTick  = 50;
    m_stepsPrea  = 50;
    m_simuRate   = 1000000;
    m_noLinAcc = 5; // Non-Linear accuracy

    m_stepsPerus = 1;

    m_RefTimer.start();
}
Simulator::~Simulator()
{
    m_CircuitFuture.waitForFinished();
}

inline void Simulator::solveMatrix()
{
    for( eNode* node : m_eChangedNodeList ) node->stampMatrix();
    m_eChangedNodeList.clear();

    if( !m_matrix.solveMatrix() )                // Try to solve matrix,
    {                                         // if fail stop simulation
        std::cout << "Simulator::solveMatrix(), Failed to solve Matrix" << std::endl;
        m_error = true;
    }                                // m_matrix sets the eNode voltages
}

void Simulator::timerEvent( QTimerEvent* e )  //update at m_timerTick rate (50 ms, 20 Hz max)
{
    e->accept();

    if( !m_isrunning ) return;
    if( m_error )
    {
        CircuitWidget::self()->powerCircOff();
        CircuitWidget::self()->setRate( -1 );
        return;
    }
    if( !m_CircuitFuture.isFinished() ) // Stop remaining parallel thread
    {
        m_isrunning = false;
        m_CircuitFuture.waitForFinished();
        m_isrunning = true;
        //return;
    }
    // Get Real Simulation Speed
    m_refTime = m_RefTimer.nsecsElapsed();
    m_tStep   = m_step;

    runGraphicStep1();
    // Run Circuit in parallel thread
    m_CircuitFuture = QtConcurrent::run( this, &Simulator::runCircuit ); // Run Circuit in a parallel thread

    runGraphicStep2();
}

void Simulator::runGraphicStep()
{
    runGraphicStep1();
    runGraphicStep2();
}

void Simulator::runGraphicStep1()
{
    for( eElement* el : m_updateList ) el->updateStep();
    //emit stepTerms();
    //TerminalWidget::self()->step();
}

void Simulator::runGraphicStep2()
{
    //qDebug() <<"Simulator::runGraphicStep";
    if( m_debugging ) m_tStep = m_step;        // Debugger driving, not timer

    if( Circuit::self()->animate() )
    {
        Circuit::self()->updateConnectors();
        for( eNode* enode : m_eNodeList ) enode->setVoltChanged( false );
    }

    uint64_t deltaRefTime = m_refTime-m_lastRefTime;
    if( deltaRefTime >= 1e9 )               // We want steps per Sec = 1e9 ns
    {
        stepsPerSec = ( m_tStep-m_lastStep )*1e9/deltaRefTime;
        CircuitWidget::self()->setRate( (stepsPerSec*100)/(1e6*m_stepsPerus) /*m_simuRate*/ );
        m_lastStep    = m_tStep;
        m_lastRefTime = m_refTime;
    }

    CircuitView::self()->setCircTime( m_tStep/m_stepsPerus );
    PlotterWidget::self()->step();
}

void Simulator::runCircuit()
{
/*uint64_t time0 = m_RefTimer.nsecsElapsed();
static uint64_t count;
static uint64_t circTime;
if( m_step == 0 )
{
    count = 0;
    circTime = 0;
}*/

    for( uint64_t i=0; i<m_circuitRate; i++ )
    {
        m_circTime = m_step*m_stepNS;         // Circuit Time in nanoseconds

        if( m_runMcu ) BaseProcessor::self()->step();             // Run MCU

        if( !m_simuClock.isEmpty() )     // Run elements at Simulation Clock
        {
            for( eElement* el : m_simuClock ) el->simuClockStep();
        }
        if( !m_changedFast.isEmpty() )                  // Run Fast elements
        {
            for( eElement* el : m_changedFast) el->setVChanged();
            m_changedFast.clear();
        }

        if( m_step >= m_reacCounter )               // Run Reactive Elements
        {
            m_reacCounter += m_stepsPrea*m_stepsPerus;
            if( !m_reactive.isEmpty() )
            {
                for( eElement* el : m_reactive ) el->setVChanged();
                m_reactive.clear();
            }
        }

        if( !m_nonLinear.isEmpty() )              // Run Non-Linear elements
        {
            int counter = 0;
            int limit = (m_noLinAcc-2)*100;

            while( !m_nonLinear.isEmpty() )      // Run untill all converged
            {
                for( eElement* el : m_nonLinear ) el->setVChanged();
                m_nonLinear.clear();

                if( !m_eChangedNodeList.isEmpty() ) solveMatrix();

                if( ++counter > limit ) break; // Limit the number of loops
            }
            //if( counter > 0 ) qDebug() << "\nSimulator::runCircuit  Non-Linear Solved in steps:"<<counter;
        }
        if( !m_eChangedNodeList.isEmpty() ) solveMatrix();

        m_step ++;
        if( !m_isrunning ) break; // Keep this at the end for debugger to run 1 step
    }
/*uint64_t time = m_RefTimer.nsecsElapsed()-time0;
count++;
circTime += time;*/
//if( count == 1000 )
//qDebug() << "Simulator::runCircuit Time nS:"<<time<<circTime<<count << circTime/count;
}

void Simulator::runExtraStep( uint64_t cycle )
{
    //if( !m_isrunning ) return;

    //qDebug() <<"\nSimulator::runExtraStep"<<m_circTime<<m_step<<m_stepNS;
    m_circTime = cycle*m_mcuStepNS;
    //qDebug() <<"Simulator::runExtraStep"<<m_circTime<<cycle<<m_mcuStepNS;

    if( !m_eChangedNodeList.isEmpty() )
    {
        solveMatrix();
        //if( !m_isrunning ) return;
    }

    // Run Fast elements
    for( eElement* el : m_changedFast ) el->setVChanged();
    m_changedFast.clear();
}

void Simulator::runContinuous()
{
    if( m_debugging )                     // Debugger Controllig Simulation
    {
        debug( false );
        emit resumeDebug();
        return;
    }
    simuRateChanged( m_simuRate );
    startSim();

    std::cout << "\n    Simulation Running... \n"<<std::endl;
    m_timerId = this->startTimer( m_timerTick, Qt::PreciseTimer );
}

void Simulator::debug( bool run )
{
    if( run )
    {
        m_debugging = false;
        runContinuous();
    }
    else
    {
        startSim();
        m_isrunning = false;
        m_debugging = true;
        std::cout << "\n    Debugger Controllig Simulation... \n"<<std::endl;
    }
    m_runMcu = false;
}

void Simulator::startSim()
{
    std::cout <<"\nStarting Circuit Simulation...\n"<< std::endl;

    for( eNode* busNode : m_eNodeBusList ) busNode->initialize(); // Clear Buses

    std::cout <<"  Initializing "<< m_elementList.size() << "\teElements"<< std::endl;
    for( eElement* el : m_elementList )    // Initialize all Elements
    {
        //std::cout << "initializing  "<< el->getId()<<  std::endl;
        if( !m_paused ) el->resetState();
        el->initialize();
    }

    std::cout <<"  Initializing "<< m_eNodeBusList.size() << "\tBuses"<< std::endl;
    for( eNode* busNode : m_eNodeBusList ) busNode->createBus(); // Create Buses

    m_nonLinear.clear();
    m_changedFast.clear();
    m_reactive.clear();
    m_eChangedNodeList.clear();

    // Connect Elements with internal circuits.
    for( eElement* el : m_elementList ) el->attach();

    if( McuComponent::self() && !m_paused ) McuComponent::self()->runAutoLoad();


    m_numEnodes = m_eNodeList.size();
    std::cout <<"  Initializing "<< m_eNodeList.size()<< "\teNodes"<< std::endl;
    for( int i=0; i<m_numEnodes; i++ )
    {
        eNode* enode = m_eNodeList.at(i);
        enode->setNodeNumber( i+1 );
        enode->initialize();
    }
    for( eElement* el : m_elementList ) el->stamp();

    // Initialize Matrix
    m_matrix.createMatrix( m_eNodeList );

    // Try to solve matrix, if fails stop simulation
    // m_matrix.printMatrix();
    if( !m_matrix.solveMatrix() )
    {
        std::cout << "Simulator::startSim, Failed to solve Matrix"
                  <<  std::endl;
        CircuitWidget::self()->powerCircOff();
        CircuitWidget::self()->setRate( -1 );
        return;
    }
    //for( eElement* el : m_elementList ) el->setVChanged();
    std::cout << "\nCircuit Matrix looks good" <<  std::endl;
    if( !m_paused )
    {
        m_lastStep     = 0;
        m_lastRefTime  = 0;
        m_updtCounter  = m_circuitRate;
        m_reacCounter  = m_stepsPrea;
    }
    m_isrunning = true;
    m_paused = false;
    m_error = false;
    CircuitView::self()->setCircTime( 0 );
}

void Simulator::stopDebug()
{
    m_debugging = false;
    stopSim();
}

void Simulator::stopSim()
{
    //if( !m_isrunning ) return;

    stopTimer();
    //emit pauseDebug();
    m_paused = false;
    m_circTime = 0;
    m_step = 0;

    for( eNode* node  : m_eNodeList  )  node->setVolt( 0 );
    for( eElement* el : m_elementList ) el->resetState();
    for( eElement* el : m_updateList )  el->updateStep();

    if( McuComponent::self() ) McuComponent::self()->reset();

    CircuitWidget::self()->setRate( 0 );
    Circuit::self()->update();

    std::cout << "\n    Simulation Stopped \n" << std::endl;
}

void Simulator::pauseSim()
{
    emit pauseDebug();
    m_paused = true;
    stopTimer();

    std::cout << "\n    Simulation Paused \n" << std::endl;
}

void Simulator::resumeSim()
{
    m_isrunning = true;
    m_paused = false;

    emit resumeDebug();

    if( m_debugging ) return;

    std::cout << "\n    Resuming Simulation\n" << std::endl;
    m_timerId = this->startTimer( m_timerTick, Qt::PreciseTimer );
}

void Simulator::stopTimer()
{
    if( m_timerId != 0 )
    {
        m_isrunning = false;
        this->killTimer( m_timerId );
        m_timerId = 0;
        m_CircuitFuture.waitForFinished();
    }
}

void Simulator::resumeTimer()
{
    if( m_timerId == 0 )
    {
        m_isrunning = true;
        m_timerId = this->startTimer( m_timerTick, Qt::PreciseTimer );
    }
}

int Simulator::simuRateChanged( int rate )
{
    //if( rate > 1e6 ) rate = 1e6;
    if( rate < 1 )   rate = 1;

    m_stepsPerus = rate/1e6;
    if( m_stepsPerus < 1 ) m_stepsPerus = 1;
    m_stepNS = 1000/m_stepsPerus;

    m_runMcu = false;

    if( BaseProcessor::self() )
    {
        //double mcuSteps = McuComponent::self()->freq()*1e6;
        //if( mcuSteps > rate ) m_stepsPerus = (int)

        double mcuSteps = McuComponent::self()->freq()/m_stepsPerus;
        BaseProcessor::self()->setSteps( mcuSteps );
        m_mcuStepNS = 1000/McuComponent::self()->freq();
        m_runMcu = true;
        //qDebug() <<"Simulator::simuRateChanged mcuSteps"<<mcuSteps<<m_mcuStepNS;
    }

    m_timerTick  = 50/m_timerSc;
    int fps = 1000/m_timerTick;
    int mult = 20;
    if( fps == 40 ) mult = 40;

    m_circuitRate = rate/fps;

    if( rate < fps )
    {
        fps = rate;
        m_circuitRate = 1;
        m_timerTick = 1000/rate;
    }

    if( m_isrunning )
    {
        pauseSim();
        resumeSim();
    }

    //PlotterWidget::self()->setPlotterTick( m_circuitRate*mult );
    PlotterWidget::self()->setPlotterTick( m_circuitRate*mult/m_stepsPerus );

    m_simuRate = m_circuitRate*fps;

    std::cout << "\nFPS:              " << fps
              << "\nCircuit Rate:     " << m_circuitRate
              << std::endl
              << "\nSimulation Speed: " << m_simuRate
              << "\nReactive SubRate: " << m_stepsPrea
              << std::endl
              << std::endl;

    return m_simuRate;
}

double Simulator::stepsPerus()
{
    return m_stepsPerus;
}

bool Simulator::isRunning()
{
    return m_isrunning;
}

bool Simulator::isPaused()
{
    return m_paused;
}

int Simulator::reaClock()
{
    return m_stepsPrea;
}

void Simulator::setReaClock( int value )
{
    bool running = m_isrunning;
    if( running ) stopSim();

    if     ( value < 1  )  value = 1;
    else if( value > 100 ) value = 100;

    m_stepsPrea = value;

    if( running ) runContinuous();
}

int Simulator::noLinAcc(){ return m_noLinAcc; }
void  Simulator::setNoLinAcc( int ac )
{
    bool running = m_isrunning;
    if( running ) stopSim();

    if     ( ac < 3 )  ac = 3;
    else if( ac > 14 ) ac = 14;
    m_noLinAcc = ac;

    if( running ) runContinuous();
}
double Simulator::NLaccuracy()
{
    return 1/pow(10,m_noLinAcc)/2;
}

uint64_t Simulator::step()
{
    return m_step;
}

uint64_t Simulator::circTime()
{
    return m_circTime;
}

void Simulator::addToEnodeBusList( eNode* nod )
{
    if( !m_eNodeBusList.contains(nod) ) m_eNodeBusList.append( nod );
}

void Simulator::remFromEnodeBusList( eNode* nod, bool del )
{
    if( m_eNodeBusList.contains(nod) ) m_eNodeBusList.removeOne( nod );
    if( del ){ delete nod; }
}

void Simulator::addToEnodeList( eNode* nod )
{
    if( !m_eNodeList.contains(nod) ) m_eNodeList.append( nod );
}

void Simulator::remFromEnodeList( eNode* nod, bool del )
{
    if( m_eNodeList.contains(nod) )
    {
        m_eNodeList.removeOne( nod );
        if( del ) delete nod;
    }
}

void Simulator::addToChangedNodeList( eNode* nod )
{
    if( !m_eChangedNodeList.contains(nod) ) m_eChangedNodeList.append( nod );
}
void Simulator::remFromChangedNodeList( eNode* nod )
{
    m_eChangedNodeList.removeOne( nod );
}

void Simulator::addToElementList( eElement* el )
{
    if( !m_elementList.contains(el) ) m_elementList.append(el);
}

void Simulator::remFromElementList( eElement* el )
{
    if( m_elementList.contains(el) ) m_elementList.removeOne(el);
}

void Simulator::addToUpdateList( eElement* el )
{
    if( !m_updateList.contains(el) ) m_updateList.append(el);
}

void Simulator::remFromUpdateList( eElement* el )
{
    m_updateList.removeOne(el);
}

void Simulator::addToSimuClockList( eElement* el )
{
    if( !m_simuClock.contains(el) ) m_simuClock.append(el);
}

void Simulator::remFromSimuClockList( eElement* el )
{
    m_simuClock.removeOne(el);
}

void Simulator::addToChangedFast( eElement* el )
{
    if( !m_changedFast.contains(el) ) m_changedFast.append(el);
}

void Simulator::remFromChangedFast( eElement* el )
{
    m_changedFast.removeOne(el);
}

void Simulator::addToReactiveList( eElement* el )
{
    if( !m_reactive.contains(el) ) m_reactive.append(el);
}

void Simulator::remFromReactiveList( eElement* el )
{
    m_reactive.removeOne(el);
}

void Simulator::addToNoLinList( eElement* el )
{
    if( !m_nonLinear.contains(el) ) m_nonLinear.append(el);
}

void Simulator::remFromNoLinList( eElement* el )
{
    m_nonLinear.removeOne(el);
}

void Simulator::addToMcuList( BaseProcessor* proc )
{
    if( !m_mcuList.contains(proc) ) m_mcuList.append( proc );
}
void Simulator::remFromMcuList( BaseProcessor* proc ) { m_mcuList.removeOne( proc ); }

#include "moc_simulator.cpp"

