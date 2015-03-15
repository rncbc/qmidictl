// qmidictlOptionsForm.cpp
//
/****************************************************************************
   Copyright (C) 2010-2015, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include "qmidictlOptionsForm.h"

#include "qmidictlOptions.h"

#include <QMessageBox>


//----------------------------------------------------------------------------
// qmidictlOptionsForm -- UI wrapper form.

// Constructor.
qmidictlOptionsForm::qmidictlOptionsForm (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QDialog(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Initialize the dialog widgets with deafult settings...
	m_sDefInterface = tr("(Any)");
	m_ui.InterfaceComboBox->clear();
	m_ui.InterfaceComboBox->addItem(m_sDefInterface);
	m_ui.InterfaceComboBox->addItem("wlan0");
	m_ui.InterfaceComboBox->addItem("eth0");

	m_ui.UdpAddrComboBox->clear();
	m_ui.UdpAddrComboBox->addItem(QMIDICTL_UDP_ADDR);

	m_ui.UdpPortSpinBox->setValue(QMIDICTL_UDP_PORT);

	// Populate dialog widgets with current settings...
	qmidictlOptions *pOptions = qmidictlOptions::getInstance();
	if (pOptions) {
		if (pOptions->sInterface.isEmpty())
			m_ui.InterfaceComboBox->setCurrentIndex(0);
		else
			m_ui.InterfaceComboBox->setEditText(pOptions->sInterface);
		if (pOptions->sUdpAddr.isEmpty())
			m_ui.UdpAddrComboBox->setCurrentIndex(0);
		else
			m_ui.UdpAddrComboBox->setEditText(pOptions->sUdpAddr);
		m_ui.UdpPortSpinBox->setValue(pOptions->iUdpPort);
		m_ui.MmcDeviceSpinBox->setValue(pOptions->iMmcDevice);
	}

#if defined(Q_OS_SYMBIAN) || defined(Q_OS_WINDOWS)
        m_ui.InterfaceComboBox->setEnabled(false);
#endif

	// Start clean.
	m_iDirtyCount = 0;

	// Try to fix window geometry.
	adjustSize();

	// UI signal/slot connections...
	QObject::connect(m_ui.InterfaceComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(change()));
	QObject::connect(m_ui.UdpAddrComboBox,
		SIGNAL(editTextChanged(const QString&)),
		SLOT(change()));
	QObject::connect(m_ui.UdpPortSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(change()));
	QObject::connect(m_ui.MmcDeviceSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(change()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
		SLOT(reject()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(clicked(QAbstractButton *)),
		SLOT(buttonClick(QAbstractButton *)));
}


// Change settings (anything else slot).
void qmidictlOptionsForm::change (void)
{
	m_iDirtyCount++;
}


// Accept settings (OK button slot).
void qmidictlOptionsForm::accept (void)
{
	// Save options...
	if (m_iDirtyCount > 0) {
		qmidictlOptions *pOptions = qmidictlOptions::getInstance();
		if (pOptions) {
			// Display options...
			pOptions->sInterface = m_ui.InterfaceComboBox->currentText();
			pOptions->sUdpAddr   = m_ui.UdpAddrComboBox->currentText();
			pOptions->iUdpPort   = m_ui.UdpPortSpinBox->value();
			pOptions->iMmcDevice = m_ui.MmcDeviceSpinBox->value();
			// Take care of some translatable adjustments...
			if (pOptions->sInterface == m_sDefInterface)
				pOptions->sInterface.clear();
			// Save/commit to disk.
			pOptions->saveOptions();
			// Clean all dirt...
			m_iDirtyCount = 0;
		}
	}

	// Just go with dialog acceptance
	QDialog::accept();
}


// Reject options (Cancel button slot).
void qmidictlOptionsForm::reject (void)
{
	// Check if there's any pending changes...
	if (m_iDirtyCount > 0) {
		switch (QMessageBox::warning(this,
			QDialog::windowTitle(),
			tr("Some settings have been changed.\n\n"
			"Do you want to apply the changes?"),
			QMessageBox::Apply |
			QMessageBox::Discard |
			QMessageBox::Cancel)) {
		case QMessageBox::Discard:
			break;
		case QMessageBox::Apply:
			accept();
		default:
			return;
		}
	}

	// Just go with dialog rejection...
	QDialog::reject();
}


// Reset options (generic button slot).
void qmidictlOptionsForm::buttonClick ( QAbstractButton *pButton )
{
	const QDialogButtonBox::ButtonRole buttonRole
		= m_ui.DialogButtonBox->buttonRole(pButton);
	if (buttonRole == QDialogButtonBox::ResetRole) {
		m_ui.InterfaceComboBox->setCurrentIndex(0);
		m_ui.UdpAddrComboBox->setCurrentIndex(0);
		m_ui.UdpPortSpinBox->setValue(QMIDICTL_UDP_PORT);
	}
}


// end of qmidictlOptionsForm.cpp

