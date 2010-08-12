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
	ControlType ctype, unsigned short iChannel, unsigned short iParam,
	bool bLogarithmic )
{
	MapKey key(ctype, iChannel, iParam, bLogarithmic);

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

	ControlMap::ConstIterator iter = m_controlMap.constFind(key);
	if (iter == m_controlMap.constEnd()) {
		key.setChannel(TrackParam);
		iter = m_controlMap.constFind(key);
		if (iter == m_controlMap.constEnd()) {
			key.setChannel(iChannel); // Fallback to original channel.
			key.setParam(iParam | TrackParam);
			iter = m_controlMap.constFind(key);
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
		sValue += ',' + QString::number(int(key.isLogarithmic()));
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
		bool bLogarithmic = (sValue.section(',', 3, 3).toInt() > 0);
		mapCommand(command, ctype, iChannel, iParam, bLogarithmic);
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


//----------------------------------------------------------------------------
// MIDI Note Names - Default note names hash map.

static struct
{
	unsigned short note;
	const char *name;

} g_aNoteNames[] = {

	// Diatonic note map...
	{  0, QT_TR_NOOP("C")  },
	{  1, QT_TR_NOOP("C#") },
	{  2, QT_TR_NOOP("D")  },
	{  3, QT_TR_NOOP("D#") },
	{  4, QT_TR_NOOP("E")  },
	{  5, QT_TR_NOOP("F")  },
	{  6, QT_TR_NOOP("F#") },
	{  7, QT_TR_NOOP("G")  },
	{  8, QT_TR_NOOP("G#") },
	{  9, QT_TR_NOOP("A")  },
	{ 10, QT_TR_NOOP("A#") },
	{ 11, QT_TR_NOOP("B")  },

	// GM Drum note map...
	{ 35, QT_TR_NOOP("Acoustic Bass Drum") },
	{ 36, QT_TR_NOOP("Bass Drum 1") },
	{ 37, QT_TR_NOOP("Side Stick") },
	{ 38, QT_TR_NOOP("Acoustic Snare") },
	{ 39, QT_TR_NOOP("Hand Clap") },
	{ 40, QT_TR_NOOP("Electric Snare") },
	{ 41, QT_TR_NOOP("Low Floor Tom") },
	{ 42, QT_TR_NOOP("Closed Hi-Hat") },
	{ 43, QT_TR_NOOP("High Floor Tom") },
	{ 44, QT_TR_NOOP("Pedal Hi-Hat") },
	{ 45, QT_TR_NOOP("Low Tom") },
	{ 46, QT_TR_NOOP("Open Hi-Hat") },
	{ 47, QT_TR_NOOP("Low-Mid Tom") },
	{ 48, QT_TR_NOOP("Hi-Mid Tom") },
	{ 49, QT_TR_NOOP("Crash Cymbal 1") },
	{ 50, QT_TR_NOOP("High Tom") },
	{ 51, QT_TR_NOOP("Ride Cymbal 1") },
	{ 52, QT_TR_NOOP("Chinese Cymbal") },
	{ 53, QT_TR_NOOP("Ride Bell") },
	{ 54, QT_TR_NOOP("Tambourine") },
	{ 55, QT_TR_NOOP("Splash Cymbal") },
	{ 56, QT_TR_NOOP("Cowbell") },
	{ 57, QT_TR_NOOP("Crash Cymbal 2") },
	{ 58, QT_TR_NOOP("Vibraslap") },
	{ 59, QT_TR_NOOP("Ride Cymbal 2") },
	{ 60, QT_TR_NOOP("Hi Bongo") },
	{ 61, QT_TR_NOOP("Low Bongo") },
	{ 62, QT_TR_NOOP("Mute Hi Conga") },
	{ 63, QT_TR_NOOP("Open Hi Conga") },
	{ 64, QT_TR_NOOP("Low Conga") },
	{ 65, QT_TR_NOOP("High Timbale") },
	{ 66, QT_TR_NOOP("Low Timbale") },
	{ 67, QT_TR_NOOP("High Agogo") },
	{ 68, QT_TR_NOOP("Low Agogo") },
	{ 69, QT_TR_NOOP("Cabasa") },
	{ 70, QT_TR_NOOP("Maracas") },
	{ 71, QT_TR_NOOP("Short Whistle") },
	{ 72, QT_TR_NOOP("Long Whistle") },
	{ 73, QT_TR_NOOP("Short Guiro") },
	{ 74, QT_TR_NOOP("Long Guiro") },
	{ 75, QT_TR_NOOP("Claves") },
	{ 76, QT_TR_NOOP("Hi Wood Block") },
	{ 77, QT_TR_NOOP("Low Wood Block") },
	{ 78, QT_TR_NOOP("Mute Cuica") },
	{ 79, QT_TR_NOOP("Open Cuica") },
	{ 80, QT_TR_NOOP("Mute Triangle") },
	{ 81, QT_TR_NOOP("Open Triangle") },

	{  0, NULL }
};

static QHash<unsigned short, QString> g_noteNames;

// Default note name map accessor.
const QString qmidictlMidiControl::noteName (
	unsigned short iParam, bool fDrums )
{
	if (fDrums) {
		// Pre-load drum-names hash table...
		if (g_noteNames.isEmpty()) {
			for (int i = 12; g_aNoteNames[i].name; ++i) {
				g_noteNames.insert(
					g_aNoteNames[i].note,
					QObject::tr(g_aNoteNames[i].name));
			}
		}
		// Check whether the drum note exists...
		QHash<unsigned short, QString>::ConstIterator iter
			= g_noteNames.constFind(iParam);
		if (iter != g_noteNames.constEnd())
			return iter.value();
	}

	return QObject::tr(g_aNoteNames[iParam % 12].name)
		+ QString::number((iParam / 12) - 2);
}


//----------------------------------------------------------------------------
// MIDI Controller Names - Default controller names hash map.

static struct
{
	unsigned short controller;
	const char *name;

} g_aControllerNames[] = {

	{   0, QT_TR_NOOP("Bank Select (coarse)") },
	{   1, QT_TR_NOOP("Modulation Wheel (coarse)") },
	{   2, QT_TR_NOOP("Breath Controller (coarse)") },
	{   4, QT_TR_NOOP("Foot Pedal (coarse)") },
	{   5, QT_TR_NOOP("Portamento Time (coarse)") },
	{   6, QT_TR_NOOP("Data Entry (coarse)") },
	{   7, QT_TR_NOOP("Volume (coarse)") },
	{   8, QT_TR_NOOP("Balance (coarse)") },
	{  10, QT_TR_NOOP("Pan Position (coarse)") },
	{  11, QT_TR_NOOP("Expression (coarse)") },
	{  12, QT_TR_NOOP("Effect Control 1 (coarse)") },
	{  13, QT_TR_NOOP("Effect Control 2 (coarse)") },
	{  16, QT_TR_NOOP("General Purpose Slider 1") },
	{  17, QT_TR_NOOP("General Purpose Slider 2") },
	{  18, QT_TR_NOOP("General Purpose Slider 3") },
	{  19, QT_TR_NOOP("General Purpose Slider 4") },
	{  32, QT_TR_NOOP("Bank Select (fine)") },
	{  33, QT_TR_NOOP("Modulation Wheel (fine)") },
	{  34, QT_TR_NOOP("Breath Controller (fine)") },
	{  36, QT_TR_NOOP("Foot Pedal (fine)") },
	{  37, QT_TR_NOOP("Portamento Time (fine)") },
	{  38, QT_TR_NOOP("Data Entry (fine)") },
	{  39, QT_TR_NOOP("Volume (fine)") },
	{  40, QT_TR_NOOP("Balance (fine)") },
	{  42, QT_TR_NOOP("Pan Position (fine)") },
	{  43, QT_TR_NOOP("Expression (fine)") },
	{  44, QT_TR_NOOP("Effect Control 1 (fine)") },
	{  45, QT_TR_NOOP("Effect Control 2 (fine)") },
	{  64, QT_TR_NOOP("Hold Pedal (on/off)") },
	{  65, QT_TR_NOOP("Portamento (on/off)") },
	{  66, QT_TR_NOOP("Sustenuto Pedal (on/off)") },
	{  67, QT_TR_NOOP("Soft Pedal (on/off)") },
	{  68, QT_TR_NOOP("Legato Pedal (on/off)") },
	{  69, QT_TR_NOOP("Hold 2 Pedal (on/off)") },
	{  70, QT_TR_NOOP("Sound Variation") },
	{  71, QT_TR_NOOP("Sound Timbre") },
	{  72, QT_TR_NOOP("Sound Release Time") },
	{  73, QT_TR_NOOP("Sound Attack Time") },
	{  74, QT_TR_NOOP("Sound Brightness") },
	{  75, QT_TR_NOOP("Sound Control 6") },
	{  76, QT_TR_NOOP("Sound Control 7") },
	{  77, QT_TR_NOOP("Sound Control 8") },
	{  78, QT_TR_NOOP("Sound Control 9") },
	{  79, QT_TR_NOOP("Sound Control 10") },
	{  80, QT_TR_NOOP("General Purpose Button 1 (on/off)") },
	{  81, QT_TR_NOOP("General Purpose Button 2 (on/off)") },
	{  82, QT_TR_NOOP("General Purpose Button 3 (on/off)") },
	{  83, QT_TR_NOOP("General Purpose Button 4 (on/off)") },
	{  91, QT_TR_NOOP("Effects Level") },
	{  92, QT_TR_NOOP("Tremulo Level") },
	{  93, QT_TR_NOOP("Chorus Level") },
	{  94, QT_TR_NOOP("Celeste Level") },
	{  95, QT_TR_NOOP("Phaser Level") },
	{  96, QT_TR_NOOP("Data Button Increment") },
	{  97, QT_TR_NOOP("Data Button Decrement") },
	{  98, QT_TR_NOOP("Non-Registered Parameter (fine)") },
	{  99, QT_TR_NOOP("Non-Registered Parameter (coarse)") },
	{ 100, QT_TR_NOOP("Registered Parameter (fine)") },
	{ 101, QT_TR_NOOP("Registered Parameter (coarse)") },
	{ 120, QT_TR_NOOP("All Sound Off") },
	{ 121, QT_TR_NOOP("All Controllers Off") },
	{ 122, QT_TR_NOOP("Local Keyboard (on/off)") },
	{ 123, QT_TR_NOOP("All Notes Off") },
	{ 124, QT_TR_NOOP("Omni Mode Off") },
	{ 125, QT_TR_NOOP("Omni Mode On") },
	{ 126, QT_TR_NOOP("Mono Operation") },
	{ 127, QT_TR_NOOP("Poly Operation") },

	{   0, NULL }
};

static QHash<unsigned short, QString> g_controllerNames;

// Default controller name accessor.
const QString& qmidictlMidiControl::controllerName (
	unsigned short iParam )
{
	if (g_controllerNames.isEmpty()) {
		// Pre-load controller-names hash table...
		for (int i = 0; g_aControllerNames[i].name; ++i) {
			g_controllerNames.insert(
				g_aControllerNames[i].controller,
				QObject::tr(g_aControllerNames[i].name));
		}
	}

	return g_controllerNames[iParam];
}


// end of qmidictlMidiControl.cpp
