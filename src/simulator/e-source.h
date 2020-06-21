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

#ifndef ESOURCE_H
#define ESOURCE_H


#include "e-element.h"
#include "e-node.h"

class MAINMODULE_EXPORT eSource : public eElement
{
    public:
        eSource( std::string id, ePin* epin );
        virtual ~eSource();

        virtual void initialize();
        
        void stamp();
        void stampOutput();

        double voltHight();
        void  setVoltHigh( double v );

        double voltLow();
        void  setVoltLow( double v );
        
        bool  out();
        void  setOut( bool hight );

        bool  isInverted();
        void  setInverted( bool inverted );

        double imp();
        void  setImp( double imp );

        double getVolt();

        ePin* getEpin();
        ePin* getEpin( QString pinName );

        void  createPin();

    protected:
        
        double m_voltHigh;
        double m_voltLow;
        double m_voltOut;
        double m_imp;
        double m_admit;

        bool m_out;
        bool m_inverted;

        eNode* m_scrEnode;
};
#endif

