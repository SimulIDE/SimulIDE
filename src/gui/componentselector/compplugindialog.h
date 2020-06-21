/***************************************************************************
 *   Copyright (C) 2017 by santiago González                               *
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
 
#ifndef COMPPLUGINDIALOG_H
#define COMPPLUGINDIALOG_H

#include <QDialog>
#include <QtWidgets/QListWidget>
#include <QTreeWidgetItem>

namespace Ui {
class ComponentPlugins;
}

class CompPluginDialog : public QDialog
{
    Q_OBJECT

    public:
        CompPluginDialog(QWidget *parent);
        ~CompPluginDialog();

        void setPluginList();

    private slots:
        void slotItemChanged( QListWidgetItem* item );
        
    private:
        Ui::ComponentPlugins* ui;

        void reject();

        void addItem( QTreeWidgetItem* item );

        void setItemVisible(QListWidgetItem* listItem, bool visible );


        QHash<QListWidgetItem*, QTreeWidgetItem*>  m_qtwItem;

        QListWidget* m_compList;

        bool m_initialized;
};
#endif
