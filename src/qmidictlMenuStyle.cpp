// qmidictlMenuStyle.cpp
//
/****************************************************************************
   Copyright (C) 2010-2018, rncbc aka Rui Nuno Capela. All rights reserved.

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

/* This code is borrowed/adapted from menustyle.cpp
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

#include "qmidictlMenuStyle.h"

#include <QFontMetrics>
#include <QApplication>
#include <QScreen>
#include <QStyleOptionToolButton>


qmidictlMenuStyle::qmidictlMenuStyle ( QStyle *style ) : QProxyStyle(style)
{
}


int qmidictlMenuStyle::dpToPixels ( int dp )
{
	return (dp * qApp->primaryScreen()->physicalDotsPerInch()) / 160;
}


void qmidictlMenuStyle::drawComplexControl (
	ComplexControl control, const QStyleOptionComplex *option,
	QPainter *painter, const QWidget *widget ) const
{
	if (control == CC_ToolButton) {
		const QStyleOptionToolButton *opt
			= reinterpret_cast<const QStyleOptionToolButton *>(option);
		QStyleOptionToolButton newOption(*opt);
		newOption.state = State_AutoRaise;
		if (option->state & State_Enabled) {
			newOption.state |= State_Enabled;
		}
		QProxyStyle::drawComplexControl(control, &newOption, painter, widget);
		return;
	}

	QProxyStyle::drawComplexControl(control, option, painter, widget);
}


void qmidictlMenuStyle::drawControl (
	ControlElement element, const QStyleOption *option,
	QPainter *painter, const QWidget *widget ) const
{
	if (element == CE_PushButton) {
		if (const QStyleOptionButton *opt = qstyleoption_cast<const QStyleOptionButton *>(option)) {
			if (opt->state | State_HasFocus) {
				QStyleOptionButton newOption(*opt);
				newOption.state &= ~State_HasFocus;
				QProxyStyle::drawControl(element, &newOption, painter, widget);
				return;
			}
		}
	}

	QProxyStyle::drawControl(element, option, painter, widget);
}


int qmidictlMenuStyle::pixelMetric ( PixelMetric metric,
	const QStyleOption* option, const QWidget* widget ) const
{
	switch (metric) {
	case PM_ButtonIconSize:
	case PM_SmallIconSize:
	case PM_ToolBarIconSize:
		return dpToPixels(24);
	case PM_LargeIconSize:
		return dpToPixels(48);
	default:
		return QProxyStyle::pixelMetric(metric, option, widget);
	}
}


// end of qmidictlMenuStyle.cpp
