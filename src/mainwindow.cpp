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

#include "mainwindow.h"

#include "appiface.h"
#include "circuit.h"
#include "circuitwidget.h"
#include "componentselector.h"
#include "editorwindow.h"
#include "filewidget.h"
#include "propertieswidget.h"
#include "simuapi_apppath.h"
#include "utils.h"

MainWindow *MainWindow::m_pSelf = 0l;

MainWindow::MainWindow() :
    QMainWindow(),
    m_settings(QStandardPaths::standardLocations(QStandardPaths::DataLocation)
                       .first() +
                   "/simulide.ini",
               QSettings::IniFormat, this)
{
    setWindowIcon(QIcon(":/simulide.png"));
    m_pSelf   = this;
    m_circuit = 0l;
    m_version = "SimulIDE-" + QString(APP_VERSION);

    this->setWindowTitle(m_version);

    QString userAddonPath =
        SIMUAPI_AppPath::self()->RWDataFolder().absoluteFilePath("addons");

    QDir pluginsDir(userAddonPath);

    if (!pluginsDir.exists())
        pluginsDir.mkpath(userAddonPath);

    m_fontScale = 1.0;
    if (m_settings.contains("fontScale")) {
        m_fontScale = m_settings.value("fontScale").toFloat();
        if (m_fontScale == 0)
            m_fontScale = 1;
    } else {
        double dpiX = qApp->desktop()->logicalDpiX();
        m_fontScale = dpiX / 96.0;
    }
    // qDebug()<<dpiX;
    loadCircHelp();
    createWidgets();
    readSettings();

    loadPlugins();

    QString backPath = m_settings.value("backupPath").toString();
    if (!backPath.isEmpty()) {
        // qDebug() << "MainWindow::readSettings" << backPath;
        if (QFile::exists(backPath))
            CircuitWidget::self()->loadCirc(backPath);
    }
}
MainWindow::~MainWindow() {}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_editor->close()) {
        event->ignore();
        return;
    }
    if (!m_circuit->newCircuit()) {
        event->ignore();
        return;
    }

    writeSettings();

    event->accept();
}

void MainWindow::readSettings()
{
    restoreGeometry(m_settings.value("geometry").toByteArray());
    restoreState(m_settings.value("windowState").toByteArray());
    m_Centralsplitter->restoreState(
        m_settings.value("Centralsplitter/geometry").toByteArray());

    int autoBck = 15;
    if (m_settings.contains("autoBck"))
        autoBck = m_settings.value("autoBck").toInt();
    Circuit::self()->setAutoBck(autoBck);
}

void MainWindow::writeSettings()
{
    m_settings.setValue("autoBck", m_autoBck);
    m_settings.setValue("fontScale", m_fontScale);
    m_settings.setValue("geometry", saveGeometry());
    m_settings.setValue("windowState", saveState());
    m_settings.setValue("Centralsplitter/geometry",
                        m_Centralsplitter->saveState());

    QList<QTreeWidgetItem *> list =
        m_components->findItems("", Qt::MatchStartsWith | Qt::MatchRecursive);

    for (QTreeWidgetItem *item : list)
        m_settings.setValue(item->text(0) + "/collapsed", !item->isExpanded());

    FileWidget::self()->writeSettings();
}

QString MainWindow::loc()
{
    return Circuit::self()->loc();
}

void MainWindow::setLoc(QString loc)
{
    Circuit::self()->setLoc(loc);
}

int MainWindow::autoBck()
{
    return m_autoBck;
}

void MainWindow::setAutoBck(int secs)
{
    m_autoBck = secs;
}

void MainWindow::setTitle(QString title)
{
    setWindowTitle(m_version + "  -  " + title);
}

void MainWindow::createWidgets()
{
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    QGridLayout *baseWidgetLayout = new QGridLayout(centralWidget);
    baseWidgetLayout->setSpacing(0);
    baseWidgetLayout->setContentsMargins(0, 0, 0, 0);
    baseWidgetLayout->setObjectName("gridLayout");

    m_Centralsplitter = new QSplitter(this);
    m_Centralsplitter->setObjectName("Centralsplitter");
    m_Centralsplitter->setOrientation(Qt::Horizontal);

    m_sidepanel = new QTabWidget(this);
    m_sidepanel->setObjectName("sidepanel");
    m_sidepanel->setTabPosition(QTabWidget::West);
    QString fontSize = QString::number(int(11 * m_fontScale));
    m_sidepanel->tabBar()->setStyleSheet("QTabBar { font-size:" + fontSize +
                                         "px; }");
    m_Centralsplitter->addWidget(m_sidepanel);

    m_components = new ComponentSelector(m_sidepanel);
    m_components->setObjectName("components");
    m_sidepanel->addTab(m_components, tr("Components"));

    m_ramTabWidget = new QWidget(this);
    m_ramTabWidget->setObjectName("ramTabWidget");
    m_ramTabWidgetLayout = new QGridLayout(m_ramTabWidget);
    m_ramTabWidgetLayout->setSpacing(0);
    m_ramTabWidgetLayout->setContentsMargins(0, 0, 0, 0);
    m_ramTabWidgetLayout->setObjectName("ramTabWidgetLayout");
    m_sidepanel->addTab(m_ramTabWidget, tr("RamTable"));

    m_itemprop = new PropertiesWidget(this);
    m_itemprop->setObjectName("properties");
    m_sidepanel->addTab(m_itemprop, tr("Properties"));

    m_fileSystemTree = new FileWidget(this);
    m_fileSystemTree->setObjectName("fileExplorer");
    m_sidepanel->addTab(m_fileSystemTree, tr("File explorer"));

    m_circuit = new CircuitWidget(this);
    m_circuit->setObjectName("circuit");
    m_Centralsplitter->addWidget(m_circuit);

    m_editor = new EditorWindow(this);
    m_editor->setObjectName(QString::fromUtf8("editor"));
    m_Centralsplitter->addWidget(m_editor);

    baseWidgetLayout->addWidget(m_Centralsplitter, 0, 0);

    QList<int> sizes;
    sizes << 150 << 350 << 500;
    m_Centralsplitter->setSizes(sizes);

    this->showMaximized();
}

void MainWindow::loadCircHelp()
{
    QString locale = "_" + QLocale::system().name().split("_").first();
    QString dfPath = SIMUAPI_AppPath::self()->availableDataFilePath(
        "help/" + locale + "/circuit" + locale + ".txt");

    if (dfPath == "")
        dfPath =
            SIMUAPI_AppPath::self()->availableDataFilePath("help/circuit.txt");

    if (dfPath != "") {
        QFile file(dfPath);

        if (file.open(QFile::ReadOnly | QFile::Text)) // Get Text from Help File
        {
            QTextStream s1(&file);
            s1.setCodec("UTF-8");

            m_circHelp = "";
            m_circHelp.append(s1.readAll());

            file.close();
        }
    }
}

QString *MainWindow::circHelp()
{
    return &m_circHelp;
}

void MainWindow::loadPlugins()
{
    // Load main Plugins
    QDir pluginsDir(qApp->applicationDirPath());

    pluginsDir.cd("../lib/simulide/plugins");

    loadPluginsAt(pluginsDir);

    // Load main Component Sets
    QDir compSetDir = SIMUAPI_AppPath::self()->RODataFolder();

    if (compSetDir.exists())
        ComponentSelector::self()->LoadCompSetAt(compSetDir);

    // Load Addons
    QString userPluginsPath =
        SIMUAPI_AppPath::self()->RWDataFolder().absoluteFilePath("addons");

    pluginsDir.setPath(userPluginsPath);

    if (!pluginsDir.exists())
        return;

    for (QString pluginFolder : pluginsDir.entryList(QDir::Dirs)) {
        if (pluginFolder.contains("."))
            continue;
        // qDebug() << pluginFolder;
        pluginsDir.cd(pluginFolder);

        ComponentSelector::self()->LoadCompSetAt(pluginsDir);

        if (pluginsDir.entryList(QDir::Dirs).contains("lib")) {
            pluginsDir.cd("lib");
            loadPluginsAt(pluginsDir);
            pluginsDir.cd("../");
        }
        pluginsDir.cd("../");
    }
}

void MainWindow::loadPluginsAt(QDir pluginsDir)
{
    QString pluginName = "*plugin";

#ifndef Q_OS_UNIX
    pluginName += ".dll";
#else
    pluginName += ".so";
#endif

    pluginsDir.setNameFilters(QStringList(pluginName));

    QStringList fileList = pluginsDir.entryList(QDir::Files);

    if (fileList.isEmpty())
        return; // No plugins to load

    qDebug() << "\n    Loading Plugins at:\n"
             << pluginsDir.absolutePath() << "\n";

    for (QString libName : fileList) {
        pluginName =
            libName.split(".").first().remove("lib").remove("plugin").toUpper();

        if (m_plugins.contains(pluginName))
            continue;

        QPluginLoader *pluginLoader =
            new QPluginLoader(pluginsDir.absoluteFilePath(libName));
        QObject *plugin = pluginLoader->instance();

        if (plugin) {
            AppIface *item = qobject_cast<AppIface *>(plugin);

            if (item) {
                item->initialize();
                m_plugins[pluginName] = pluginLoader;
                qDebug() << "        Plugin Loaded Successfully:\t"
                         << pluginName;
            } else {
                pluginLoader->unload();
                delete pluginLoader;
            }
        } else {
            QString errorMsg = pluginLoader->errorString();
            qDebug() << "        " << pluginName
                     << "\tplugin FAILED: " << errorMsg;

            if (errorMsg.contains("libQt5SerialPort"))
                errorMsg = tr(" Qt5SerialPort is not installed in your "
                              "system\n\n    Mcu SerialPort will not work\n    "
                              "Just Install libQt5SerialPort package\n    To "
                              "have Mcu Serial Port Working");

            QMessageBox::warning(0, tr("Plugin Error:"), errorMsg);
        }
    }
    qDebug() << "\n";
}

void MainWindow::unLoadPugin(QString pluginName)
{
    if (m_plugins.contains(pluginName)) {
        QPluginLoader *pluginLoader = m_plugins[pluginName];
        QObject *plugin             = pluginLoader->instance();
        AppIface *item              = qobject_cast<AppIface *>(plugin);
        item->terminate();
        pluginLoader->unload();
        m_plugins.remove(pluginName);
        delete pluginLoader;
    }
}

void MainWindow::applyStyle()
{
    QFile file(":/simulide.qss");
    file.open(QFile::ReadOnly);

    m_styleSheet = QLatin1String(file.readAll());

    qApp->setStyleSheet(m_styleSheet);
}

QSettings *MainWindow::settings()
{
    return &m_settings;
}

#include "moc_mainwindow.cpp"
