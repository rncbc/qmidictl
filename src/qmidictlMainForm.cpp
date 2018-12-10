// qmidictlMainForm.cpp
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

#include "qmidictlAbout.h"
#include "qmidictlMainForm.h"

#include "qmidictlOptionsForm.h"
#include "qmidictlMidiControlForm.h"

#include "qmidictlOptions.h"
#include "qmidictlMidiControl.h"
#include "qmidictlUdpDevice.h"
#include "qmidictlDialStyle.h"

#include <QTimer>

#include <QMessageBox>

#if defined(Q_OS_ANDROID)
#include "qmidictlActionBar.h"
#endif

// Possible cubic-root optimization.
// (borrowed from metamerist.com)
#include <cmath>

static inline float cbrtf2 ( float x )
{
#ifdef CONFIG_FLOAT32
	// Avoid strict-aliasing optimization (gcc -O2).
	union { float f; int i; } u;
	u.f = x;
	u.i = (u.i / 3) + 710235478;
	return u.f;
#else
	return cbrtf(x);
#endif
}

static inline float cubef2 ( float x )
{
	return x * x * x;
}


//----------------------------------------------------------------------------
// qmidictlMainForm -- UI wrapper form.
//

qmidictlMainForm *qmidictlMainForm::g_pMainForm = NULL;

// Constructor.
qmidictlMainForm::qmidictlMainForm (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QMainWindow(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// The main network device.
	m_pUdpDevice = new qmidictlUdpDevice(this);

	// Strip page/states.
	m_iStripPages = 4;
	m_pStripStates = NULL;
	m_iCurrentStripPage = 0;

	// Jog-wheel last known state.
	m_iJogWheelDelta = 0;
	m_iJogWheelValue = 0;

	// Activity LED counters.
	m_iMidiInLed  = 0;
	m_iMidiOutLed = 0;

	// Kind of soft-mutex.
	m_iBusy = 0;

	// Hack the jog-wheel dial style and palette...
	m_pDialStyle = new qmidictlDialStyle();
	m_ui.jogWheelDial->setStyle(m_pDialStyle);

#if defined(Q_OS_ANDROID)

	// Special stuff for the android platform.
	m_ui.optionsAction->setIcon(QIcon(":/images/actionOptions.png"));
	m_ui.configureAction->setIcon(QIcon(":/images/actionConfigure.png"));
	m_ui.aboutAction->setIcon(QIcon(":/images/actionAbout.png"));
//	m_ui.exitAction->setIcon(QIcon(":/images/actionCancel.png"));

	// Special action-bar for the android stuff.
	m_pActionBar = new qmidictlActionBar();
	m_pActionBar->setIcon(QIcon(":/images/qmidictl.png"));
#if QT_VERSION >= 0x050100
	m_pActionBar->setTitle(QApplication::applicationDisplayName());
#else
	m_pActionBar->setTitle(QMIDICTL_TITLE " - " + tr(QMIDICTL_SUBTITLE));
#endif
	// Action-bar left-drop-down menu items...
	m_pActionBar->addMenuItem(m_ui.optionsAction);
	m_pActionBar->addMenuItem(m_ui.configureAction);
	m_pActionBar->addMenuItem(m_ui.aboutAction);
//	m_pActionBar->addMenuItem(m_ui.exitAction);
	// Action-bar right-overflow button items...
	m_pActionBar->addButton(m_ui.optionsAction);
	m_pActionBar->addButton(m_ui.configureAction);
	m_pActionBar->addButton(m_ui.aboutAction);
//	m_pActionBar->addButton(m_ui.exitAction);
	// Make it at the top...
	m_ui.MainCentralLayout->insertWidget(0, m_pActionBar);

#endif

	// Pseudo-singleton reference setup.
	g_pMainForm = this;

	// Initialize strip connections...
	initMixerStrips();

	// Main menu action connections.
	QObject::connect(m_ui.optionsAction,
		SIGNAL(triggered(bool)),
		SLOT(optionsSlot()));
	QObject::connect(m_ui.configureAction,
		SIGNAL(triggered(bool)),
		SLOT(configureSlot()));
	QObject::connect(m_ui.aboutAction,
		SIGNAL(triggered(bool)),
		SLOT(aboutSlot()));
	QObject::connect(m_ui.exitAction,
		SIGNAL(triggered(bool)),
		SLOT(exitSlot()));

	// UI widgets signal/slot connections...
	QObject::connect(m_ui.prevStripPageButton,
		SIGNAL(clicked()),
		SLOT(prevStripPageSlot()));
	QObject::connect(m_ui.nextStripPageButton,
		SIGNAL(clicked()),
		SLOT(nextStripPageSlot()));

	QObject::connect(m_ui.jogWheelDial,
		SIGNAL(valueChanged(int)),
		SLOT(jogWheelSlot(int)));

	QObject::connect(m_ui.resetButton,
		SIGNAL(clicked()),
		SLOT(resetSlot()));
	QObject::connect(m_ui.rewindButton,
		SIGNAL(toggled(bool)),
		SLOT(rewindSlot(bool)));
	QObject::connect(m_ui.playButton,
		SIGNAL(toggled(bool)),
		SLOT(playSlot(bool)));
	QObject::connect(m_ui.stopButton,
		SIGNAL(clicked()),
		SLOT(stopSlot()));
	QObject::connect(m_ui.recordButton,
		SIGNAL(toggled(bool)),
		SLOT(recordSlot(bool)));
	QObject::connect(m_ui.forwardButton,
		SIGNAL(toggled(bool)),
		SLOT(forwardSlot(bool)));

	// Network signal/slot connections...
	QObject::connect(m_pUdpDevice,
		SIGNAL(received(const QByteArray&)),
		SLOT(receiveSlot(const QByteArray&)));
}


// Destructor.
qmidictlMainForm::~qmidictlMainForm (void)
{
#if defined(Q_OS_ANDROID)
	// No need for special android stuff anymore.
	delete m_pActionBar;
#endif

	// No need for fancy styling no more.
	delete m_pDialStyle;

	// No more strip states.
	delete [] m_pStripStates;

	// No more network device.
	delete m_pUdpDevice;

	// Pseudo-singleton reference shut-down.
	g_pMainForm = NULL;
}


// Kind of singleton reference.
qmidictlMainForm *qmidictlMainForm::getInstance (void)
{
	return g_pMainForm;
}


// Mixer strip page accessors.
void qmidictlMainForm::setCurrentStripPage ( int iStripPage )
{
	if (iStripPage < 0 || iStripPage > m_iStripPages - 1)
		return;

	if (iStripPage != m_iCurrentStripPage)
		saveStripPage(m_iCurrentStripPage);

	m_iCurrentStripPage = iStripPage;

	loadStripPage(m_iCurrentStripPage);
	stabilizeForm();
}

int qmidictlMainForm::currentStripPage (void) const
{
	return m_iCurrentStripPage;
}


// Setup method.
void qmidictlMainForm::setup (void)
{
	qmidictlOptions *pOptions = qmidictlOptions::getInstance();
	if (pOptions == NULL)
		return;

	if (!m_pUdpDevice->open(
			pOptions->sInterface,
			pOptions->sUdpAddr,
			pOptions->iUdpPort)) {
		QMessageBox::critical(this,
			tr("Network Inferface Error - %1").arg(QMIDICTL_TITLE),
			tr("The network interface could not be established.\n\n"
			"Please, make sure you have an on-line network connection "
			"and try again."));
	}
}


// Initialize mixer strips.
void qmidictlMainForm::initMixerStrips (void)
{
	initStripStates();

	initMixerStrip(m_ui.strip1);
	initMixerStrip(m_ui.strip2);
	initMixerStrip(m_ui.strip3);
	initMixerStrip(m_ui.strip4);

	setCurrentStripPage(0);
}

void qmidictlMainForm::initMixerStrip ( qmidictlMixerStrip *pStrip )
{
	QObject::connect(pStrip,
		SIGNAL(recordToggled(int, bool)),
		SLOT(stripRecordSlot(int, bool)));
	QObject::connect(pStrip,
		SIGNAL(muteToggled(int, bool)),
		SLOT(stripMuteSlot(int, bool)));
	QObject::connect(pStrip,
		SIGNAL(soloToggled(int, bool)),
		SLOT(stripSoloSlot(int, bool)));
	QObject::connect(pStrip,
		SIGNAL(sliderChanged(int, int)),
		SLOT(stripSliderSlot(int, int)));
}


// Strip states methods.
void qmidictlMainForm::initStripStates (void)
{
	int iStrips = 4 * m_iStripPages;

	m_pStripStates = new StripState [iStrips];

	for (int iStrip = 0; iStrip < iStrips; ++iStrip) {
		StripState *pState = &m_pStripStates[iStrip];
		pState->strip  = iStrip;
		pState->record = false;
		pState->mute   = false;
		pState->solo   = false;
		pState->slider = false;
	}
}


void qmidictlMainForm::saveStripState ( qmidictlMixerStrip *pStrip, int iStrip )
{
	StripState *pState = &m_pStripStates[iStrip];

	pState->strip  = pStrip->strip();
	pState->record = pStrip->isRecord();
	pState->mute   = pStrip->isMute();
	pState->solo   = pStrip->isSolo();
	pState->slider = pStrip->slider();
}


void qmidictlMainForm::loadStripState ( qmidictlMixerStrip *pStrip, int iStrip )
{
	StripState *pState = &m_pStripStates[iStrip];

	pStrip->setStrip(pState->strip,
		pState->record,
		pState->mute,
		pState->solo,
		pState->slider);
}


void qmidictlMainForm::saveStripPage ( int iStripPage )
{
	int iStrip = (iStripPage * 4);

	saveStripState(m_ui.strip1, iStrip + 0);
	saveStripState(m_ui.strip2, iStrip + 1);
	saveStripState(m_ui.strip3, iStrip + 2);
	saveStripState(m_ui.strip4, iStrip + 3);
}


void qmidictlMainForm::loadStripPage ( int iStripPage )
{
	int iStrip = (iStripPage * 4);

	loadStripState(m_ui.strip1, iStrip + 0);
	loadStripState(m_ui.strip2, iStrip + 1);
	loadStripState(m_ui.strip3, iStrip + 2);
	loadStripState(m_ui.strip4, iStrip + 3);
}


// Common form stabilizer.
void qmidictlMainForm::stabilizeForm(void)
{
	m_ui.prevStripPageButton->setEnabled(m_iCurrentStripPage > 0);
	m_ui.nextStripPageButton->setEnabled(m_iCurrentStripPage < m_iStripPages - 1);
}


// Mixer strip slots.
void qmidictlMainForm::prevStripPageSlot (void)
{
	setCurrentStripPage(currentStripPage() - 1);
}


void qmidictlMainForm::nextStripPageSlot (void)
{
	setCurrentStripPage(currentStripPage() + 1);
}


// MMC Command dispatcher.
void qmidictlMainForm::sendMmcCommand (
	MmcCommand cmd, unsigned char *data, unsigned short len )
{
	// Build up the MMC sysex message...
	unsigned char *pSysex;
	unsigned short iSysex = 6;

	if (data && len > 0)
		iSysex += 1 + len;
	pSysex = new unsigned char [iSysex];
	iSysex = 0;

	pSysex[iSysex++] = 0xf0; // Sysex header.
	pSysex[iSysex++] = 0x7f; // Realtime sysex.
	pSysex[iSysex++] = qmidictlOptions::getInstance()->iMmcDevice;
	pSysex[iSysex++] = 0x06; // MMC command mode.
	pSysex[iSysex++] = (unsigned char) cmd;	// MMC command code.
	if (data && len > 0) {
		pSysex[iSysex++] = len;
		::memcpy(&pSysex[iSysex], data, len);
		iSysex += len;
	}
	pSysex[iSysex++] = 0xf7; // Sysex trailer.

	// Go.
	sendData(pSysex, iSysex);

	// Done.
	delete pSysex;
}


// MMC dispatch special commands.
void qmidictlMainForm::sendMmcMaskedWrite (
	MmcSubCommand scmd, int iTrack, bool bOn )
{
	unsigned char data[4];
	int iMask = (1 << (iTrack < 2 ? iTrack + 5 : (iTrack - 2) % 7));

	data[0] = scmd;
	data[1] = (unsigned char) (iTrack < 2 ? 0 : 1 + (iTrack - 2) / 7);
	data[2] = (unsigned char) iMask;
	data[3] = (unsigned char) (bOn ? iMask : 0);

	sendMmcCommand(MMC_MASKED_WRITE, data, sizeof(data));
}


void qmidictlMainForm::sendMmcLocate ( unsigned long iLocate )
{
	unsigned char data[6];

	data[0] = 0x01;
	data[1] = iLocate / (3600 * 30); iLocate -= (3600 * 30) * (int) data[1];
	data[2] = iLocate / (  60 * 30); iLocate -= (  60 * 30) * (int) data[2];
	data[3] = iLocate / (       30); iLocate -= (       30) * (int) data[3];
	data[4] = iLocate;
	data[5] = 0;

	sendMmcCommand(MMC_LOCATE, data, sizeof(data));
}


void qmidictlMainForm::sendMmcStep ( int iDelta )
{
	unsigned char data;
	if (iDelta < 0) {
		data = (-iDelta & 0x3f) | 0x40;
	} else {
		data = (+iDelta & 0x3f);
	}

	sendMmcCommand(MMC_STEP, &data, sizeof(data));
}


// MIDI data transmitter helpers.
void qmidictlMainForm::sendData3 (
	unsigned char data1, unsigned char data2, unsigned char data3 )
{
	unsigned char data[3];

	data[0] = data1;
	data[1] = data2;
	data[2] = data3;

	sendData(data, sizeof(data));
}


void qmidictlMainForm::sendData2 (
	unsigned char data1, unsigned char data2 )
{
	unsigned char data[2];

	data[0] = data1;
	data[1] = data2;

	sendData(data, sizeof(data));
}


void qmidictlMainForm::sendPitchBend ( unsigned short iChannel, int iValue )
{
	unsigned char data[3];
	unsigned short val = (unsigned short) (0x2000 + iValue);

	data[0] = 0xe0 + iChannel;
	data[1] = (val & 0x007f);
	data[2] = (val & 0x3f80) >> 7;

	sendData(data, sizeof(data));
}


// Network transmitter.
void qmidictlMainForm::sendData ( unsigned char *data, unsigned short len )
{
#ifdef CONFIG_DEBUG
	fprintf(stderr, "sendData:");
	for(int i = 0; i < len; ++i)
		fprintf(stderr, " %02X", data[i]);
	fprintf(stderr, "\n");
#endif

	if (m_pUdpDevice->sendData(data, len)) {
		if (++m_iMidiOutLed < 2) {
			m_ui.midiOutLedLabel->setPixmap(QPixmap(":/images/ledOn.png"));
			QTimer::singleShot(200, this, SLOT(timerSlot()));
		}
	}
}


// Generic command dispatcher.
void qmidictlMainForm::sendCommand ( int iCommand, int iTrack, int iValue )
{
	qmidictlMidiControl *pMidiControl = qmidictlMidiControl::getInstance();
	if (pMidiControl == NULL)
		return;
	
	qmidictlMidiControl::Command command
		= qmidictlMidiControl::Command(iCommand);
	if (!pMidiControl->isCommandMapped(command))
		return;

	const qmidictlMidiControl::MapKey& key
		= pMidiControl->commandMap().value(command);

	unsigned short iChannel = key.channel();
	if (key.isChannelTrack()) {
		iChannel &= qmidictlMidiControl::TrackParamMask;
		iChannel += iTrack;
	}

	unsigned short iParam = key.param();
	if (key.isParamTrack()) {
		iParam &= qmidictlMidiControl::TrackParamMask;
		iParam += iTrack;
	}

	if (key.isLogarithmic())
		iValue = int(127.0f * cbrtf2(float(iValue) / 127.0f));

	bool bOn = (iValue > 0);
	switch (key.type()) {
	case qmidictlMidiControl::MMC:
		switch (command) {
		case qmidictlMidiControl::RST:
			sendMmcLocate(0);
			break;
		case qmidictlMidiControl::REW:
			sendMmcCommand(bOn ? MMC_REWIND : MMC_STOP);
			break;
		case qmidictlMidiControl::STOP:
			sendMmcCommand(MMC_STOP);
			break;
		case qmidictlMidiControl::PLAY:
			sendMmcCommand(bOn ? MMC_PLAY : MMC_PAUSE);
			break;
		case qmidictlMidiControl::REC:
			sendMmcCommand(bOn ? MMC_RECORD_STROBE : MMC_RECORD_EXIT);
			break;
		case qmidictlMidiControl::FFWD:
			sendMmcCommand(bOn ? MMC_FAST_FORWARD : MMC_STOP);
			break;
		case qmidictlMidiControl::JOG_WHEEL:
			sendMmcStep(iValue);
			break;
		case qmidictlMidiControl::TRACK_SOLO:
			sendMmcMaskedWrite(MMC_TRACK_SOLO, iTrack, bOn);
			break;
		case qmidictlMidiControl::TRACK_MUTE:
			sendMmcMaskedWrite(MMC_TRACK_MUTE, iTrack, bOn);
			break;
		case qmidictlMidiControl::TRACK_REC:
			sendMmcMaskedWrite(MMC_TRACK_RECORD, iTrack, bOn);
			break;
		case qmidictlMidiControl::TRACK_VOL: // Not handled here.
		default:
			break;
		}
		break;
	case qmidictlMidiControl::NOTE_OFF:
		sendData3(0x80 + iChannel, iParam, iValue);
		break;
	case qmidictlMidiControl::NOTE_ON:
		sendData3(0x90 + iChannel, iParam, iValue);
		break;
	case qmidictlMidiControl::KEY_PRESS:
		sendData3(0xa0 + iChannel, iParam, iValue);
		break;
	case qmidictlMidiControl::CONTROLLER:
		sendData3(0xb0 + iChannel, iParam, iValue);
		break;
	case qmidictlMidiControl::PGM_CHANGE:
		sendData2(0xc0 + iChannel, iParam);
		break;
	case qmidictlMidiControl::CHAN_PRESS:
		sendData2(0xd0 + iChannel, iValue);
		break;
	case qmidictlMidiControl::PITCH_BEND:
		sendPitchBend(iChannel, iValue);
		break;
	}
}


// Mixer strip slots.
void qmidictlMainForm::stripRecordSlot ( int iStrip, bool bOn )
{
	// Update strip state...
	if (iStrip >= 0 && iStrip < (4 * m_iStripPages))
		m_pStripStates[iStrip].record = bOn;

	sendCommand(qmidictlMidiControl::TRACK_REC, iStrip, int(bOn));
}


void qmidictlMainForm::stripMuteSlot ( int iStrip, bool bOn )
{
	// Update strip state...
	if (iStrip >= 0 && iStrip < (4 * m_iStripPages))
		m_pStripStates[iStrip].mute = bOn;

	sendCommand(qmidictlMidiControl::TRACK_MUTE, iStrip, int(bOn));
}


void qmidictlMainForm::stripSoloSlot ( int iStrip, bool bOn )
{
	// Update strip state...
	if (iStrip >= 0 && iStrip < (4 * m_iStripPages))
		m_pStripStates[iStrip].solo = bOn;

	sendCommand(qmidictlMidiControl::TRACK_SOLO, iStrip, int(bOn));
}


void qmidictlMainForm::stripSliderSlot ( int iStrip, int iValue )
{
	// Update strip state...
	if (iStrip >= 0 && iStrip < (4 * m_iStripPages))
		m_pStripStates[iStrip].slider = iValue;

	sendCommand(qmidictlMidiControl::TRACK_VOL, iStrip, (127 * iValue) / 100);
}


// Jog wheel slot.
void qmidictlMainForm::jogWheelSlot ( int iValue )
{
	int iDelta = (iValue - m_iJogWheelValue);
	if ((iDelta * m_iJogWheelDelta) < 0)
		iDelta = -(m_iJogWheelDelta);

	sendCommand(qmidictlMidiControl::JOG_WHEEL, 0, iDelta);

	m_iJogWheelValue = iValue;
	m_iJogWheelDelta = iDelta;
}


// Reset action slot
void qmidictlMainForm::resetSlot (void)
{
	sendCommand(qmidictlMidiControl::RST);
}


// Rewind action slot
void qmidictlMainForm::rewindSlot ( bool bOn )
{
	if (m_iBusy > 0)
		return;

	m_iBusy++;
	m_ui.forwardButton->setChecked(false);
	m_iBusy--;

	sendCommand(qmidictlMidiControl::REW, 0, int(bOn));
}


// Start/Play action slot
void qmidictlMainForm::playSlot ( bool bOn )
{
	if (m_iBusy > 0)
		return;

	sendCommand(qmidictlMidiControl::PLAY, 0, int(bOn));
}


// Stop action slot
void qmidictlMainForm::stopSlot (void)
{
	if (m_iBusy > 0)
		return;

	m_iBusy++;
	m_ui.rewindButton->setChecked(false);
	m_ui.forwardButton->setChecked(false);
	m_ui.recordButton->setChecked(false);
	m_ui.playButton->setChecked(false);
	m_iBusy--;

	sendCommand(qmidictlMidiControl::STOP);
}


// Record action slot
void qmidictlMainForm::recordSlot ( bool bOn )
{
	if (m_iBusy > 0)
		return;

	sendCommand(qmidictlMidiControl::REC, 0, int(bOn));
}


// Forward action slot
void qmidictlMainForm::forwardSlot ( bool bOn )
{
	if (m_iBusy > 0)
		return;

	m_iBusy++;
	m_ui.rewindButton->setChecked(false);
	m_iBusy--;

	sendCommand(qmidictlMidiControl::FFWD, 0, int(bOn));
}


// Network listener/receiver slot.
void qmidictlMainForm::receiveSlot ( const QByteArray& data )
{
	recvData((unsigned char *) data.constData(), data.length());
}

void qmidictlMainForm::recvData ( unsigned char *data, unsigned short len )
{
	qmidictlMidiControl *pMidiControl = qmidictlMidiControl::getInstance();
	if (pMidiControl == NULL)
		return;

	m_iBusy++;

#ifdef CONFIG_DEBUG
	fprintf(stderr, "recvData:");
	for(int i = 0; i < len; ++i)
		fprintf(stderr, " %02X", data[i]);
	fprintf(stderr, "\n");
#endif

	// Flash the MIDI In LED...
	if (++m_iMidiInLed < 2) {
		m_ui.midiInLedLabel->setPixmap(QPixmap(":/images/ledOn.png"));
		QTimer::singleShot(200, this, SLOT(timerSlot()));
	}

	// Handle immediate incoming MIDI data...
	int iTracks = (4 * m_iStripPages);
	int iTrack  = -1;

	qmidictlMidiControl::ControlType ctype
		= qmidictlMidiControl::ControlType(0);
	unsigned short iChannel = 0;
	unsigned short iParam = 0;
	int iValue = 0;

	// SysEx (actually)...
	if (data[0] == 0xf0 && data[len - 1] == 0xf7) {
		// MMC command...
		unsigned char mmcid = qmidictlOptions::getInstance()->iMmcDevice;
		if (data[1] == 0x7f && data[3] == 0x06
			&& (mmcid == 0x7f || data[2] == mmcid)) {
			ctype = qmidictlMidiControl::MMC;
			MmcCommand cmd = MmcCommand(data[4]);
			switch (cmd) {
			case MMC_STOP:
			case MMC_RESET:
				iParam = int(qmidictlMidiControl::STOP);
				iValue = int(true);
				break;
			case MMC_PLAY:
			case MMC_DEFERRED_PLAY:
				iParam = int(qmidictlMidiControl::PLAY);
				iValue = int(true);
				break;
			case MMC_REWIND:
				iParam = int(qmidictlMidiControl::REW);
				iValue = int(true);
				break;
			case MMC_FAST_FORWARD:
				iParam = int(qmidictlMidiControl::FFWD);
				iValue = int(true);
				break;
			case MMC_RECORD_STROBE:
				iParam = int(qmidictlMidiControl::REC);
				iValue = int(true);
				break;
			case MMC_RECORD_EXIT:
			case MMC_RECORD_PAUSE:
				iParam = int(qmidictlMidiControl::REC);
				iValue = int(false);
				break;
			case MMC_PAUSE:
				iParam = int(qmidictlMidiControl::PLAY);
				iValue = int(false);
				break;
			case MMC_MASKED_WRITE:
				if (int(data[5]) > 3) {
					MmcSubCommand scmd = MmcSubCommand(data[6]);
					iTrack = (data[7] > 0 ? (data[7] * 7) : 0) - 5;
					iValue = int(false);
					for (int i = 0; i < 7; ++i) {
						int iMask = (1 << i);
						if (data[8] & iMask) {
							iValue = (data[9] & iMask);
							break;
						}
						++iTrack;
					}
					// Patch corresponding track/strip state...
					if (iTrack >= 0 && iTrack < iTracks) {
						switch (scmd) {
						case MMC_TRACK_RECORD:
							iParam = int(qmidictlMidiControl::TRACK_REC);
							break;
						case MMC_TRACK_MUTE:
							iParam = int(qmidictlMidiControl::TRACK_MUTE);
							break;
						case MMC_TRACK_SOLO:
							iParam = int(qmidictlMidiControl::TRACK_SOLO);
							break;
						default:
							break;
						}
					}
				}
				// Fall thru...
			default:
				break;
			}
		}
	} else {
		// Channel event...
		iChannel = (data[0] & 0x0f);
		iParam = data[1];
		if (len > 2)
			iValue = data[2];
		// Channel control type...
		switch (data[0] & 0xf0) {
		case 0x80:
			ctype = qmidictlMidiControl::NOTE_OFF;
			break;
		case 0x90:
			ctype = qmidictlMidiControl::NOTE_ON;
			break;
		case 0xa0:
			ctype = qmidictlMidiControl::KEY_PRESS;
			break;
		case 0xb0:
			ctype = qmidictlMidiControl::CONTROLLER;
			break;
		case 0xc0:
			ctype = qmidictlMidiControl::PGM_CHANGE;
			break;
		case 0xd0:
			ctype = qmidictlMidiControl::CHAN_PRESS;			
			break;
		case 0xe0:
			ctype = qmidictlMidiControl::PITCH_BEND;
			iParam = 0;
			iValue = ((int(data[2]) << 7) | data[1]) - 0x2000;
			break;
		}
	}

	// Lookup the command mapping...
	qmidictlMidiControl::ControlMap::ConstIterator iter
		= pMidiControl->find(ctype, iChannel, iParam);
	if (iter != pMidiControl->controlMap().constEnd()) {
		const qmidictlMidiControl::MapKey& key = iter.key();
		qmidictlMidiControl::Command command = iter.value();
		if (key.isChannelTrack())
			iTrack  = int(iChannel);
		else if (key.isParamTrack()) {
			iTrack  = int(iParam);
			iTrack -= int(key.param() & qmidictlMidiControl::TrackParamMask);
		}
		if (key.isLogarithmic())
			iValue = int(127.0f * cubef2(float(iValue) / 127.0f));
	#ifdef CONFIG_DEBUG
		qDebug("recvData: Command=0x%04x Type=0x%04x Track=%d Value=%d",
			uint(command), uint(ctype), iTrack, iValue);
	#endif
		switch (command) {
		case qmidictlMidiControl::RST:
			// Nothing to do...
			break;
		case qmidictlMidiControl::REW:
			m_ui.rewindButton->setChecked(true);
			m_ui.forwardButton->setChecked(false);
			break;
		case qmidictlMidiControl::STOP:
			m_ui.rewindButton->setChecked(false);
			m_ui.playButton->setChecked(false);
			m_ui.recordButton->setChecked(false);
			m_ui.forwardButton->setChecked(false);
			break;
		case qmidictlMidiControl::PLAY:
			m_ui.playButton->setChecked(iValue > 0);
			break;
		case qmidictlMidiControl::REC:
			m_ui.recordButton->setChecked(iValue > 0);
			break;
		case qmidictlMidiControl::FFWD:
			m_ui.rewindButton->setChecked(false);
			m_ui.forwardButton->setChecked(true);
			break;
		case qmidictlMidiControl::JOG_WHEEL:
			// Nothing to do...
			break;
		case qmidictlMidiControl::TRACK_SOLO:
			if (iTrack >= 0 && iTrack < iTracks)
				m_pStripStates[iTrack].solo = bool(iValue > 0);
			break;
		case qmidictlMidiControl::TRACK_MUTE:
			if (iTrack >= 0 && iTrack < iTracks)
				m_pStripStates[iTrack].mute = bool(iValue > 0);
			break;
		case qmidictlMidiControl::TRACK_REC:
			if (iTrack >= 0 && iTrack < iTracks)
				m_pStripStates[iTrack].record = bool(iValue > 0);
			break;
		case qmidictlMidiControl::TRACK_VOL:
			if (iTrack >= 0 && iTrack < iTracks)
				m_pStripStates[iTrack].slider = (100 * iValue) / 127;
			break;
		}
	}

	// Update corresponding strip state, when currently visible...
	int iStrip = (4 * m_iCurrentStripPage);
	if (iTrack >= iStrip && iTrack < iStrip + 4) {
		switch (iTrack % 4) {
		case 0: loadStripState(m_ui.strip1, iTrack); break;
		case 1: loadStripState(m_ui.strip2, iTrack); break;
		case 2: loadStripState(m_ui.strip3, iTrack); break;
		case 3: loadStripState(m_ui.strip4, iTrack); break;
		}
	}

	// Done.
	m_iBusy--;
}


// Timer slot funtion.
void qmidictlMainForm::timerSlot (void)
{
	// Handle pending incoming MIDI data...
	if (m_iMidiInLed > 0) {
		m_iMidiInLed = 0;
		m_ui.midiInLedLabel->setPixmap(QPixmap(":/images/ledOff.png"));
	}

	// Handle pending outgoing MIDI data...
	if (m_iMidiOutLed > 0) {
		m_iMidiOutLed = 0;
		m_ui.midiOutLedLabel->setPixmap(QPixmap(":/images/ledOff.png"));
	}
}


// Options action slot.
void qmidictlMainForm::optionsSlot (void)
{
	if (qmidictlOptionsForm(this).exec())
		setup();
}


// MIDI control configuration dialog.
void qmidictlMainForm::configureSlot (void)
{
	qmidictlMidiControlForm(this).exec();
}


// About dialog.
void qmidictlMainForm::aboutSlot (void)
{
	// Stuff the about box text...
	QString sText = "<p>\n";
	sText += "<b>" QMIDICTL_TITLE " - " + tr(QMIDICTL_SUBTITLE) + "</b><br />\n";
	sText += "<br />\n";
	sText += tr("Version") + ": <b>" CONFIG_BUILD_VERSION "</b><br />\n";
//	sText += "<small>" + tr("Build") + ": " CONFIG_BUILD_DATE "</small><br />\n";
#ifdef CONFIG_DEBUG
	sText += "<small><font color=\"red\">";
	sText += tr("Debugging option enabled.");
	sText += "</font></small><br />\n";
#endif
	sText += "<br />\n";
	sText += tr("Website") + ": <a href=\"" QMIDICTL_WEBSITE "\">" QMIDICTL_WEBSITE "</a><br />\n";
	sText += "<br />\n";
	sText += "<small>";
	sText += QMIDICTL_COPYRIGHT "<br />\n";
	sText += "<br />\n";
	sText += tr("This program is free software; you can redistribute it and/or modify it") + "<br />\n";
	sText += tr("under the terms of the GNU General Public License version 2 or later.");
	sText += "</small>";
	sText += "</p>\n";

	QMessageBox::about(this, tr("About %1").arg(QMIDICTL_TITLE), sText);
}


// Exit/quit action slot.
void qmidictlMainForm::exitSlot (void)
{
	close();
}


// end of qmidictlMainForm.cpp
