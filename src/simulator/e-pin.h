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

#ifndef EPIN_H
#define EPIN_H

//Have to use #include <QtCore/QtGlobal> for Q_DECL_EXPORT?
#include <string>

class eNode;

class MAINMODULE_EXPORT ePin
{
    public:
        ePin( std::string id, int index );
        ~ePin();

        bool isConnected();
        void setConnected( bool connected );

        double getVolt();

        eNode* getEnode();
        void   setEnode( eNode* enode );

        eNode* getEnodeComp();
        void   setEnodeComp( eNode* enode ); // The enode at other side of component

        bool inverted();
        void setInverted( bool inverted );

        void stampAdmitance( double data );
        void stampCurrent( double data );

        void reset();
        
        std::string getId();
        void setId( std::string id );

    protected:
        eNode* m_enode;
        eNode* m_enodeCon;

        std::string m_id;
        int m_index;

        bool m_connected;
        bool m_inverted;
};

#endif
