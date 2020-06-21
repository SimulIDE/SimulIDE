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

//#include <QtGui>

#include "editorwindow.h"
#include "mainwindow.h"
#include "filebrowser.h"
#include "utils.h"

EditorWindow*  EditorWindow::m_pSelf = 0l;

EditorWindow::EditorWindow( QWidget* parent )
            : QWidget( parent )
{
    m_pSelf = this;
    
    createWidgets();
    createActions();
    createToolBars();
    readSettings();
}
EditorWindow::~EditorWindow(){}

bool EditorWindow::close()
{    
    writeSettings();

    for( int i=0; i<m_docWidget->count(); i++ )
    {
        closeTab( m_docWidget->currentIndex() );
    }
    return maybeSave();
}

void EditorWindow::keyPressEvent( QKeyEvent* event )
{
    if( event->key() == Qt::Key_N && (event->modifiers() & Qt::ControlModifier))
    {
        newFile();
    }
    else if( event->key() == Qt::Key_S && (event->modifiers() & Qt::ControlModifier))
    {
        if( event->modifiers() & Qt::ShiftModifier) saveAs();
        else                                        save();
    }
    else if( event->key() == Qt::Key_O && (event->modifiers() & Qt::ControlModifier))
    {
        open();
    }
}

void EditorWindow::newFile()
{
    CodeEditorWidget* baseWidget = new CodeEditorWidget( this );
    
    m_docWidget->addTab( baseWidget, "New" );
    m_docWidget->setCurrentWidget( baseWidget );
    
    connect( baseWidget->m_codeEditor->document(), SIGNAL( contentsChanged()),
             this,                                 SLOT(   documentWasModified()));
            
    m_fileList << "New";
    enableFileActs( true ); 
    enableDebugActs( true );
}

void EditorWindow::open()
{
    const QString dir = m_lastDir;
    QString fileName = QFileDialog::getOpenFileName( this, tr("Load File"), dir,
                       tr("All files")+" (*);;Arduino (*.ino);;Asm (*.asm);;GcBasic (*.gcb)" );

    if( !fileName.isEmpty() ) loadFile( fileName );
}

void EditorWindow::loadFile( const QString &fileName )
{
    if( m_fileList.contains( fileName ) )
    {
        m_docWidget->setCurrentIndex( m_fileList.indexOf( fileName ) );
        //return;
    }
    else newFile();
    QApplication::setOverrideCursor( Qt::WaitCursor );
    
    CodeEditor* ce = getCodeEditor();
    ce->setPlainText( fileToString( fileName, "EditorWindow" ) );
    ce->setFile( fileName );
    
    m_lastDir = fileName;
    int index = m_docWidget->currentIndex();
    m_fileList.replace( index, fileName );
    m_docWidget->setTabText( index, strippedName(fileName) );
    enableFileActs( true );   // enable file actions
    //if( ce->hasDebugger() )
        enableDebugActs( true );
    QApplication::restoreOverrideCursor();
}

void EditorWindow::reload()
{
    QString fileName = m_fileList.at( m_docWidget->currentIndex() );
    loadFile( fileName );
}

bool EditorWindow::save()
{
    QString file = getCodeEditor()->getFilePath();
    if( file.isEmpty() ) return saveAs();
    else                 return saveFile(file);
}

bool EditorWindow::saveAs()
{
    CodeEditor* ce = getCodeEditor();

    QFileInfo fi = QFileInfo( ce->getFilePath() );
    QString ext  = fi.suffix();
    QString path = fi.absolutePath();
    if( path == "" ) path = m_lastDir;
    //qDebug() << "EditorWindow::saveAs" << path;

    QString extensions = "";
    if( ext == "" ) extensions = tr("All files")+" (*);;Arduino (*.ino);;Asm (*.asm);;GcBasic (*.gcb)";
    else            extensions = "."+ext+"(*."+ext+");;"+tr("All files")+" (*.*)";

    QString fileName = QFileDialog::getSaveFileName( this, tr("Save Document As"), path, extensions );
    if( fileName.isEmpty() ) return false;

    m_fileList.replace( m_docWidget->currentIndex(), fileName );

    return saveFile( fileName );
}

bool EditorWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if( !file.open(QFile::WriteOnly | QFile::Text) )
    {
        QMessageBox::warning(this, "EditorWindow::saveFile",
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    CodeEditor* ce = getCodeEditor();
    out << ce->toPlainText();
    ce->setFile( fileName );
    QApplication::restoreOverrideCursor();
    
    ce->document()->setModified(false);
    documentWasModified();

    m_docWidget->setTabText( m_docWidget->currentIndex(), strippedName(fileName) );
    return true;
}

bool EditorWindow::maybeSave()
{
    if( m_fileList.isEmpty() ) return true;
    if( getCodeEditor()->document()->isModified() )
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning( this, "EditorWindow::saveFile",
              tr("\nThe Document has been modified.\nDo you want to save your changes?\n"),
              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
              
        if     ( ret == QMessageBox::Save )   return save();
        else if( ret == QMessageBox::Cancel ) return false;
    }
    return true;
}

void EditorWindow::documentWasModified()
{
    CodeEditor* ce = getCodeEditor();
    QTextDocument *doc = ce->document();

    bool    modified = doc->isModified();
    int     index    = m_docWidget->currentIndex();
    QString tabText  = m_docWidget->tabText( index );

    if     ( modified && !tabText.endsWith("*") ) tabText.append("*");
    else if( !modified && tabText.endsWith("*") ) tabText.remove("*");

    m_docWidget->setTabText( index, tabText );

    redoAct->setEnabled( false );
    undoAct->setEnabled( false );
    if( doc->isRedoAvailable() ) redoAct->setEnabled( true );
    if( doc->isUndoAvailable() ) undoAct->setEnabled( true );
    
    ce->setCompiled( false );
}

void EditorWindow::enableFileActs( bool enable )
{
    saveAct->setEnabled( enable );
    saveAsAct->setEnabled( enable );
    cutAct->setEnabled( enable );
    copyAct->setEnabled( enable );
    pasteAct->setEnabled( enable );
    undoAct->setEnabled( enable );
    redoAct->setEnabled( enable );
    findQtAct->setEnabled( enable );
}

void EditorWindow::enableDebugActs( bool enable )
{
    debugAct->setEnabled( enable );
    runAct->setEnabled( enable );
    stepAct->setEnabled( enable );
    stepOverAct->setEnabled( enable );
    pauseAct->setEnabled( enable );
    resetAct->setEnabled( enable );
    stopAct->setEnabled( enable );
    compileAct->setEnabled( enable );
    loadAct->setEnabled( enable );
}

void EditorWindow::tabContextMenu( const QPoint &eventpoint )
{
    CodeEditor* ce = getCodeEditor();
    if( !ce ) return;
    
    QMenu* menu = new QMenu();
    QAction* setCompilerAction = menu->addAction(QIcon(":/copy.png"),tr("Set Compiler Path"));
    connect( setCompilerAction, SIGNAL( triggered()), this, SLOT(setCompiler()) );

    QAction* reloadAction = menu->addAction(QIcon(":/reload.png"),tr("Reload"));
    connect( reloadAction, SIGNAL( triggered()), this, SLOT(reload()) );

    menu->exec( mapToGlobal(eventpoint) );
    menu->deleteLater();
}

void EditorWindow::tabChanged( int tab )
{
    qDebug() << "EditorWindow::tabChanged" << m_docWidget->currentIndex() << tab;
}

void EditorWindow::setCompiler()
{
    CodeEditor* ce = getCodeEditor();
    if( ce ) ce->setCompilerPath();
}

void EditorWindow::createWidgets()
{
    baseWidgetLayout = new QGridLayout( this );
    baseWidgetLayout->setSpacing(0);
    baseWidgetLayout->setContentsMargins(0, 0, 0, 0);
    baseWidgetLayout->setObjectName("gridLayout");
    
    m_editorToolBar = new QToolBar( this );
    baseWidgetLayout->addWidget( m_editorToolBar );
    
    m_debuggerToolBar = new QToolBar( this );
    m_debuggerToolBar->setVisible( false );
    baseWidgetLayout->addWidget( m_debuggerToolBar );
    
    m_docWidget = new QTabWidget( this );
    m_docWidget->setObjectName("docWidget");
    m_docWidget->setTabPosition( QTabWidget::North );
    m_docWidget->setTabsClosable ( true );
    m_docWidget->setContextMenuPolicy( Qt::CustomContextMenu );
    double fontScale = MainWindow::self()->fontScale();
    QString fontSize = QString::number( int(10*fontScale) );
    m_docWidget->tabBar()->setStyleSheet("QTabBar { font-size:"+fontSize+"px; }");
    //m_docWidget->setMovable( true );
    baseWidgetLayout->addWidget( m_docWidget );
    
    connect( m_docWidget, SIGNAL( tabCloseRequested(int)), 
             this,        SLOT(   closeTab(int)));
             
    connect( m_docWidget, SIGNAL( customContextMenuRequested(const QPoint &)), 
             this,        SLOT(   tabContextMenu(const QPoint &)));
                        
    connect( m_docWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    
    setLayout( baseWidgetLayout );
    
    findRepDiaWidget = new FindReplaceDialog(this);
    findRepDiaWidget->setModal(false);
}

void EditorWindow::createActions()
{
    newAct = new QAction(QIcon(":/new.png"), tr("&New\tCtrl+N"), this);
    newAct->setStatusTip(tr("Create a new file"));
    connect( newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/open.png"), tr("&Open...\tCtrl+O"), this);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/save.png"), tr("&Save\tCtrl+S"), this);
    saveAct->setStatusTip(tr("Save the document to disk"));
    saveAct->setEnabled(false);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(QIcon(":/saveas.png"),tr("Save &As...\tCtrl+Shift+S"), this);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    saveAsAct->setEnabled(false);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(QIcon(":/exit.png"),tr("E&xit"), this);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    cutAct = new QAction(QIcon(":/cut.png"), tr("Cu&t\tCtrl+X"), this);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    cutAct->setEnabled(false);
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

    copyAct = new QAction(QIcon(":/copy.png"), tr("&Copy\tCtrl+C"), this);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    copyAct->setEnabled(false);
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/paste.png"), tr("&Paste\tCtrl+V"), this);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    pasteAct->setEnabled(false);
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

    undoAct = new QAction(QIcon(":/undo.png"), tr("Undo\tCtrl+Z"), this);
    undoAct->setStatusTip(tr("Undo the last action"));
    undoAct->setEnabled(false);
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

    redoAct = new QAction(QIcon(":/redo.png"), tr("Redo\tCtrl+Shift+Z"), this);
    redoAct->setStatusTip(tr("Redo the last action"));
    redoAct->setEnabled(false);
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

    runAct =  new QAction(QIcon(":/runtobk.png"),tr("Run To Breakpoint"), this);
    runAct->setStatusTip(tr("Run to next breakpoint"));
    runAct->setEnabled(false);
    connect(runAct, SIGNAL(triggered()), this, SLOT(run()));

    stepAct = new QAction(QIcon(":/step.png"),tr("Step"), this);
    stepAct->setStatusTip(tr("Step debugger"));
    stepAct->setEnabled(false);
    connect( stepAct, SIGNAL(triggered()), this, SLOT(step()) );

    stepOverAct = new QAction(QIcon(":/rotateCW.png"),tr("StepOver"), this);
    stepOverAct->setStatusTip(tr("Step Over"));
    stepOverAct->setEnabled(false);
    stepOverAct->setVisible(false);
    connect( stepOverAct, SIGNAL(triggered()), this, SLOT(stepOver()) );

    pauseAct = new QAction(QIcon(":/pause.png"),tr("Pause"), this);
    pauseAct->setStatusTip(tr("Pause debugger"));
    pauseAct->setEnabled(false);
    connect( pauseAct, SIGNAL(triggered()), this, SLOT(pause()) );

    resetAct = new QAction(QIcon(":/reset.png"),tr("Reset"), this);
    resetAct->setStatusTip(tr("Reset debugger"));
    resetAct->setEnabled(false);
    connect( resetAct, SIGNAL(triggered()), this, SLOT(reset()) );

    stopAct = new QAction(QIcon(":/stop.png"),tr("Stop Debugger"), this);
    stopAct->setStatusTip(tr("Stop debugger"));
    stopAct->setEnabled(false);
    connect( stopAct, SIGNAL(triggered()), this, SLOT(stop()) );

    compileAct = new QAction(QIcon(":/compile.png"),tr("Compile"), this);
    compileAct->setStatusTip(tr("Compile Source"));
    compileAct->setEnabled(false);
    connect( compileAct, SIGNAL(triggered()), this, SLOT(compile()) );
    
    loadAct = new QAction(QIcon(":/load.png"),tr("UpLoad"), this);
    loadAct->setStatusTip(tr("Load Firmware"));
    loadAct->setEnabled(false);
    connect( loadAct, SIGNAL(triggered()), this, SLOT(upload()) );

    /*aboutAct = new QAction(QIcon(":/info.png"),tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));*/

    /*aboutQtAct = new QAction(QIcon(":/info.png"),tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));*/

    //connect(m_codeEditor, SIGNAL(copyAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
    //connect(m_codeEditor, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));
    
    findQtAct = new QAction(QIcon(":/find.png"),tr("Find Replace"), this);
    findQtAct->setStatusTip(tr("Find Replace"));
    findQtAct->setEnabled(false);
    connect(findQtAct, SIGNAL(triggered()), this, SLOT(findReplaceDialog()));
    
    debugAct =  new QAction(QIcon(":/play.png"),tr("Debug"), this);
    debugAct->setStatusTip(tr("Start Debugger"));
    debugAct->setEnabled(false);
    connect(debugAct, SIGNAL(triggered()), this, SLOT(debug()));
}

void EditorWindow::enableStepOver( bool en )
{
    stepOverAct->setVisible( en );
}

CodeEditor* EditorWindow::getCodeEditor()
{
    CodeEditorWidget* actW = dynamic_cast<CodeEditorWidget*>(m_docWidget->currentWidget());
    if( actW )return actW->m_codeEditor;
    else      return 0l;
}

void EditorWindow::closeTab( int index )
{
    m_docWidget->setCurrentIndex( index );
    if( !maybeSave() ) return;

    m_fileList.removeAt(index);

    if( m_fileList.isEmpty() )
    {
        enableFileActs( false ); // disable file actions
        enableDebugActs( false );
    }
    if( m_debuggerToolBar->isVisible() ) stop();

    CodeEditorWidget* actW = dynamic_cast<CodeEditorWidget*>( m_docWidget->widget(index));
    m_docWidget->removeTab( index );
    delete actW;

    int last = m_docWidget->count()-1;
    if( index > last ) m_docWidget->setCurrentIndex( last );
    else               m_docWidget->setCurrentIndex( index );
}

void EditorWindow::cut()     { getCodeEditor()->cut(); }
void EditorWindow::copy()    { getCodeEditor()->copy(); }
void EditorWindow::paste()   { getCodeEditor()->paste(); }
void EditorWindow::undo()    { getCodeEditor()->undo(); }
void EditorWindow::redo()    { getCodeEditor()->redo(); }

void EditorWindow::debug()    
{ 
    CodeEditor* ce = getCodeEditor();

    if( ce->initDebbuger() )
    {
        m_editorToolBar->setVisible( false);
        m_debuggerToolBar->setVisible( true );

        runAct->setEnabled( true );
        stepAct->setEnabled( true );
        stepOverAct->setEnabled( true );
        resetAct->setEnabled( true );
        pauseAct->setEnabled( false );
    }
}

void EditorWindow::run()
{ 
    setStepActs();
    QTimer::singleShot( 10, getCodeEditor(), SLOT( run()) );
}

void EditorWindow::step()    
{ 
    setStepActs();
    QTimer::singleShot( 10, getCodeEditor(), SLOT( step()) );
    //getCodeEditor()->step( false ); 
}

void EditorWindow::stepOver()
{
    setStepActs();
    QTimer::singleShot( 10, getCodeEditor(), SLOT( stepOver()) );
    //getCodeEditor()->step( true ); 
}

void EditorWindow::setStepActs()
{ 
    runAct->setEnabled( false );
    stepAct->setEnabled( false );
    stepOverAct->setEnabled( false );
    resetAct->setEnabled( false );
    pauseAct->setEnabled( true );
}

void EditorWindow::pause()   
{
    getCodeEditor()->pause();
    runAct->setEnabled( true );
    stepAct->setEnabled( true );
    stepOverAct->setEnabled( true );
    resetAct->setEnabled( true );
    pauseAct->setEnabled( false );
}
void EditorWindow::reset()   
{ 
    getCodeEditor()->reset();
}

void EditorWindow::stop()    
{ 
    getCodeEditor()->stopDebbuger();
    m_debuggerToolBar->setVisible( false );
    m_editorToolBar->setVisible( true);
}

void EditorWindow::compile() 
{ 
    getCodeEditor()->compile();
}

void EditorWindow::upload()  
{ 
    getCodeEditor()->upload();
}

void EditorWindow::findReplaceDialog() 
{
    CodeEditor* ce = getCodeEditor();
    
    findRepDiaWidget->setTextEdit( ce );
    
    QString text =ce->textCursor().selectedText();
    if( text != "" ) findRepDiaWidget->setTextToFind( text );

    findRepDiaWidget->show(); 
}

void EditorWindow::createToolBars()
{
    m_editorToolBar->addAction(newAct);
    m_editorToolBar->addAction(openAct);
    m_editorToolBar->addAction(saveAct);
    m_editorToolBar->addAction(saveAsAct);
    m_editorToolBar->addSeparator();

    /*m_editorToolBar->addAction(cutAct);
    m_editorToolBar->addAction(copyAct);
    m_editorToolBar->addAction(pasteAct);
    m_editorToolBar->addSeparator();
    m_editorToolBar->addAction(undoAct);
    m_editorToolBar->addAction(redoAct);*/
    m_editorToolBar->addAction(findQtAct);
    m_editorToolBar->addSeparator();
    
    m_editorToolBar->addAction(compileAct);
    m_editorToolBar->addAction(loadAct);
    m_editorToolBar->addSeparator();
    
    m_editorToolBar->addAction(debugAct);
    
    m_debuggerToolBar->addAction(stepAct);
    m_debuggerToolBar->addAction(stepOverAct);
    m_debuggerToolBar->addAction(runAct);
    m_debuggerToolBar->addAction(pauseAct);
    m_debuggerToolBar->addAction(resetAct);
    m_debuggerToolBar->addSeparator();
    m_debuggerToolBar->addAction(stopAct);
}

void EditorWindow::readSettings()
{
    QSettings* settings = MainWindow::self()->settings();
    restoreGeometry( settings->value("geometry").toByteArray() );
    m_docWidget->restoreGeometry( settings->value("docWidget/geometry").toByteArray() );
    m_lastDir = settings->value("lastDir").toString();
}

void EditorWindow::writeSettings()
{
    QSettings* settings = MainWindow::self()->settings();
    settings->setValue( "geometry", saveGeometry() );
    settings->setValue( "docWidget/geometry", m_docWidget->saveGeometry() );
    settings->setValue( "lastDir", m_lastDir );
}

QString EditorWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void EditorWindow::about()
{
   /*QMessageBox::about(this, tr("About Application"),
            tr(""));*/
            ;
}
#include  "moc_editorwindow.cpp"
