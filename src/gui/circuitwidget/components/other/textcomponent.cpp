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

#include "textcomponent.h"
#include "circuit.h"

static const char* TextComponent_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Text"),
    QT_TRANSLATE_NOOP("App::Property","Font"),
    QT_TRANSLATE_NOOP("App::Property","Font Size"),
    QT_TRANSLATE_NOOP("App::Property","Fixed Width"),
    QT_TRANSLATE_NOOP("App::Property","Margin"),
    QT_TRANSLATE_NOOP("App::Property","Border"),
    QT_TRANSLATE_NOOP("App::Property","Opacity")
};

Component* TextComponent::construct( QObject* parent, QString type, QString id )
{
    return new TextComponent( parent, type, id );
}

LibraryItem* TextComponent::libraryItem()
{
    return new LibraryItem(
        tr( "Text" ),
        tr( "Graphical" ),
        "text.png",
        "TextComponent",
    TextComponent::construct );
}

TextComponent::TextComponent( QObject* parent, QString type, QString id )
             : Component( parent, type, id )
{
    Q_UNUSED( TextComponent_properties );

    m_opac = 1;
    
    m_color = QColor( 255, 255, 220 );
    m_font  = "Helvetica [Cronyx]";

    QFont sansFont( m_font, 10 );
    #if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    sansFont.setWeight( QFont::Medium );
    #else
    sansFont.setWeight( QFont::Normal);
    #endif
    sansFont.setFixedPitch(true);

    m_text = new QGraphicsTextItem( this );
    //m_text->setTextInteractionFlags( Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::TextEditable);
    m_text->setTextInteractionFlags( 0 );
    m_text->setTextWidth( 90 );
    m_text->setFont( sansFont );
    m_text->setPlainText("... TEXT ...");
    m_text->setPos( 0, 0 );
    m_text->document()->setTextWidth(-1);
    m_text->setDefaultTextColor( Qt::darkBlue );
    m_text->setCursor( Qt::OpenHandCursor );

    m_margin = 5;
    m_border = 1;
    updateGeometry( 0, 0, 0 );
    
    setFontSize( 10 );

    connect(m_text->document(), SIGNAL( contentsChange(int, int, int )),
                          this, SLOT( updateGeometry(int, int, int )));
}
TextComponent::~TextComponent(){}

void TextComponent::updateGeometry(int, int, int)
{
    m_text->document()->setTextWidth(-1);
    
    int margin = m_margin;
    
    m_area = QRect( -margin, -margin, m_text->boundingRect().width()+margin*2, m_text->boundingRect().height()+margin*2 );
    
    Circuit::self()->update();
}

int TextComponent::margin() { return m_margin; }

void TextComponent::setMargin( int margin )
{
    if( margin < 2 ) margin = 2;
    m_margin = margin;
    updateGeometry( 0, 0, 0 );
}

bool TextComponent::fixedW() { return m_fixedW; }

void TextComponent::setFixedW( bool fixedW ) 
{ 
    m_fixedW = fixedW;
    
    QFont font = m_text->font();
    font.setFixedPitch( fixedW );
    m_text->setFont( font );
    updateGeometry( 0, 0, 0 );
}

QString TextComponent::getFont()
{
    return m_font;
}

void TextComponent::setFont( QString font )
{
    if( font == "" ) return;
    m_font = font;
    
    QFont Tfont = m_text->font();
    Tfont.setFamily( font );
    m_text->setFont( Tfont );
    updateGeometry( 0, 0, 0 );
}

int TextComponent::fontSize()
{
    return m_fontSize;
}

void TextComponent::setFontSize( int size )
{
    if( size < 1 ) return;
    m_fontSize = size;
    
    QFont font = m_text->font();
    font.setPixelSize( size );
    m_text->setFont( font );
    updateGeometry( 0, 0, 0 );
}

int  TextComponent::border() { return m_border; }
void TextComponent::setBorder( int border ) { m_border = border; }

QString TextComponent::getText() { return m_text->toPlainText(); }
void    TextComponent::setText( QString text ) { m_text->document()->setPlainText( text ); }

qreal TextComponent::opac()
{
    return m_opac;
}

void TextComponent::setOpac( qreal op )
{
    m_opac = op;
    update();
}

void TextComponent::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        m_text->setTextInteractionFlags( Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::TextEditable);
        m_text->setCursor( Qt::IBeamCursor );
        m_text->setFocus();
        setSelected( false );
    }
}

bool TextComponent::eventFilter( QObject* object, QEvent* event )
{
    if( event->type() == QEvent::FocusIn )
    {
        if( object == m_text)
        {
            Circuit::self()->deselectAll();
        }
    }
    return false;
}

void TextComponent::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    if( !m_text->hasFocus() )
    {
        m_text->setTextInteractionFlags( 0 );
        m_text->setCursor( Qt::OpenHandCursor );
    }
    p->setOpacity( m_opac );

    Component::paint( p, option, widget );

    QPen pen( QColor( 0, 0, 0 ), m_border, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen( pen );

    if( m_border >0 ) p->drawRect( m_area );
    else              p->fillRect( m_area, p->brush() );

    p->setOpacity( 1 );
}

#include "moc_textcomponent.cpp"
