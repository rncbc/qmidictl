// qmidictlMixerStrip.cpp
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

#include "qmidictlMixerStrip.h"


//----------------------------------------------------------------------------
// qmidictlMixerStrip -- UI wrapper form.
//

// Constructor.
qmidictlMixerStrip::qmidictlMixerStrip ( QWidget *pParent )
	: QWidget(pParent)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Strip number.
	m_iStrip = 0;
	m_iBusy  = 0;

#if defined(Q_OS_ANDROID)
	// Specific R/M/S color overlays...
	QPalette pal = QWidget::palette();
	pal.setColor(QPalette::HighlightedText, Qt::black);
	pal.setColor(QPalette::Highlight, Qt::red);
	m_ui.recordButton->setPalette(pal);
	pal.setColor(QPalette::Highlight, Qt::yellow);
	m_ui.muteButton->setPalette(pal);
	pal.setColor(QPalette::Highlight, Qt::cyan);
	m_ui.soloButton->setPalette(pal);
#endif

#if defined(Q_OS_ANDROID)
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
	m_ui.recordButton->setMaximumWidth(48);
	m_ui.muteButton->setMaximumWidth(48);
	m_ui.soloButton->setMaximumWidth(48);
	m_ui.stripSlider->setMinimumWidth(48);
#endif
#endif

	// UI widgets signal/slot connections...
	QObject::connect(m_ui.recordButton,
		SIGNAL(toggled(bool)),
		SLOT(recordSlot(bool)));
	QObject::connect(m_ui.muteButton,
		SIGNAL(toggled(bool)),
		SLOT(muteSlot(bool)));
	QObject::connect(m_ui.soloButton,
		SIGNAL(toggled(bool)),
		SLOT(soloSlot(bool)));
	QObject::connect(m_ui.stripSlider,
		SIGNAL(valueChanged(int)),
		SLOT(sliderSlot(int)));
}


// Destructor.
qmidictlMixerStrip::~qmidictlMixerStrip (void)
{
}


// Mixer strip accessors.
void qmidictlMixerStrip::setStrip (
	int iStrip, bool bRecord, bool bMute, bool bSolo, int iSlider )
{
	m_iBusy++;

	m_iStrip = iStrip;

	m_ui.stripLabel->setText(QString::number(m_iStrip + 1));

	m_ui.recordButton->setChecked(bRecord);
	m_ui.muteButton->setChecked(bMute);
	m_ui.soloButton->setChecked(bSolo);
	m_ui.stripSlider->setValue(iSlider);

	m_iBusy--;
}

int qmidictlMixerStrip::strip (void) const
{
	return m_iStrip;
}

bool qmidictlMixerStrip::isRecord (void) const
{
	return m_ui.recordButton->isChecked();
}

bool qmidictlMixerStrip::isMute (void) const
{
	return m_ui.muteButton->isChecked();
}

bool qmidictlMixerStrip::isSolo (void) const
{
	return m_ui.soloButton->isChecked();
}

int qmidictlMixerStrip::slider (void) const
{
	return m_ui.stripSlider->value();
}


// Mixer strip slots.
void qmidictlMixerStrip::recordSlot ( bool bOn )
{
	if (m_iBusy > 0)
		return;

	emit recordToggled(m_iStrip, bOn);
}


void qmidictlMixerStrip::muteSlot ( bool bOn )
{
	if (m_iBusy > 0)
		return;

	emit muteToggled(m_iStrip, bOn);
}


void qmidictlMixerStrip::soloSlot ( bool bOn )
{
	if (m_iBusy > 0)
		return;

	emit soloToggled(m_iStrip, bOn);
}


void qmidictlMixerStrip::sliderSlot ( int iValue )
{
	if (m_iBusy > 0)
		return;

	emit sliderChanged(m_iStrip, iValue);
}


// end of qmidictlMixerStrip.cpp
