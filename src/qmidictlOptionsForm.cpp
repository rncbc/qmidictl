// qmidictlOptionsForm.cpp
//
/****************************************************************************
   Copyright (C) 2010-2023, rncbc aka Rui Nuno Capela. All rights reserved.

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

#if defined(CONFIG_IPV6)
#include <QNetworkInterface>
#endif

#if defined(Q_OS_ANDROID)
#include "qmidictlActionBar.h"
#include <QAction>
#include <QLineEdit>
#endif


//----------------------------------------------------------------------------
// qmidictlOptionsForm -- UI wrapper form.

// Constructor.
qmidictlOptionsForm::qmidictlOptionsForm ( QWidget *pParent )
	: QDialog(pParent)
{
	// Setup UI struct...
	m_ui.setupUi(this);
#if QT_VERSION < QT_VERSION_CHECK(6, 1, 0)
	QDialog::setWindowIcon(QIcon(":/images/qmidictl.png"));
#endif

#if defined(Q_OS_ANDROID)

	// Custom dialog font size...
	const QFont& font = QDialog::font();
	QDialog::setFont(QFont(font.family(), font.pointSize() + 1));

	// Special actions for the android stuff.
	m_pBackAction = new QAction(QIcon(":/images/actionBack.png"), tr("Back"),  this);
	m_pResetAction = new QAction(QIcon(":/images/actionReset.png"), tr("Reset"),  this);
	m_pAcceptAction = new QAction(QIcon(":/images/actionAccept.png"), tr("Done"),  this);
	m_pCancelAction = new QAction(QIcon(":/images/actionCancel.png"), tr("Cancel"), this);

	QObject::connect(m_pBackAction, SIGNAL(triggered(bool)), this, SLOT(reject()));
	QObject::connect(m_pResetAction, SIGNAL(triggered(bool)), this, SLOT(reset()));
	QObject::connect(m_pAcceptAction, SIGNAL(triggered(bool)), this, SLOT(accept()));
	QObject::connect(m_pCancelAction, SIGNAL(triggered(bool)), this, SLOT(reject()));

	// Special action-bar for the android stuff.
	m_pActionBar = new qmidictlActionBar();
	m_pActionBar->setIcon(QDialog::windowIcon());
	m_pActionBar->setTitle(QDialog::windowTitle());
	// Action-bar back-button...
	m_pActionBar->addMenuItem(m_pBackAction);
	// Action-bar right-overflow button items...
	m_pActionBar->addButton(m_pResetAction);
	m_pActionBar->addButton(m_pAcceptAction);
	m_pActionBar->addButton(m_pCancelAction);
	// Make it at the top...
	m_ui.MainCentralLayout->insertWidget(0, m_pActionBar);

	m_ui.DialogButtonBox->hide();

#endif

	// Initialize the dialog widgets with deafult settings...
	m_sDefInterface = tr("(Any)");
	m_ui.InterfaceComboBox->clear();
	m_ui.InterfaceComboBox->addItem(m_sDefInterface);
#if defined(CONFIG_IPV6)
	foreach (const QNetworkInterface& iface, QNetworkInterface::allInterfaces()) {
		if (iface.isValid() &&
			iface.flags().testFlag(QNetworkInterface::CanMulticast) &&
			iface.flags().testFlag(QNetworkInterface::IsUp) &&
			iface.flags().testFlag(QNetworkInterface::IsRunning) &&
			!iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
			m_ui.InterfaceComboBox->addItem(iface.name());
		}
	}
#else
	m_ui.InterfaceComboBox->addItem("wlan0");
	m_ui.InterfaceComboBox->addItem("eth0");
#endif
#if defined(Q_OS_ANDROID)
#if QT_VERSION < QT_VERSION_CHECK(6, 1, 0)
//	m_ui.InterfaceComboBox->setMinimumWidth(240);
	m_ui.InterfaceComboBox->setMinimumHeight(128);
	m_ui.InterfaceComboBox->lineEdit()->setMinimumHeight(96);
#endif
#endif

	m_ui.UdpAddrComboBox->clear();
	m_ui.UdpAddrComboBox->addItem(QMIDICTL_UDP_IPV4_ADDR);
#if defined(CONFIG_IPV6)
	m_ui.UdpAddrComboBox->addItem(QMIDICTL_UDP_IPV6_ADDR);
#endif
#if defined(Q_OS_ANDROID)
#if QT_VERSION < QT_VERSION_CHECK(6, 1, 0)
//	m_ui.UdpAddrComboBox->setMinimumWidth(240);
	m_ui.UdpAddrComboBox->setMinimumHeight(128);
	m_ui.UdpAddrComboBox->lineEdit()->setMinimumHeight(96);
#endif
#endif

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
#if defined(Q_OS_ANDROID) || defined(Q_OS_SYMBIAN)
	showMaximized();
#else
	adjustSize();
#endif

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


// Destructor.
qmidictlOptionsForm::~qmidictlOptionsForm (void)
{
#if defined(Q_OS_ANDROID)
	// No need for special android stuff anymore.
	delete m_pCancelAction;
	delete m_pAcceptAction;
	delete m_pResetAction;
	delete m_pBackAction;
	delete m_pActionBar;
#endif
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



// Reset settings (action button slot).
void qmidictlOptionsForm::buttonClick ( QAbstractButton *pButton )
{
#ifdef CONFIG_DEBUG
	qDebug("qmidictlOptionsForm::buttonClick(%p)", pButton);
#endif

	QDialogButtonBox::ButtonRole role
		= m_ui.DialogButtonBox->buttonRole(pButton);
	if ((role & QDialogButtonBox::ResetRole) == QDialogButtonBox::ResetRole)
		reset();
}


// Reset options (generic button slot).
void qmidictlOptionsForm::reset (void)
{
#ifdef CONFIG_DEBUG
	qDebug("qmidictlOptionsForm::reset()");
#endif

	if (QMessageBox::warning(this,
		QDialog::windowTitle(),
		tr("All settings will be reset to the original default.\n\n"
		"Are you sure to apply the changes?"),
		QMessageBox::Reset |
		QMessageBox::Cancel) == QMessageBox::Cancel)
		return;

	m_ui.MmcDeviceSpinBox->setValue(QMIDICTL_MMC_DEVICE);

	m_ui.InterfaceComboBox->setCurrentIndex(0);
#if defined(CONFIG_IPV6)
	const QString& sUdpAddr
		= m_ui.UdpAddrComboBox->currentText();
	QHostAddress addr;
	if (addr.setAddress(sUdpAddr) &&
		addr.protocol() == QAbstractSocket::IPv6Protocol)
		m_ui.UdpAddrComboBox->setEditText(QMIDICTL_UDP_IPV6_ADDR);
	else
#endif
		m_ui.UdpAddrComboBox->setEditText(QMIDICTL_UDP_IPV4_ADDR);
	m_ui.UdpPortSpinBox->setValue(QMIDICTL_UDP_PORT);
}


// end of qmidictlOptionsForm.cpp
