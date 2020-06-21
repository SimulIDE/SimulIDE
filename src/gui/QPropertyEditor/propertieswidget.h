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

#ifndef PROPERTIESWIDGET_H
#define PROPERTIESWIDGET_H

#include <QPlainTextEdit>

#include "QPropertyEditorWidget.h"


class PropertiesWidget : public QWidget
{
    Q_OBJECT

    public:
        PropertiesWidget( QWidget *parent );
        ~PropertiesWidget();

 static PropertiesWidget* self() { return m_pSelf; }
 
        void setHelpText( QString* text );
 
    private:
 static PropertiesWidget* m_pSelf;
 
        void createWidgets();

        QPropertyEditorWidget* m_properties;
        QPlainTextEdit*        m_help;
};

#endif // PROPERTIESWIDGET_H
