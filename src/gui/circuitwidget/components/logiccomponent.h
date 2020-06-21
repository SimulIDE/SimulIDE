/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#ifndef LOGICCOMPONENT_H
#define LOGICCOMPONENT_H

#include "component.h"
#include "pin.h"

class MAINMODULE_EXPORT LogicComponent : public Component
{
    Q_OBJECT
    public:

        LogicComponent( QObject* parent, QString type, QString id );
        ~LogicComponent();
                
        virtual void init( QStringList pins );

        virtual void setNumInps( int inPins );

        virtual void setNumOuts(int outPins );

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    public slots:
        virtual void remove();
        
    protected:
        Pin* createPin( QString data , QString pinId );
        
        void deleteInputs( int inputs );
        void deleteOutputs( int inputs );
    
        std::vector<Pin*> m_inPin;
        std::vector<Pin*> m_outPin;
        
        int m_width;
        int m_height;
    
        int m_numInPins;
        int m_numOutPins;
};

#endif
