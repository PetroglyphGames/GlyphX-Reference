//////////////////////////////////////////////////////////////////////////////////////////////////
//
// (C) Petroglyph Games, LLC
//
//
//  *****           **                          *                   *
//  *   **          *                           *                   *
//  *    *          *                           *                   *
//  *    *          *     *                 *   *          *        *
//  *   *     *** ******  * **  ****      ***   * *      * *****    * ***
//  *  **    *  *   *     **   *   **   **  *   *  *    * **   **   **   *
//  ***     *****   *     *   *     *  *    *   *  *   **  *    *   *    *
//  *       *       *     *   *     *  *    *   *   *  *   *    *   *    *
//  *       *       *     *   *     *  *    *   *   * **   *   *    *    *
//  *       **       *    *   **   *   **   *   *    **    *  *     *   *
// **        ****     **  *    ****     *****   *    **    ***      *   *
//                                          *        *     *
//                                          *        *     *
//                                          *       *      *
//                                      *  *        *      *
//                                      ****       *       *
//
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/StoryMode/StoryEvent.h $
//
//             Author: Jason Curtice
//
//               Date: 07/21/2004 6:35PM
//
///////////////////////////////////////////////////////////////////////////////////////////////////




#ifndef STORYEVENT_H
#define STORYEVENT_H


#include "vector.h"
#include "CRC.h"
#include "XML.h"
#include "Vector3.h"
#include "ShipClassType.h"
#include "PGSignal/SignalGenerator.h"
#include "CorruptionType.h"



class GameObjectClass;
class StoryEventClass;
class StorySubPlotClass;
class ChunkReaderClass;
class ChunkWriterClass;
enum DifficultyLevelType;

	
enum StoryEventEnum
{
	STORY_NONE,
	STORY_ENTER,
	STORY_LAND_ON,
	STORY_CONQUER,
	STORY_CONSTRUCT,
	STORY_CONSTRUCT_LEVEL,
	STORY_DESTROY,
	STORY_DESTROY_BASE,
	STORY_TACTICAL_DESTROY,
	STORY_BEGIN_ERA,
	STORY_TECH_LEVEL,
	STORY_POLITICAL_CONTROL,
	STORY_DEPLOY,
	STORY_MOVE,
	STORY_ACCUMULATE,
	STORY_CONQUER_COUNT,
	STORY_ELAPSED,
	STORY_CAPTURE_HERO,
	STORY_DEFEAT_HERO,
	STORY_WIN_BATTLES,
	STORY_LOSE_BATTLES,
	STORY_RETREAT,
	STORY_REINFORCE,
	STORY_PLANET_DESTROYED,

	STORY_SPACE_TACTICAL,
	STORY_LAND_TACTICAL,
	STORY_SELECT_PLANET,
	STORY_ZOOM_INTO_PLANET,
	STORY_ZOOM_OUT_PLANET,
	STORY_CLICK_GUI,
	STORY_SPEECH_DONE,
	STORY_FOG_OBJECT_REVEAL,
	STORY_FOG_POSITION_REVEAL,
	STORY_GENERIC,
	STORY_TRIGGER,				// Generic trigger event
	STORY_AI_NOTIFICATION,
	STORY_SELECT_UNIT,
	STORY_COMMAND_UNIT,
	STORY_UNIT_ARRIVED,
	STORY_FULL_STOP,
	STORY_ATTACK_HARDPOINT,
	STORY_GUARD_UNIT,
	STORY_UNIT_PROXIMITY,
	STORY_DIFFICULTY_LEVEL,
	STORY_FLAG,
	STORY_LOAD_TACTICAL_MAP,
	STORY_CHECK_DESTROYED,
	STORY_VICTORY,
	STORY_MOVIE_DONE,
	STORY_FLEET_BOUNCED,
	STORY_MISSION_LOST,
	STORY_INVASION_BOUNCED,
	STORY_OBJECTIVE_TIMEOUT,
	STORY_CAPTURE_STRUCTURE,

	// New for expansion
	STORY_CORRUPTION_CHANGED,
	STORY_GARRISON_UNIT,
	STORY_BUY_BLACK_MARKET,
	STORY_GALACTIC_SABOTAGE,
	STORY_CORRUPTION_TACTICAL_COMPLETE,
	STORY_CORRUPTION_TACTICAL_FAILED,
	STORY_OPEN_CORRUPTION,

	STORY_COUNT
};



enum StoryRewardEnum
{
	REWARD_NONE,
	REWARD_BUILDABLE_UNIT,
	REWARD_UNIQUE_UNIT,
	REWARD_REMOVE_UNIT,
	REWARD_CREDITS,
	REWARD_SPAWN_HERO,
	REWARD_INFORMATION,
	REWARD_STORY_ELEMENT,
	REWARD_STATISTIC_CHANGE,
	REWARD_SFX,
	REWARD_SPEECH,
	REWARD_FLASH_GUI,
	REWARD_FLASH_PLANET_GUI,
	REWARD_FLASH_PLANET,
	REWARD_HIDE_TUTORIAL_CURSOR,
	REWARD_SCREEN_TEXT,
	REWARD_DISABLE_EVENT,
	REWARD_ENABLE_EVENT,
	REWARD_PICK_PLANET,
	REWARD_SWITCH_SIDES,
	REWARD_ZOOM_IN,
	REWARD_ZOOM_OUT,
	REWARD_PAUSE_GAME,
	REWARD_TUTORIAL_DIALOG,
	REWARD_POSITION_CAMERA,
	REWARD_LOCK_CONTROLS,
	REWARD_SCROLL_CAMERA,
	REWARD_FLASH_OBJECT,
	REWARD_ENABLE_FOW,
	REWARD_ENABLE_VICTORY,
	REWARD_MOVE_FLEET,
	REWARD_TRIGGER_AI,
	REWARD_DISABLE_STORY_EVENT,
	REWARD_DISABLE_BRANCH,
	REWARD_INVADE_PLANET,
	REWARD_LOAD_CAMPAIGN,
	REWARD_FLASH_TERRAIN,
	REWARD_SET_HEALTH,
	REWARD_NEW_POWER_FOR_ALL,
	REWARD_REMOVE_POWER_FROM_ALL,
	REWARD_SET_TACTICAL_MAP,
	REWARD_DISABLE_AUTORESOLVE,
	REWARD_ENABLE_AUTORESOLVE,
	REWARD_DISABLE_MOVIES,
	REWARD_REMOVE_STORY_GOAL,
	REWARD_CHANGE_OWNER,
	REWARD_DESTROY_OBJECT,
	REWARD_VICTORY,
	REWARD_SWITCH_CONTROL,
	REWARD_DISABLE_RETREAT,
	REWARD_FLASH_UNIT,
	REWARD_REVEAL_PLANET,
	REWARD_DUAL_FLASH,
	REWARD_SET_FLAG,
	REWARD_INCREMENT_FLAG,
	REWARD_HILITE_OBJECT,
	REWARD_STORY_GOAL_COMPLETED,
	REWARD_REVEAL_ALL_PLANETS,
	REWARD_DISABLE_REINFORCEMENTS,
	REWARD_FLASH_RADAR,
	REWARD_RESET_BRANCH,
	REWARD_RESET_EVENT,
	REWARD_LINK_TACTICAL,
	REWARD_RANDOM_STORY,
	REWARD_DISABLE_SPECIAL_STRUCTURE,
	REWARD_SET_TECH_LEVEL,
	REWARD_DISABLE_SPAWN,
	REWARD_TRIGGER_EVENT,
	REWARD_ENABLE_GALACTIC_REVEAL,
	REWARD_ACTIVATE_RETRY_DIALOG,
	REWARD_SET_USE_RETRY_DIALOG,
	REWARD_START_MOVIE,
	REWARD_SET_PLANET_SPAWN,
	REWARD_COMMANDBAR_MOVIE,
	REWARD_STOP_COMMANDBAR_MOVIE,
	REWARD_SET_WEATHER,
	REWARD_PLANET_FACTION,
	REWARD_LOCK_UNIT,
	REWARD_FORCE_RETREAT,
	REWARD_SET_PLANET_RESTRICTED,
	REWARD_MULTIMEDIA,
	REWARD_SET_PLANET_VISIBILITY_LEVEL,
	REWARD_PAUSE_GALACTIC_GAME,
	REWARD_ADD_OBJECTIVE,
	REWARD_REMOVE_OBJECTIVE,
	REWARD_OBJECTIVE_COMPLETE,
	REWARD_REMOVE_ALL_OBJECTIVES,
	REWARD_OBJECTIVE_FAILED,
	REWARD_ENABLE_DIRECT_INVASION,
	REWARD_DISABLE_DIRECT_INVASION,
	REWARD_ENABLE_OBJECTIVE_DISPLAY,
	REWARD_FLASH_FLEET_WITH_UNIT,
	REWARD_SELECT_PLANET,
	REWARD_FORCE_CLICK_GUI,
	REWARD_FLASH_PRODUCTION_CHOICE,
	REWARD_SET_MAX_TECH_LEVEL,
	REWARD_TUTORIAL_PLAYER,
	REWARD_PAUSE_GALACTIC,
	REWARD_UNPAUSE_GALACTIC,
	REWARD_ENABLE_OVERWHELMING_ODDS,
	REWARD_FLASH_SPECIAL_ABILITY,
	REWARD_SET_SANDBOX_OBJECTIVES,
	REWARD_FORCE_RESPAWN,
	REWARD_SCROLL_LOCK,
	REWARD_SKIRMISH_RULES,
	REWARD_RESET_GALACTIC_FILTERS,
	REWARD_SET_ADVISOR,
	REWARD_START_CINEMATIC_MODE,
	REWARD_STOP_CINEMATIC_MODE,
	REWARD_SHOW_COMMAND_BAR,
	REWARD_HIDE_AUTORESOLVE,
	REWARD_DISABLE_BUILDABLE,
	REWARD_ENABLE_BUILDABLE,
	REWARD_HIDE_CURSOR_ON_CLICK,
	REWARD_LOCK_PLANET_SELECTION,
	REWARD_SHOW_SMUGGLE_SLOT,
	REWARD_HIDE_SMUGGLE_SLOT,
	REWARD_SHOW_RAID_SLOT,
	REWARD_HIDE_RAID_SLOT,
	REWARD_SHOW_STEAL_SLOT,
	REWARD_HIDE_STEAL_SLOT,
	REWARD_FINISHED_TUTORIAL,
	REWARD_BOMBARD_OVERRIDE_DELAY,
	REWARD_ENABLE_BOUNTY_COLLECTION,
	REWARD_DISABLE_BOUNTY_COLLECTION,
	REWARD_REMOVE_CORRUPTION,
	REWARD_GIVE_BLACK_MARKET,
	REWARD_SHOW_SPECIAL_SLOT,
	REWARD_SHOW_NEUTRALIZE_HERO_SLOT,
	REWARD_SABOTAGE_STRUCTURE,
	REWARD_ENABLE_SABOTAGE,
	REWARD_FLASH_ADVANCED_MAP_OBJECT,
	REWARD_ENABLE_INVASION,
	REWARD_RESTRICT_ALL_ABILITIES,
	REWARD_RESTRICT_CORRUPTION,
	REWARD_RESTRICT_BLACK_MARKET,
	REWARD_RESTRICT_SABOTAGE,
	REWARD_ENABLE_FLEET_COMBINE,
	REWARD_ENABLE_COMBAT_CINEMATIC,
	REWARD_RESTRICT_AUTORESOLVE,
	REWARD_ENABLE_GALACTIC_CORRUPTION_HOLOGRAM,
	REWARD_REPLACE_OBJECTIVE,
	REWARD_ENABLE_CAMPAIGN_VICTORY_MOVIE,

	REWARD_COUNT
};



enum StoryEventFilter
{
	EVENT_FILTER_NONE,

	EVENT_FILTER_FRIENDLY_ONLY,
	EVENT_FILTER_NEUTRAL_ONLY,
	EVENT_FILTER_ENEMY_ONLY,
	EVENT_FILTER_FRIENDLY_AND_NEUTRAL,
	EVENT_FILTER_ENEMY_AND_NEUTRAL,
	EVENT_FILTER_FRIENDLY_AND_ENEMY,

	EVENT_FILTER_COUNT
};



enum StoryBaseFilter
{
	BASE_GROUND,
	BASE_SPACE,
	BASE_EITHER,

	BASE_COUNT
};




enum StoryPlanetStats
{
	STATS_NONE,

	STATS_CREDIT_VALUE,
	STATS_POLITICAL_CONTROL,
	STATS_MAX_SPECIAL_STRUCTS,
	STATS_MAX_STARBASE_LEVEL,
	STATS_MAX_GROUND_BASE_LEVEL
};




struct StoryEnumRemapStruct
{
	const char *Text;
	int EnumValue;
};



enum StorySaveLoadEnum
{
	STORY_MODE_MICRO_DATA_CHUNK,
	STORY_MODE_THIS_ID_CHUNK,
	STORY_MODE_ALL_PLOTS_CHUNK,
	STORY_MODE_PARENT_TYPE_CHUNK,
	STORY_MODE_PLOT_DATA_CHUNK,
	STORY_MODE_PLOT_NAME_CHUNK,
	STORY_MODE_PLOT_CHUNK,
	STORY_MODE_SUB_PLOT_LIST_CHUNK,
	STORY_MODE_PLOT_COUNT_CHUNK,
	STORY_MODE_PLAYER_PLOT_FILE_PLAYER_CHUNK,
	STORY_MODE_PLAYER_PLOT_FILE_NAME_CHUNK,
	STORY_PLOT_DATA_CHUNK,
	STORY_PLOT_THIS_ID_CHUNK,
	STORY_PLOT_ACTIVE_CHUNK,
	STORY_PLOT_PLAYER_ID_CHUNK,
	STORY_PLOT_EVENT_DATA_CHUNK,
	STORY_PLOT_EVENT_NAME_CHUNK,
	STORY_PLOT_EVENT_CHUNK,
	STORY_PLOT_EVENT_LIST_CHUNK,
	STORY_EVENT_TRIGGERED_CHUNK,
	STORY_EVENT_ACTIVE_CHUNK,
	STORY_EVENT_OLD_ERA_CHUNK,
	STORY_EVENT_CUR_COUNT_CHUNK,
	STORY_EVENT_LAST_TIME_CHUNK,
	STORY_EVENT_ELAPSED_INITIALIZED_CHUNK,
	STORY_EVENT_BASE_CLASS_CHUNK,
	STORY_EVENT_DATA_CHUNK,
	STORY_EVENT_THIS_ID_CHUNK,
	STORY_MODE_VARIABLE_LIST_CHUNK,
	STORY_LUA_SCRIPT_NAME_CHUNK,
	STORY_LUA_SCRIPT_DATA_CHUNK,
	STORY_EVENT_COUNT_CHUNK,
	STORY_EVENT_EXTRA_STRING_CHUNK,
	STORY_EVENT_EXTRA_ID_CHUNK,
	STORY_EVENT_REWARD_PARAM_CHUNK,
	STORY_EVENT_REWARD_TYPE_CHUNK,
	STORY_EVENT_DIALOG_NAME_CHUNK,
	STORY_EVENT_OBJECT_NAME_CHUNK,
	STORY_EVENT_COUNT_COPY_CHUNK,
	STORY_EVENT_FILTER_CHUNK,
	STORY_EVENT_TRIGGER_STRING_CHUNK,
	STORY_EVENT_LEVEL_CHUNK,
	STORY_EVENT_ELAPSED_TIME_CHUNK,
	STORY_PLOT_TIMEOUT_DATA_CHUNK,
	STORY_PLOT_TIMEOUT_CHUNK,
	STORY_EVENT_TIMEOUT_START_CHUNK,
	STORY_EVENT_ALLOW_STEALTH_CHUNK,
	STORY_PLOT_EVENT_TYPE_CHUNK,
	STORY_EVENT_OBJECTIVE_NAME_CHUNK,
	STORY_MODE_ALL_OBJECTIVES_CHUNK,
	STORY_MODE_OBJECTIVE_TEXT_CHUNK,
	STORY_MODE_OBJECTIVE_DISPLAY_TEXT_CHUNK,
	STORY_MODE_OBJECTIVE_STATUS_CHUNK,
	STORY_MODE_OBJECTIVE_SUGGESTION_CHUNK,
	STORY_MODE_OBJECTIVE_NUMBER_CHUNK,
	STORY_MODE_CURRENT_OBJECTIVE_CHUNK,
	STORY_EVENT_DISABLED_CHUNK,
	STORY_MODE_OBJECTIVE_DISPLAY_NON_MICRO_TEXT_CHUNK
};





enum StoryCompareEnum
{
	COMPARE_NONE,

	COMPARE_GREATER_THAN,
	COMPARE_LESS_THAN,
	COMPARE_EQUAL_TO,
	COMPARE_NOT_EQUAL_TO,
	COMPARE_GREATER_THAN_EQUAL_TO,
	COMPARE_LESS_THAN_EQUAL_TO
};




enum FleetContentEnum
{
	FC_ANY,

	FC_FRIENDLY_CONTAINS,
	FC_FRIENDLY_ONLY_CONTAINS,
	FC_ENEMY_CONTAINS,
	FC_ENEMY_ONLY_CONTAINS
};



enum StoryDestroyEnum
{
	DESTROY_NONE,

	DESTROY_ALL,
	DESTROY_ALL_UNITS,
	DESTROY_ALL_STRUCTURES,
	DESTROY_ALL_INDIGENOUS_SPAWNERS,
};

enum StoryPostLinkedTacticalEnum
{
	POST_LINKED_TACTICAL_NONE,

	POST_LINKED_TACTICAL_RETREAT_PLAYER,
	POST_LINKED_TACTICAL_RETREAT_AI,
	POST_LINKED_TACTICAL_DESTROY_AI,
};







class StoryDatabaseParserClass
{
	friend class DatabaseMapClass;

public:

	StoryDatabaseParserClass();
	~StoryDatabaseParserClass() {}

	void Parse_Database_Entry( XMLDatabase *database_object_entry );
	StoryEventClass *Get_Event();
	static StoryEventClass *Create_Event(StoryEventEnum type);

private:

	std::string Name;
	std::string EventType;
	std::vector<std::string> EventParam1;
	std::vector<std::string> EventParam2;
	std::vector<std::string> EventParam3;
	std::vector<std::string> EventParam4;
	std::vector<std::string> EventParam5;
	std::vector<std::string> EventParam6;
	std::vector<std::string> EventParam7;
	std::string Filter;
	std::string RewardType;
	std::string RewardParam1;
	std::string RewardParam2;
	std::string RewardParam3;
	std::string RewardParam4;
	std::string RewardParam5;
	std::string RewardParam6;
	std::string RewardParam7;
	std::string RewardParam8;
	std::string RewardParam9;
	std::string RewardParam10;
	std::string RewardParam11;
	std::string RewardParam12;
	std::string RewardParam13;
	std::string RewardParam14;
	std::vector<std::string> RewardParamList;
	std::vector<std::string> AndPrereqs;
	DynamicVectorClass< DynamicVectorClass<std::string> > Prereqs;
	std::string StoryDialog;
	int StoryDialogChapter;
	std::string StoryDialogVar;
	std::string StoryDialogTag;
	std::string StoryDialogIncoming;
	bool StoryDialogPopup;
	bool StoryDialogSFX;
	bool Multiplayer;
	float InactiveDelay;
	Vector3 Position;
	std::string BranchName;
	bool Perpetual;
	float Timeout;
};







class StoryEventClass : public SignalGeneratorClass
{
public:

	StoryEventClass();
	virtual ~StoryEventClass();

	void Set_Sub_Plot(StorySubPlotClass *subplot) { SubPlot = subplot; }

	void Parent_Triggered();

	const std::string *Get_Name() { return (&EventName); }
	void Set_Name(const std::string *name) { EventName = *name; }
	StoryEventEnum Get_Event_Type() { return (EventType); }
	void Set_Event_Type(StoryEventEnum type) { EventType = type; }
	StoryRewardEnum Get_Reward_Type() { return (RewardType); }
	void Set_Reward_Type(StoryRewardEnum type) { RewardType = type; }
	bool Is_Triggered() { return (Triggered); }
	void Set_Triggered(bool onoff) { Triggered = onoff; }
	bool Is_Active() { return (Active); }
	void Set_Active(bool onoff) { Active = onoff; }
	bool Is_Multiplayer_Active() { return (Multiplayer); }
	void Set_Multiplayer_Active(bool onoff) { Multiplayer = onoff; }
	const std::string *Get_Reward_Param(int index) { return (&RewardParam[index]); }
	void Set_Reward_Param(int index, const std::string *param) { RewardParam[index] = *param; }
	void Set_Reward_Param_List(std::vector<std::string> *param_list) { RewardParamList = *param_list; }
	const std::string *Get_Story_Dialog() { return (&StoryDialog); }
	void Set_Story_Dialog(const std::string *param) { StoryDialog = *param; }
	int Get_Story_Dialog_Chapter() { return (StoryDialogChapter); }
	void Set_Story_Dialog_Chapter(int chapter) { StoryDialogChapter = chapter; }
	void Set_Story_Dialog_Var(const std::string *var) { StoryDialogVar = *var; }
	void Set_Story_Dialog_Tag(const std::string *var) { StoryDialogTag = *var; }
	const std::string *Get_Story_Dialog_Tag() { return (&StoryDialogTag); }
	void Set_Story_Dialog_Incoming(const std::string *var) { StoryDialogIncoming = *var; }
	const std::string *Get_Story_Dialog_Incoming() { return (&StoryDialogIncoming); }
	void Set_Story_Dialog_Popup(bool popup) { StoryDialogPopup = popup; }
	void Set_Story_Dialog_SFX(bool play_sfx) { StoryDialogSFX = play_sfx; }
	std::vector<std::wstring> *Get_Story_Dialog_Extra_Text() { return (&StoryDialogExtraText); }
	void Set_Inactive_Delay(float delay) { InactiveDelay = delay; }
	float Get_Inactive_Delay() { return (InactiveDelay); }
	void Set_Inactive_Elapsed(float elapsed) { InactiveElapsed = elapsed; }
	float Get_Inactive_Elapsed() { return (InactiveElapsed); }
	void Set_Reward_Position(const Vector3 &pos) { RewardPosition = pos; }
	Vector3 &Get_Reward_Position() { return (RewardPosition); }
	void Set_Perpetual(bool perpetual) { Perpetual = perpetual; }
	bool Get_Perpetual() { return (Perpetual); }
	void Set_Timeout_Time(float timeout) { TimeoutTime = timeout; }
	float Get_Timeout_Time() { return (TimeoutTime); }
	void Set_Start_Time(float starttime) { StartTime = starttime; }
	float Get_Start_Time() { return (StartTime); }

	void Disable_Event(bool onoff) { Disabled = onoff; }
	bool Is_Disabled() { return (Disabled); }

	void Set_Branch_Name(const std::string &name) { BranchName = name; }
	const std::string &Get_Branch_Name() { return (BranchName); }

	void Add_Prereqs(DynamicVectorClass<DynamicVectorClass<std::string> > *prereqs);
	void Compute_Dependants();
	void Add_Dependant(StoryEventClass *dependant);

	bool Event_Filter_Matches(GameObjectClass *planet, StoryEventFilter filter);

	static void Build_Enum_Lookup_Map();
	static void Clear_Enum_Lookup_Map() { EnumLookup.clear(); }
	static int Lookup_Enum(const char *text);

	virtual void Shutdown() {}
	virtual void Set_Param(int, std::vector<std::string> *) {}
	virtual void Evaluate_Event(void *, void *) {}
	virtual void Reset();

	virtual void Planet_Destroyed(const std::string &) {}

	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name) { Replace_Reward_Variable(var_name,new_name); }

	virtual void Activate() {}

	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

	void Event_Triggered(GameObjectClass *planet = NULL, bool inactive = false);
	void Clear_Triggered() { Triggered = false; Reset(); }

	// Look into script for a special case where we want to load a specific map
	virtual bool Check_Special_Land_Tactical_Map(GameObjectClass *, GameObjectClass *) { return (false); }
	virtual bool Check_Planet_Entry_Restrictions(GameObjectClass *, GameObjectClass *) { return false; }
	virtual bool Check_Special_Space_Tactical_Map(GameObjectClass *, GameObjectClass *) { return (false); }

	// Retry mission dialog
	static void Set_Dialog_Ptr(void (*retry_dialog_activate)(void), bool (*is_retry_dialog_active)(void));
	static void Activate_Retry_Dialog() { if (RetryDialog) (*RetryDialog)(); }
	static bool Is_Retry_Dialog_Active() { if (RetryDialogActive) return ((*RetryDialogActive)()); else return (false); }

	// Debugging
	void Set_Index(int index) { Index = index; }
	int Get_Index() { return (Index); }
	void Dump_Status();

protected:

	// Reward functions
	void Give_Reward(GameObjectClass *planet);
	void Reward_Buildable_Unit();
	void Reward_Unique_Unit(GameObjectClass *planet);
	void Reward_Remove_Unit();
	void Reward_Credits();
	void Reward_Spawn_Hero(GameObjectClass *planet) { Reward_Unique_Unit(planet); }
	void Reward_Information() {};
	void Reward_Story_Element();
	void Reward_Statistic_Change(GameObjectClass *planet);
	void Reward_New_Power_For_All();
	void Reward_Remove_Power_From_All();
	void Reward_SFX();
	void Reward_Speech(const char *speech_name = NULL);
	void Reward_Flash_GUI();
	void Reward_Flash_Planet_GUI();
	void Reward_Hide_Tutorial_Cursor();
	void Reward_Screen_Text();
	void Reward_Disable_Event();
	void Reward_Enable_Event();
	void Reward_Pick_Planet();
	void Reward_Switch_Sides();
	void Reward_Zoom_In();
	void Reward_Zoom_Out();
	void Reward_Pause_Game();
	void Reward_Flash_Planet();
	void Reward_Tutorial_Dialog_Box();
	void Reward_Position_Camera();
	void Reward_Lock_Controls();
	void Reward_Scroll_Camera();
	void Reward_Flash_Object();
	void Reward_Toggle_FOW();
	void Reward_Enable_Victory();
	void Reward_Move_Fleet();
	void Reward_Trigger_AI();
	void Reward_Disable_Story_Event();
	void Reward_Disable_Branch();
	void Reward_Invade_Planet();
	void Reward_Load_Campaign();
	void Reward_Flash_Terrain();
	void Reward_Set_Health();
	void Reward_Set_Tactical_Map();
	void Reward_Disable_Autoresolve();
	void Reward_Enable_Autoresolve();
	void Reward_Disable_Movies();
	void Reward_Remove_Story_Goal();
	void Reward_Change_Owner();
	void Reward_Destroy_Object();
	void Reward_Victory();
	void Reward_Switch_Control();
	void Reward_Disable_Retreat();
	void Reward_Flash_Unit();
	void Reward_Reveal_Planet();
	void Reward_Dual_Flash();
	void Reward_Set_Flag();
	void Reward_Increment_Flag();
	void Reward_Hilite_Object();
	void Reward_Story_Goal_Completed();
	void Reward_Reveal_All_Planets();
	void Reward_Disable_Reinforcements();
	void Reward_Reset_Branch();
	void Reward_Reset_Event();
	void Reward_Link_Tactical();
	void Reward_Random_Story();
	void Reward_Disable_Special_Structure();
	void Reward_Set_Tech_Level();
	void Reward_Disable_Spawn();
	void Reward_Trigger_Event();
	void Reward_Enable_Galactic_Reveal();
	void Reward_Activate_Retry_Dialog();
	void Reward_Set_Use_Retry_Dialog();
	void Reward_Start_Movie();
	void Reward_Set_Planet_Spawn();
	void Reward_Commandbar_Movie(const char *param0=NULL, const char *param1=NULL);
	void Reward_Stop_Commandbar_Movie();
	void Reward_Set_Weather();
	void Reward_Planet_Faction();
	void Reward_Lock_Unit();
	void Reward_Force_Retreat();
	void Reward_Set_Planet_Restricted();
	void Reward_Multimedia();
	void Reward_Set_Planet_Visibility_Level();
	void Reward_Pause_Galactic_Game();
	void Reward_Add_Objective();
	void Reward_Remove_Objective();
	void Reward_Objective_Complete();
	void Reward_Remove_All_Objectives();
	void Reward_Objective_Failed();
	void Reward_Enable_Direct_Invasion();
	void Reward_Disable_Direct_Invasion();
	void Reward_Enable_Objective_Display();
	void Reward_Flash_Fleet_With_Unit();
	void Reward_Select_Planet();
	void Reward_Force_Click_GUI();
	void Reward_Flash_Production_Choice();
	void Reward_Set_Max_Tech_Level();
	void Reward_Tutorial_Player();
	void Reward_Pause_Galactic();
	void Reward_Unpause_Galactic();
	void Reward_Enable_Overwhelming_Odds();
	void Reward_Flash_Special_Ability();
	void Reward_Set_Sandbox_Objectives();
	void Reward_Force_Respawn();
	void Reward_Scroll_Lock();
	void Reward_Skirmish_Rules();
	void Reward_Reset_Galactic_Filters();
	void Reward_Set_Advisor();
	void Reward_Start_Cinematic_Mode();
	void Reward_Stop_Cinematic_Mode();
	void Reward_Show_Command_Bar();
	void Reward_Hide_Autoresolve();
	void Reward_Disable_Buildable();
	void Reward_Enable_Buildable();
	void Reward_Hide_Cursor_On_Click();
	void Reward_Lock_Planet_Selection();
	void Reward_Show_Smuggle_Slot();
	void Reward_Hide_Smuggle_Slot();
	void Reward_Show_Raid_Slot();
	void Reward_Hide_Raid_Slot();
	void Reward_Show_Steal_Slot();
	void Reward_Hide_Steal_Slot();
	void Reward_Finished_Tutorial();
	void Reward_Bombard_Override_Delay();
	void Reward_Enable_Bounty_Collection();
	void Reward_Disable_Bounty_Collection();
	void Reward_Remove_Corruption();
	void Reward_Give_Black_Market();
	void Reward_Show_Special_Slot();
	void Reward_Sabotage_Structure();
	void Reward_Enable_Sabotage();
	void Reward_Flash_Advanced_Map_Object();
	void Reward_Enable_Invasion();
	void Reward_Restrict_All_Abilities();
	void Reward_Restrict_Corruption();
	void Reward_Restrict_Black_Market();
	void Reward_Restrict_Sabotage();
	void Reward_Enable_Fleet_Combine();
	void Reward_Enable_Combat_Cinematic();
	void Reward_Restrict_Autoresolve();
	void Reward_Enable_Galactic_Corruption_Hologram();
	void Reward_Replace_Objective();
	void Reward_Enable_Campaign_Victory_Movie();

	void Replace_Reward_Variable(const std::string &var_name, const std::string &new_name);

	bool All_Structures_Destroyed(PlayerClass *player);
	bool All_Units_Destroyed(PlayerClass *player, bool check_structures);
	bool All_Indigenous_Spawners_Destroyed(const GameObjectTypeClass *type);
	
	static std::map<CRCValue,int> EnumLookup;

	std::string EventName;
	StoryEventEnum EventType;
	StoryRewardEnum RewardType;
	Vector3 RewardPosition;
	bool Triggered;
	bool Active;
	bool Multiplayer;
	bool Disabled;
	std::string RewardParam[14];
	std::vector<std::string> RewardParamList;
	StorySubPlotClass *SubPlot;
	std::string StoryDialog;
	int StoryDialogChapter;
	std::string StoryDialogVar;
	std::string StoryDialogTag;
	std::string StoryDialogIncoming;
	bool StoryDialogPopup;
	bool StoryDialogSFX;
	std::vector<std::wstring> StoryDialogExtraText;
	float InactiveDelay;
	float InactiveElapsed;
	std::string BranchName;
	bool Perpetual;

	float StartTime;
	float TimeoutTime;

	DynamicVectorClass< DynamicVectorClass<std::string> > RawPrereqs;
	DynamicVectorClass< DynamicVectorClass<StoryEventClass *> > Prereqs;
	DynamicVectorClass<StoryEventClass *> Dependants;

	// Dialog activation function pointers
	static void (*RetryDialog)( void );
	static bool (*RetryDialogActive)( void );

	// Used for debugging
	int Index;
};





class StoryEventFogRevealClass : public StoryEventClass
{
public:
	StoryEventFogRevealClass() {}

	virtual void Set_Param(int, std::vector<std::string> *);
	virtual void Evaluate_Event(void *, void *);

private:

	Vector3 Position;
	std::string ObjectName;
};






class StoryEventRetreatClass : public StoryEventClass
{
public:

	StoryEventRetreatClass() : Filter(EVENT_FILTER_ENEMY_ONLY) {}

	virtual void Set_Param(int, std::vector<std::string> *);
	virtual void Evaluate_Event(void *, void *);

private:

	StoryEventFilter Filter;
};





class StoryEventStartTacticalClass : public StoryEventClass
{
public:

	StoryEventStartTacticalClass();

	virtual void Shutdown();
	virtual void Set_Param(int, std::vector<std::string> *param);
	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);
	virtual void Planet_Destroyed(const std::string &planet_name);

private:

	std::vector<std::string> Planet;
	std::string ScriptName;
};




// If the string matches, the event is triggered
class StoryEventStringClass : public StoryEventClass
{
public:

	StoryEventStringClass() {}

	virtual void Shutdown();
	virtual void Set_Param(int, std::vector<std::string> *);
	virtual void Evaluate_Event(void *, void *);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);

private:

	std::vector<std::string> Strings;
};





// Supports ENTER, LAND ON
class StoryEventEnterClass : public StoryEventClass
{
public:

	StoryEventEnterClass();

	virtual void Shutdown();
	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);
	virtual void Planet_Destroyed(const std::string &planet_name);
	virtual bool Check_Special_Land_Tactical_Map(GameObjectClass *hero, GameObjectClass *planet);
	virtual bool Check_Planet_Entry_Restrictions(GameObjectClass *fleet, GameObjectClass *planet);
	virtual bool Check_Special_Space_Tactical_Map(GameObjectClass *hero, GameObjectClass *planet);

	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

private:

	bool Check_Fleet_Contents(GameObjectClass *fleet);
	bool Check_Orbit_Contents(GameObjectClass *planet);

	std::vector<std::string> Planet;
	std::vector<std::string> EnteringShip;
	std::vector<std::string> OrbitingShip;
	StoryEventFilter Filter;
	bool AllowStealth;
	bool ExclusiveEnter;
	bool RaidEnter;
};





// Supports CONQUER, CONSTRUCT, DEFEAT HERO, CAPTURE HERO
class StoryEventSingleObjectNameClass : public StoryEventClass
{
public:

	StoryEventSingleObjectNameClass() : Filter(EVENT_FILTER_FRIENDLY_ONLY) {};

	virtual void Shutdown();
	virtual void Evaluate_Event(void *param1, void *);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);
	virtual void Reset(); 

	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

private:

	std::vector<std::string> ObjectName;
	std::vector<int> Count;
	std::vector<int> CountCopy;
	StoryEventFilter Filter;
};




class StoryEventConstructLevelClass : public StoryEventClass
{
public:

	StoryEventConstructLevelClass() : Filter(BASE_EITHER) {}

	virtual void Shutdown();
	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);
	virtual void Planet_Destroyed(const std::string &planet_name);

	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

private:

	unsigned int Level;
	StoryBaseFilter Filter;
	std::vector<std::string> Planet;
};



class StoryEventDestroyBaseClass : public StoryEventClass
{
public:

	StoryEventDestroyBaseClass() : Filter(BASE_EITHER), OwnerFilter(EVENT_FILTER_ENEMY_ONLY) {}

	virtual void Shutdown();
	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);
	virtual void Planet_Destroyed(const std::string &planet_name);

private:

	StoryBaseFilter Filter;
	std::vector<std::string> Planet;
	StoryEventFilter OwnerFilter;
};



class StoryEventDestroyClass : public StoryEventClass
{
public:

	StoryEventDestroyClass() : Filter(EVENT_FILTER_NONE) {};

	virtual void Shutdown();
	virtual void Evaluate_Event(void *, void *);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);
	virtual void Reset();

	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

private:

	std::vector<std::string> Object;
	std::vector<std::string> Planet;
	std::vector<int> Count;
	std::vector<int> CountCopy;
	StoryEventFilter Filter;
};



// Era and tech level
class StoryEventBeginEraClass : public StoryEventClass
{
public:

	StoryEventBeginEraClass() : OldEra(-1),Era(-1) {}

	virtual void Evaluate_Event(void *, void *);
	virtual void Set_Param(int index, std::vector<std::string> *param);

	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

private:

	int OldEra;
	int Era;
};



class StoryEventCorruptionLevelClass : public StoryEventClass
{
public:

	StoryEventCorruptionLevelClass() : CorruptionType(CORRUPTION_ANY) {};

	virtual void Shutdown();
	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);

private:

	CorruptionTypeEnum CorruptionType;
	std::vector<std::string> Planet;
};



// DEPLOY, MOVE
class StoryEventHeroMoveClass : public StoryEventClass
{
public:

	StoryEventHeroMoveClass(){};

	virtual void Shutdown();
	virtual void Evaluate_Event(void *, void *);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);
	virtual void Planet_Destroyed(const std::string &planet_name);
	virtual bool Check_Special_Land_Tactical_Map(GameObjectClass *hero, GameObjectClass *planet);
	virtual bool Check_Planet_Entry_Restrictions(GameObjectClass *fleet, GameObjectClass *planet);
	virtual bool Check_Special_Space_Tactical_Map(GameObjectClass *hero_move, GameObjectClass *planet);
	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

private:

	std::vector<std::string> Hero;
	std::vector<std::string> Planet;
};



class StoryEventAccumulateClass : public StoryEventClass
{
public:

	StoryEventAccumulateClass() : Credits(0), Comparison(COMPARE_NONE) {};

	virtual void Evaluate_Event(void * param1, void *);
	virtual void Set_Param(int index, std::vector<std::string> *param);

private:

	int Credits;
	StoryCompareEnum Comparison;
};



class StoryEventConquerCountClass : public StoryEventClass
{
public:

	StoryEventConquerCountClass() : CurCount(0) {}

	virtual void Evaluate_Event(void *, void *);
	virtual void Set_Param(int index, std::vector<std::string> *param);

	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

private:

	int CurCount;
	int TriggerCount;
};



class StoryEventElapsedClass : public StoryEventClass
{
public:

	StoryEventElapsedClass() : LastTime(0), Elapsed(0), TriggerTime(-1), Initialized(false) {}

	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Reset() { StoryEventClass::Reset(); Elapsed = 0; Initialized = false; }

	virtual void Activate();


	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

private:

	float LastTime;
	float Elapsed;
	float TriggerTime;
	bool Initialized;
};



// Also used for lose battle event
class StoryEventWinBattlesClass : public StoryEventClass
{
public:

	StoryEventWinBattlesClass() : CurCount(0), Filter(BASE_EITHER), ContentFilter(FC_ANY) {}

	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Reset() { CurCount = 0; }

	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

private:

	bool Content_Matches(GameObjectClass *planet);

	int CurCount;
	int TriggerCount;
	StoryBaseFilter Filter;
	FleetContentEnum ContentFilter;
	DynamicVectorClass<ShipClassType> ContentTypes;
	std::vector<std::string> Planet;
};




class StoryEventAINotificationClass : public StoryEventClass
{
public:

	StoryEventAINotificationClass() {}

	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);
	virtual void Planet_Destroyed(const std::string &planet_name);
		
	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

private:

	std::vector<std::string> TriggerString;
	std::vector<std::string> Planet;
};



class StoryEventCommandUnitClass : public StoryEventClass
{
public:

	StoryEventCommandUnitClass() : MaxDist(0) {}

	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);

protected:

	std::vector<std::string> UnitName;
	std::vector<std::string> TargetName;
	float MaxDist;
};




class StoryEventGuardUnitClass : public StoryEventClass
{
public:
	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);

private:

	std::vector<std::string> Guard;
	std::vector<std::string> Guardee;
};




class StoryEventProximityClass : public StoryEventCommandUnitClass
{
public:
	virtual void Evaluate_Event(void *param1, void *);

private:
	void Check_For_Proximity(Vector3 &pos);
};




class StoryEventDifficultyClass : public StoryEventClass
{
public:
	virtual void Evaluate_Event(void *, void *);
	virtual void Set_Param(int, std::vector<std::string> *param);

private:

	DifficultyLevelType Difficulty;
};



class StoryEventFlagClass : public StoryEventClass
{
public:

	StoryEventFlagClass() : Value(0), Comparison(COMPARE_EQUAL_TO) {};

	virtual void Evaluate_Event(void * param1, void *);
	virtual void Set_Param(int index, std::vector<std::string> *param);

private:

	std::vector<std::string> FlagNames;
	int Value;
	StoryCompareEnum Comparison;
};



class StoryEventLoadTacticalClass : public StoryEventClass
{
public:

	StoryEventLoadTacticalClass() : Base(BASE_GROUND) {};

	virtual void Shutdown();
	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);
	virtual void Planet_Destroyed(const std::string &planet_name);

	// Same as evaluate event but won't trigger event
	virtual bool Check_Special_Land_Tactical_Map(GameObjectClass *hero, GameObjectClass *planet);

private:

	std::vector<std::string> Hero;
	std::vector<std::string> Planet;
	StoryBaseFilter Base;
};


// Different than destroyed class since it checks to see if all units/structures/both are destroyed
// instead of reacting to the destruction of a single unit
class StoryCheckDestroyedClass : public StoryEventClass
{
public:

	StoryCheckDestroyedClass() : Player(NULL), Filter(DESTROY_ALL), TypeFilter(NULL) {}

	virtual void Evaluate_Event(void *, void *);
	virtual void Set_Param(int index, std::vector<std::string> *param);

private:

	PlayerClass *Player;
	StoryDestroyEnum Filter;
	const GameObjectTypeClass *TypeFilter;
};


// This event is triggered when the current mission ends in a 
// victory.
class StoryEventVictoryClass : public StoryEventClass
{
public:

	StoryEventVictoryClass() : Faction(NULL) {}

	virtual void Evaluate_Event(void *, void *);
	virtual void Set_Param(int index, std::vector<std::string> *param);


private:

	FactionClass *Faction;
};


// This event is triggered when the named movie has played and finished.
class StoryEventMovieDoneClass : public StoryEventClass
{
public:

	virtual void Evaluate_Event(void *, void *);
	//virtual void Set_Param(int index, std::vector<std::string> *param);


private:

	//std::string Movie_Name;
};


class StoryEventObjectiveTimeoutClass : public StoryEventClass
{
public:

	StoryEventObjectiveTimeoutClass() : LastTime(0), Elapsed(0), TriggerTime(-1), Initialized(false) {}

	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Reset() { StoryEventClass::Reset(); Elapsed = 0; Initialized = false; }

	virtual void Activate();


	virtual bool Load( ChunkReaderClass *reader );
	virtual bool Save( ChunkWriterClass *writer );

private:

	float LastTime;
	float Elapsed;
	float TriggerTime;
	bool Initialized;
	std::string ObjectiveName;
};







class StoryEventCaptureClass : public StoryEventClass
{
public:
	virtual void Evaluate_Event(void *param1, void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);

private:

	std::vector<std::string> Structure;
	const FactionClass *NewFaction;
};





void Story_Debug_Printf(char *text, ...);
void Story_Debug_Printf(const char *text, ...);


#endif