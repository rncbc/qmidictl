// qmidictlMidiControlForm.h
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

#ifndef __qmidictlMidiControlForm_h
#define __qmidictlMidiControlForm_h

#include "ui_qmidictlMidiControlForm.h"


// Forward declarations.
#if defined(Q_OS_ANDROID)
class qmidictlActionBar;
class QAction;
#endif


//----------------------------------------------------------------------------
// qmidictlMidiControlForm -- UI wrapper form.

class qmidictlMidiControlForm : public QDialog
{
	Q_OBJECT

public:

	// Constructor.
	qmidictlMidiControlForm(QWidget *pParent = nullptr);

	// Destructor.
	~qmidictlMidiControlForm();

protected slots:

	void activateCommand(int);
	void activateControlType(int);
	void change();

	void buttonClick(QAbstractButton *);

	void accept();
	void reject();
	void reset();

private:

	// The Qt-designer UI struct...
	Ui::qmidictlMidiControlForm m_ui;

	// Instance variables.
	int m_iDirtyCount;
	int m_iDirtySetup;

#if defined(Q_OS_ANDROID)
	// Special action-bar for the android stuff.
	qmidictlActionBar *m_pActionBar;

	QAction *m_pBackAction;
	QAction *m_pResetAction;
	QAction *m_pAcceptAction;
	QAction *m_pCancelAction;
#endif
};


#endif	// __qmidictlMidiControlForm_h


// end of qmidictlMidiControlForm.h
