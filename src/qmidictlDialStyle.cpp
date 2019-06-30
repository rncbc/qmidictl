// qmidictlDialStyle.cpp
//
/****************************************************************************
   Copyright (C) 2010-2019, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

/* This code is borrowed from Qt 4.6 source code.
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (qt-info@nokia.com)
 */

#include "qmidictlDialStyle.h"

#include <QStyleOption>
#include <QPainter>
#include <QPixmapCache>

#include <math.h>

#define Q_PI M_PI

#define qSin(x)	qreal(::sin(x))
#define qCos(x)	qreal(::cos(x))


static
QString uniqueName ( const QString &key, const QStyleOption *option, const QSize &size )
{
	const QStyleOptionComplex *complexOption = qstyleoption_cast<const QStyleOptionComplex *>(option);
	QString tmp = QString::fromLatin1("%1-%2-%3-%4-%5-%6x%7").arg(key).arg(uint(option->state)).arg(option->direction)
					.arg(complexOption ? uint(complexOption->activeSubControls) : uint(0))
					.arg(option->palette.cacheKey()).arg(size.width()).arg(size.height());
	return tmp;
}


static
int calcBigLineSize ( int radius )
{
	int bigLineSize = radius / 6;
	if (bigLineSize < 4)
		bigLineSize = 4;
	if (bigLineSize > radius / 2)
		bigLineSize = radius / 2;
	return bigLineSize;
}


static
QPolygonF calcLines ( const QStyleOptionSlider *dial )
{
	QPolygonF poly;
	int width = dial->rect.width();
	int height = dial->rect.height();
	qreal r = qMin(width, height) / 2;
	int bigLineSize = calcBigLineSize(int(r));

	qreal xc = width / 2 + 0.5;
	qreal yc = height / 2 + 0.5;
	int ns = dial->tickInterval;
	int notches = (dial->maximum + ns - 1 - dial->minimum) / ns;
	if (notches <= 0)
		return poly;
	if (dial->maximum < dial->minimum || dial->maximum - dial->minimum > 1000) {
		int maximum = dial->minimum + 1000;
		notches = (maximum + ns - 1 - dial->minimum) / ns;
	}

	poly.resize(2 + 2 * notches);
	int smallLineSize = bigLineSize / 2;
	for (int i = 0; i <= notches; ++i) {
		qreal angle = dial->dialWrapping ? Q_PI * 3 / 2 - i * 2 * Q_PI / notches
				  : (Q_PI * 8 - i * 10 * Q_PI / notches) / 6;
		qreal s = qSin(angle);
		qreal c = qCos(angle);
		if (i == 0 || (((ns * i) % (dial->pageStep ? dial->pageStep : 1)) == 0)) {
			poly[2 * i] = QPointF(xc + (r - bigLineSize) * c,
								  yc - (r - bigLineSize) * s);
			poly[2 * i + 1] = QPointF(xc + r * c, yc - r * s);
		} else {
			poly[2 * i] = QPointF(xc + (r - 1 - smallLineSize) * c,
								  yc - (r - 1 - smallLineSize) * s);
			poly[2 * i + 1] = QPointF(xc + (r - 1) * c, yc -(r - 1) * s);
		}
	}
	return poly;
}


static
QPointF calcRadialPos ( const QStyleOptionSlider *dial, qreal offset )
{
	const int width = dial->rect.width();
	const int height = dial->rect.height();
	const int r = qMin(width, height) / 2;
	const int currentSliderPosition = dial->upsideDown ? dial->sliderPosition : (dial->maximum - dial->sliderPosition);
	qreal a = 0;
	if (dial->maximum == dial->minimum)
		a = Q_PI / 2;
	else if (dial->dialWrapping)
		a = Q_PI * 3 / 2 - (currentSliderPosition - dial->minimum) * 2 * Q_PI
			/ (dial->maximum - dial->minimum);
	else
		a = (Q_PI * 8 - (currentSliderPosition - dial->minimum) * 10 * Q_PI
			/ (dial->maximum - dial->minimum)) / 6;
	qreal xc = width / 2.0;
	qreal yc = height / 2.0;
	qreal len = r - calcBigLineSize(r) - 3;
	qreal back = offset * len;
	QPointF pos(QPointF(xc + back * qCos(a), yc - back * qSin(a)));
	return pos;
}


static
void drawDial ( const QStyleOptionSlider *option, QPainter *painter )
{
	QPalette pal = option->palette;
	QColor buttonColor = pal.button().color();
	const int width = option->rect.width();
	const int height = option->rect.height();
	const bool enabled = option->state & QStyle::State_Enabled;
	qreal r = qMin(width, height) / 2;
	r -= r/50;
	const qreal penSize = r/20.0;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing);

	// Draw notches
	if (option->subControls & QStyle::SC_DialTickmarks) {
		painter->setPen(option->palette.dark().color().darker(120));
		painter->drawLines(calcLines(option));
	}

	// Cache dial background
	QString a = QString::fromLatin1("qdial");
	QRect rect = option->rect;
	QPixmap internalPixmapCache;
	QImage imageCache;
	QPainter *p = painter;
	QString unique = uniqueName((a), option, option->rect.size());
	int txType = painter->deviceTransform().type() | painter->worldTransform().type();
	bool doPixmapCache = txType <= QTransform::TxTranslate;
	if (doPixmapCache && QPixmapCache::find(unique, &internalPixmapCache)) {
		painter->drawPixmap(option->rect.topLeft(), internalPixmapCache);
	} else {
		if (doPixmapCache) {
			rect.setRect(0, 0, option->rect.width(), option->rect.height());
			imageCache = QImage(option->rect.size(), QImage::Format_ARGB32_Premultiplied);
			imageCache.fill(0);
			p = new QPainter(&imageCache);
		}
	//--BEGIN_STYLE_PIXMAPCACHE(QString::fromLatin1("qdial"));

		p->setRenderHint(QPainter::Antialiasing);

		const qreal d_ = r / 6;
		const qreal dx = option->rect.x() + d_ + (width - 2 * r) / 2 + 1;
		const qreal dy = option->rect.y() + d_ + (height - 2 * r) / 2 + 1;

		QRectF br = QRectF(dx + 0.5, dy + 0.5,
						   int(r * 2 - 2 * d_ - 2),
						   int(r * 2 - 2 * d_ - 2));
		buttonColor.setHsv(buttonColor .hue(),
						   qMin(140, buttonColor .saturation()),
						   qMax(180, buttonColor.value()));
		QColor shadowColor(0, 0, 0, 20);

		if (enabled) {
			// Drop shadow
			qreal shadowSize = qMax(1.0, penSize/2.0);
			QRectF shadowRect= br.adjusted(-2*shadowSize, -2*shadowSize,
										   2*shadowSize, 2*shadowSize);
			QRadialGradient shadowGradient(shadowRect.center().x(),
										   shadowRect.center().y(), shadowRect.width()/2.0,
										   shadowRect.center().x(), shadowRect.center().y());
			shadowGradient.setColorAt(qreal(0.91), QColor(0, 0, 0, 40));
			shadowGradient.setColorAt(qreal(1.0), Qt::transparent);
			p->setBrush(shadowGradient);
			p->setPen(Qt::NoPen);
			p->translate(shadowSize, shadowSize);
			p->drawEllipse(shadowRect);
			p->translate(-shadowSize, -shadowSize);

			// Main gradient
			QRadialGradient gradient(br.center().x() - br.width()/3, dy,
									 br.width()*1.3, br.center().x(),
									 br.center().y() - br.height()/2);
			gradient.setColorAt(0, buttonColor.lighter(110));
			gradient.setColorAt(qreal(0.5), buttonColor);
			gradient.setColorAt(qreal(0.501), buttonColor.darker(102));
			gradient.setColorAt(1, buttonColor.darker(115));
			p->setBrush(gradient);
		} else {
			p->setBrush(Qt::NoBrush);
		}

		p->setPen(QPen(buttonColor.darker(280)));
		p->drawEllipse(br);
		p->setBrush(Qt::NoBrush);
		p->setPen(buttonColor.lighter(110));
		p->drawEllipse(br.adjusted(1, 1, -1, -1));

		if (option->state & QStyle::State_HasFocus) {
			QColor highlight = pal.highlight().color();
			highlight.setHsv(highlight.hue(),
							 qMin(160, highlight.saturation()),
							 qMax(230, highlight.value()));
			highlight.setAlpha(127);
			p->setPen(QPen(highlight, 2.0));
			p->setBrush(Qt::NoBrush);
			p->drawEllipse(br.adjusted(-1, -1, 1, 1));
		}
	//--END_STYLE_PIXMAPCACHE
		if (doPixmapCache) {
			p->end();
			delete p;
			internalPixmapCache = QPixmap::fromImage(imageCache);
			painter->drawPixmap(option->rect.topLeft(), internalPixmapCache);
			QPixmapCache::insert(unique, internalPixmapCache);
		}
	}


	QPointF dp = calcRadialPos(option, qreal(0.70));
	buttonColor = buttonColor.lighter(104);
	buttonColor.setAlphaF(qreal(0.8));
	const qreal ds = r/qreal(7.0);
	QRectF dialRect(dp.x() - ds, dp.y() - ds, 2*ds, 2*ds);
	QRadialGradient dialGradient(dialRect.center().x() + dialRect.width()/2,
								 dialRect.center().y() + dialRect.width(),
								 dialRect.width()*2,
								 dialRect.center().x(), dialRect.center().y());
	dialGradient.setColorAt(1, buttonColor.darker(140));
	dialGradient.setColorAt(qreal(0.4), buttonColor.darker(120));
	dialGradient.setColorAt(0, buttonColor.darker(110));
	if (penSize > 3.0) {
		painter->setPen(QPen(QColor(0, 0, 0, 25), penSize));
		painter->drawLine(calcRadialPos(option, qreal(0.90)), calcRadialPos(option, qreal(0.96)));
	}

	painter->setBrush(dialGradient);
	painter->setPen(QColor(255, 255, 255, 150));
	painter->drawEllipse(dialRect.adjusted(-1, -1, 1, 1));
	painter->setPen(QColor(0, 0, 0, 80));
	painter->drawEllipse(dialRect);
	painter->restore();
}


void qmidictlDialStyle::drawComplexControl (
	ComplexControl cc, const QStyleOptionComplex *optc,
	QPainter *painter, const QWidget *widget) const
{
	if (cc == QStyle::CC_Dial) {
		const QStyleOptionSlider *option
			= qstyleoption_cast<const QStyleOptionSlider *> (optc);
		if (option) {
			drawDial(option, painter);
			return;
		}
	}

	QCommonStyle::drawComplexControl(cc, optc, painter, widget);
}


// end of qmidictlDialSkulptureStyle.cpp
