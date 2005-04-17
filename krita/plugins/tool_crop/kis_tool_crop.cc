/*
 *  kis_tool_crop.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include <qpainter.h>
#include <qpen.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qcombobox.h>
#include <qrect.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include <kis_global.h>
#include <kis_painter.h>
#include <kis_canvas_controller.h>
#include <kis_canvas_subject.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_button_press_event.h>
#include <kis_button_release_event.h>
#include <kis_move_event.h>
#include <kis_transaction.h>

#include "kis_tool_crop.h"
#include "wdg_tool_crop.h"


KisToolCrop::KisToolCrop()
{
	setName("tool_crop");
	setCursor(KisCursor::selectCursor());
	m_subject = 0;
	m_selecting = false;
	m_startPos = QPoint(0, 0);
	m_endPos = QPoint(0, 0);
        m_handleSize = 12;
        m_cropSelectionDrawn = false;
	m_optWidget = 0;
}

KisToolCrop::~KisToolCrop()
{
}

void KisToolCrop::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	super::update(m_subject);
}

void KisToolCrop::paint(QPainter& gc)
{
	if (m_selecting)
		paintOutlineWithHandles(gc, QRect());
}

void KisToolCrop::paint(QPainter& gc, const QRect& rc)
{
	if (m_selecting)
		paintOutlineWithHandles(gc, rc);
}

void KisToolCrop::clearRect()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		KisImageSP img = m_subject -> currentImg();

		Q_ASSERT(controller);

		controller -> canvas() -> update();
		
		m_startPos = QPoint(0, 0);
		m_endPos = QPoint(0, 0);

		((WdgToolCrop*)m_optWidget) -> intStartX -> setValue(m_startPos.x());
		((WdgToolCrop*)m_optWidget) -> intStartY -> setValue(m_startPos.y());
		((WdgToolCrop*)m_optWidget) -> intEndX -> setValue(m_endPos.x());
		((WdgToolCrop*)m_optWidget) -> intEndY -> setValue(m_endPos.y());

		m_selecting = false;
	}
}

void KisToolCrop::buttonPress(KisButtonPressEvent *e)
{
	if (m_subject) {
		KisImageSP img = m_subject -> currentImg();

		if (img && img -> activeDevice() && e -> button() == LeftButton) {
                        //clearRect();
			
                        if( !m_cropSelectionDrawn ) //if the selection is not already drawn
                        {
                                m_startPos = e -> pos().floorQPoint();
                                m_endPos = e -> pos().floorQPoint();
                        }
                        else
                        {
                        m_type = mouseOnHandle( e -> pos().floorQPoint() );
                        }

			((WdgToolCrop*)m_optWidget) -> intStartX -> setValue(m_startPos.x());
			((WdgToolCrop*)m_optWidget) -> intStartY -> setValue(m_startPos.y());
			((WdgToolCrop*)m_optWidget) -> intEndX -> setValue(m_endPos.x());
			((WdgToolCrop*)m_optWidget) -> intEndY -> setValue(m_endPos.y());

			m_selecting = true;
                }
        }
}

void KisToolCrop::move(KisMoveEvent *e)
{
        if ( m_subject )
        {
                if( m_selecting ) //if the user selects
                {
                        if( !m_cropSelectionDrawn ) //if the cropSelection is not yet drawn
                        {
                                if (m_startPos != m_endPos)
                                paintOutlineWithHandles();

                                m_endPos = e -> pos().floorQPoint();
                                ((WdgToolCrop*)m_optWidget) -> intEndX -> setValue(m_endPos.x());
                                ((WdgToolCrop*)m_optWidget) -> intEndY -> setValue(m_endPos.y());

                                paintOutlineWithHandles();
                        }
                        else //if the crop selection is already drawn
                        {
                                QPoint pos = e -> pos().floorQPoint();
                                Q_INT32 width = QABS( m_startPos.x() - m_endPos.x() ); //with of the selected rectangle
                                Q_INT32 height = QABS( m_startPos.y() - m_endPos.y() ); //height of the selected rectangle
                                switch (m_type)
                                {
                                        case (UpperLeft):
                                                if (m_startPos != m_oldStartPos)
                                                        paintOutlineWithHandles();
                                                m_oldStartPos = m_startPos;
                                                m_startPos.setX( pos.x() - m_dx );
                                                m_startPos.setY( pos.y() - m_dy );
                                                ((WdgToolCrop*)m_optWidget) -> intStartX -> setValue(m_startPos.x());
                                                ((WdgToolCrop*)m_optWidget) -> intStartY -> setValue(m_startPos.y());
                                                paintOutlineWithHandles();
                                                break;
                                        case (LowerRight):
                                                if (m_startPos != m_endPos)
                                                        paintOutlineWithHandles();
                                                m_endPos.setX( pos.x() + m_dx );
                                                m_endPos.setY( pos.y() + m_dy );
                                                ((WdgToolCrop*)m_optWidget) -> intEndX -> setValue(m_endPos.x());
                                                ((WdgToolCrop*)m_optWidget) -> intEndY -> setValue(m_endPos.y());
                                                paintOutlineWithHandles();
                                                break;
                                        case (LowerLeft):
                                                if ( (m_startPos != m_oldStartPos) || (m_endPos != m_oldEndPos) )
                                                        paintOutlineWithHandles();
                                                
                                                m_startPos.setX( pos.x() - m_dx );
                                                m_startPos.setY( pos.y() - height + m_dy );
                                                m_endPos.setX( pos.x() + width - m_dx );
                                                m_endPos.setY( pos.y() + m_dy );
                                                ((WdgToolCrop*)m_optWidget) -> intStartX -> setValue(m_endPos.x());
                                                ((WdgToolCrop*)m_optWidget) -> intStartY -> setValue(m_endPos.y());
                                                ((WdgToolCrop*)m_optWidget) -> intEndX -> setValue(m_endPos.x());
                                                ((WdgToolCrop*)m_optWidget) -> intEndY -> setValue(m_endPos.y());
                                                paintOutlineWithHandles();
                                                break;
                                        case (UpperRight):
                                                if ( (m_startPos != m_oldStartPos) || (m_endPos != m_oldEndPos) )
                                                        paintOutlineWithHandles();
                                                m_oldStartPos = m_startPos;
                                                m_oldEndPos = m_endPos;
                                                m_startPos.setX( pos.x() - width + m_dx);
                                                m_startPos.setY( pos.y() - m_dy );
                                                m_endPos.setX( pos.x() + m_dx );
                                                m_endPos.setY( pos.y() + height - m_dy);
                                                ((WdgToolCrop*)m_optWidget) -> intStartX -> setValue(m_endPos.x());
                                                ((WdgToolCrop*)m_optWidget) -> intStartY -> setValue(m_endPos.y());
                                                ((WdgToolCrop*)m_optWidget) -> intEndX -> setValue(m_endPos.x());
                                                ((WdgToolCrop*)m_optWidget) -> intEndY -> setValue(m_endPos.y());
                                                paintOutlineWithHandles();
                                                break;
                                }
                        }
                }
                else //if we are not selecting
                {
                        if ( m_cropSelectionDrawn )  //if the crop selection is drawn
                        {
                                Q_INT32 type = mouseOnHandle( e -> pos().floorQPoint() );
                                //set resize cursor if we are on one of the handles
                                setMoveResizeCursor(type);
                        } 
                }
        }
}

void KisToolCrop::buttonRelease(KisButtonReleaseEvent *e)
{
	KisImageSP img = m_subject -> currentImg();

	if (!img)
		return;

        if (m_startPos != m_endPos)
        {
                m_cropSelectionDrawn = true;
        }

	if (m_subject && m_selecting) {
		if (m_startPos == m_endPos) {
			clearRect();
		}
		if( !m_cropSelectionDrawn )
                {
                        m_endPos = e -> pos().floorQPoint();
                        ((WdgToolCrop*)m_optWidget) -> intEndX -> setValue(m_endPos.x());
                        ((WdgToolCrop*)m_optWidget) -> intEndY -> setValue(m_endPos.y());
                }
		m_selecting = false;
	}
}

void KisToolCrop::paintOutlineWithHandles()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		QWidget *canvas = controller -> canvas();
		QPainter gc(canvas);
		QRect rc;

		paintOutlineWithHandles(gc, rc);
	}
}

void KisToolCrop::paintOutlineWithHandles(QPainter& gc, const QRect&)
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		RasterOp op = gc.rasterOp();
		QPen old = gc.pen();
		QPen pen(Qt::SolidLine);
		pen.setWidth(1);
		QPoint start;
		QPoint end;

		Q_ASSERT(controller);
		start = controller -> windowToView(m_startPos);
		end = controller -> windowToView(m_endPos);

		gc.setRasterOp(Qt::NotROP);
                gc.setPen(pen);
                //draw handles
                m_handlesRegion = handles(QRect(start, end));

                Q_INT32 startx;
                Q_INT32 starty;
                Q_INT32 endx;
                Q_INT32 endy;
                if(start.x()<=end.x())
                {
                        startx=start.x();
                        endx=end.x();
                }
                else
                {
                        startx=end.x();
                        endx=start.x();
                }
                if(start.y()<=end.y())
                {
                        starty=start.y();
                        endy=end.y();
                }
                else
                {
                        starty=end.y();
                        endy=start.y();
                }
                gc.drawRect(QRect(start, end));
                gc.drawLine(0,endy,startx,endy);
                gc.drawLine(startx,endy, startx, m_subject -> currentImg() -> height());
                gc.drawLine(endx,0,endx,starty);
                gc.drawLine(endx,starty,m_subject -> currentImg() -> width(),starty);
                QMemArray <QRect> rects = m_handlesRegion.rects (); 
                for (QMemArray <QRect>::ConstIterator it = rects.begin (); 
                        it != rects.end ();
                        it++)
                {
                        gc.fillRect (*it, Qt::black);
                }


                gc.setRasterOp(op);
		gc.setPen(old);
	}
}

void KisToolCrop::crop() {
	// XXX: Should cropping be part of KisImage/KisPaintDevice's API?

        m_cropSelectionDrawn = false;

	KisImageSP img = m_subject -> currentImg();

	if (img -> undoAdapter())
		img -> undoAdapter() -> beginMacro(i18n("Crop"));
	
	if (!img)
		return;

	if (m_endPos.y() < 0)
		m_endPos.setY(0);
	
	if (m_endPos.y() > img -> height())
		m_endPos.setY(img -> height());
	
	if (m_endPos.x() < 0)
		m_endPos.setX(0);
	
	if (m_endPos.x() > img -> width())
				m_endPos.setX(img -> width());
	
	QRect rc(m_startPos, m_endPos);
	rc = rc.normalize();
	
	
	if (((WdgToolCrop *)m_optWidget) -> cmbType -> currentItem() == 0) {
		KisLayerSP layer = img -> activeLayer();
		cropLayer(layer, rc);
		KNamedCommand * cmd = layer -> moveCommand(m_subject -> canvasController(),  -rc.x(), -rc.y());
		if (m_subject -> undoAdapter()) m_subject -> undoAdapter() -> addCommand(cmd);
		img -> notify();
	}
	else {
		vKisLayerSP layers = img -> layers();
		vKisLayerSP_it it;
		for ( it = layers.begin(); it != layers.end(); ++it ) {
			KisLayerSP layer = (*it);
			cropLayer(layer, rc);
			KNamedCommand * cmd = layer -> moveCommand(m_subject -> canvasController(),  -rc.x(), -rc.y());
			if (m_subject -> undoAdapter()) m_subject -> undoAdapter() -> addCommand(cmd);
		}
		img -> resize(rc);
		img -> notify(QRect(0, 0, rc.width(), rc.height()));
	}

	if (img -> undoAdapter())
		img -> undoAdapter() -> endMacro();

	((WdgToolCrop*)m_optWidget) -> intStartX -> setValue(0);
	((WdgToolCrop*)m_optWidget) -> intStartY -> setValue(0);
	((WdgToolCrop*)m_optWidget) -> intEndX -> setValue(0);
	((WdgToolCrop*)m_optWidget) -> intEndY -> setValue(0);

}

void KisToolCrop::cropLayer(KisLayerSP layer, QRect rc) 
{
	KisTransaction * t = new KisTransaction(i18n("Crop"), layer.data());
	Q_CHECK_PTR(t);
	
	layer -> crop(rc);

	m_subject -> undoAdapter() -> addCommand(t);
	
}

QWidget* KisToolCrop::createOptionWidget(QWidget* parent)
{
	WdgToolCrop * w = new WdgToolCrop(parent);
	Q_CHECK_PTR(w);

	connect(w -> bnCrop, SIGNAL(clicked()), this, SLOT(crop()));

	m_optWidget = w;
	return m_optWidget;
}

QWidget* KisToolCrop::optionWidget()
{
	return m_optWidget;
}

void KisToolCrop::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Crop"), 
					    "crop", 
					    0, 
					    this,
					    SLOT(activate()), 
					    collection, 
					    name());
		Q_CHECK_PTR(m_action);

		m_action -> setExclusiveGroup("tools");

		m_ownAction = true;
	}
}

QRegion KisToolCrop::handles(QRect rect)
{
        QRegion handlesRegion;

        //add handle at the lower right corner
        handlesRegion += QRect( QABS( rect.width() ) - m_handleSize, QABS( rect.height() ) - m_handleSize, m_handleSize, m_handleSize );
        //add handle at the upper right corner
        handlesRegion += QRect( QABS( rect.width() ) - m_handleSize, 0, m_handleSize, m_handleSize );
        //add rectangle at the lower left corner
        handlesRegion += QRect( 0, QABS( rect.height() ) - m_handleSize, m_handleSize, m_handleSize );
        //add rectangle at the upper left corner
        handlesRegion += QRect( 0, 0, m_handleSize, m_handleSize );

        //move the handles to the correct position
        if( rect.width() >= 0 && rect.height() >= 0)
        {
                handlesRegion.translate ( rect.x(), rect.y() );
        }
        else if( rect.width() < 0 && rect.height() >= 0)
        {
                handlesRegion.translate ( rect.x() - QABS( rect.width() ), rect.y() );
        }
        else if( rect.width() >= 0 && rect.height() < 0)
        {
                handlesRegion.translate ( rect.x(), rect.y() - QABS( rect.height() ) );
        }
        else if( rect.width() < 0 && rect.height() < 0)
        {
                handlesRegion.translate ( rect.x() - QABS( rect.width() ), rect.y() - QABS( rect.height() ) );
        }
        return handlesRegion;
}

Q_INT32 KisToolCrop::mouseOnHandle(QPoint currentViewPoint)
{
        KisCanvasControllerInterface *controller = m_subject -> canvasController();
        Q_ASSERT(controller);
        QPoint start = controller -> windowToView(m_startPos);
        QPoint end = controller -> windowToView(m_endPos);

        Q_INT32 startx;
                Q_INT32 starty;
                Q_INT32 endx;
                Q_INT32 endy;
                if(start.x()<=end.x())
                {
                        startx=start.x();
                        endx=end.x();
                }
                else
                {
                        startx=end.x();
                        endx=start.x();
                }
                if(start.y()<=end.y())
                {
                        starty=start.y();
                        endy=end.y();
                }
                else
                {
                        starty=end.y();
                        endy=start.y();
                }

        if ( QRect ( startx, starty, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dx=QABS( startx-currentViewPoint.x() );
                        m_dy=QABS( starty-currentViewPoint.y() );
                }
                return UpperLeft;
        }
        else if ( QRect ( startx, endy -  m_handleSize, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dx=QABS( startx-currentViewPoint.x() );
                        m_dy=QABS( endy-currentViewPoint.y() );
                }
                return LowerLeft;
        }
        else if ( QRect ( endx - m_handleSize, starty, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dx=QABS( endx-currentViewPoint.x() );
                        m_dy=QABS( starty-currentViewPoint.y() );
                }
                return UpperRight;
        }
        else if ( QRect ( endx - m_handleSize, endy - m_handleSize, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dx=QABS( endx-currentViewPoint.x() );
                        m_dy=QABS( endy-currentViewPoint.y() );
                }
                return LowerRight;
        }
        else return None;
}

void KisToolCrop::setMoveResizeCursor (Q_INT32 handle)
{
        switch (handle)
        {
        case (UpperLeft):
        case (LowerRight):
                m_subject -> setCanvasCursor(KisCursor::sizeFDiagCursor());
                return;
        case (LowerLeft):
        case (UpperRight):
                m_subject -> setCanvasCursor(KisCursor::moveCursor());
                return;
        }
        m_subject -> setCanvasCursor(KisCursor::selectCursor());
        return;
}

#include "kis_tool_crop.moc"
