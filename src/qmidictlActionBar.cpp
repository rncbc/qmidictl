// qmidictlActionBar.cpp
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

/* This code is borrowed, stirred and adapted from actionbar.cpp
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

#if QT_VERSION < 0x050100
#include "qmidictlAbout.h"
#include <QDesktopWidget>
#endif

#include "qmidictlActionBar.h"

#include <QAction>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QMenu>
#include <QFont>

#include <QApplication>
#include <QStyleOption>
#include <QPainter>
#include <QScreen>

#include <QResizeEvent>


qmidictlActionBar::qmidictlActionBar ( QWidget *parent ) : QWidget(parent)
{
	// Create layout
	m_layout = new QHBoxLayout(this);
	m_layout->setSpacing(8);
	m_layout->setMargin(8);
//	m_layout->setContentsMargins(0, 0, 0, 0);
	m_layout->setSizeConstraint(QLayout::SetNoConstraint);

	// App Navigation/Menu Icon
	m_appIcon = QIcon(":/images/actionMenu.png");

	// App Navigation/Menu Button
	m_appButton = new QToolButton();
	m_appButton->setAutoRaise(true);
	m_appButton->setFocusPolicy(Qt::NoFocus);
	m_appButton->setPopupMode(QToolButton::InstantPopup);
	m_appMenu = new QMenu(m_appButton);
	QObject::connect(m_appMenu, SIGNAL(aboutToHide()), SLOT(aboutToHideAppMenu()));
	QObject::connect(m_appMenu, SIGNAL(aboutToShow()), SLOT(aboutToShowAppMenu()));
	m_layout->addWidget(m_appButton);

	// View Control Button
	m_appTitle = new QLabel();
	const QFont& font = QWidget::font();
	m_appTitle->setFont(QFont(font.family(), font.pointSize() + 1, 75));
	m_layout->addWidget(m_appTitle);

	// Spacer
	m_layout->addStretch(1);

	// Action Overflow Button
	m_overflowButton = new QToolButton();
	m_overflowButton->setIcon(QIcon(":/images/actionOverflow.png"));
	m_overflowButton->setToolTip(tr("more"));
	m_overflowButton->setAutoRaise(true);
	m_overflowButton->setFocusPolicy(Qt::NoFocus);
	m_overflowButton->setPopupMode(QToolButton::InstantPopup);

	m_overflowMenu = new QMenu(m_overflowButton);
	m_overflowButton->setMenu(m_overflowMenu);
	m_layout->addWidget(m_overflowButton);
}


qmidictlActionBar::~qmidictlActionBar (void)
{
}


void qmidictlActionBar::resizeEvent ( QResizeEvent *event )
{
	const int oldWidth = event->oldSize().width();
	const int newWidth = event->size().width();

	if (oldWidth != newWidth)
		adjustContent();
}


void qmidictlActionBar::paintEvent ( QPaintEvent * )
{
	QStyleOption opt;
	opt.init(this);

	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}


void qmidictlActionBar::setIcon ( const QIcon& icon )
{
	m_appIcon = icon;
}


const QIcon& qmidictlActionBar::icon (void) const
{
	return m_appIcon;
}


void qmidictlActionBar::setTitle ( const QString& title )
{
	m_appTitle->setText(title);

	if (!m_appMenu->isEmpty()) {
		m_appButton->setIcon(QIcon(":/images/actionMenu.png"));
		m_appButton->setMenu(m_appMenu);
	} else {
		m_appButton->setIcon(m_appIcon);
		m_appButton->setMenu(NULL);
	}

	adjustContent();
}


QString qmidictlActionBar::title (void) const
{
	return m_appTitle->text();
}


void qmidictlActionBar::adjustContent (void)
{
#if QT_VERSION >= 0x050100
	const int screenWidth = QApplication::primaryScreen()->availableSize().width();
#else
	const int screenWidth = QApplication::desktop()->screen()->width();
#endif

	if (!m_appMenu->isEmpty()) {
		m_appButton->setIcon(QIcon(":/images/actionMenu.png"));
		m_appButton->setMenu(m_appMenu);
	}

	m_overflowButton->hide();

	// Check if all action buttons fit into the available space.
	for (int i = 0; i < m_buttonActions.size(); ++i) {
		QAction *action = m_buttonActions.at(i);
		QToolButton *button = m_actionButtons.at(i);
		if (action->isVisible()) {
			button->show();
		}
	}

	if (sizeHint().width() > screenWidth) {
		// The buttons don't fit, we need an overflow menu.
		m_overflowButton->show();
		m_overflowMenu->clear();
	} else {
		m_overflowButton->hide();
	}

	// Show/Hide action buttons and overflow menu entries
	QAction *lastAction = 0;
	for (int i = m_buttonActions.size() - 1; i >= 0; --i) {
		QAction *action = m_buttonActions.at(i);
		QToolButton *button = m_actionButtons.at(i);
		if (action->isVisible()) {
			if (sizeHint().width() <= screenWidth) {
				// show as button
				button->show();
			} else {
				// show as overflow menu entry
				button->hide();
				m_overflowMenu->insertAction(lastAction, action);
				lastAction = action;
			}
		}
	}
}


void qmidictlActionBar::addMenuItem ( QAction *action )
{
	QWidget::addAction(action);
	m_appMenu->addAction(action);
	if (!m_appMenu->isEmpty())
		m_appButton->setMenu(m_appMenu);
}


void qmidictlActionBar::addMenuItems ( QList<QAction*> actions )
{
	for (int i = 0; i < actions.size(); ++i)
		addMenuItem(actions.at(i));
}


void qmidictlActionBar::removeMenuItem ( QAction *action )
{
	QWidget::removeAction(action);
	m_appMenu->removeAction(action);
	if (m_appMenu->isEmpty())
		m_appButton->setMenu(NULL);
}


void qmidictlActionBar::addButton ( QAction *action, int position )
{
	if (position == -1)
		position = m_buttonActions.size();

	m_buttonActions.insert(position, action);
	QToolButton *button = new QToolButton();
	button->setText(action->text());
	button->setToolTip(action->text());
	const QIcon& icon = action->icon();
	if (!icon.isNull()) {
		button->setIcon(icon);
		button->setToolButtonStyle(Qt::ToolButtonIconOnly);
	}

	button->setEnabled(action->isEnabled());
	button->setFocusPolicy(Qt::NoFocus);
	button->setAutoRaise(true);
	QObject::connect(button, SIGNAL(clicked(bool)), action, SLOT(trigger()));
	QObject::connect(action, SIGNAL(changed()), SLOT(actionChanged()));
	m_actionButtons.insert(position, button);
	m_layout->insertWidget(position + 3, button);
}


void qmidictlActionBar::removeButton ( QAction* action )
{
	QToolButton *button = NULL;
	for (int i = 0; i < m_buttonActions.size(); ++i) {
		if (m_buttonActions.at(i) == action) {
			button = m_actionButtons.at(i);
			break;
		}
	}

	if (button) {
		m_layout->removeWidget(button);
		m_actionButtons.removeOne(button);
		delete button;
		m_buttonActions.removeOne(action);
	}
}


void qmidictlActionBar::openOverflowMenu (void)
{
    if (m_overflowButton->isVisible())
        m_overflowButton->click();
}


void qmidictlActionBar::actionChanged (void)
{
	QAction *action = qobject_cast<QAction *> (sender());
	const int index = m_buttonActions.indexOf(action);
	m_actionButtons[index]->setEnabled(action->isEnabled());
	m_actionButtons[index]->setIcon(action->icon());
}


void qmidictlActionBar::aboutToHideAppMenu (void)
{
	m_appButton->setIcon(QIcon(":/images/actionMenu.png"));
}


void qmidictlActionBar::aboutToShowAppMenu (void)
{
	m_appButton->setIcon(m_appIcon);
}


// end of qmidictlActionBar.cpp
