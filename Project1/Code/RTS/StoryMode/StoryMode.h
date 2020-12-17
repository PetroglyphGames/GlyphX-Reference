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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/StoryMode/StoryMode.h $
//
//             Author: Jason Curtice
//
//               Date: 07/21/2004 6:34PM
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef STORYMODE_H
#define STORYMODE_H



#include "MultiLinkedList.h"
#include "StorySubPlot.h"

class StoryModeClass;
class GameObjectTypeClass;
class PlayerClass;
class ChunkReaderClass;
class ChunkWriterClass;
enum ComponentId;
class SpeechEventClass;
class GameModeClass;

//extern class StoryModeClass TheStoryMode;


struct DelayedEventStruct
{
	StoryEventEnum Event;
	PlayerClass *Player;
	void *Param1;
	void *Param2;
	std::string Param1String;
};


struct FlagStruct
{
	char Name[32];	// Not used but kept around for debugging purposes
	int Value;			
};

enum ObjectiveStatusEnum
{
	OBJECTIVE_NOT_COMPLETE,
	OBJECTIVE_COMPLETE,
	OBJECTIVE_FAILED,
	OBJECTIVE_DOESNT_EXIST
};

struct ObjectiveStruct
{
	std::string ObjectiveText;
	std::wstring DisplayText;
	ObjectiveStatusEnum Status;
	bool Suggestion;
	int ObjectiveNumber;
};



class StoryModeClass
{
public:

	StoryModeClass() : ParentMode(NULL), SandboxPlot(NULL), CurrentObjective(1), DelayedBattleEnd(false) {}
	~StoryModeClass();

	bool Load_Plots(const std::string &name, PlayerClass *player);
	StorySubPlotClass *Load_Single_Plot(std::string &value, std::string &lua_script, bool active, PlayerClass *player);
	void Remove_Plots();
	void Add_Plot(CRCValue crc, StorySubPlotClass *plot) { SubPlots[crc] = plot; }
	void Remove_Plot(CRCValue crc);
	void Reload_Scripts(std::vector<std::string> &files);
	LuaScriptClass *Find_Lua_Script(const std::string &name);
	void Handle_ESC(void);
	void Set_Parent(GameModeClass *parent) { ParentMode = parent; }

	bool Activate_Sub_Plot(const std::string &name);
	bool Deactivate_Sub_Plot(const std::string &name);
	StorySubPlotClass *Get_Sub_Plot(const std::string &name);

	void Story_Event(StoryEventEnum event, PlayerClass *player, void *param1, void *param2);

	void Execute_Delayed_Events();

	// Game events
	void Enter_System(GameObjectClass *planet, GameObjectClass *fleet);
	void Land_On_Planet(GameObjectClass *planet, GameObjectClass *transport);
	void Hero_Land_On_Planet(GameObjectClass *planet, GameObjectClass *hero);	// slight variation on Land_On_Planet when you don't have a transport
	void Conquer(GameObjectClass *planet);
	void Construct(GameObjectClass *planet, const GameObjectTypeClass *object_type);
	void Destroy(GameObjectClass *object);
	void Destroy_Base(GameObjectClass *planet, StoryBaseFilter type);
	void Tactical_Destroy(GameObjectClass *object);
	void Begin_Time_Era(int era);
	void Tech_Level(int level, PlayerClass *player);
	void Political_Control(GameObjectClass *planet, int allegiance);
	void Accumulate(PlayerClass *player, int credits);
	void Elapsed(float elapsed);
	void Capture_Hero(GameObjectClass *hero);
	void Defeat_Hero(GameObjectClass *hero);
	void Win_Space_Battle(GameObjectClass *planet, PlayerClass *player);
	void Lose_Space_Battle(GameObjectClass *planet, PlayerClass *player);
	void Win_Land_Battle(GameObjectClass *planet, PlayerClass *player);
	void Lose_Land_Battle(GameObjectClass *planet, PlayerClass *player);
	void Begin_Space_Tactical(StoryModeClass *space_story);
	void Begin_Land_Tactical(StoryModeClass *land_story);
	void Retreat(PlayerClass *player);
	void Reinforce(PlayerClass *player, const GameObjectClass *object);
	void Planet_Destroyed(GameObjectClass *planet);
	void Attack_Hardpoint(const std::string *name);
	void Load_Tactical_Map(GameObjectClass *planet, StoryBaseFilter *location);
	void Fleet_Bounced(GameObjectClass *planet, GameObjectClass *fleet);
	void Victory(PlayerClass *player);
	void Mission_Lost(const std::string &plot_name);
	void Invasion_Bounced(GameObjectClass *planet);
	void Structure_Captured(GameObjectClass *object, PlayerClass *new_owner);
	void Galactic_Sabotage(GameObjectClass *object);
	void Buy_Black_Market(const std::string &item_name);

	void Speech_Killed(DynamicVectorClass<std::string> *killed_names);

	void Check_Plots(float elapsed);
	void Check_Proximity();
	void Check_Destroyed();

	bool Check_Special_Land_Tactical_Map(GameObjectClass *hero, GameObjectClass *planet, bool check_land_only=false);
	bool Check_Planet_Entry_Restrictions(GameObjectClass *fleet, GameObjectClass *planet);
	bool Check_Special_Space_Tactical_Map(GameObjectClass *planet, GameObjectClass *hero = NULL);

	// Tutorial events
	void Tutorial_Click_UI(const char *name);
	void Tutorial_Select_Unit(GameObjectClass *unit, int player_id);
	void Tutorial_Command_Unit(ReferenceListClass<GameObjectClass> *units, const Vector3 &pos);
	void Tutorial_Guard_Unit(ReferenceListClass<GameObjectClass> *units, const GameObjectClass *target);
	void Tutorial_Unit_Arrives(GameObjectClass *unit);
	void Tutorial_Full_Stop(GameObjectClass *unit);
	void Tutorial_Select_Planet(GameObjectClass *planet);
	void Tutorial_Zoom_In_Planet(GameObjectClass *planet);
	void Tutorial_Zoom_Out_Planet(GameObjectClass *planet);
	void Tutorial_Speech_Done(const char *speech);
	void Tutorial_Generic(const char *name);
	void Tutorial_Continue_Tutorial();

	// Special events
	void Replace_Variable(const std::string &var_name, const std::string &new_name);
	void Remove_Destroyed_Planet(const std::string &planet_name);
	static void Set_Land_Forces(ObjectIDType *forces, int count);
	static DynamicVectorClass<ObjectIDType> &Get_Land_Forces() { return (LandForces); }

	// Expansion events
	void Corruption_Increased(GameObjectClass *planet, int corruption_type);
	void Corruption_Decreased(GameObjectClass *planet, int corruption_type);
	void Corruption_Tactical_Complete(GameObjectClass *planet, int corruption_type);
	void Corruption_Tactical_Failed(GameObjectClass *planet, int corruption_type);
	void Corruption_Dialog_Opened(GameObjectClass *planet);
	void Garrison_Unit(GameObjectClass *unit);

	// Transition back from tactical
	void Trigger_All_Speech_Done_Events();

	// Script defined variables.  They're available across all scripts
	static void Set_Flag(const char *name, int value);
	static int Get_Flag(const char *name);
	static int Increment_Flag(const char *name, int increment);
	static void Reset_Flags() { Flags.clear(); }

	// Objectives
	void Add_Objective(const std::string &objective, const std::wstring *display_text = NULL, bool suggestion = false, int index = -1);
	int Remove_Objective(const std::string &objective, bool rebuild = true);
	void Remove_All_Objectives();
	void Objective_Completed(const std::string &objective);
	void Objective_Failed(const std::string &objective);
	ObjectiveStatusEnum Objective_Status(const std::string &objective);
	const DynamicVectorClass<ObjectiveStruct> *Get_Objective_List() { return (&ObjectiveList); }
	void Replace_Objective(const std::string &old_objective, const std::string &new_objective);

	// Sandbox tactical maps will have special scripts
	void Load_Default_Sandbox_Script(PlayerClass *defender);
	void Set_Sandbox_Objectives();

	// Delayed check for the Battle End Dialog being shut so that other stuff can fire first
	void Set_Delayed_Battle_End(bool onoff) { DelayedBattleEnd = onoff; }

	// Static functions
	static void Speech_Callback(const SpeechEventClass *speech_event);
	static void Set_Foreground_App_Ptr(const bool *ptr) { IsForegroundApp = ptr; }

	bool Load( ChunkReaderClass *reader );
	bool Save( ChunkWriterClass *writer );

	// Debugging
	void Dump_Status();
	void Trigger_Event(const char *event_name);
	void Disable_Event(const char *event_name, bool onoff);

	typedef stdext::hash_map<CRCValue, FlagStruct> StoryFlagListType;

private:

	void Get_Sandbox_Primary_Objective(std::string *win_text, PlayerClass *defender, PlayerClass *player);

	std::string PlotName;			// Name of current plot file.
	std::string MissionPlotName;	// Galactic level story mode may need to store the name of a tactical mission plot
	GameModeClass *ParentMode;
	typedef std::pair<int, std::string> PlayerPlotPairType;
	std::vector<PlayerPlotPairType> Plots;
	typedef stdext::hash_map<CRCValue, StorySubPlotClass *> SubPlotListType;
	SubPlotListType SubPlots;
	DynamicVectorClass<DelayedEventStruct *> DelayedEvents;
	DynamicVectorClass<ObjectiveStruct> ObjectiveList;
	int CurrentObjective;

	// Special sandbox variables
	StorySubPlotClass *SandboxPlot;
	PlayerClass *SandboxDefender;

	bool DelayedBattleEnd;

	static DynamicVectorClass<int> LandForces;
	static StoryFlagListType Flags;
	static const bool *IsForegroundApp;
};


		
void Event_Debug_Printf(char *text, ...);
void Event_Debug_Printf(const char *text, ...);




#endif
