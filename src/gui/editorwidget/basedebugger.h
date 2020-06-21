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

#ifndef BASEDEBUGGER_H
#define BASEDEBUGGER_H

#include <QtGui>
#include "outpaneltext.h"
#include "mcucomponent.h"

class BaseDebugger : public QObject    // Base Class for all debuggers
{
    Q_OBJECT
    Q_PROPERTY( bool    Drive_Circuit READ driveCirc    WRITE setDriveCirc    DESIGNABLE true USER true )
    Q_PROPERTY( QString Compiler_Path READ compilerPath WRITE setCompilerPath DESIGNABLE true USER true )
    
    public:
        BaseDebugger( QObject* parent=0, OutPanelText* outPane=0, QString filePath=0 );
        ~BaseDebugger();
        
        bool driveCirc();
        void setDriveCirc( bool drive );

        QString compilerPath();
        virtual void setCompilerPath( QString path );
        
        virtual bool loadFirmware();
        virtual void upload();
        virtual int  step();        // Run 1 step,returns current source line number
        virtual int  stepOver();    // Run until next source file
        virtual void stop();

        virtual void getProcName();
        virtual void setProcType( int type ){ m_processorType = type; }
        
        virtual int compile()=0;
        virtual void mapFlashToSource()=0;
        
        virtual int getValidLine( int line );   

        virtual void getCompilerPath();
        
        virtual void readSettings();
        
        virtual QString getVarType( QString var );
        
        virtual QStringList getVarList();
        
        virtual QList<int> getSubLines() { return m_subLines; }
        
        int type;
        
    public slots:
        void ProcRead();

    protected:
        void toolChainNotFound();
        virtual void getSubs(){;}
    
        OutPanelText*  m_outPane;

 static bool m_loadStatus;                          // Is symbol file loaded?
        bool m_running;                             // Is processor running?

        int m_processorType;
        int m_lastLine;
        
        QString m_device;
        QString m_appPath;
        QString m_firmware;
        QString m_fileDir;
        QString m_fileName;
        QString m_fileExt;
        QString m_compilerPath;
        QString m_compSetting;
        
        QStringList m_varNames;
        QStringList m_subs;
        QList<int>  m_subLines;
        
        QHash<QString, QString> m_typesList;
        QHash<QString, QString> m_varList;
        QHash<int, int> m_flashToSource;            // Map flash adress to Source code line
        QHash<int, int> m_sourceToFlash;            // Map .asm code line to flash adress
        
        QProcess m_compProcess;
};

#endif

