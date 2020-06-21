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

#include "findreplacedialog.h"
#include "ui_findreplacedialog.h"

FindReplaceDialog::FindReplaceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindReplaceDIALOG)
{
    ui->setupUi(this);
}

FindReplaceDialog::~FindReplaceDialog()
{
    delete ui;
}

void FindReplaceDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        //ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void FindReplaceDialog::setTextEdit(CodeEditor *textEdit) {
    ui->findReplaceForm->setTextEdit(textEdit);
}

void FindReplaceDialog::writeSettings(QSettings &settings, const QString &prefix) {
    ui->findReplaceForm->writeSettings(settings, prefix);
}

void FindReplaceDialog::readSettings(QSettings &settings, const QString &prefix) {
    ui->findReplaceForm->readSettings(settings, prefix);
}

void FindReplaceDialog::findNext() {
    ui->findReplaceForm->findNext();
}

void FindReplaceDialog::findPrev() {
    ui->findReplaceForm->findPrev();
}

void FindReplaceDialog::setTextToFind( QString text)
{
    ui->findReplaceForm->setTextToFind( text );
}

#include  "moc_findreplacedialog.cpp"

