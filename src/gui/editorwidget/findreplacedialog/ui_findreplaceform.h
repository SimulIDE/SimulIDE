/*
 * Copyright ( C ) 2009  Lorenzo Bettini <http://www.lorenzobettini.it>
 * See COPYING file that comes with this distribution
 */
 
/***************************************************************************
 *   Modified 2012 by santiago González                                    *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   ( at your option ) any later version.                                   *
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

#ifndef UI_FINDREPLACEFORM_H
#define UI_FINDREPLACEFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FindReplaceForm
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout_5;
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout_3;
    QLabel *label;
    QLineEdit *textToFind;
    QLabel *replaceLabel;
    QLineEdit *textToReplace;
    QLabel *errorLabel;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_3;
    QRadioButton *downRadioButton;
    QRadioButton *upRadioButton;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_4;
    QCheckBox *caseCheckBox;
    QCheckBox *wholeCheckBox;
    QCheckBox *regexCheckBox;
    QVBoxLayout *verticalLayout;
    QPushButton *findButton;
    QPushButton *closeButton;
    QPushButton *replaceButton;
    QPushButton *replaceAllButton;
    QSpacerItem *verticalSpacer;

    void setupUi( QWidget *FindReplaceForm )
    {
        if ( FindReplaceForm->objectName(  ).isEmpty(  ) )
            FindReplaceForm->setObjectName( QStringLiteral( "FindReplaceForm" ) );
        FindReplaceForm->resize( 483, 288 );
        gridLayout = new QGridLayout( FindReplaceForm );
        gridLayout->setObjectName( QStringLiteral( "gridLayout" ) );
        verticalLayout_5 = new QVBoxLayout(  );
        verticalLayout_5->setObjectName( QStringLiteral( "verticalLayout_5" ) );
        verticalLayout_2 = new QVBoxLayout(  );
        verticalLayout_2->setObjectName( QStringLiteral( "verticalLayout_2" ) );
        gridLayout_3 = new QGridLayout(  );
        gridLayout_3->setObjectName( QStringLiteral( "gridLayout_3" ) );
        label = new QLabel( FindReplaceForm );
        label->setObjectName( QStringLiteral( "label" ) );

        gridLayout_3->addWidget( label, 0, 0, 1, 1 );

        textToFind = new QLineEdit( FindReplaceForm );
        textToFind->setObjectName( QStringLiteral( "textToFind" ) );

        gridLayout_3->addWidget( textToFind, 0, 1, 1, 1 );

        replaceLabel = new QLabel( FindReplaceForm );
        replaceLabel->setObjectName( QStringLiteral( "replaceLabel" ) );

        gridLayout_3->addWidget( replaceLabel, 1, 0, 1, 1 );

        textToReplace = new QLineEdit( FindReplaceForm );
        textToReplace->setObjectName( QStringLiteral( "textToReplace" ) );

        gridLayout_3->addWidget( textToReplace, 1, 1, 1, 1 );


        verticalLayout_2->addLayout( gridLayout_3 );


        verticalLayout_5->addLayout( verticalLayout_2 );

        errorLabel = new QLabel( FindReplaceForm );
        errorLabel->setObjectName( QStringLiteral( "errorLabel" ) );

        verticalLayout_5->addWidget( errorLabel );

        horizontalLayout = new QHBoxLayout(  );
        horizontalLayout->setObjectName( QStringLiteral( "horizontalLayout" ) );
        groupBox = new QGroupBox( FindReplaceForm );
        groupBox->setObjectName( QStringLiteral( "groupBox" ) );
        verticalLayout_3 = new QVBoxLayout( groupBox );
        verticalLayout_3->setObjectName( QStringLiteral( "verticalLayout_3" ) );
        downRadioButton = new QRadioButton( groupBox );
        downRadioButton->setObjectName( QStringLiteral( "downRadioButton" ) );
        downRadioButton->setChecked( true );

        verticalLayout_3->addWidget( downRadioButton );

        upRadioButton = new QRadioButton( groupBox );
        upRadioButton->setObjectName( QStringLiteral( "upRadioButton" ) );

        verticalLayout_3->addWidget( upRadioButton );


        horizontalLayout->addWidget( groupBox );

        groupBox_2 = new QGroupBox( FindReplaceForm );
        groupBox_2->setObjectName( QStringLiteral( "groupBox_2" ) );
        verticalLayout_4 = new QVBoxLayout( groupBox_2 );
        verticalLayout_4->setObjectName( QStringLiteral( "verticalLayout_4" ) );
        caseCheckBox = new QCheckBox( groupBox_2 );
        caseCheckBox->setObjectName( QStringLiteral( "caseCheckBox" ) );

        verticalLayout_4->addWidget( caseCheckBox );

        wholeCheckBox = new QCheckBox( groupBox_2 );
        wholeCheckBox->setObjectName( QStringLiteral( "wholeCheckBox" ) );

        verticalLayout_4->addWidget( wholeCheckBox );

        regexCheckBox = new QCheckBox( groupBox_2 );
        regexCheckBox->setObjectName( QStringLiteral( "regexCheckBox" ) );

        verticalLayout_4->addWidget( regexCheckBox );


        horizontalLayout->addWidget( groupBox_2 );


        verticalLayout_5->addLayout( horizontalLayout );


        gridLayout->addLayout( verticalLayout_5, 0, 0, 1, 1 );

        verticalLayout = new QVBoxLayout(  );
        verticalLayout->setObjectName( QStringLiteral( "verticalLayout" ) );
        verticalLayout->setContentsMargins( 10, -1, -1, -1 );
        findButton = new QPushButton( FindReplaceForm );
        findButton->setObjectName( QStringLiteral( "findButton" ) );
        findButton->setEnabled( false );

        verticalLayout->addWidget( findButton );

        closeButton = new QPushButton( FindReplaceForm );
        closeButton->setObjectName( QStringLiteral( "closeButton" ) );

        verticalLayout->addWidget( closeButton );

        replaceButton = new QPushButton( FindReplaceForm );
        replaceButton->setObjectName( QStringLiteral( "replaceButton" ) );
        replaceButton->setEnabled( false );

        verticalLayout->addWidget( replaceButton );

        replaceAllButton = new QPushButton( FindReplaceForm );
        replaceAllButton->setObjectName( QStringLiteral( "replaceAllButton" ) );
        replaceAllButton->setEnabled( false );

        verticalLayout->addWidget( replaceAllButton );

        verticalSpacer = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );

        verticalLayout->addItem( verticalSpacer );


        gridLayout->addLayout( verticalLayout, 0, 1, 1, 1 );

#ifndef QT_NO_SHORTCUT
        label->setBuddy( textToFind );
        replaceLabel->setBuddy( textToReplace );
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder( textToFind, textToReplace );
        QWidget::setTabOrder( textToReplace, downRadioButton );
        QWidget::setTabOrder( downRadioButton, upRadioButton );
        QWidget::setTabOrder( upRadioButton, caseCheckBox );
        QWidget::setTabOrder( caseCheckBox, wholeCheckBox );
        QWidget::setTabOrder( wholeCheckBox, regexCheckBox );
        QWidget::setTabOrder( regexCheckBox, findButton );
        QWidget::setTabOrder( findButton, closeButton );

        retranslateUi( FindReplaceForm );

        QMetaObject::connectSlotsByName( FindReplaceForm );
    } // setupUi

    void retranslateUi( QWidget *FindReplaceForm )
    {
        FindReplaceForm->setWindowTitle( QApplication::translate( "FindReplaceForm", "Find/Replace", 0 ) );
        label->setText( QApplication::translate( "FindReplaceForm", "&Find:", 0 ) );
        replaceLabel->setText( QApplication::translate( "FindReplaceForm", "R&eplace with:", 0 ) );
        errorLabel->setText( QApplication::translate( "FindReplaceForm", "Error", 0 ) );
        groupBox->setTitle( QApplication::translate( "FindReplaceForm", "D&irection", 0 ) );
        downRadioButton->setText( QApplication::translate( "FindReplaceForm", "&Down", 0 ) );
        upRadioButton->setText( QApplication::translate( "FindReplaceForm", "&Up", 0 ) );
        groupBox_2->setTitle( QApplication::translate( "FindReplaceForm", "&Options", 0 ) );
        caseCheckBox->setText( QApplication::translate( "FindReplaceForm", "&Case sensitive", 0 ) );
        wholeCheckBox->setText( QApplication::translate( "FindReplaceForm", "&Whole words only", 0 ) );
        regexCheckBox->setText( QApplication::translate( "FindReplaceForm", "R&egular Expression", 0 ) );
        findButton->setText( QApplication::translate( "FindReplaceForm", "&Find", 0 ) );
        closeButton->setText( QApplication::translate( "FindReplaceForm", "&Close", 0 ) );
        replaceButton->setText( QApplication::translate( "FindReplaceForm", "&Replace", 0 ) );
        replaceAllButton->setText( QApplication::translate( "FindReplaceForm", "Replace &All", 0 ) );
    } // retranslateUi

};

namespace Ui {
    class FindReplaceFORM: public Ui_FindReplaceForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FINDREPLACEFORM_H
