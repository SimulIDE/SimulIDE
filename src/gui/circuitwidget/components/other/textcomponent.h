#ifndef TEXTCOMPONENT_H
#define TEXTCOMPONENT_H


#include "component.h"
#include "itemlibrary.h"

class MAINMODULE_EXPORT TextComponent : public Component
{
    Q_OBJECT
    Q_PROPERTY( QString  Text  READ getText    WRITE setText )
    Q_PROPERTY( QString  Font  READ getFont    WRITE setFont     DESIGNABLE true USER true )
    Q_PROPERTY( int  Font_Size   READ fontSize WRITE setFontSize DESIGNABLE true USER true )
    Q_PROPERTY( bool Fixed_Width READ fixedW   WRITE setFixedW   DESIGNABLE true USER true )
    Q_PROPERTY( int  Margin      READ margin   WRITE setMargin   DESIGNABLE true USER true )
    Q_PROPERTY( int  Border      READ border   WRITE setBorder   DESIGNABLE true USER true )
    Q_PROPERTY( qreal Opacity    READ opac     WRITE setOpac     DESIGNABLE true USER true )
    
    public:
        TextComponent( QObject* parent, QString type, QString id );
        ~TextComponent();
        
        QRectF boundingRect() const 
        { 
            return QRectF( m_area.x()-m_border/2-1, m_area.y()-m_border/2-1, 
                           m_area.width()+m_border+2, m_area.height()+m_border+2 ); 
        }

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem *libraryItem();
 
        int  margin();
        void setMargin( int margin );
        
        int  border();
        void setBorder( int border );
        
        int  fontSize();
        void setFontSize( int size );
        
        bool fixedW();
        void setFixedW( bool fixedW );

        QString getText();
        void    setText( QString text );
        
        QString getFont();
        void    setFont( QString font );

        qreal opac();
        void setOpac( qreal op );

        bool eventFilter( QObject* object, QEvent* event );

        void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    signals:

    public slots:
        void updateGeometry(int, int, int);

    protected:
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event );

    private:
        QGraphicsTextItem* m_text;

        qreal m_opac;
        
        int  m_fontSize;
        int  m_margin;
        int  m_border;

        bool m_fixedW;
        
        QString m_font;
};

#endif // TEXTCOMPONENT_H
