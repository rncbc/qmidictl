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

/* This code is borrowed/adapted from actionbar.cpp
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

#include "qmidictlActionBar.h"
#include "qmidictlMenuStyle.h"

#include <QIcon>
#include <QFont>
#include <QApplication>
#include <QScreen>
#include <QStyleFactory>
#include <QStyleOption>
#include <QPainter>

qmidictlActionBar::qmidictlActionBar ( QWidget *parent ) : QWidget(parent)
{
	const int paddingPixels = qmidictlMenuStyle::dpToPixels(16);
	const int minWidth = qmidictlMenuStyle::dpToPixels(48);
	styleSheetTemplate = QString(
	//	"* {background:lightGray}"
		"QToolButton {height: %3px; min-width: %2px}"
		"QToolButton QMenu::item {padding: %1px %1px %1px %1px; border: 1px solid transparent}"
		"QToolButton QMenu::indicator {image: none}"
		"QToolButton QMenu::item::selected {border-color: black}"
		"QToolButton#viewControl {font:bold}"
		"QToolButton::menu-indicator {image: none}").arg(paddingPixels).arg(minWidth);
	QStyle *style = QStyleFactory::create("Android");
	setStyle(new qmidictlMenuStyle(style));
	QScreen *screen = qApp->primaryScreen();
	QSizeF physicalSize = screen->physicalSize();
	if (qMax(physicalSize.width(), physicalSize.height()) > 145) {
        // Over 6.5" screen, this is a tablet
        setStyleSheet(styleSheetTemplate.arg(qmidictlMenuStyle::dpToPixels(56)));
    } else {
		// Phone, height changes when the screen is rotated
		screenGeometryChanged(screen->geometry());
		connect(screen, &QScreen::geometryChanged, this, &qmidictlActionBar::screenGeometryChanged);
	}

	// Create layout
	layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	layout->setMargin(0);
	layout->setContentsMargins(0,0,0,0);
	layout->setSizeConstraint(QLayout::SetNoConstraint);

	// App Icon, Up, and Navigation Button
	appIcon=new QToolButton();
	appIcon->setIcon(QIcon(":/images/qmidictl.png"));
	appIcon->setAutoRaise(true);
	appIcon->setFocusPolicy(Qt::NoFocus);
	appIcon->setPopupMode(QToolButton::InstantPopup);
	navigationMenu = new QMenu(appIcon);
	connect(navigationMenu, &QMenu::aboutToHide, this, &qmidictlActionBar::aboutToHideNavigationMenu);
	connect(navigationMenu, &QMenu::aboutToShow, this, &qmidictlActionBar::aboutToShowNavigationMenu);
	layout->addWidget(appIcon);

	// View Control Button
	viewControl=new QToolButton();
	viewControl->setObjectName("viewControl");
	viewControl->setText(QApplication::applicationDisplayName());
	viewControl->setAutoRaise(true);
	viewControl->setFocusPolicy(Qt::NoFocus);
	viewControl->setPopupMode(QToolButton::InstantPopup);
	viewMenu=new QMenu(viewControl);
	viewControl->setMenu(viewMenu);
	layout->addWidget(viewControl);

	// Spacer
	layout->addStretch(1);

	// Action Overflow Button
	overflowButton=new QToolButton();
	overflowButton->setIcon(QIcon(":/images/actionOverflow.png"));
	overflowButton->setToolTip(tr("more"));
	overflowButton->setAutoRaise(true);
	overflowButton->setFocusPolicy(Qt::NoFocus);
	overflowButton->setPopupMode(QToolButton::InstantPopup);
	overflowMenu=new QMenu(overflowButton);
	overflowButton->setMenu(overflowMenu);
	layout->addWidget(overflowButton);
}


qmidictlActionBar::~qmidictlActionBar (void)
{
}


void qmidictlActionBar::resizeEvent ( QResizeEvent* event )
{
	int oldWidth=event->oldSize().width();
	int newWidth=event->size().width();
	qDebug("resize from %i to %i",oldWidth,newWidth);
	if (oldWidth!=newWidth) {
		adjustContent();
	}
}


void qmidictlActionBar::paintEvent ( QPaintEvent * )
{
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


void qmidictlActionBar::screenGeometryChanged ( const QRect& geometry )
{
	if (geometry.height() > geometry.width()) {
		setStyleSheet(styleSheetTemplate.arg(qmidictlMenuStyle::dpToPixels(48)));
	} else {
		setStyleSheet(styleSheetTemplate.arg(qmidictlMenuStyle::dpToPixels(40)));
	}
}

void qmidictlActionBar::setTitle ( const QString& title, bool showUpButton )
{
	viewControl->setText(title);

	if (!navigationMenu->isEmpty()) {
		appIcon->setIcon(QIcon(":/images/actionMenu.png"));
		disconnect(appIcon, &QToolButton::clicked, this, &qmidictlActionBar::appIconClicked);
		appIcon->setMenu(navigationMenu);
	} else if (showUpButton) {
		appIcon->setIcon(QIcon(":/images/qmidictl.png"));
		appIcon->setToolTip(tr("up"));
		connect(appIcon, &QToolButton::clicked, this, &qmidictlActionBar::appIconClicked);
		appIcon->setMenu(0);
	} else {
		appIcon->setIcon(QIcon(":/images/qmidictl.png"));
		appIcon->setToolTip("");
		disconnect(appIcon, &QToolButton::clicked, this, &qmidictlActionBar::appIconClicked);
		appIcon->setMenu(0);
	}

	adjustContent();
}


QString qmidictlActionBar::getTitle (void)
{
	return viewControl->text();
}


void qmidictlActionBar::appIconClicked (void)
{
	emit up();
}


void qmidictlActionBar::adjustContent (void)
{
	int screenWidth = qApp->primaryScreen()->availableSize().width();

	if (!navigationMenu->isEmpty()) {
		appIcon->setIcon(QIcon(":/images/actionMenu.png"));
		disconnect(appIcon, &QToolButton::clicked, this, &qmidictlActionBar::appIconClicked);
		appIcon->setMenu(navigationMenu);
	}

	viewMenu->repaint();
	overflowButton->hide();

	// Check if all action buttons fit into the available space.
	for (int i = 0; i < buttonActions.size(); i++) {
		QAction *action = buttonActions.at(i);
		QToolButton *button = actionButtons.at(i);
		if (action->isVisible()) {
			button->show();
		}
	}

	if (sizeHint().width() > screenWidth) {
		// The buttons don't fit, we need an overflow menu.
		overflowButton->show();
		overflowMenu->clear();
	} else {
		overflowButton->hide();
	}

	// Show/Hide action buttons and overflow menu entries
	QAction *lastAction = 0;
	for (int i = buttonActions.size() - 1; i >= 0; i--) {
		QAction *action = buttonActions.at(i);
		QToolButton *button = actionButtons.at(i);
		if (action->isVisible()) {
			if (sizeHint().width() <= screenWidth) {
				// show as button
				button->show();
			} else {
				// show as overflow menu entry
				button->hide();
				overflowMenu->insertAction(lastAction, action);
				lastAction = action;
			}
		}
	}
}


void qmidictlActionBar::addMenuItem ( QAction *action )
{
	QWidget::addAction(action);
	navigationMenu->addAction(action);
	if (!navigationMenu->isEmpty()) {
		appIcon->setMenu(navigationMenu);
	}
}


void qmidictlActionBar::addMenuItems ( QList<QAction*> actions )
{
	QWidget::addActions(actions);
	for (int i = 0; i < actions.size(); i++) {
		addAction(actions.at(i));
	}
}


void qmidictlActionBar::removeMenuItem ( QAction *action )
{
	QWidget::removeAction(action);
	navigationMenu->removeAction(action);
	if (navigationMenu->isEmpty()) {
		appIcon->setMenu(0);
	}
}


void qmidictlActionBar::addView ( QAction *action )
{
	QWidget::addAction(action);
	viewMenu->addAction(action);
	if (!viewMenu->isEmpty()) {
		viewControl->setMenu(viewMenu);
	}
}


void qmidictlActionBar::addViews ( QList<QAction*> actions )
{
    QWidget::addActions(actions);
    for (int i=0; i<actions.size(); i++) {
        addAction(actions.at(i));
    }
}


void qmidictlActionBar::removeView ( QAction *action )
{
	QWidget::removeAction(action);
	viewMenu->removeAction(action);
	if (viewMenu->isEmpty()) {
		viewControl->setMenu(NULL);
	}
}


void qmidictlActionBar::addButton ( QAction* action, int position )
{
	if (position==-1) {
		position=buttonActions.size();
	}

	buttonActions.insert(position,action);
	QToolButton* button=new QToolButton();
	button->setText(action->text());
	button->setToolTip(action->text());
	QIcon icon = action->icon();
	if (!icon.isNull()) {
		button->setIcon(action->icon());
		button->setToolButtonStyle(Qt::ToolButtonIconOnly);
	}

	button->setEnabled(action->isEnabled());
	button->setFocusPolicy(Qt::NoFocus);
	button->setAutoRaise(true);
	connect(button, &QToolButton::clicked, action, &QAction::trigger);
	connect(action, &QAction::changed, this, &qmidictlActionBar::actionChanged);
	actionButtons.insert(position, button);
	layout->insertWidget(position + 3, button);
}


void qmidictlActionBar::removeButton ( QAction* action )
{
	QToolButton* button=NULL;
	for (int i=0; i<buttonActions.size(); i++) {
		if (buttonActions.at(i)==action) {
			button=actionButtons.at(i);
			break;
		}
	}
	if (button) {
		layout->removeWidget(button);
		actionButtons.removeOne(button);
		delete button;
		buttonActions.removeOne(action);
	}
}


void qmidictlActionBar::openOverflowMenu (void)
{
    if (overflowButton->isVisible()) {
        overflowButton->click();
    }
}


void qmidictlActionBar::actionChanged (void)
{
	QAction *action = qobject_cast<QAction *>(sender());
	int index = buttonActions.indexOf(action);
	actionButtons[index]->setEnabled(action->isEnabled());
	actionButtons[index]->setIcon(action->icon());
}


void qmidictlActionBar::aboutToHideNavigationMenu (void)
{
	appIcon->setIcon(QIcon(":/images/actionMenu.png"));
}


void qmidictlActionBar::aboutToShowNavigationMenu (void)
{
	appIcon->setIcon(QIcon(":/images/qmidictl.png"));
}


// end of qmidictlActionBar.cpp
