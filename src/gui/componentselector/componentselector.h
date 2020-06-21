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

#ifndef COMPONENTSELECTOR_H
#define COMPONENTSELECTOR_H

#include <QtWidgets>

#include "compplugindialog.h"
#include "itemlibrary.h"

class MAINMODULE_EXPORT ComponentSelector : public QTreeWidget
{
    Q_OBJECT
    
    public:
        ComponentSelector( QWidget* parent );
        ~ComponentSelector();

 static ComponentSelector* self() { return m_pSelf; }

        void addLibraryItem( LibraryItem* libItem );
        //void removeLibItem( LibraryItem* libItem );

        void LoadCompSetAt( QDir compSetDir );
        void loadXml( const QString &setFile );

        QString getXmlFile( QString compName );

        void mouseReleaseEvent(QMouseEvent*);

    private slots:
        void slotItemClicked( QTreeWidgetItem* item, int column );
        void slotContextMenu(const QPoint&);
        void slotManageComponents();

    private:
 static ComponentSelector* m_pSelf;

        void addItem( LibraryItem* libItem );
        void addItem( const QString &caption,
                      const QString &category,
                      const QString &icon,
                      const QString &type );

        void LoadLibraryItems();
        //void LoadCompSet();
        //void istallPlugin( QString item );
        //void unistallPlugin( QString item );

        //void reLoadItems();

        QStringList m_compSetUnique;
        QStringList m_categories;
        //QString m_lastItemClicked;
        QHash<QString, QString> m_xmlFileList;

        CompPluginDialog m_pluginsdDialog;

        ItemLibrary m_itemLibrary;
};

#endif
