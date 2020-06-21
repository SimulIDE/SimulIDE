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

#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

//#include <QDialog>
#include <QtWidgets>

#include "findreplace_global.h"

namespace Ui {
    class FindReplaceDIALOG;
}

class QTextEdit;
class QSettings;

/**
  * A find/replace dialog.
  *
  * It relies on a FindReplaceForm object (see that class for the functionalities provided).
  */
class FindReplaceDialog : public QDialog {
    Q_OBJECT
public:
    FindReplaceDialog(QWidget *parent = 0);
    virtual ~FindReplaceDialog();

    /**
      * Associates the text editor where to perform the search
      * @param textEdit
      */
    void setTextEdit(CodeEditor *textEdit);

    /**
      * Writes the state of the form to the passed settings.
      * @param settings
      * @param prefix the prefix to insert in the settings
      */
    virtual void writeSettings(QSettings &settings, const QString &prefix = "FindReplaceDialog");

    /**
      * Reads the state of the form from the passed settings.
      * @param settings
      * @param prefix the prefix to look for in the settings
      */
    virtual void readSettings(QSettings &settings, const QString &prefix = "FindReplaceDialog");
    
    void setTextToFind( QString text);

public slots:
    /**
     * Finds the next occurrence
     */
    void findNext();

    /**
     * Finds the previous occurrence
     */
    void findPrev();

protected:
    void changeEvent(QEvent *e);

    Ui::FindReplaceDIALOG *ui;
};

#endif // FINDREPLACEDIALOG_H
