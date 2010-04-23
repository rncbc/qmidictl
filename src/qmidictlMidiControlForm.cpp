// qmidictlMidiControlForm.cpp
//
/****************************************************************************
   Copyright (C) 2010, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "qmidictlAbout.h"
#include "qmidictlMidiControlForm.h"
#include "qmidictlMidiControl.h"

#include "qmidictlOptions.h"

#include <QMessageBox>


//----------------------------------------------------------------------------
// qmidictlMidiControlForm -- UI wrapper form.

// Constructor.
qmidictlMidiControlForm::qmidictlMidiControlForm (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QDialog(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Populate command list.
	m_ui.CommandComboBox->addItem(
		qmidictlMidiControl::textFromCommand(qmidictlMidiControl::RST));
	m_ui.CommandComboBox->addItem(
		qmidictlMidiControl::textFromCommand(qmidictlMidiControl::REW));
	m_ui.CommandComboBox->addItem(
		qmidictlMidiControl::textFromCommand(qmidictlMidiControl::STOP));
	m_ui.CommandComboBox->addItem(
		qmidictlMidiControl::textFromCommand(qmidictlMidiControl::PLAY));
	m_ui.CommandComboBox->addItem(
		qmidictlMidiControl::textFromCommand(qmidictlMidiControl::REC));
	m_ui.CommandComboBox->addItem(
		qmidictlMidiControl::textFromCommand(qmidictlMidiControl::FFWD));
	m_ui.CommandComboBox->addItem(
		qmidictlMidiControl::textFromCommand(qmidictlMidiControl::JOG_WHEEL));
	m_ui.CommandComboBox->addItem(
		qmidictlMidiControl::textFromCommand(qmidictlMidiControl::TRACK_SOLO));
	m_ui.CommandComboBox->addItem(
		qmidictlMidiControl::textFromCommand(qmidictlMidiControl::TRACK_MUTE));
	m_ui.CommandComboBox->addItem(
		qmidictlMidiControl::textFromCommand(qmidictlMidiControl::TRACK_REC));
	m_ui.CommandComboBox->addItem(
		qmidictlMidiControl::textFromCommand(qmidictlMidiControl::TRACK_VOL));

	// Populate command list.
	m_ui.ControlTypeComboBox->addItem(
		qmidictlMidiControl::textFromType(qmidictlMidiControl::MMC));
	m_ui.ControlTypeComboBox->addItem(
		qmidictlMidiControl::textFromType(qmidictlMidiControl::NOTE_ON));
	m_ui.ControlTypeComboBox->addItem(
		qmidictlMidiControl::textFromType(qmidictlMidiControl::NOTE_OFF));
	m_ui.ControlTypeComboBox->addItem(
		qmidictlMidiControl::textFromType(qmidictlMidiControl::KEY_PRESS));
	m_ui.ControlTypeComboBox->addItem(
		qmidictlMidiControl::textFromType(qmidictlMidiControl::CONTROLLER));
	m_ui.ControlTypeComboBox->addItem(
		qmidictlMidiControl::textFromType(qmidictlMidiControl::PGM_CHANGE));
	m_ui.ControlTypeComboBox->addItem(
		qmidictlMidiControl::textFromType(qmidictlMidiControl::CHAN_PRESS));
	m_ui.ControlTypeComboBox->addItem(
		qmidictlMidiControl::textFromType(qmidictlMidiControl::PITCH_BEND));

	// Populate param list.
	activateCommand(m_ui.CommandComboBox->currentText());

	// Start clean.
	m_iDirtyCount = 0;

	// Try to fix window geometry.
	adjustSize();

	// UI signal/slot connections...
	QObject::connect(m_ui.CommandComboBox,
		SIGNAL(activated(const QString&)),
		SLOT(activateCommand(const QString&)));
	QObject::connect(m_ui.ControlTypeComboBox,
		SIGNAL(activated(const QString&)),
		SLOT(activateControlType(const QString&)));
	QObject::connect(m_ui.ChannelSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(change()));
	QObject::connect(m_ui.ParamComboBox,
		SIGNAL(activated(int)),
		SLOT(change()));
	QObject::connect(m_ui.ParamTrackCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(change()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(clicked(QAbstractButton *)),
		SLOT(click(QAbstractButton *)));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
		SLOT(reject()));
}


// List view item activation.
void qmidictlMidiControlForm::activateCommand (
	const QString& sCommand )
{
#ifdef CONFIG_DEBUG
	qDebug("qmidictlMidiControlForm::activateCommand(\"%s\")",
		sCommand.toUtf8().constData());
#endif

	qmidictlMidiControl::Command command
		= qmidictlMidiControl::commandFromText(sCommand);
	bool bEnabled = (command != qmidictlMidiControl::Command(0));

	qmidictlMidiControl *pMidiControl = qmidictlMidiControl::getInstance();
	if (bEnabled)
		bEnabled = (pMidiControl != NULL);

	m_ui.ControlTypeTextLabel->setEnabled(bEnabled);
	m_ui.ControlTypeComboBox->setEnabled(bEnabled);

	if (bEnabled)
		bEnabled = pMidiControl->isCommandMapped(command);

	m_ui.ParamTextLabel->setEnabled(bEnabled);
	m_ui.ParamComboBox->setEnabled(bEnabled);
	m_ui.ChannelTextLabel->setEnabled(bEnabled);
	m_ui.ChannelSpinBox->setEnabled(bEnabled);
	m_ui.ParamTextLabel->setEnabled(bEnabled);
	m_ui.ParamComboBox->setEnabled(bEnabled);
	m_ui.ParamTrackCheckBox->setEnabled(bEnabled);

	if (bEnabled) {
		const qmidictlMidiControl::MapKey& key
			= pMidiControl->commandMap().value(command);
		const QString& sControlType
			= qmidictlMidiControl::textFromType(key.type());
		m_ui.ControlTypeComboBox->setCurrentIndex(
			m_ui.ControlTypeComboBox->findText(sControlType));
		activateControlTypeEx(sControlType);
		int iChannel = 0;
		if (m_ui.ChannelSpinBox->isEnabled())
			iChannel = (key.isChannelTrack() ? 0 : key.channel() + 1);
		m_ui.ChannelSpinBox->setValue(iChannel);
		int iParam = key.param();
		if (key.isParamTrack())
			iParam &= qmidictlMidiControl::TrackParamMask;
		m_ui.ParamComboBox->setCurrentIndex(iParam);
		if (key.isChannelTrack()) {
			m_ui.ParamTrackCheckBox->setEnabled(false);
			m_ui.ParamTrackCheckBox->setChecked(false);
		} else {
			m_ui.ParamTrackCheckBox->setChecked(key.isParamTrack());
		}
	} else {
		m_ui.ControlTypeComboBox->setCurrentIndex(0);
		m_ui.ChannelSpinBox->setValue(0);
		m_ui.ParamComboBox->clear();
		m_ui.ParamTrackCheckBox->setChecked(false);
	}
}


void qmidictlMidiControlForm::activateControlType (
	const QString& sControlType )
{
#ifdef CONFIG_DEBUG
	qDebug("qmidictlMidiControlForm::activateControlType(\"%s\")",
		sControlType.toUtf8().constData());
#endif

	activateControlTypeEx(sControlType);
	change();
}


void qmidictlMidiControlForm::activateControlTypeEx (
	const QString& sControlType )
{
#ifdef CONFIG_DEBUG
	qDebug("qmidictlMidiControlForm::activateControlTypeEx(\"%s\")",
		sControlType.toUtf8().constData());
#endif

	qmidictlMidiControl::ControlType ctype
		= qmidictlMidiControl::typeFromText(sControlType);
	if (!ctype)
		return;

	m_ui.ParamComboBox->clear();
	switch (ctype) {
	case qmidictlMidiControl::MMC:
	case qmidictlMidiControl::CHAN_PRESS:
	case qmidictlMidiControl::PITCH_BEND:
		m_ui.ChannelTextLabel->setEnabled(false);
		m_ui.ChannelSpinBox->setEnabled(false);
		m_ui.ParamTextLabel->setEnabled(false);
		m_ui.ParamComboBox->setEnabled(false);
		m_ui.ParamTrackCheckBox->setEnabled(false);
		break;
	case qmidictlMidiControl::NOTE_ON:
	case qmidictlMidiControl::NOTE_OFF:
	case qmidictlMidiControl::CONTROLLER:
	case qmidictlMidiControl::KEY_PRESS:
	case qmidictlMidiControl::PGM_CHANGE:
		m_ui.ChannelTextLabel->setEnabled(true);
		m_ui.ChannelSpinBox->setEnabled(true);
		m_ui.ParamTextLabel->setEnabled(true);
		m_ui.ParamComboBox->setEnabled(true);
		for (int i = 0; i < 128; ++i)
			m_ui.ParamComboBox->addItem(QString::number(i));
		if (m_ui.ChannelSpinBox->value() == 0) {
			m_ui.ParamTrackCheckBox->setEnabled(false);
			m_ui.ParamTrackCheckBox->setChecked(false);
		} else {
			m_ui.ParamTrackCheckBox->setEnabled(true);
		}
		break;
	}
}


// Change settings (OK button slot).
void qmidictlMidiControlForm::change (void)
{
#ifdef CONFIG_DEBUG
	qDebug("qmidictlMidiControlForm::change()");
#endif

	qmidictlMidiControl *pMidiControl = qmidictlMidiControl::getInstance();
	if (pMidiControl == NULL)
		return;

	const QString& sCommand = m_ui.CommandComboBox->currentText();
	qmidictlMidiControl::Command command
		= qmidictlMidiControl::commandFromText(sCommand);
	if (!command)
		return;

	const QString& sControlType = m_ui.ControlTypeComboBox->currentText();
	qmidictlMidiControl::ControlType ctype
		= qmidictlMidiControl::typeFromText(sControlType);
	if (!ctype)
		return;

	unsigned short iChannel = 0;
	if (m_ui.ChannelSpinBox->isEnabled()) {
		iChannel = m_ui.ChannelSpinBox->value();
		if (iChannel == 0) {
			iChannel |= qmidictlMidiControl::TrackParam;
			m_ui.ParamTrackCheckBox->setEnabled(false);
			m_ui.ParamTrackCheckBox->setChecked(false);
		} else {
			iChannel--;
			m_ui.ParamTrackCheckBox->setEnabled(true);
		}
	}

	unsigned short iParam = 0;
	if (m_ui.ParamComboBox->isEnabled()) {
		iParam = m_ui.ParamComboBox->currentIndex();
		if (m_ui.ParamTrackCheckBox->isEnabled() &&
			m_ui.ParamTrackCheckBox->isChecked())
			iParam |= qmidictlMidiControl::TrackParam;
	}

	pMidiControl->unmapCommand(command);
	pMidiControl->mapCommand(command, ctype, iChannel, iParam);

	m_iDirtyCount++;
}


// Reset settings (Reset button slot).
void qmidictlMidiControlForm::reset (void)
{
#ifdef CONFIG_DEBUG
	qDebug("qmidictlMidiControlForm::reset()");
#endif

	qmidictlMidiControl *pMidiControl = qmidictlMidiControl::getInstance();
	if (pMidiControl) {
		pMidiControl->clear();
		m_iDirtyCount++;
	}

	activateCommand(m_ui.CommandComboBox->currentText());
}


// Reset settings (action button slot).
void qmidictlMidiControlForm::click ( QAbstractButton *pButton )
{
#ifdef CONFIG_DEBUG
	qDebug("qmidictlMidiControlForm::buttonClick(%p)", pButton);
#endif

	if (m_ui.DialogButtonBox->buttonRole(pButton) & QDialogButtonBox::ResetRole)
		reset();
}


// Accept settings (OK button slot).
void qmidictlMidiControlForm::accept (void)
{
#ifdef CONFIG_DEBUG
	qDebug("qmidictlMidiControlForm::accept()");
#endif

	// Save settings...
	if (m_iDirtyCount > 0) {
		qmidictlMidiControl *pMidiControl = qmidictlMidiControl::getInstance();
		if (pMidiControl) {
			qmidictlOptions *pOptions = qmidictlOptions::getInstance();
			if (pOptions) {
				pMidiControl->save(pOptions->settings());
				m_iDirtyCount = 0;
			}
		}
	}

	// Just go with dialog acceptance...
	QDialog::accept();
}


// Reject settings (Cancel button slot).
void qmidictlMidiControlForm::reject (void)
{
#ifdef CONFIG_DEBUG
	qDebug("qmidictlMidiControlForm::reject()");
#endif

	// Reload settings...
	if (m_iDirtyCount > 0) {
		qmidictlMidiControl *pMidiControl = qmidictlMidiControl::getInstance();
		if (pMidiControl) {
			qmidictlOptions *pOptions = qmidictlOptions::getInstance();
			if (pOptions) {
				pMidiControl->load(pOptions->settings());
				m_iDirtyCount = 0;
			}
		}
	}

	// Just go with dialog rejection...
	QDialog::reject();
}


// end of qmidictlMidiControlForm.cpp

