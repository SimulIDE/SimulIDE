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


#ifndef COMPONENTITEM_H
#define COMPONENTITEM_H

#include <QtWidgets>
#include <QPointer>

#include "QPropertyEditorWidget.h"

Q_DECLARE_METATYPE( QList<int> )

class Pin;
class Label;

class MAINMODULE_EXPORT Component : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

    Q_PROPERTY( QString  itemtype  READ itemType  USER true )
    Q_PROPERTY( QString  id        READ idLabel   WRITE setIdLabel DESIGNABLE true USER true )
    Q_PROPERTY( bool     Show_id   READ showId    WRITE setShowId  DESIGNABLE true USER true )
    Q_PROPERTY( qreal    rotation  READ rotation  WRITE setRotation )
    Q_PROPERTY( int      x         READ x         WRITE setX )
    Q_PROPERTY( int      y         READ y         WRITE setY )
    Q_PROPERTY( int      labelx    READ labelx    WRITE setLabelX )
    Q_PROPERTY( int      labely    READ labely    WRITE setLabelY )
    Q_PROPERTY( int      labelrot  READ labelRot  WRITE setLabelRot )
    Q_PROPERTY( int      valLabelx READ valLabelx WRITE setValLabelX )
    Q_PROPERTY( int      valLabely READ valLabely WRITE setValLabelY )
    Q_PROPERTY( int      valLabRot READ valLabRot WRITE setValLabRot )
    Q_PROPERTY( int      hflip     READ hflip     WRITE setHflip )
    Q_PROPERTY( int      vflip     READ vflip     WRITE setVflip )

    public:
        QRectF boundingRect() const { return QRectF( m_area.x()-2, m_area.y()-2, m_area.width()+4 ,m_area.height()+4 ); }

        Component( QObject* parent, QString type, QString id );
        ~Component();

        enum { Type = UserType + 1 };
        int type() const { return Type; }

        QString idLabel();
        void setIdLabel( QString id );

        QString itemID();
        void setId( QString id );
        
        bool showId();
        void setShowId( bool show );
        
        bool showVal();
        void setShowVal( bool show );
        
        QString unit();
        void setUnit( QString un );

        int labelx();
        void setLabelX( int x );

        int labely();
        void setLabelY( int y );

        int labelRot();
        void setLabelRot( int rot );
        
        void setLabelPos( int x, int y, int rot=0 );
        void setLabelPos();
        
        int valLabelx();
        void setValLabelX( int x );

        int valLabely();
        void setValLabelY( int y );

        int valLabRot();
        void setValLabRot( int rot );
        
        int hflip();
        void setHflip( int hf );
        
        int vflip();
        void setVflip( int vf );
        
        void setValLabelPos( int x, int y, int rot );
        void setValLabelPos();
        
        void updateLabel( Label* label, QString txt );
        
        double getmultValue();
        
        //QString getHelp( QString file );
        virtual void setBackground( QString bck ){ m_BackGround = bck; }
        
        void setPrintable( bool p );
        QString print();

        QString itemType();
        QString category();
        QIcon   icon();

        virtual void inStateChanged( int ){}

        virtual void move( QPointF delta );
        void moveTo( QPointF pos );

        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    signals:
        void moved();

    public slots:
        virtual void slotProperties();
        virtual void rotateCW();
        virtual void rotateCCW();
        virtual void rotateHalf();
        virtual void H_flip();
        virtual void V_flip();
        virtual void slotRemove();
        void slotCopy();

        virtual void remove();

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
        void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );

        void setValue( double val );
        void setflip();
        
        double m_value;

        const QString multUnits;
        QString m_unit;
        QString m_mult;
        double  m_unitMult;
        
        int m_Hflip;
        int m_Vflip;
 static int m_error;

        Label* m_idLabel;
        Label* m_valLabel;

        QString m_id;
        QString m_type;
        QString m_category;
        QString m_BackGround;   // BackGround Image
        
        QString* m_help;
        
        QIcon   m_icon;
        QColor  m_color;
        QRectF  m_area;         // bounding rect
        QPointF m_eventpoint;

        bool m_showId;
        bool m_showVal;
        bool m_moving;
        bool m_printable;
        
        std::vector<Pin*> m_pin;
};

typedef Component* (*createItemPtr)( QObject* parent, QString type, QString id );


class Label : public QGraphicsTextItem
{
    friend class Component;
    
    Q_OBJECT
    public:
        Label( Component* parent );
        ~Label();

        void setLabelPos();
        
        //virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    public slots:
        void rotateCW();
        void rotateCCW();
        void rotate180();
        void H_flip( int hf );
        void V_flip( int vf );
        void updateGeometry(int, int, int);

    protected:
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent* event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
        void focusOutEvent(QFocusEvent *event);

    private:
        Component*  m_parentComp;
        
        int m_labelx;
        int m_labely;
        int m_labelrot;
};
#endif

