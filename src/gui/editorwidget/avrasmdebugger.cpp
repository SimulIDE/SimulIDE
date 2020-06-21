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

#include "avrasmdebugger.h"
#include "baseprocessor.h"
#include "mainwindow.h"
#include "utils.h"
#include "simuapi_apppath.h"

static const char* AvrAsmDebugger_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Avra Inc Path")
};

AvrAsmDebugger::AvrAsmDebugger( QObject* parent, OutPanelText* outPane, QString filePath )
              : BaseDebugger( parent,outPane, filePath )
{
    Q_UNUSED( AvrAsmDebugger_properties );
    
    setObjectName( "AVR asm Compiler/Debugger" );
    
    m_compilerPath = "";
    m_avraIncPath  = "";
    m_compSetting = "avra_Path";
    
    readSettings();
    
    QSettings* settings = MainWindow::self()->settings();
    
    if( settings->contains("avra_Inc_Path") )
        m_avraIncPath = settings->value("avra_Inc_Path").toString();
    
    if( m_avraIncPath == "" )
        m_avraIncPath = SIMUAPI_AppPath::self()->availableDataDirPath("codeeditor/tools/avra");

    m_typesList["byte"]    = "uint8";
}
AvrAsmDebugger::~AvrAsmDebugger() {}

int AvrAsmDebugger::compile()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    //getProcType();
    QString file = m_fileDir+m_fileName+m_fileExt;
    QString avraIncPath = m_avraIncPath;
    
    m_outPane->writeText( "-------------------------------------------------------\n" );
    
    QString listFile = m_fileDir+m_fileName+".lst";
    QString command  = m_compilerPath+"avra";
    
    QProcess checkComp( this );
    checkComp.start( command  );
    checkComp.waitForFinished(-1);
    
    QString p_stdo = checkComp.readAllStandardOutput();
    if( !p_stdo.toUpper().contains("VERSION") )
    {
        m_outPane->appendText( "\navra" );
        toolChainNotFound();
        return -1;
    }

    #ifndef Q_OS_UNIX
    command  = addQuotes( command );
    listFile = addQuotes( listFile );
    file     = addQuotes( file );
    avraIncPath = addQuotes( avraIncPath );
    #endif
    
    command.append(" -W NoRegDef");             // supress some warnings
    command.append(" -l "+ listFile );               // output list file
    if( m_avraIncPath != "" )
        command.append(" -I "+ avraIncPath);                // include dir
    command.append(" "+file );                       // File to assemble

    m_outPane->appendText( "Exec: ");
    m_outPane->appendText( command );
    m_outPane->writeText( "\n\n" );
    
    m_compProcess.start( command );
    m_compProcess.waitForFinished(-1);
    
    QString p_stderr = m_compProcess.readAllStandardError();
    m_outPane->appendText( p_stderr );
    m_outPane->writeText( "\n\n" );

    int error = 0;

    if( p_stderr.toUpper().contains("ERROR ") )
    {
        QStringList lines = p_stderr.split("\n");
        for( QString line : lines )
        {
            if( !(line.toUpper().contains( "ERROR " )) ) continue;
            QStringList words = line.split(":");
            QString filePath = m_fileDir+m_fileName+m_fileExt;
            error = words.first().remove(filePath).remove("(").remove(")").toInt();
            break;
        }
        if( error == 0 ) error = 1;
    }
    m_firmware = m_fileDir+m_fileName+".hex";
    
    QApplication::restoreOverrideCursor();
    return error;
}

void AvrAsmDebugger::mapFlashToSource()
{
    m_flashToSource.clear();
    m_sourceToFlash.clear();
    QString asmFileName = m_fileDir + m_fileName + ".asm";

    QString lstFileName = m_fileDir + m_fileName + ".lst";

    QStringList asmLines = fileToStringList( asmFileName, "AvrAsmDebugger::mapLstToAsm" );
    QStringList lstLines = fileToStringList( lstFileName, "AvrAsmDebugger::mapLstToAsm" );
    
    m_lastLine = 0;

    QString asmLine;
    int asmLineNumber = 0;
    int lastAsmLine = asmLines.size();

    for( QString lstLine : lstLines )
    {
        if( !lstLine.startsWith( "C:") ) continue;            // avra code lines start with C:

        lstLine = lstLine.toUpper().remove(0,2).replace("\t", " ").remove((" "));

        while( true )
        {
            if( ++asmLineNumber >= lastAsmLine ) break;                   // End of asm file
            asmLine = asmLines.at( asmLineNumber ).toUpper();
            asmLine = asmLine.replace("\t", " ").remove(" ");
            if( asmLine.isEmpty() ) continue;
            if( asmLine.startsWith(";")) continue;
            if( asmLine.startsWith("#")) continue;
            if( asmLine.startsWith(".")) continue;

            //qDebug() <<"AvrAsmDebugger::mapLstToAsm() "<<asmLine;
            //qDebug() <<"AvrAsmDebugger::mapLstToAsm() "<<lstLine;

            if( lstLine.contains(asmLine) ) break;                            // Line found
        }
        QString numberText = lstLine.left( 6 );    // first 6 digits in lst file is address
        bool ok = false;
        int address = numberText.toInt( &ok, 16 )*2;         // adress*2: instruc = 2 bytes
        if( ok )
        {
            m_flashToSource[address] = asmLineNumber;
            if( asmLineNumber > m_lastLine ) m_lastLine = asmLineNumber;
            //qDebug() <<"AvrAsmDebugger::mapLstToAsm() "<<address<<asmLineNumber;
        }
    }
    QHashIterator<int, int> i(m_flashToSource);
    while( i.hasNext() )
    {
        i.next();
        int address       = i.key();
        int asmLineNumber = i.value();
        m_sourceToFlash[asmLineNumber] = address;
        //qDebug() <<"AvrAsmDebugger::mapLstToAsm() "<<address<<asmLineNumber;
    }
}

QString AvrAsmDebugger::avraIncPath()
{
    return m_avraIncPath;
}

void AvrAsmDebugger::setAvraIncPath( QString path )
{
    m_avraIncPath = path;
    MainWindow::self()->settings()->setValue( "avra_Inc_Path", m_avraIncPath );
}

#include "moc_avrasmdebugger.cpp"
