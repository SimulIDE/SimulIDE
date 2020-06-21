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

#include "gcbdebugger.h"
#include "baseprocessor.h"
#include "utils.h"
#include "simuapi_apppath.h"

GcbDebugger::GcbDebugger( QObject* parent, OutPanelText* outPane, QString filePath ) 
           : BaseDebugger( parent,outPane, filePath )
{
    setObjectName( "GcBasic Compiler/Debugger" );
    
    m_compilerPath = "";
    m_compSetting = "gcbasic_Path";
    type = 1;
    
    readSettings();

    m_typesList["byte"]    = "uint8";
    m_typesList["integer"] = "int16";
    m_typesList["word"]    = "uint16";
    m_typesList["long"]    = "uint32";
    m_typesList["string"]  = "string";
}
GcbDebugger::~GcbDebugger(){}

int GcbDebugger::compile()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    QDir gcBasicDir( m_compilerPath );
    if( !gcBasicDir.exists() )
    {
        m_outPane->appendText( "\nGcBasic" );
        toolChainNotFound();
        return -1;
    }

    QString file = m_fileDir+m_fileName+m_fileExt;
    QString args = " -NP -K:L -A:GCASM -R:text  ";
    QString command = m_compilerPath + "gcbasic";
    
    #ifndef Q_OS_UNIX
    command  = addQuotes( command );
    file     = addQuotes( file );
    #endif

    command.append( args + file );
    
    m_outPane->appendText( command );
    m_outPane->writeText( "\n\n" );

    QProcess compGcb( 0l );
    compGcb.setWorkingDirectory( m_fileDir );
    compGcb.start( command );
    compGcb.waitForFinished(-1);
    QString p_stdout = compGcb.readAllStandardOutput();

    m_outPane->writeText( p_stdout.remove("The message has been logged to the file Errors.txt.\n") );

    int error = -1;
    if( p_stdout=="" )
    {
        m_outPane->appendText( "\nGcBasic" );
        toolChainNotFound();
        error = -1;
    }
    else if( p_stdout.toUpper().contains("DONE")) 
    {
        m_firmware = m_fileDir+m_fileName+".hex";
        error = 0;
    }
    /*else // some error found
    {
        QStringList lines = p_stdout.split("\n");
        for( QString line : lines )
        {
            if( !line.contains( "Error:" ) ) continue;
            QStringList words = line.split(" ");
            words.removeFirst();
            error = words.first().remove("(").remove(")").remove(":").toInt();
            break;
        }
    }*/
    QApplication::restoreOverrideCursor();
    return error;
}

void GcbDebugger::getSubs()
{
    m_subs.clear();
    
    QStringList lines = fileToStringList( m_fileDir+m_fileName+".report.txt", "GcbDebugger::mapGcbToAsm" );

    while( true )
    {
        if( lines.isEmpty() ) break;
        QString line = lines.takeFirst();
        if( line.startsWith( "Subroutines" ) ) 
        {
            m_subs.append( lines.takeFirst() );
            break;
        }
    }
    while( true )
    {
        if( lines.isEmpty() ) break;
        QString line = lines.takeFirst();
        if( line.isEmpty() ) 
        {
            if( lines.isEmpty() ) break;
            line = lines.takeFirst();
            if( !line.startsWith( ";" ) && !line.isEmpty() ) m_subs.append( line.toUpper() );
        }
    }
    //qDebug() << "GcbDebugger::getSubs()\n" << m_subs;
}

void GcbDebugger::mapFlashToSource()
{
    getProcType(); // Determine Pic or Avr
    getSubs();
    mapLstToAsm();
    mapGcbToAsm();
    //qDebug() << "GcbDebugger::mapFlashToSource() Done";
}

void GcbDebugger::mapGcbToAsm()  // Map asm_source_line <=> gcb_source_line
{
    m_varList.clear();
    m_varNames.clear();
    m_subLines.clear();

    QStringList gcbLines = fileToStringList( m_fileDir+m_fileName+".gcb", "GcbDebugger::mapGcbToAsm" );
    
    bool isFunc = false;
    int lineNum = 0;
    for( QString gcbLine : gcbLines )              // Get Declared Variables
    {
        QStringList wordList = gcbLine.split( " " );
        wordList.removeAll( "" );
        
        QString line = gcbLine;
        line = line.toUpper();
        if     ( line.startsWith( "FUNCTION" ) )     isFunc = true;
        else if( line.startsWith( "END FUNCTION" ) ) isFunc = false;
        
        for( QString word : wordList )             // Find Subs Calls
        {
            if( isFunc ) break;
            
            QString wordUp = word;
            wordUp = wordUp.toUpper();
            
            if( wordUp == "IF" ) break;
            
            if( m_subs.contains( wordUp ) )
            {
                m_subLines.append( lineNum );
                break;
            }
        }
        lineNum++;
        
        if( !line.contains( "DIM" )) continue; // Search lines containing "Dim"
        
        line = line.replace( "'", ";" ).split( ";" ).first(); // Remove comments
        
        gcbLine = gcbLine.replace( "\t", " " );
        
        if( !line.contains( "AS" ))  // Should be an array
        {
            if( !line.contains( "(" )) continue;
            QStringList wordList = gcbLine.split( "(" );
            QString varName = wordList.takeFirst();
            QString varSize = wordList.takeFirst();
            
            wordList = varName.split( " " );
            wordList.removeAll( "" );
            varName = wordList.last();
            
            wordList = varSize.split( " " );
            wordList.removeAll( "" );
            varSize  = wordList.first().replace( ")", "" );
            int size = varSize.toInt();
            varSize  = QString::number( size+1 );
            
            QString type = "array"+varSize;
            if( !m_varList.contains( varName ) )
            {
                m_varList[ varName ] = type;
                m_varNames.append( varName );
            }
            
            //qDebug() << "GcbDebugger::mapGcbToAsm  Array "<<type<<varName;
        }
        else
        {
            //QStringList wordList = gcbLine.split( " " );
            //wordList.removeAll( "" );
            if( wordList.first().toUpper() != "DIM" ) continue;
            if( wordList.size() < 4 ) continue;
            
            QString type = wordList.at(3).toLower();
            if( m_typesList.contains( type ) )
            {
                QString varName = wordList.at(1).toLower();
                if( !m_varList.contains( varName ) )
                {
                    m_varList[ varName ] = m_typesList[ type ];
                    m_varNames.append( varName );
                }
                //qDebug() << "GcbDebugger::mapGcbToAsm  variable "<<type<<varName<<m_typesList[ type ];
            }
        }
    }//qDebug() << "GcbDebugger::mapGcbToAsm() SubLines\n" << m_subLines;
    m_flashToSource.clear();
    m_sourceToFlash.clear();
    
    QString asmFileName = m_fileDir + m_fileName + ".asm";

    QStringList asmLines = fileToStringList( asmFileName, "GcbDebugger::mapGcbToAsm" );

    bool haveVariable = false;
    int asmLineNumber = 0;
    m_lastLine = 0;
    QString srcm = ";Source:F1L";           // Gcbasic parses source lines to asm file

    for( QString asmLine : asmLines ) 
    {
        if( asmLine.contains(srcm))
        {
            asmLine.remove( srcm );
            int gcbLineNum = asmLine.split("S").first().toInt();
            
            if( gcbLineNum > m_lastLine ) m_lastLine = gcbLineNum;
            
            int asmLineNum = asmLineNumber+1;
            while( m_asmToFlash.value( asmLineNum ) == 0 ) asmLineNum++; // Avoid banksels and so
            
            int flashAddr = m_asmToFlash[asmLineNum];
            m_flashToSource[ flashAddr ]  = gcbLineNum;
            //qDebug()<<"GcbDebugger::mapGcbToAsm  gcb:" << gcbLineNum <<"  flashAddr:"<< flashAddr;
        }
        else if( asmLine.contains("locations for variables")) haveVariable = true;
        else if( asmLine.contains(";*"))                      haveVariable = false;
        else if( haveVariable & (asmLine != "") )
        {
            QStringList text;
            if( m_processorType == 1 )
            {
                asmLine.remove( "EQU").replace( "\t", " ");
                text = asmLine.split(" ");
            }
            else
            {
                asmLine.remove( ".EQU").remove("\t").remove(" ");
                text = asmLine.split("=");
            }
            QString name = text.first().toLower();
            int  address = text.last().toInt();
            QString type = "uint8";
            if( m_varList.contains( name ) ) type = m_varList[ name ];
            BaseProcessor::self()->addWatchVar( name, address ,type  );
            //qDebug()<<"GcbDebugger::mapGcbToAsm var:"<<asmLine<<name<< address<<type;
            
            if( type.contains( "array" ) )
            {
                m_varNames.removeOne( name );
                m_varNames.append( name );
                
                QString ty = type;
                int size = ty.replace( "array", "" ).toInt();
                for( int i=1; i<size; i++ )
                {
                    QString elmName = name+"("+QString::number( i )+")";
                    BaseProcessor::self()->addWatchVar( elmName, address+i ,"uint8"  );
                    if( !m_varList.contains( elmName ) ) 
                    {
                        m_varList[ elmName ] = m_typesList[ "uint8" ];
                        m_varNames.append( elmName );
                    }
                }
            }
        }
        asmLineNumber++;
    }
    QHashIterator<int, int> i(m_flashToSource);
    while( i.hasNext() )
    {
        i.next();
        int address    = i.key();
        int gcbLineNum = i.value();
        m_sourceToFlash[ gcbLineNum ] = address;
        //qDebug()<<"GcbDebugger::mapGcbToAsm  gcb:" << gcbLineNum <<"  flashAddr:"<< address;
    }
}

void GcbDebugger::mapLstToAsm()
{
    m_flashToAsm.clear();
    m_asmToFlash.clear();
    QString asmFileName = m_fileDir + m_fileName + ".asm";

    QString lstFileName = m_fileDir + m_fileName + ".lst";

    QStringList asmLines = fileToStringList( asmFileName, "GcbDebugger::mapLstToAsm" );
    QStringList lstLines = fileToStringList( lstFileName, "GcbDebugger::mapLstToAsm" );

    int asmLineNumber = 0;
    int lastAsmLine = asmLines.size();

    for( QString asmLine : asmLines ) // Go to the program start in asm file
    {
        if( asmLine.contains( "BASPROGRAMSTART" )) break;
        asmLineNumber++;
    }
    bool hasCeroAddr = false; // Deal with Banksel addr =0
    for( QString line : lstLines )
    {
        if( !line.startsWith("0") ) continue; // Code lines start with address
        //if( line.isEmpty() )      continue;
        line = line.replace("\t", " ").toUpper();
        line = line.remove(" ");
        line = line.split(";").first();

        while( true )
        {
            if( ++asmLineNumber >= lastAsmLine ) break; // End of asm file
            QString asmLine = asmLines.at( asmLineNumber ).toUpper();
            asmLine = asmLine.replace("\t", " ").remove(" ");
            if( asmLine.isEmpty() ) continue;
            if( asmLine.startsWith("_")) continue;
            if( asmLine.startsWith(";")) continue;
            if( asmLine.startsWith("#")) continue;
            if( asmLine.startsWith(".")) continue;

            asmLine = asmLine.split(";").first();
            //qDebug() << "GcbDebugger::mapLstToAsm" << line << asmLine;
            if( line.contains(asmLine) ) break;
        }
        if( asmLineNumber >= lastAsmLine )
        {
            asmLineNumber = 0;
            continue; // End of asm file
        }

        QString numberText = line.left( 6 ); // first 6 digits in lst file is address
        bool ok = false;
        int address = numberText.toInt( &ok, 16 )*m_processorType;  // AVR: adress*2: instruc = 2 bytes
        if( ok )
        {
            if( address==0 )                 // Deal with Banksel addr =0
            {
                if( hasCeroAddr ) continue;
                hasCeroAddr = true;
            }
            m_flashToAsm[address] = asmLineNumber;
           //qDebug() << "GcbDebugger::mapLstToAsm........." << address << asmLineNumber;
        }
    }
    QHashIterator<int, int> i(m_flashToAsm);
    while( i.hasNext() )
    {
        i.next();
        int address       = i.key();
        int asmLineNumber = i.value();
        m_asmToFlash[asmLineNumber] = address;
        //qDebug() << "GcbDebugger::mapLstToAsm........." << address << asmLineNumber;
    }
}

void GcbDebugger::getProcType()
{
   //qDebug() << "m_asmPath: " << m_asmPath;
    QStringList lines = fileToStringList( m_fileDir+m_fileName+".asm", "CodeEditor::getProcType" );

    for( QString line : lines )
    {
        line = line.toLower();
        if( line.contains("include") )  // Find info in included header
        {
            line.remove("include").remove("#").remove(" ").remove("<").remove(">");
            if(line.startsWith("p")) m_processorType = 1; // if mcu name starts with p then is Pic
            else                     m_processorType = 2; // Avr
            break;
        }
    }
    //qDebug() << "Processor Type: " << m_processorType;
}
#include "moc_gcbdebugger.cpp"
