// qmidictlActionBarStyle.cpp
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

/* This code is borrowed, stirred and adapted from menustyle.cpp
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

#include "qmidictlActionBarStyle.h"

#include <QStyleOption>

#include <QApplication>
#include <QScreen>
#include <QPainter>


qmidictlActionBarStyle::qmidictlActionBarStyle ( QStyle *style )
	: QProxyStyle(style)
{
}


int qmidictlActionBarStyle::dpToPixels ( int dp )
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
	return (dp * QApplication::primaryScreen()->physicalDotsPerInch()) / 160;
#else
	return dp;
#endif
}


void qmidictlActionBarStyle::drawComplexControl (
	ComplexControl control, const QStyleOptionComplex *option,
	QPainter *painter, const QWidget *widget ) const
{
	if (control == CC_ToolButton) {
		const QStyleOptionToolButton *opt
			= qstyleoption_cast<const QStyleOptionToolButton *> (option);
		if (opt) {
			QStyleOptionToolButton newOption(*opt);
		//	newOption.state = State_AutoRaise;
		//	if (opt->state & State_Enabled)
		//		newOption.state |= State_Enabled;
			if (opt->state | State_DownArrow)
				newOption.state &= ~State_DownArrow;
			if (opt->state & (State_Sunken | State_On)) {
				newOption.palette.setBrush(
					QPalette::ButtonText, newOption.palette.highlightedText());
				newOption.palette.setBrush(
					QPalette::Button, newOption.palette.highlight());
			}
			QProxyStyle::drawComplexControl(control, &newOption, painter, widget);
		#if defined(Q_OS_ANDROID)
			if (opt->state & (State_Sunken | State_On)) {
				QColor over = newOption.palette.highlight().color();
				over.setAlpha(120);
				painter->save();
				painter->fillRect(newOption.rect.adjusted(+8, +8, -8, -8), over);
				painter->restore();
			}
		#endif
			return;
		}
	}

	QProxyStyle::drawComplexControl(control, option, painter, widget);
}


void qmidictlActionBarStyle::drawControl ( ControlElement control,
	const QStyleOption *option, QPainter *painter, const QWidget *widget ) const
{
	if (control == CE_PushButton) {
		const QStyleOptionButton *opt
			= qstyleoption_cast<const QStyleOptionButton *> (option);
		if (opt) {
			QStyleOptionButton newOption(*opt);
			if (opt->state | State_HasFocus)
				newOption.state &= ~State_HasFocus;
			if (opt->state | State_DownArrow)
				newOption.state &= ~State_DownArrow;
			if (opt->state & (State_Sunken | State_On)) {
				newOption.palette.setBrush(
					QPalette::ButtonText, newOption.palette.highlightedText());
				newOption.palette.setBrush(
					QPalette::Button, newOption.palette.highlight());
			}
			QProxyStyle::drawControl(control, &newOption, painter, widget);
		#if defined(Q_OS_ANDROID)
			if (opt->state & (State_Sunken | State_On)) {
				QColor over = newOption.palette.color(QPalette::Highlight);
				over.setAlpha(120);
				painter->save();
				painter->fillRect(newOption.rect.adjusted(+8, +8, -8, -8), over);
				painter->restore();
			}
		#endif
			return;
		}
	}

	QProxyStyle::drawControl(control, option, painter, widget);
}


int qmidictlActionBarStyle::pixelMetric (
	PixelMetric metric, const QStyleOption* option, const QWidget* widget ) const
{
	switch (metric) {
	case PM_ButtonIconSize:
	case PM_SmallIconSize:
	case PM_ToolBarIconSize:
		return dpToPixels(32);
	case PM_LargeIconSize:
		return dpToPixels(48);
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
	case PM_SliderThickness:
	case PM_SliderLength:
		return dpToPixels(32);
#endif
	default:
		return QProxyStyle::pixelMetric(metric, option, widget);
	}
}


// end of qmidictlActionBarStyle.cpp
