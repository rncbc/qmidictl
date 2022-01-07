// qmidictlMixerStrip.h
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

#ifndef __qmidictlMixerStrip_h
#define __qmidictlMixerStrip_h

#include "ui_qmidictlMixerStrip.h"


//----------------------------------------------------------------------------
// qmidictlMixerStrip -- UI wrapper form.

class qmidictlMixerStrip : public QWidget
{
	Q_OBJECT

public:

	// Constructor.
	qmidictlMixerStrip(QWidget *pParent = nullptr);
	// Destructor.
	~qmidictlMixerStrip();

	// Strip accessors.
	void setStrip(int iStrip,
		bool bRecord, bool bMute, bool bSolo, int iSlider);

	int  strip()    const;
	bool isRecord() const;
	bool isMute()   const;
	bool isSolo()   const;
	int  slider()   const;

signals:

	// Strip signals.
	void recordToggled(int iStrip, bool bOn);
	void muteToggled(int iStrip, bool bOn);
	void soloToggled(int iStrip, bool bOn);
	void sliderChanged(int iStrip, int iValue);

protected slots:

	// Strip slots.
	void recordSlot(bool bOn);
	void muteSlot(bool bOn);
	void soloSlot(bool bOn);
	void sliderSlot(int iValue);

private:

	// The Qt-designer UI struct...
	Ui::qmidictlMixerStrip m_ui;

	// Current mixer strip page.
	int m_iStrip;
	int m_iBusy;
};


#endif	// __qmidictlMixerStrip_h


// end of qmidictlMixerStrip.h
