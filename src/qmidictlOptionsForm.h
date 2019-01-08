// qmidictlOptionsForm.h
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

#ifndef __qmidictlOptionsForm_h
#define __qmidictlOptionsForm_h

#include "ui_qmidictlOptionsForm.h"

// Forward declarations.
#if defined(Q_OS_ANDROID)
class qmidictlActionBar;
class QAction;
#endif


//----------------------------------------------------------------------------
// qmidictlOptionsForm -- UI wrapper form.

class qmidictlOptionsForm : public QDialog
{
	Q_OBJECT

public:

	// Constructor.
	qmidictlOptionsForm(QWidget *pParent = 0, Qt::WindowFlags wflags = 0);

	// Destructor.
	~qmidictlOptionsForm();

protected slots:

	void change();

	void buttonClick(QAbstractButton *);

	void accept();
	void reject();
	void reset();

private:

	// The Qt-designer UI struct...
	Ui::qmidictlOptionsForm m_ui;

	// Instance variables.
	int m_iDirtyCount;

	// Default (translatable) interface name.
	QString m_sDefInterface;

#if defined(Q_OS_ANDROID)
	// Special action-bar for the android stuff.
	qmidictlActionBar *m_pActionBar;

	QAction *m_pBackAction;
	QAction *m_pResetAction;
	QAction *m_pAcceptAction;
	QAction *m_pCancelAction;
#endif
};


#endif	// __qmidictlOptionsForm_h


// end of qmidictlOptionsForm.h
