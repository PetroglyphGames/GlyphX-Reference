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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/StoryMode/StoryEvent.cpp $
//
//             Author: Jason Curtice
//
//               Date: 07/21/2004 3:30PM
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma hdrstop

#include "gameobject.h"
#include "StoryEvent.h"
#include "FactionList.h"
#include "Faction.h"
#include "PlayerList.h"
#include "DatabaseMap.h"
#include "StorySubPlot.h"
#include "DebugPrint.h"
#include "FrameSynchronizer.h"
#include "GameObjectType.h"
#include "GameObjectTypeManager.h"
#include "GameObjectManager.h"
#include "ProductionBehavior.h"
#include "PlanetaryBehavior.h"
#include "StoryMode.h"
#include "StoryDialogManager.h"
#include "ChunkFile.h"
#include "CommandBar.h"
#include "SFXEventManager.h"
#include "SpeechEventManager.h"
#include "SpeechConversationManager.h"
#include "SFXConversationManager.h"
#include "Tutorial.h"
#include "GalacticCamera.h"
#include "AI/PathFinding/GalacticPath.h"
#include "AI/PathFinding/GalacticPathFinder.h"
#include "SelectEvent.h"
#include "MoveThroughObjectsEvent.h"
#include "OutgoingEventQueue.h"
#include "FleetManagementEvent.h"
#include "ICombatantBehavior.h"
#include "InvadeEvent.h"
#include "TransportBehavior.h"
#include "CampaignData.h"
#include "CampaignDataManager.h"
#include "AI/AIPlayer.h"
#include "AI/TacticalAIManager.h"
#include "AI/Execution/AIExecutionSystem.h"
#include "AI/Execution/AIFreeStore.h"
#include "FleetBehavior.h"
#include "EnumConversion.h"
#include "DifficultyLevelType.h"
#include "VictoryMonitor.h"
#include "RandomStoryMode.h"
#include "BaseShieldBehavior.h"
#include "Diagnostics/DebugWindow.h"
#include "Diagnostics/LogWindow.h"
#include "StoryEventSelectPlanet.h" // This event was moved out of this file -Eric_Y
#include "CinematicsManager.h"
#include "GalacticMode.h"
#include "SpawnIndigenousUnitsBehavior.h"
#include "GameConstants.h"
#include "GameText.h"
#include "CameraFXManager.h"
#include "FleetLocomotorBehavior.h"
#include "PlayerProfile.h"
#include "PlanetaryBombardManager.h"
#include "BlackMarketItemList.h"
#include "BlackMarketItem.h"
#include "Abilities/GalacticSabotageAbility.h"

#define MAX_SPEECH_TIME 60


std::map<CRCValue,int> StoryEventClass::EnumLookup;

enum ShowSlotType
{
	SHOW_NONE,

	SHOW_RAID_SLOT,
	SHOW_SMUGGLE_SLOT,
	SHOW_STEAL_SLOT,
	SHOW_BLACK_MARKET_SLOT,
	SHOW_SABOTAGE_SLOT,
	SHOW_CORRUPTION_SLOT,
	SHOW_REMOVE_CORRUPTION_SLOT,
	SHOW_NEUTRALIZE_HERO_SLOT
};



static StoryEnumRemapStruct EnumRemap[] = {
	// Story events
	{ "STORY_NONE", STORY_NONE },
	{ "STORY_ENTER", STORY_ENTER },
	{ "STORY_LAND_ON", STORY_LAND_ON },
	{ "STORY_CONQUER", STORY_CONQUER },
	{ "STORY_CONSTRUCT", STORY_CONSTRUCT },
	{ "STORY_CONSTRUCT_LEVEL", STORY_CONSTRUCT_LEVEL },
	{ "STORY_DESTROY", STORY_DESTROY },
	{ "STORY_DESTROY_BASE", STORY_DESTROY_BASE },
	{ "STORY_TACTICAL_DESTROY", STORY_TACTICAL_DESTROY },
	{ "STORY_BEGIN_ERA", STORY_BEGIN_ERA },
	{ "STORY_TECH_LEVEL", STORY_TECH_LEVEL },
	{ "STORY_POLITICAL_CONTROL", STORY_POLITICAL_CONTROL },
	{ "STORY_DEPLOY", STORY_DEPLOY },
	{ "STORY_MOVE", STORY_MOVE },
	{ "STORY_ACCUMULATE", STORY_ACCUMULATE },
	{ "STORY_CONQUER_COUNT", STORY_CONQUER_COUNT },
	{ "STORY_ELAPSED", STORY_ELAPSED },
	{ "STORY_CAPTURE_HERO", STORY_CAPTURE_HERO },
	{ "STORY_DEFEAT_HERO", STORY_DEFEAT_HERO },
	{ "STORY_WIN_BATTLES", STORY_WIN_BATTLES },
	{ "STORY_LOSE_BATTLES", STORY_LOSE_BATTLES },
	{ "STORY_RETREAT", STORY_RETREAT },
	{ "STORY_REINFORCE", STORY_REINFORCE },
	{ "STORY_PLANET_DESTROYED", STORY_PLANET_DESTROYED },
	{ "STORY_SPACE_TACTICAL", STORY_SPACE_TACTICAL },
	{ "STORY_LAND_TACTICAL", STORY_LAND_TACTICAL },
	{ "STORY_SELECT_PLANET", STORY_SELECT_PLANET },
	{ "STORY_ZOOM_INTO_PLANET", STORY_ZOOM_INTO_PLANET },
	{ "STORY_ZOOM_OUT_PLANET", STORY_ZOOM_OUT_PLANET },
	{ "STORY_CLICK_GUI", STORY_CLICK_GUI },
	{ "STORY_SPEECH_DONE", STORY_SPEECH_DONE },
	{ "STORY_FOG_OBJECT_REVEAL", STORY_FOG_OBJECT_REVEAL },
	{ "STORY_FOG_POSITION_REVEAL", STORY_FOG_POSITION_REVEAL },
	{ "STORY_GENERIC", STORY_GENERIC },
	{ "STORY_TRIGGER", STORY_TRIGGER },
	{ "STORY_AI_NOTIFICATION", STORY_AI_NOTIFICATION },
	{ "STORY_SELECT_UNIT", STORY_SELECT_UNIT },
	{ "STORY_COMMAND_UNIT", STORY_COMMAND_UNIT },
	{ "STORY_UNIT_ARRIVED", STORY_UNIT_ARRIVED },
	{ "STORY_FULL_STOP", STORY_FULL_STOP },
	{ "STORY_ATTACK_HARDPOINT", STORY_ATTACK_HARDPOINT },
	{ "STORY_GUARD_UNIT", STORY_GUARD_UNIT },
	{ "STORY_UNIT_PROXIMITY", STORY_UNIT_PROXIMITY },
	{ "STORY_DIFFICULTY_LEVEL", STORY_DIFFICULTY_LEVEL },
	{ "STORY_FLAG", STORY_FLAG },
	{ "STORY_LOAD_TACTICAL_MAP", STORY_LOAD_TACTICAL_MAP },
	{ "STORY_CHECK_DESTROYED", STORY_CHECK_DESTROYED },
	{ "STORY_VICTORY", STORY_VICTORY },
	{ "STORY_MOVIE_DONE", STORY_MOVIE_DONE },
	{ "STORY_FLEET_BOUNCED", STORY_FLEET_BOUNCED },
	{ "STORY_MISSION_LOST", STORY_MISSION_LOST },
	{ "STORY_INVASION_BOUNCED", STORY_INVASION_BOUNCED },
	{ "STORY_OBJECTIVE_TIMEOUT", STORY_OBJECTIVE_TIMEOUT },
	{ "STORY_CAPTURE_STRUCTURE", STORY_CAPTURE_STRUCTURE },
	{ "STORY_CORRUPTION_CHANGED", STORY_CORRUPTION_CHANGED },
	{ "STORY_GARRISON_UNIT", STORY_GARRISON_UNIT },
	{ "STORY_BUY_BLACK_MARKET", STORY_BUY_BLACK_MARKET },
	{ "STORY_GALACTIC_SABOTAGE", STORY_GALACTIC_SABOTAGE },
	{ "STORY_CORRUPTION_TACTICAL_COMPLETE", STORY_CORRUPTION_TACTICAL_COMPLETE },
	{ "STORY_CORRUPTION_TACTICAL_FAILED", STORY_CORRUPTION_TACTICAL_FAILED },
	{ "STORY_OPEN_CORRUPTION", STORY_OPEN_CORRUPTION },
	{ "STORY_CORRUPTION_INCREASED", STORY_CORRUPTION_CHANGED },

	// Rewards
	{ "BUILDABLE_UNIT", REWARD_BUILDABLE_UNIT },
	{ "UNIQUE_UNIT", REWARD_UNIQUE_UNIT },
	{ "REMOVE_UNIT", REWARD_REMOVE_UNIT },
	{ "CREDITS", REWARD_CREDITS },
	{ "SPAWN_HERO", REWARD_SPAWN_HERO },
	{ "INFORMATION", REWARD_INFORMATION },
	{ "STORY_ELEMENT", REWARD_STORY_ELEMENT },
	{ "STATISTIC_CHANGE", REWARD_STATISTIC_CHANGE },
	{ "SFX", REWARD_SFX },
	{ "SPEECH", REWARD_SPEECH },
	{ "FLASH_GUI", REWARD_FLASH_GUI },
	{ "FLASH_PLANET_GUI", REWARD_FLASH_PLANET_GUI },
	{ "HIDE_TUTORIAL_CURSOR", REWARD_HIDE_TUTORIAL_CURSOR },
	{ "SCREEN_TEXT", REWARD_SCREEN_TEXT },
	{ "DISABLE_EVENT", REWARD_DISABLE_EVENT },
	{ "ENABLE_EVENT", REWARD_ENABLE_EVENT },
	{ "PICK_PLANET", REWARD_PICK_PLANET },
	{ "SWITCH_SIDES", REWARD_SWITCH_SIDES },
	{ "ZOOM_IN", REWARD_ZOOM_IN },
	{ "ZOOM_OUT", REWARD_ZOOM_OUT },
	{ "PAUSE_GAME", REWARD_PAUSE_GAME },
	{ "FLASH_PLANET", REWARD_FLASH_PLANET },
	{ "TUTORIAL_DIALOG", REWARD_TUTORIAL_DIALOG },
	{ "POSITION_CAMERA", REWARD_POSITION_CAMERA },
	{ "LOCK_CONTROLS", REWARD_LOCK_CONTROLS },
	{ "SCROLL_CAMERA", REWARD_SCROLL_CAMERA },
	{ "FLASH_OBJECT", REWARD_FLASH_OBJECT },
	{ "ENABLE_FOW", REWARD_ENABLE_FOW },
	{ "ENABLE_VICTORY", REWARD_ENABLE_VICTORY },
	{ "MOVE_FLEET", REWARD_MOVE_FLEET },
	{ "TRIGGER_AI", REWARD_TRIGGER_AI },
	{ "DISABLE_STORY_EVENT", REWARD_DISABLE_STORY_EVENT },
	{ "DISABLE_BRANCH", REWARD_DISABLE_BRANCH },
	{ "INVADE_PLANET", REWARD_INVADE_PLANET },
	{ "LOAD_CAMPAIGN", REWARD_LOAD_CAMPAIGN },
	{ "FLASH_TERRAIN", REWARD_FLASH_TERRAIN },
	{ "SET_HEALTH", REWARD_SET_HEALTH },
	{ "NEW_POWER_FOR_ALL", REWARD_NEW_POWER_FOR_ALL },
	{ "REMOVE_POWER_FROM_ALL", REWARD_REMOVE_POWER_FROM_ALL },
	{ "SET_TACTICAL_MAP", REWARD_SET_TACTICAL_MAP },
	{ "DISABLE_AUTORESOLVE", REWARD_DISABLE_AUTORESOLVE },
	{ "ENABLE_AUTORESOLVE", REWARD_ENABLE_AUTORESOLVE },
	{ "DISABLE_MOVIES", REWARD_DISABLE_MOVIES },
	{ "REMOVE_STORY_GOAL", REWARD_REMOVE_STORY_GOAL },
	{ "CHANGE_OWNER", REWARD_CHANGE_OWNER },
	{ "DESTROY_OBJECT", REWARD_DESTROY_OBJECT },
	{ "VICTORY", REWARD_VICTORY },
	{ "SWITCH_CONTROL", REWARD_SWITCH_CONTROL },
	{ "DISABLE_RETREAT", REWARD_DISABLE_RETREAT },
	{ "FLASH_UNIT", REWARD_FLASH_UNIT },
	{ "REVEAL_PLANET", REWARD_REVEAL_PLANET },
	{ "DUAL_FLASH", REWARD_DUAL_FLASH },
	{ "SET_FLAG", REWARD_SET_FLAG },
	{ "INCREMENT_FLAG", REWARD_INCREMENT_FLAG },
	{ "HIGHLIGHT_OBJECT", REWARD_HILITE_OBJECT },
	{ "STORY_GOAL_COMPLETED", REWARD_STORY_GOAL_COMPLETED },
	{ "REVEAL_ALL_PLANETS", REWARD_REVEAL_ALL_PLANETS },
	{ "DISABLE_REINFORCEMENTS", REWARD_DISABLE_REINFORCEMENTS },
	{ "RESET_BRANCH", REWARD_RESET_BRANCH },
	{ "RESET_EVENT", REWARD_RESET_EVENT },
	{ "LINK_TACTICAL", REWARD_LINK_TACTICAL },
	{ "RANDOM_STORY", REWARD_RANDOM_STORY },
	{ "DISABLE_SPECIAL_STRUCTURE", REWARD_DISABLE_SPECIAL_STRUCTURE },
	{ "SET_TECH_LEVEL", REWARD_SET_TECH_LEVEL },
	{ "SET_SPAWN", REWARD_DISABLE_SPAWN },
	{ "TRIGGER_EVENT", REWARD_TRIGGER_EVENT },
	{ "ENABLE_GALACTIC_REVEAL", REWARD_ENABLE_GALACTIC_REVEAL },
	{ "ACTIVATE_RETRY_DIALOG", REWARD_ACTIVATE_RETRY_DIALOG },
	{ "USE_RETRY_DIALOG", REWARD_SET_USE_RETRY_DIALOG },
	{ "START_MOVIE", REWARD_START_MOVIE },
	{ "SET_PLANET_SPAWN", REWARD_SET_PLANET_SPAWN },
	{ "COMMANDBAR_MOVIE", REWARD_COMMANDBAR_MOVIE },
	{ "STOP_COMMANDBAR_MOVIE", REWARD_STOP_COMMANDBAR_MOVIE },
	{ "SET_WEATHER", REWARD_SET_WEATHER },
	{ "PLANET_FACTION", REWARD_PLANET_FACTION },
	{ "LOCK_UNIT", REWARD_LOCK_UNIT },
	{ "FORCE_RETREAT", REWARD_FORCE_RETREAT },
	{ "SET_PLANET_RESTRICTED", REWARD_SET_PLANET_RESTRICTED },
	{ "MULTIMEDIA", REWARD_MULTIMEDIA },
	{ "SET_PLANET_VISIBILITY_LEVEL", REWARD_SET_PLANET_VISIBILITY_LEVEL },
	{ "PAUSE_GALACTIC_GAME", REWARD_PAUSE_GALACTIC_GAME },
	{ "ADD_OBJECTIVE", REWARD_ADD_OBJECTIVE },
	{ "REMOVE_OBJECTIVE", REWARD_REMOVE_OBJECTIVE },
	{ "OBJECTIVE_COMPLETE", REWARD_OBJECTIVE_COMPLETE },
	{ "REMOVE_ALL_OBJECTIVES", REWARD_REMOVE_ALL_OBJECTIVES },
	{ "OBJECTIVE_FAILED", REWARD_OBJECTIVE_FAILED },
	{ "ENABLE_DIRECT_INVASION", REWARD_ENABLE_DIRECT_INVASION },
	{ "DISABLE_DIRECT_INVASION", REWARD_DISABLE_DIRECT_INVASION },
	{ "ENABLE_OBJECTIVE_DISPLAY", REWARD_ENABLE_OBJECTIVE_DISPLAY },
	{ "FLASH_FLEET_WITH_UNIT", REWARD_FLASH_FLEET_WITH_UNIT },
	{ "SELECT_PLANET", REWARD_SELECT_PLANET },
	{ "FORCE_CLICK_GUI", REWARD_FORCE_CLICK_GUI },
	{ "FLASH_PRODUCTION_CHOICE", REWARD_FLASH_PRODUCTION_CHOICE },
	{ "FLASH_SPECIAL_ABILITY", REWARD_FLASH_SPECIAL_ABILITY },
	{ "SET_MAX_TECH_LEVEL", REWARD_SET_MAX_TECH_LEVEL },
	{ "TUTORIAL_PLAYER", REWARD_TUTORIAL_PLAYER },
	{ "PAUSE_GALACTIC", REWARD_PAUSE_GALACTIC },
	{ "UNPAUSE_GALACTIC", REWARD_UNPAUSE_GALACTIC },
	{ "ENABLE_OVERWHELMING_ODDS", REWARD_ENABLE_OVERWHELMING_ODDS },
	{ "SET_SANDBOX_OBJECTIVES", REWARD_SET_SANDBOX_OBJECTIVES },
	{ "FORCE_RESPAWN", REWARD_FORCE_RESPAWN },
	{ "SCROLL_LOCK", REWARD_SCROLL_LOCK },
	{ "SKIRMISH_RULES", REWARD_SKIRMISH_RULES },
	{ "RESET_GALACTIC_FILTERS", REWARD_RESET_GALACTIC_FILTERS },
	{ "SET_ADVISOR", REWARD_SET_ADVISOR },
	{ "START_CINEMATIC_MODE", REWARD_START_CINEMATIC_MODE },
	{ "STOP_CINEMATIC_MODE", REWARD_STOP_CINEMATIC_MODE },
	{ "SHOW_COMMAND_BAR", REWARD_SHOW_COMMAND_BAR },
	{ "HIDE_AUTORESOLVE", REWARD_HIDE_AUTORESOLVE },
	{ "DISABLE_BUILDABLE", REWARD_DISABLE_BUILDABLE },
	{ "ENABLE_BUILDABLE", REWARD_ENABLE_BUILDABLE },
	{ "HIDE_CURSOR_ON_CLICK", REWARD_HIDE_CURSOR_ON_CLICK },
	{ "LOCK_PLANET_SELECTION", REWARD_LOCK_PLANET_SELECTION },
	{ "SHOW_SMUGGLE_SLOT", REWARD_SHOW_SMUGGLE_SLOT },
	{ "HIDE_SMUGGLE_SLOT", REWARD_HIDE_SMUGGLE_SLOT },
	{ "SHOW_RAID_SLOT", REWARD_SHOW_RAID_SLOT },
	{ "HIDE_RAID_SLOT", REWARD_HIDE_RAID_SLOT },
	{ "SHOW_STEAL_SLOT", REWARD_SHOW_STEAL_SLOT },
	{ "HIDE_STEAL_SLOT", REWARD_HIDE_STEAL_SLOT },
	{ "FINISHED_TUTORIAL", REWARD_FINISHED_TUTORIAL },
	{ "BOMBARD_DELAY", REWARD_BOMBARD_OVERRIDE_DELAY },
	{ "ENABLE_BOUNTY_COLLECTION", REWARD_ENABLE_BOUNTY_COLLECTION },
	{ "DISABLE_BOUNTY_COLLECTION", REWARD_DISABLE_BOUNTY_COLLECTION },
	{ "REMOVE_CORRUPTION", REWARD_REMOVE_CORRUPTION },
	{ "GIVE_BLACK_MARKET", REWARD_GIVE_BLACK_MARKET },
	{ "SHOW_SPECIAL_SLOT", REWARD_SHOW_SPECIAL_SLOT },
	{ "SABOTAGE_STRUCTURE", REWARD_SABOTAGE_STRUCTURE },
	{ "ENABLE_SABOTAGE", REWARD_ENABLE_SABOTAGE },
	{ "FLASH_ADVANCED_MAP_OBJECT", REWARD_FLASH_ADVANCED_MAP_OBJECT },
	{ "ENABLE_INVASION", REWARD_ENABLE_INVASION },
	{ "RESTRICT_ALL_ABILITIES", REWARD_RESTRICT_ALL_ABILITIES },
	{ "RESTRICT_CORRUPTION", REWARD_RESTRICT_CORRUPTION },
	{ "RESTRICT_BLACK_MARKET", REWARD_RESTRICT_BLACK_MARKET },
	{ "RESTRICT_SABOTAGE", REWARD_RESTRICT_SABOTAGE },
	{ "ENABLE_FLEET_COMBINE", REWARD_ENABLE_FLEET_COMBINE },
	{ "ENABLE_COMBAT_CINEMATIC", REWARD_ENABLE_COMBAT_CINEMATIC },
	{ "RESTRICT_AUTORESOLVE", REWARD_RESTRICT_AUTORESOLVE },
	{ "ENABLE_GALACTIC_CORRUPTION_HOLOGRAM", REWARD_ENABLE_GALACTIC_CORRUPTION_HOLOGRAM },
	{ "UPDATE_OBJECTIVE", REWARD_REPLACE_OBJECTIVE },
	{ "ENABLE_CAMPAIGN_VICTORY_MOVIE", REWARD_ENABLE_CAMPAIGN_VICTORY_MOVIE },

	{ "FILTER_NONE", EVENT_FILTER_NONE },
	{ "FILTER_FRIENDLY_ONLY", EVENT_FILTER_FRIENDLY_ONLY },
	{ "FILTER_NEUTRAL_ONLY", EVENT_FILTER_NEUTRAL_ONLY },
	{ "FILTER_ENEMY_ONLY", EVENT_FILTER_ENEMY_ONLY },
	{ "FILTER_FRIENDLY_AND_NEUTRAL", EVENT_FILTER_FRIENDLY_AND_NEUTRAL },
	{ "FILTER_ENEMY_AND_NEUTRAL", EVENT_FILTER_ENEMY_AND_NEUTRAL },
	{ "FILTER_FRIENDLY_AND_ENEMY", EVENT_FILTER_FRIENDLY_AND_ENEMY },

	{ "GROUND", BASE_GROUND },
	{ "SPACE", BASE_SPACE },
	{ "EITHER", BASE_EITHER },

	{ "CREDIT_VALUE", STATS_CREDIT_VALUE },
	{ "POLITICAL_CONTROL", STATS_POLITICAL_CONTROL },
	{ "MAX_SPECIAL_STRUCTS", STATS_MAX_SPECIAL_STRUCTS },
	{ "MAX_STARBASE_LEVEL", STATS_MAX_STARBASE_LEVEL },
	{ "MAX_GROUND_BASE_LEVEL", STATS_MAX_GROUND_BASE_LEVEL },

	{ "FLASH_AFFILIATION", FLASH_AFFILIATION },
	{ "FLASH_FLEET", FLASH_FLEET },
	{ "FLASH_TROOPS", FLASH_TROOPS },
	{ "FLASH_PLANET_NAME", FLASH_PLANET_NAME },
	{ "FLASH_BASE_BARS", FLASH_BASE_BARS },
	{ "FLASH_CREDITS", FLASH_CREDITS },
	{ "FLASH_PRODUCTION", FLASH_PRODUCTION },
	{ "FLASH_COINS", FLASH_COINS },
	{ "FLASH_SMUGGLER", FLASH_SMUGGLER },
	{ "FLASH_SMUGGLED", FLASH_SMUGGLED },
	{ "FLASH_PLANET_VALUE", FLASH_PLANET_VALUE },
	{ "FLASH_WEATHER", FLASH_WEATHER },
	{ "FLASH_PLANET", FLASH_PLANET },
	{ "FLASH_ABILITY_SLOT", FLASH_ABILITY_SLOT },

	{ "TUTORIAL_CLICK_GUI", TUTORIAL_CLICK_GUI },
	{ "TUTORIAL_CLICK_PLANET", TUTORIAL_CLICK_PLANET },
	{ "TUTORIAL_DRAG_FLEET", TUTORIAL_DRAG_FLEET },
	{ "TUTORIAL_ZOOM", TUTORIAL_ZOOM },
	{ "TUTORIAL_ALL", TUTORIAL_ALL },

	{ "GREATER_THAN", COMPARE_GREATER_THAN },
	{ "LESS_THAN", COMPARE_LESS_THAN },   
	{ "EQUAL_TO", COMPARE_EQUAL_TO },    
	{ "NOT_EQUAL_TO", COMPARE_NOT_EQUAL_TO },
	{ "GREATER_THAN_EQUAL_TO", COMPARE_GREATER_THAN_EQUAL_TO },
	{ "LESS_THAN_EQUAL_TO", COMPARE_LESS_THAN_EQUAL_TO },

	{ "FRIENDLY_CONTAINS", FC_FRIENDLY_CONTAINS },
	{ "FRIENDLY_ONLY_CONTAINS", FC_FRIENDLY_ONLY_CONTAINS },
	{ "ENEMY_CONTAINS", FC_ENEMY_CONTAINS },
	{ "ENEMY_ONLY_CONTAINS", FC_ENEMY_ONLY_CONTAINS },

	{ "CLASS_FIGHTER", CLASS_FIGHTER },
	{ "CLASS_BOMBER", CLASS_BOMBER },
	{ "CLASS_TRANSPORT", CLASS_TRANSPORT },
	{ "CLASS_CORVETTE", CLASS_CORVETTE },
	{ "CLASS_FRIGATE", CLASS_FRIGATE },
	{ "CLASS_CAPITAL", CLASS_CAPITAL },
	{ "CLASS_SUPER", CLASS_SUPER },

	{ "REGION_NONE", REGION_NONE },
	{ "REGION_ORGANIZE", REGION_ORGANIZE },
	{ "REGION_PRODUCTION", REGION_PRODUCTION },
	{ "REGION_SELECTION", REGION_SELECTION },

	{ "DESTROY_ALL", DESTROY_ALL },
	{ "DESTROY_ALL_UNITS", DESTROY_ALL_UNITS },
	{ "DESTROY_ALL_STRUCTURES", DESTROY_ALL_STRUCTURES },
	{ "DESTROY_ALL_INDIGENOUS_SPAWNERS", DESTROY_ALL_INDIGENOUS_SPAWNERS },

	{ "RETREAT_PLAYER", POST_LINKED_TACTICAL_RETREAT_PLAYER },
	{ "RETREAT_AI", POST_LINKED_TACTICAL_RETREAT_AI },
	{ "DESTROY_AI", POST_LINKED_TACTICAL_DESTROY_AI },

	{ "RAID_SLOT", SHOW_RAID_SLOT },
	{ "SMUGGLE_SLOT", SHOW_SMUGGLE_SLOT },
	{ "STEAL_SLOT", SHOW_STEAL_SLOT },
	{ "BLACK_MARKET_SLOT", SHOW_BLACK_MARKET_SLOT },
	{ "SABOTAGE_SLOT", SHOW_SABOTAGE_SLOT },
	{ "CORRUPTION_SLOT", SHOW_CORRUPTION_SLOT },
	{ "REMOVE_CORRUPTION_SLOT", SHOW_REMOVE_CORRUPTION_SLOT },
	{ "NEUTRALIZE_HERO_SLOT", SHOW_NEUTRALIZE_HERO_SLOT },

	{ "CORRUPTION_NONE", CORRUPTION_NONE },
	{ "INTIMIDATION", CORRUPTION_INTIMIDATION },
	{ "PIRACY", CORRUPTION_PIRACY },
	{ "KIDNAPPING", CORRUPTION_KIDNAPPING },
	{ "RACKETEERING", CORRUPTION_RACKETEERING },
	{ "CORRUPT_MILITIA", CORRUPTION_CORRUPT_MILITIA },
	{ "BRIBERY", CORRUPTION_BRIBERY },
	{ "BLACK_MARKET", CORRUPTION_BLACK_MARKET },
	{ "BONDED_CITIZENS", CORRUPTION_BONDED_CITIZENS },
	{ "CORRUPTION_ANY", CORRUPTION_ANY },

	{ NULL, 0 }
};




void (*StoryEventClass::RetryDialog)( void ) = NULL;
bool (*StoryEventClass::RetryDialogActive)( void ) = NULL;



/**************************************************************************************************
* StoryDatabaseParserClass::StoryDatabaseParserClass -- Constructor
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:31PM JAC
**************************************************************************************************/
StoryDatabaseParserClass::StoryDatabaseParserClass() :
	StoryDialogChapter(0),
	StoryDialogPopup(true),
	StoryDialogSFX(true),
	Multiplayer(false),
	InactiveDelay(0),
	Position(0,0,0),
	Perpetual(false),
	Timeout(-1)
{
	Name.clear();
	EventType.clear();
	EventParam1.clear();
	EventParam2.clear();
	EventParam3.clear();
	Filter.clear();
	RewardType.clear();
	RewardParam1.clear();
	RewardParam2.clear();
	RewardParam3.clear();
	RewardParam4.clear();
	RewardParam5.clear();
	RewardParam6.clear();
	RewardParam7.clear();
	RewardParam8.clear();
	RewardParam9.clear();
	RewardParam10.clear();
	RewardParam11.clear();
	RewardParam12.clear();
	RewardParam13.clear();
	RewardParam14.clear();
	AndPrereqs.clear();
	StoryDialog.clear();
	RewardParamList.clear();
}




/**************************************************************************************************
* StoryDatabaseParserClass::Parse_Database_Entry -- Read in XML data
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:31PM JAC
**************************************************************************************************/
void StoryDatabaseParserClass::Parse_Database_Entry(XMLDatabase *database_object_entry)
{
	assert( database_object_entry != NULL );

	static int _recursion_level = 0;

	if ( database_object_entry != NULL ) 
	{
		if ( _recursion_level == 0 )
		{
			//
			// Get the object name in the database
			//
			std::string object_name;
			#ifndef NDEBUG
				HRESULT result = 
			#endif NDEBUG
			database_object_entry->Get_Attribute( "Name", object_name );
			assert(SUCCEEDED(result));
			//
			// Store the name of the object entry in the database
			//
			Name = object_name;
		}

		//
		// Recursively iterate through the object database data, with children
		//
		HRESULT result = S_OK;
		do 
		{
			//
			// Get the key (key = value) for this database node
			//
			std::string key_name;
			result = database_object_entry->Get_Key(&key_name);
			assert(SUCCEEDED(result));

			//
			// Get the value (key = value) for this database node
			//
			std::string value_text;
			result = database_object_entry->Get_Value(&value_text);
			assert(SUCCEEDED(result));

			//
			// If no value, don't bother processing - the key just may be an ID
			//
			if ( value_text.empty() == false ) 
			{
				#if( 0 )
				#ifndef NDEBUG
					for (int index = 0; index < _recursion_level; index++) 
					{
						Debug_Print( "  " );
					}
					Debug_Print("---> %s = %s\r\n", key_name.c_str(), value_text.c_str());
				#endif
				#endif

				//
				// Temporary To-Be-Determined flag converting to empty string
				//
				if( value_text == "TBD" )
				{
					Debug_Print( "Warning: CommandBarComponentClass::Parse_Database_Entry() - %s key %s value is TBD.\r\n", Name.c_str(), key_name.c_str());
					value_text.clear();
				}
				else
				{
					//
					// Attempt to find the db key name in the data table, and then apply the db value to the corresponding class member variable
					//
					bool data_mapped = TheDatabaseMapper.Map_DB_Data_To_StoryParserClass( key_name, value_text, this, database_object_entry );
	
					//
					// Display a warning if the key was not found in the data map list
					//
					if ( data_mapped == false ) 
					{	
						Debug_Print( "Warning: CommandBarComponentClass::Parse_Database_Entry() - Unprocessed entry '%s' in object '%s'.\r\n", key_name.c_str(), Name.c_str() );
					}

					if (AndPrereqs.size())
					{
						DynamicVectorClass<std::string> prereq;
						for (unsigned int i=0; i<AndPrereqs.size(); i++)
						{
							prereq.Add(AndPrereqs[i]);
						}
						Prereqs.Add(prereq);
						AndPrereqs.resize(0);
					}
				}
			}

			//
			// Recurse to child data of object
			//
			if ( database_object_entry->Has_Children() == true ) 
			{
				result = database_object_entry->Descend();
				if ( SUCCEEDED(result) && result != S_FALSE ) 
				{
					_recursion_level ++;
					Parse_Database_Entry( database_object_entry );	// Recursively call this function
					database_object_entry->Ascend();
					_recursion_level --;
				}
			}

			//
			// We want to go to the next database node if we are still parsing the one object entry
			//
			if ( _recursion_level > 0 ) 
			{
				result = database_object_entry->Next_Node();
			}
			//
			// If we are done parsing the one object entry, we need to stop here so the next above can be parsed at a higher elvel
			//
			else
			{
				break;
			}

		} while ( result == S_OK );
	}
}










StoryEventClass *StoryDatabaseParserClass::Create_Event(StoryEventEnum type)
{
	StoryEventClass *event = NULL;

	switch (type)
	{
		case STORY_ENTER:
		case STORY_LAND_ON:
		case STORY_PLANET_DESTROYED:
		case STORY_FLEET_BOUNCED:
		case STORY_INVASION_BOUNCED:
			event = new StoryEventEnterClass;
			break;

		case STORY_SELECT_PLANET:
		case STORY_ZOOM_INTO_PLANET:
		case STORY_ZOOM_OUT_PLANET:
			event = new StoryEventSelectPlanetClass;
			break;

		case STORY_CONQUER:
		case STORY_CONSTRUCT:
		case STORY_CAPTURE_HERO:
		case STORY_DEFEAT_HERO:
		case STORY_REINFORCE:
		case STORY_SELECT_UNIT:
		case STORY_UNIT_ARRIVED:
		case STORY_FULL_STOP:
		case STORY_ATTACK_HARDPOINT:
		case STORY_GARRISON_UNIT:
			event = new StoryEventSingleObjectNameClass;
			break;

		case STORY_CONSTRUCT_LEVEL:
			event = new StoryEventConstructLevelClass;
			break;

		case STORY_DESTROY:
		case STORY_TACTICAL_DESTROY:
			event = new StoryEventDestroyClass;
			break;

		case STORY_DESTROY_BASE:
			event = new StoryEventDestroyBaseClass;
			break;

		case STORY_BEGIN_ERA:
		case STORY_TECH_LEVEL:
			event = new StoryEventBeginEraClass;
			break;

		case STORY_CORRUPTION_CHANGED:
		case STORY_CORRUPTION_TACTICAL_COMPLETE:
		case STORY_CORRUPTION_TACTICAL_FAILED:
			event = new StoryEventCorruptionLevelClass;
			break;

		case STORY_DEPLOY:
			event = new StoryEventHeroMoveClass;
			break;

		case STORY_MOVE:
			event = new StoryEventHeroMoveClass;
			break;

		case STORY_ACCUMULATE:
			event = new StoryEventAccumulateClass;
			break;

		case STORY_CONQUER_COUNT:
			event = new StoryEventConquerCountClass;
			break;

		case STORY_ELAPSED:
			event = new StoryEventElapsedClass;
			break;

		case STORY_WIN_BATTLES:
		case STORY_LOSE_BATTLES:
			event = new StoryEventWinBattlesClass;
			break;

		case STORY_RETREAT:
			event = new StoryEventRetreatClass;
			break;

		case STORY_SPACE_TACTICAL:
		case STORY_LAND_TACTICAL:
			event = new StoryEventStartTacticalClass;
			break;

		case STORY_CLICK_GUI:
		case STORY_SPEECH_DONE:
		case STORY_GENERIC:
		case STORY_MISSION_LOST:
		case STORY_BUY_BLACK_MARKET:
		case STORY_GALACTIC_SABOTAGE:
		case STORY_OPEN_CORRUPTION:
			event = new StoryEventStringClass;
			break;

		case STORY_TRIGGER:
			event = new StoryEventClass;
			break;

		case STORY_FOG_OBJECT_REVEAL:
		case STORY_FOG_POSITION_REVEAL:
			event = new StoryEventFogRevealClass;
			break;

		case STORY_AI_NOTIFICATION:
			event = new StoryEventAINotificationClass;
			break;

		case STORY_COMMAND_UNIT:
			event = new StoryEventCommandUnitClass;
			break;

		case STORY_GUARD_UNIT:
			event = new StoryEventGuardUnitClass;
			break;

		case STORY_UNIT_PROXIMITY:
			event = new StoryEventProximityClass;
			break;

		case STORY_DIFFICULTY_LEVEL:
			event = new StoryEventDifficultyClass;
			break;

		case STORY_FLAG:
			event = new StoryEventFlagClass;
			break;

		case STORY_LOAD_TACTICAL_MAP:
			event = new StoryEventLoadTacticalClass;
			break;

		case STORY_CHECK_DESTROYED:
			event = new StoryCheckDestroyedClass;
			break;

		case STORY_VICTORY:
			event = new StoryEventVictoryClass;
			break;

		case STORY_MOVIE_DONE:
			event = new StoryEventMovieDoneClass;
			break;

		case STORY_OBJECTIVE_TIMEOUT:
			event = new StoryEventObjectiveTimeoutClass;
			break;

		case STORY_CAPTURE_STRUCTURE:
			event = new StoryEventCaptureClass;
			break;

		case STORY_POLITICAL_CONTROL:
		default:
			return (NULL);
			break;
	}

	event->Set_Event_Type(type);
	return (event);
}








/**************************************************************************************************
* StoryDatabaseParserClass::Get_Event -- Create event from parsed XML data
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:32PM JAC
**************************************************************************************************/
StoryEventClass *StoryDatabaseParserClass::Get_Event()
{
	if (EventType.empty() || Name.empty())
	{
		return (NULL);
	}

	StoryEventEnum type = (StoryEventEnum)StoryEventClass::Lookup_Enum(EventType.c_str());
	StoryEventClass *event = NULL;

	event = Create_Event(type);
	if (event == NULL)
	{
		Story_Debug_Printf("STORY MODE ERROR!  Unable to process event of type %s\r\n",EventType.c_str());
		assert(event);
		return NULL;
	}

	event->Set_Name(&Name);
	StoryRewardEnum rewardtype = (StoryRewardEnum)StoryEventClass::Lookup_Enum(RewardType.c_str());
	event->Set_Reward_Type(rewardtype);
	if (!EventParam1.empty()) event->Set_Param(0,&EventParam1);
	if (!EventParam2.empty()) event->Set_Param(1,&EventParam2);
	if (!EventParam3.empty()) event->Set_Param(2,&EventParam3);
	if (!EventParam4.empty()) event->Set_Param(3,&EventParam4);
	if (!EventParam5.empty()) event->Set_Param(4,&EventParam5);
	if (!EventParam6.empty()) event->Set_Param(5,&EventParam6);
	if (!EventParam7.empty()) event->Set_Param(6,&EventParam7);
	if (!RewardParam1.empty()) event->Set_Reward_Param(0,&RewardParam1);
	if (!RewardParam2.empty()) event->Set_Reward_Param(1,&RewardParam2);
	if (!RewardParam3.empty()) event->Set_Reward_Param(2,&RewardParam3);
	if (!RewardParam4.empty()) event->Set_Reward_Param(3,&RewardParam4);
	if (!RewardParam5.empty()) event->Set_Reward_Param(4,&RewardParam5);
	if (!RewardParam6.empty()) event->Set_Reward_Param(5,&RewardParam6);
	if (!RewardParam7.empty()) event->Set_Reward_Param(6,&RewardParam7);
	if (!RewardParam8.empty()) event->Set_Reward_Param(7,&RewardParam8);
	if (!RewardParam9.empty()) event->Set_Reward_Param(8,&RewardParam9);
	if (!RewardParam10.empty()) event->Set_Reward_Param(9,&RewardParam10);
	if (!RewardParam11.empty()) event->Set_Reward_Param(10,&RewardParam11);
	if (!RewardParam12.empty()) event->Set_Reward_Param(11,&RewardParam12);
	if (!RewardParam13.empty()) event->Set_Reward_Param(12,&RewardParam13);
	if (!RewardParam14.empty()) event->Set_Reward_Param(13,&RewardParam14);
	if (!RewardParamList.empty()) event->Set_Reward_Param_List(&RewardParamList);
	if (!BranchName.empty()) event->Set_Branch_Name(BranchName);
	event->Set_Inactive_Delay(InactiveDelay);
	event->Set_Reward_Position(Position);
	event->Set_Perpetual(Perpetual);
	event->Set_Timeout_Time(Timeout);
	if ((type == STORY_SPEECH_DONE) && (Timeout == -1))
	{
		event->Set_Timeout_Time(MAX_SPEECH_TIME);
	}

	Story_Debug_Printf("\tReward type %s\r\n",RewardType.c_str());

	// Add prereqs....
	if (Prereqs.Size())
	{
		event->Add_Prereqs(&Prereqs);
		// This even has prereqs, so it shouldn't start active
		event->Set_Active(false);
	}
	else
	{
		// No prereqs so this must be active
		event->Set_Active(true);
	}

	if (!StoryDialog.empty())
	{
		event->Set_Story_Dialog(&StoryDialog);
		StoryDialogManagerClass::Add_Story(StoryDialog.c_str());
	}

	event->Set_Story_Dialog_Chapter(StoryDialogChapter);
	event->Set_Story_Dialog_Popup(StoryDialogPopup);
	event->Set_Story_Dialog_SFX(StoryDialogSFX);
	if (!StoryDialogVar.empty())
	{
		event->Set_Story_Dialog_Var(&StoryDialogVar);
	}
	if (!StoryDialogTag.empty())
	{
		event->Set_Story_Dialog_Tag(&StoryDialogTag);
	}
	if (!StoryDialogIncoming.empty())
	{
		event->Set_Story_Dialog_Tag(&StoryDialogIncoming);
	}

	event->Set_Multiplayer_Active(Multiplayer);

	return (event);
}









/**************************************************************************************************
* StoryEventClass::StoryEventClass -- Event constructor
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:33PM JAC
**************************************************************************************************/
StoryEventClass::StoryEventClass() :
	EventType(STORY_NONE),
	RewardType(REWARD_NONE),
	Triggered(false),
	Active(false),
	Disabled(false),
	SubPlot(NULL),
	InactiveDelay(0),
	InactiveElapsed(-1),
	Perpetual(false),
	StartTime(-1),
	TimeoutTime(-1)
{
}




/**************************************************************************************************
* ::~StoryEventClass -- Event destructor
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:33PM JAC
**************************************************************************************************/
StoryEventClass::~StoryEventClass()
{
	Prereqs.Clear();
	Dependants.Clear();
}





void StoryEventClass::Set_Dialog_Ptr(void (*retry_dialog_activate)(void), bool (*is_retry_dialog_active)(void))
{
	RetryDialog = retry_dialog_activate;
	RetryDialogActive = is_retry_dialog_active;
}




/**************************************************************************************************
* StoryEventClass::Build_Enum_Lookup_Map -- Convert string to enum
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:33PM JAC
**************************************************************************************************/
void StoryEventClass::Build_Enum_Lookup_Map()
{
	int index = 0;
	StoryEnumRemapStruct *remap;

	remap = &EnumRemap[index];

	while (remap->Text != NULL)
	{
		CRCValue enumcrc = CRCClass::Calculate_CRC(remap->Text, strlen(remap->Text));

		EnumLookup[enumcrc] = remap->EnumValue;

		index++;
		remap = &EnumRemap[index];
	}
}





/**************************************************************************************************
* StoryEventClass::Lookup_Enum -- Find enum
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:34PM JAC
**************************************************************************************************/
int StoryEventClass::Lookup_Enum(const char *text)
{
	if (text == NULL)
	{
		// 0 value should always be NONE
		return (0);
	}

	CRCValue enumcrc = CRCClass::Calculate_CRC(text, strlen(text));
	std::map<CRCValue,int>::iterator enumptr;

	enumptr = EnumLookup.find(enumcrc);

	if (enumptr != EnumLookup.end())
	{
		return (enumptr->second);
	}
	else
	{
		// 0 value should always be NONE
		return (0);
	}
}







/**************************************************************************************************
* StoryEventClass::Event_Filter_Matches -- See if the filter matches
*
* In:		
*
* Out:	
*
*
* History: 11/23/2004 2:47PM JAC
**************************************************************************************************/
bool StoryEventClass::Event_Filter_Matches(GameObjectClass *object, StoryEventFilter filter)
{
	PlayerClass *local_player = SubPlot->Get_Local_Player();
	const FactionClass *player_faction = local_player->Get_Faction();
	const FactionClass *object_faction = FactionList.Get_Faction_From_Allegiance(object->Get_Allegiance());

	switch (filter)
	{
		case EVENT_FILTER_NONE:
			return (true);
			break;

		case EVENT_FILTER_FRIENDLY_ONLY:
			if (player_faction == object_faction)
			{
				return (true);
			}
			break;

		case EVENT_FILTER_NEUTRAL_ONLY:
			if (object_faction->Is_Neutral())
			{
				return (true);
			}
			break;

		case EVENT_FILTER_ENEMY_ONLY:
			// Can the player be neutral?
			if ((object_faction != player_faction) && !object_faction->Is_Neutral())
			{
				return (true);
			}
			break;

		case EVENT_FILTER_FRIENDLY_AND_NEUTRAL:
			if (object_faction->Is_Neutral() || (object_faction == player_faction))
			{
				return (true);
			}
			break;

		case EVENT_FILTER_ENEMY_AND_NEUTRAL:
			if (object_faction->Is_Neutral() || (object_faction != player_faction))
			{
				return (true);
			}
			break;

		case EVENT_FILTER_FRIENDLY_AND_ENEMY:
			// if (object_faction->Is_Good() || object_faction->Is_Evil())
			if (object_faction->Is_Neutral() == false)
			{
				return (true);
			}
			break;

		default:
			return (false);
			break;
	}

	return (false);
}







/**************************************************************************************************
* StoryEventClass::Event_Triggered -- An event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:34PM JAC
**************************************************************************************************/
void StoryEventClass::Event_Triggered(GameObjectClass *planet, bool inactive)
{
	if (Disabled)
	{
		return;
	}

	// If the event was triggered but this event is really riggered by inaction, then the event isn't
	// considered triggered and the timer is reset.
	if ((InactiveDelay > 0) && !inactive)
	{
		Story_Debug_Printf("STORY EVENT INACTIVE - Event %s activity.  Resetting timer\r\n",Get_Name()->c_str());
		InactiveElapsed = -1;
		return;
	}

	Story_Debug_Printf("Story event %s triggered\r\n",EventName.c_str());

	if (SubPlot)
	{
		SubPlot->Lua_Trigger_Event(EventName);
		if (TimeoutTime != -1)
		{
			SubPlot->Remove_Timeout_Event(this);
		}
	}

	Triggered = true;

	Give_Reward(planet);

	if (!StoryDialog.empty())
	{
		bool remove_completed = false;
		std::string tag = StoryDialogTag;
		if (tag.empty())
		{
			// Create a unique tag if one isn't provided
			char unique_name[128];
			sprintf(unique_name,"%s_%d",StoryDialog.c_str(),StoryDialogChapter);
			tag = unique_name;
			remove_completed = true;
		}

		// Add this story to the list
		StoryDialogManagerClass::Add_Story_Dialog_Goal(StoryDialog.c_str(),StoryDialogChapter,&StoryDialogVar,&tag,&StoryDialogExtraText,remove_completed);

		if (StoryDialogManagerClass::Is_Summary_Active())
		{
			StoryDialogManagerClass::Refresh_Story_Summary();
		}
		else if (StoryDialogPopup)
		{			
			StoryDialogManagerClass::Launch_Story_Summary(StoryDialog.c_str(),StoryDialogChapter);
		}
		else
		{
			BaseComponentClass *component = Component_Logic_Get_Component(COMPONENT_ID_STORY_ARC_BUTTON);
			if (component)
			{
				// Flash the story arc button to indicate that a new story event available
				component->Flash(false,true);
			}

			//if (StoryDialogIncoming.empty())
			//{
			//	TheCommandBar.Play_Movie("Default_Incoming_Transmission",false);
			//}
			//else
			//{
			//	TheCommandBar.Play_Movie(StoryDialogIncoming.c_str(),false);
			//}

			// Play the "incoming transmission" SFX
			//TheSFXEventManager.Start_SFX_Event("GUI_Transmission_SFX");

			PlayerClass *local_player = PlayerList.Get_Local_Player();
			ENFORCED_IF( local_player != NULL && local_player->Get_Faction() != NULL )
			{
				TheCommandBar.Add_Advisor_Text(TheGameText.Get(local_player->Get_Faction()->Get_Default_Transmission_Message_ID()),
														 RGBAClass(255, 255, 255), true);

				// Play the "incoming transmission" V/O
				if (StoryDialogSFX)
				{
					const SFXEventClass *sfx_event = local_player->Get_Faction()->Get_SFXEvent_Mission_Added();
					if ( sfx_event != NULL )
					{
						TheSFXEventManager.Start_SFX_Event( sfx_event );
					}
				}
			}
		}
	}

	for (int i=0; i<Dependants.Size(); i++)
	{
		Dependants[i]->Parent_Triggered();
	}

	// Perpetual events can be triggered any number of times
	if (Perpetual)
	{
		Triggered = false;
		Reset();

		// The Reset() call may turn Active to false.  In this case we want to keep it true unless it's a STORY_TRIGGER.
		// In that case, it will never get triggered again because it only gets triggered when it's inactive
		if (EventType != STORY_TRIGGER)
		{
			Active = true;
			StartTime = GameModeManager.Get_Frame_Timer() * FrameSynchronizer.Get_Inv_Logical_FPS();
		}
	}
}





/**************************************************************************************************
* StoryEventClass::Parent_Triggered -- Another event has been triggered.  Check to see if this needs to be activated
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:35PM JAC
**************************************************************************************************/
void StoryEventClass::Parent_Triggered()
{
	if (Is_Active() || Is_Triggered())
	{
		// If it's already active or if it's already triggered don't do anything
		// It's OK if it's disabled since it can be both Active and Disabled.  Disabled has precedence
		return;
	}

	// See if all of the prerequisites have been satisfied
	for (int i=0; i<Prereqs.Size(); i++)
	{
		DynamicVectorClass<StoryEventClass *> *andlist = &Prereqs[i];
		bool allactive = true;

		for (int j=0; j<andlist->Size(); j++)
		{
			StoryEventClass *event = (*andlist)[j];
			if (!event->Is_Triggered())
			{
				allactive = false;
			}
		}

		if (allactive == true)
		{
			Active = true;

			// Save off the start time and add it to the timeout list if a timeout is specified
			StartTime = GameModeManager.Get_Frame_Timer() * FrameSynchronizer.Get_Inv_Logical_FPS();
			if (TimeoutTime != -1)
			{
				Story_Debug_Printf("Adding timeout event %s, time %f, timeout %f\r\n",EventName.c_str(),StartTime,StartTime+TimeoutTime);
				SubPlot->Add_Timout_Event(this);
			}

			if (EventType == STORY_TRIGGER)
			{
				// Special case for trigger events.  Trigger events are triggered when all prereqs are true
				Event_Triggered();
			}
			else
			{
				Activate();
			}

			return;
		}
	}
}


/**************************************************************************************************
* StoryEventClass::Reset -- 
*
* In:		
*
* Out:	
*
*
* History: 06/14/2005 1:49PM JSY
**************************************************************************************************/
void StoryEventClass::Reset()
{
	if (Prereqs.Size() > 0)
	{
		Active = false;
		if (TimeoutTime != -1)
		{
			SubPlot->Remove_Timeout_Event(this);
		}
	}
}




/**************************************************************************************************
* StoryEventClass::Add_Prereqs -- Add a list of prerequisites to this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:35PM JAC
**************************************************************************************************/
void StoryEventClass::Add_Prereqs(DynamicVectorClass<DynamicVectorClass<std::string> > *prereqs)
{
//	int index = Prereqs.Size();

	for (int i=0; i<prereqs->Size(); i++)
	{
		DynamicVectorClass<std::string> *stringvec = &(*prereqs)[i];
		DynamicVectorClass<std::string> tempvec;

		for (int j=0; j<stringvec->Size(); j++)
		{
			tempvec.Add((*stringvec)[j]);
		}

		RawPrereqs.Add(tempvec);
	}
}




/**************************************************************************************************
* StoryEventClass::Compute_Dependants -- Determine what events this event relies on
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:36PM JAC
**************************************************************************************************/
void StoryEventClass::Compute_Dependants()
{
	assert(SubPlot);

	// First we have to convert the raw (string) prerequisite lists into a regular list pointing to the prereqs
	for (int i=0; i<RawPrereqs.Size(); i++)
	{
		DynamicVectorClass<std::string> *rawreqs = &RawPrereqs[i];
		DynamicVectorClass<StoryEventClass *> tempvec;

		for (int j=0; j<rawreqs->Size(); j++)
		{
			StoryEventClass *event = SubPlot->Get_Event((*rawreqs)[j].c_str());
			if (event == NULL)
			{
				Debug_Printf("\r\nERROR!  Story event - Unable to find event %s in event %s, plot %s\r\n",(*rawreqs)[j].c_str(),EventName.c_str(),SubPlot->Get_Name().c_str());
				assert(event);
				continue;
			}

			tempvec.Add(event);
		}

		Prereqs.Add(tempvec);
	}

	// We're done with this data
	RawPrereqs.Clear();

	// Now that we know all the events this event relies on, tell those events to notify this event
	// when they get triggered so this event can determine if it should be activated
	for (int i=0; i<Prereqs.Size(); i++)
	{
		DynamicVectorClass<StoryEventClass *> *andlist = &Prereqs[i];

		for (int j=0; j<andlist->Size(); j++)
		{
			if ((*andlist)[j] != NULL)
			{
				(*andlist)[j]->Add_Dependant(this);
			}
			else
			{
				Story_Debug_Printf("ERROR!  Compute_Dependants has come across an invalid dependant for %s in plot %s\r\n",EventName.c_str(),SubPlot->Get_Name().c_str());
			}
		}
	}
}




/**************************************************************************************************
* StoryEventClass::Add_Dependant -- The supplied event relies on this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:36PM JAC
**************************************************************************************************/
void StoryEventClass::Add_Dependant(StoryEventClass *dependant)
{
	for (int i=0; i<Dependants.Size(); i++)
	{
		// Check to see if this dependant is already in the list
		if (Dependants[i] == dependant)
		{
			return;
		}
	}

	Dependants.Add(dependant);
}









/**************************************************************************************************
* StoryEventClass::Give_Reward -- Give player reward for triggering event
*
* In:		
*
* Out:	
*
*
* History: 07/23/2004 2:00PM JAC
**************************************************************************************************/
void StoryEventClass::Give_Reward(GameObjectClass *planet)
{
	switch (RewardType)
	{
		case REWARD_BUILDABLE_UNIT:
			Reward_Buildable_Unit();
			break;

		case REWARD_UNIQUE_UNIT:
			Reward_Unique_Unit(planet);
			break;

		case REWARD_REMOVE_UNIT:
			Reward_Remove_Unit();
			break;

		case REWARD_CREDITS:
			Reward_Credits();
			break;

		case REWARD_SPAWN_HERO:
			// Just calls Reward_Unique_Unit
			Reward_Spawn_Hero(planet);
			break;

		case REWARD_INFORMATION:
			Reward_Information();
			break;

		case REWARD_STORY_ELEMENT:
			Reward_Story_Element();
			break;

		case REWARD_STATISTIC_CHANGE:
			Reward_Statistic_Change(planet);
			break;

		case REWARD_NEW_POWER_FOR_ALL:
			Reward_New_Power_For_All();
			break;

		case REWARD_REMOVE_POWER_FROM_ALL:
			Reward_Remove_Power_From_All();
			break;

		case REWARD_SFX:
			Reward_SFX();
			break;

		case REWARD_SPEECH:
			Reward_Speech();
			break;

		case REWARD_FLASH_GUI:
			Reward_Flash_GUI();
			break;

		case REWARD_FLASH_PLANET_GUI:
			Reward_Flash_Planet_GUI();
			break;

		case REWARD_FLASH_PLANET:
			Reward_Flash_Planet();
			break;

		case REWARD_HIDE_TUTORIAL_CURSOR:
			Reward_Hide_Tutorial_Cursor();
			break;

		case REWARD_SCREEN_TEXT:
			Reward_Screen_Text();
			break;

		case REWARD_DISABLE_EVENT:
			Reward_Disable_Event();
			break;

		case REWARD_ENABLE_EVENT:
			Reward_Enable_Event();
			break;

		case REWARD_PICK_PLANET:
			Reward_Pick_Planet();
			break;

		case REWARD_SWITCH_SIDES:
			Reward_Switch_Sides();
			break;

		case REWARD_ZOOM_IN:
			Reward_Zoom_In();
			break;

		case REWARD_ZOOM_OUT:
			Reward_Zoom_Out();
			break;

		case REWARD_PAUSE_GAME:
			Reward_Pause_Game();
			break;

		case REWARD_TUTORIAL_DIALOG:
			Reward_Tutorial_Dialog_Box();
			break;

		case REWARD_POSITION_CAMERA:
			Reward_Position_Camera();
			break;

		case REWARD_LOCK_CONTROLS:
			Reward_Lock_Controls();
			break;

		case REWARD_SCROLL_CAMERA:
			Reward_Scroll_Camera();
			break;

		case REWARD_FLASH_OBJECT:
			Reward_Flash_Object();
			break;

		case REWARD_ENABLE_FOW:
			Reward_Toggle_FOW();
			break;

		case REWARD_ENABLE_VICTORY:
			Reward_Enable_Victory();
			break;

		case REWARD_MOVE_FLEET:
			Reward_Move_Fleet();
			break;

		case REWARD_TRIGGER_AI:
			Reward_Trigger_AI();
			break;

		case REWARD_DISABLE_STORY_EVENT:
			Reward_Disable_Story_Event();
			break;

		case REWARD_DISABLE_BRANCH:
			Reward_Disable_Branch();
			break;

		case REWARD_INVADE_PLANET:
			Reward_Invade_Planet();
			break;

		case REWARD_LOAD_CAMPAIGN:
			Reward_Load_Campaign();
			break;

		case REWARD_FLASH_TERRAIN:
			Reward_Flash_Terrain();
			break;

		case REWARD_SET_HEALTH:
			Reward_Set_Health();
			break;

		case REWARD_SET_TACTICAL_MAP:
			Reward_Set_Tactical_Map();
			break;

		case REWARD_DISABLE_AUTORESOLVE:
			Reward_Disable_Autoresolve();
			break;

		case REWARD_ENABLE_AUTORESOLVE:
			Reward_Disable_Autoresolve();
			break;

		case REWARD_DISABLE_MOVIES:
			Reward_Disable_Movies();
			break;

		case REWARD_REMOVE_STORY_GOAL:
			Reward_Remove_Story_Goal();
			break;

		case REWARD_CHANGE_OWNER:
			Reward_Change_Owner();
			break;

		case REWARD_DESTROY_OBJECT:
			Reward_Destroy_Object();
			break;

		case REWARD_VICTORY:
			Reward_Victory();
			break;

		case REWARD_SWITCH_CONTROL:
			Reward_Switch_Control();
			break;

		case REWARD_DISABLE_RETREAT:
			Reward_Disable_Retreat();
			break;

		case REWARD_FLASH_UNIT:
			Reward_Flash_Unit();
			break;

		case REWARD_REVEAL_PLANET:
			Reward_Reveal_Planet();
			break;

		case REWARD_DUAL_FLASH:
			Reward_Dual_Flash();
			break;

		case REWARD_SET_FLAG:
			Reward_Set_Flag();
			break;

		case REWARD_INCREMENT_FLAG:
			Reward_Increment_Flag();
			break;

		case REWARD_HILITE_OBJECT:
			Reward_Hilite_Object();
			break;

		case REWARD_STORY_GOAL_COMPLETED:
			Reward_Story_Goal_Completed();
			break;

		case REWARD_REVEAL_ALL_PLANETS:
			Reward_Reveal_All_Planets();
			break;

		case REWARD_DISABLE_REINFORCEMENTS:
			Reward_Disable_Reinforcements();
			break;

		case REWARD_RESET_BRANCH:
			Reward_Reset_Branch();
			break;

		case REWARD_RESET_EVENT:
			Reward_Reset_Event();
			break;

		case REWARD_LINK_TACTICAL:
			Reward_Link_Tactical();
			break;

		case REWARD_RANDOM_STORY:
			Reward_Random_Story();
			break;

		case REWARD_DISABLE_SPECIAL_STRUCTURE:
			Reward_Disable_Special_Structure();
			break;

		case REWARD_SET_TECH_LEVEL:
			Reward_Set_Tech_Level();
			break;

		case REWARD_DISABLE_SPAWN:
			Reward_Disable_Spawn();
			break;

		case REWARD_TRIGGER_EVENT:
			Reward_Trigger_Event();
			break;

		case REWARD_ENABLE_GALACTIC_REVEAL:
			Reward_Enable_Galactic_Reveal();
			break;

		case REWARD_ACTIVATE_RETRY_DIALOG:
			Reward_Activate_Retry_Dialog();
			break;

		case REWARD_SET_USE_RETRY_DIALOG:
			Reward_Set_Use_Retry_Dialog();
			break;

		case REWARD_START_MOVIE:
			Reward_Start_Movie();
			break;

		case REWARD_SET_PLANET_SPAWN:
			Reward_Set_Planet_Spawn();
			break;

		case REWARD_COMMANDBAR_MOVIE:
			Reward_Commandbar_Movie();
			break;

		case REWARD_STOP_COMMANDBAR_MOVIE:
			Reward_Stop_Commandbar_Movie();
			break;

		case REWARD_SET_WEATHER:
			Reward_Set_Weather();
			break;

		case REWARD_PLANET_FACTION:
			Reward_Planet_Faction();
			break;

		case REWARD_LOCK_UNIT:
			Reward_Lock_Unit();
			break;

		case REWARD_FORCE_RETREAT:
			Reward_Force_Retreat();
			break;

		case REWARD_SET_PLANET_RESTRICTED:
			Reward_Set_Planet_Restricted();
			break;

		case REWARD_MULTIMEDIA:
			Reward_Multimedia();
			break;

		case REWARD_SET_PLANET_VISIBILITY_LEVEL:
			Reward_Set_Planet_Visibility_Level();
			break;

		case REWARD_PAUSE_GALACTIC_GAME:
			Reward_Pause_Galactic_Game();
			break;

		case REWARD_ADD_OBJECTIVE:
			Reward_Add_Objective();
			break;

		case REWARD_REMOVE_OBJECTIVE:
			Reward_Remove_Objective();
			break;

		case REWARD_OBJECTIVE_COMPLETE:
			Reward_Objective_Complete();
			break;

		case REWARD_REMOVE_ALL_OBJECTIVES:
			Reward_Remove_All_Objectives();
			break;

		case REWARD_OBJECTIVE_FAILED:
			Reward_Objective_Failed();
			break;

		case REWARD_ENABLE_DIRECT_INVASION:
			Reward_Enable_Direct_Invasion();
			break;

		case REWARD_DISABLE_DIRECT_INVASION:
			Reward_Disable_Direct_Invasion();
			break;

		case REWARD_ENABLE_OBJECTIVE_DISPLAY:
			Reward_Enable_Objective_Display();
			break;

		case REWARD_FLASH_FLEET_WITH_UNIT:
			Reward_Flash_Fleet_With_Unit();
			break;

		case REWARD_SELECT_PLANET:
			Reward_Select_Planet();
			break;

		case REWARD_FORCE_CLICK_GUI:
			Reward_Force_Click_GUI();
			break;

		case REWARD_FLASH_PRODUCTION_CHOICE:
			Reward_Flash_Production_Choice();
			break;

		case REWARD_SET_MAX_TECH_LEVEL:
			Reward_Set_Max_Tech_Level();
			break;

		case REWARD_TUTORIAL_PLAYER:
			Reward_Tutorial_Player();
			break;

		case REWARD_PAUSE_GALACTIC:
			Reward_Pause_Galactic();
			break;

		case REWARD_UNPAUSE_GALACTIC:
			Reward_Unpause_Galactic();
			break;

		case REWARD_FLASH_SPECIAL_ABILITY:
			Reward_Flash_Special_Ability();
			break;

		case REWARD_SET_SANDBOX_OBJECTIVES:
			Reward_Set_Sandbox_Objectives();
			break;

		case REWARD_FORCE_RESPAWN:
			Reward_Force_Respawn();
			break;

		case REWARD_SCROLL_LOCK:
			Reward_Scroll_Lock();
			break;

		case REWARD_SKIRMISH_RULES:
			Reward_Skirmish_Rules();
			break;

		case REWARD_RESET_GALACTIC_FILTERS:
			Reward_Reset_Galactic_Filters();
			break;

		case REWARD_SET_ADVISOR:
			Reward_Set_Advisor();
			break;

		case REWARD_START_CINEMATIC_MODE:
			Reward_Start_Cinematic_Mode();
			break;

		case REWARD_STOP_CINEMATIC_MODE:
			Reward_Stop_Cinematic_Mode();
			break;

		case REWARD_SHOW_COMMAND_BAR:
			Reward_Show_Command_Bar();
			break;

		case REWARD_HIDE_AUTORESOLVE:
			Reward_Hide_Autoresolve();
			break;

		case REWARD_DISABLE_BUILDABLE:
			Reward_Disable_Buildable();
			break;

		case REWARD_ENABLE_BUILDABLE:
			Reward_Enable_Buildable();
			break;

		case REWARD_HIDE_CURSOR_ON_CLICK:
			Reward_Hide_Cursor_On_Click();
			break;

		case REWARD_LOCK_PLANET_SELECTION:
			Reward_Lock_Planet_Selection();
			break;

		case REWARD_SHOW_SMUGGLE_SLOT:
			Reward_Show_Smuggle_Slot();
			break;

		case REWARD_HIDE_SMUGGLE_SLOT:
			Reward_Hide_Smuggle_Slot();
			break;

		case REWARD_SHOW_RAID_SLOT:
			Reward_Show_Raid_Slot();
			break;

		case REWARD_HIDE_RAID_SLOT:
			Reward_Hide_Raid_Slot();
			break;

		case REWARD_SHOW_STEAL_SLOT:
			Reward_Show_Steal_Slot();
			break;

		case REWARD_HIDE_STEAL_SLOT:
			Reward_Hide_Steal_Slot();
			break;

		case REWARD_FINISHED_TUTORIAL:
			Reward_Finished_Tutorial();
			break;

		case REWARD_BOMBARD_OVERRIDE_DELAY:
			Reward_Bombard_Override_Delay();
			break;

		case REWARD_ENABLE_BOUNTY_COLLECTION:
			Reward_Enable_Bounty_Collection();
			break;

		case REWARD_DISABLE_BOUNTY_COLLECTION:
			Reward_Disable_Bounty_Collection();
			break;

		case REWARD_REMOVE_CORRUPTION:
			Reward_Remove_Corruption();
			break;

		case REWARD_GIVE_BLACK_MARKET:
			Reward_Give_Black_Market();
			break;

		case REWARD_SHOW_SPECIAL_SLOT:
			Reward_Show_Special_Slot();
			break;

		case REWARD_SABOTAGE_STRUCTURE:
			Reward_Sabotage_Structure();
			break;

		case REWARD_ENABLE_SABOTAGE:
			Reward_Enable_Sabotage();
			break;

		case REWARD_FLASH_ADVANCED_MAP_OBJECT:
			Reward_Flash_Advanced_Map_Object();
			break;

		case REWARD_ENABLE_INVASION:
			Reward_Enable_Invasion();
			break;

		case REWARD_RESTRICT_ALL_ABILITIES:
			Reward_Restrict_All_Abilities();
			break;

		case REWARD_RESTRICT_CORRUPTION:
			Reward_Restrict_Corruption();
			break;

		case REWARD_RESTRICT_BLACK_MARKET:
			Reward_Restrict_Black_Market();
			break;

		case REWARD_RESTRICT_SABOTAGE:
			Reward_Restrict_Sabotage();
			break;

		case REWARD_ENABLE_FLEET_COMBINE:
			Reward_Enable_Fleet_Combine();
			break;

		case REWARD_ENABLE_COMBAT_CINEMATIC:
			Reward_Enable_Combat_Cinematic();
			break;

		case REWARD_RESTRICT_AUTORESOLVE:
			Reward_Restrict_Autoresolve();
			break;

		case REWARD_ENABLE_GALACTIC_CORRUPTION_HOLOGRAM:
			Reward_Enable_Galactic_Corruption_Hologram();
			break;

		case REWARD_REPLACE_OBJECTIVE:
			Reward_Replace_Objective();
			break;

		case REWARD_ENABLE_CAMPAIGN_VICTORY_MOVIE:
			Reward_Enable_Campaign_Victory_Movie();
			break;

		case REWARD_NONE:
			break;

		default:
			// This is still considered an error since events without rewards will hit REWARD_NONE.  This only hits
			// if a new reward hasn't been added to the case statement.
			Story_Debug_Printf("STORY REWARD - ERROR!  Reward for event %s needs some code support!\r\n",EventName.c_str());
			break;
	}
}





/**************************************************************************************************
* StoryEventClass::Reward_Buildable_Unit -- Enable and possibly disable building of a unit
*
* In:		
*
* Out:	
*
*
* History: 07/23/2004 4:36PM JAC
**************************************************************************************************/
void StoryEventClass::Reward_Buildable_Unit()
{
	PlayerClass *subplot_local_player = SubPlot->Get_Local_Player();

	const GameObjectTypeClass *type_to_lock = NULL;		// may remain NULL
	if (!RewardParam[0].empty())
	{
		type_to_lock = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	}
	if (type_to_lock)
	{
		subplot_local_player->Remove_Object_Type_From_Build_Locked_List(type_to_lock);
		subplot_local_player->Add_Object_Type_To_Build_Unlocked_List(type_to_lock);
		Debug_Printf("STORY REWARD - Unlocking unit %s\r\n", RewardParam[0].c_str());
	}


	const GameObjectTypeClass *type_to_unlock = NULL;	// may remain NULL
	if (!RewardParam[1].empty())
	{
		type_to_unlock = GameObjectTypeManager.Find_Object_Type(RewardParam[1].c_str());
	}
	if (type_to_unlock)
	{
		subplot_local_player->Add_Object_Type_To_Build_Locked_List(type_to_unlock);
		Debug_Printf("STORY REWARD - Locking unit %s\r\n", RewardParam[1].c_str());
	}

	// New buildable unlocked - play the SFXEvent for new construction options available
	ENFORCED_IF( subplot_local_player->Get_Faction() != NULL )
	{
		// MBL 11.5.2005 ADDED THIS CHECK - Only play if this is the real local player
		if ( subplot_local_player == PlayerList.Get_Local_Player() && type_to_unlock )
		{

			// MBL 11.5.2005 Temporay disable since playing when campaign scenario starts right up
			// Chuck wants this removed for the demo. Jason will need to add an optional flag
			// to this function for it the audio should play or not. It should not play during
			// initial campaign startup, but it should play when a reward occurs during the
			// course of the campaign
			#if( 0 )
				const SFXEventClass *sfx_event = subplot_local_player->Get_Faction()->Get_SFXEvent_New_Construction_Options_Available();
				if ( sfx_event != NULL )
				{
					TheSFXEventManager.Start_SFX_Event( sfx_event );
				}
			#endif
		}
	}

	// Need to check to see if we should advance the tech level if the player is rebel
	const FactionClass *owner_faction = subplot_local_player->Get_Faction();
	if (FactionList.Find_Faction_By_Name(TheGameConstants.Get_Good_Side_Name().c_str()) == owner_faction)
	{
		unsigned int tech_level = subplot_local_player->Get_Tech_Level();
		int total_num_types = GameObjectTypeManager.Get_Total_Object_Types();
		int type_idx = 0;
		bool advance_level = true;

		for (; type_idx < total_num_types; ++type_idx)
		{
			// Get this type.
			const GameObjectTypeClass *type = GameObjectTypeManager.Get_Game_Object_Type(type_idx);

			// Can the player's faction build this kind of object?
			if ( type->Is_Affiliated_With_Faction(owner_faction) == false )
				continue;

			// Is it buildable at this tech level?
			if ( type->Get_Tech_Level() > tech_level )
				continue;

			// Is it initially locked?
			if ( type->Is_Build_Initially_Locked() == false )
				continue;

			// Does the player already know how to build it?
			if ( subplot_local_player->Is_Object_Type_On_Build_Unlocked_List(type) == true )
				continue;

			// Can it be unlocked by a slicer?
			if ( type->Can_Be_Unlocked_By_Slicer() == false )
				continue;

			advance_level = false;
			break;
		}

		// If there's nothing 
		if (advance_level)
		{
			subplot_local_player->Increment_Tech_Level();
		}
	}
}



void StoryEventClass::Reward_Lock_Unit()
{
	PlayerClass *local_player = SubPlot->Get_Local_Player();

	const GameObjectTypeClass *locktype = NULL;
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - Error! Lock Unit - No unit specified\r\n");
	}
	else
	{
		locktype = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	}

	if (locktype)
	{
		local_player->Add_Object_Type_To_Build_Locked_List(locktype);
		Story_Debug_Printf("STORY REWARD - Locking production of %s\r\n",RewardParam[0].c_str());
	}
}





/**************************************************************************************************
* StoryEventClass::Reward_Unique_Unit -- Create a unique unit
*
* In:		
*
* Out:	
*
*
* History: 07/23/2004 2:00PM JAC
**************************************************************************************************/
void StoryEventClass::Reward_Unique_Unit(GameObjectClass *intended_planet)
{
	assert(!RewardParam[0].empty());
	PlayerClass *local_player = SubPlot->Get_Local_Player();

	const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	FAIL_IF( type == NULL )
	{
		Story_Debug_Printf("Reward Unique Unit - Invalid object type specified\r\n");
		return;
	}

	// First find the type that defines the planet where we're creating the object
	if (!RewardParam[1].empty())
	{
		// If a planet is specified in the XML, place it there
		GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[1].c_str());
		assert(planet_type);
		const DynamicVectorClass<GameObjectClass *> *planet_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);

		if (planet_list->Size() == 0)
		{
			planet_list = GAME_OBJECT_MANAGER.Find_Objects(BEHAVIOR_PLANET,local_player->Get_ID());
			if (planet_list->Size() == 0)
			{
				Story_Debug_Printf("STORY REWARD - Reward Unique Unit - can't find a planet!\r\n");
				return;
			}
		}

		intended_planet = (*planet_list)[0];
	}

	if (intended_planet == NULL)
	{
		Story_Debug_Printf("STORY REWARD - Reward Unique Unit - No planet specified\r\n");
		return;
	}

	// Find the planet nearest to the given one that is owned by the appropriate player (could be
	// the same planet).
	GameObjectClass *place_on_planet = intended_planet;
	PlanetaryBehaviorClass *planet_behavior = (PlanetaryBehaviorClass*)intended_planet->Get_Behavior(BEHAVIOR_PLANET);
	ENFORCED_IF( planet_behavior != NULL )
	{
		place_on_planet = planet_behavior->Find_Nearest_Planet_Owned_By_Player(intended_planet, local_player->Get_ID());
	}

	//Attempt to ensure that space is safe.
	PlanetaryBehaviorClass *place_planet_behavior = static_cast<PlanetaryBehaviorClass*>(place_on_planet->Get_Behavior(BEHAVIOR_PLANET));
	ENFORCED_IF(place_planet_behavior)
	{
		PlayerClass *orbit_player = place_planet_behavior->Get_Occupying_Orbit_Player(place_on_planet);
		if (orbit_player && orbit_player != local_player)
		{
			//Oh dear.  Space is not safe at all.  Let's keep looking then...
			GameObjectClass *alternative_place_on_planet = planet_behavior->Find_Nearest_Planet_With_Orbit_Safe_For_Player(intended_planet, local_player->Get_ID());
			if (alternative_place_on_planet)
			{
				place_on_planet = alternative_place_on_planet;
			}
		}
	}

	Story_Debug_Printf("STORY CREATE - CREATING %s NEAR %s (chose %s)\r\n",RewardParam[0].c_str(),intended_planet->Get_Type()->Get_Name()->c_str(),place_on_planet->Get_Type()->Get_Name()->c_str());

	// Now create the object at the planet - could return a NULL object if a special type
	ProductionBehaviorClass *pbehavior = static_cast<ProductionBehaviorClass *> (place_on_planet->Get_Behavior( BEHAVIOR_PRODUCTION ));
	assert(pbehavior);

	//Optional third parameter is number of objects to create
	int create_count = 1;
	if (!RewardParam[2].empty())
	{
		create_count = atoi(RewardParam[2].c_str());
	}

	for (int i = 0; i < create_count; ++i)
	{
		GameObjectClass *new_object = pbehavior->Create_And_Place_Object_Type_At_Location(type,place_on_planet,local_player->Get_ID());

		// Revamped Galactic GUI - Adding concept of orbital "slots" at the planets for fleets
		// For human players, optimize so there there is only one fleet in any of the 4 guard slots
		if ( new_object != NULL )
		{
			//Do this stuff before calling Unify_Orbiting_Guard_Position_Fleets_For_Non_AI_Player.  It's not a big deal since
			//that function is currently empty, but if it ever got reimplemented it might merge the new object into some other fleet
			//and schedule the newly created object for deletion.
			if (local_player->Get_Is_AI_Controlled())
			{
				AIFreeStoreClass *freestore = local_player->Get_AI_Player()->Get_Tactical_Manager_By_Mode(GameModeManager.Get_Sub_Type())->
					Get_Execution_System()->Get_Free_Store();
				if (freestore) 
					freestore->Add_Free_Store_Object(new_object);
			}

			// We may have spawned a new hero.  Let the command bar know so it can flash the hero icon
			TheCommandBar.Hero_Spawned(place_on_planet,new_object);

			//More user feeback - flash the new icon and the radar map.
			if (PlayerList.Get_Local_Player() == local_player)
			{
				if (new_object->Behaves_Like(BEHAVIOR_FLEET))
				{
					TheCommandBar.Flash_Fleet(new_object, NULL, false);
				}
				else if (new_object->Get_Parent_Container_Object())
				{
					if (new_object->Get_Parent_Container_Object()->Behaves_Like(BEHAVIOR_FLEET))
					{
						TheCommandBar.Flash_Fleet(new_object->Get_Parent_Container_Object(), NULL, false);
					}
					else if (new_object->Get_Parent_Container_Object()->Behaves_Like(BEHAVIOR_PLANET))
					{
						TheCommandBar.Flash_Planet_Component(new_object->Get_Parent_Container_Object()->Get_Type()->Get_Name()->c_str(), 
																			FLASH_TROOPS, -1, NULL, false);
					}
				}

				TheCommandBar.Flash_Galactic_Radar(place_on_planet, true, 1.5f);
			}

			PlanetaryBehaviorClass *planet_behavior = (PlanetaryBehaviorClass *) place_on_planet->Get_Behavior( BEHAVIOR_PLANET );
			assert( planet_behavior != NULL );
			if ( local_player->Is_Human() ) 
			{
				planet_behavior->Unify_Orbiting_Guard_Position_Fleets_For_Non_AI_Player( place_on_planet, local_player );
			}
		}
	}

	if (PlayerList.Get_Local_Player() == local_player)
	{
	}

	//We created units.  We can be sure that we didn't create them on a planet containing enemy ground units, but
	//(though we try) we can't be 100% that we didn't create them in space at the same location as enemy ships.
	//If necessary we should start up a space battle.
	const DynamicVectorClass<GameObjectClass*> *fleet_list = place_on_planet->Get_Planetary_Data()->Get_Orbiting_Fleets();
	if (fleet_list && fleet_list->Size() > 0)
	{
		//Need to be sure to check for battle using a non stealth fleet
		GameObjectClass *test_fleet = NULL;
		for (int i = 0; i < fleet_list->Size(); ++i)
		{
			if (!fleet_list->Get_At(i)->Skips_Space_Combat())
			{
				test_fleet = fleet_list->Get_At(i);
				break;
			}
		}

		if (!test_fleet)
		{
			return;
		}

		FleetBehaviorClass *fleet_behavior = static_cast<FleetBehaviorClass*>(test_fleet->Get_Behavior(BEHAVIOR_FLEET));
		ENFORCED_IF(fleet_behavior)
		{
			fleet_behavior->Unlink_Fleet_From_Planetary_Orbit(place_on_planet);
			if (fleet_behavior->Link_Fleet_To_Planetary_Orbit(place_on_planet))
			{
				//Conflict!  Collect nearby fleets and get the show started
				FleetLocomotorBehaviorClass *fleet_locomotor = static_cast<FleetLocomotorBehaviorClass*>(test_fleet->Get_Behavior(BEHAVIOR_LOCO));
				ENFORCED_IF(fleet_locomotor)
				{
					fleet_locomotor->Add_Nearby_Fleets_To_Planet(test_fleet, place_on_planet);
				}

				//The local player is definitely the aggressor since he's the one with the units that only just showed up.
				test_fleet->Get_Manager()->Initiate_Space_Conflict(place_on_planet, local_player->Get_ID());
			}
		}
	}

}



/**************************************************************************************************
* StoryEventClass::Reward_Remove_Unit -- Remove a unit from the game
*
* In:		
*
* Out:	
*
*
* History: 11/23/2004 2:46PM JAC
**************************************************************************************************/
void StoryEventClass::Reward_Remove_Unit()
{
	assert(!RewardParam[0].empty());

	const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());

	if (type != NULL)
	{
		// Force the commandbar to stop dragging anything since the player might be dragging the
		// unit that will be destroyed.
		TheCommandBar.Set_Drag_Object(NULL);
		Debug_Printf("\n== Clearing out drag object\n");

		ReferenceListIterator<GameObjectClass> object_iterator = GAME_OBJECT_MANAGER.Get_Object_Iterator();
		for ( ; !object_iterator.Is_Done(); object_iterator.Next())
		{
			GameObjectClass *object = object_iterator.Current_Object();

			//Skip objects that are already on their way out
			if (object->Is_Dead() || object->Is_Delete_Pending() || object->Is_Marked_For_Death())
			{
				continue;
			}

			const GameObjectTypeClass *object_type = object->Get_Original_Object_Type();
			if (object_type == type)
			{
				object->Destroy();
				Story_Debug_Printf("STORY REWARD - REMOVE OBJECT %s\r\n",RewardParam[0].c_str());
				if (RewardParam[1].empty())
				{
					return;
				}
			}
		}

		// Remove any respawning objects that match
		const DynamicVectorClass<GameObjectClass*> *objs = GAME_OBJECT_MANAGER.Find_Objects(BEHAVIOR_PLANET);
		for (int i = 0; i < objs->Count(); i++)
		{
			PlanetaryDataPackClass *pdata = (*objs)[i]->Get_Planetary_Data();
			PlanetaryBehaviorClass *pbehavior = (PlanetaryBehaviorClass *)(*objs)[i]->Get_Behavior(BEHAVIOR_PLANET);

			if (pdata && pbehavior)
			{
				const PlanetaryDataPackClass::SpawnListType &slist = pdata->Get_Spawn_List();
				PlanetaryDataPackClass::SpawnListType::const_iterator it;
				for (it = slist.begin(); it != slist.end(); it++)
				{
					const PlanetaryDataPackClass::ScheduledObjectSpawnClass &spawn = *it;
					if (spawn.Type == type)
					{
						pbehavior->Remove_Scheduled_Object_Type_Creation(type);

						//Reset the iterator!  Remove_Scheduled_Object_Type_Creation will have modified the list and we could
						//now be pointing to junk.
						it = slist.begin();
						if (RewardParam[1].empty())
						{
							return;
						}
					}
					else if (spawn.Type->Get_Num_Ground_Company_Units() > 0)
					{
						//The respawn type will be the company, while the removal type may well be specified as an individual.  So...
						const GameObjectTypeClass *company_member_type = spawn.Type->Get_Ground_Company_Unit(0);
						if (company_member_type == type)
						{
							pbehavior->Remove_Scheduled_Object_Type_Creation(spawn.Type);

							//Reset the iterator!  Remove_Scheduled_Object_Type_Creation will have modified the list and we could
							//now be pointing to junk.
							it = slist.begin();
							if (RewardParam[1].empty())
							{
								return;
							}
						}
					}
				}
			}
		}
	}
}




/**************************************************************************************************
* StoryEventClass::Reward_Credits -- Give the player credits
*
* In:		
*
* Out:	
*
*
* History: 07/23/2004 2:00PM JAC
**************************************************************************************************/
void StoryEventClass::Reward_Credits()
{
	float credits = (float)atof(RewardParam[0].c_str());
	PlayerClass *local_player = SubPlot->Get_Local_Player();
	assert(local_player);
	Story_Debug_Printf("STORY REWARD - CREDITS %f\r\n",credits);
	local_player->Add_Credits(credits, true);
	TheCommandBar.Blink_Component(COMPONENT_ID_PLAYER_CREDITS, true, false, false); 
	TheCommandBar.Blink_Component(COMPONENT_ID_SPACE_TACTICAL_CREDITS, true, false, false); 
}





/**************************************************************************************************
* StoryEventClass::Reward_Story_Element -- Open up new story threads
*
* In:		
*
* Out:	
*
*
* History: 07/23/2004 2:01PM JAC
**************************************************************************************************/
void StoryEventClass::Reward_Story_Element()
{
	if (RewardParam[0].empty())
	{
		return;
	}

	std::string ext = ".xml";
	std::string story_name = RewardParam[0] + ext;

	Story_Debug_Printf("STORY REWARD - STORY ELEMENTS %s\r\n",story_name.c_str());
	GameModeManager.Get_Active_Mode()->Get_Story_Mode().Activate_Sub_Plot(story_name);
}






/**************************************************************************************************
* StoryEventClass::Reward_Statistic_Change -- Change the statistics of a planet
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:54AM JAC
**************************************************************************************************/
void StoryEventClass::Reward_Statistic_Change(GameObjectClass *planet)
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	if (!RewardParam[2].empty())
	{
		// If a planet is specified in the XML, change stats there.  Otherwise change it on the one passed in
		GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[2].c_str());
		assert(planet_type);
		const DynamicVectorClass<GameObjectClass *> *planet_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);

		if (planet_list->Size() != 0)
		{
			planet = (*planet_list)[0];
		}
	}

	if (planet == NULL)
	{
		Story_Debug_Printf("Reward Statistic Change - No planet specified\r\n");
		return;
	}

	StoryPlanetStats stattype = (StoryPlanetStats)Lookup_Enum(RewardParam[0].c_str());
	PlanetaryDataPackClass *planet_data = planet->Get_Planetary_Data();
	assert(planet_data);
	int newvalue = atoi(RewardParam[1].c_str());

	switch (stattype)
	{
		case STATS_CREDIT_VALUE :
			planet_data->Set_Current_Base_Credit_Value(newvalue);
			if (planet->Get_Owner_Player() != NULL) {
				planet->Get_Owner_Player()->Flag_Income_Has_Changed();
			}
			break;

		case STATS_POLITICAL_CONTROL :
			planet_data->Set_Current_Base_Political_Control_Value(newvalue);
			break;

		case STATS_MAX_SPECIAL_STRUCTS :
			assert(newvalue <= 4);
			planet_data->Set_Current_Max_Special_Structures((unsigned int)newvalue);
			break;

		case STATS_MAX_STARBASE_LEVEL :
			assert(newvalue <= 5);
			planet_data->Set_Current_Max_Starbase_Level((unsigned int)newvalue);
			break;

		case STATS_MAX_GROUND_BASE_LEVEL :
			assert(newvalue <= 5);
			planet_data->Set_Current_Max_Ground_Base_Level((unsigned int)newvalue);
			break;

		default:
			break;
	}
}







/**************************************************************************************************
* StoryEventClass::Reward_New_Power_For_All -- Unlock a new power for multiple objects.
*
* In:		
*
* Out:	
*
*
* History: 01/18/2005 4:28PM AJA
**************************************************************************************************/
void StoryEventClass::Reward_New_Power_For_All()
{
	// Will reward all instances of that object type with the named power for the player that owns the SubPlot.
	// Param0 = object type, Param1 = power name
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	// Get the object type that we'll be rewarding with a new ability.
	const GameObjectTypeClass *object_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (object_type == NULL)
	{
		Story_Debug_Printf("Unable to reward %s with power %s.  Unknown object type.\r\n",RewardParam[0].c_str(),RewardParam[1].c_str());
		return;
	}

	// Unlock that ability for all instances of that object type for the player that owns the SubPlot!
	if ( SubPlot->Get_Local_Player()->Set_Special_Ability_Type_Lock(object_type, RewardParam[1].c_str(), false) == false )
	{
		Story_Debug_Printf("Unable to reward %s with power %s.  It may already have been granted.\r\n",RewardParam[0].c_str(),RewardParam[1].c_str());
	}
}


/**************************************************************************************************
* StoryEventClass::Reward_Remove_Power_From_All -- Revoke a special power from several objects.
*
* In:		
*
* Out:	
*
*
* History: 01/19/2005 11:00AM AJA
**************************************************************************************************/
void StoryEventClass::Reward_Remove_Power_From_All()
{
	// Will deny all instances of that object type from using the named power for the player that owns the SubPlot.
	// Param0 = object type, Param1 = power name
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	// Get the object type that we'll be rewarding with a new ability.
	const GameObjectTypeClass *object_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (object_type == NULL)
	{
		Story_Debug_Printf("Unable to remove power %s from %s.  Unknown object type.\r\n",RewardParam[1].c_str(),RewardParam[0].c_str());
		return;
	}

	// Lock that ability for all instances of that object type for the player that owns the SubPlot!
	if ( SubPlot->Get_Local_Player()->Set_Special_Ability_Type_Lock(object_type, RewardParam[1].c_str(), true) == false )
	{
		Story_Debug_Printf("Unable to remove power %s from %s.  It may already have been revoked.\r\n",RewardParam[1].c_str(),RewardParam[0].c_str());
	}
}



/**************************************************************************************************
* StoryEventClass::Reward_SFX -- Playe back a sound
*
* In:		
*
* Out:	
*
*
* History: 11/23/2004 2:46PM JAC
**************************************************************************************************/
void StoryEventClass::Reward_SFX()
{
	if (RewardParam[0].empty())
	{
		return;
	}

	TheSFXEventManager.Start_SFX_Event(RewardParam[0].c_str());
}






void StoryEventClass::Reward_Speech(const char *speech_name)
{
	if (RewardParam[0].empty() && (speech_name == NULL))
	{
		return;
	}

	const char *name = NULL;
	if (speech_name != NULL)
	{
		name = speech_name;
	}
	else if (!RewardParam[0].empty())
	{
		name = RewardParam[0].c_str();
	}

	TheSpeechConversationManager.System_Stop_Active_Conversation();
	//TheSFXConversationManager.System_Stop_Active_Conversation();
	TheSpeechEventManager.System_Kill_All_Active_Events();
	TheSpeechEventManager.Queue_Speech_Event(name);

	Story_Debug_Printf("STORY REWARD - Speech %s\n",name);
}






void StoryEventClass::Reward_Flash_GUI()
{
	if (RewardParam[0].empty())
	{
		return;
	}

	const char *tag = NULL;
	if (!RewardParam[1].empty())
	{
		tag = RewardParam[1].c_str();
	}

	if (TheTutorial.Is_Dual_Flash())
	{
		TheTutorial.Add_Dual_Flash(RewardParam[0],FLASH_NONE,0);
	}
	else
	{
		TheCommandBar.Flash_Component(RewardParam[0].c_str(),true,FLASH_DURATION,FLASH_COUNT,tag);
	}
}






void StoryEventClass::Reward_Flash_Planet_GUI()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	FlashTarget target = (FlashTarget)Lookup_Enum(RewardParam[1].c_str());
	int fleet_index = -1;
	if (!RewardParam[2].empty())
	{
		fleet_index = atoi(RewardParam[2].c_str());
	}

	const char *tag = NULL;
	if (!RewardParam[3].empty())
	{
		tag = RewardParam[3].c_str();
	}

	bool persistent = false;
	if (!RewardParam[4].empty())
	{
		persistent = true;
	}

	if (TheTutorial.Is_Dual_Flash())
	{
		TheTutorial.Add_Dual_Flash(RewardParam[0],target,fleet_index);
	}
	else
	{
		TheCommandBar.Flash_Planet_Component(RewardParam[0].c_str(),target,fleet_index,tag,true,persistent);
	}
}





void StoryEventClass::Reward_Flash_Planet()
{
	if (RewardParam[0].empty())
	{
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	const DynamicVectorClass<GameObjectClass *> *planet_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);

	if (planet_list->Size() > 0)
	{
		GameObjectClass *planet = (*planet_list)[0];

		planet->Do_GUI_Lighting_Effect(GUI_LIGHT_MULTI_FLASH);
	}
}





void StoryEventClass::Reward_Hide_Tutorial_Cursor()
{
	if (RewardParam[0].empty())
	{
		TheCommandBar.Hide_Tutorial_Cursor();
	}
	else
	{
		TheCommandBar.Hide_Tutorial_Cursor(RewardParam[0].c_str());
	}

	TheTutorial.Stop_Dual_Flash();
}




void StoryEventClass::Reward_Screen_Text()
{
	if (RewardParam[0].empty())
	{
		return;
	}

	float duration = 5;
	const GameObjectTypeClass *object = NULL;

	if (!RewardParam[1].empty())
	{
		duration = (float)atof(RewardParam[1].c_str());
	}

	if (!RewardParam[2].empty())
	{
		object = GameObjectTypeManager.Find_Object_Type(RewardParam[2].c_str());
	}

	// Should text simulate teletype display or reveal immediately?
	bool teletype = true;
	if (!RewardParam[4].empty())
	{
		if (atoi(RewardParam[4].c_str()) == 0)
		{
			teletype = false;
		}
	}

	bool is_subtitle = true;
	// Subtitles will be detected automatically by the color used
	/*
	if (!RewardParam[6].empty())
	{
		if (atoi(RewardParam[6].c_str()) != 0)
		{
			is_subtitle = true;
		}
	}
	*/

	// Let script specify color of text
	RGBAClass color = *TheGameConstants.Get_Message_Text_Color();

	if (!RewardParam[6].empty())
	{
		char color_type[64];
		strcpy(color_type,RewardParam[6].c_str());
		_strupr(color_type);
		if (strcmp(color_type,"HINT") == 0)
		{
			TheSFXEventManager.Start_SFX_Event("GUI_Text_Hint_SFX");
			color = *TheGameConstants.Get_Hint_Text_Color();
			is_subtitle = false;
		}
		else if (strcmp(color_type,"SYSTEM") == 0)
		{
			color = *TheGameConstants.Get_System_Text_Color();
			is_subtitle = false;
		}
		else if (strcmp(color_type,"TASK") == 0)
		{
			color = *TheGameConstants.Get_Task_Text_Color();
			is_subtitle = false;
		}
		else if (strcmp(color_type,"ALIEN_SPEECH") == 0)
		{
			is_subtitle = false;
		}
		else if (strcmp(color_type,"SPEECH") == 0)
		{
			// This is the default color
			// These are considered subtitles
		}
	}

	if (!RewardParam[5].empty())
	{
		char color_str[32];
		strcpy(color_str,RewardParam[5].c_str());
		char *token;

		token = strtok(color_str," ");
		assert(token);
		color.Set_Red((unsigned char)atoi(token));

		token = strtok(NULL," ");
		assert(token);
		color.Set_Green((unsigned char)atoi(token));

		token = strtok(NULL," ");
		assert(token);
		color.Set_Blue((unsigned char)atoi(token));
	}

	bool remove_text = false;
	if (!RewardParam[3].empty())
	{
		// This was set up so that any text in RewardParam[3] would remove the string
		remove_text = true;
	}

	if (remove_text)
	{
		TheCommandBar.Remove_Tutorial_Text(RewardParam[0].c_str());
	}
	else
	{
		TheCommandBar.Add_Tutorial_Text(RewardParam[0].c_str(),duration,object,teletype,&color,is_subtitle);
	}
}





void StoryEventClass::Reward_Disable_Event()
{
	if (RewardParam[0].empty())
	{
		return;
	}

	TutorialEventEnum eventtype = (TutorialEventEnum)StoryEventClass::Lookup_Enum(RewardParam[0].c_str());
	TheTutorial.Disable_Event(eventtype,RewardParam[1],RewardParam[2]);
}






void StoryEventClass::Reward_Enable_Event()
{
	if (RewardParam[0].empty())
	{
		return;
	}

	TutorialEventEnum eventtype = (TutorialEventEnum)StoryEventClass::Lookup_Enum(RewardParam[0].c_str());
	TheTutorial.Enable_Event(eventtype,RewardParam[1],RewardParam[2]);
}






void StoryEventClass::Reward_Pick_Planet()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	StoryEventFilter filter = (StoryEventFilter)StoryEventClass::Lookup_Enum(RewardParam[0].c_str());
	const DynamicVectorClass<GameObjectClass *> *planet_list = GAME_OBJECT_MANAGER.Find_Objects( BEHAVIOR_PLANET );
	int player_id = PlayerList.Get_Local_Player_ID();
	bool found = false;
	bool ally;
	bool enemy;

	for (int i=0; i<planet_list->Size(); i++)
	{
		GameObjectClass *planet = (*planet_list)[i];

		ally = planet->Is_Ally(player_id);
		enemy = planet->Is_Enemy(player_id);

		switch (filter)
		{
			case EVENT_FILTER_NONE:
				found = true;
				break;

			case EVENT_FILTER_FRIENDLY_ONLY:
				if (ally)
				{
					found = true;
				}
				break;

			case EVENT_FILTER_NEUTRAL_ONLY:
				if (!ally && !enemy)
				{
					found = true;
				}
				break;

			case EVENT_FILTER_ENEMY_ONLY:
				if (enemy)
				{
					found = true;
				}
				break;

			case EVENT_FILTER_FRIENDLY_AND_NEUTRAL:
				if (!enemy)
				{
					found = true;
				}
				break;

			case EVENT_FILTER_ENEMY_AND_NEUTRAL:
				if (!ally)
				{
					found = true;
				}
				break;

			case EVENT_FILTER_FRIENDLY_AND_ENEMY:
				if (ally || enemy)
				{
					found = true;
				}
				break;

			default:
				break;
		}

		if (found)
		{
			char name[ 256 ];
			strcpy( name, RewardParam[1].c_str() );
			_strupr( name );
			std::string planet_name(name);
			
			Story_Debug_Printf("Replacing all instances of %s with planet %s\r\n",planet_name.c_str(),planet->Get_Type()->Get_Name()->c_str());
			GameModeManager.Get_Active_Mode()->Get_Story_Mode().Replace_Variable(planet_name,*planet->Get_Type()->Get_Name());
			return;
		}
	}
}









void StoryEventClass::Reward_Switch_Sides()
{
	if (!GameModeManager.Is_Multiplayer_Mode())
	{
		if (RewardParam[0].empty())
		{
			PlayerList.Switch_Sides();
		}
		else
		{
			// Cycle through all the factions until the specified one is found or we've come full circle
			PlayerClass *player = PlayerList.Get_Local_Player();
			const FactionClass *first_faction = player->Get_Faction();
			const FactionClass *cur_faction = player->Get_Faction();
			do
			{
				PlayerList.Switch_Sides();
				player = PlayerList.Get_Local_Player();
				cur_faction = player->Get_Faction();
			}
			while ((first_faction != cur_faction) && (cur_faction != FactionList.Find_Faction_By_Name(RewardParam[0].c_str())));
		}

		int modes = GameModeManager.Get_Game_Mode_Count();
		for (int i=0 ; i<modes ; i++) {
			GameModeManager.Get_Game_Mode_By_Index(i)->Get_Object_Manager().Refresh_Visibility();
		}
	}
}








void StoryEventClass::Reward_Zoom_In()
{
	Story_Debug_Printf("STORY REWARD - Zoom in on planet\r\n");
	TheCommandBar.Test_Galactic_Camera_Zoom_In();
}








void StoryEventClass::Reward_Zoom_Out()
{
	Story_Debug_Printf("STORY REWARD - Zoom out of planet\r\n");
	TheCommandBar.Test_Galactic_Camera_Zoom_Out();
}






void StoryEventClass::Reward_Pause_Game()
{
	#if 1
	Story_Debug_Printf("STORY REWARD - Pausing game\r\n");

	// The command bar already has a pointer to the options dialog so go through that
	TheCommandBar.Start_Options();
	#else
	// MBL 08.31.2004 Pause the current game
	GameModeClass *game_mode = GameModeManager.Get_Active_Mode();
	if ( game_mode != NULL )
	{
		GameModeClass::GameModeType game_mode_type = game_mode->Get_Type();
		if ( game_mode_type == GameModeClass::SOLO )
		{
			// game_mode->Pause_Game();
			if( FrameSynchronizer.Is_Single_Step_Mode_Enabled() == false )
			{
				FrameSynchronizer.Enable_Single_Step_Mode( true );
			}
		}
	}
	#endif
}






void StoryEventClass::Reward_Tutorial_Dialog_Box()
{
	int quit_only = 0;

	if (!RewardParam[1].empty())
	{
		quit_only = atoi(RewardParam[1].c_str());
	}

#ifdef PUBLIC_DEMO
	// Force a quit at the end of tutorial 5
	GameModeClass *mode = GameModeManager.Get_Active_Mode();
	if (mode->Get_Sub_Type() != SUB_GAME_MODE_GALACTIC) {
		mode = GameModeManager.Get_Parent_Game_Mode(mode);
		assert(mode->Get_Sub_Type() == SUB_GAME_MODE_GALACTIC);
	}

	GalacticModeClass *galactic_mode = static_cast<GalacticModeClass*>(mode);
	const std::string &campaign_name = galactic_mode->Get_Campaign_Name();
	if (stricmp(campaign_name.c_str(), "Tutorial_Five") == 0) {
		TheTutorial.Display_Dialog_Box_Text(RewardParam[0].c_str(), true);
		return;
	}
#endif PUBLIC_DEMO

	TheTutorial.Display_Dialog_Box_Text(RewardParam[0].c_str(),(quit_only != 0));
}






void StoryEventClass::Reward_Position_Camera()
{
	Vector3 pos(0,0,0);

	if (RewardParam[0].empty())
	{
		pos = RewardPosition;
	}
	else
	{
		const GameObjectTypeClass *object_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
		const DynamicVectorClass<GameObjectClass *> *object_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(object_type);

		if (object_list->Size() == 0)
		{
			Story_Debug_Printf("STORY REWARD - Unable to find object %s\r\n",object_type->Get_Name());
		}
		else
		{
			GameObjectClass *object = (*object_list)[0];
			object->Get_Model()->Get_Transform().Get_Translation(&pos);
		}
	}

	if (!RewardParam[1].empty())
	{
		char *param1 = new char[RewardParam[1].length() + 1];
		strcpy(param1,RewardParam[1].c_str());
		char* value_str;
		Vector3 offset(0,0,0);

		value_str = strtok(param1,",");
		offset.x = (float) atof(value_str);

		value_str = strtok(NULL,",");
		offset.y = (float) atof(value_str);

		value_str = strtok(NULL,",");
		offset.z = (float) atof(value_str);

		delete [] param1;
		pos += offset;
	}

	Story_Debug_Printf("STORY REWARD - Positioning camera to (%f,%f,%f)\r\n",pos.x,pos.y,pos.z);

	TheTutorial.Position_Camera(pos);
}







void StoryEventClass::Reward_Scroll_Camera()
{
	Vector3 pos(0,0,0);

	if (RewardParam[0].empty())
	{
		pos = RewardPosition;
	}
	else
	{
		const GameObjectTypeClass *object_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
		const DynamicVectorClass<GameObjectClass *> *object_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(object_type);

		if (object_list->Size() == 0)
		{
			Story_Debug_Printf("STORY REWARD - Unable to find object %s\r\n",object_type->Get_Name());
		}
		else
		{
			GameObjectClass *object = (*object_list)[0];
			object->Get_Model()->Get_Transform().Get_Translation(&pos);
		}
	}

	Story_Debug_Printf("STORY REWARD - Positioning camera to (%f,%f,%f)\r\n",pos.x,pos.y,pos.z);
	TheTutorial.Scroll_Camera(pos);
}






void StoryEventClass::Reward_Lock_Controls()
{
	if (!RewardParam[0].empty())
	{
		int lock = atoi(RewardParam[0].c_str());
		if (lock != 0)
		{
			TheTutorial.Lock_Controls(true);
		}
		else
		{
			TheTutorial.Lock_Controls(false);
		}
	}
}






void StoryEventClass::Reward_Flash_Object()
{
	Story_Debug_Printf("STORY REWARD - FLASH OBJECT\r\n");

	if (!RewardParam[0].empty())
	{
		const GameObjectTypeClass *object_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
		const DynamicVectorClass<GameObjectClass *> *object_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(object_type);

		if (object_list->Size() == 0)
		{
			Story_Debug_Printf("STORY REWARD FLASH OBJECT- Unable to find object %s\r\n",object_type->Get_Name());
		}
		else
		{
			GameObjectClass *object = (*object_list)[0];
			object->Do_GUI_Lighting_Effect(GUI_LIGHT_MULTI_FLASH);
		}
	}
}





void StoryEventClass::Reward_Toggle_FOW()
{
	if (RewardParam[0].empty())
	{
		GameModeManager.Get_Active_Mode()->Enable_Fog_Of_War(false);
	}
	else
	{
		int lock = atoi(RewardParam[0].c_str());
		if (lock != 0)
		{
			GameModeManager.Get_Active_Mode()->Enable_Fog_Of_War(true);
			alGraphicsDriver::Enable_Fog_Of_War_Rendering( true );
		}
		else
		{
			GameModeManager.Get_Active_Mode()->Enable_Fog_Of_War(false);
			alGraphicsDriver::Enable_Fog_Of_War_Rendering( false );

			// Cinematics already turn off FOW.  At the end it will turn it back on.  Clear out its variable so it doesn't turn it back on
			if (CinematicsManager.Is_Cinematic_Playing())
			{
				CinematicClass *cinematic = CinematicsManager.Get_Cinematic_Player();
				cinematic->Set_FOW_Disabled(false);
			}
		}
	}
}






void StoryEventClass::Reward_Enable_Victory()
{
	if (!RewardParam[0].empty())
	{
		VictoryMonitorClass *victory_monitor = GameModeManager.Get_Active_Mode()->Get_Victory_Monitor();

		int lock = atoi(RewardParam[0].c_str());
		if (lock != 0)
		{
			// Switching over to the delayed victory system Mike added
			if (victory_monitor->Get_Wait_Count() > 0)
			{
				victory_monitor->Remove_Wait();
			}
		}
		else
		{
			// Switching over to the delayed victory system Mike added
			victory_monitor->Add_Wait();
		}
	}
}







void StoryEventClass::Reward_Move_Fleet()
{
	if (RewardParam[0].empty() || RewardParam[1].empty() || RewardParam[2].empty())
	{
		return;
	}

	// Find the source planet
	const GameObjectTypeClass *object_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	const DynamicVectorClass<GameObjectClass *> *object_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(object_type);
	GameObjectClass *planet = NULL;

	if (object_list->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD FLASH OBJECT- Unable to find planet %s\r\n",object_type->Get_Name());
	}
	else
	{
		planet = (*object_list)[0];
	}

	// Find the destination planet
	object_type = GameObjectTypeManager.Find_Object_Type(RewardParam[1].c_str());
	object_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(object_type);
	GameObjectClass *dest_planet = NULL;

	if (object_list->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD FLASH OBJECT- Unable to find planet %s\r\n",object_type->Get_Name());
	}
	else
	{
		dest_planet = (*object_list)[0];
	}

	// If there isn't a source or dest planet, we can't move a fleet
	if ((planet == NULL) || (dest_planet == NULL))
	{
		return;
	}

	// Get all the fleets around a planet and move the ones that the story specifies (friendly/enemy)
	DynamicVectorClass<GameObjectClass *> select;
	StoryEventFilter filter = (StoryEventFilter)Lookup_Enum(RewardParam[2].c_str());
	DynamicVectorClass<GameObjectClass *> *fleet_list = planet->Get_Attached_Fleets();

	for (int i=0; i<fleet_list->Size(); i++)
	{
		GameObjectClass *fleet = (*fleet_list)[i];
		bool move = false;

		if (fleet->Is_Ally(SubPlot->Get_Local_Player()))
		{
			if (filter == EVENT_FILTER_FRIENDLY_ONLY)
			{
				move = true;
			}
		}
		else
		{
			if (filter == EVENT_FILTER_ENEMY_ONLY)
			{
				move = true;
			}
		}

		if (move)
		{
			select.Add(fleet);
		}
	}

	PlayerClass *player = SubPlot->Get_Local_Player();
	if (filter == EVENT_FILTER_ENEMY_ONLY)
	{
		player = player->Get_Enemy();
	}

	if (select.Size() == 0)
	{
		return;
	}

	GameObjectClass *first_fleet = select[0];

	if (select.Size() > 1)
	{
		// Merge all the fleets into a single fleet
		for (int i=1; i<select.Size(); i++)
		{
			GameObjectClass *fleet = select[i];
			FleetManagementEventClass *event = new FleetManagementEventClass;
			event->Init(planet->Get_ID(), first_fleet->Get_ID(), fleet->Get_ID(), FleetManagementEventClass::ACTION_MERGE);
			OutgoingEventQueue.Add(event);
		}

		// Best way to do this???
		select.Resize(0);
		select.Add(first_fleet);
	}

	// Select the fleets
	SelectEventClass *event = new SelectEventClass;
	event->Init(&select);
	OutgoingEventQueue.Add(event);

	// Create a path to the destination
	GalacticPathClass path;
	GalacticPathFindCallPropertiesStruct path_find_config;
	path_find_config.Player = player;
	path_find_config.StartPlanet = planet;
	path_find_config.EndPlanet = dest_planet;

	const FactionClass *faction = path_find_config.Player->Get_Faction();

	path_find_config.IsNeutralPassable = faction->Is_Neutral() || first_fleet->Skips_Space_Combat();
	path_find_config.IsHostilePassable = first_fleet->Skips_Space_Combat();
	path_find_config.AllowBadFactionIfStartEnd = true;
	path_find_config.IsDefendedPassable = first_fleet->Skips_Space_Combat();

	bool success = GalacticPathFinderClass::Get().Find_Path( path_find_config, path );

	if (success)
	{
		// Move the fleet along the determined path
		MoveThroughObjectsEventClass *event = new MoveThroughObjectsEventClass;
		assert(event != NULL );
		event->Init( path, first_fleet );
		OutgoingEventQueue.Add(event);

		Story_Debug_Printf("STORY REWARD - Moving fleet from %s to %s\r\n",planet->Get_Type()->Get_Name()->c_str(),dest_planet->Get_Type()->Get_Name()->c_str());
	}
}






void StoryEventClass::Reward_Trigger_AI()
{
	Story_Debug_Printf("STORY REWARD - Triggering AI\r\n");

	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	const FactionClass *faction = TheFactionTypeConverterPtr->Get_Managed_Object(RewardParam[1]);
	if (!faction)
	{
		Story_Debug_Printf("STORY REWARD - Triggering AI: faction %s not found, trigger failed\r\n", RewardParam[1].c_str());
		return;
	}

	PlayerClass *player = PlayerList.Get_Player_Of_Faction(faction);
	if (!player)
	{
		Story_Debug_Printf("STORY REWARD - Triggering AI: cannot find player of faction %s, trigger failed\r\n", RewardParam[1].c_str());
		return;
	}

	AIPlayerClass *ai_player = player->Get_AI_Player();
	if (!ai_player)
	{
		Story_Debug_Printf("STORY REWARD - Triggering AI: player of faction %s has no AI, trigger failed\r\n", RewardParam[1].c_str());
		return;
	}

	GameObjectClass *target = NULL;

	//Optional third parameter is planet target for AI behavior
	if (!RewardParam[2].empty())
	{
		GameObjectTypeClass *object_type = GameObjectTypeManager.Find_Object_Type(RewardParam[2].c_str());
		const DynamicVectorClass<GameObjectClass *> *object_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(object_type);

		if (object_list->Size() == 0)
		{
			Story_Debug_Printf("STORY REWARD TRIGGER AI- Unable to find planet %s\r\n", object_type->Get_Name());
		}
		else
		{
			target = (*object_list)[0];
		}
	}

	ai_player->Set_AI_Story_Arc_Trigger(RewardParam[0], target, true);
}







void StoryEventClass::Reward_Disable_Story_Event()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		Story_Debug_Printf("STORY REWARD DISABLE STORY EVENT- Missing parameter, ignoring reward"); // added so it wouldn't quietly fail. Eric_Y 6/7/5
		return;
	}

	Story_Debug_Printf("STORY REWARD - Disable story event %s\r\n",RewardParam[0].c_str());

	bool onoff = false;

	if (atoi(RewardParam[1].c_str()) != 0)
	{
		onoff = true;
	}

	bool global_disable = false;

	if (!RewardParam[2].empty() && atoi(RewardParam[2].c_str()) != 0)
	{
		global_disable = true;
	}

	if (global_disable)
	{
		// Disable all events with this name in any plot
		SubPlot->Get_Story_Mode()->Disable_Event(RewardParam[0].c_str(),onoff);
	}
	else
	{
		// Disable event with this name in this plot
		StoryEventClass *event = SubPlot->Get_Event(RewardParam[0].c_str());
		if (event)
		{
			event->Disable_Event(onoff);
		}
	}
}






void StoryEventClass::Reward_Disable_Branch()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		Story_Debug_Printf("STORY REWARD DISABLE BRANCH- Missing parameter, ignoring reward\r\n"); // added so it wouldn't quietly fail. Eric_Y 6/7/5
		return;
	}

	Story_Debug_Printf("STORY REWARD - Disable story event %s\r\n",RewardParam[0].c_str());

	bool onoff = false;

	if (atoi(RewardParam[1].c_str()) != 0)
	{
		onoff = true;
	}

	SubPlot->Disable_Branch(RewardParam[0].c_str(),onoff);
}





void StoryEventClass::Reward_Invade_Planet()
{
	if (RewardParam[0].empty())
	{
		return;
	}

	Story_Debug_Printf("STORY REWARD - Invade planet %s\r\n",RewardParam[0].c_str());

	// Find the source planet
	const GameObjectTypeClass *object_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	const DynamicVectorClass<GameObjectClass *> *object_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(object_type);
	GameObjectClass *planet = NULL;

	if (object_list->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD FLASH OBJECT- Unable to find planet %s\r\n",object_type->Get_Name());
	}
	else
	{
		planet = (*object_list)[0];
	}

	// Get script defined filter
	// For now forcing it to only allow forcing of player invasion since that's what the
	// tutorial calls for.  Also, it doesn't seem to work forcing enemy to invade.
	StoryEventFilter filter = EVENT_FILTER_FRIENDLY_ONLY;
	//StoryEventFilter filter = (StoryEventFilter)Lookup_Enum(RewardParam[1].c_str());

	// Get all the fleets around a planet
	DynamicVectorClass<GameObjectClass *> *fleet_list = planet->Get_Attached_Fleets();

	// The planet shouldn't match the side specified in the filter
	if (planet->Is_Ally(SubPlot->Get_Local_Player()))
	{
		if (filter == EVENT_FILTER_FRIENDLY_ONLY)
		{
			return;
		}
	}
	else
	{
		if (filter == EVENT_FILTER_ENEMY_ONLY)
		{
			return;
		}
	}

	// Find a suitable invasion fleet
	for (int i=0; i<fleet_list->Size(); i++)
	{
		GameObjectClass *fleet = (*fleet_list)[i];
		bool match = false;

		// See if this fleet matches the filter specified in the script
		if (fleet->Is_Ally(SubPlot->Get_Local_Player()))
		{
			if (filter == EVENT_FILTER_FRIENDLY_ONLY)
			{
				match = true;
			}
		}
		else
		{
			if (filter == EVENT_FILTER_ENEMY_ONLY)
			{
				match = true;
			}
		}

		// If the fleet matches the filter, see if the fleet can invade
		if (match)
		{
			DynamicVectorClass<GameObjectClass*> fleet_objects;
			fleet->Get_Fleet_Breakdown(fleet_objects,false);

			// Loop through all the fleet contents until we find a transport that can invade
			for (int i=0; i<fleet_objects.Size(); i++)
			{
				GameObjectClass *transport = fleet_objects[i];
				if (transport->Behaves_Like(BEHAVIOR_TRANSPORT))
				{
					// The script can specify a unit type to invade with
					TransportBehaviorClass *tbehavior = static_cast<TransportBehaviorClass *> (transport->Get_Behavior(BEHAVIOR_TRANSPORT));
					const GameObjectTypeClass *unit_type = tbehavior->Get_Dummy_Company_Object_Type(transport);

					// If no unit type specified or we have the right unit type, see if the unit can invade
					if (RewardParam[1].empty() || (RewardParam[1] == *unit_type->Get_Name()))
					{
						// Only send down this unit if it can fight
						ICombatantBehaviorPtr ic = transport;
						if (ic != NULL && (ic->Can_Participate_In_Land_Combat() == true))
						{
							DynamicVectorClass<GameObjectClass*> invasion;
							invasion.Add(transport);

							InvadeEventClass *event = new InvadeEventClass;
							event->Init(&invasion, planet);
							OutgoingEventQueue.Add(event);

							// Only one event needed for invasion
							return;
						}
					}
					else if (!RewardParam[1].empty())
					{
						Story_Debug_Printf("INVADE PLANET - %s does not match %s\r\n",unit_type->Get_Name()->c_str(),RewardParam[1].c_str());
					}
				}
			}
		}
	}
}







void StoryEventClass::Reward_Load_Campaign()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		Story_Debug_Printf("STORY REWARD - Requires two parameters: campaign name & rebel/empire index (0 or 1)");
		return;
	}

	int index = atoi(RewardParam[1].c_str());

	char name_string[ 128 ];
	assert( RewardParam[0].size() < sizeof( name_string ) );
	strcpy( name_string, RewardParam[0].c_str() );
	_strupr( name_string );

	unsigned int name_crc = CRCClass::Calculate_CRC( name_string, strlen( name_string ) );

	Story_Debug_Printf("STORY REWARD - Loading campaign %s, side %s\r\n",name_string,index ? "EMPIRE" : "REBEL");
	CampaignDataClass *campaign_set = CampaignDataManager.Find_Campaign_Data(name_crc);
	CampaignDataClass *campaign = NULL;

	if (campaign_set)
	{
		campaign = CampaignDataManager.Find_Campaign_Data_Of_Set_And_Faction_Index(campaign_set->Get_Campaign_Set_Name_CRC(),index);
	}

	if (campaign == NULL)
	{
		Story_Debug_Printf("STORY REWARD - Unable to find campaign data\r\n");
	}
	else
	{
		//TheTutorial.Quit_Game();
		//GameModeManager.Start_Campaign( TheTutorial.Get_Font(), campaign, GameModeClass::SOLO );

		// Queue up the campaign.  Loading it now will cause problems since we're in the middle of evaluating an event.
		TheTutorial.Queue_Next_Campaign(campaign);
	}
}





void StoryEventClass::Reward_Flash_Terrain()
{
	if (RewardParam[0].empty())
	{
		return;
	}

	Vector3 pos = RewardPosition;
	float duration = (float)atof(RewardParam[0].c_str());

	Story_Debug_Printf("STORY REWARD - Flash terrain position (%f,%f,%f), duration %f\r\n",pos.x,pos.y,pos.z,duration);

	TheCommandBar.Add_Radar_Blip(RewardPosition,duration,-1,true,NULL);
}






void StoryEventClass::Reward_Set_Health()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	int int_percent = atoi(RewardParam[1].c_str());
	float percent = (float)int_percent / 100.0f;
	int count = 1;

	if (!RewardParam[2].empty())
	{
		count = atoi(RewardParam[2].c_str());
	}

	char name_string[ 128 ];
	assert( RewardParam[0].size() < sizeof( name_string ) );
	strcpy( name_string, RewardParam[0].c_str() );
	_strupr( name_string );

	Story_Debug_Printf("STORY REWARD - Set health of %s, to %d percent\r\n",name_string,int_percent);

	const GameObjectTypeClass *object_type = GameObjectTypeManager.Find_Object_Type(name_string);
	if (object_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - Error!  Can't find type %s\r\n",name_string);
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *object_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(object_type);

	if (count == 0)
	{
		count = object_list->Size();
	}

	for (int i=0; i<object_list->Size(); i++)
	{
		GameObjectClass *object = (*object_list)[i];
		object->Set_Health_Percent(percent);

		count--;
		if (count <= 0)
		{
			return;
		}
	}
}







void StoryEventClass::Reward_Set_Tactical_Map()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	StoryBaseFilter location = (StoryBaseFilter)Lookup_Enum(RewardParam[1].c_str());

	if (location == BASE_GROUND)
	{
		TheTutorial.Set_Land_Map(&RewardParam[0]);
		Story_Debug_Printf("STORY REWARD - SET LAND TACTICAL MAP %s\r\n",RewardParam[0].c_str());
	}
	else if (location == BASE_SPACE)
	{
		TheTutorial.Set_Space_Map(&RewardParam[0]);
		Story_Debug_Printf("STORY REWARD - SET SPACE TACTICAL MAP %s\r\n",RewardParam[0].c_str());
	}
	else
	{
		Story_Debug_Printf("STORY REWARD - SET TACTICAL MAP %s failed due to invalid location (GROUND/SPACE)\r\n",RewardParam[0].c_str());
	}

	GameModeClass::MapOwnerType player_type = GameModeClass::MAP_OWNER_INVALID;

	if (!RewardParam[2].empty())
	{
		if ( _stricmp( "Rebel", RewardParam[2].c_str() ) == 0 ) 
		{
			player_type = GameModeClass::MAP_OWNER_REBEL;
		}
		else if ( _stricmp( "Empire", RewardParam[2].c_str() ) == 0 )
		{
			player_type = GameModeClass::MAP_OWNER_EMPIRE;
		}
		else if ( _stricmp( "Pirates", RewardParam[2].c_str() ) == 0 )
		{
			player_type = GameModeClass::MAP_OWNER_PIRATE;
		}
		else if ( _stricmp( "Underworld", RewardParam[2].c_str() ) == 0 )
		{
			player_type = GameModeClass::MAP_OWNER_UNDERWORLD;
		}
		else if ( _stricmp( "Hutts", RewardParam[2].c_str() ) == 0 )
		{
			player_type = GameModeClass::MAP_OWNER_HUTTS;
		}
	}

	TheTutorial.Set_Override_Map_Owner(player_type);
}







void StoryEventClass::Reward_Disable_Autoresolve()
{
	Story_Debug_Printf("STORY REWARD - DISABLE AUTORESOLVE in no longer working.  This should not be used.\r\n");
	//TheTutorial.Set_Disable_Autoresolve(true);
}






void StoryEventClass::Reward_Enable_Autoresolve()
{
	Story_Debug_Printf("STORY REWARD - ENABLE AUTORESOLVE in no longer working.  This should not be used.\r\n");
	//TheTutorial.Set_Disable_Autoresolve(false);
}






void StoryEventClass::Reward_Disable_Movies()
{
	Story_Debug_Printf("STORY REWARD - DISABLE MOVIES\r\n");

	bool onoff = false;

	if (atoi(RewardParam[0].c_str()) != 0)
	{
		onoff = true;
	}

	TheTutorial.Set_Disable_Movies(onoff);
}







void StoryEventClass::Reward_Remove_Story_Goal()
{
	if (!RewardParam[0].empty())
	{
		StoryDialogManagerClass::Remove_Story_Dialog_Goal(&RewardParam[0]);
		Story_Debug_Printf("STORY REWARD - Removing story goal %s\r\n",RewardParam[0].c_str());
	}
}





void StoryEventClass::Reward_Change_Owner()
{
	if (!RewardParam[0].empty() && !RewardParam[1].empty())
	{
		const FactionClass *faction = FactionList.Find_Faction_By_Name(RewardParam[1]);
		if (faction)
		{
			PlayerClass *new_owner = PlayerList.Get_Player_Of_Faction(faction);
			if (new_owner)
			{
				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
				const DynamicVectorClass<GameObjectClass *> *target_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);
				int max_change = min(1,target_list->Size());
	
				if (!RewardParam[2].empty())
				{
					int desired_change = atoi(RewardParam[2].c_str());
					if (desired_change == 0)
					{
						// A value of 0 means change sides of all units found
						max_change = target_list->Size();
					}
					else
					{
						// Change a specific number of units (if that many exist)
						max_change = min(target_list->Size(),desired_change);
					}
				}
	
				for (int i=0; i<max_change; i++)
				{
					GameObjectClass *object = (*target_list)[i];

					if (object->Behaves_Like(BEHAVIOR_PLANET))
					{
						PlanetaryBehaviorClass *planet_behave = static_cast<PlanetaryBehaviorClass*>(object->Get_Behavior(BEHAVIOR_PLANET));
						ENFORCED_IF(planet_behave)
						{
							planet_behave->Obliterate_All_Occupied_Player_Presence(object, object->Get_Owner_Player());
							planet_behave->Force_Transition_Ownership(object, new_owner->Get_ID());
						}
					}
					else
					{
						object->Change_Owner(new_owner->Get_ID());
					}
					Story_Debug_Printf("STORY REWARD - Changing owner of unit %s to %s\r\n",RewardParam[0].c_str(),RewardParam[1].c_str());
				}
			}
		}
	}
}






void StoryEventClass::Reward_Destroy_Object()
{
	if (!RewardParam[0].empty())
	{
		const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
		const DynamicVectorClass<GameObjectClass *> *target_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);
		if (target_list->Size())
		{
			GameObjectClass *object = (*target_list)[0];
			if (object)
			{
				object->Take_Damage(DAMAGE_MISC, INSTANT_KILL_DAMAGE_INFLICTION, NULL, NULL, NULL);
				Story_Debug_Printf("STORY REWARD - Destroying object %s\r\n",RewardParam[0].c_str());
			}
		}
		else
		{
			Story_Debug_Printf("STORY REWARD - Unable to find object %s to destroy\r\n",RewardParam[0].c_str());
		}
	}
}





void StoryEventClass::Reward_Victory()
{

	if (!RewardParam[0].empty())
	{
		const FactionClass *faction = FactionList.Find_Faction_By_Name(RewardParam[0]);
		if (faction)
		{
			PlayerClass *player = PlayerList.Get_Player_Of_Faction(faction);
			if (player)
			{
				VictoryConditionClass victory_condition( VICTORY_SUB_TACTICAL_STORY_MODE_TRIGGERED_VICTORY, player->Get_ID(), FrameSynchronizer.Get_Logical_FPS()*7 );

				// If Param2 is set to true, don't show the "You Are Victorious" TExt and VO.
				if (!RewardParam[1].empty())
				{
					int value = atoi(RewardParam[1].c_str());
					if (value != 0)
					{
						victory_condition.Set_Suppress_Victory_Text(true);
						victory_condition.Set_Victory_Delay_Frames(FrameSynchronizer.Get_Logical_FPS()*1);
					}
				}

				// Remove all waits on the victory monitor so we can win, but make sure we don't check for vicotry when removing waits or wrong side may win
				GameModeManager.Get_Active_Mode()->Get_Victory_Monitor()->Remove_All_Waits(false);
				// Don't know if it's proper to force a victory this way.  We'll find out!
				GameModeManager.Get_Active_Mode()->Get_Victory_Monitor()->Add_Victory_Condition(victory_condition);
				//GameModeManager.Get_Active_Mode()->Get_Victory_Monitor()->Register_Pending_Victory(victory_condition);
				GameModeManager.Test_For_Victory_Conditions( VICTORY_SUB_TACTICAL_STORY_MODE_TRIGGERED_VICTORY, player->Get_ID() );

				Story_Debug_Printf("STORY REWARD - Victory for faction %s\r\n",RewardParam[0].c_str());
			}
			else
			{
				Story_Debug_Printf("STORY REWARD - WARNING! Unable to find player of faction %s to force victory\r\n",RewardParam[0].c_str());
			}
		}
		else
		{
			Story_Debug_Printf("STORY REWARD - WARNING! Unable to find faction %s to force victory\r\n",RewardParam[0].c_str());
		}
	}
}





void StoryEventClass::Reward_Switch_Control()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	//char *faction_name = RewardParam[0].c_str();

	const FactionClass *faction = FactionList.Find_Faction_By_Name( RewardParam[0] );
	if ( !faction )
	{
		return;
	}

	PlayerClass *player = PlayerList.Get_Player_Of_Faction( faction );
	if ( !player )
	{
		return;
	}

	//char *ai_player_name = RewardParam[1].c_str();

	player->Set_AI_Control( RewardParam[1] );

	if (player->Get_AI_Player() && !player->Is_Human())
	{
		//Give the player the default difficulty level
		player->Get_AI_Player()->Set_Difficulty(GameModeManager.Get_Difficulty_Level());
	}
}






void StoryEventClass::Reward_Flash_Unit()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	CommandBarRegionEnum region = (CommandBarRegionEnum)Lookup_Enum(RewardParam[0].c_str());
	if (region == REGION_NONE)
	{
		Story_Debug_Printf("STORY REWARD FLASH UNIT - Warning!  Can't find region %s\r\n",RewardParam[0].c_str());
	}
	else
	{
		const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(RewardParam[1].c_str());
		if (type == NULL)
		{
			Story_Debug_Printf("STORY REWARD FLASH UNIT - Warning!  Can't find unit type %s\r\n",RewardParam[1].c_str());
		}
		else
		{
			Story_Debug_Printf("STORY REWARD - Flashing unit %s, region %s\r\n",RewardParam[1].c_str(),RewardParam[0].c_str());
			TheCommandBar.Flash_Unit(region,type);
		}
	}

}






void StoryEventClass::Reward_Disable_Retreat()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	const FactionClass *faction = FactionList.Find_Faction_By_Name( RewardParam[0] );
	if ( !faction )
	{
		Story_Debug_Printf("STORY REWARD - Warning!  Can't find faction of %s\r\n",RewardParam[0].c_str());
		return;
	}

	int value = atoi(RewardParam[1].c_str());
	if (value == 0)
	{
		TheTutorial.Disable_Retreat(faction,false);
	}
	else
	{
		TheTutorial.Disable_Retreat(faction,true);
	}

}








void StoryEventClass::Reward_Reveal_All_Planets()
{
	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_Objects( BEHAVIOR_PLANET );

	for (int i=0 ; i<planets->Get_Count() ; i++)
	{
		GameObjectClass *planet = planets->Get_At(i);
		PlanetaryDataPackClass *planet_data = planet->Get_Planetary_Data();
		if (planet_data)
		{
			planet_data->Set_Is_Locally_Revealed(true);
			planet->Refresh_Visibility(PlayerList.Get_Local_Player());
		}
	}
}






void StoryEventClass::Reward_Dual_Flash()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	float total_time = (float)atof(RewardParam[0].c_str());
	float flash_rate = (float)atof(RewardParam[1].c_str());

	Story_Debug_Printf("STORY REWARD - Starting a dual flash. Time %f, rate %f\r\n",total_time,flash_rate);
	TheTutorial.Start_Dual_Flash(total_time,flash_rate);
}








void StoryEventClass::Reward_Set_Flag()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	//CRCValue crc = CRCClass::Calculate_CRC( RewardParam[0].c_str(), sizeof(RewardParam[0].c_str()) );
	int value = atoi(RewardParam[1].c_str());

	StoryModeClass::Set_Flag(RewardParam[0].c_str(),value);
}








void StoryEventClass::Reward_Increment_Flag()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	//CRCValue crc = CRCClass::Calculate_CRC( RewardParam[0].c_str(), sizeof(RewardParam[0].c_str()) );
	int value = atoi(RewardParam[1].c_str());

	int new_value = StoryModeClass::Increment_Flag(RewardParam[0].c_str(),value);

	if (new_value == UNDEFINED_STORY_FLAG)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Attempting to increment undefined flag %s\r\n",RewardParam[0].c_str());
		assert(0);
	}
}




void StoryEventClass::Reward_Hilite_Object()
{
	if (!RewardParam[0].empty() && !RewardParam[1].empty())
	{
		const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
		const DynamicVectorClass<GameObjectClass *> *target_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);
		if (target_list->Size())
		{
			GameObjectClass *object = (*target_list)[0];
			if (object)
			{
				bool hilite_radar = false;
				int radar_id = -1;
				bool hilite = true;
				if (atoi(RewardParam[1].c_str()) == 0)
				{
					hilite = false;
				}

				if (!RewardParam[2].empty())
				{
					radar_id = atoi(RewardParam[2].c_str());
				}

				if (radar_id != -1)
				{
					hilite_radar = true;
				}

				if (hilite)
				{
					Story_Debug_Printf("STORY REWARD - Highlighting object %s\r\n",object->Get_Type()->Get_Name()->c_str());
					object->Highlight();
					if (hilite_radar)
					{
						Vector3 pos = object->Get_Position();
						TheCommandBar.Add_Radar_Blip(pos,-1,radar_id,false,object);
					}
				}
				else
				{
					Story_Debug_Printf("STORY REWARD - Unhighlighting object %s\r\n",object->Get_Type()->Get_Name()->c_str());
					object->Un_Highlight();
					if (hilite_radar)
					{
						TheCommandBar.Remove_Radar_Blip(radar_id);
					}
				}
			}
		}
	}
}







void StoryEventClass::Reward_Story_Goal_Completed()
{
	if (!RewardParam[0].empty())
	{
		// Changing so completed goals are removed
		//StoryDialogManagerClass::Remove_Story_Dialog_Goal(&RewardParam[0]);
		StoryDialogManagerClass::Story_Dialog_Goal_Completed(&RewardParam[0]);
		Story_Debug_Printf("STORY REWARD - Removing story goal %s\r\n",RewardParam[0].c_str());
	}
}









void StoryEventClass::Reward_Reveal_Planet()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	bool reveal = true;
	if (atoi(RewardParam[1].c_str()) == 0)
	{
		reveal = false;
	}

	const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (type)
	{
		const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);

		for (int i=0 ; i<planets->Get_Count() ; i++)
		{
			GameObjectClass *planet = planets->Get_At(i);
			if (planet && planet->Behaves_Like(BEHAVIOR_PLANET))
			{
				PlanetaryDataPackClass *planet_data = planet->Get_Planetary_Data();
				if (planet_data)
				{
					planet_data->Set_Is_Locally_Revealed(reveal);
					planet->Refresh_Visibility(PlayerList.Get_Local_Player());
					Story_Debug_Printf("STORY REWARD - Revealing planet %s\r\n",planet->Get_Type()->Get_Name()->c_str());
				}
			}
		}
	}
}






void StoryEventClass::Reward_Disable_Reinforcements()
{
	if (RewardParam[0].empty())
	{
		return;
	}

	bool disable = true;
	if (atoi(RewardParam[0].c_str()) == 0)
	{
		disable = false;
	}

	if (!RewardParam[1].empty())
	{
		// Find the player of the specified faction
		const FactionClass *faction = FactionList.Find_Faction_By_Name(RewardParam[1].c_str());
		if (faction == NULL)
		{
			Story_Debug_Printf("STORY REWARD ERROR!  Unable to %s reinforcements for player of faction %s\r\n",disable?"disable":"enable",RewardParam[1].c_str());
			return;
		}

		PlayerClass *player = PlayerList.Get_Player_Of_Faction(faction);
		if (player == NULL)
		{
			Story_Debug_Printf("STORY REWARD ERROR!  Unable to %s reinforcements for player of faction %s\r\n",disable?"disable":"enable",RewardParam[1].c_str());
			return;
		}

		player->Set_Can_Reinforce(!disable);
		Story_Debug_Printf("STORY REWARD - Reinforcements %s for player id %d\r\n",disable ? "DISABLED" : "ENABLED",player->Get_ID());
	}
	else
	{
		// Enable/disable reinforcements for all players
		int count = PlayerList.Get_Num_Players();
		for (int i=0; i<count; i++)
		{
			PlayerClass *player = PlayerList.Get_Player_By_Index(i);
			if (player)
			{
				player->Set_Can_Reinforce(!disable);
				Story_Debug_Printf("STORY REWARD - Reinforcements %s for player id %d\r\n",disable ? "DISABLED" : "ENABLED",player->Get_ID());
			}
		}
	}
}








void StoryEventClass::Reward_Reset_Branch()
{
	// In (error) cases, resetting a branch was resulting in infinite-recursion. -Eric_Y 
	// Basically this happens when a STORY_TRIGGER event reset's itself, and all of it's
	// prereq's are already satisfied, it will recursively trigger forever. This is an
	// error case in the XML but can easily happen if events are pasted in.
	static int recursion_limiter = 0; 

	if (recursion_limiter > 2)
	{
		Story_Debug_Printf("STORY ERROR - Event resets itself over and over forever: %s in branch %s\r\n",RewardParam[1].c_str(),RewardParam[0].c_str());
		assert(false);
		return;
	}

	if (RewardParam[0].empty())
	{
		return;
	}

	recursion_limiter++;

	SubPlot->Reset_Branch(RewardParam[0].c_str());
	Story_Debug_Printf("STORY REWARD - Resetting branch %s\r\n",RewardParam[0].c_str());

	//	This may not be necessary
	if (!RewardParam[1].empty())
	{
		SubPlot->Trigger_Event(RewardParam[1].c_str());
		Story_Debug_Printf("STORY REWARD - Triggering event %s in branch %s\r\n",RewardParam[1].c_str(),RewardParam[0].c_str());
	}

	recursion_limiter--;
}








void StoryEventClass::Reward_Reset_Event()
{
	if (!RewardParam[0].empty())
	{
		SubPlot->Reset_Event(RewardParam[0].c_str());
		Story_Debug_Printf("STORY REWARD - Resetting event %s\r\n",RewardParam[0].c_str());
	}

	if (!RewardParamList.empty())
	{
		for (unsigned int i=0; i<RewardParamList.size(); i++)
		{
			SubPlot->Reset_Event(RewardParamList[i].c_str());
			Story_Debug_Printf("STORY REWARD - Resetting event %s\r\n",RewardParam[0].c_str());
		}
	}
}








void StoryEventClass::Reward_Link_Tactical()
{
	//<Reward_Param1> Param 0 = Planet name
	//<Reward_Param2> Param 1 = Planet location (space,land)
	//<Reward_Param3> Param 2 = Attacking faction
	//<Reward_Param4> Param 3 = Map name
	//<Reward_Param5> Param 4 = Defending faction
	//<Reward_Param6> Param 5 = Hero pathfinder name
	//<Reward_Param7> Param 6 = Tactical script name
	//<Reward_Param8> Param 7 = Use sandbox units
	//<Reward_Param9> Param 8 = Enable Prebattle Cinematic
	//<Reward_Param10> Param 9 = Start Scene Faded Out
	//<Reward_Param11> Param 10 = Start Scene Letterboxed
	//<Reward_Param11> Param 11 = Post tactical resolution type.
	//<Reward_Param11> Param 12 = Show pattle pending
	//<Reward_Param12> Param 13 = Allow user defined pathfinder
	if (RewardParam[0].empty() || RewardParam[1].empty() || RewardParam[2].empty() || RewardParam[3].empty() || RewardParam[4].empty())
	{
		return;
	}

	GameObjectClass *planet = NULL;

	// Find the planet object
	const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (type)
	{
		// Pull the planet out of the galactic game
		const DynamicVectorClass<GameObjectClass *> *planets = GALACTIC_GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);
		if (planets->Size() > 0)
		{
			planet = (*planets)[0];
		}
	}

	if ((type == NULL) || (planet == NULL))
	{
		Story_Debug_Printf("STORY REWARD - ERROR! Invalid planet %s in Link_Tactical\r\n",RewardParam[0].c_str());
	}

	// Find the battle location (SPACE/LAND)
	SubGameModeType submode = SUB_GAME_MODE_INVALID;
	StoryBaseFilter location = (StoryBaseFilter)Lookup_Enum(RewardParam[1].c_str());
	if (location == BASE_GROUND)
	{
		submode = SUB_GAME_MODE_LAND;
	}
	else if (location == BASE_SPACE)
	{
		submode = SUB_GAME_MODE_SPACE;
	}

	if (submode == SUB_GAME_MODE_INVALID)
	{
		Story_Debug_Printf("STORY REWARD - ERROR! Invalid combat location %s in Link_Tactical\r\n",RewardParam[1].c_str());
		return;
	}

	// Find the attacking player
	PlayerClass *player = NULL;
	const FactionClass *faction = FactionList.Find_Faction_By_Name(RewardParam[2].c_str());
	if (faction == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR! Invalid attacking faction %s in Link_Tactical\r\n",RewardParam[2].c_str());
		return;
	}
	else
	{
		player = PlayerList.Get_Player_Of_Faction(faction);
		if (player == NULL)
		{
			Story_Debug_Printf("STORY REWARD - ERROR! Invalid attacking faction %s in Link_Tactical\r\n",RewardParam[2].c_str());
			return;
		}
	}

	bool use_sandbox = false;
	if (!RewardParam[7].empty())
	{
		if (atoi(RewardParam[7].c_str()) != 0)
		{
			use_sandbox = true;
		}
	}

	int prebattle_cinematic_enabled = 1;
	bool start_scene_faded = false;
	bool start_scene_letterboxed = false;

	// see if we should disable the pre battle cinematic
	if (!RewardParam[8].empty())
	{
		prebattle_cinematic_enabled = atoi(RewardParam[8].c_str());
	}
	// see if we should start the scene faded out
	if (!RewardParam[9].empty())
	{
		if (atoi(RewardParam[9].c_str()) != 0)
			start_scene_faded = true;
	}

	// see if we should start the scene with letter boxing on
	if (!RewardParam[10].empty())
	{
		if (atoi(RewardParam[10].c_str()) != 0)
			start_scene_letterboxed = true;
	}


	// Find the hero object
	const GameObjectTypeClass *hero_type = GameObjectTypeManager.Find_Object_Type(RewardParam[5].c_str());

	StoryPostLinkedTacticalEnum resolution_type = POST_LINKED_TACTICAL_DESTROY_AI;
	if (!RewardParam[11].empty())
	{
		resolution_type = static_cast<StoryPostLinkedTacticalEnum>(Lookup_Enum(RewardParam[11].c_str()));
		FAIL_IF(resolution_type == POST_LINKED_TACTICAL_NONE)
		{
			Story_Debug_Printf("STORY_REWARD - ERROR! Unknown resolution %s to apply following this linked tactical battle.", RewardParam[11].c_str());
			resolution_type = POST_LINKED_TACTICAL_DESTROY_AI;
		}
	}

	bool show_battle_pending = true;
	if (!RewardParam[12].empty())
	{
		show_battle_pending = (atoi(RewardParam[12].c_str()) != 0);
	}

	bool allow_user_pathfinder = true;
	if (!RewardParam[13].empty())
	{
		allow_user_pathfinder = (atoi(RewardParam[13].c_str()) != 0);
	}

	// Queue up tactical map
	Story_Debug_Printf("STORY REWARD - Linked tactical map.  Planet %s, Mode %s, Attacker %s, Map %s, Defender %s, Hero %s, Script %s, sandbox %s\r\n",RewardParam[0].c_str(),RewardParam[1].c_str(),
					 RewardParam[2].c_str(),RewardParam[3].c_str(),RewardParam[4].c_str(),RewardParam[5].c_str(),RewardParam[6].c_str(),use_sandbox ? "true" : "false");

	GameModeManager.Queue_Next_Campaign_Tactical_Conflict(planet->Get_ID(),
																			submode,
																			player->Get_ID(),
																			RewardParam[3].c_str(),
																			RewardParam[4].c_str(),
																			hero_type,
																			&RewardParam[6],
																			use_sandbox,
																			prebattle_cinematic_enabled,
																			start_scene_faded,
																			start_scene_letterboxed,
																			resolution_type,
																			show_battle_pending,
																			allow_user_pathfinder);

	// This may not work, but what the heck.  If we're forcing combat from galactic, force the transition to the combat otherwise it will never trigger
	if (GameModeManager.Get_Active_Mode()->Get_Sub_Type() == SUB_GAME_MODE_GALACTIC)
	{
		//The loading dialog already pauses and we need to match pauses with unpauses.
		//GameModeManager.Pause_Galactic_Game();
		GameModeManager.Transition_To_Next_Campaign_Queued_Tactical_Conflict();
	}
}







void StoryEventClass::Reward_Random_Story()
{
	TheRandomStoryMode.Generate_Random_Story();
	Story_Debug_Printf("STORY REWARD - Generating random story\r\n");
}







void StoryEventClass::Reward_Disable_Special_Structure()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		return;
	}

	GameObjectClass *structure = NULL;
	bool disable = false;

	const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (type)
	{
		const DynamicVectorClass<GameObjectClass *> *structures = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);
		if (structures->Size() > 0)
		{
			structure = (*structures)[0];
		}
	}

	if ((type == NULL) || (structure == NULL))
	{
		Story_Debug_Printf("STORY REWARD - ERROR! Invalid structure %s in DISABLE_SPECIAL_STRUCTURE\r\n",RewardParam[0].c_str());
		return;
	}

	if (atoi(RewardParam[1].c_str()) != 0)
	{
		disable = true;
	}

	// Figure out what special ability to disable.  For now only sheild ability is being disabled
	if (structure->Behaves_Like(BEHAVIOR_BASE_SHIELD))
	{
		BaseShieldBehaviorClass *sbehavior = (BaseShieldBehaviorClass *)structure->Get_Behavior(BEHAVIOR_BASE_SHIELD);
		if (sbehavior)
		{
			sbehavior->Force_Off(disable);
		}
	}
}




void StoryEventClass::Reward_Set_Tech_Level()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		Story_Debug_Printf("STORY REWARD ERROR!  Set tech level failed due to incomplete parameters.\r\n");
		return;
	}

	const FactionClass *faction = FactionList.Find_Faction_By_Name(RewardParam[0].c_str());
	if (faction == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Unable to find faction %s\r\n",RewardParam[0].c_str());
		return;
	}

	PlayerClass *player = PlayerList.Get_Player_Of_Faction(faction);
	if (player == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Unable to find plater of faction %s\r\n",RewardParam[0].c_str());
		return;
	}

	int tech_level = atoi(RewardParam[1].c_str());
	if (player->Get_Max_Tech_Level() < tech_level)
	{
		player->Set_Max_Tech_Level(tech_level);
	}
	player->Set_Tech_Level(tech_level);
	Story_Debug_Printf("STORY REWARD - Setting tech level for %s to %d\n",RewardParam[0].c_str(),tech_level);
}






void StoryEventClass::Reward_Disable_Spawn()
{
	// Function has changed to allow specifying the spawn percentage instead of only turning on/off
	#if 0
	if (RewardParam[0].empty() || RewardParam[1].empty() || RewardParam[2].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Disable spawn failed due to incomplete parameters.\r\n");
		return;
	}

	int count;
	bool disable = false;
	GameObjectClass *structure = NULL;
	const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	const DynamicVectorClass<GameObjectClass *> *structures = NULL;

	if (type)
	{
		structures = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);
	}

	if ((type == NULL) || (structures == NULL) || (structures->Size() == 0))
	{
		Story_Debug_Printf("STORY REWARD - ERROR! Invalid structure %s in DISABLE_SPAWN\r\n",RewardParam[0].c_str());
		return;
	}

	if (atoi(RewardParam[1].c_str()) != 0)
	{
		disable = true;
	}

	count = atoi(RewardParam[2].c_str());
	if ((count <= 0) || (count > structures->Size()))
	{
		count = structures->Size();
	}

	for (int i=0; i<count; i++)
	{
		structure = (*structures)[i];
		structure->Set_Garrison_Spawn_Enabled(!disable);
	}
	#else
	
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Set spawn failed due to incomplete parameters.\r\n");
		return;
	}

	float spawn_percent = (float)atof(RewardParam[1].c_str());
	SpawnIndigenousUnitsBehaviorClass::Adjust_Spawn_Distribution(RewardParam[0],spawn_percent);

	#endif
}






void StoryEventClass::Reward_Trigger_Event()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Trigger event failed due to incomplete parameters.\r\n");
		return;
	}

	GameModeManager.Get_Active_Mode()->Get_Story_Mode().Trigger_Event(RewardParam[0].c_str());
}






void StoryEventClass::Reward_Enable_Galactic_Reveal()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Galactic reveal failed due to incomplete parameters.\r\n");
		return;
	}

	GalacticModeClass *galactic = (GalacticModeClass *)GameModeManager.Get_Game_Mode_By_Sub_Type(SUB_GAME_MODE_GALACTIC);
	if (galactic)
	{
		if (atoi(RewardParam[0].c_str()) == 0)
		{
			galactic->Set_Planet_Auto_Reveal(false);
			Story_Debug_Printf("STORY REWARD - Turning galactic planet autoreveal OFF\r\n");
		}
		else
		{
			galactic->Set_Planet_Auto_Reveal(true);
			Story_Debug_Printf("STORY REWARD - Turning galactic planet autoreveal ON\r\n");
		}
	}
}






void StoryEventClass::Reward_Activate_Retry_Dialog()
{
	// No parameters for this reward
	Story_Debug_Printf("STORY REWARD - Activating retry dialog box\r\n");

	if (!Is_Retry_Dialog_Active())
	{
		Activate_Retry_Dialog();
	}
}






void StoryEventClass::Reward_Set_Use_Retry_Dialog()
{
	// No parameters for this reward
	Story_Debug_Printf("STORY REWARD - Setting retry dialog box\r\n");

	GameModeManager.Get_Active_Mode()->Get_Victory_Monitor()->Set_Use_Retry_Dialog(true);
}






void StoryEventClass::Reward_Start_Movie()
{
	Story_Debug_Printf("*** START MOVIE ***");
	FrameSynchronizer.Pause();
	GameModeManager.Play_Movie(RewardParam[0].c_str());
}





void StoryEventClass::Reward_Set_Planet_Spawn()
{
	if (RewardParam[0].empty() || RewardParam[1].empty() || RewardParam[2].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Set planet spawn failed due to incomplete parameters.\r\n");
		return;
	}

	float percentage = (float)atof(RewardParam[2].c_str());

	const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (type)
	{
		const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);
		if (planets->Size() > 0)
		{
			GameObjectClass *planet = (*planets)[0];
         PlanetaryDataPackClass *planet_data = planet->Get_Planetary_Data();
			planet_data->Add_Indigenous_Spawn_Bonus(RewardParam[1].c_str(), percentage);
		}
	}
}






void StoryEventClass::Reward_Commandbar_Movie(const char *param0, const char *param1)
{
	if (RewardParam[0].empty() && (param0 == NULL))
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Commandbar movie failed due to incomplete parameters.\r\n");
		return;
	}

	bool loop = false;

	if (!RewardParam[1].empty() && (param1 == NULL))
	{
		if (atoi(RewardParam[1].c_str()) != 0)
		{
			loop = true;
		}
	}
	else if (param1 != NULL)
	{
		if (atoi(param1) != 0)
		{
			loop = true;
		}
	}


	if (param0 != NULL)
	{
		TheCommandBar.Play_Movie(param0,loop);
	}
	else
	{
		TheCommandBar.Play_Movie(RewardParam[0].c_str(),loop);
	}
}






void StoryEventClass::Reward_Stop_Commandbar_Movie()
{
	TheCommandBar.Stop_Movie();
}




void StoryEventClass::Reward_Set_Weather()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Set weather failed due to incomplete parameters.\r\n");
		return;
	}
}





void StoryEventClass::Reward_Planet_Faction()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Planet faction failed due to incomplete parameters.\r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Planet faction failed due to invalid planet %s.\r\n",RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Planet faction failed due to non-existant planet %s.\r\n",RewardParam[0].c_str());
		return;
	}

	const FactionClass *new_faction = FactionList.Find_Faction_By_Name(RewardParam[1].c_str());
	if (new_faction == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Planet faction failed due to invalid faction %s.\r\n",RewardParam[1].c_str());
		return;
	}

	GameObjectClass *planet = (*planets)[0];
	PlanetaryBehaviorClass *planet_behavior = (PlanetaryBehaviorClass*)planet->Get_Behavior(BEHAVIOR_PLANET);
	planet_behavior->Change_Faction(planet,new_faction);

	Story_Debug_Printf("STORY REWARD - Changing planet %s to faction %s\r\n",RewardParam[0].c_str(),RewardParam[1].c_str());
}







void StoryEventClass::Reward_Force_Retreat()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Force retreat failed due to incomplete parameters.\r\n");
		return;
	}

	const FactionClass *new_faction = FactionList.Find_Faction_By_Name(RewardParam[0].c_str());
	if (new_faction == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Force retreat failed due to invalid faction %s.\r\n",RewardParam[0].c_str());
		return;
	}

	PlayerClass *player = PlayerList.Get_Player_Of_Faction(new_faction);
	if (player == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Force retreat failed due to no player of faction %s.\r\n",RewardParam[0].c_str());
		return;
	}

	RetreatCoordinatorClass *retreat_coordinator = GameModeManager.Get_Active_Mode()->Get_Retreat_Coordinator();
	if ( retreat_coordinator == NULL )
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  Force retreat failed due to no retreat coordinator.\r\n");
		return;
	}

	if ( retreat_coordinator->Is_A_Retreat_Underway() == false )
	{
		retreat_coordinator->Request_Start_Player_Retreat( player->Get_ID(), false );
		Story_Debug_Printf("STORY REWARD - Force retreat of player %s.\r\n",RewardParam[0].c_str());
	}
}



void StoryEventClass::Reward_Set_Planet_Restricted()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_PLANET_RESTRICTED failed due to incomplete parameters.\r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_PLANET_RESTRICTED failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_PLANET_RESTRICTED failed due to non-existant planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	bool restrict = false;
	if (atoi(RewardParam[1].c_str()) != 0)
	{
		restrict = true;
	}

	bool turn_red = true;
	if (!RewardParam[3].empty())
	{
		if (atoi(RewardParam[3].c_str()) == 0)
		{
			turn_red = false;
		}
	}

	PlanetaryBehaviorClass *planet_behavior = static_cast<PlanetaryBehaviorClass*>(planets->Get_At(0)->Get_Behavior(BEHAVIOR_PLANET));
	if (!planet_behavior)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_PLANET_RESTRICTED failed because %s is not a planet.\r\n", RewardParam[0].c_str());
		return;
	}

	planet_behavior->Set_Access_Restricted(planets->Get_At(0), restrict, turn_red);
	Story_Debug_Printf("STORY REWARD - Restriction of planet %s %s.\r\n",RewardParam[0].c_str(),restrict?"ON":"OFF");

	// Newly added to turn on/off special ability slots on restricted planets
	bool ability_restrict = false;
	if (!RewardParam[2].empty() && atoi(RewardParam[2].c_str()) != 0)
	{
		ability_restrict = true;
	}

	TheTutorial.Set_Restrict_All_Abilities(planets->Get_At(0), ability_restrict);
}





void StoryEventClass::Reward_Multimedia()
{
	// Param0 = Reward_Param1 - text string to display
	// Param1 = Reward_Param2 - duration to display text
	// Param2 = Reward_Param3 - object (name) to include in the screen text.  typically a planet
	// Param3 = Reward_Param4 - remove text
	// Param4 = Reward_Param5 - use teletype
	// Param5 = Reward_Param6 - text color
	// Param6 = Reward_Param7 - is this text a subtitle?
	// Param7 = Reward_Param8 - speech file to play
	// Param8 = Reward_Param9 - movie to play
	// Param9 = Reward_Param10 - should movie loop

	// Do screen text
	if (!RewardParam[0].empty())
	{
		// The parameters match up with the screen_text order so we don't need to pass anything in
		Reward_Screen_Text();
	}

	// Do speech
	if (!RewardParam[7].empty())
	{
		Reward_Speech(RewardParam[7].c_str());
	}

	// Do commandbar movie
	if (!RewardParam[8].empty())
	{
		if (RewardParam[9].empty())
		{
			Reward_Commandbar_Movie(RewardParam[8].c_str(),"0");
		}
		else
		{
			Reward_Commandbar_Movie(RewardParam[8].c_str(),RewardParam[9].c_str());
		}
	}
}





void StoryEventClass::Reward_Set_Planet_Visibility_Level()
{
	//Parameter 0: Planet in question
	//Parameter 1: Player in question
	//Parameter 2: visibility to apply (or none to undo a previous modification)
	//Parameter 3: string to tag this visibility modification
	//Parameter 4: (optional) duration for modification.  Defaults to forever.

	if (RewardParam[0].empty() || RewardParam[1].empty() || RewardParam[2].empty() || RewardParam[3].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_PLANET_VISIBILITY_LEVEL failed due to incomplete parameters.\r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_PLANET_VISIBILITY_LEVEL failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_PLANET_VISIBILITY_LEVEL failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const FactionClass *faction = FactionList.Find_Faction_By_Name(RewardParam[1].c_str());
	if (faction == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_PLANET_VISIBILITY_LEVEL failed due to invalid faction %s.\r\n",RewardParam[1].c_str());
		return;
	}

	PlayerClass *player = PlayerList.Get_Player_Of_Faction(faction);
	if (player == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_PLANET_VISIBILITY_LEVEL failed due to no player of faction %s.\r\n",RewardParam[1].c_str());
		return;
	}

	PlanetaryBehaviorClass *planet_behavior = static_cast<PlanetaryBehaviorClass*>(planets->Get_At(0)->Get_Behavior(BEHAVIOR_PLANET));
	if (!planet_behavior)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_PLANET_VISIBILITY_LEVEL failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	int modifier_id = CRCClass::Calculate_CRC(RewardParam[3].c_str(), RewardParam[3].size());

	if (_stricmp(RewardParam[2].c_str(), "none") == 0)
	{
		planet_behavior->Remove_Visibility_Modifier(player, modifier_id);
	}
	else
	{
		VisibilityLevelType visibility = VISIBILITY_LEVEL_INVALID;
		if (!TheVisibilityLevelTypeConverterPtr->String_To_Enum(RewardParam[2], visibility))
		{
			Story_Debug_Printf("STORY REWARD - ERROR!  SET_PLANET_VISIBILITY_LEVEL failed due to unrecognized visibility level %s.\r\n", RewardParam[2].c_str());
			return;
		}

		float duration = MODIFIER_GOOD_UNTIL_REMOVED;
		if (!RewardParam[4].empty())
		{
			duration = static_cast<float>(atof(RewardParam[4].c_str()));
		}

		planet_behavior->Set_Visibility_Modifier(player, visibility, modifier_id, duration);
	}
}




void StoryEventClass::Reward_Pause_Galactic_Game()
{
	float pause = true;

	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
		{
			pause = false;
		}
	}

	if (pause)
	{
		GameModeManager.Pause_Galactic_Game();
	}
	else
	{
		GameModeManager.Resume_Galactic_Game();
	}
}





void StoryEventClass::Reward_Add_Objective()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  ADD_OBJECTIVE failed due to incomplete parameters.\r\n");
		return;
	}

	Story_Debug_Printf("STORY REWARD - ADD_OBJECTIVE %s.\r\n",RewardParam[0].c_str());
	SubPlot->Get_Story_Mode()->Add_Objective(RewardParam[0]);
}




void StoryEventClass::Reward_Remove_Objective()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  REMOVE_OBJECTIVE failed due to incomplete parameters.\r\n");
		return;
	}

	Story_Debug_Printf("STORY REWARD - REMOVE_OBJECTIVE %s.\r\n",RewardParam[0].c_str());
	SubPlot->Get_Story_Mode()->Remove_Objective(RewardParam[0]);
}




void StoryEventClass::Reward_Objective_Complete()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  REMOVE_OBJECTIVE failed due to incomplete parameters.\r\n");
		return;
	}

	Story_Debug_Printf("STORY REWARD - OBJECTIVE_COMPLETE %s.\r\n",RewardParam[0].c_str());
	SubPlot->Get_Story_Mode()->Objective_Completed(RewardParam[0]);
}




void StoryEventClass::Reward_Remove_All_Objectives()
{
	Story_Debug_Printf("STORY REWARD - Removing all objectives.\r\n");
	SubPlot->Get_Story_Mode()->Remove_All_Objectives();
}




void StoryEventClass::Reward_Objective_Failed()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  OBJECTIVE_FAILED failed due to incomplete parameters.\r\n");
		return;
	}

	Story_Debug_Printf("STORY REWARD - OBJECTIVE_FAILED %s.\r\n",RewardParam[0].c_str());
	SubPlot->Get_Story_Mode()->Objective_Failed(RewardParam[0]);
}





void StoryEventClass::Reward_Enable_Direct_Invasion()
{
	Story_Debug_Printf("STORY REWARD - Direct invasion has been ENABLED.\r\n");
	TheTutorial.Set_Enable_Direct_Invasion(true);
}




void StoryEventClass::Reward_Disable_Direct_Invasion()
{
	Story_Debug_Printf("STORY REWARD - Direct invasion has been DISABLED.\r\n");
	TheTutorial.Set_Enable_Direct_Invasion(false);
}




void StoryEventClass::Reward_Enable_Objective_Display()
{
	bool enable = true;
	if (!RewardParam[0].empty() && (atoi(RewardParam[0].c_str()) == 0))
	{
		enable = false;
	}

	Story_Debug_Printf("STORY REWARD - Objective display has been %s.\r\n",enable ? "ENABLED" : "DISABLED");
	TheCommandBar.Show_Objectives(enable);
}




void StoryEventClass::Reward_Flash_Fleet_With_Unit()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FLASH_FLEET_WITH_UNIT failed due to incomplete parameters.\r\n");
		return;
	}

	const GameObjectTypeClass *unit_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (unit_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FLASH_FLEET_WITH_UNIT failed due to invalid unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	GameObjectClass *first_unit = NULL;

	#if 1
	// New way of doing this can detect teams in transports
	ReferenceListIterator<GameObjectClass> it(GAME_OBJECT_MANAGER.Get_Selectable_Objects());
	for (; !it.Is_Done(); it.Next())
	{
		GameObjectClass *unit = it.Current_Object();

		const GameObjectTypeClass *this_unit_type = unit->Get_Type();
		if (this_unit_type)
		{
			if (this_unit_type == unit_type)
			{
				first_unit = unit;
				break;
			}
			else
			{
				//Debug_Printf("NO MATCH - %s != %s\n",this_unit_type->Get_Name()->c_str(),unit_type->Get_Name()->c_str());
			}
		}

		this_unit_type = unit->Get_Original_Object_Type();
		if (this_unit_type)
		{
			if (this_unit_type == unit_type)
			{
				first_unit = unit;
				break;
			}
			else
			{
				//Debug_Printf("NO MATCH - %s != %s\n",this_unit_type->Get_Name()->c_str(),unit_type->Get_Name()->c_str());
			}
		}

		if (unit->Behaves_Like(BEHAVIOR_TRANSPORT))
		{
			TransportBehaviorClass *tbehave = (TransportBehaviorClass *)unit->Get_Behavior(BEHAVIOR_TRANSPORT);
			for (int s = 0; s < (int)tbehave->Get_Contained_Objects_Count(); s++)
			{
				GameObjectClass *gunit = tbehave->Get_Contained_Object(unit, s);
				this_unit_type = gunit->Get_Type();
				if (this_unit_type)
				{
					if (this_unit_type == unit_type)
					{
						first_unit = gunit;
						break;
					}
					else
					{
						//Debug_Printf("NO MATCH - %s != %s\n",this_unit_type->Get_Name()->c_str(),unit_type->Get_Name()->c_str());
					}
				}
			}
		}

		if (unit->Behaves_Like(BEHAVIOR_FLEET))
		{
			FleetBehaviorClass *behave = (FleetBehaviorClass *)unit->Get_Behavior(BEHAVIOR_FLEET);
			for (int t = 0; t < (int)behave->Get_Contained_Objects_Count(); t++)
			{
				GameObjectClass *gunit = behave->Get_Contained_Object(unit, t);
				this_unit_type = gunit->Get_Type();
				if (this_unit_type)
				{
					if (this_unit_type == unit_type)
					{
						first_unit = gunit;
						break;
					}
					else
					{
						//Debug_Printf("NO MATCH - %s != %s\n",this_unit_type->Get_Name()->c_str(),unit_type->Get_Name()->c_str());
					}
				}
			}
		}
	}

	if (first_unit == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FLASH_FLEET_WITH_UNIT failed due to non-existent unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	#else

	// Old way of doing this didn't detect teams in transports
	const DynamicVectorClass<GameObjectClass *> *units = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(unit_type);
	if (units->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FLASH_FLEET_WITH_UNIT failed due to non-existent unit %s.\r\n", RewardParam[0].c_str());
		return;
	}
	if (units->Size() > 1)
	{
		Story_Debug_Printf("STORY REWARD - WARNING!  FLASH_FLEET_WITH_UNIT multiple units %s found.\r\n", RewardParam[0].c_str());
	}
	first_unit = (*units)[0];

	#endif

	GameObjectClass *parent = first_unit->Get_Parent_Container_Object();

	while (parent && !parent->Behaves_Like(BEHAVIOR_FLEET) && !parent->Behaves_Like(BEHAVIOR_PLANET))
	{
		parent = parent->Get_Parent_Container_Object();
	}

	if (parent != NULL)
	{
		const char *tag = NULL;
		if (!RewardParam[1].empty())
		{
			tag = RewardParam[1].c_str();
		}

		if (parent->Behaves_Like(BEHAVIOR_FLEET))
		{
			TheCommandBar.Flash_Fleet(parent,tag);
		}
		else
		{
			TheCommandBar.Flash_Planet_Component(parent->Get_Type()->Get_Name()->c_str(),FLASH_TROOPS,-1,tag);
		}
	}
}

void StoryEventClass::Reward_Set_Max_Tech_Level()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_MAX_TECH_LEVEL failed due to incomplete parameters.\r\n");
		return;
	}

	const FactionClass *faction = TheFactionTypeConverterPtr->Get_Managed_Object(RewardParam[0]);
	if (!faction)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_MAX_TECH_LEVEL failed because of unknown player faction %s.\r\n", RewardParam[0].c_str());
		return;
	}

	PlayerClass *player = PlayerList.Get_Player_Of_Faction(faction);
	if (!player)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_MAX_TECH_LEVEL failed because no player exists for faction %s.\r\n", RewardParam[0].c_str());
		return;
	}

	int tech_level = atoi(RewardParam[1].c_str());

	player->Set_Max_Tech_Level(tech_level);
}


void StoryEventClass::Reward_Tutorial_Player()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_TUTORIAL_PLAYER needs param1 = player faction\r\n");
		return;
	}

	const FactionClass *faction = TheFactionTypeConverterPtr->Get_Managed_Object(RewardParam[0]);
	if (!faction)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_TUTORIAL_PLAYER failed because of unknown player faction %s.\r\n", RewardParam[0].c_str());
		return;
	}

	PlayerClass *player = PlayerList.Get_Player_Of_Faction(faction);
	if (!player)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_MAX_TECH_LEVEL failed because no player exists for faction %s.\r\n", RewardParam[0].c_str());
		return;
	}

	player->Set_Is_Tutorial_Player(true);
}


void StoryEventClass::Reward_Select_Planet()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SELECT_PLANET failed due to incomplete parameters.\r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SELECT_PLANET failed due to invalid unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SELECT_PLANET failed due to non-existent unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	Story_Debug_Printf("STORY REWARD - Selecting planet %s.\r\n", RewardParam[0].c_str());

	GameObjectClass *planet = (*planets)[0];
	TheCommandBar.Select_Planet(planet);
}





void StoryEventClass::Reward_Force_Click_GUI()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FORCE_CLICK_GUI failed due to incomplete parameters.\r\n");
		return;
	}

	BaseComponentClass *component = TheCommandBar.Get_Component(RewardParam[0].c_str());
	if (component == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FORCE_CLICK_GUI failed due to invalid component name %s.\r\n",RewardParam[0].c_str());
		return;
	}
	else
	{
		// Fake clicking on component
		component->Left_Button_Released(true);
	}
}





void StoryEventClass::Reward_Flash_Production_Choice()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FLASH_PRODUCTION_CHOICE failed due to incomplete parameters.\r\n");
		return;
	}

	const GameObjectTypeClass *unit_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (unit_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FLASH_PRODUCTION_CHOICE failed due to invalid unit type %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const char *tag = NULL;
	if (!RewardParam[1].empty())
	{
		tag = RewardParam[1].c_str();
	}

	Story_Debug_Printf("STORY REWARD - Flashing production choice %s.\r\n",RewardParam[0].c_str());
	TheCommandBar.Flash_Production_Choice(unit_type,tag);
}



void StoryEventClass::Reward_Flash_Special_Ability()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FLASH_SPECIAL_ABILITY failed due to incomplete parameters.\r\n");
		return;
	}

	UnitAbilityType ability_type = ABILITY_NONE;
	if (!TheUnitAbilityTypeConverterPtr->String_To_Enum(RewardParam[0], ability_type))
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FLASH_SPECIAL_ABILITY failed due to invalid ability type %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const char *tag = NULL;
	if (!RewardParam[1].empty())
	{
		tag = RewardParam[1].c_str();
	}

	Story_Debug_Printf("STORY REWARD - Flashing special ability %s.\r\n",RewardParam[0].c_str());
	TheCommandBar.Flash_Tactical_Select_Button((int) ability_type,tag);
}




void StoryEventClass::Reward_Pause_Galactic()
{
	Story_Debug_Printf("STORY REWARD - Pausing galactic mode.\r\n");
	GameModeManager.Pause_Galactic_Game();
}




void StoryEventClass::Reward_Unpause_Galactic()
{
	Story_Debug_Printf("STORY REWARD - Unpausing galactic mode.\r\n");
	GameModeManager.Resume_Galactic_Game();
}




void StoryEventClass::Reward_Enable_Overwhelming_Odds()
{
	bool onoff = true;

	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
		{
			onoff = false;
		}
	}

	if (onoff)
	{
		Story_Debug_Printf("STORY REWARD - Enabling overwhelming odds.\r\n");
	}
	else
	{
		Story_Debug_Printf("STORY REWARD - Disabling overwhelming odds.\r\n");
	}

	GameModeManager.Get_Active_Mode()->Set_Enable_Overwhelming_Odds(onoff);
}





void StoryEventClass::Reward_Set_Sandbox_Objectives()
{
	SubPlot->Get_Story_Mode()->Set_Sandbox_Objectives();
}




void StoryEventClass::Reward_Force_Respawn()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FORCE_RESPAWN failed due to incomplete parameters.\r\n");
		return;
	}

	char name[128];
	strcpy(name,RewardParam[0].c_str());
	std::string hero_name = _strupr(name);

	const GameObjectTypeClass *hero_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (hero_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FORCE_RESPAWN failed due to invalid hero unit type %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass*> *objs = GAME_OBJECT_MANAGER.Find_Objects(BEHAVIOR_PLANET);
	for (int i = 0; i < objs->Count(); i++)
	{
		GameObjectClass *object = (*objs)[i];
		PlanetaryBehaviorClass *planet_behave = static_cast<PlanetaryBehaviorClass*>(object->Get_Behavior(BEHAVIOR_PLANET));

		if (planet_behave->Is_Object_Type_Scheduled_For_Creation(hero_type))
		{
			planet_behave->Force_Respawn(hero_type);
		}
	}
}



void StoryEventClass::Reward_Scroll_Lock()
{

	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SCROLL_LOCK takes one parameter, and param = 0 or 1 \r\n");
		return;
	}

	bool onoff = true;
	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
		{
			onoff = false;
		}
	}

	if (onoff)
		Story_Debug_Printf("STORY REWARD - Locking user scrolling (SCROLL_LOCK = 1)\r\n");
	else
		Story_Debug_Printf("STORY REWARD - Unlocking user scrolling (SCROLL_LOCK = 0)\r\n");

	GameModeManager.Get_Active_Mode()->Set_User_Scrolling_Disable(onoff);
}



void StoryEventClass::Reward_Skirmish_Rules()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SKIRMISH_RULES takes one parameter, and param = 0 or 1 \r\n");
		return;
	}

	bool onoff = true;
	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
		{
			onoff = false;
		}
	}

	if (onoff)
		Story_Debug_Printf("STORY REWARD - Setting game to use Skirmish Rules.\r\n");
	else
		Story_Debug_Printf("STORY REWARD - Turning off skirmish rule override.\r\n");

	GameModeManager.Force_Skirmish_Game_Rules(onoff);
}


void StoryEventClass::Reward_Reset_Galactic_Filters()
{
	Story_Debug_Printf("STORY REWARD - Resetting (turning OFF) all galactic filters");
	TheCommandBar.All_Filters_Off();
}


// This reward turns on/off the advisor that gives you on-screen hints during gameplay.
// There are situations where the hints are too distracting (e.g. During a tutorial where you are already
// receiving instructions). -ERic_Y

void StoryEventClass::Reward_Set_Advisor()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SET_ADVISOR takes one parameter, and param = 0 or 1 \r\n");
		return;
	}

	bool onoff = true;
	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
			onoff = false;
	}

	Story_Debug_Printf("STORY REWARD - Setting on-screen advisor to %s.\r\n",onoff?"ON":"OFF");

	PlayerClass *local_player = SubPlot->Get_Local_Player();
	SubGameModeType sub_type = GameModeManager.Get_Active_Mode()->Get_Sub_Type();
	local_player->Set_Advisor_Hints_Enabled(sub_type,onoff);
}


void StoryEventClass::Reward_Start_Cinematic_Mode()
{
	Story_Debug_Printf("STORY REWARD - Starting Cinematic Mode");
	//CinematicsManager.Init_Movie_Mode(true);
	CinematicsManager.Init_Tutorial_Galactic_Tour();
}

void StoryEventClass::Reward_Stop_Cinematic_Mode()
{
	Story_Debug_Printf("STORY REWARD - Stopping Cinematic Mode");
	CinematicsManager.End_Cinematic_Mode();
}


// Note: Enables/Disables display of the command bar. This was added for Tutorial Zero. Eric_Y 10/24/05
void StoryEventClass::Reward_Show_Command_Bar()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_COMMAND_BAR takes one parameter, and param = 0 or 1 \r\n");
		return;
	}

	bool onoff = true;
	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
			onoff = false;
	}

	Story_Debug_Printf("STORY REWARD - Setting show-command-bar to %s.\r\n",onoff?"ON":"OFF");


	SubGameModeType sub_type = GameModeManager.Get_Active_Mode()->Get_Sub_Type();
	switch(sub_type)
	{
	case SUB_GAME_MODE_GALACTIC:
		TheCommandBar.Toggle_Galactic_Command_Bar(onoff);
		break;

	case SUB_GAME_MODE_LAND:
	case SUB_GAME_MODE_SPACE:
		TheCommandBar.Toggle_Tactical_Command_Bar(onoff);
		TheRadarMap.Enable(onoff);
		break;

	default: 
		assert(false); // Unknown sub-game mode type! 
	}

}

/**************************************************************************************************
* StoryEventClass::Reward_Hide_Autoresolve()
*
* This tells the command bar to hide the auto-resolve interface. Right now the tutorial text and
* the auto-resolve interface overlap onscreen. We also don't want players to auto-resolve tutorials.
*
* History: 11/01/2005 ENY
**************************************************************************************************/

void StoryEventClass::Reward_Hide_Autoresolve()
{
	Story_Debug_Printf("STORY REWARD - Hiding the autoresolve interface.");
	TheCommandBar.Set_Hide_Autoresolve(true);
}

/**************************************************************************************************
* StoryEventClass::Reward_Disable_Buildable()
*
* Tell the command bar to disable (gray out) a given buildable unit/structure/technology. 
* This is used in the tutorial / demo to show the player the possibilities without allowing them
* to get side-tracked.
*
* History: 11/07/2005 ENY
**************************************************************************************************/

void StoryEventClass::Reward_Disable_Buildable()
{
	PlayerClass *local_player = SubPlot->Get_Local_Player();

	const GameObjectTypeClass *disable_type = NULL;
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - Error! Disable Buildable - No unit specified\r\n");
	}
	else
	{
		disable_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	}

	if (disable_type)
	{
		local_player->Add_Object_Type_To_Build_Disabled_List(disable_type);
		Story_Debug_Printf("STORY REWARD - Disabling production of %s\r\n",RewardParam[0].c_str());
	}
}

/**************************************************************************************************
* StoryEventClass::Reward_Enable_Buildable()
*
* Tell the command bar to re-enable a given buildable unit/structure/technology. See Reward_Disalbe_Buildable.
*
* History: 11/07/2005 ENY
**************************************************************************************************/

void StoryEventClass::Reward_Enable_Buildable()
{
	PlayerClass *local_player = SubPlot->Get_Local_Player();

	const GameObjectTypeClass *enable_type = NULL;
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - Error! Enable Buildable - No unit specified\r\n");
	}
	else
	{
		enable_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	}

	if (enable_type)
	{
		local_player->Remove_Object_Type_From_Build_Disabled_List(enable_type);
		Story_Debug_Printf("STORY REWARD - Re-enabling production of %s\r\n",RewardParam[0].c_str());
	}
}

/**************************************************************************************************
* StoryEventClass::Reward_Hide_Cursor_On_Click()
*
* Tell the command bar to hide the tutorial cursor when a mouse click (of any sort) occurs. 
* There are many places in the campaigns and tutorials where if the user clicks, the red tutorial
* cursor cross-hairs highlight an empty or invalid spot.
*
* History: 11/11/2005 ENY
**************************************************************************************************/

void StoryEventClass::Reward_Hide_Cursor_On_Click()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  HIDE_CURSOR_ON_CLICK takes one parameter, param = 0 or 1 \r\n");
		return;
	}

	bool onoff = true;
	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
		{
			onoff = false;
		}
	}

	Story_Debug_Printf("STORY REWARD - HIDE_CURSOR_ON_CLICK set to %s\r\n",onoff?"ON":"OFF");

	TheCommandBar.Set_Hide_Cursor_On_Click(onoff);
}


/**************************************************************************************************
* StoryEventClass::Reward_Lock_Planet_Selection()
*
* Tell the command bar to not allow (or allow) the player to change which planet is selected.
*
* History: 11/15/2005 ENY
**************************************************************************************************/

void StoryEventClass::Reward_Lock_Planet_Selection()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  LOCK_PLANET_SELECTION takes one parameter, param = 0 or 1 \r\n");
		return;
	}

	bool onoff = true;
	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
		{
			onoff = false;
		}
	}

	Story_Debug_Printf("STORY REWARD - LOCK_PLANET_SELECTION set to %s\r\n",onoff?"ON":"OFF");

	TheCommandBar.Lock_Planet_Selection(onoff);
}





void StoryEventClass::Reward_Show_Smuggle_Slot()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_SMUGGLE_SLOT takes one parameter, a planet name \r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SELECT_PLANET failed due to invalid unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SELECT_PLANET failed due to non-existent unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	TheTutorial.Set_Show_Smuggle_Planet((*planets)[0]);
	Story_Debug_Printf("STORY REWARD - Forcing smuggle slot to be shown at planet %s\r\n",RewardParam[0].c_str());
}






void StoryEventClass::Reward_Hide_Smuggle_Slot()
{
	TheTutorial.Set_Show_Smuggle_Planet(NULL);
	Story_Debug_Printf("STORY REWARD - No longer forcing smuggle slot to be shown\r\n");
}






void StoryEventClass::Reward_Show_Raid_Slot()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_RAID_SLOT takes one parameter, a planet name \r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_RAID_SLOT failed due to invalid unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_RAID_SLOT failed due to non-existent unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	TheTutorial.Set_Show_Raid_Planet((*planets)[0]);
	Story_Debug_Printf("STORY REWARD - Forcing raid slot to be shown at planet %s\r\n",RewardParam[0].c_str());
}






void StoryEventClass::Reward_Hide_Raid_Slot()
{
	TheTutorial.Set_Show_Raid_Planet(NULL);
	Story_Debug_Printf("STORY REWARD - No longer forcing raid slot to be shown\r\n");
}







void StoryEventClass::Reward_Show_Steal_Slot()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_STEAL_SLOT takes one parameter, a planet name \r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_STEAL_SLOT failed due to invalid unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_STEAL_SLOT failed due to non-existent unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	TheTutorial.Set_Show_Steal_Planet((*planets)[0]);
	Story_Debug_Printf("STORY REWARD - Forcing steal slot to be shown at planet %s\r\n",RewardParam[0].c_str());
}






void StoryEventClass::Reward_Hide_Steal_Slot()
{
	TheTutorial.Set_Show_Steal_Planet(NULL);
	Story_Debug_Printf("STORY REWARD - No longer forcing steal slot to be shown\r\n");
}








void StoryEventClass::Reward_Finished_Tutorial()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FINISHED_TUTORIAL takes one parameter, name of the tutorial. \r\n");
		return;
	}

	Story_Debug_Printf("STORY REWARD - Finished Tutorial %s\r\n",RewardParam[0].c_str());
	PlayerProfile.Set_Has_Played_Tutorial(CRCClass::Calculate_CRC(RewardParam[0].c_str(),RewardParam[0].size()));
	PlayerProfile.Write_Current_Profile();
}





void StoryEventClass::Reward_Bombard_Override_Delay()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  BOMBARD_DELAY takes one parameter. \r\n");
		return;
	}

	float delay = (float)atof(RewardParam[0].c_str());
	Story_Debug_Printf("STORY REWARD - Setting planetary bombard delay %s\r\n",delay);

	PlanetaryBombardManagerClass *bomb_manager = GameModeManager.Get_Active_Mode()->Get_Land_Planetary_Bombard_Manager();
	if (bomb_manager)
	{
		bomb_manager->Set_Override_Delay(delay);
	}
}



void StoryEventClass::Reward_Enable_Bounty_Collection()
{
	Story_Debug_Printf("STORY REWARD - Enabling bounty collection\r\n");

	TheTutorial.Set_Enable_Bounty_Collection(true);
}


void StoryEventClass::Reward_Disable_Bounty_Collection()
{
	Story_Debug_Printf("STORY REWARD - Disabling bounty collection\r\n");

	TheTutorial.Set_Enable_Bounty_Collection(false);
}



void StoryEventClass::Reward_Remove_Corruption()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  REMOVE_CORRUPTION needs a planet name. \r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  REMOVE_CORRUPTION failed due to invalid unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  REMOVE_CORRUPTION failed due to non-existent unit %s.\r\n", RewardParam[0].c_str());
		return;
	}

	GameObjectClass *planet = planets->Get_At(0);
	if (!planet->Behaves_Like(BEHAVIOR_PLANET))
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  REMOVE_CORRUPTION failed due to %s not being a planet.\r\n", RewardParam[0].c_str());
		return;
	}

	float transition_time = 10.0f;
	if (!RewardParam[1].empty())
	{
		transition_time = (float)atof(RewardParam[1].c_str());
	}

	PlanetaryBehaviorClass *planet_behavior = (PlanetaryBehaviorClass *)planet->Get_Behavior(BEHAVIOR_PLANET);
	planet_behavior->Begin_Corruption_Removal(planet,NULL,transition_time);

	Story_Debug_Printf("STORY REWARD - Removing corruption on %s in %d seconds\r\n",planet_type->Get_Name()->c_str(),transition_time);
}




void StoryEventClass::Reward_Give_Black_Market()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  GIVE_BLACK_MARKET needs an item name. \r\n");
		return;
	}

	PlayerClass *player = SubPlot->Get_Local_Player();
	assert(player);

	for (int i=0; i<BlackMarketItemList.Get_Num_Items(); i++)
	{
		const BlackMarketItemClass *item = BlackMarketItemList.Get_Item(i);

		if (item->Get_Name() == RewardParam[0])
		{
			player->Add_Black_Market_Ability(item);
			Story_Debug_Printf("STORY REWARD - Unlocking Black Market item %s\r\n",RewardParam[0].c_str());
			return;
		}
	}
}





void StoryEventClass::Reward_Show_Special_Slot()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_SPECIAL_SLOT takes two parameters, a planet name and slot type \r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_SPECIAL_SLOT failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_SPECIAL_SLOT failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	ShowSlotType slot = (ShowSlotType)Lookup_Enum(RewardParam[1].c_str());
	if (slot == SHOW_NONE)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SHOW_SPECIAL_SLOT failed due to non-existent slot type %s.\r\n", RewardParam[1].c_str());
	}

	GameObjectClass *target = planets->Get_At(0);

	if (!RewardParam[2].empty())
	{
		if (atoi(RewardParam[2].c_str()) == 0)
		{
			target = NULL;
		}
	}

	switch (slot)
	{
		case SHOW_RAID_SLOT:
			TheTutorial.Set_Show_Raid_Planet(target);
			Story_Debug_Printf("STORY REWARD - Forcing raid slot to be shown at planet %s\r\n",RewardParam[0].c_str());
			break;

		case SHOW_SMUGGLE_SLOT:
			TheTutorial.Set_Show_Smuggle_Planet(target);
			Story_Debug_Printf("STORY REWARD - Forcing smuggle slot to be shown at planet %s\r\n",RewardParam[0].c_str());
			break;

		case SHOW_STEAL_SLOT:
			TheTutorial.Set_Show_Steal_Planet(target);
			Story_Debug_Printf("STORY REWARD - Forcing steal slot to be shown at planet %s\r\n",RewardParam[0].c_str());
			break;

		case SHOW_BLACK_MARKET_SLOT:
			TheTutorial.Set_Show_Black_Market_Planet(target);
			Story_Debug_Printf("STORY REWARD - Forcing black market slot to be shown at planet %s\r\n",RewardParam[0].c_str());
			break;

		case SHOW_SABOTAGE_SLOT:
			TheTutorial.Set_Show_Sabotage_Planet(target);
			Story_Debug_Printf("STORY REWARD - Forcing sabotage slot to be shown at planet %s\r\n",RewardParam[0].c_str());
			break;

		case SHOW_CORRUPTION_SLOT:
			TheTutorial.Set_Show_Corruption_Planet(target);
			Story_Debug_Printf("STORY REWARD - Forcing corruption slot to be shown at planet %s\r\n",RewardParam[0].c_str());
			break;

		case SHOW_REMOVE_CORRUPTION_SLOT:
			TheTutorial.Set_Show_Remove_Corruption_Planet(target);
			Story_Debug_Printf("STORY REWARD - Forcing remove corruption slot to be shown at planet %s\r\n",RewardParam[0].c_str());
			break;

		default:
			break;
	}
}





void StoryEventClass::Reward_Sabotage_Structure()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SABOTAGE_STRUCTURE takes two parameters, a planet name and building type \r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SABOTAGE_STRUCTURE failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SABOTAGE_STRUCTURE failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const GameObjectTypeClass *structure_type = GameObjectTypeManager.Find_Object_Type(RewardParam[1].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SABOTAGE_STRUCTURE failed due to invalid structure %s.\r\n", RewardParam[1].c_str());
		return;
	}

	GameObjectClass *planet = planets->Get_At(0);
	const PlanetaryDataPackClass::SpecialStructureVectorType &structures = planet->Get_Planetary_Data()->Get_Ground_Special_Structures();

	// Search through all the structures on the planet surface for the one we want
	GameObjectClass *target = NULL;
	for (int i=0; i<structures.Size(); i++)
	{
		GameObjectClass *curtarget = structures[i];
		if (curtarget->Get_Type() == structure_type)
		{
			target = curtarget;
			break;
		}
	}

	if (target)
	{
		// Not MP friendly
		GalacticSabotageAbilityClass::Perform_Galactic_Sabotage(target,0,0,-1,planet,true);
		Story_Debug_Printf("STORY REWARD - SABOTAGE_STRUCTURE success at planet %s.\r\n", RewardParam[0].c_str());
	}
	else
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SABOTAGE_STRUCTURE failed due to non-existent structure %s.\r\n", RewardParam[1].c_str());
	}
}




void StoryEventClass::Reward_Enable_Sabotage()
{
	bool enable = true;

	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
		{
			enable = false;
		}
	}

	TheTutorial.Set_Enable_Sabotage(enable);
	Story_Debug_Printf("STORY REWARD - ENABLE_SABOTAGE set to %s.\r\n", enable ? "TRUE" : "FALSE");
}





void StoryEventClass::Reward_Flash_Advanced_Map_Object()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FLASH_ADVANCED_MAP_OBJECT takes a parameter, the object name\r\n");
		return;
	}

	const GameObjectTypeClass *object_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (object_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  FLASH_ADVANCED_MAP_OBJECT failed due to invalid object %s.\r\n", RewardParam[0].c_str());
		return;
	}

	Story_Debug_Printf("STORY REWARD - FLASH_ADVANCED_MAP_OBJECT %s.\r\n", RewardParam[0].c_str());
	TheCommandBar.Flash_Advanced_Map_Object(object_type);
}





void StoryEventClass::Reward_Enable_Invasion()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  DISABLE_INVASION takes a parameter, the planet name\r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SABOTAGE_STRUCTURE failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  SABOTAGE_STRUCTURE failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	bool enable = true;
	if (!RewardParam[1].empty())
	{
		if (atoi(RewardParam[1].c_str()) == 0)
		{
			enable = false;
		}
	}

	Story_Debug_Printf("STORY REWARD - ENABLE_INVASION, Invasions %s at %s.\r\n", enable ? "ENABLED" : "DISABLED",RewardParam[0].c_str());
	TheTutorial.Enable_Invasion(planets->Get_At(0),enable);
}





void StoryEventClass::Reward_Restrict_All_Abilities()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_ALL_ABILITIES takes a parameter, the planet name\r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_ALL_ABILITIES failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_ALL_ABILITIES failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	bool restrict = true;
	if (!RewardParam[1].empty())
	{
		if (atoi(RewardParam[1].c_str()) == 0)
		{
			restrict = false;
		}
	}

	Story_Debug_Printf("STORY REWARD - RESTRICT_ALL_ABILITIES, Abilities %s at %s.\r\n", restrict ? "RESTRICTED":"ENABLED",RewardParam[0].c_str());
	TheTutorial.Set_Restrict_All_Abilities(planets->Get_At(0),restrict);
}




void StoryEventClass::Reward_Restrict_Corruption()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_CORRUPTION takes a parameter, the planet name\r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_CORRUPTION failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_CORRUPTION failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	bool restrict = true;
	if (!RewardParam[1].empty())
	{
		if (atoi(RewardParam[1].c_str()) == 0)
		{
			restrict = false;
		}
	}

	Story_Debug_Printf("STORY REWARD - RESTRICT_CORRUPTION, Corruption %s at %s.\r\n", restrict ? "RESTRICTED" : "ENABLED",RewardParam[0].c_str());
	TheTutorial.Set_Restrict_Corruption(planets->Get_At(0),restrict);
}





void StoryEventClass::Reward_Restrict_Black_Market()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_BLACK_MARKET takes a parameter, the planet name\r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_BLACK_MARKET failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_BLACK_MARKET failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	bool restrict = true;
	if (!RewardParam[1].empty())
	{
		if (atoi(RewardParam[1].c_str()) == 0)
		{
			restrict = false;
		}
	}

	Story_Debug_Printf("STORY REWARD - RESTRICT_BLACK_MARKET, Black market %s at %s.\r\n", restrict ? "RESTRICTED" : "ENABLED",RewardParam[0].c_str());
	TheTutorial.Set_Restrict_Black_Market(planets->Get_At(0),restrict);
}





void StoryEventClass::Reward_Restrict_Sabotage()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_SABOTAGE takes a parameter, the planet name\r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_SABOTAGE failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  RESTRICT_SABOTAGE failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	bool restrict = true;
	if (!RewardParam[1].empty())
	{
		if (atoi(RewardParam[1].c_str()) == 0)
		{
			restrict = false;
		}
	}

	Story_Debug_Printf("STORY REWARD - RESTRICT_SABOTAGE, Sabotage %s at %s.\r\n", restrict ? "RESTRICTED" : "ENABLED",RewardParam[0].c_str());
	TheTutorial.Set_Restrict_Sabotage(planets->Get_At(0),restrict);
}





void StoryEventClass::Reward_Enable_Fleet_Combine()
{
	bool enable = true;
	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
		{
			enable = false;
		}
	}

	Story_Debug_Printf("STORY REWARD - ENABLE_FLEET_COMBINE, Fleet combining %s.\r\n", enable ? "ENABLED" : "DISABLED");
	TheTutorial.Set_Enable_Fleet_Combine(enable);
}






void StoryEventClass::Reward_Enable_Combat_Cinematic()
{
	bool enable = true;
	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
		{
			enable = false;
		}
	}

	Story_Debug_Printf("STORY REWARD - ENABLE_COMBAT_CINEMATIC, Combat cinematic %s.\r\n", enable ? "ENABLED" : "DISABLED");
	TheTutorial.Set_Enable_Combat_Cinematic(enable);
}







void StoryEventClass::Reward_Restrict_Autoresolve()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  ENABLE_AUTORESOLVE takes a parameter, the planet name\r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  ENABLE_AUTORESOLVE failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  ENABLE_AUTORESOLVE failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	bool restrict = true;
	if (!RewardParam[1].empty())
	{
		if (atoi(RewardParam[1].c_str()) == 0)
		{
			restrict = false;
		}
	}

	Story_Debug_Printf("STORY REWARD - ENABLE_AUTORESOLVE, Autoresolve %s at %s.\r\n", restrict ? "DISABLED" : "ENABLED",RewardParam[0].c_str());
	TheTutorial.Set_Restrict_Autoresolve(planets->Get_At(0),restrict);
}


/**************************************************************************************************
* StoryEventClass::Reward_Enable_Galactic_Corruption_Hologram -- Turns the use of post-mission
*	corruption hologram sequences on/off for a specific planet.
*
* In:		param0 - Planet name
*			param1 - enable (true) / disable (false) flag.
*
* Out:	
*
*
* History: 08/08/2006 5:49PM AJA
**************************************************************************************************/
void StoryEventClass::Reward_Enable_Galactic_Corruption_Hologram()
{
	if (RewardParam[0].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  ENABLE_GALACTIC_CORRUPTION_HOLOGRAM takes the planet name as the first parameter.\r\n");
		return;
	}

	const GameObjectTypeClass *planet_type = GameObjectTypeManager.Find_Object_Type(RewardParam[0].c_str());
	if (planet_type == NULL)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  ENABLE_GALACTIC_CORRUPTION_HOLOGRAM failed due to invalid planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(planet_type);
	if (planets->Size() == 0)
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  ENABLE_GALACTIC_CORRUPTION_HOLOGRAM failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}
	GameObjectClass *planet = planets->Get_At(0);
	FAIL_IF( planet == NULL )
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  ENABLE_GALACTIC_CORRUPTION_HOLOGRAM failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}
	PlanetaryDataPackClass *planet_data = planet->Get_Planetary_Data();
	FAIL_IF( planet_data == NULL )
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  ENABLE_GALACTIC_CORRUPTION_HOLOGRAM failed due to non-existent planet %s.\r\n", RewardParam[0].c_str());
		return;
	}

	bool enable = true;
	if (!RewardParam[1].empty())
	{
		if (atoi(RewardParam[1].c_str()) == 0)
		{
			enable = false;
		}
	}

	planet_data->Allow_Galactic_Corruption_Holograms(enable);
	Story_Debug_Printf("STORY REWARD - ENABLE_GALACTIC_CORRUPTION_HOLOGRAM, Hologram %s at %s.\r\n", enable ? "ENABLED" : "DISABLED", RewardParam[0].c_str());
}







void StoryEventClass::Reward_Replace_Objective()
{
	if (RewardParam[0].empty() || RewardParam[1].empty())
	{
		Story_Debug_Printf("STORY REWARD - ERROR!  REPLACE_OBJECTIVE takes two parameters.\r\n");
		return;
	}

	Story_Debug_Printf("STORY REWARD - REPLACE_OBJECTIVE %s with %s.\r\n",RewardParam[0].c_str(),RewardParam[1].c_str());
	SubPlot->Get_Story_Mode()->Replace_Objective(RewardParam[0],RewardParam[1]);
}







void StoryEventClass::Reward_Enable_Campaign_Victory_Movie()
{
	bool enable = true;
	if (!RewardParam[0].empty())
	{
		if (atoi(RewardParam[0].c_str()) == 0)
		{
			enable = false;
		}
	}
	
	Story_Debug_Printf("STORY REWARD - Enable victory movies turned %d.\r\n",enable ? "ON" : "OFF");
	TheTutorial.Set_Enable_Victory_Movie(enable);
}




										  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// End of rewards
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



















































void StoryEventClass::Replace_Reward_Variable(const std::string &var_name, const std::string &new_name)
{
	if (RewardType != REWARD_PICK_PLANET)
	{
		char name[256];
		std::string param;

		for (int i=0; i<3; i++)
		{
			strcpy(name, RewardParam[i].c_str());
			_strupr(name);
			param = name;

			if (param == var_name)
			{
				RewardParam[i] = new_name;
				Story_Debug_Printf("EVENT %s, Param %d, replacing %s with %s\r\n",EventName.c_str(),i,var_name.c_str(),new_name.c_str());
			}
		}
	}                         
}







/**************************************************************************************************
* StoryEventClass::Load -- Load event data
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:54AM JAC
**************************************************************************************************/
bool StoryEventClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );
	bool ok = true;

	StoryDialogExtraText.clear();
	std::wstring tstr;
	void *extra_id = NULL;
	int reward_param_index = 0;

	while (reader->Open_Micro_Chunk())
	{
		switch ( reader->Cur_Micro_Chunk_ID() )
		{
			READ_MICRO_CHUNK_THIS_PTR(STORY_EVENT_THIS_ID_CHUNK);
			READ_MICRO_CHUNK(STORY_EVENT_TRIGGERED_CHUNK,Triggered);
			READ_MICRO_CHUNK(STORY_EVENT_ACTIVE_CHUNK,Active);
			READ_MICRO_CHUNK(STORY_EVENT_EXTRA_ID_CHUNK, extra_id);
			READ_MICRO_CHUNK_STRING(STORY_EVENT_DIALOG_NAME_CHUNK, StoryDialog);
			READ_MICRO_CHUNK(STORY_EVENT_REWARD_TYPE_CHUNK, RewardType);
			READ_MICRO_CHUNK(STORY_EVENT_TIMEOUT_START_CHUNK, StartTime);
			READ_MICRO_CHUNK(STORY_EVENT_DISABLED_CHUNK, Disabled);

		case STORY_EVENT_REWARD_PARAM_CHUNK:
			ok &= reader->Read_String(RewardParam[reward_param_index++]);
			break;

		case STORY_EVENT_EXTRA_STRING_CHUNK:
				ok &= reader->Read_String(tstr);
				StoryDialogExtraText.push_back(tstr);
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Micro_Chunk();
	}

	if (extra_id)
	{
		SaveLoadClass::Register_Pointer(extra_id, &StoryDialogExtraText);
	}

	return (ok);
}





/**************************************************************************************************
* StoryEventClass::Save -- Save event data
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:54AM JAC
**************************************************************************************************/
bool StoryEventClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	void *extra_id = &StoryDialogExtraText;

	WRITE_MICRO_CHUNK_THIS_PTR(STORY_EVENT_THIS_ID_CHUNK);
	WRITE_MICRO_CHUNK(STORY_EVENT_TRIGGERED_CHUNK,Triggered);
	WRITE_MICRO_CHUNK(STORY_EVENT_ACTIVE_CHUNK,Active);
	WRITE_MICRO_CHUNK(STORY_EVENT_EXTRA_ID_CHUNK, extra_id);
	WRITE_MICRO_CHUNK(STORY_EVENT_DISABLED_CHUNK, Disabled);
	
	for (int i = 0; i < (int)StoryDialogExtraText.size(); i++)
	{
		WRITE_MICRO_CHUNK_STRING(STORY_EVENT_EXTRA_STRING_CHUNK, StoryDialogExtraText[i]);
	}
	SaveLoadClass::Set_Object_As_Saved(&StoryDialogExtraText);

	for (int i = 0; i < ARRAY_SIZE(RewardParam); ++i)
	{
		WRITE_MICRO_CHUNK_STRING(STORY_EVENT_REWARD_PARAM_CHUNK, RewardParam[i]);
	}

	WRITE_MICRO_CHUNK_STRING(STORY_EVENT_DIALOG_NAME_CHUNK, StoryDialog);
	WRITE_MICRO_CHUNK(STORY_EVENT_REWARD_TYPE_CHUNK, RewardType);
	WRITE_MICRO_CHUNK(STORY_EVENT_TIMEOUT_START_CHUNK, StartTime);

	return (ok);
}




void StoryEventClass::Dump_Status()
{
//	Story_Debug_Printf("\t\t%s, Active %s, Triggered %s, Disabled %s\r\n",Get_Name()->c_str(),Is_Active()?"Y":"N",Is_Triggered()?"Y":"N",Is_Disabled()?"Y":"N");
	int enum_remap_index;
	for (enum_remap_index = 0; enum_remap_index < STORY_COUNT; enum_remap_index++)
	{
		if (EnumRemap[enum_remap_index].EnumValue == EventType)
			break;
	}

	float starttime = GameModeManager.Get_Frame_Timer() * FrameSynchronizer.Get_Inv_Logical_FPS();
	float elapsed = starttime - StartTime;
	if (!Active || (starttime == -1) || Triggered)
	{
		Story_Debug_Printf("\t\t%s, type: %s\r\n",Get_Name()->c_str(),EnumRemap[enum_remap_index].Text);
	}
	else
	{
		Story_Debug_Printf("\t\t%s, type: %s, active time: %f\r\n",Get_Name()->c_str(),EnumRemap[enum_remap_index].Text,elapsed);
	}


	for (int i=0; i<Prereqs.Size(); i++)
	{
		Story_Debug_Printf("\t\t\t");

		DynamicVectorClass<StoryEventClass *> *andlist = &Prereqs[i];

		for (int j=0; j<andlist->Size(); j++)
		{
			if ((*andlist)[j]->Is_Triggered())
			{
				Story_Debug_Printf("(%s) ",(*andlist)[j]->Get_Name()->c_str());
			}
			else
			{
				Story_Debug_Printf("%s ",(*andlist)[j]->Get_Name()->c_str());
			}
		}
		Story_Debug_Printf("\r\n");
	}

	if (TimeoutTime != -1)
	{
		Story_Debug_Printf("\t\t\ttimeout: %f\r\n",TimeoutTime);
	}

	Story_Debug_Printf("\r\n");
}







void StoryEventFogRevealClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		if (EventType == STORY_FOG_POSITION_REVEAL)
		{
			assert(param->size() == 3);
			Position.x = (float)atof((*param)[0].c_str());
			Position.y = (float)atof((*param)[1].c_str());
			Position.z = (float)atof((*param)[2].c_str());
		}
		else
		{
			char name[256];
			ObjectName.resize(0);

			if ((*param)[0].size())
			{
				strcpy( name, (*param)[0].c_str() );
				//_strupr( name );
				ObjectName = name;

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("STORY MODE ERROR!  Event %s, invalid object name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
			}
		}
	}
}







void StoryEventFogRevealClass::Evaluate_Event(void *, void *)
{
	GameModeClass *gamemode = GameModeManager.Get_Active_Mode();
	assert(gamemode);
	bool fogged = true;

	if (EventType == STORY_FOG_POSITION_REVEAL)
	{
		fogged = gamemode->Is_Fogged(SubPlot->Get_Local_Player()->Get_ID(),Position);
		if (!fogged)
		{
			Story_Debug_Printf("STORY EVENT - Position (%f,%f,%f) unfogged\r\n",Position.x,Position.y,Position.z);
			Event_Triggered();
		}
	}
	else
	{
		const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(ObjectName.c_str());
		if (type)
		{
			const DynamicVectorClass<GameObjectClass *> *object_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);

			for (int i=0; i<object_list->Size(); i++)
			{
				GameObjectClass *object = (*object_list)[i];
				fogged = gamemode->Is_Fogged(SubPlot->Get_Local_Player()->Get_ID(),object);
				if (!fogged)
				{
					Story_Debug_Printf("STORY EVENT - Object %s unfogged\r\n",ObjectName.c_str());
					Event_Triggered();
				}
			}
		}
	}
}













void StoryEventRetreatClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		Filter = (StoryEventFilter)Lookup_Enum((*param)[0].c_str());
	}
}







void StoryEventRetreatClass::Evaluate_Event(void *param1, void *)
{
	if (param1 == NULL) 
	{
		return;
	}

	PlayerClass *player = (PlayerClass *)param1;

	// Not all combinations currently supported.  Design only calls for enemy retreat events.
	switch (Filter)
	{
		case EVENT_FILTER_FRIENDLY_ONLY:
			if (player == SubPlot->Get_Local_Player())
			{
				Event_Triggered(NULL);
			}
			break;

		case EVENT_FILTER_ENEMY_ONLY:
			if (player->Is_Enemy(SubPlot->Get_Local_Player()))
			{
				Event_Triggered(NULL);
			}
			break;

		default:
			break;
	}
}





/**************************************************************************************************
* StoryEventStartTacticalClass::StoryEventStartTacticalClass -- Start of tactical combat event constructor
*
* In:		
*
* Out:	
*
*
* History: 11/23/2004 2:44PM JAC
**************************************************************************************************/
StoryEventStartTacticalClass::StoryEventStartTacticalClass()
{
}




/**************************************************************************************************
* StoryEventStartTacticalClass::Shutdown -- Clear out data
*
* In:		
*
* Out:	
*
*
* History: 11/23/2004 2:44PM JAC
**************************************************************************************************/
void StoryEventStartTacticalClass::Shutdown()
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		Planet[i].clear();
	}

	Planet.clear();
}




/**************************************************************************************************
* StoryEventStartTacticalClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 11/23/2004 2:45PM JAC
**************************************************************************************************/
void StoryEventStartTacticalClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		assert(param->size() >= 1);
		ScriptName = (*param)[0];
		assert(!ScriptName.empty());
	}
	else if (index == 1)
	{
		char name[ 256 ];
		Planet.resize(0);

		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string planet_name(name);
				Planet.push_back(planet_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid planet name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
			}
		}
	}
}




/**************************************************************************************************
* StoryEventStartTacticalClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 11/23/2004 2:45PM JAC
**************************************************************************************************/
void StoryEventStartTacticalClass::Evaluate_Event(void *param1, void *param2)
{
	if (param1 == NULL) 
	{
		return;
	}

	GameObjectClass *planet = (GameObjectClass *)param2;
	StoryModeClass *story = (StoryModeClass *)param1;

	if (planet == NULL || Planet.size() == 0)
	{
		Event_Triggered(NULL);
		#ifdef STORY_LOGGING
			Story_Debug_Printf("STORY - Loading tactical story %s\r\n",ScriptName.c_str());
		#endif
		if (!ScriptName.empty())
		{
			story->Load_Plots(ScriptName,SubPlot->Get_Local_Player());
		}
	}
	else
	{
		for (unsigned int i=0; i<Planet.size(); i++)
		{
			if (Planet[i] == *planet->Get_Type()->Get_Name())
			{
	  			Event_Triggered(planet);
				#ifdef STORY_LOGGING
					Story_Debug_Printf("STORY - Loading tactical story %s\r\n",ScriptName.c_str());
				#endif
				if (!ScriptName.empty())
				{
					story->Load_Plots(ScriptName,SubPlot->Get_Local_Player());
				}
			}
		}
	}
}





void StoryEventStartTacticalClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == var_name)
		{
			Planet[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}






void StoryEventStartTacticalClass::Planet_Destroyed(const std::string &planet_name)
{
	std::vector<std::string>::iterator nameptr;

	for (nameptr = Planet.begin(); nameptr != Planet.end(); nameptr++)
	{
		std::string name = *nameptr;

		if (name == planet_name)
		{
			Story_Debug_Printf("EVENT %s, removing planet %s\r\n",planet_name.c_str());
			Planet.erase(nameptr);
			if (Planet.empty())
			{
				Disabled = true;
				return;
			}
		}
	}
}







void StoryEventStringClass::Shutdown()
{
	for (unsigned int i=0; i<Strings.size(); i++)
	{
		Strings[i].clear();
	}

	Strings.clear();
}





void StoryEventStringClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		char name[ 256 ];
		Strings.resize(0);

		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string new_text(name);
				Strings.push_back(new_text);
			}
		}
	}
}





void StoryEventStringClass::Evaluate_Event(void *param1, void *)
{
	if (param1 == NULL) 
	{
		Story_Debug_Printf("STORY EVENT STRING - missing parameter 1: string to trigger on.\r\n");
		return;
	}

	const char *text = (const char *)param1;
	char new_text[256];
	strcpy( new_text, text );
	_strupr( new_text );
	std::string test_string(new_text);

	for (unsigned int i=0; i<Strings.size(); i++)
	{
		if (Strings[i] == test_string)
		{
			Event_Triggered(NULL);
		}
	}
}








void StoryEventStringClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Strings.size(); i++)
	{
		if (Strings[i] == var_name)
		{
			Strings[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}









/**************************************************************************************************
* StoryEventEnterClass::StoryEventEnterClass -- Enter event constructor
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:37PM JAC
**************************************************************************************************/
StoryEventEnterClass::StoryEventEnterClass() :
	Filter(EVENT_FILTER_NONE),
	AllowStealth(false),
	ExclusiveEnter(false),
	RaidEnter(false)
{
}




/**************************************************************************************************
* StoryEventEnterClass::Shutdown -- Clear out data
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:55AM JAC
**************************************************************************************************/
void StoryEventEnterClass::Shutdown()
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		Planet[i].clear();
	}

	Planet.clear();
}

/**************************************************************************************************
* StoryEventEnterClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:38PM JAC
**************************************************************************************************/
void StoryEventEnterClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		char name[ 256 ];
		Planet.resize(0);

		//Story_Debug_Printf("Enter param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string planet_name(name);
				Planet.push_back(planet_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid planet name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
	else if (index == 1)
	{
		Filter = (StoryEventFilter)Lookup_Enum((*param)[0].c_str());
		//Story_Debug_Printf("Enter param 2 - %s\r\n",(*param)[0].c_str());
	}
	else if (index == 2)
	{
		char name[ 256 ];
		EnteringShip.resize(0);

		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string ship_name(name);
				EnteringShip.push_back(ship_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid ship name %s\r\n",EventName.c_str(),name);
				}
			}
		}
	}
	else if (index == 3)
	{
		char name[ 256 ];
		OrbitingShip.resize(0);

		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string ship_name(name);
				OrbitingShip.push_back(ship_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid ship name %s\r\n",EventName.c_str(),name);
				}
			}
		}
	}
	else if (index == 4)
	{
		FAIL_IF(param->size() != 1) { return; }
		AllowStealth = (atoi(param->at(0).c_str()) != 0);
	}
	else if (index == 5)
	{
		FAIL_IF(param->size() < 1) { return; }
		ExclusiveEnter = (atoi(param->at(0).c_str()) != 0);
	}
	else if (index == 6)
	{
		FAIL_IF(param->size() < 1) { return; }
		RaidEnter = (atoi(param->at(0).c_str()) != 0);
	}
}




/**************************************************************************************************
* StoryEventEnterClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:38PM JAC
**************************************************************************************************/
void StoryEventEnterClass::Evaluate_Event(void *param1, void *param2)
{
	if (param1 == NULL) 
	{
		return;
	}

	GameObjectClass *planet = (GameObjectClass *)param1;
	GameObjectClass *fleet = (GameObjectClass *)param2;

	// If no planet is specified in the script, assume any planet will do
	if (Planet.empty())
	{
		Story_Debug_Printf("STORY EVENT - No planet specified in script.  Assuming any planet will trigger event\r\n");
		if (AllowStealth || !fleet || !fleet->Is_Stealth_Object())
		{
			Event_Triggered(planet,false);
		}
		return;
	}

	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == *planet->Get_Type()->Get_Name())
		{
			if ((fleet == NULL) || Event_Filter_Matches(fleet,Filter))
			{
				if (Check_Fleet_Contents(fleet) && Check_Orbit_Contents(planet))
				{
					Event_Triggered(planet,false);
				}
			}
		}
	}
}






void StoryEventEnterClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == var_name)
		{
			Planet[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}






void StoryEventEnterClass::Planet_Destroyed(const std::string &planet_name)
{
	std::vector<std::string>::iterator nameptr;

	for (nameptr = Planet.begin(); nameptr != Planet.end(); nameptr++)
	{
		std::string name = *nameptr;

		if (name == planet_name)
		{
			Story_Debug_Printf("EVENT %s, removing planet %s\r\n",planet_name.c_str());
			Planet.erase(nameptr);
			if (Planet.empty())
			{
				Disabled = true;
				return;
			}
		}
	}
}





bool StoryEventEnterClass::Check_Special_Land_Tactical_Map(GameObjectClass *, GameObjectClass *planet)
{
	// Only true if the reward is a linked tactical map
	if (RewardType != REWARD_LINK_TACTICAL)
	{
		return (false);
	}

	// Check to see if this is the right planet to trigger the event
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == *planet->Get_Type()->Get_Name())
		{
			return (true);
		}
	}

	return (false);
}



bool StoryEventEnterClass::Check_Planet_Entry_Restrictions(GameObjectClass *fleet, GameObjectClass *planet)
{
	FAIL_IF(!fleet) { return false; }
	FAIL_IF(!planet) { return false; }

	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == *planet->Get_Type()->Get_Name())
		{
			if ((fleet == NULL) || Event_Filter_Matches(fleet,Filter))
			{
				if (Check_Fleet_Contents(fleet) && Check_Orbit_Contents(planet))
				{
					return true;
				}
			}
		}
	}

	return false;
}



bool StoryEventEnterClass::Check_Special_Space_Tactical_Map(GameObjectClass *hero, GameObjectClass *planet)
{
	if (planet == NULL)
	{
		return (false);
	}

	bool hero_found = false;
	if (hero == NULL)
	{
		hero_found = true;
	}
	else
	{
		const GameObjectTypeClass *hero_type = hero->Get_Original_Object_Type();
		assert(hero_type);
		std::string hero_name = *hero_type->Get_Name();
		for (unsigned int i=0; i<EnteringShip.size(); i++)
		{
			if (EnteringShip[i] == hero_name)
			{
				hero_found = true;
				break;
			}
		}
	}

	if (hero_found == false)
	{
		return (false);
	}

	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == *planet->Get_Type()->Get_Name())
		{
			return true;
		}
	}

	return (false);
}






bool StoryEventEnterClass::Check_Fleet_Contents(GameObjectClass *fleet)
{
	if ((fleet == NULL))
	{
		return (true);
	}

	if (EnteringShip.empty() && !RaidEnter)
	{
		return AllowStealth || !fleet->Is_Stealth_Object();
	}

	DynamicVectorClass<GameObjectClass*> fleet_objects;

	if (RaidEnter)
	{
		FleetBehaviorClass *fleet_behavior = (FleetBehaviorClass*)fleet->Get_Behavior(BEHAVIOR_FLEET);
		if (fleet_behavior->Is_Raid_Capable())
		{
			return (true);
		}
	}
	else
	{
		// Return true if all the specified ships are within the fleet
		fleet->Get_Fleet_Breakdown(fleet_objects);
		for (unsigned int i = 0; i < EnteringShip.size(); ++i)
		{
			bool is_in_fleet = false;
			const GameObjectTypeClass *required_type = GameObjectTypeManager.Find_Object_Type(EnteringShip[i]);
			for (int j = 0; j < fleet_objects.Size(); ++j)
			{
				if (required_type == fleet_objects[j]->Get_Original_Object_Type())
				{
					is_in_fleet = true;
					break;
				}
			}

			if (!is_in_fleet)
			{
				return false;
			}
		}

		if (ExclusiveEnter)
		{
			fleet_objects.Truncate();
			fleet->Get_Fleet_Breakdown(fleet_objects, false);
			return fleet_objects.Size() == static_cast<int>(EnteringShip.size());
		}
		else
		{
			return true;
		}
	}

	return (false);
}





bool StoryEventEnterClass::Check_Orbit_Contents(GameObjectClass *planet)
{
	if ((planet == NULL) || OrbitingShip.empty())
	{
		return (true);
	}

	DynamicVectorClass<GameObjectClass*> fleets;
	DynamicVectorClass<GameObjectClass*> fleet_objects;
	PlanetaryBehaviorClass *pbehavior = static_cast<PlanetaryBehaviorClass *> (planet->Get_Behavior( BEHAVIOR_PLANET ));

	for (int i=FLEET_ORBIT_GUARDING_PLANET_POSITION_FIRST; i<=FLEET_ORBIT_GUARDING_PLANET_POSITION_LAST; i++)
	{
		pbehavior->Get_Orbiting_Fleets_At_Orbit_Position(planet,(FleetOrbitPositionType)i,fleets);
	}

	// Search all the fleets around the planet for the desired ship
	for (int k=0; k<fleets.Size(); k++)
	{
		GameObjectClass *fleet = fleets[k];

		fleet_objects.Truncate();
		fleet->Get_Fleet_Breakdown(fleet_objects);

		for (int i=0; i<fleet_objects.Size(); i++)
		{
			GameObjectClass *ship = fleet_objects[i];
			for (unsigned int j=0; j<OrbitingShip.size(); j++)
			{
				if (OrbitingShip[j] == *ship->Get_Original_Object_Type()->Get_Name())
				{
					return (true);
				}
			}
		}
	}

	return (false);
}





bool StoryEventEnterClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(STORY_EVENT_BASE_CLASS_CHUNK);
		ok &= StoryEventClass::Save(writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(STORY_EVENT_DATA_CHUNK);

	for (unsigned int i = 0; i < Planet.size(); ++i)
	{
		WRITE_MICRO_CHUNK_STRING(STORY_EVENT_OBJECT_NAME_CHUNK, Planet[i]);
	}

	WRITE_MICRO_CHUNK(STORY_EVENT_FILTER_CHUNK, Filter);
	WRITE_MICRO_CHUNK(STORY_EVENT_ALLOW_STEALTH_CHUNK, AllowStealth);

	ok &= writer->End_Chunk();

	return (ok);	
}




bool StoryEventEnterClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	bool ok = true;
	Planet.resize(0);
	std::string str;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
			case STORY_EVENT_BASE_CLASS_CHUNK:
				ok &= StoryEventClass::Load(reader);
				break;

			case STORY_EVENT_DATA_CHUNK:
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK(STORY_EVENT_FILTER_CHUNK, Filter);
						READ_MICRO_CHUNK(STORY_EVENT_ALLOW_STEALTH_CHUNK, AllowStealth);

						case STORY_EVENT_OBJECT_NAME_CHUNK:
							ok &= reader->Read_String(str);
							Planet.push_back(str);
							break;

						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return( ok );
}





/**************************************************************************************************
* StoryEventSingleObjectNameClass::Shutdown -- Clear out data
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:55AM JAC
**************************************************************************************************/
void StoryEventSingleObjectNameClass::Shutdown()
{
	for (unsigned int i=0; i<ObjectName.size(); i++)
	{
		ObjectName[i].clear();
	}

	ObjectName.clear();
}



/**************************************************************************************************
* StoryEventSingleObjectNameClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:38PM JAC
**************************************************************************************************/
void StoryEventSingleObjectNameClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		char name[ 256 ];
		ObjectName.resize(0);

		//Story_Debug_Printf("Enter param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string planet_name(name);
				ObjectName.push_back(planet_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if ((type == NULL) && (EventType != STORY_ATTACK_HARDPOINT))
				{
					Story_Debug_Printf("Warning!  Story Mode - Event %s, invalid object name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}

		for (unsigned int i=0; i<param->size(); i++)
		{
			Count.push_back(1);
			CountCopy.push_back(1);
		}

		//Story_Debug_Printf("\r\n");
	}
	else if (index == 1)
	{
		for (unsigned int i=0; i<param->size(); i++)
		{
			int count = atoi((*param)[0].c_str());
			Count[i] = count;
			CountCopy[i] = count;
		}
	}
	else if (index == 2)
	{
		Filter = (StoryEventFilter)Lookup_Enum((*param)[0].c_str());
		//Story_Debug_Printf("Enter param 2 - %s\r\n",(*param)[0].c_str());
	}
}





/**************************************************************************************************
* StoryEventSingleObjectNameClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:39PM JAC
**************************************************************************************************/
void StoryEventSingleObjectNameClass::Evaluate_Event(void *param1, void *param2)
{
	if (param1 == NULL)
	{
		return;
	}

	std::string *objname = (std::string *)param1;
	GameObjectClass *planet = (GameObjectClass *)param2;

	for (unsigned int i=0; i<ObjectName.size(); i++)
	{
		if (ObjectName[i] == *objname)
		{
			if (planet == NULL)
			{
				Event_Triggered(planet);
			}
			else
			{
				if (Event_Filter_Matches(planet,Filter))
				{
					int curcount = --Count[i];

					if (curcount <= 0)
					{
						Event_Triggered(planet);
					}

					return;
				}
				else
				{
					Story_Debug_Printf("Event didn't trigger because filter did not match\r\n");
				}
			}
		}
	}
}







void StoryEventSingleObjectNameClass::Reset()
{
	StoryEventClass::Reset();
	for (unsigned int i=0; i<CountCopy.size(); i++)
	{
		Count[i] = CountCopy[i];
	}
}







void StoryEventSingleObjectNameClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<ObjectName.size(); i++)
	{
		if (ObjectName[i] == var_name)
		{
			ObjectName[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}







/**************************************************************************************************
* StoryEventSingleObjectNameClass::Load -- Custom load for event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:57AM JAC
**************************************************************************************************/
bool StoryEventSingleObjectNameClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	bool ok = true;
	ObjectName.resize(0);
	Count.resize(0);
	CountCopy.resize(0);
	std::string str;
	int count = 0;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
			case STORY_EVENT_BASE_CLASS_CHUNK:
				ok &= StoryEventClass::Load(reader);
				break;

			case STORY_EVENT_DATA_CHUNK:
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK(STORY_EVENT_FILTER_CHUNK, Filter);
						
						case STORY_EVENT_COUNT_CHUNK:
							ok &= reader->Read(&count, sizeof(count));
							Count.push_back(count);
							break;

						case STORY_EVENT_COUNT_COPY_CHUNK:
							ok &= reader->Read(&count, sizeof(count));
							CountCopy.push_back(count);
							break;

						case STORY_EVENT_OBJECT_NAME_CHUNK:
							ok &= reader->Read_String(str);
							ObjectName.push_back(str);
							break;

						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return( ok );
}





/**************************************************************************************************
* StoryEventSIngleObjectNameClass::Save -- Custom save for event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:57AM JAC
**************************************************************************************************/
bool StoryEventSingleObjectNameClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(STORY_EVENT_BASE_CLASS_CHUNK);
		ok &= StoryEventClass::Save(writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(STORY_EVENT_DATA_CHUNK);
	for (unsigned int i=0; i<Count.size(); i++)
	{
		WRITE_MICRO_CHUNK(STORY_EVENT_COUNT_CHUNK,Count[i]);
	}

	for (unsigned int i = 0; i < CountCopy.size(); ++i)
	{
		WRITE_MICRO_CHUNK(STORY_EVENT_COUNT_COPY_CHUNK, CountCopy[i]);
	}

	for (unsigned int i = 0; i < ObjectName.size(); ++i)
	{
		WRITE_MICRO_CHUNK_STRING(STORY_EVENT_OBJECT_NAME_CHUNK, ObjectName[i]);
	}

	WRITE_MICRO_CHUNK(STORY_EVENT_FILTER_CHUNK, Filter);

	ok &= writer->End_Chunk();

	return (ok);
}












/**************************************************************************************************
* StoryEventConstructLevelClass::Shutdown -- Clear out data
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:55AM JAC
**************************************************************************************************/
void StoryEventConstructLevelClass::Shutdown()
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		Planet[i].clear();
	}

	Planet.clear();
}


/**************************************************************************************************
* StoryEventConstructLevelClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:39PM JAC
**************************************************************************************************/
void StoryEventConstructLevelClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		char name[ 256 ];
		Planet.resize(0);

		//Story_Debug_Printf("Story construct level param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string planet_name(name);
				Planet.push_back(planet_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid planet name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
	else if (index == 1)
	{
		Level = atoi((*param)[0].c_str());
		//Story_Debug_Printf("Story construct param 2 - %d (%s)\r\n",Level,(*param)[0].c_str());
	}
	else if (index == 2)
	{
		Filter = (StoryBaseFilter)Lookup_Enum((*param)[0].c_str());
		//Story_Debug_Printf("Story construct param 3 - %s\r\n",(*param)[0].c_str());
	}
}





/**************************************************************************************************
* StoryEventConstructLevelClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:39PM JAC
**************************************************************************************************/
void StoryEventConstructLevelClass::Evaluate_Event(void *param1, void *param2)
{
	if (param1 == NULL)
	{
		return;
	}

	GameObjectClass *planet = (GameObjectClass *)param1;
	const std::string *planetname = planet->Get_Type()->Get_Name();
	const GameObjectTypeClass *type = (const GameObjectTypeClass *)param2;

	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == *planetname)
		{
			switch (Filter)
			{
				case BASE_GROUND:
					if (type->Behaves_Like( BEHAVIOR_DUMMY_GROUND_BASE ) && (type->Get_Base_Level() >= Level))
					{
						Event_Triggered(planet);
					}
					break;

				case BASE_SPACE:
					if (type->Behaves_Like( BEHAVIOR_DUMMY_STAR_BASE ) && (type->Get_Base_Level() >= Level))
					{
						Event_Triggered(planet);
					}
					break;

				default:
					if (type->Get_Base_Level() >= Level)
					{
						Event_Triggered(planet);
					}
					break;
			}
		}
	}
}






void StoryEventConstructLevelClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == var_name)
		{
			Planet[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}






void StoryEventConstructLevelClass::Planet_Destroyed(const std::string &planet_name)
{
	std::vector<std::string>::iterator nameptr;

	for (nameptr = Planet.begin(); nameptr != Planet.end(); nameptr++)
	{
		std::string name = *nameptr;

		if (name == planet_name)
		{
			Story_Debug_Printf("EVENT %s, removing planet %s\r\n",planet_name.c_str());
			Planet.erase(nameptr);
			if (Planet.empty())
			{
				Disabled = true;
				return;
			}
		}
	}
}





/**************************************************************************************************
* StoryEventConstructLevelClass::Load -- Custom load for event
*
* In:		
*
* Out:	
*
*
* History: 6/20/2005 6:07PM JSY
**************************************************************************************************/
bool StoryEventConstructLevelClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	bool ok = true;
	Planet.resize(0);
	std::string str;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
			case STORY_EVENT_BASE_CLASS_CHUNK:
				ok &= StoryEventClass::Load(reader);
				break;

			case STORY_EVENT_DATA_CHUNK:
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK(STORY_EVENT_FILTER_CHUNK, Filter);
						READ_MICRO_CHUNK(STORY_EVENT_LEVEL_CHUNK, Level);
				
						case STORY_EVENT_OBJECT_NAME_CHUNK:
							ok &= reader->Read_String(str);
							Planet.push_back(str);
							break;

						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return( ok );
}





/**************************************************************************************************
* StoryEventConstructLevelClass::Save -- Custom save for event
*
* In:		
*
* Out:	
*
*
* History: 6/20/2005 6:07PM JSY
**************************************************************************************************/
bool StoryEventConstructLevelClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(STORY_EVENT_BASE_CLASS_CHUNK);
		ok &= StoryEventClass::Save(writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(STORY_EVENT_DATA_CHUNK);

	for (unsigned int i = 0; i < Planet.size(); ++i)
	{
		WRITE_MICRO_CHUNK_STRING(STORY_EVENT_OBJECT_NAME_CHUNK, Planet[i]);
	}

	WRITE_MICRO_CHUNK(STORY_EVENT_FILTER_CHUNK, Filter);
	WRITE_MICRO_CHUNK(STORY_EVENT_LEVEL_CHUNK, Level);

	ok &= writer->End_Chunk();

	return (ok);
}





/**************************************************************************************************
* StoryEventCorruptionLevelClass::Shutdown -- Clear out data
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:55AM JAC
**************************************************************************************************/
void StoryEventCorruptionLevelClass::Shutdown()
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		Planet[i].clear();
	}

	Planet.clear();
}



/**************************************************************************************************
* StoryEventCorruptionLevelClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:39PM JAC
**************************************************************************************************/
void StoryEventCorruptionLevelClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		char name[ 256 ];
		Planet.resize(0);

		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string planet_name(name);
				Planet.push_back(planet_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid planet name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
			}
		}
	}
	else if (index == 1)
	{
		CorruptionType = (CorruptionTypeEnum)Lookup_Enum((*param)[0].c_str());
	}
}





/**************************************************************************************************
* StoryEventCorruptionLevelClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:40PM JAC
**************************************************************************************************/
void StoryEventCorruptionLevelClass::Evaluate_Event(void *param1, void *param2)
{
	if (param1 == NULL)
	{
		return;
	}

	GameObjectClass *planet = (GameObjectClass *)param1;
	assert(planet);
	const std::string *planetname = planet->Get_Type()->Get_Name();
	CorruptionTypeEnum corruption_type = *(CorruptionTypeEnum *)param2;

	if ((corruption_type == CorruptionType) || ((CorruptionType == CORRUPTION_ANY) && (corruption_type != CORRUPTION_NONE)))
	{
		for (unsigned int i=0; i<Planet.size(); i++)
		{
			if (Planet[i] == *planetname)
			{
				Event_Triggered(planet);
				return;
			}
		}
	}
}







void StoryEventCorruptionLevelClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == var_name)
		{
			Planet[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}













/**************************************************************************************************
* StoryEventHeroMoveClass::Shutdown -- Clear out data
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:56AM JAC
**************************************************************************************************/
void StoryEventHeroMoveClass::Shutdown()
{
	for (unsigned int i=0; i<Hero.size(); i++)
	{
		Hero[i].clear();
	}

	Hero.clear();

	for (unsigned int i=0; i<Planet.size(); i++)
	{
		Planet[i].clear();
	}

	Planet.clear();
}



/**************************************************************************************************
* StoryEventHeroMoveClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:40PM JAC
**************************************************************************************************/
void StoryEventHeroMoveClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		char name[ 256 ];
		Hero.resize(0);

		//Story_Debug_Printf("Story political control param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string hero_name(name);
				Hero.push_back(hero_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid hero name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
	else if (index == 1)
	{
		char name[ 256 ];
		Planet.resize(0);

		//Story_Debug_Printf("Story political control param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string planet_name(name);
				Planet.push_back(planet_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid planet name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
}





/**************************************************************************************************
* StoryEventHeroMoveClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:40PM JAC
**************************************************************************************************/
void StoryEventHeroMoveClass::Evaluate_Event(void *param1, void *param2)
{
	if ((param1 == NULL) || (param2 == NULL)) 
	{
		return;
	}

	std::string *heroname = (std::string *)param1;
	GameObjectClass *planet = (GameObjectClass *)param2;
	const std::string *planetname = planet->Get_Type()->Get_Name();

	bool herofound = false;

	for (unsigned int i=0; i<Hero.size(); i++)
	{
		if (Hero[i] == *heroname)
		{
			herofound = true;
			break;
		}
	}

	if (herofound)
	{
		for (unsigned int i=0; i<Planet.size(); i++)
		{
			if (Planet[i] == *planetname)
			{
				Event_Triggered(planet);
				return;
			}
		}
	}
}





bool StoryEventHeroMoveClass::Check_Special_Land_Tactical_Map(GameObjectClass *hero, GameObjectClass *planet)
{
	// Only true if the reward is a linked tactical map
	if (RewardType != REWARD_LINK_TACTICAL)
	{
		return (false);
	}

	bool herofound = false;

	if (hero == NULL)
	{
		// Certain times we just want to see if there's an event that will spawn a linked tactical 
		herofound = true;
	}
	else
	{
		const std::string *heroname = hero->Get_Original_Object_Type()->Get_Name();

		// Make sure that the hero we have will trigger this event
		for (unsigned int i=0; i<Hero.size(); i++)
		{
			if (Hero[i] == *heroname)
			{
				herofound = true;
				break;
			}
		}
	}

	if (herofound)
	{
		// Check to see if this is the right planet to trigger the event
		for (unsigned int i=0; i<Planet.size(); i++)
		{
			if (Planet[i] == *planet->Get_Type()->Get_Name())
			{
				return (true);
			}
		}
	}

	return (false);
}



bool StoryEventHeroMoveClass::Check_Planet_Entry_Restrictions(GameObjectClass *fleet, GameObjectClass *planet)
{
	FAIL_IF(!fleet) { return false; }
	FAIL_IF(!planet) { return false; }

	bool hero_found = false;
	for (unsigned int i=0; i<Hero.size(); i++)
	{
		const GameObjectTypeClass *hero_type = GameObjectTypeManager.Find_Object_Type(Hero[i]);
		if (hero_type && fleet->Contains_Object_Type(hero_type))
		{
			hero_found = true;
			break;
		}
	}
	
	if (hero_found)
	{
		for (unsigned int i=0; i<Planet.size(); i++)
		{
			if (Planet[i] == *planet->Get_Type()->Get_Name())
			{
				return true;
			}
		}
	}

	return false;
}



bool StoryEventHeroMoveClass::Check_Special_Space_Tactical_Map(GameObjectClass *hero_move, GameObjectClass *planet)
{
	if (planet == NULL)
	{
		return (false);
	}

	bool hero_found = false;
	if (hero_move == NULL)
	{
		hero_found = true;
	}
	else
	{
		const GameObjectTypeClass *hero_type = hero_move->Get_Original_Object_Type();
		assert(hero_type);
		std::string hero_name = *hero_type->Get_Name();
		for (unsigned int i=0; i<Hero.size(); i++)
		{
			if (Hero[i] == hero_name)
			{
				hero_found = true;
				break;
			}
		}
	}

	if (hero_found == false)
	{
		return (false);
	}

	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == *planet->Get_Type()->Get_Name())
		{
			return true;
		}
	}

	return (false);
}







void StoryEventHeroMoveClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == var_name)
		{
			Planet[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	for (unsigned int i=0; i<Hero.size(); i++)
	{
		if (Hero[i] == var_name)
		{
			Hero[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}







void StoryEventHeroMoveClass::Planet_Destroyed(const std::string &planet_name)
{
	std::vector<std::string>::iterator nameptr;

	for (nameptr = Planet.begin(); nameptr != Planet.end(); nameptr++)
	{
		std::string name = *nameptr;

		if (name == planet_name)
		{
			Story_Debug_Printf("EVENT %s, removing planet %s\r\n",planet_name.c_str());
			Planet.erase(nameptr);
			if (Planet.empty())
			{
				Disabled = true;
				return;
			}
		}
	}
}

bool StoryEventHeroMoveClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	bool ok = true;
	Planet.resize(0);
	std::string str;

	/*
	** Support for legacy save games. ST - 5/8/2006 6:34PM
	*/
	/* AJA 07/26/2006 - Removed for the expansion. Dev builds would trigger this and
	refuse to load this story data. We also don't care about backwards compatibility
	of savegames from the expansion just yet.

	static unsigned long _added_at_changelist_number = 40025;
	unsigned long save_game_version = SaveLoadClass::Get_Save_Game_Version();
	if (save_game_version < _added_at_changelist_number) {
		return(true);
	}
	*/

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
			case STORY_EVENT_BASE_CLASS_CHUNK:
				ok &= StoryEventClass::Load(reader);
				break;

			case STORY_EVENT_DATA_CHUNK:
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						case STORY_EVENT_OBJECT_NAME_CHUNK:
							ok &= reader->Read_String(str);
							Planet.push_back(str);
							break;

						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			default:
				assert(false);
				break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return( ok );
}





/**************************************************************************************************
* StoryEventHeroMoveClass::Save -- Custom save for event
*
* In:		
*
* Out:	
*
*
* History: 6/20/2005 6:07PM JSY
**************************************************************************************************/
bool StoryEventHeroMoveClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(STORY_EVENT_BASE_CLASS_CHUNK);
		ok &= StoryEventClass::Save(writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(STORY_EVENT_DATA_CHUNK);

	for (unsigned int i = 0; i < Planet.size(); ++i)
	{
		WRITE_MICRO_CHUNK_STRING(STORY_EVENT_OBJECT_NAME_CHUNK, Planet[i]);
	}

	ok &= writer->End_Chunk();

	return (ok);
}





/**************************************************************************************************
* StoryEventAccumulateClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:41PM JAC
**************************************************************************************************/
void StoryEventAccumulateClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		Credits = atoi((*param)[0].c_str());
	}
	else if (index == 1)
	{
		if ((*param)[0].size())
		{
			Comparison = (StoryCompareEnum)Lookup_Enum((*param)[0].c_str());
		}
	}
}





/**************************************************************************************************
* StoryEventAccumulateClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:41PM JAC
**************************************************************************************************/
void StoryEventAccumulateClass::Evaluate_Event(void *param1, void *)
{
	if (param1 == NULL)
	{
		return;
	}

	int credits = *(int *)param1;

	switch (Comparison)
	{
		case COMPARE_NONE:
		case COMPARE_GREATER_THAN:
			if (credits > Credits)
			{
				Event_Triggered();
			}
			break;

		case COMPARE_LESS_THAN:
			if (credits < Credits)
			{
				Event_Triggered();
			}
			break;

		case COMPARE_EQUAL_TO:
			if (credits == Credits)
			{
				Event_Triggered();
			}
			break;

		case COMPARE_GREATER_THAN_EQUAL_TO:
			if (credits >= Credits)
			{
				Event_Triggered();
			}
			break;

		case COMPARE_LESS_THAN_EQUAL_TO:
			if (credits <= Credits)
			{
				Event_Triggered();
			}
			break;

		default:
			break;
	}

}






/**************************************************************************************************
* StoryEventConquerCountClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:41PM JAC
**************************************************************************************************/
void StoryEventConquerCountClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		TriggerCount = atoi((*param)[0].c_str());
	}
}





/**************************************************************************************************
* StoryEventConquerCountClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:41PM JAC
**************************************************************************************************/
void StoryEventConquerCountClass::Evaluate_Event(void *param1, void *)
{
	CurCount++;

	if (CurCount >= TriggerCount)
	{
		GameObjectClass *planet = (GameObjectClass *)param1;
		Event_Triggered(planet);
	}
}





/**************************************************************************************************
* StoryEventConquerCountClass::Load -- Custom load for Conquer event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:56AM JAC
**************************************************************************************************/
bool StoryEventConquerCountClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
			case STORY_EVENT_BASE_CLASS_CHUNK:
				ok &= StoryEventClass::Load(reader);
				break;

			case STORY_EVENT_DATA_CHUNK:
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK(STORY_EVENT_CUR_COUNT_CHUNK, CurCount);
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return( ok );

}





/**************************************************************************************************
* StoryEventConquerCountClass::Save -- Custom save for Conquer event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:56AM JAC
**************************************************************************************************/
bool StoryEventConquerCountClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(STORY_EVENT_BASE_CLASS_CHUNK);
		ok &= StoryEventClass::Save(writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(STORY_EVENT_DATA_CHUNK);
		WRITE_MICRO_CHUNK(STORY_EVENT_CUR_COUNT_CHUNK, CurCount);
	ok &= writer->End_Chunk();

	return (ok);
}









/**************************************************************************************************
* StoryEventElapsedClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:42PM JAC
**************************************************************************************************/
void StoryEventElapsedClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		TriggerTime = (float)atof((*param)[0].c_str());
	}
}





/**************************************************************************************************
* StoryEventElapsedClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:42PM JAC
**************************************************************************************************/
void StoryEventElapsedClass::Evaluate_Event(void *param1, void *param2)
{
	if ((param1 == NULL) || (param2 == NULL)) 
	{
		return;
	}

	float elapsed = *(float *)param1;
	bool paused = *(bool *)param2;

	if (!Initialized)
	{
		// The first time through, we need to set LastTime otherwise the timer will be from the start of the game
		// Since the elapsed event is evaluated every frame, we won't miss out on anything
		LastTime = elapsed;
		Initialized = true;
		return;
	}

	if (elapsed < LastTime)
	{
		Story_Debug_Printf("STORY_ELAPSED %s, LastTime %f > elapsed time %f\r\n",EventName.c_str(),LastTime,elapsed);
		LastTime = elapsed;
		return;
	}

	if (!paused)
	{
		Elapsed += elapsed - LastTime;
		//Story_Debug_Printf("Elapsed %f\r\n",Elapsed);
	}

	LastTime = elapsed;

	if (Elapsed >= TriggerTime)
	{
		Story_Debug_Printf("STORY_ELAPSED %s triggered at %f\r\n",EventName.c_str(),elapsed);
		Event_Triggered();
	}
}





/**************************************************************************************************
* StoryEventElapsedClass::Load -- Custom load for Elapsed event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:57AM JAC
**************************************************************************************************/
bool StoryEventElapsedClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
			case STORY_EVENT_BASE_CLASS_CHUNK:
				ok &= StoryEventClass::Load(reader);
				break;

			case STORY_EVENT_DATA_CHUNK:
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK(STORY_EVENT_LAST_TIME_CHUNK,LastTime);
						READ_MICRO_CHUNK(STORY_EVENT_ELAPSED_TIME_CHUNK,Elapsed);
						READ_MICRO_CHUNK(STORY_EVENT_ELAPSED_INITIALIZED_CHUNK,Initialized);
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return( ok );
}





/**************************************************************************************************
* StoryEventElapsedClass::Save -- Custom save for Elapsed event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:57AM JAC
**************************************************************************************************/
bool StoryEventElapsedClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(STORY_EVENT_BASE_CLASS_CHUNK);
		ok &= StoryEventClass::Save(writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(STORY_EVENT_DATA_CHUNK);
		WRITE_MICRO_CHUNK(STORY_EVENT_LAST_TIME_CHUNK,LastTime);
		WRITE_MICRO_CHUNK(STORY_EVENT_ELAPSED_TIME_CHUNK,Elapsed);
		WRITE_MICRO_CHUNK(STORY_EVENT_ELAPSED_INITIALIZED_CHUNK,Initialized);
	ok &= writer->End_Chunk();

	return (ok);
}






void StoryEventElapsedClass::Activate()
{
	if (Initialized)
	{
		Story_Debug_Printf("\n!!! STORY_ELAPSED is already initialized when activated\r\n");
	}

	assert(TriggerTime >= 0);

	LastTime = GameModeManager.Get_Frame_Timer() * FrameSynchronizer.Get_Inv_Logical_FPS();
	Initialized = true;

	Story_Debug_Printf("STORY_ELAPSED %s starting timer at time %f, end time %f\r\n",EventName.c_str(),LastTime,LastTime+TriggerTime);
}










/**************************************************************************************************
* StoryEventBeginEraClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:35PM JAC
**************************************************************************************************/
void StoryEventBeginEraClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		Era = atoi((*param)[0].c_str());
	}
}





/**************************************************************************************************
* StoryEventBeginEraClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:36PM JAC
**************************************************************************************************/
void StoryEventBeginEraClass::Evaluate_Event(void *param1, void *)
{
	if (param1 == NULL) 
	{
		return;
	}

	int newera = *(int *)param1;

	if (newera >= Era)
	{
		if (EventType == STORY_BEGIN_ERA)
		{
			Story_Debug_Printf("STORY ERA %d reached\r\n",newera);
		}
		else
		{
			Story_Debug_Printf("STORY TECH LEVEL %d reached\r\n",newera);
		}
		Event_Triggered();
	}
	else if (Era == -1)
	{
		// If Era isn't specified, then the next era should trigger this
		if (OldEra == -1)
		{
			// We need to initialize the current era before we can tell if the era has changed
			OldEra = newera;
		}
		else
		{
			if (OldEra != newera)
			{
				if (EventType == STORY_BEGIN_ERA)
				{
					Story_Debug_Printf("STORY next ERA %d reached\r\n",newera);
				}
				else
				{
					Story_Debug_Printf("STORY next TECH LEVEL %d reached\r\n",newera);
				}
				Event_Triggered();
			}
		}
	}
}




/**************************************************************************************************
* StoryEventBeginEraClass::Load -- Custom load for Begin Era event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:57AM JAC
**************************************************************************************************/
bool StoryEventBeginEraClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
			case STORY_EVENT_BASE_CLASS_CHUNK:
				ok &= StoryEventClass::Load(reader);
				break;

			case STORY_EVENT_DATA_CHUNK:
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK(STORY_EVENT_OLD_ERA_CHUNK,OldEra);
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return( ok );
}





/**************************************************************************************************
* StoryEventBeginEraClass::Save -- Custom save for Begin Era event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:57AM JAC
**************************************************************************************************/
bool StoryEventBeginEraClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(STORY_EVENT_BASE_CLASS_CHUNK);
		ok &= StoryEventClass::Save(writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(STORY_EVENT_DATA_CHUNK);
		WRITE_MICRO_CHUNK(STORY_EVENT_OLD_ERA_CHUNK, OldEra);
	ok &= writer->End_Chunk();

	return (ok);
}









/**************************************************************************************************
* StoryEventDestroyClass::Shutdown -- Clear out data
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:57AM JAC
**************************************************************************************************/
void StoryEventDestroyClass::Shutdown()
{
	for (unsigned int i=0; i<Object.size(); i++)
	{
		Object[i].clear();
	}

	Object.clear();

	for (unsigned int i=0; i<Planet.size(); i++)
	{
		Planet[i].clear();
	}

	Planet.clear();
}


/**************************************************************************************************
* StoryEventDestroyClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:36PM JAC
**************************************************************************************************/
void StoryEventDestroyClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		char name[ 256 ];
		Object.resize(0);

		//Story_Debug_Printf("Story political control param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string object_name(name);
				Object.push_back(object_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid object name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}

		for (unsigned int i=0; i<param->size(); i++)
		{
			Count.push_back(1);
			CountCopy.push_back(1);
		}

		//Story_Debug_Printf("\r\n");
	}
	else if (index == 1)
	{
		char name[ 256 ];
		Planet.resize(0);

		//Story_Debug_Printf("Story political control param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string planet_name(name);
				Planet.push_back(planet_name);
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
	else if (index == 2)
	{
		if (Count.size() < param->size())
		{
			Count.resize(param->size());
			CountCopy.resize(param->size());
		}

		for (unsigned int i=0; i<param->size(); i++)
		{
			int count = atoi((*param)[i].c_str());
			Count[i] = count;
			CountCopy[i] = count;
		}
	}
	else if (index == 3)
	{
		Filter = (StoryEventFilter)Lookup_Enum((*param)[0].c_str());
		//Story_Debug_Printf("Enter param 2 - %s\r\n",(*param)[0].c_str());
	}
}





/**************************************************************************************************
* StoryEventDestroyClass::Evaluate_Event -- See if the event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:36PM JAC
**************************************************************************************************/
void StoryEventDestroyClass::Evaluate_Event(void *param1, void *param2)
{
	if ((param1 == NULL)) 
	{
		return;
	}

	const GameObjectTypeClass *object_type = (const GameObjectTypeClass *)param1;
	const std::string *objname = object_type->Get_Name();
	GameObjectClass *planet = (GameObjectClass *)param2;
	const std::string *planetname = NULL;
	if (planet)
	{
		planetname = planet->Get_Type()->Get_Name();
	}
	bool objfound = false;

	for (unsigned int i=0; i<Object.size(); i++)
	{
		if (Object[i] == *objname)
		{
			objfound = true;
			break;
		}
	}

	if (objfound)
	{
		if (Planet.empty())
		{
			int curcount = --Count[i];

			if (curcount <= 0)
			{
				Event_Triggered(planet);
			}

			return;
		}
		else
		{
			for (unsigned int i=0; i<Planet.size(); i++)
			{
				if (planetname && (Planet[i] == *planetname))
				{
					if (Event_Filter_Matches(planet,Filter))
					{
						int curcount = --Count[i];

						if (curcount <= 0)
						{
							Event_Triggered(planet);
						}

						return;
					}
				}
			}
		}
	}
}







void StoryEventDestroyClass::Reset()
{
	StoryEventClass::Reset();
	for (unsigned int i=0; i<CountCopy.size(); i++)
	{
		Count[i] = CountCopy[i];
	}
}







void StoryEventDestroyClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == var_name)
		{
			Planet[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	for (unsigned int i=0; i<Object.size(); i++)
	{
		if (Object[i] == var_name)
		{
			Object[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}





/**************************************************************************************************
* StoryEventDestroyClass::Load -- Custom load for event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:57AM JAC
**************************************************************************************************/
bool StoryEventDestroyClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
		case STORY_EVENT_BASE_CLASS_CHUNK:
				ok &= StoryEventClass::Load(reader);
				break;

			case STORY_EVENT_DATA_CHUNK:

				//1/12/2006 JSY - Clear out the initial count loaded from the plot file; the number we load from the save game
				//can be different if we're already part-way to the target and it's this number that takes precedence.
				Count.resize(0);
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						case STORY_EVENT_COUNT_CHUNK:   
							{
								int new_count = 0;
								PG_STATIC_ASSERT(sizeof(new_count) < 255 && sizeof("Micro Chunks can only contain less than 255 bytes") > 0);   
								reader->Read(&new_count, sizeof(new_count));  
								Count.push_back(new_count);
								break;   
							}  
						
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return( ok );
}





/**************************************************************************************************
* StoryEventDestroyClass::Save -- Custom save for event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:57AM JAC
**************************************************************************************************/
bool StoryEventDestroyClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(STORY_EVENT_BASE_CLASS_CHUNK);
		ok &= StoryEventClass::Save(writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(STORY_EVENT_DATA_CHUNK);
	for (unsigned int i=0; i<Count.size(); i++)
	{
		WRITE_MICRO_CHUNK(STORY_EVENT_COUNT_CHUNK,Count[i]);
	}
	ok &= writer->End_Chunk();

	return (ok);
}















void StoryEventDestroyBaseClass::Shutdown()
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		Planet[i].clear();
	}

	Planet.clear();
}






void StoryEventDestroyBaseClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		char name[ 256 ];
		Planet.resize(0);

		//Story_Debug_Printf("Story construct level param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string planet_name(name);
				Planet.push_back(planet_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid planet name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
	else if (index == 1)
	{
		Filter = (StoryBaseFilter)Lookup_Enum((*param)[0].c_str());
		//Story_Debug_Printf("Story construct param 3 - %s\r\n",(*param)[0].c_str());
	}
	else if (index == 2)
	{
		OwnerFilter = (StoryEventFilter)Lookup_Enum((*param)[0].c_str());
		//Story_Debug_Printf("Enter param 2 - %s\r\n",(*param)[0].c_str());
	}
}





void StoryEventDestroyBaseClass::Evaluate_Event(void *param1, void *param2)
{
	if (param1 == NULL)
	{
		return;
	}

	GameObjectClass *planet = (GameObjectClass *)param1;
	const std::string *planetname = planet->Get_Type()->Get_Name();
	StoryBaseFilter type = *(StoryBaseFilter *)param2;

	if (Planet.empty())
	{
		if (Event_Filter_Matches(planet,OwnerFilter))
		{
			if ((type == Filter) || (Filter == BASE_EITHER))
			{
				Story_Debug_Printf("STORY MODE - Base destruction event %s triggered\r\n",EventName.c_str());
				Event_Triggered(planet);
			}
		}
	}
	else
	{
		for (unsigned int i=0; i<Planet.size(); i++)
		{
			if (Planet[i] == *planetname)
			{
				if (Event_Filter_Matches(planet,OwnerFilter))
				{
					if ((type == Filter) || (Filter == BASE_EITHER))
					{
						Story_Debug_Printf("STORY MODE - Base destruction event %s triggered\r\n",EventName.c_str());
						Event_Triggered(planet);
						break;
					}
				}
			}
		}
	}
}






void StoryEventDestroyBaseClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == var_name)
		{
			Planet[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}
}






void StoryEventDestroyBaseClass::Planet_Destroyed(const std::string &planet_name)
{
	std::vector<std::string>::iterator nameptr;

	for (nameptr = Planet.begin(); nameptr != Planet.end(); nameptr++)
	{
		std::string name = *nameptr;

		if (name == planet_name)
		{
			Story_Debug_Printf("EVENT %s, removing planet %s\r\n",planet_name.c_str());
			Planet.erase(nameptr);
			if (Planet.empty())
			{
				Disabled = true;
				return;
			}
		}
	}
}










/**************************************************************************************************
* StoryEventWinBattlesClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:37PM JAC
**************************************************************************************************/
void StoryEventWinBattlesClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		TriggerCount = atoi((*param)[0].c_str());
	}
	else if (index == 1)
	{
		Filter = (StoryBaseFilter)Lookup_Enum((*param)[0].c_str());
	}
	else if (index == 2)
	{
		ContentTypes.Truncate();
		if (param->size() > 1)
		{
			ContentFilter = (FleetContentEnum)Lookup_Enum((*param)[0].c_str());
			for (unsigned int i=1; i<param->size(); i++)
			{
				ShipClassType ship_type = (ShipClassType)Lookup_Enum((*param)[i].c_str());
				ContentTypes.Add(ship_type);
			}
		}
	}
	if (index == 3)
	{
		Planet.resize(0);
		char name[ 256 ];

		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string planet_name(name);
				Planet.push_back(planet_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid planet name %s\r\n",EventName.c_str(),name);
				}
			}
		}
	}
}




/**************************************************************************************************
* StoryEventWinBattlesClass::Evaluate_Event -- See if the event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:37PM JAC
**************************************************************************************************/
void StoryEventWinBattlesClass::Evaluate_Event(void *param1, void *param2)
{
	if (param1 == NULL)
	{
		return;
	}

	StoryBaseFilter location = *(StoryBaseFilter *)param1;
	GameObjectClass *planet = (GameObjectClass *)param2;

	// Sometimes for space battles we care about the contents of the fleet to properly trigger
	if ((location != BASE_SPACE) || Content_Matches(planet))
	{
		if ((location == Filter) || (Filter == BASE_EITHER))
		{
			CurCount++;
			if (CurCount >= TriggerCount)
			{
				Story_Debug_Printf("Event %s triggered because %d of %d battles were won\r\n",EventName.c_str(),CurCount,TriggerCount);
				if (Planet.empty())
				{
					Event_Triggered(planet);
				}
				else
				{
					const std::string *planetname = planet->Get_Type()->Get_Name();
					for (unsigned int i=0; i<Planet.size(); i++)
					{
						if (Planet[i] == *planetname)
						{
							Event_Triggered(planet);
							break;
						}
					}
				}
			}
			else
			{
				Story_Debug_Printf("Event %s didn't trigger because only %d of %d battles won\r\n",EventName.c_str(),CurCount,TriggerCount);
			}
		}
	}
}




bool StoryEventWinBattlesClass::Content_Matches(GameObjectClass *planet)
{
	if (ContentFilter == FC_ANY)
	{
		// Any fleet contents will do
		return (true);
	}

	if (planet == NULL)
	{
		return (false);
	}

	assert(planet->Behaves_Like(BEHAVIOR_PLANET));

	DynamicVectorClass<ShipClassType> *fleet_list;

	if ((ContentFilter == FC_FRIENDLY_CONTAINS) || (ContentFilter == FC_FRIENDLY_ONLY_CONTAINS))
	{
		// If the story's local player is same as the game's local player then get the friendly fleet from the tutorial
		if (SubPlot->Get_Local_Player()->Get_ID() == PlayerList.Get_Local_Player_ID())
		{
			fleet_list = TheTutorial.Get_Space_Combat_Ships(true);
		}
		else
		{
			fleet_list = TheTutorial.Get_Space_Combat_Ships(false);
		}
	}
	else
	{
		// If the story's local player is same as the game's local player then get the enemy fleet from the tutorial
		if (SubPlot->Get_Local_Player()->Get_ID() == PlayerList.Get_Local_Player_ID())
		{
			fleet_list = TheTutorial.Get_Space_Combat_Ships(false);
		}
		else
		{
			fleet_list = TheTutorial.Get_Space_Combat_Ships(true);
		}
	}

	switch (ContentFilter)
	{
		case FC_FRIENDLY_CONTAINS:
		case FC_ENEMY_CONTAINS:
			// The fleet can contain other ship types than the one(s) specified
			{
				bool found = false;

				for (int i=0; i<ContentTypes.Size(); i++)
				{
					ShipClassType target_class = ContentTypes[i];
					found = false;

					for (int j=0; j<fleet_list->Size(); j++)
					{
						ShipClassType ship_class = (*fleet_list)[j];
						if (ship_class == target_class)
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						return (false);
					}
				}

				// If we made it to here, then every ship type must have been found
				return (true);
			}
			break;

		case FC_FRIENDLY_ONLY_CONTAINS:
		case FC_ENEMY_ONLY_CONTAINS:
			// The fleet can only contain the ship type(s) specified
			{
				for (int j=0; j<fleet_list->Size(); j++)
				{
					ShipClassType ship_class = (*fleet_list)[j];
					bool found = false;

					// Loop through our exclusive type list to see if this ship is on the list
					for (int i=0; i<ContentTypes.Size(); i++)
					{
						ShipClassType target_class = ContentTypes[i];
						if (ship_class == target_class)
						{
							found = true;
							break;
						}
					}

					// If any ship isn't on the exclusive ship class list, then the check has failed
					if (!found)
					{
						return (false);
					}
				}

				return (true);
			}
			break;

		default:
			return (true);
			break;
	}
}


/**************************************************************************************************
* StoryEventWinBattlesClass::Load -- Custom load for Win Battles event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:58AM JAC
**************************************************************************************************/
bool StoryEventWinBattlesClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
			case STORY_EVENT_BASE_CLASS_CHUNK:
				ok &= StoryEventClass::Load(reader);
				break;

			case STORY_EVENT_DATA_CHUNK:
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK(STORY_EVENT_CUR_COUNT_CHUNK,CurCount);
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return (ok);
}





/**************************************************************************************************
* StoryEventWinBattlesClass::Save -- Custom save for Win Battles event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:58AM JAC
**************************************************************************************************/
bool StoryEventWinBattlesClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(STORY_EVENT_BASE_CLASS_CHUNK);
		ok &= StoryEventClass::Save(writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(STORY_EVENT_DATA_CHUNK);
		WRITE_MICRO_CHUNK(STORY_EVENT_CUR_COUNT_CHUNK, CurCount);
	ok &= writer->End_Chunk();

	return (ok);
}



/**************************************************************************************************
* StoryEventAINotificationClass::Evaluate_Event -- See if the event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 1/21/2005 6:29PM JSY
**************************************************************************************************/
void StoryEventAINotificationClass::Evaluate_Event(void *param1, void *param2)
{
	if (param1 == NULL)
	{
		return;
	}

	std::string *trigger_string = static_cast<std::string*>(param1);
	GameObjectClass *planet = static_cast<GameObjectClass*>(param2);

	for (unsigned i = 0; i < TriggerString.size(); ++i)
	{
		if (_stricmp(TriggerString[i].c_str(), trigger_string->c_str()) == 0)
		{
			if (planet && Planet.size() > 0)
			{
				for (unsigned int j = 0; j < Planet.size(); ++j)
				{
					if (_stricmp(planet->Get_Type()->Get_Name()->c_str(), Planet[j].c_str()) == 0)
					{
						Event_Triggered(planet);
						break;
					}
				}
			}
			else if (!planet && Planet.size() == 0)
			{
				Event_Triggered(planet);
			}
			break;
		}
	}
}



/**************************************************************************************************
* StoryEventAINotificationClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 1/21/2005 6:30PM JSY
**************************************************************************************************/
void StoryEventAINotificationClass::Set_Param(int index, std::vector<std::string> *param)
{
	FAIL_IF(!param) { return; }

	char name[256];
	Planet.resize(0);

	for (unsigned int i = 0; i < param->size(); i++)
	{
		if (param->at(i).size())
		{
			strcpy( name, (*param)[i].c_str() );
			_strupr( name );

			if (index == 0)
			{
				Planet.push_back(std::string(name));

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid planet name %s\r\n", EventName.c_str(), name);
				}
			}
			else
			{
				TriggerString.push_back(std::string(name));
			}
		}
	}
}



/**************************************************************************************************
* StoryEventAINotificationClass::Replace_Variable -- Substitute a new string parameter for an existing one
*
* In:		
*
* Out:	
*
*
* History: 1/21/2005 6:31PM JSY
**************************************************************************************************/
void StoryEventAINotificationClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i = 0; i < Planet.size(); ++i)
	{
		if (Planet[i] == var_name)
		{
			Planet[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n", EventName.c_str(), var_name.c_str(), new_name.c_str());
		}
	}

	for (unsigned int i = 0; i < TriggerString.size(); ++i)
	{
		if (TriggerString[i] == var_name)
		{
			TriggerString[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n", EventName.c_str(), var_name.c_str(), new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name, new_name);
}


/**************************************************************************************************
* StoryEventAINotificationClass::Planet_Destroyed -- Handle destruction of the named planet
*
* In:		
*
* Out:	
*
*
* History: 1/21/2005 6:32PM JSY
**************************************************************************************************/
void StoryEventAINotificationClass::Planet_Destroyed(const std::string &planet_name)
{
	std::vector<std::string>::iterator nameptr;

	for (nameptr = Planet.begin(); nameptr != Planet.end();)
	{
		std::string name = *nameptr;

		if (name == planet_name)
		{
			Story_Debug_Printf("EVENT %s, removing planet %s\r\n", EventName.c_str(), planet_name.c_str());
			nameptr = Planet.erase(nameptr);
			if (Planet.empty())
			{
				Disabled = true;
				return;
			}
		}
		else
		{
			++nameptr;
		}
	}
}






/**************************************************************************************************
* StoryEventAINotificationClass::Load -- Custom load for event
*
* In:		
*
* Out:	
*
*
* History: 6/20/2005 6:11PM JAC
**************************************************************************************************/
bool StoryEventAINotificationClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	bool ok = true;
	Planet.resize(0);
	TriggerString.resize(0);
	std::string str;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
			case STORY_EVENT_BASE_CLASS_CHUNK:
				ok &= StoryEventClass::Load(reader);
				break;

			case STORY_EVENT_DATA_CHUNK:
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						case STORY_EVENT_TRIGGER_STRING_CHUNK:
							ok &= reader->Read_String(str);
							TriggerString.push_back(str);
							break;

						case STORY_EVENT_OBJECT_NAME_CHUNK:
							ok &= reader->Read_String(str);
							Planet.push_back(str);
							break;

						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return( ok );
}





/**************************************************************************************************
* StoryEventAINotificationClass::Save -- Custom save for event
*
* In:		
*
* Out:	
*
*
* History: 6/20/2005 6:12PM JAC
**************************************************************************************************/
bool StoryEventAINotificationClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(STORY_EVENT_BASE_CLASS_CHUNK);
		ok &= StoryEventClass::Save(writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(STORY_EVENT_DATA_CHUNK);

	for (unsigned int i = 0; i < Planet.size(); ++i)
	{
		WRITE_MICRO_CHUNK_STRING(STORY_EVENT_OBJECT_NAME_CHUNK, Planet[i]);
	}

	for (unsigned int i = 0; i < TriggerString.size(); ++i)
	{
		WRITE_MICRO_CHUNK_STRING(STORY_EVENT_TRIGGER_STRING_CHUNK, TriggerString[i]);
	}

	ok &= writer->End_Chunk();

	return (ok);
}






void StoryEventCommandUnitClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);
	char name[ 256 ];

	if (index == 0)
	{
		UnitName.resize(0);
		//Story_Debug_Printf("Enter param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string unit_name(name);
				UnitName.push_back(unit_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid object name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
	else if (index == 1)
	{
		TargetName.resize(0);
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string target_name(name);
				TargetName.push_back(target_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid object name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
			}
		}
	}
	else if (index == 2)
	{
		if ((*param)[0].size())
		{
			MaxDist = (float)atof((*param)[0].c_str());
		}
	}
}





void StoryEventCommandUnitClass::Evaluate_Event(void *param1, void *param2)
{
	if (param1 == NULL)
	{
		return;
	}

	std::string *objname = (std::string *)param1;
	Vector3 *pos = (Vector3 *)param2;

	for (unsigned int i=0; i<UnitName.size(); i++)
	{
		if (UnitName[i] == *objname)
		{
			if (TargetName.empty() || (MaxDist == 0))
			{
				Event_Triggered();
				return;
			}
			else
			{
				for (unsigned int j=0; j<TargetName.size(); j++)
				{
					const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(TargetName[j].c_str());
					const DynamicVectorClass<GameObjectClass *> *target_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);
					for (int k=0; k<target_list->Size(); k++)
					{
						GameObjectClass *target = (*target_list)[k];
						Vector3 targetpos = target->Get_Position();
						Vector3 v = targetpos - *pos;
						float dist = v.Length();
						if (dist < MaxDist)
						{
							Event_Triggered();
							return;
						}
					}
				}
			}
		}
	}
}








void StoryEventCommandUnitClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<UnitName.size(); i++)
	{
		if (UnitName[i] == var_name)
		{
			UnitName[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	for (unsigned int i=0; i<TargetName.size(); i++)
	{
		if (TargetName[i] == var_name)
		{
			TargetName[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}















void StoryEventGuardUnitClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);
	char name[ 256 ];

	if (index == 0)
	{
		Guard.resize(0);
		//Story_Debug_Printf("Enter param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string unit_name(name);
				Guard.push_back(unit_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid object name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
	else if (index == 1)
	{
		Guardee.resize(0);
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string target_name(name);
				Guardee.push_back(target_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid object name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
			}
		}
	}
}





void StoryEventGuardUnitClass::Evaluate_Event(void *param1, void *param2)
{
	if (param1 == NULL)
	{
		return;
	}

	std::string *objname = (std::string *)param1;
	std::string *targetname = (std::string *)param2;

	for (unsigned int i=0; i<Guard.size(); i++)
	{
		if (Guard[i] == *objname)
		{
			for (unsigned int j=0; j<Guardee.size(); j++)
			{
				if (Guardee[i] == *targetname)
				{
					Event_Triggered();
					return;
				}
			}
		}
	}
}








void StoryEventGuardUnitClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Guard.size(); i++)
	{
		if (Guard[i] == var_name)
		{
			Guard[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	for (unsigned int i=0; i<Guardee.size(); i++)
	{
		if (Guardee[i] == var_name)
		{
			Guardee[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}







void StoryEventProximityClass::Evaluate_Event(void *param1, void *)
{
	if (param1 == NULL)
	{
		return;
	}

	GameObjectClass *object = (GameObjectClass *)param1;
	if (object == NULL)
	{
		return;
	}

	Vector3 pos = object->Get_Position();

	if (UnitName.empty())
	{
		Check_For_Proximity(pos);
	}
	else
	{
		for (unsigned int i=0; i<UnitName.size(); i++)
		{
			if (UnitName[i] == *object->Get_Type()->Get_Name())
			{
				Check_For_Proximity(pos);
			}
		}
	}
}





void StoryEventProximityClass::Check_For_Proximity(Vector3 &pos)
{
	for (unsigned int j=0; j<TargetName.size(); j++)
	{
		const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(TargetName[j].c_str());
		const DynamicVectorClass<GameObjectClass *> *target_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);
		for (int k=0; k<target_list->Size(); k++)
		{
			GameObjectClass *target = (*target_list)[k];
			Vector3 targetpos = target->Get_Position();
			Vector3 v = targetpos - pos;
			float dist = v.Length();
			if (dist < MaxDist)
			{
				Event_Triggered();
				return;
			}
		}
	}
}







void StoryEventDifficultyClass::Set_Param(int index, std::vector<std::string> *param)
{
	if (index == 0)
	{
		bool result = TheDifficultyLevelTypeConverterPtr->String_To_Enum((*param)[0],Difficulty);
		if (!result)
		{
			Story_Debug_Printf("WARNING!  Difficulty level %s unrecognized!\r\n",(*param)[0].c_str());
		}
	}
}





void StoryEventDifficultyClass::Evaluate_Event(void *, void *)
{
	DifficultyLevelType cur_difficulty = GameModeManager.Get_Difficulty_Level();

	if (cur_difficulty == Difficulty)
	{
		Story_Debug_Printf("STORY EVENT - Difficulty level %s triggered\r\n",Difficulty);
		Event_Triggered();
		return;
	}
}









void StoryEventFlagClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);
	char name[ 256 ];

	if (index == 0)
	{
		FlagNames.resize(0);
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string flag_name(name);
				FlagNames.push_back(flag_name);
			}
		}
	}
	else if (index == 1)
	{
		Value = atoi((*param)[0].c_str());
	}
	else if (index == 2)
	{
		if ((*param)[0].size())
		{
			Comparison = (StoryCompareEnum)Lookup_Enum((*param)[0].c_str());
		}
	}
}





void StoryEventFlagClass::Evaluate_Event(void *, void *)
{
	for (unsigned int i=0; i<FlagNames.size(); i++)
	{
		std::string *name = &FlagNames[i];
		int value = StoryModeClass::Get_Flag(name->c_str());

		if (value != UNDEFINED_STORY_FLAG)
		{
			switch (Comparison)
			{
				case COMPARE_NONE:
				case COMPARE_GREATER_THAN:
					if (value > Value)
					{
						Event_Triggered();
					}
					break;

				case COMPARE_LESS_THAN:
					if (value < Value)
					{
						Event_Triggered();
					}
					break;

				case COMPARE_EQUAL_TO:
					if (value == Value)
					{
						Event_Triggered();
					}
					break;

				case COMPARE_GREATER_THAN_EQUAL_TO:
					if (value >= Value)
					{
						Event_Triggered();
					}
					break;

				case COMPARE_LESS_THAN_EQUAL_TO:
					if (value <= Value)
					{
						Event_Triggered();
					}
					break;

				default:
					break;
			}
		}
	}
}





/**************************************************************************************************
* StoryEventLoadTacticalClass::Shutdown -- Clear out data
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:56AM JAC
**************************************************************************************************/
void StoryEventLoadTacticalClass::Shutdown()
{
	for (unsigned int i=0; i<Hero.size(); i++)
	{
		Hero[i].clear();
	}

	Hero.clear();

	for (unsigned int i=0; i<Planet.size(); i++)
	{
		Planet[i].clear();
	}

	Planet.clear();
}



/**************************************************************************************************
* StoryEventLoadTacticalClass::Set_Param -- Set the parameters for this event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:40PM JAC
**************************************************************************************************/
void StoryEventLoadTacticalClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		char name[ 256 ];
		Planet.resize(0);

		//Story_Debug_Printf("Story political control param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string planet_name(name);
				Planet.push_back(planet_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid planet name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
	else if (index == 1)
	{
		char name[ 256 ];
		Hero.resize(0);

		//Story_Debug_Printf("Story political control param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string hero_name(name);
				Hero.push_back(hero_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid hero name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
	else if (index == 2)
	{
		Base = (StoryBaseFilter)Lookup_Enum((*param)[0].c_str());
	}
}





/**************************************************************************************************
* StoryEventLoadTacticalClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 3:40PM JAC
**************************************************************************************************/
void StoryEventLoadTacticalClass::Evaluate_Event(void *param1, void *param2)
{
	if ((param1 == NULL) || (param2 == NULL)) 
	{
		return;
	}

	GameObjectClass *planet = (GameObjectClass *)param1;
	const std::string *planetname = planet->Get_Type()->Get_Name();
	StoryBaseFilter *location = (StoryBaseFilter *)param2;

	if (*location != Base)
	{
		return;
	}

	bool planetfound = false;

	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == *planetname)
		{
			planetfound = true;
			break;
		}
	}

	if (planetfound)
	{
		if (Hero.empty())
		{
			Event_Triggered(planet);
		}
		else
		{
			for (unsigned int i=0; i<Hero.size(); i++)
			{
				// Search through the hero list to see if any of them are on the planet
				GameObjectTypeClass *hero_type = GameObjectTypeManager.Find_Object_Type(Hero[i].c_str());
				if (hero_type)
				{
					const DynamicVectorClass<GameObjectClass *> *hero_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(hero_type);

					if (hero_list->Size() > 0)
					{
						GameObjectClass *hero = (*hero_list)[i];
						GameObjectClass *parent = hero->Get_Parent_Container_Object();

						// Find out which planet this hero is on
						while (parent && !parent->Behaves_Like(BEHAVIOR_PLANET))
						{
							parent = parent->Get_Parent_Container_Object();
						}

						if (parent && parent->Behaves_Like(BEHAVIOR_PLANET))
						{
							// If the hero is at this planet, we're done
							if (parent == planet)
							{
								Event_Triggered(planet);
								return;
							}
						}
					}
				}
			}
		}
	}
}







void StoryEventLoadTacticalClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == var_name)
		{
			Planet[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	for (unsigned int i=0; i<Hero.size(); i++)
	{
		if (Hero[i] == var_name)
		{
			Hero[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}







void StoryEventLoadTacticalClass::Planet_Destroyed(const std::string &planet_name)
{
	std::vector<std::string>::iterator nameptr;

	for (nameptr = Planet.begin(); nameptr != Planet.end(); nameptr++)
	{
		std::string name = *nameptr;

		if (name == planet_name)
		{
			Story_Debug_Printf("EVENT %s, removing planet %s\r\n",planet_name.c_str());
			Planet.erase(nameptr);
			if (Planet.empty())
			{
				Disabled = true;
				return;
			}
		}
	}
}







bool StoryEventLoadTacticalClass::Check_Special_Land_Tactical_Map(GameObjectClass *hero, GameObjectClass *planet)
{
	if ((planet == NULL) || (hero == NULL)) 
	{
		return (false);
	}

	const std::string *planetname = planet->Get_Type()->Get_Name();

	// We're only checking for special land tactical maps
	if (Base != BASE_GROUND)
	{
		return (false);
	}

	bool planetfound = false;

	// See if the supplied planet is in the planet list
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == *planetname)
		{
			planetfound = true;
			break;
		}
	}

	if (planetfound)
	{
		// Sometimes we only care if a LINK_TACTICAL exists on a planet, not if a hero will trigger it
		if (hero == NULL)
		{
			return (true);
		}
		else
		{
			const std::string *heroname = hero->Get_Type()->Get_Name();
			// See if the supplied hero is in the hero list
			for (unsigned int i=0; i<Hero.size(); i++)
			{
				if (Hero[i] == *heroname)
				{
					return (true);
				}
			}
		}
	}

	return (false);
}














const char *Story_Log_File = "StoryLog.txt";
static bool _first_run = true;
/**************************************************************************************************
* Story_Debug_Print -- Standard debug print function.
*
* In:  text string
*
* Out: nothing
*
*
*
* History: 6/1/2005 JAC
**************************************************************************************************/
void Story_Debug_Printf(char *text, ...)
{
	//assert(text != NULL);
	char _temp_buffer[1024];
	char *str_ptr = _temp_buffer;

	int len = strlen(text);

	if (len > 256) {
		str_ptr = (char*)_alloca(min(len*3, 64*1024));
	}

	if (text != NULL) {

		/*
		** Format the string.
		*/
		va_list va;
		va_start(va, text);
		vsprintf(str_ptr, text, va);
		va_end(va);

		len = strlen(str_ptr);

		if (TheStoryLogWindowPtr)
		{
			TheStoryLogWindowPtr->Debug_Log_Callback(str_ptr,len);
		}

		if (_first_run)
		{
			DeleteFile(Story_Log_File);
			_first_run = false;
		}

		Debug_Print_Alt(Story_Log_File, str_ptr);
	}
}


/**************************************************************************************************
* Story_Debug_Print -- Standard debug print function.
*
* In:  text string
*
* Out: nothing
*
*
*
* History: 6/1/2005 JAC
**************************************************************************************************/
void Story_Debug_Printf(const char *text, ...)
{
	//assert(text != NULL);
	static char _temp_buffer[1024];
	char *str_ptr = _temp_buffer;

	int len = strlen(text);

	if (len > 256) {
		str_ptr = (char*)_alloca(min(len*3, 64*1024));
	}

	if (text != NULL) {

		/*
		** Format the string.
		*/
		va_list va;
		va_start(va, text);
		vsprintf(str_ptr, text, va);
		va_end(va);

		len = strlen(str_ptr);

		if (TheStoryLogWindowPtr)
		{
			TheStoryLogWindowPtr->Debug_Log_Callback(str_ptr,len);
		}

		if (_first_run)
		{
			DeleteFile(Story_Log_File);
			_first_run = false;
		}

		Debug_Print_Alt(Story_Log_File, str_ptr);
	}
}








void StoryCheckDestroyedClass::Set_Param(int index, std::vector<std::string> *param)
{
	if (index == 0)
	{
		const FactionClass *faction = FactionList.Find_Faction_By_Name((*param)[0].c_str());
		if (faction == NULL)
		{
			Story_Debug_Printf("ERROR! STORY MODE - Unable to find faction %s\r\n",(*param)[0].c_str());
			return;
		}

		Player = PlayerList.Get_Player_Of_Faction(faction);
		if (Player == NULL)
		{
			Story_Debug_Printf("ERROR!  STORY MODE - Unable to find plater of faction %s\r\n",(*param)[0].c_str());
			return;
		}
	}
	else if (index == 1)
	{
		Filter = (StoryDestroyEnum)Lookup_Enum((*param)[0].c_str());
	}
	else if (index == 2)
	{
		TypeFilter = GameObjectTypeManager.Find_Object_Type((*param)[0]);
		if (!TypeFilter)
		{
			Story_Debug_Printf("ERROR!  STORY MODE - Unable to find object type %s\r\n",(*param)[0].c_str());
			return;
		}
	}
}





void StoryCheckDestroyedClass::Evaluate_Event(void *, void *)
{
	if (Player)
	{
		switch (Filter)
		{
			case DESTROY_ALL:
				if (All_Units_Destroyed(Player,true))
				{
					Event_Triggered();
				}
				break;

			case DESTROY_ALL_UNITS:
				if (All_Units_Destroyed(Player,false))
				{
					Event_Triggered();
				}
				break;

			case DESTROY_ALL_STRUCTURES:
				if (All_Structures_Destroyed(Player))
				{
					Event_Triggered();
				}
				break;

			case DESTROY_ALL_INDIGENOUS_SPAWNERS:
				if (All_Indigenous_Spawners_Destroyed(TypeFilter))
				{
					Event_Triggered();
					break;
				}

			default:
				break;
		}
	}
}





bool StoryEventClass::All_Units_Destroyed(PlayerClass *player, bool check_structures)
{
	// Find objects owned by the player
	int player_id = player->Get_ID();
	const DynamicVectorClass<GameObjectClass *> *objects = GAME_OBJECT_MANAGER.Find_Objects(BEHAVIOR_SELECTABLE, player_id);
	bool friendly_units = false;
	for (int i=0 ; i<objects->Get_Count() ; i++)
	{
		GameObjectClass *test_object = (*objects)[i];
		if (test_object->Get_Type()->Get_Is_Victory_Relevant())
		{
			if (!test_object->Is_Delete_Pending())
			{
				if (!test_object->Is_Dead())
				{
					if (test_object->Behaves_Like(BEHAVIOR_DUMMY_LAND_BASE_LEVEL_COMPONENT))
					{
						if (check_structures)
						{
							friendly_units = true;
							break;
						}
					}
					else
					{
						friendly_units = true;
						break;
					}
				}
			}
		}
	}

	return (!friendly_units);
}





bool StoryEventClass::All_Structures_Destroyed(PlayerClass *player)
{
	int player_id = player->Get_ID();
	const DynamicVectorClass<GameObjectClass *> *base_component_structures = GAME_OBJECT_MANAGER.Find_Objects
	(
		BEHAVIOR_DUMMY_LAND_BASE_LEVEL_COMPONENT, 
		player_id
	);

	bool all_base_components_destroyed = false;
	if ( base_component_structures->Get_Size() == 1 ) 
	{
		GameObjectClass *test_object = (*base_component_structures)[ 0 ];
		assert( test_object != NULL );
		if ( test_object->Is_Delete_Pending() || test_object->Is_Dead() )
		{
			all_base_components_destroyed = true;
		}
	}
	else if ( base_component_structures->Get_Size() == 0 ) 
	{
		all_base_components_destroyed = true;
	}

	return (all_base_components_destroyed);
}




bool StoryEventClass::All_Indigenous_Spawners_Destroyed(const GameObjectTypeClass *type)
{
	const DynamicVectorClass<GameObjectClass*> &spawners = SpawnIndigenousUnitsBehaviorClass::Get_All_Spawners();
	for (int i = 0; i < spawners.Size(); ++i)
	{
		GameObjectClass *spawn_structure = spawners[i];
		if (!spawn_structure || spawn_structure->Is_Dead() || spawn_structure->Is_Delete_Pending())
		{
			continue;
		}

		if (!type || spawn_structure->Get_Type() == type)
		{
			return false;
		}
	}

	return true;
}



void StoryEventVictoryClass::Evaluate_Event(void *winning_faction, void *)
{
	if (!Faction)
	{
		Story_Debug_Printf("ERROR! STORY_VICTORY - Can't trigger event, we couldn't find the player at startup.\r\n");
		return;
	}

	if (Faction == winning_faction)
			Event_Triggered();
}


void StoryEventVictoryClass::Set_Param(int index, std::vector<std::string> *param)
{
	if (index == 0)
	{
		Faction = const_cast<FactionClass *>(FactionList.Find_Faction_By_Name((*param)[0].c_str()));
		if (Faction == NULL)
		{
			Story_Debug_Printf("ERROR! STORY_VICTORY - Unable to find faction %s\r\n",(*param)[0].c_str());
			return;
		}
	}
}


void StoryEventMovieDoneClass::Evaluate_Event(void * /*param1*/, void *)
{
	//std::string *param_movie_name = static_cast<std::string *>(param1);
	//if (0 == Movie_Name.compare(*param_movie_name))
	Event_Triggered();
	FrameSynchronizer.Resume();
}


//void StoryEventMovieDoneClass::Set_Param(int index, std::vector<std::string> * param)
//{
//	if (index == 0)
//	{
//		Movie_Name = (*param)[0].c_str();
//	}
//	else
//	{
//		Story_Debug_Printf("ERROR! STORY_MOVIE_DONE: Event only takes one parameter, the name of the movie");
//	}
//}

/**************************************************************************************************
* StoryEventObjectiveTimeoutClass::Set_Param -- Set the parameters for this event
*
* In:		Parameters from the XML	
*
* Sets the timeout and name of the objective to keep track of. If the objective is not completed
* by the timeout time, then the event fires.
*
* History: 09/21/2005 3:42PM ENY
**************************************************************************************************/
void StoryEventObjectiveTimeoutClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		TriggerTime = (float)atof((*param)[0].c_str());
	}
	else if (index == 1)
	{
		ObjectiveName = (*param)[0];
	}

}




/**************************************************************************************************
* StoryEventObjectiveTimeoutClass::Evaluate_Event -- See if this event has been triggered
*
* In:	param1 = (float*) elapsed time, param2 = (bool*) whether (galactic mode) is paused
*
* If the objective hasn't been completed within the specified time, then the timeout is triggered.
* If the objective completed, failed or no longer exists, event DOESN'T go off. Only triggers
* if the objective is waiting to be completed. This is so we can remind the player of the 
* objective.
*
* History: 07/21/2004 3:42PM ENY
**************************************************************************************************/

void StoryEventObjectiveTimeoutClass::Evaluate_Event(void *param1, void *param2)
{
	if ((param1 == NULL) || (param2 == NULL)) 
	{
		return;
	}

	float elapsed = *(float *)param1;
	bool paused = *(bool *)param2;

	if (!Initialized)
	{
		// The first time through, we need to set LastTime otherwise the timer will be from the start of the game
		// Since the elapsed event is evaluated every frame, we won't miss out on anything
		LastTime = elapsed;
		Initialized = true;
		return;
	}

	if (elapsed < LastTime)
	{
		Story_Debug_Printf("STORY_OBJECTIVE_TIMEOUT %s, LastTime %f > elapsed time %f\r\n",EventName.c_str(),LastTime,elapsed);
		LastTime = elapsed;
		return;
	}

	if (!paused)
	{
		Elapsed += elapsed - LastTime;
		//Story_Debug_Printf("Elapsed %f\r\n",Elapsed);
	}

	LastTime = elapsed;

	if (Elapsed >= TriggerTime)
	{
		if (SubPlot->Get_Story_Mode()->Objective_Status(ObjectiveName) == OBJECTIVE_NOT_COMPLETE)
		{
			Story_Debug_Printf("STORY_OBJECTIVE_TIMEOUT %s triggered at %f\r\n",EventName.c_str(),elapsed);
			Event_Triggered();
		}
		else
		{
			// If the status is something other than OBJECTIVE_NOT_COMPLETE, then there is no need to keep
			// checking this event, so disable it.
			Disable_Event(true);
		}
	}
}


/**************************************************************************************************
* StoryEventObjectiveTimeoutClass::Load -- Custom load for Elapsed event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:57AM JAC
**************************************************************************************************/
bool StoryEventObjectiveTimeoutClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
		case STORY_EVENT_BASE_CLASS_CHUNK:
			ok &= StoryEventClass::Load(reader);
			break;

		case STORY_EVENT_DATA_CHUNK:
			while (reader->Open_Micro_Chunk())
			{
				switch ( reader->Cur_Micro_Chunk_ID() )
				{
					READ_MICRO_CHUNK(STORY_EVENT_LAST_TIME_CHUNK,LastTime);
					READ_MICRO_CHUNK(STORY_EVENT_ELAPSED_TIME_CHUNK,Elapsed);
					READ_MICRO_CHUNK(STORY_EVENT_ELAPSED_INITIALIZED_CHUNK,Initialized);
					READ_MICRO_CHUNK_STRING(STORY_EVENT_OBJECTIVE_NAME_CHUNK, ObjectiveName);
					default: assert(false); break;	// Unknown Chunk
				}
				reader->Close_Micro_Chunk();
			}
			break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return( ok );
}


/**************************************************************************************************
* StoryEventObjectiveTimeoutClass::Save -- Custom save for Elapsed event
*
* In:		
*
* Out:	
*
*
* History: 08/03/2004 11:57AM JAC
**************************************************************************************************/
bool StoryEventObjectiveTimeoutClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(STORY_EVENT_BASE_CLASS_CHUNK);
	ok &= StoryEventClass::Save(writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(STORY_EVENT_DATA_CHUNK);
	WRITE_MICRO_CHUNK(STORY_EVENT_LAST_TIME_CHUNK,LastTime);
	WRITE_MICRO_CHUNK(STORY_EVENT_ELAPSED_TIME_CHUNK,Elapsed);
	WRITE_MICRO_CHUNK(STORY_EVENT_ELAPSED_INITIALIZED_CHUNK,Initialized);
	WRITE_MICRO_CHUNK_STRING(STORY_EVENT_OBJECTIVE_NAME_CHUNK, ObjectiveName);
	ok &= writer->End_Chunk();

	return (ok);
}


void StoryEventObjectiveTimeoutClass::Activate()
{
	if (Initialized)
	{
		Story_Debug_Printf("\n!!! STORY_ELAPSED is already initialized when activated\r\n");
	}

	assert(TriggerTime >= 0);

	LastTime = GameModeManager.Get_Frame_Timer() * FrameSynchronizer.Get_Inv_Logical_FPS();
	Initialized = true;

	Story_Debug_Printf("STORY_OBJECTIVE_TIMEOUT %s starting timer at time %f, end time %f\r\n",EventName.c_str(),LastTime,LastTime+TriggerTime);
}











void StoryEventCaptureClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);
	char name[ 256 ];

	if (index == 0)
	{
		Structure.resize(0);
		//Story_Debug_Printf("Enter param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string unit_name(name);
				Structure.push_back(unit_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid object name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
	else if (index == 1)
	{
		if (param->size() > 0)
		{
			NewFaction = FactionList.Find_Faction_By_Name((*param)[0]);
			assert(NewFaction != NULL);
		}
	}
}





void StoryEventCaptureClass::Evaluate_Event(void *param1, void *param2)
{
	if (param1 == NULL)
	{
		return;
	}

	GameObjectClass *object = static_cast<GameObjectClass*>(param1);
	const FactionClass *new_faction = (const FactionClass *)param2;

	for (unsigned int i=0; i<Structure.size(); i++)
	{
		if (Structure[i] == *object->Get_Type()->Get_Name())
		{
			if (new_faction == NewFaction)
			{
				Event_Triggered();
				return;
			}
		}
	}
}








void StoryEventCaptureClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Structure.size(); i++)
	{
		if (Structure[i] == var_name)
		{
			Structure[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}



