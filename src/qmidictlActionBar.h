// qmidictlActionBar.h
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

#ifndef __qmidictlActionBar_h
#define __qmidictlActionBar_h

/* This code is borrowed/adapted from actionbar.h
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

#include <QWidget>
#include <QIcon>


// Forward decls.
class QHBoxLayout;
class QToolButton;
class QLabel;
class QMenu;


/**
 * Toolbar similar to Android's Action Bar, can also be used on Desktop OS.
 * The action bar shows an icon, a title and any number of action buttons.
 * Also the title can have a menu of navigation actions.
 * <p>
 * If the buttons do not fit into the window, then they are displayed
 * as an "overflow" menu on the right side.
 * <p>
 * See http://developer.android.com/design/patterns/actionbar.html
 *
 * To be used within a vertical box layout this way:
 * <pre><code>
 * MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
 *     QVBoxLayout *layout = new QVBoxLayout(this);
 *     layout->setMargin(0);
 *     layout->setSizeConstraint(QLayout::SetNoConstraint);
 *
 *     // Action bar
 *     ActionBar *actionBar=new ActionBar(this);
 *     actionBar->setTitle("My App",false);
 *     actionBar->addMenuItem(new QAction("News",this));
 *     actionBar->addMenuItem(new QAction("Weather",this));
 *     actionBar->addButton(new QAction(QIcon(":icons/search"),"Search",this));
 *     actionBar->addButton(new QAction(QIcon(":icons/call"),"Call",this));
 *     actionBar->addButton(new QAction(QIcon(":icons/settings"),"Settings",this));
 *     layout->addWidget(actionBar);
 *
 *     // Content of main window below the action bar
 *     layout->addWidget(new QLabel("Hello",this));
 *     layout->addStretch();
 * }
 * </code></pre>
 * The layout of the main window must use the option QLayout::SetNoConstraint,
 * to support screen rotation on mpbile devices.
 * <p>
 * The action bar needs two icons in the resource file:
 * <ul>
 *     <li>QIcon(":icons/app") is used for the initial display.
 *     <li>QIcon(":icons/app_up") is used when you enable "up" navigation by calling setTitle().
 * </ul>
 */

class qmidictlActionBar : public QWidget
{
	Q_OBJECT

public:
	/**
	 * Constructor, creates an Activity bar with icon and title but no action buttons.
	 * The icon is loaded from the resource file via QIcon(":icons/app").
	 * The title is taken from QApplication::applicationDisplayName().
	 *
	 * @param parent Points to the parent window.
	 */
	explicit qmidictlActionBar(QWidget *parent = 0);

	/**
	 * Destructor.
	 */
	~qmidictlActionBar();

	/**
	 * Adds an item to the navigation menu of the action bar.
	 * @param action The action, containing at least a text and optionally an icon. The action emits signal triggered() when clicked.
	 */
	void addMenuItem(QAction *action);

	/**
	 * Adds many items to the navigation menu of the action bar.
	 * @param actions List of actions.
	 * @see addAction()
	 */
	void addMenuItems(QList<QAction *> actions);

	/**
	 * Removes an item from the navigation menu of the action bar.
	 * @param action The action that had been added before.
	 */
	void removeMenuItem(QAction *action);

	/**
	 * Set icon of the action bar.
	 * @param icon Either the icon of the application or the current view within the application.
	 */
	void setIcon(const QIcon& icon);

	/** Get the current icon of the action bar. */
	const QIcon& icon() const;

	/**
	 * Set title of the action bar.
	 * @param title Either the name of the application or title of the current view within the application.
	 * @param showUpButton Enables "up" navigation. Then the action bar emits signal up() on click on the icon.
	 */
	void setTitle(const QString& title);

	/** Get the current title of the action bar. */
	QString title() const;

	/**
	 * Adds an action button (or overflow menu item) to the action bar.
	 * @param action The action, containing at least a text and optinally an icon. The action emits signal triggered() when clicked.
	 * @param position Insert before this position. 0=before first button, 1=second button. Default is -1=append to the end.
	 */
	void addButton(QAction *action, int position = -1);

	/**
	 * Removes an action button (or overflow menu item) from the action bar.
	 * @param action The action that had been added before.
	 */
	void removeButton(QAction *action);

	/**
	 * Adjust the number of buttons in the action bar. Buttons that don't fit go into the overflow menu.
	 * You need to call this method after adding, removing or changing the visibility of an action.
	 */
	void adjustContent();

public slots:
	/** Can be used to open the overflow menu */
	void openOverflowMenu();

protected:
	/**
	 * Overrides the paintEvent method to draw background color properly.
	 */
	void paintEvent(QPaintEvent* event);

	/**
	 * Overrides the resizeEvent to adjust the content depending on the new size.
	 */
	void resizeEvent(QResizeEvent* event);

protected slots:
	/** Listener for changes in action enabled/disabled status */
	void actionChanged();

	/** Listener for navigation menu open signals */
	void aboutToShowAppMenu();

	/** Listener for navigation menu close signals */
	void aboutToHideAppMenu();

private:
	/** Horizontal layout of the navigation bar */
	QHBoxLayout *m_layout;

	/** The application or current view icon. */
	QIcon m_appIcon;

	/** The Button that contains the applications icon, used for "up" navigation. */
	QToolButton *m_appButton;

	/** The menu that appears when the user clicks on the navigation menu button */
	QMenu *m_appMenu;

	/** The label that contains the title. */
	QLabel *m_appTitle;

	/** List of actions for the action buttons and overflow menu. */
	QList<QAction *> m_buttonActions;

	/** List of action buttons, same order as buttonActions. */
	QList<QToolButton *> m_actionButtons;

	/** Overflow button, is only visible when there are more buttons than available space. */
	QToolButton *m_overflowButton;

	/** The menu that appears when the user clicks on the overflow button. */
	QMenu *m_overflowMenu;
};


#endif	// __qmidictlActionBar_h

// end of qmidictlActionBar.h
