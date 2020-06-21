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

#ifndef SUBCIRCUIT_H
#define SUBCIRCUIT_H

#include "chip.h"
#include "itemlibrary.h"

class MAINMODULE_EXPORT SubCircuit : public Chip
{
    Q_OBJECT

    public:

        SubCircuit( QObject* parent, QString type, QString id );
        ~SubCircuit();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void initChip();

        virtual void initialize();
        virtual void attach();

        virtual void setLogicSymbol( bool ls );
        
    public slots:
        virtual void remove();

    protected:
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

        virtual void initSubcircuit();
        
        void clear();

        void connectEpin( ePin *epin, QString connetTo );

        int m_numItems;
        
        QString m_subcFile;     // file containig subcircuit defs

        QList<eNode*> m_internal_eNode;
        QList<eElement*> m_elementList;
        QVector<QList<ePin*> > m_pinConections;
};
#endif

