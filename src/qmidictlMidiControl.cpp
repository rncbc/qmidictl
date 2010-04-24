// qmidictlMidiControl.cpp
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

#include "qmidictlMidiControl.h"

#include <QStringList>
#include <QSettings>


//----------------------------------------------------------------------
// qmidictlMidiControl -- MIDI control map (singleton).
//

// Kind of singleton reference.
qmidictlMidiControl* qmidictlMidiControl::g_pMidiControl = NULL;

// Constructor.
qmidictlMidiControl::qmidictlMidiControl (void)
{
	// Pseudo-singleton reference setup.
	g_pMidiControl = this;

	// Default controller mapping...
	clear();
}


// Destructor.
qmidictlMidiControl::~qmidictlMidiControl (void)
{
	// Pseudo-singleton reference shut-down.
	g_pMidiControl = NULL;
}


// Kind of singleton reference.
qmidictlMidiControl *qmidictlMidiControl::getInstance (void)
{
	return g_pMidiControl;
}

// Clear control map (reset to default).
void qmidictlMidiControl::clear (void)
{
	m_controlMap.clear();

	mapCommand(RST,        MMC, 0, int(RST));
	mapCommand(REW,        MMC, 0, int(REW));
	mapCommand(STOP,       MMC, 0, int(STOP));
	mapCommand(PLAY,       MMC, 0, int(PLAY));
	mapCommand(REC,        MMC, 0, int(REC));
	mapCommand(FFWD,       MMC, 0, int(FFWD));

	mapCommand(JOG_WHEEL,  MMC, 0, int(JOG_WHEEL));

	mapCommand(TRACK_SOLO, MMC, 0, int(TRACK_SOLO));
	mapCommand(TRACK_MUTE, MMC, 0, int(TRACK_MUTE));
	mapCommand(TRACK_REC,  MMC, 0, int(TRACK_REC));

	mapCommand(TRACK_VOL, CONTROLLER, 15, TrackParam | 0x40);
}


// Map accessors.
const qmidictlMidiControl::CommandMap& qmidictlMidiControl::commandMap (void) const
{
	return m_commandMap;
}

const qmidictlMidiControl::ControlMap& qmidictlMidiControl::controlMap (void) const
{
	return m_controlMap;
}


// Insert new controller mappings.
void qmidictlMidiControl::mapCommand ( Command command,
	ControlType ctype, unsigned short iChannel, unsigned short iParam )
{
	MapKey key(ctype, iChannel, iParam);

	m_commandMap.insert(command, key);
	m_controlMap.insert(key, command);
}

// Remove existing controller mapping.
void qmidictlMidiControl::unmapCommand ( Command command )
{
	if (isCommandMapped(command)) {
		const MapKey& key = m_commandMap.value(command);
		m_controlMap.remove(key);
		m_commandMap.remove(command);
	}
}


// Check if given command is currently mapped.
bool qmidictlMidiControl::isCommandMapped ( Command command ) const
{
	return m_commandMap.contains(command);
}

// Check if given channel, param pair is currently mapped.
bool qmidictlMidiControl::isChannelParamMapped (
	ControlType ctype, unsigned short iChannel, unsigned short iParam ) const
{
	return m_controlMap.contains(MapKey(ctype, iChannel, iParam));
}


// Lookup the command mapping...
qmidictlMidiControl::ControlMap::ConstIterator qmidictlMidiControl::find (
	ControlType ctype, unsigned short iChannel, unsigned short iParam ) const
{
	MapKey key(ctype, iChannel, iParam);

	ControlMap::ConstIterator iter = m_controlMap.find(key);
	if (iter == m_controlMap.constEnd()) {
		key.setChannel(TrackParam);
		iter = m_controlMap.find(key);
		if (iter == m_controlMap.constEnd()) {
			key.setChannel(iChannel); // Fallback to original channel.
			key.setParam(iParam | TrackParam);
			iter = m_controlMap.find(key);
		}
	}

	return iter;
}


// Save into global settings.
void qmidictlMidiControl::save ( QSettings& settings )
{
	settings.beginGroup("/MidiControl/Default");

	CommandMap::ConstIterator iter = m_commandMap.constBegin();
	for (; iter != m_commandMap.constEnd(); ++iter) {
		Command command = iter.key();
		const MapKey& key = iter.value();
		int iChannel = (key.isChannelTrack() ? 0 : key.channel() + 1);
		int iParam = key.param();
		if (key.isParamTrack())
			iParam &= qmidictlMidiControl::TrackParamMask;
		QString sValue = textFromType(key.type());
		sValue += ',' + QString::number(iChannel);
		sValue += ',' + QString::number(iParam);
		sValue += ',' + QString::number(int(key.isParamTrack()));
		settings.setValue('/' + textFromCommand(command), sValue);
	}

	settings.endGroup();
	settings.sync();
}


// Load from global settings.
void qmidictlMidiControl::load ( QSettings& settings )
{
	clear();

	settings.beginGroup("/MidiControl/Default");

	QStringList commands;
	CommandMap::ConstIterator iter = m_commandMap.constBegin();
	for (; iter != m_commandMap.constEnd(); ++iter)
		commands.append(textFromCommand(iter.key()));

	QStringListIterator it(commands);
	while (it.hasNext()) {
		const QString& sCommand = it.next();
		Command command = commandFromText(sCommand);
		if (!command)
			continue;
		const QString& sValue = settings.value('/' + sCommand).toString();
		if (sValue.isEmpty())
			continue;
		const QString& sControlType = sValue.section(',', 0, 0);
		ControlType ctype = typeFromText(sControlType);
		if (!ctype)
			continue;
		unmapCommand(command);
		unsigned short iChannel = sValue.section(',', 1, 1).toUInt();
		if (iChannel == 0)
			iChannel |= TrackParam;
		else
			iChannel--;
		unsigned short iParam = sValue.section(',', 2, 2).toUInt();
		if (sValue.section(',', 3, 3).toInt() > 0)
			iParam |= TrackParam;
		mapCommand(command, ctype, iChannel, iParam);
	}

	settings.endGroup();
}


// Textual helpers.
qmidictlMidiControl::ControlType qmidictlMidiControl::typeFromText (
	const QString& sText )
{
	if (sText == "MMC")
		return MMC;
	else
	if (sText == "NOTE_ON")
		return NOTE_ON;
	else
	if (sText == "NOTE_OFF")
		return NOTE_OFF;
	else
	if (sText == "KEY_PRESS")
		return KEY_PRESS;
	else
	if (sText == "CONTROLLER")
		return CONTROLLER;
	else
	if (sText == "PGM_CHANGE")
		return PGM_CHANGE;
	else
	if (sText == "CHAN_PRESS")
		return CHAN_PRESS;
	else
	if (sText == "PITCH_BEND")
		return PITCH_BEND;
	else
		return ControlType(0);
}

QString qmidictlMidiControl::textFromType (
	qmidictlMidiControl::ControlType ctype )
{
	QString sText;

	switch (ctype) {
	case MMC:
		sText = "MMC";
		break;
	case NOTE_ON:
		sText = "NOTE_ON";
		break;
	case NOTE_OFF:
		sText = "NOTE_OFF";
		break;
	case KEY_PRESS:
		sText = "KEY_PRESS";
		break;
	case CONTROLLER:
		sText = "CONTROLLER";
		break;
	case PGM_CHANGE:
		sText = "PGM_CHANGE";
		break;
	case CHAN_PRESS:
		sText = "CHAN_PRESS";
		break;
	case PITCH_BEND:
		sText = "PITCH_BEND";
		break;
	}

	return sText;
}


unsigned short qmidictlMidiControl::keyFromText ( const QString& sText )
{
	if (sText == "TrackParam" || sText == "*" || sText.isEmpty())
		return TrackParam;
	else
		return sText.toUShort();
}

QString qmidictlMidiControl::textFromKey ( unsigned short iKey )
{
	if (iKey & TrackParam)
		return "*"; // "TrackParam";
	else
		return QString::number(iKey);
}


qmidictlMidiControl::Command qmidictlMidiControl::commandFromText (
	const QString& sText )
{
	if (sText == "RST")
		return RST;
	else
	if (sText == "REW")
		return REW;
	else
	if (sText == "STOP")
		return STOP;
	else
	if (sText == "PLAY")
		return PLAY;
	else
	if (sText == "REC")
		return REC;
	else
	if (sText == "FFWD")
		return FFWD;
	else
	if (sText == "JOG_WHEEL")
		return JOG_WHEEL;
	else
	if (sText == "TRACK_SOLO")
		return TRACK_SOLO;
	else
	if (sText == "TRACK_MUTE")
		return TRACK_MUTE;
	else
	if (sText == "TRACK_REC")
		return TRACK_REC;
	else
	if (sText == "TRACK_VOL")
		return TRACK_VOL;
	else
		return Command(0);
}

QString qmidictlMidiControl::textFromCommand (
	qmidictlMidiControl::Command command )
{
	QString sText;

	switch (command) {
	case RST:
		sText = "RST";
		break;
	case REW:
		sText = "REW";
		break;
	case STOP:
		sText = "STOP";
		break;
	case PLAY:
		sText = "PLAY";
		break;
	case REC:
		sText = "REC";
		break;
	case FFWD:
		sText = "FFWD";
		break;
	case JOG_WHEEL:
		sText = "JOG_WHEEL";
		break;
	case TRACK_SOLO:
		sText = "TRACK_SOLO";
		break;
	case TRACK_MUTE:
		sText = "TRACK_MUTE";
		break;
	case TRACK_REC:
		sText = "TRACK_REC";
		break;
	case TRACK_VOL:
		sText = "TRACK_VOL";
		break;
	}

	return sText;
}


// end of qmidictlMidiControl.cpp
