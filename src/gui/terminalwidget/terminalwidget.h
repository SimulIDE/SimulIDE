/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QtWidgets>

#include "outpaneltext.h"

class SerialTerm;

class MAINMODULE_EXPORT TerminalWidget : public QWidget
{
    Q_OBJECT

    public:
        TerminalWidget( QWidget* parent=0, SerialTerm* ser=0 );
        ~TerminalWidget();

        void step();

        int uart() { return m_uart+1; }
        void setUart( int uart );

    public slots:
        void uartChanged( int uart );

    protected:
        virtual void closeEvent( QCloseEvent* event );

    private slots:
        void onTextChanged();
        void onValueChanged();
        void valueButtonClicked();
        void ascciButtonClicked();
        void addCRClicked();
        void clearInClicked();
        void clearOutClicked();

        void uartIn( int uart, int value );
        void uartOut( int uart, int value );

    private:
        QVBoxLayout   m_verticalLayout;
        QHBoxLayout   m_sendLayout;
        QHBoxLayout   m_textLayout;
        QLineEdit     m_sendText;
        QLineEdit     m_sendValue;
        OutPanelText  m_uartInPanel;
        OutPanelText  m_uartOutPanel;
        QPushButton   m_ascciButton;
        QPushButton   m_valueButton;
        QPushButton   m_addCrButton;
        QPushButton   m_clearInButton;
        QPushButton   m_clearOutButton;
        QSpinBox      m_uartBox;

        SerialTerm* m_serComp;

        bool m_printASCII;
        bool m_addCR;

        int m_uart;
};

#endif
