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

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <qtconcurrentrun.h>
#include <QElapsedTimer>

#include "circmatrix.h"

class BaseProcessor;
class eElement;
class eNode;

class MAINMODULE_EXPORT Simulator : public QObject
{
    Q_OBJECT
    public:
        Simulator( QObject* parent=0 );
        ~Simulator();

 static Simulator* self() { return m_pSelf; }

        void runContinuous();
        void stopTimer();
        void resumeTimer();
        void pauseSim();
        void resumeSim();
        void stopSim();
        void stopDebug();
        void startSim();
        void debug( bool run );
        void runGraphicStep();
        void runExtraStep( uint64_t cycle );
        void runGraphicStep1();
        void runGraphicStep2();
        void runCircuit();
        
        int circuitRate() { return m_circuitRate; }
        int simuRate() { return m_simuRate; }
        int simuRateChanged( int rate );
        
        void setTimerScale( int ts ) { m_timerSc = ts; }

        int  reaClock();
        void setReaClock( int value );

        int  noLinAcc();
        void setNoLinAcc( int ac );
        double NLaccuracy();
        
        bool isRunning();
        bool isPaused();
        
        uint64_t step();
        uint64_t circTime();
        void setCircTime( uint64_t time );

        QList<eNode*> geteNodes() { return m_eNodeList; }

        void addToEnodeBusList( eNode* nod );
        void remFromEnodeBusList( eNode* nod, bool del );

        void addToEnodeList( eNode* nod );
        void remFromEnodeList( eNode* nod, bool del );
        
        void addToChangedNodeList( eNode* nod );
        void remFromChangedNodeList( eNode* nod );
        
        void addToElementList( eElement* el );
        void remFromElementList( eElement* el );
        
        void addToUpdateList( eElement* el );
        void remFromUpdateList( eElement* el );
        
        void addToChangedFast( eElement* el );
        void remFromChangedFast( eElement* el );
        
        void addToReactiveList( eElement* el );
        void remFromReactiveList( eElement* el );
        
        void addToSimuClockList( eElement* el );
        void remFromSimuClockList( eElement* el );
        
        void addToNoLinList( eElement* el );
        void remFromNoLinList( eElement* el );
        
        void addToMcuList( BaseProcessor* proc );
        void remFromMcuList( BaseProcessor* proc );

        void timerEvent( QTimerEvent* e );

        double stepsPerus();
        
        uint64_t stepsPerSec;

        uint64_t mS(){ return m_RefTimer.elapsed(); }

    signals:
        void pauseDebug();
        void resumeDebug();
        
    private:
 static Simulator* m_pSelf;

        inline void solveMatrix();

        QFuture<void> m_CircuitFuture;

        CircMatrix m_matrix;

        QList<eNode*>    m_eNodeList;
        QList<eNode*>    m_eChangedNodeList;
        QList<eNode*>    m_eNodeBusList;

        QList<eElement*> m_elementList;
        QList<eElement*> m_updateList;
        
        QList<eElement*> m_changedFast;
        QList<eElement*> m_reactive;
        QList<eElement*> m_nonLinear;
        QList<eElement*> m_simuClock;
        QList<BaseProcessor*> m_mcuList;

        bool m_isrunning;
        bool m_debugging;
        bool m_runMcu;
        bool m_paused;
        bool m_error;

        int m_timerId;
        int m_timerTick;
        int m_timerSc;
        int m_noLinAcc;
        int m_numEnodes;
        int m_simuRate;
        int m_stepsPrea;

        double m_stepsPerus;
        double m_stepNS;
        double m_mcuStepNS;
        
        uint64_t m_circuitRate;
        uint64_t m_reacCounter;
        uint64_t m_updtCounter;

        uint64_t m_circTime;
        uint64_t m_step;
        uint64_t m_tStep;
        uint64_t m_lastStep;
        
        uint64_t m_refTime;
        uint64_t m_lastRefTime;
        QElapsedTimer m_RefTimer;
};
 #endif


