// qmidictlMidiControl.cpp
//
/****************************************************************************
   Copyright (C) 2010-2019, rncbc aka Rui Nuno Capela. All rights reserved.

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

// Translatable macro contextualizer.
#undef  _TR
#define _TR(x) QT_TR_NOOP(x)


//----------------------------------------------------------------------
// qmidictlMidiControl -- MIDI control map (singleton).
//

// Kind of singleton reference.
qmidictlMidiControl* qmidictlMidiControl::g_pMidiControl = nullptr;

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
	g_pMidiControl = nullptr;
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

	mapCommand(TRACK_SOLO, MMC, 0, int(TRACK_SOLO));
	mapCommand(TRACK_MUTE, MMC, 0, int(TRACK_MUTE));
	mapCommand(TRACK_REC,  MMC, 0, int(TRACK_REC));

	mapCommand(TRACK_VOL,  CONTROLLER, 15, TrackParam | 0x40);

	mapCommand(JOG_WHEEL,  MMC, 0, int(JOG_WHEEL));
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
#if 0
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
#else
	ControlMap::ConstIterator iter = m_controlMap.constBegin();
	for ( ; iter != m_controlMap.constEnd(); ++iter) {
		const MapKey& key = iter.key();
		if (key.match(ctype, iChannel, iParam))
			break;
	}
	return iter;
#endif
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
	if (sText == "JOG_WHEEL")
		return JOG_WHEEL;
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
	case JOG_WHEEL:
		sText = "JOG_WHEEL";
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
	{  0, _TR("C")     },
	{  1, _TR("C#/Db") },
	{  2, _TR("D")     },
	{  3, _TR("D#/Eb") },
	{  4, _TR("E")     },
	{  5, _TR("F")     },
	{  6, _TR("F#/Gb") },
	{  7, _TR("G")     },
	{  8, _TR("G#/Ab") },
	{  9, _TR("A")     },
	{ 10, _TR("A#/Bb") },
	{ 11, _TR("B")     },

	// GM Drum note map...
	{ 35, _TR("Acoustic Bass Drum") },
	{ 36, _TR("Bass Drum 1") },
	{ 37, _TR("Side Stick") },
	{ 38, _TR("Acoustic Snare") },
	{ 39, _TR("Hand Clap") },
	{ 40, _TR("Electric Snare") },
	{ 41, _TR("Low Floor Tom") },
	{ 42, _TR("Closed Hi-Hat") },
	{ 43, _TR("High Floor Tom") },
	{ 44, _TR("Pedal Hi-Hat") },
	{ 45, _TR("Low Tom") },
	{ 46, _TR("Open Hi-Hat") },
	{ 47, _TR("Low-Mid Tom") },
	{ 48, _TR("Hi-Mid Tom") },
	{ 49, _TR("Crash Cymbal 1") },
	{ 50, _TR("High Tom") },
	{ 51, _TR("Ride Cymbal 1") },
	{ 52, _TR("Chinese Cymbal") },
	{ 53, _TR("Ride Bell") },
	{ 54, _TR("Tambourine") },
	{ 55, _TR("Splash Cymbal") },
	{ 56, _TR("Cowbell") },
	{ 57, _TR("Crash Cymbal 2") },
	{ 58, _TR("Vibraslap") },
	{ 59, _TR("Ride Cymbal 2") },
	{ 60, _TR("Hi Bongo") },
	{ 61, _TR("Low Bongo") },
	{ 62, _TR("Mute Hi Conga") },
	{ 63, _TR("Open Hi Conga") },
	{ 64, _TR("Low Conga") },
	{ 65, _TR("High Timbale") },
	{ 66, _TR("Low Timbale") },
	{ 67, _TR("High Agogo") },
	{ 68, _TR("Low Agogo") },
	{ 69, _TR("Cabasa") },
	{ 70, _TR("Maracas") },
	{ 71, _TR("Short Whistle") },
	{ 72, _TR("Long Whistle") },
	{ 73, _TR("Short Guiro") },
	{ 74, _TR("Long Guiro") },
	{ 75, _TR("Claves") },
	{ 76, _TR("Hi Wood Block") },
	{ 77, _TR("Low Wood Block") },
	{ 78, _TR("Mute Cuica") },
	{ 79, _TR("Open Cuica") },
	{ 80, _TR("Mute Triangle") },
	{ 81, _TR("Open Triangle") },

	{  0, nullptr }
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
				g_noteNames.insert(g_aNoteNames[i].note,
					QObject::tr(g_aNoteNames[i].name, "noteName"));
			}
		}
		// Check whether the drum note exists...
		QHash<unsigned short, QString>::ConstIterator iter
			= g_noteNames.constFind(iParam);
		if (iter != g_noteNames.constEnd())
			return iter.value();
	}

	return QObject::tr(g_aNoteNames[iParam % 12].name, "noteName")
		+ QString::number((iParam / 12) - 2);
}


//----------------------------------------------------------------------------
// MIDI Controller Names - Default controller names hash map.

static struct
{
	unsigned short controller;
	const char *name;

} g_aControllerNames[] = {

	{   0, _TR("Bank Select (coarse)") },
	{   1, _TR("Modulation Wheel (coarse)") },
	{   2, _TR("Breath Controller (coarse)") },
	{   4, _TR("Foot Pedal (coarse)") },
	{   5, _TR("Portamento Time (coarse)") },
	{   6, _TR("Data Entry (coarse)") },
	{   7, _TR("Volume (coarse)") },
	{   8, _TR("Balance (coarse)") },
	{  10, _TR("Pan Position (coarse)") },
	{  11, _TR("Expression (coarse)") },
	{  12, _TR("Effect Control 1 (coarse)") },
	{  13, _TR("Effect Control 2 (coarse)") },
	{  16, _TR("General Purpose Slider 1") },
	{  17, _TR("General Purpose Slider 2") },
	{  18, _TR("General Purpose Slider 3") },
	{  19, _TR("General Purpose Slider 4") },
	{  32, _TR("Bank Select (fine)") },
	{  33, _TR("Modulation Wheel (fine)") },
	{  34, _TR("Breath Controller (fine)") },
	{  36, _TR("Foot Pedal (fine)") },
	{  37, _TR("Portamento Time (fine)") },
	{  38, _TR("Data Entry (fine)") },
	{  39, _TR("Volume (fine)") },
	{  40, _TR("Balance (fine)") },
	{  42, _TR("Pan Position (fine)") },
	{  43, _TR("Expression (fine)") },
	{  44, _TR("Effect Control 1 (fine)") },
	{  45, _TR("Effect Control 2 (fine)") },
	{  64, _TR("Hold Pedal (on/off)") },
	{  65, _TR("Portamento (on/off)") },
	{  66, _TR("Sustenuto Pedal (on/off)") },
	{  67, _TR("Soft Pedal (on/off)") },
	{  68, _TR("Legato Pedal (on/off)") },
	{  69, _TR("Hold 2 Pedal (on/off)") },
	{  70, _TR("Sound Variation") },
	{  71, _TR("Sound Timbre") },
	{  72, _TR("Sound Release Time") },
	{  73, _TR("Sound Attack Time") },
	{  74, _TR("Sound Brightness") },
	{  75, _TR("Sound Control 6") },
	{  76, _TR("Sound Control 7") },
	{  77, _TR("Sound Control 8") },
	{  78, _TR("Sound Control 9") },
	{  79, _TR("Sound Control 10") },
	{  80, _TR("General Purpose Button 1 (on/off)") },
	{  81, _TR("General Purpose Button 2 (on/off)") },
	{  82, _TR("General Purpose Button 3 (on/off)") },
	{  83, _TR("General Purpose Button 4 (on/off)") },
	{  91, _TR("Effects Level") },
	{  92, _TR("Tremulo Level") },
	{  93, _TR("Chorus Level") },
	{  94, _TR("Celeste Level") },
	{  95, _TR("Phaser Level") },
	{  96, _TR("Data Button Increment") },
	{  97, _TR("Data Button Decrement") },
	{  98, _TR("Non-Registered Parameter (fine)") },
	{  99, _TR("Non-Registered Parameter (coarse)") },
	{ 100, _TR("Registered Parameter (fine)") },
	{ 101, _TR("Registered Parameter (coarse)") },
	{ 120, _TR("All Sound Off") },
	{ 121, _TR("All Controllers Off") },
	{ 122, _TR("Local Keyboard (on/off)") },
	{ 123, _TR("All Notes Off") },
	{ 124, _TR("Omni Mode Off") },
	{ 125, _TR("Omni Mode On") },
	{ 126, _TR("Mono Operation") },
	{ 127, _TR("Poly Operation") },

	{   0, nullptr }
};

static QHash<unsigned short, QString> g_controllerNames;

// Default controller name accessor.
const QString& qmidictlMidiControl::controllerName (
	unsigned short iParam )
{
	if (g_controllerNames.isEmpty()) {
		// Pre-load controller-names hash table...
		for (int i = 0; g_aControllerNames[i].name; ++i) {
			g_controllerNames.insert(g_aControllerNames[i].controller,
				QObject::tr(g_aControllerNames[i].name, "controllerName"));
		}
	}

	return g_controllerNames[iParam];
}


// end of qmidictlMidiControl.cpp
