/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include "image.h"
#include "circuit.h"

static const char* Image_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Image File")
};

Component* Image::construct( QObject* parent, QString type, QString id )
{ return new Image( parent, type, id ); }

LibraryItem* Image::libraryItem()
{
    return new LibraryItem(
            tr( "Image" ),
            tr( "Graphical" ),
            "image.png",
            "Image",
            Image::construct);
}

Image::Image( QObject* parent, QString type, QString id )
     : Shape( parent, type, id )
{
    Q_UNUSED( Image_properties );

    m_BackGround = "";
    m_image = QPixmap( ":/saveimage.png" );
    m_hSize = 80;
    m_vSize = 80;
    m_area = QRectF( -40, -40, 80, 80 );

    m_movie = 0l;
}
Image::~Image(){}

void Image::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else
    {
        event->accept();
        QMenu* menu = new QMenu();
        contextMenu( event, menu );
        Component::contextMenu( event, menu );
        menu->deleteLater();
    }
}

void Image::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* loadAction = menu->addAction( QIcon(":/load.png"),tr("Load Image") );
    connect( loadAction, SIGNAL(triggered()), this, SLOT(slotLoad()) );

    menu->addSeparator();
}

void Image::slotLoad()
{
    QString fil = m_BackGround;
    if( fil.isEmpty() ) fil = Circuit::self()->getFileName();

    const QString dir = fil;

    QString fileName = QFileDialog::getOpenFileName( 0l, tr("Load Image"), dir,
                       tr("All files (*.*)"));

    if( fileName.isEmpty() ) return; // User cancels loading

    setBackground( fileName );
}

void Image::updateGif( const QRect &rect )
{
    m_image = m_movie->currentPixmap();
    update();
}

void Image::setBackground( QString bck )
{
    if( bck.isEmpty() ) return;

    QDir circuitDir = QFileInfo( Circuit::self()->getFileName() ).absoluteDir();
    QString absPath = circuitDir.absoluteFilePath( bck );

    if( bck.endsWith(".gif") )
    {
        if( m_movie ) delete m_movie;

        m_movie = new QMovie( bck );
        m_movie->setParent( this );

        if( m_movie->isValid() )
        {
            m_movie->setCacheMode( QMovie::CacheAll );
            connect( m_movie, SIGNAL( updated(const QRect &) ), this, SLOT( updateGif(const QRect &) ) );
            m_movie->start();
        }
        else qDebug() << "Image::setBackground : not a valid Gif animation";
    }

    if( m_image.load( absPath ) )
    {
        m_BackGround = absPath;
        Shape::setHSize( m_image.width() );
        Shape::setVSize( m_image.height() );
    }
    else if( m_BackGround.isEmpty() )
    {
        m_image = QPixmap( ":/saveimage.png" );
    }
    else
    {
        m_image = QPixmap( m_BackGround );
    }
}

QString Image::background()
{
    QDir circuitDir = QFileInfo( Circuit::self()->getFileName() ).absoluteDir();

    return circuitDir.relativeFilePath( m_BackGround );
}

void Image::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    QPen pen(Qt::black, m_border, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    if( isSelected() ) pen.setColor( Qt::darkGray);

    p->setBrush( m_color );
    p->setPen( pen );
    
    if( m_border > 0 ) p->drawRect( m_area );
    else               p->fillRect( m_area, p->brush() );

    p->drawPixmap( m_area, m_image, m_image.rect() );
}

#include "moc_image.cpp"
