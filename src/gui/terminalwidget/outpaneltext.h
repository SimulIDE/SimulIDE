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

#ifndef OUTPANELTEXT_H
#define OUTPANELTEXT_H

#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QObject>

class OutHighlighter;

class MAINMODULE_EXPORT OutPanelText : public QPlainTextEdit
{
    Q_OBJECT
    public:
        OutPanelText( QWidget* parent=0 );
        ~OutPanelText();

        void appendText( const QString text );
        void writeText( const QString text );
        
        void step();

    private:
 
        QString m_text;
 
        OutHighlighter* m_highlighter;
};

// CLASS OutHighlighter ****************************************

class QSyntaxHighlighter;

class OutHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

    public:
        OutHighlighter( QTextDocument *parent = 0 );
        ~OutHighlighter();

    protected:
        void highlightBlock( const QString &text );

    private:
        struct HighlightingRule
        {
            QRegExp pattern;
            QTextCharFormat format;
        };
        QVector<HighlightingRule> highlightingRules;

        QTextCharFormat errorFormat;
        QTextCharFormat fileFormat;
        QTextCharFormat successFormat;
        QTextCharFormat sentFormat;
        QTextCharFormat warningFormat;
        QTextCharFormat numberFormat;
        QTextCharFormat quotationFormat;
};

#endif
