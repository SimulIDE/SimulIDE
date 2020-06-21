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

#include "basedebugger.h"
#include "baseprocessor.h"
#include "editorwindow.h"
#include "mainwindow.h"
#include "simulator.h"

static const char* BaseDebugger_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Drive Circuit"),
    QT_TRANSLATE_NOOP("App::Property","Compiler Path")
};

bool BaseDebugger::m_loadStatus = false;

BaseDebugger::BaseDebugger( QObject* parent, OutPanelText* outPane, QString filePath ) 
            : QObject( parent )
            , m_compProcess( 0l )
{
    Q_UNUSED( BaseDebugger_properties );
    
    m_outPane = outPane;
    m_appPath   = QCoreApplication::applicationDirPath();
    
    m_fileDir  = filePath;
    m_fileName = filePath.split("/").last();
    m_fileDir.remove( m_fileDir.lastIndexOf( m_fileName ), m_fileName.size() );
    m_fileExt  = "."+m_fileName.split(".").last();
    m_fileName = m_fileName.remove( m_fileName.lastIndexOf( m_fileExt ), m_fileExt.size() );

    m_processorType = 0;
    type = 0;
    
    connect( &m_compProcess, SIGNAL(readyRead()), SLOT(ProcRead()) );
}
BaseDebugger::~BaseDebugger( )
{
    if( BaseProcessor::self() ) BaseProcessor::self()->getRamTable()->remDebugger( this );
}

bool BaseDebugger::loadFirmware()
{
    if ( m_firmware == "" )  return false;
    
    upload();
    if( m_loadStatus ) return false;
    
    m_loadStatus = true;

    return true;
}

void BaseDebugger::upload()
{
    if( m_loadStatus )
    {
        QMessageBox::warning( 0, "BaseDebugger::loadFirmware",
                                tr("Debugger already running")+"\n"+tr("Stop active session") );
        return;
    }
    m_outPane->writeText( "-------------------------------------------------------\n" );
    m_outPane->appendText( "\n"+tr("Uploading: ")+"\n" );
    m_outPane->appendText( m_firmware );
    m_outPane->writeText( "\n\n" );
    
    if( McuComponent::self() ) 
    {
        McuComponent::self()->load( m_firmware );
        m_outPane->appendText( "\n"+tr("FirmWare Uploaded to ")+McuComponent::self()->device()+"\n" );
        m_outPane->writeText( "\n\n" );

        BaseProcessor::self()->getRamTable()->setDebugger( this );
        mapFlashToSource();
    }
    else m_outPane->writeText( "\n"+tr("Error: No Mcu in Simulator... ")+"\n" );
}

void BaseDebugger::stop()
{
    m_loadStatus = false;
}

void BaseDebugger::getProcName()
{
}

int BaseDebugger::step()
{
    int pc = BaseProcessor::self()->pc();
    
    int i = 0;
    for( i=0; i<10; i++ ) // If runs 10 times and get to same PC return 0
    {
        BaseProcessor::self()->stepOne();

        int pc2 = BaseProcessor::self()->pc();
        //qDebug() <<"BaseDebugger::step "<<pc<<pc2;
        if( pc != pc2 ) 
        {
            pc = pc2;
            break;
        }
    }
    //qDebug() <<"BaseDebugger::step PC"<<pc<<i;
    int line = -1;
    if( i == 10 ) line = 0;        // It ran 10 times and get to same PC 
    else if( m_flashToSource.contains( pc )) line = m_flashToSource[ pc ];

    return line ;
}

int BaseDebugger::stepOver(){return 0;}

int BaseDebugger::getValidLine( int line )
{
    while( !m_sourceToFlash.contains(line) && line<=m_lastLine ) line++;
    return line;
}

QString BaseDebugger::getVarType( QString var )
{
    var= var.toUpper();
    return m_varList[ var ];
}

QStringList BaseDebugger::getVarList()
{
    /*QStringList varList = m_varList.keys();
    varList.sort();
    return varList;*/
    return m_varNames;
}

void BaseDebugger::ProcRead()
{
    while( m_compProcess.canReadLine() ) 
    {
        m_outPane->appendText( QString::fromLocal8Bit( m_compProcess.readLine()) );
        m_outPane->writeText( "\n" );
    }
}

void BaseDebugger::readSettings()
{
    QSettings* settings = MainWindow::self()->settings();
    
    if( settings->contains( m_compSetting ) )
        m_compilerPath = settings->value( m_compSetting ).toString();
}

void BaseDebugger::getCompilerPath()
{
        m_compilerPath = QFileDialog::getExistingDirectory( 0L,
                               tr("Select Compiler toolchain directory"),
                               m_compilerPath,
                               QFileDialog::ShowDirsOnly
                             | QFileDialog::DontResolveSymlinks);

        if( m_compilerPath != "" ) m_compilerPath += "/";

        MainWindow::self()->settings()->setValue( m_compSetting, m_compilerPath);

        m_outPane->appendText( "\n"+tr("Using Compiler Path: ")+"\n" );
        m_outPane->writeText( m_compilerPath+"\n\n" );
}

bool BaseDebugger::driveCirc()
{
    CodeEditor* ce = EditorWindow::self()->getCodeEditor();
    return ce->driveCirc();
}

void BaseDebugger::setDriveCirc( bool drive )
{
    CodeEditor* ce = EditorWindow::self()->getCodeEditor();
    ce->setDriveCirc( drive );
}

QString BaseDebugger::compilerPath()
{
    return m_compilerPath;
}

void BaseDebugger::setCompilerPath( QString path )
{
    m_compilerPath = path;
    MainWindow::self()->settings()->setValue( m_compSetting, m_compilerPath );
}

void BaseDebugger::toolChainNotFound()
{
    m_outPane->appendText( tr(": ToolChain not found")+"\n" );
    m_outPane->writeText( "\n"+tr("Right-Click on Document Tab to set Path")+"\n\n" );
    QApplication::restoreOverrideCursor();
}
#include "moc_basedebugger.cpp"

