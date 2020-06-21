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

#include "codeeditorwidget.h"
#include "mainwindow.h"

CodeEditorWidget::CodeEditorWidget( QWidget *parent ) 
                : QWidget( parent )
{
    createWidgets();
    m_outPane->appendPlainText( tr("Ready")+"\n" );
}

CodeEditorWidget::~CodeEditorWidget() {}

void CodeEditorWidget::createWidgets()
{
    QGridLayout* codeWidgetLayout = new QGridLayout( this );
    codeWidgetLayout->setSpacing(0);
    codeWidgetLayout->setContentsMargins(0, 0, 0, 0);
    codeWidgetLayout->setObjectName( "codeWidgetLayout" );
    
    QSplitter* splitter0 = new QSplitter( this );
    splitter0->setObjectName("splitter0");
    splitter0->setOrientation( Qt::Vertical );
    codeWidgetLayout->addWidget( splitter0 );

    m_outPane    = new OutPanelText( this );
    m_codeEditor = new CodeEditor( this, m_outPane );
    
    splitter0->addWidget( m_codeEditor );
    splitter0->addWidget( m_outPane );

    QList <int> sizes;
    sizes<<300<<100;
    splitter0->setSizes( sizes );
}

void CodeEditorWidget::setVisible( bool visible)
{
    m_outPane->setVisible( visible );
    QWidget::setVisible( visible );
}

#include  "moc_codeeditorwidget.cpp"
