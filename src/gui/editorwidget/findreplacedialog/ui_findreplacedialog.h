/*
 * Copyright (C) 2009  Lorenzo Bettini <http://www.lorenzobettini.it>
 * See COPYING file that comes with this distribution
 */
 
/***************************************************************************
 *   Modified 2012 by santiago González                                    *
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

#ifndef UI_FINDREPLACEDIALOG_H
#define UI_FINDREPLACEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <findreplaceform.h>

QT_BEGIN_NAMESPACE

class Ui_FindReplaceDialog
{
public:
    QGridLayout *gridLayout;
    FindReplaceForm *findReplaceForm;

    void setupUi(QDialog *FindReplaceDialog)
    {
        if (FindReplaceDialog->objectName().isEmpty())
            FindReplaceDialog->setObjectName(QStringLiteral("FindReplaceDialog"));
        FindReplaceDialog->resize(342, 140);
        gridLayout = new QGridLayout(FindReplaceDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        findReplaceForm = new FindReplaceForm(FindReplaceDialog);
        findReplaceForm->setObjectName(QStringLiteral("findReplaceForm"));

        gridLayout->addWidget(findReplaceForm, 0, 0, 1, 1);


        retranslateUi(FindReplaceDialog);

        QMetaObject::connectSlotsByName(FindReplaceDialog);
    } // setupUi

    void retranslateUi(QDialog *FindReplaceDialog)
    {
        FindReplaceDialog->setWindowTitle(QApplication::translate("FindReplaceDialog", "Find/Replace", 0));
    } // retranslateUi

};

namespace Ui {
    class FindReplaceDIALOG: public Ui_FindReplaceDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FINDREPLACEDIALOG_H
