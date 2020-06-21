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

#ifndef INODEBUGGER_H
#define INODEBUGGER_H

#include <QHash>

#include "basedebugger.h"

class InoDebugger : public BaseDebugger
{
    Q_OBJECT
    Q_PROPERTY( board_t Board        READ board       WRITE setBoard       DESIGNABLE true USER true )
    Q_PROPERTY( QString Custom_Board READ customBoard WRITE setCustomBoard DESIGNABLE true USER true )
    Q_ENUMS( board_t )
    
    public:
        InoDebugger( QObject* parent, OutPanelText* outPane, QString filePath  );
        ~InoDebugger();
        
        enum board_t {
            Uno = 0,
            Mega,
            Nano,
            Duemilanove,
            Leonardo,
            Custom
        };
        
        QString customBoard() { return m_customBoard; }
        void setCustomBoard( QString b ){ m_customBoard = b; }
        
        board_t board() { return m_board; }
        void setBoard( board_t b ){ m_board = b; }

        int compile();
        void mapFlashToSource();
        
        virtual void upload();
        
    protected:
        void getVariables();
        
    private:
        int m_lastInoLine;
        int m_loopInoLine;
        int m_processorType;
        
        QStringList boardList;
        
        QString m_customBoard;
        board_t m_board;
};


#endif
