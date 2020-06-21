/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "propertieswidget.h"
#include "mainwindow.h"

PropertiesWidget* PropertiesWidget::m_pSelf = 0l;

PropertiesWidget::PropertiesWidget( QWidget *parent ) 
                :QWidget( parent )
{
    m_pSelf   = this;
    
    createWidgets();
}

PropertiesWidget::~PropertiesWidget()
{
}

void PropertiesWidget::createWidgets()
{
    QGridLayout* widgetLayout = new QGridLayout( this );
    widgetLayout->setSpacing(0);
    widgetLayout->setContentsMargins(0, 0, 0, 0);
    widgetLayout->setObjectName( "widgetLayout" );
    
    QSplitter* splitter0 = new QSplitter( this );
    splitter0->setObjectName("splitter0");
    splitter0->setOrientation( Qt::Vertical );
    widgetLayout->addWidget( splitter0 );

    m_properties = new  QPropertyEditorWidget( this );
    m_help =       new QPlainTextEdit( this );
    
    splitter0->addWidget( m_properties );
    splitter0->addWidget( m_help );
    
    m_help->appendPlainText( tr( "Here will be some help ..............................................\n" ));
    m_help->setReadOnly( true );
}

void PropertiesWidget::setHelpText( QString* text )
{
    if( !text ) return;
    m_help->clear();
    m_help->appendPlainText( *text );
}

#include  "moc_propertieswidget.cpp"
