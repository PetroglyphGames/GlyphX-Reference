

#ifndef STORYSUBPLOT_H
#define STORYSUBPLOT_H


#include "StoryEvent.h"
#include "PGSignal/SignalGenerator.h"


class PlayerClass;
class LuaScriptClass;
class StoryModeClass;


#define UNDEFINED_STORY_FLAG -99999999

class StorySubPlotClass : public SignalGeneratorClass
{
public:

	StorySubPlotClass(const char *name, bool load_plot = true);
	~StorySubPlotClass();

	typedef stdext::hash_map<CRCValue, StoryEventClass *> StoryEventListType;

	void Set_Story_Mode(StoryModeClass *storymode) { StoryMode = storymode; }
	StoryModeClass *Get_Story_Mode() { return (StoryMode); }

	StoryEventClass *Get_Event(const char *name);
	StoryEventListType *Get_All_Events() { return (&StoryEvents); }
	void Add_Event(CRCValue crc, StoryEventClass *event) { StoryEvents[crc] = event; }
	void Sort_Events_And_Compute_Dependants();

	const std::string &Get_Name() { return (Name); }
	bool Is_Active() { return (Active); }
	void Set_Active(bool onoff) { Active = onoff; }

	void Set_Local_Player(PlayerClass *player) { LocalPlayer = player; }
	PlayerClass *Get_Local_Player() { return (LocalPlayer); }

	bool Attach_Lua_Script(const std::string &name, bool is_load = false, bool reload = false);
	bool Reload_Script(std::vector<std::string> &file_list);
	void Lua_Trigger_Event(const std::string &event);
	void Lua_Script_Service(void);
	LuaScriptClass *Get_Lua_Script(void) const { return LuaScript; }

	// Game events
	void Story_Event(StoryEventEnum event, PlayerClass *player, void *param1, void *param2);
	void Replace_Variable(const std::string &var_name, const std::string &new_name);
	void Check_Inactive(float elapsed);
	void Check_Flags();
	bool Check_Special_Land_Tactical_Map(GameObjectClass *hero, GameObjectClass *planet, bool check_land_only);
	bool Check_Planet_Entry_Restrictions(GameObjectClass *fleet, GameObjectClass *planet);
	bool Check_Special_Space_Tactical_Map(GameObjectClass *planet, GameObjectClass *hero = NULL);
	void Check_Timeout(float elapsed);

	void Planet_Destroyed(const std::string &planet_name);

	void Speech_Killed();

	void Disable_Branch(const char *branch, bool onoff);
	void Reset_Branch(const char *branch);
	void Reset_Event(const char *event_name);
	void Reset_All_Events();

	bool Load(ChunkReaderClass *reader);
	bool Save(ChunkWriterClass *writer);

	// Used to check timeout
	void Add_Timout_Event(StoryEventClass *event);
	void Remove_Timeout_Event(StoryEventClass *event);

	// Debugging
	void Dump_Status();
	void Dump_Status_Filtered(bool active, bool triggered, bool disabled);
#ifndef NDEBUG
	void Check_For_Null_Events();
#endif
	void Determine_Null_Event(CRCValue crc_index);

	void Trigger_Event(const char *event_name);
	void Disable_Event(const char *event_name, bool onoff);


private:

	bool Is_Event_Active(StoryEventClass *event);

	StoryEventListType StoryEvents;													// All events
	DynamicVectorClass<StoryEventClass *> SortedEvents[STORY_COUNT];		// All events sorted by type
	SmartPtr<LuaScriptClass> LuaScript;
	DynamicVectorClass<StoryEventClass *> TimeoutEvents;						// Some events timeout after awhile

	StoryModeClass *StoryMode;
	PlayerClass *LocalPlayer;
	std::string Name;
	bool Active;
};

















#endif
