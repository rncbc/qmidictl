// qmidictlActionBarStyle.h
//
/****************************************************************************
   Copyright (C) 2010-2022, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __qmidictlActionBarStyle_h
#define __qmidictlActionBarStyle_h

/* This code is borrowed, stirred and adapted from menustyle.h
 *
 * https://github.com/mbnoimi/QtActionBar.git
 *
 * (c) 2015 by Muhammad Bashir Al-Noimi
 * (c) 2014 by Stefan Frings
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 */

#include <QProxyStyle>


// Forward decls.
class QStyleOption;

/**
 * This proxy style changes the size of icons relative to the font size.
 * I use it to give icons a proper size on high-res displays.
 */

class qmidictlActionBarStyle : public QProxyStyle
{
public:
	/** Constructor */
	qmidictlActionBarStyle(QStyle *style = nullptr);

	/** Convert the provided dimension in dp (device-independent pixels) to the
	 *  corresponding number of actual pixels on the current display. */
	static int dpToPixels(int dp);

	/** Always draw QToolButtons in the default flat style */
	void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *option,
		QPainter *painter, const QWidget *widget = 0) const;

	/**Never show buttons with input focus as highlighted */
	void drawControl(ControlElement element, const QStyleOption *option,
		QPainter *painter, const QWidget *widget = 0) const;

	/** Calculate the size of icons relative to the font size */
	int pixelMetric(PixelMetric metric,
		const QStyleOption *option = 0, const QWidget *widget = 0) const;
};


#endif	// __qmidictlActionBarStyle_h

// end of qmidictlActionBarStyle.h
