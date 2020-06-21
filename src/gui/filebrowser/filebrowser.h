/***************************************************************************
 *   Copyright (C) 2018 by Pavel Lamonov                                   *
 *   leamonpaul@yandex.ru                                                  *
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

#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <QtWidgets>

class MAINMODULE_EXPORT FileBrowser : public QTreeView
{
    Q_OBJECT

    public:
        FileBrowser( QWidget *parent );
        ~FileBrowser();
        
 static FileBrowser* self() { return m_pSelf; }
 
        void setPath(QString path);
        
    public slots:
        void cdUp();
        void open();
        void openInEditor();
        void addBookMark();
        void showHidden();
        
    protected:
        void contextMenuEvent( QContextMenuEvent* event );

    private:
    
 static FileBrowser*  m_pSelf;
 
        void mouseDoubleClickEvent(QMouseEvent *event);
        void keyPressEvent( QKeyEvent *event );
        
        QFileSystemModel* m_fileSystemModel;
        
        QString m_currentPath;
        
        bool m_showHidden;
};

#endif
