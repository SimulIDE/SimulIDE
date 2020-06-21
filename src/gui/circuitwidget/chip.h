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

#ifndef CHIP_H
#define CHIP_H

#include "component.h"
#include "e-element.h"


class MAINMODULE_EXPORT Chip : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( bool Logic_Symbol READ logicSymbol WRITE setLogicSymbol DESIGNABLE true USER true )

    public:
        Chip( QObject* parent, QString type, QString id );
        ~Chip();
        
        bool logicSymbol();
        virtual void setLogicSymbol( bool ls );

        virtual void initEpins(){;}

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        virtual void remove();

    protected:
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

        virtual void addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle );
        
        virtual void updatePin( QString id, QString type, QString label,
                                int pos, int xpos, int ypos, int angle ){;}

        virtual void initChip();

        int m_numpins;
        int m_width;
        int m_height;
        
        bool m_isLS;
        bool m_initialized;

        QColor m_lsColor;
        QColor m_icColor;

        QString m_pkgeFile;     // file containig package defs
        //QString m_dataFile;     // xml file containig entry
        
        QList<Pin*> m_topPin;
        QList<Pin*> m_botPin;
        QList<Pin*> m_lefPin;
        QList<Pin*> m_rigPin;
};
#endif

