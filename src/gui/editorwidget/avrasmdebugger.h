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

#ifndef AVRASMDEBUGGER_H
#define AVRASMDEBUGGER_H

#include <QtGui>
#include <QHash>

#include "basedebugger.h"

class AvrAsmDebugger : public BaseDebugger
{
    Q_OBJECT
    Q_PROPERTY( QString  Avra_Inc_Path   READ avraIncPath    WRITE setAvraIncPath   DESIGNABLE true USER true )
    
    public:
        AvrAsmDebugger( QObject* parent, OutPanelText* outPane, QString filePath  );
        ~AvrAsmDebugger();
        
        QString avraIncPath();
        void    setAvraIncPath( QString path );

        int  compile();
        void mapFlashToSource();
        
    private:
        QString m_avraIncPath;                  // path to avra includes
};


#endif
