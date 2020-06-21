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

#include "picasmdebugger.h"
#include "baseprocessor.h"
#include "utils.h"
#include "simuapi_apppath.h"

PicAsmDebugger::PicAsmDebugger( QObject* parent, OutPanelText* outPane, QString filePath )
              : BaseDebugger( parent,outPane, filePath )
{
    setObjectName( "PIC asm Compiler/Debugger" );
    
    m_compilerPath = "";
    m_compSetting = "gpasm_Path";
    
    readSettings();

    m_typesList["byte"]    = "uint8";
}
PicAsmDebugger::~PicAsmDebugger() {}

int PicAsmDebugger::compile()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    //getProcType();
    QString file = m_fileDir+m_fileName+m_fileExt;
    
    m_outPane->writeText( "-------------------------------------------------------\n" );
    QString command = m_compilerPath+"gpasm";
    
    QProcess checkComp( this );
    checkComp.start( command  );
    checkComp.waitForFinished(-1);
    
    QString p_stdo = checkComp.readAllStandardOutput();
    if( !p_stdo.toUpper().contains("OPTIONS") )
    {
        m_outPane->appendText( "\ngpasm" );
        toolChainNotFound();
        return -1;
    }
    
    #ifndef Q_OS_UNIX
    command  = addQuotes( command );
    file     = addQuotes( file );
    #endif

    command.append(" -L -w1  -i  -ainhx32 -I "+m_fileDir+" "+ file);

    m_outPane->appendText( "Exec: ");
    m_outPane->appendText( command );
    m_outPane->writeText( "\n\n" );
    
    QProcess compAsm( this );
    compAsm.start( command  );
    compAsm.waitForFinished(-1);
    
    QString p_stdout = compAsm.readAllStandardOutput();
    QString p_stderr = compAsm.readAllStandardError();
    m_outPane->writeText( p_stdout );
    m_outPane->writeText( p_stderr );

    int error = 0;
    if( p_stdout.toUpper().contains("ERROR") ) 
    {
        QStringList lines = p_stdout.split("\n");
        for( QString line : lines )
        {
            if( !(line.toUpper().contains( "ERROR" )) ) continue;
            QStringList words = line.split(":");
            if( words.size() > 1)
            {
                error = words.at(1).toInt();
                qDebug() <<line;
            }
            else error = -1;
            break;
        }
    }
    m_firmware = m_fileDir+m_fileName+".hex";
    
    QApplication::restoreOverrideCursor();
    return error;
}

void PicAsmDebugger::mapFlashToSource()
{
    m_flashToSource.clear();
    m_sourceToFlash.clear();

    QString asmFileName = m_fileDir + m_fileName + ".asm";
    QString lstFileName = m_fileDir + m_fileName + ".lst";

    QStringList asmLines = fileToStringList( asmFileName, "PicAsmDebugger::mapLstToAsm" );
    QStringList lstLines = fileToStringList( lstFileName, "PicAsmDebugger::mapLstToAsm" );
    
    m_lastLine = 0;

    QString asmLine;
    int asmLineNumber = 0;
    int lastAsmLine = asmLines.size();

    for( QString line : lstLines )
    {
        if( line.isEmpty() )      continue;
        line = line.toUpper().replace("\t", " ");
        if( line.startsWith(" ") ) continue;
        if( line.contains("ORG") ) continue;

        while( true )
        {
            if( ++asmLineNumber >= lastAsmLine ) break; // End of asm file
            asmLine = asmLines.at( asmLineNumber ).toUpper();
            asmLine = asmLine.replace("\t", " ").remove(" ");
            if( asmLine.isEmpty() )      continue;
            if( asmLine.startsWith("_")) continue;
            if( asmLine.startsWith(";")) continue;
            if( asmLine.startsWith("#")) continue;
            if( asmLine.startsWith(".")) continue;

            QString lstline = line;
            if( lstline.remove(" ").contains(asmLine) ) break;
        }
        if( asmLineNumber >= lastAsmLine )
        {
            asmLineNumber = 0;
            continue; // End of asm file
        }
        QStringList words = line.split(' ');
        QString numberText = words.at(0);
        //QString numberText = line.left( 4 );
        bool ok = false;
        int address = numberText.toInt( &ok, 16 );  // original adress*2: instruc = 2 bytes
        if( ok )
        {
            m_flashToSource[address] = asmLineNumber;
            if( asmLineNumber > m_lastLine ) m_lastLine = asmLineNumber;
            //qDebug() <<"asmLineNumber"<<asmLineNumber<<"address"<<address;
        }
    }
    QHashIterator<int, int> i( m_flashToSource );
    while( i.hasNext() )
    {
        i.next();
        int address       = i.key();
        int asmLineNumber = i.value();
        m_sourceToFlash[asmLineNumber] = address;
    }
}

#include "moc_picasmdebugger.cpp"
