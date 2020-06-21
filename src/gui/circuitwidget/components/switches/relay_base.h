/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#ifndef RELAY_BASE_H
#define RELAY_BASE_H

#include "e-inductor.h"
#include "mech_contact.h"


class MAINMODULE_EXPORT RelayBase : public MechContact
{
    Q_OBJECT
    Q_PROPERTY( bool DT         READ dt       WRITE setDt    DESIGNABLE true USER true )
    Q_PROPERTY( double Rcoil    READ rCoil    WRITE setRCoil DESIGNABLE true USER true )
    Q_PROPERTY( double IOn      READ iTrig    WRITE setITrig DESIGNABLE true USER true )
    Q_PROPERTY( double IOff     READ iRel     WRITE setIRel  DESIGNABLE true USER true )
    Q_PROPERTY( double Inductance READ induc WRITE setInduc  DESIGNABLE true USER true )

    public:

        RelayBase( QObject* parent, QString type, QString id );
        ~RelayBase();

        double rCoil() const;
        void setRCoil(double res);

        double iTrig() const;
        void setITrig( double current );

        double iRel() const;
        void setIRel( double current );

        double induc();
        void  setInduc( double c );

        virtual void stamp();

        void setVChanged();

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    public slots:
        virtual void remove();

    protected:
        eResistor* m_resistor;
        eInductor* m_inductor;

        eNode* m_internalEnode;
        double m_trigCurrent;
        double m_relCurrent;
};

#endif
