// qmidictlMidiControl.h
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

#ifndef __qmidictlMidiControl_h
#define __qmidictlMidiControl_h

#include <QHash>

// Forward declarations.
class QSettings;


//----------------------------------------------------------------------
// qmidictlMidiControl -- MIDI control map (singleton).
//

class qmidictlMidiControl
{
public:

	// Controller types.
	enum ControlType
	{
		MMC        = 1,
		NOTE_ON    = 2,
		NOTE_OFF   = 3,
		KEY_PRESS  = 4,
		CONTROLLER = 5,
		PGM_CHANGE = 6,
		CHAN_PRESS = 7,
		PITCH_BEND = 8
	};

	// Command types.
	enum Command
	{
		RST        = 1,
		REW        = 2,
		STOP       = 3,
		PLAY       = 4,
		REC        = 5,
		FFWD       = 6,
		JOG_WHEEL  = 7,
		TRACK_SOLO = 8,
		TRACK_MUTE = 9,
		TRACK_REC  = 10,
		TRACK_VOL  = 11
	};

	// Key param masks (wildcard flags).
	enum { TrackParam = 0x4000, TrackParamMask = 0x3fff };

	// MIDI control map key.
	class MapKey
	{
	public:

		// Constructor.
		MapKey(ControlType ctype = MMC,
			unsigned short iChannel = 0, unsigned short iParam = 0)
			: m_ctype(ctype), m_iChannel(iChannel), m_iParam(iParam) {}

		// Type accessors.
		void setType(ControlType ctype)
			{ m_ctype = ctype; }
		ControlType type() const
			{ return m_ctype; }

		// Channel accessors.
		void setChannel(unsigned short iChannel)
			{ m_iChannel = iChannel; }
		unsigned short channel() const
			{ return m_iChannel; }

		bool isChannel() const
			{ return ((m_iChannel & TrackParamMask) == m_iChannel); }
		bool isChannelTrack() const
			{ return (m_iChannel & TrackParam); }

		// Parameter accessors.
		void setParam (unsigned short iParam)
			{ m_iParam = iParam; }
		unsigned short param() const
			{ return m_iParam; }

		bool isParam() const
			{ return ((m_iParam & TrackParamMask) == m_iParam); }
		bool isParamTrack() const
			{ return (m_iParam & TrackParam); }

		// Generic key matcher.
		bool match (ControlType ctype,
			unsigned short iChannel, unsigned short iParam) const
		{
			return (type() == ctype 
				&& (isChannelTrack() || channel() == iChannel)
				&& (isParamTrack() || param() == iParam));
		}

		// Hash key comparator.
		bool operator== ( const MapKey& key ) const
		{
			return (key.m_ctype == m_ctype)
				&& (key.m_iChannel == m_iChannel)
				&& (key.m_iParam == m_iParam);
		}

	private:

		// Instance (key) member variables.
		ControlType    m_ctype;
		unsigned short m_iChannel;
		unsigned short m_iParam;
	};

	// MIDI command/control map type.
	typedef QHash<Command, MapKey> CommandMap;
	typedef QHash<MapKey, Command> ControlMap;

	// Constructor.
	qmidictlMidiControl();

	// Destructor.
	~qmidictlMidiControl();

	static qmidictlMidiControl *getInstance();

	// Clear control/command map (reset to default).
	void clear();

	// Map accessors.
	const CommandMap& commandMap() const;
	const ControlMap& controlMap() const;

	// Insert new controller mappings.
	void mapCommand(Command command, ControlType ctype,
		unsigned short iChannel, unsigned short iParam);

	// Remove existing controller mapping.
	void unmapCommand(Command command);

	// Check if given command is currently mapped.
	bool isCommandMapped(Command command) const;

	// Check if given channel, controller pair is currently mapped.
	bool isChannelParamMapped(ControlType ctype,
		unsigned short iChannel, unsigned short iParam) const;

	// Load from/save into global settings.
	void load(QSettings& settings);
	void save(QSettings& settings);

	// Textual helpers.
	static ControlType typeFromText(const QString& sText);
	static QString textFromType(ControlType ctype);

	static unsigned short keyFromText(const QString& sText);
	static QString textFromKey(unsigned short iKey);

	static Command commandFromText(const QString& sText);
	static QString textFromCommand(Command command);

private:

	// Command/control map.
	CommandMap m_commandMap;
	ControlMap m_controlMap;

	// Pseudo-singleton instance.
	static qmidictlMidiControl *g_pMidiControl;
};


// Hash key functions.
inline uint qHash ( const qmidictlMidiControl::Command& command )
{
	return qHash(uint(command));
}

inline uint qHash ( const qmidictlMidiControl::MapKey& key )
{
	return qHash(uint(key.type()) ^ key.channel() ^ key.param());
}


#endif  // __qmidictlMidiControl_h

// end of qmidictlMidiControl.h
