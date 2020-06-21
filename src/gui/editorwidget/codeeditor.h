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

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>

#include "highlighter.h"
#include "outpaneltext.h"
#include "ramtable.h"

#define DBG_STOPPED 0
#define DBG_STEPING 1
#define DBG_RUNNING 2
#define DBG_PAUSED  3

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class BaseDebugger;
class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
    //Q_PROPERTY( bool   centerOnScroll   READ centerOnScroll    WRITE setCenterOnScroll  DESIGNABLE true USER true )
    Q_PROPERTY( int  Font_Size     READ fontSize   WRITE setFontSize   DESIGNABLE true USER true )
    Q_PROPERTY( int  Tab_Size      READ tabSize    WRITE setTabSize    DESIGNABLE true USER true )
    Q_PROPERTY( bool Spaces_Tabs   READ spaceTabs  WRITE setSpaceTabs  DESIGNABLE true USER true )
    Q_PROPERTY( bool Show_Spaces   READ showSpaces WRITE setShowSpaces DESIGNABLE true USER true )

    public:
        CodeEditor( QWidget *parent, OutPanelText *outPane );
        ~CodeEditor();
        
        int fontSize();
        void setFontSize( int size );
        
        int tabSize();
        void setTabSize( int size );
        
        bool showSpaces();
        void setShowSpaces( bool on );
        
        bool spaceTabs();
        void setSpaceTabs( bool on );
        
        bool driveCirc();
        void setDriveCirc( bool drive );

        void setFile(const QString &filePath);
        QString getFilePath();

        void lineNumberAreaPaintEvent(QPaintEvent *event);
        int  lineNumberAreaWidth();
        
        void setCompiled( bool compiled ) { m_isCompiled = compiled; }
        
        bool debugStarted() { return m_debugging; }
        bool initDebbuger();
        bool hasDebugger() { return m_debugger!=0l; }

        void setCompilerPath();

    signals:
        void msg( QString text );

    public slots:
        void stopDebbuger();
        void slotAddBreak() { m_brkAction = 1; }
        void slotRemBreak() { m_brkAction = 2; }
        void timerTick();
        void compile();
        void upload();
        void step( bool over=false );
        void stepOver();
        void pause();
        void resume();
        void reset();
        void run();

    protected:
        void resizeEvent(QResizeEvent *event);
        void focusInEvent( QFocusEvent* );
        void keyPressEvent( QKeyEvent* event );

    private slots:
        void updateLineNumberAreaWidth(int newBlockCount);
        void updateLineNumberArea( const QRect &, int );
        void highlightCurrentLine();
        void runClockTick();

    private:
        int  getSintaxCoincidences(QString& fileName, QStringList& instructions );
        void addBreakPoint( int line );
        void remBreakPoint( int line );
        void updateScreen();

        void setupDebugTimer();
        
        void indentSelection( bool unIndent );
        
        BaseDebugger* m_debugger;
        OutPanelText* m_outPane;

        LineNumberArea *m_lNumArea;
        Highlighter    *m_hlighter;

        //QString m_appPath;
        QString m_file;
        QString m_fileDir;
        QString m_fileName;
        QString m_fileExt;
        
        QString m_tab;

        QList<int> m_brkPoints;

        int m_brkAction;    // 0 = no action, 1 = add brkpoint, 2 = rem brkpoint
        int m_debugLine;
        int m_prevDebugLine;
        int m_state;
        int m_resume;

        bool m_isCompiled;
        bool m_debugging;
        //bool m_running;

        bool m_stepOver;
        
 static bool  m_showSpaces;
 static bool  m_spaceTabs;
 static bool  m_driveCirc;
 
 static int   m_fontSize;
 static int   m_tabSize;
 static QFont m_font;
 
 static QStringList m_picInstr;
 static QStringList m_avrInstr;
};


// ********************* CLASS LineNumberArea **********************************

class LineNumberArea : public QWidget
{
    Q_OBJECT
    
    public:
        LineNumberArea(CodeEditor *editor);
        ~LineNumberArea();

        QSize sizeHint() const { return QSize(m_codeEditor->lineNumberAreaWidth(), 0); }

        int lastPos;

    protected:
        void contextMenuEvent( QContextMenuEvent *event);
        void paintEvent(QPaintEvent *event) { m_codeEditor->lineNumberAreaPaintEvent(event); }

    private:
        CodeEditor *m_codeEditor;
};

#endif
