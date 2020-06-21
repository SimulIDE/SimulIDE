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

#include <QVariant>

#include "voltwidget.h"

VoltWidget::VoltWidget()
{
    setupWidget();
    
    setFixedSize( 60, 90 );
}
VoltWidget::~VoltWidget() {}

void VoltWidget::setupWidget()
{
    DialWidget::setupWidget();
    
    dial->setMinimum(0);
    dial->setMaximum(500);
    dial->setValue(000);
    dial->setSingleStep(10);
    
    pushButton = new QPushButton(this);
    pushButton->setObjectName( QString::fromUtf8("pushButton") );
    pushButton->setCheckable(true);
    pushButton->setForegroundRole( QPalette::BrightText );
    pushButton->setFixedSize( 40, 17 );

    QFont sansFont("Helvetica [Cronyx]", 8 );
    sansFont.setPixelSize(10);
    pushButton->setFont( sansFont );
    
    verticalLayout->addWidget(pushButton);
    verticalLayout->setAlignment( pushButton, Qt::AlignHCenter );
}

