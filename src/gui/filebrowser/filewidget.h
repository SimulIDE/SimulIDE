/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#ifndef FILEWIDGET_H
#define FILEWIDGET_H

#include "filebrowser.h"

class MAINMODULE_EXPORT FileWidget : public QWidget
{
    Q_OBJECT
    
    public:
        FileWidget( QWidget* parent );
        ~FileWidget();

 static FileWidget* self() { return m_pSelf; }

        void addBookMark( QString path );
        
        void setPath( QString path );
        
        void writeSettings();

    private slots:
        void itemClicked( QListWidgetItem* );
        void remBookMark();
        void pathChanged();

    protected:
        void contextMenuEvent( QContextMenuEvent* event );

    private:
 static FileWidget* m_pSelf;

        void resizeToItems();
        void addEntry( QString name, QString path );

        QStringList m_bookmarkList;
        
        FileBrowser* m_fileBrowser;
        QListWidget* m_bookmarks;
        QPushButton* m_cdUpButton;
        QLineEdit*   m_path;
        
        bool m_blocked;
};

#endif
