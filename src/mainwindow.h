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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

class PropertiesWidget;
class ComponentSelector;
class CircuitWidget;
class EditorWindow;
class FileWidget;

class MAINMODULE_EXPORT MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow();
    ~MainWindow();

    static MainWindow *self()
    {
        return m_pSelf;
    }

    QSettings *settings();

    void loadPlugins();
    void unLoadPugin(QString pluginName);

    void readSettings();

    void setTitle(QString title);

    double fontScale()
    {
        return m_fontScale;
    }
    void setFontScale(double scale)
    {
        m_fontScale = scale;
    }

    int autoBck();
    void setAutoBck(int secs);

    QString loc();
    void setLoc(QString loc);

    QString *circHelp();
    void loadCircHelp();

    QTabWidget *m_sidepanel;
    QWidget *m_ramTabWidget;
    QGridLayout *m_ramTabWidgetLayout;

  protected:
    void closeEvent(QCloseEvent *event);

  private:
    static MainWindow *m_pSelf;

    void loadPluginsAt(QDir pluginsDir);

    bool m_blocked;

    void createWidgets();
    void createMenus();
    void createToolBars();
    void writeSettings();
    void applyStyle();

    float m_fontScale;
    int m_autoBck;

    QSettings m_settings;

    QString m_version;
    QString m_styleSheet;
    QString m_circHelp;

    QHash<QString, QPluginLoader *> m_plugins;

    CircuitWidget *m_circuit;
    ComponentSelector *m_components;
    PropertiesWidget *m_itemprop;
    EditorWindow *m_editor;

    QSplitter *m_Centralsplitter;
    FileWidget *m_fileSystemTree;
};

#endif
