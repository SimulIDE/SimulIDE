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

#include "ramtable.h"
#include "baseprocessor.h"
#include "basedebugger.h"
#include "mainwindow.h"
#include "utils.h"

RamTable::RamTable( BaseProcessor *processor )
        : QTableWidget( 60, 4 )
{
    m_processor = processor;
    m_debugger  = 0l;
    m_numRegs = 60;
    m_loadingVars = false;
    
    verticalHeader()->setSectionsMovable( true );
    
    setColumnWidth(0, 60);
    setColumnWidth(1, 55);
    setColumnWidth(2, 35);
    setColumnWidth(3, 80);
    
    int row_heigh = 23;

    QTableWidgetItem *it;

    for( int row=0; row<m_numRegs; row++ )
    {
        it = new QTableWidgetItem(0);
        it->setText( "---" );
        setVerticalHeaderItem( row, it );
        for( int col=0; col<4; col++ )
        {
            QTableWidgetItem *it = new QTableWidgetItem(0);
            if( col>0 ) it->setFlags( Qt::ItemIsEnabled );
            setItem( row, col, it );
        }
        QFont font = item( 0, 0 )->font();
        font.setBold(true);
        font.setPixelSize( 10*MainWindow::self()->fontScale() );
        for( int col=0; col<4; col++ ) item( row, col )->setFont( font );
        
        item( row, 1 )->setText("---");
        item( row, 2 )->setText("---");
        item( row, 3 )->setText("---");
        
        setRowHeight(row, row_heigh);
    }

    it = new QTableWidgetItem(0);
    it->setText( tr("Reg.") );
    setHorizontalHeaderItem( 0, it );

    it = new QTableWidgetItem(0);
    it->setText( tr("Type") );
    setHorizontalHeaderItem( 1, it );

    it = new QTableWidgetItem(0);
    it->setText( tr("Dec") );
    setHorizontalHeaderItem( 2, it );
    
    it = new QTableWidgetItem(0);
    it->setText( tr("Value") );
    setHorizontalHeaderItem( 3, it );
    
    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, SIGNAL(customContextMenuRequested(const QPoint&)),
             this, SLOT  (slotContextMenu(const QPoint&)));

    connect( this, SIGNAL(itemChanged(QTableWidgetItem*)  ), 
             this, SLOT(addToWatch(QTableWidgetItem*)) );

    show();
}
RamTable::~RamTable(){}

void RamTable::slotContextMenu( const QPoint& point )
{
    QMenu menu;
    if( m_debugger )
    {
        QAction *loadVars = menu.addAction( QIcon(":/open.png"),tr("Load Variables") );
        connect( loadVars, SIGNAL(triggered()), this, SLOT(loadVariables()) );
    }
    
    QAction *clearSelected = menu.addAction( QIcon(":/remove.png"),tr("Clear Selected") );
    connect( clearSelected, SIGNAL(triggered()), this, SLOT(clearSelected()) );
    
    QAction *clearTable = menu.addAction( QIcon(":/remove.png"),tr("Clear Table") );
    connect( clearTable, SIGNAL(triggered()), this, SLOT(clearTable()) );
    
    menu.addSeparator();
    
    QAction *loadVarSet = menu.addAction( QIcon(":/open.png"),tr("Load VarSet") );
    connect( loadVarSet, SIGNAL(triggered()), this, SLOT(loadVarSet()) );
    
    QAction *saveVarSet = menu.addAction( QIcon(":/save.png"),tr("Save VarSet") );
    connect( saveVarSet, SIGNAL(triggered()), this, SLOT(saveVarSet()) );

    menu.exec( mapToGlobal(point) );
}

void RamTable::clearSelected()
{
    for( QTableWidgetItem* item : selectedItems() ) item->setData( 0, "");
}

void RamTable::clearTable()
{
    for( QTableWidgetItem* item : findItems( "*", Qt::MatchWildcard)  )
    { if( item ) item->setData( 0, "");}
}

void RamTable::loadVarSet()
{
    m_loadingVars = true;
    
    const QString dir = m_processor->getFileName();
    //QCoreApplication::applicationDirPath()+"/data/varset";
    QString fileName = QFileDialog::getOpenFileName( this, tr("Load VarSet"), dir, tr("VarSets (*.vst);;All files (*.*)"));
    
    if( !fileName.isEmpty() )
    {
        QStringList lines = fileToStringList( fileName, "RamTable::loadVarSet" );
        int row = -1;
        for( QString line : lines )
        {
            line.remove( " " ).remove("\t");
            QStringList words = line.split( "=" );
            QString name = words.first();
            
            if( !words.last().isEmpty() )
            {
                int address = words.last().toInt();
                m_processor->addWatchVar( name, address, "uint8" );
            }
            if( row >= 0 ) item( row, 0 )->setText( name );
            row++;
            if( row >= m_numRegs ) break;
        }
    }
    m_loadingVars = false;
}
void RamTable::saveVarSet()
{
    const QString dir = m_processor->getFileName();
    //QCoreApplication::applicationDirPath()+"/data/varset";
    
    QString fileName = QFileDialog::getSaveFileName( this, tr("Save VarSet"), dir,
                                                 tr("VarSets (*.vst);;All files (*.*)"));
    if( !fileName.isEmpty() )
    {
        if( !fileName.endsWith(".vst") ) fileName.append(".vst");

        QFile file( fileName );

        if( !file.open(QFile::WriteOnly | QFile::Text) )
        {
              QMessageBox::warning(0l, "RamTable::saveVarSet",
              tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
              return;
        }

        QTextStream out(&file);
        out.setCodec("UTF-8");
        QApplication::setOverrideCursor(Qt::WaitCursor);
        
        for( int row=0; row<m_numRegs; row++ )
        {
            QString name = item( row, 0 )->text();
            out << name << "\n";
        }
        QApplication::restoreOverrideCursor();
    }
}

void RamTable::loadVariables()
{
    if( !m_debugger ) return;
    
    m_loadingVars = true;
    
    QStringList variables = m_debugger->getVarList();
    //qDebug() << "RamTable::loadVariables" << variables;
    
    for( QString var : variables )
    {
        int row = currentRow()+1;
        if( row >= m_numRegs ) break;
        item( row, 0 )->setText( var );
    }
    m_loadingVars = false;
}

void RamTable::updateValues()
{
    if( m_processor )
    {
        for( int _row: watchList.keys() )
        {
            m_currentRow = _row;
            QString name = watchList[_row];
            
            bool ok;
            int addr = name.toInt(&ok, 10); 
            if( !ok ) addr = name.toInt(&ok, 16);  
            if( !ok ) m_processor->updateRamValue( name );  // Var or Reg name
            else                                            // Address
            {
                int value = m_processor->getRamValue( addr );

                if( value >= 0 )
                {
                    item( _row, 1 )->setText("uint8");
                    item( _row, 2 )->setData( 0, value );
                    item( _row, 3 )->setData( 0, decToBase(value, 2, 8) );
                }
            }
        }
    }
}

void RamTable::setItemValue( int col, QString value  )
{
    item( m_currentRow, col )->setData( 0, value );
}

void RamTable::setItemValue( int col, float value  )
{
    item( m_currentRow, col )->setData( 0, value );
}

void RamTable::setItemValue( int col, int32_t value  )
{
    item( m_currentRow, col )->setData( 0, value );
}

void RamTable::addToWatch( QTableWidgetItem* it )
{
    if( column(it) != 0 ) return;
    int _row = row(it);
    setCurrentCell( _row, 0 );
    
    QString name = it->text().remove(" ").remove("\t").remove("*");//.toLower();

    if( name.isEmpty() )
    {
        watchList.remove(_row);
        verticalHeaderItem( _row )->setText("---");

        item( _row, 3 )->setText("---");
        item( _row, 2 )->setText("---");
        item( _row, 1 )->setText("---");
    }
    else
    {
        int value = m_processor->getRegAddress( name );
        if( value < 0 )
        {
            bool ok;
            value = name.toInt(&ok, 10); 
            if( !ok ) value = name.toInt(&ok, 16);  
            if( !ok ) value = -1;
        }
        if( value >= 0 )
        {
            watchList[_row] = name;
            verticalHeaderItem( _row )->setData( 0, value );
        }
        if( !m_debugger ) return;
        QString varType = m_debugger->getVarType( name );
        
        if( !m_loadingVars && varType.contains( "array" ) )
        {
            int size = varType.replace( "array", "" ).toInt();
            
            QStringList variables = m_debugger->getVarList();

            int indx = variables.indexOf( name );
            int listEnd = variables.size()-1;
            for( int i=1; i<size ; i++ )
            {
                int index = indx+i;
                if( index > listEnd ) break;
                
                QString varName = variables.at( index );
                if( varName.contains( name ) ) item( _row+i, 0 )->setText( varName );
            }
        }
    }
}

void RamTable::setDebugger( BaseDebugger* deb )
{
    m_debugger = deb;
}

void RamTable::remDebugger( BaseDebugger* deb )
{
    if( m_debugger == deb ) m_debugger = 0l;
}

#include "moc_ramtable.cpp"

