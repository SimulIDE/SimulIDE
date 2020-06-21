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


#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
//#include <QHash>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

    public:
        Highlighter(QTextDocument *parent = 0);
        ~Highlighter();

        void readSintaxFile( const QString &fileName );

        void addRegisters( QStringList patterns );
        
        void setMultiline( bool set );

    protected:
        void highlightBlock(const QString &text);

    private:
        struct HighlightingRule
        {
            QRegExp pattern;
            QTextCharFormat format;
        };
        
        //void addRuleSet( QTextCharFormat, QString );
        void addRule( QTextCharFormat, QString );
        void processRule( HighlightingRule rule, QString lcText );

        bool m_multiline;
        
        QVector<HighlightingRule> m_highlightingRules;

        QRegExp commentStartExpression;
        QRegExp commentEndExpression;

        QTextCharFormat keyword1Format;
        QTextCharFormat registerFormat;
        //QTextCharFormat classFormat;
        QTextCharFormat lineCommentFormat;
        QTextCharFormat preprocessorFormat;
        QTextCharFormat dataTypeFormat;
        QTextCharFormat numberFormat;
        QTextCharFormat multiLineCommentFormat;
        QTextCharFormat quotationFormat;
        QTextCharFormat htmlTagFormat;
        QTextCharFormat functionFormat;
};

#endif
