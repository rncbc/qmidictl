// qmidictlMainForm.h
//
/****************************************************************************
   Copyright (C) 2010-2011, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __qmidictlMainForm_h
#define __qmidictlMainForm_h

#include "ui_qmidictlMainForm.h"


// Forward declarations.
class qmidictlUdpDevice;
class qmidictlMixerStrip;
class qmidictlDialStyle;


//----------------------------------------------------------------------------
// qmidictlMainForm -- UI wrapper form.

class qmidictlMainForm : public QMainWindow
{
	Q_OBJECT

public:

	// Constructor.
	qmidictlMainForm(QWidget *pParent = 0, Qt::WindowFlags wflags = 0);
	// Destructor.
	~qmidictlMainForm();

	// Kind of singleton reference.
	static qmidictlMainForm *getInstance();

	// Strip bank accessors.
	void setCurrentStripPage(int iStripPage);
	int currentStripPage() const;

	// Setup method.
	void setup();

protected:

	// MMC Command Codes.
	enum MmcCommand
	{
		MMC_STOP                   = 0x01,
		MMC_PLAY                   = 0x02,
		MMC_DEFERRED_PLAY          = 0x03,
		MMC_FAST_FORWARD           = 0x04,
		MMC_REWIND                 = 0x05,
		MMC_RECORD_STROBE          = 0x06,
		MMC_RECORD_EXIT            = 0x07,
		MMC_RECORD_PAUSE           = 0x08,
		MMC_PAUSE                  = 0x09,
		MMC_EJECT                  = 0x0a,
		MMC_CHASE                  = 0x0b,
		MMC_COMMAND_ERROR_RESET    = 0x0c,
		MMC_RESET                  = 0x0d,
		MMC_JOG_START              = 0x20,
		MMC_JOG_STOP               = 0x21,
		MMC_WRITE                  = 0x40,
		MMC_MASKED_WRITE           = 0x41,
		MMC_READ                   = 0x42,
		MMC_UPDATE                 = 0x43,
		MMC_LOCATE                 = 0x44,
		MMC_VARIABLE_PLAY          = 0x45,
		MMC_SEARCH                 = 0x46,
		MMC_SHUTTLE                = 0x47,
		MMC_STEP                   = 0x48,
		MMC_ASSIGN_SYSTEM_MASTER   = 0x49,
		MMC_GENERATOR_COMMAND      = 0x4a,
		MMC_MTC_COMMAND            = 0x4b,
		MMC_MOVE                   = 0x4c,
		MMC_ADD                    = 0x4d,
		MMC_SUBTRACT               = 0x4e,
		MMC_DROP_FRAME_ADJUST      = 0x4f,
		MMC_PROCEDURE              = 0x50,
		MMC_EVENT                  = 0x51,
		MMC_GROUP                  = 0x52,
		MMC_COMMAND_SEGMENT        = 0x53,
		MMC_DEFERRED_VARIABLE_PLAY = 0x54,
		MMC_RECORD_STROBE_VARIABLE = 0x55,
		MMC_WAIT                   = 0x7c,
		MMC_RESUME                 = 0x7f
	};

	// MMC Command dispatcher.
	void sendMmcCommand (MmcCommand cmd,
		unsigned char *data = NULL, unsigned short len = 0);

	// MMC sub-command codes (as for MASKED_WRITE).
	enum MmcSubCommand {
		MMC_TRACK_NONE             = 0x00,
		MMC_TRACK_RECORD           = 0x4f,
		MMC_TRACK_MUTE             = 0x62,
		MMC_TRACK_SOLO             = 0x66 // Custom-implementation ;)
	};

	void sendMmcMaskedWrite(MmcSubCommand scmd, int iTrack, bool bOn);

	// MMC dispatch special commands.
	void sendMmcLocate(unsigned long iLocate);
	void sendMmcStep(int iDelta);

	// MIDI data transmitter helpers.
	void sendData3(
		unsigned char data1, unsigned char data2, unsigned char data3);
	void sendData2(
		unsigned char data1, unsigned char data2);

	void sendPitchBend(unsigned short iChannel, int iValue);

	// Generic command dispatcher.
	void sendCommand(int iCommand, int iTrack = 0, int iValue = 0);

	// Network transmitter.
	void sendData(unsigned char *data, unsigned short len);
	void recvData(unsigned char *data, unsigned short len);

	// Initialize mixer strips.
	void initMixerStrips();
	void initMixerStrip(qmidictlMixerStrip *pStrip);

	// Strip states methods.
	void initStripStates();

	void saveStripState(qmidictlMixerStrip *pStrip, int iStrip);
	void loadStripState(qmidictlMixerStrip *pStrip, int iStrip);

	void saveStripPage(int iStripPage);
	void loadStripPage(int iStripPage);

	// Common form stabilizer.
	void stabilizeForm();

protected slots:

	// Mixer strip slots.
	void prevStripPageSlot();
	void nextStripPageSlot();

	void stripRecordSlot(int iStrip, bool bOn);
	void stripMuteSlot(int iStrip, bool bOn);
	void stripSoloSlot(int iStrip, bool bOn);
	void stripSliderSlot(int iStrip, int iValue);

	// Jog wheel slot.
	void jogWheelSlot(int iValue);

	// Transport slots.
	void resetSlot();
	void rewindSlot(bool bOn);
	void playSlot(bool bOn);
	void stopSlot();
	void recordSlot(bool bon);
	void forwardSlot(bool bOn);

	// Network listener/receiver slot.
	void receiveSlot(const QByteArray& data);

	// Timer slot funtion.
	void timerSlot();

	// Main action slots.
	void optionsSlot();
	void configureSlot();
	void aboutSlot();
	void exitSlot();

private:

	// The Qt-designer UI struct...
	Ui::qmidictlMainForm m_ui;

	// The main network device.
	qmidictlUdpDevice *m_pUdpDevice;

	// Number of strip pages.
	int m_iStripPages;

	// Strip state table.
	struct StripState
	{
		int  strip;
		bool record;
		bool mute;
		bool solo;
		int  slider;

	} *m_pStripStates;

	// Current mixer strip page.
	int m_iCurrentStripPage;

	// Jog wheel last known state.
	int m_iJogWheelDelta;
	int m_iJogWheelValue;

	// Activity LED counters.
	int m_iMidiInLed;
	int m_iMidiOutLed;

	// Kind of soft-mutex.
	int m_iBusy;

	// Special style for the jog wheel dial.
	qmidictlDialStyle *m_pDialStyle;

	// Kind-of singleton reference.
	static qmidictlMainForm *g_pMainForm;
};


#endif	// __qmidictlMainForm_h


// end of qmidictlMainForm.h
