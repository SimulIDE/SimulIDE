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

#include "filewidget.h"
#include "mainwindow.h"
#include "simuapi_apppath.h"

FileWidget* FileWidget::m_pSelf = 0l;

FileWidget::FileWidget( QWidget* parent )
          : QWidget( parent )
{
    m_pSelf = this;
    
    m_blocked = false;
    
    QVBoxLayout* vLayout = new QVBoxLayout( this );
    
    m_bookmarks = new QListWidget( this );
    vLayout->addWidget( m_bookmarks );
    
    QHBoxLayout* hLayout = new QHBoxLayout();
    vLayout->addLayout( hLayout );
    
    m_cdUpButton = new QPushButton( this );
    m_cdUpButton->setIcon( QIcon(":/cdup.png") );
    m_cdUpButton->setToolTip( tr("cd Up") );
    hLayout->addWidget( m_cdUpButton );
    
    m_path = new QLineEdit( this );
    hLayout->addWidget( m_path );
    
    m_fileBrowser = new FileBrowser( this );
    m_fileBrowser->setPath( QDir::rootPath() );
    vLayout->addWidget( m_fileBrowser );
    
    QSettings* settings = MainWindow::self()->settings();
    QDir setDir( settings->fileName() );
    setDir.cdUp( );
    QString settingsDir = setDir.absolutePath() ;
    
    addEntry( "FileSystem", QDir::rootPath() );
    addEntry( "Home",       QDir::homePath() );
    addEntry( "Examples",   SIMUAPI_AppPath::self()->ROExamFolder().absolutePath() );
    addEntry( "Data",       SIMUAPI_AppPath::self()->RODataFolder().absolutePath() );
    addEntry( "Settings",   settingsDir );
    
    
    connect( m_bookmarks, SIGNAL( itemClicked( QListWidgetItem* )), 
             this,        SLOT(   itemClicked( QListWidgetItem* )));
             
    connect( m_cdUpButton,        SIGNAL(released()),
             FileBrowser::self(), SLOT(  cdUp()));
             
    connect( m_path, SIGNAL( editingFinished() ),
             this,   SLOT(  pathChanged()));
             
    int size = settings->beginReadArray("bookmarks");
    
    for( int i=0; i<size; i++ ) 
    {
        settings->setArrayIndex(i);
        addBookMark( settings->value("path").toString() );
    }
    settings->endArray();
}
FileWidget::~FileWidget()
{
}

void FileWidget::writeSettings()
{
    QSettings* settings = MainWindow::self()->settings();
    
    settings->beginWriteArray("bookmarks");
    
    for( int i=0; i<m_bookmarkList.size(); i++ ) 
    {
        settings->setArrayIndex(i);
        settings->setValue("path", m_bookmarkList.at(i));
    }
    settings->endArray(); 
    
    for( int i=0; i<m_bookmarks->count(); i++ )
    {
        QListWidgetItem* item =  m_bookmarks->takeItem( i );
        delete item;
    }
}

void FileWidget::addEntry( QString name, QString path )
{
    QListWidgetItem* item = new QListWidgetItem( name, m_bookmarks, 0 );
    item->setData( 4, path );
    
    QFont font;
    font.setPixelSize( 11*MainWindow::self()->fontScale() );
    font.setWeight(70);
    item->setFont( font );
    item->setIcon( QIcon(":/open.png") );
    
    resizeToItems();
}

void FileWidget::addBookMark( QString path )
{
    QDir dir = QDir( path );
    
    QString dirPath = dir.absolutePath();
    QString name    = dir.dirName();
    
    name += " ("+dirPath+")";
    
    if( !m_bookmarkList.contains( dirPath ) )
    {
        m_bookmarkList.append( dirPath );
        addEntry( name, dirPath );
    }
}

void FileWidget::remBookMark()
{
    QListWidgetItem* item =  m_bookmarks->takeItem( m_bookmarks->currentRow() );
    m_bookmarkList.removeOne( item->data( 4 ).toString() );
    delete item;
    
    resizeToItems();
}

void FileWidget::itemClicked( QListWidgetItem* item  )
{
    QString path = item->data( 4 ).toString();
    
    m_fileBrowser->setPath( path );
}

void FileWidget::setPath( QString path )
{
    m_blocked = true;
    m_path->setText( path );
    m_blocked = false;
}

void FileWidget::pathChanged()
{
    if( m_blocked ) return;        // Avoid circular call with setPath()
    
    QString path = m_path->text();
    FileBrowser::self()->setPath( path );
}

void FileWidget::resizeToItems()
{
    int size = m_bookmarks->count()*22+17;
    m_bookmarks->setFixedHeight( size );
}

void FileWidget::contextMenuEvent( QContextMenuEvent* event )
{
    QListWidgetItem* item =  m_bookmarks->currentItem();
    
    if( m_bookmarkList.contains( item->data( 4 ).toString() ) )
    {
        QPoint eventPos = event->globalPos();

        QMenu menu;

        QAction* remBookMarkAction = menu.addAction(QIcon(":/remove.png"),tr("Remove Bookmark"));
        connect( remBookMarkAction, SIGNAL( triggered()), 
                 this,              SLOT(   remBookMark() ) );
                 
        menu.exec( eventPos );
    }
}
