
// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/GameObjectWrapper.cpp#3 $
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// (C) Petroglyph Games, Inc.
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
// C O N F I D E N T I A L   S O U R C E   C O D E -- D O   N O T   D I S T R I B U T E
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/GameObjectWrapper.cpp $
//
//    Original Author: Brian Hayes
//
//            $Author: Brian_Hayes $
//
//            $Change: 641585 $
//
//          $DateTime: 2017/05/10 10:42:50 $
//
//          $Revision: #3 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */

#pragma hdrstop
#include "GameObjectWrapper.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "GameModeManager.h"
#include "SaveLoad.h"
#include "DynamicEnum.h"
#include "AI/Movement/MovementCoordinatorSystem.h"
#include "AI/Movement/MovementCoordinator.h"
#include "AITargetLocationWrapper.h"
#include "AI/AITargetLocation.h"
#include "PGSignal/SignalDispatcher.h"
#include "GameObjectTypeWrapper.h"
#include "UnitAIBehavior.h"
#include "TargetingPrioritySetManager.h"
#include "DamageTrackingBehavior.h"
#include "AI/Planning/TaskForce.h"
#include "SpecialWeaponBehavior.h"
#include "LobbingSuperWeaponBehavior.h"
#include "TeamBehavior.h"
#include "TacticalBuildObjectsBehavior.h"
#include "TacticalSellBehavior.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "PositionWrapper.h"
#include "AI/AIPlayer.h"
#include "Player.h"
#include "AI/TacticalAIManager.h"
#include "AI/Execution/AIExecutionSystem.h"
#include "AI/Execution/AIFreeStore.h"
#include "TreeCull.h"
#include "PlanetaryBehavior.h"
#include "AI/Movement/Formation.h"
#include "Utils.h"
#include "AI/Planning/TargetContrast.h"
#include "LocomotorInterface.h"
#include "TransportBehavior.h"
#include "FleetBehavior.h"
#include "UnitAbilityType.h"
#include "AbilityCountdownBehavior.h"
#include "AI/LuaScript/Commands/UnitMoveObject.h"
#include "SetUnitAbilityModeEvent.h"
#include "TacticalSpecialAbilityEvent.h"
#include "ActivateBehaviorEvent.h"
#include "AI/LuaScript/Commands/AbilityBlock.h"
#include "AI/LuaScript/Commands/UnitAnimationBlock.h"
#include "TargetingInterface.h"
#include "IonStunEffectBehavior.h"
#include "GameObjectTypeManager.h"
#include "InvulnerableBehavior.h"
#include "AI/LuaScript/Commands/MoveObject.h"
#include "TacticalSuperWeaponBehavior.h"
#include "SFXEventManager.h"
#include "FleetLocomotorBehavior.h"
#include "Commands/GenericSignalBlock.h"
#include "WeaponBehavior.h"
#include "CapturePointBehavior.h"
#include "HardPointDataManager.h"
#include "HardPointData.h"
#include "AI/LuaScript/Commands/ExploreAreaBlock.h"
#include "GameConstants.h"
#include "BattlefieldModifiers.h"
#include "MoveToGarrisonEvent.h"
#include "GarrisonUnitBehavior.h"
#include "GarrisonVehicleBehavior.h"
#include "GarrisonStructureBehavior.h"
#include "InvadeEvent.h"
#include "GameScoringManager.h"


enum {
	GAMEOBJECTID_MICRO_CHUNK,
	SIGNAL_LISTENER_BASE_MICRO_CHUNK,
	GAMEOBJECTWRAPPER_DATA_CHUNK,
	IN_RANGE_LIST_SIZE_MICRO_CHUNK,
	IN_RANGE_ENTRY_CHUNK,
	ENTRY_DISTANCE_CHUNK,
	ENTRY_FUNCTION_CHUNK,
	ENTRY_TYPE_CHUNK,
	ENTRY_PLAYER_CHUNK,
	OBJECT_POSITION_CHUNK,
};

PG_IMPLEMENT_RTTI(GameObjectWrapper, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_GAMEOBJECT_WRAPPER, GameObjectWrapper);
MEMORY_POOL_INSTANCE(GameObjectWrapper, LUA_WRAPPER_POOL_SIZE);

GameObjectWrapper::WrapperCacheType *GameObjectWrapper::WrapperCache = NULL;

GameObjectWrapper::GameObjectWrapper() : 
	ObjectInRangeListModified(false)
,	Script(NULL)
,	Persistable(true)
{
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Release", &GameObjectWrapper::Release);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Transport", &GameObjectWrapper::Lua_Is_Transport);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Hull", &GameObjectWrapper::Get_Hull);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Health", &GameObjectWrapper::Get_Hull);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Shield", &GameObjectWrapper::Get_Shield);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Energy", &GameObjectWrapper::Get_Energy);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Category", &GameObjectWrapper::Is_Category);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Parent_Object", &GameObjectWrapper::Get_Parent_Object);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Attack_Target", &GameObjectWrapper::Attack_Target);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Valid", &GameObjectWrapper::Is_Valid);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Type", &GameObjectWrapper::Lua_Get_Type);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Game_Scoring_Type", &GameObjectWrapper::Lua_Get_Game_Scoring_Type);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Set_Prefer_Ground_Over_Space", &GameObjectWrapper::Lua_Set_Prefer_Ground_Over_Space);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Set_Targeting_Priorities", &GameObjectWrapper::Set_Targeting_Priorities);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Set_Targeting_Stickiness_Time_Threshold", &GameObjectWrapper::Set_Targeting_Stickiness_Time_Threshold);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Time_Till_Dead", &GameObjectWrapper::Get_Time_Till_Dead);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Rate_Of_Damage_Taken", &GameObjectWrapper::Get_Rate_Of_Damage_Taken);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Move_To", &GameObjectWrapper::Move_To);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Guard_Target", &GameObjectWrapper::Guard_Target);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Attack_Move", &GameObjectWrapper::Attack_Move);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Fire_Special_Weapon", &GameObjectWrapper::Fire_Special_Weapon);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Contains_Hero", &GameObjectWrapper::Contains_Hero);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Contained_Heroes", &GameObjectWrapper::Get_Contained_Heroes);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Are_Engines_Online", &GameObjectWrapper::Are_Engines_Online);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Distance", &GameObjectWrapper::Get_Distance);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Build_Pad_Contents", &GameObjectWrapper::Get_Build_Pad_Contents);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Sell", &GameObjectWrapper::Sell);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Owner", &GameObjectWrapper::Get_Owner);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Starbase_Level", &GameObjectWrapper::Get_Starbase_Level);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Final_Blow_Player", &GameObjectWrapper::Get_Final_Blow_Player);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Lock_Current_Orders", &GameObjectWrapper::Lock_Current_Orders);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Event_Object_In_Range", &GameObjectWrapper::Event_Object_In_Range);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Service_Wrapper", &GameObjectWrapper::Service_Wrapper);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Cancel_Event_Object_In_Range", &GameObjectWrapper::Cancel_Event_Object_In_Range);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Position", &GameObjectWrapper::Lua_Get_Position);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Prevent_AI_Usage", &GameObjectWrapper::Prevent_AI_Usage);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Set_Importance", &GameObjectWrapper::Set_Importance);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Take_Damage", &GameObjectWrapper::Take_Damage);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Despawn", &GameObjectWrapper::Despawn);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Mark_Parent_Mode_Object_For_Death", &GameObjectWrapper::Mark_Parent_Mode_Object_For_Death);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Set_Selectable", &GameObjectWrapper::Set_Selectable);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Next_Starbase_Type", &GameObjectWrapper::Get_Next_Starbase_Type);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Change_Owner", &GameObjectWrapper::Change_Owner);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Divert", &GameObjectWrapper::Divert);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_AI_Power_Vs_Unit", &GameObjectWrapper::Get_AI_Power_Vs_Unit);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Has_Active_Orders", &GameObjectWrapper::Has_Active_Orders);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Contained_Object_Count", &GameObjectWrapper::Get_Contained_Object_Count);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Contains_Object_Type", &GameObjectWrapper::Contains_Object_Type);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Destroy_Contained_Objects", &GameObjectWrapper::Destroy_Contained_Objects);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Ability_Ready", &GameObjectWrapper::Is_Ability_Ready);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Has_Ability", &GameObjectWrapper::Has_Ability);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Activate_Ability", &GameObjectWrapper::Activate_Ability);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Set_Single_Ability_Autofire", &GameObjectWrapper::Set_Single_Ability_Autofire);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Set_All_Abilities_Autofire", &GameObjectWrapper::Set_All_Abilities_Autofire);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Has_Property", &GameObjectWrapper::Has_Property);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Unlock_Current_Orders", &GameObjectWrapper::Unlock_Current_Orders);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Play_Animation", &GameObjectWrapper::Play_Animation);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_On_Diversion", &GameObjectWrapper::Is_On_Diversion);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Affiliated_Indigenous_Type", &GameObjectWrapper::Get_Affiliated_Indigenous_Type);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Planet_Destroyed", &GameObjectWrapper::Is_Planet_Destroyed);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Turn_To_Face", &GameObjectWrapper::Turn_To_Face);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Ability_Active", &GameObjectWrapper::Is_Ability_Active);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_In_Nebula", &GameObjectWrapper::Is_In_Nebula);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_In_Ion_Storm", &GameObjectWrapper::Is_In_Ion_Storm);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_In_Asteroid_Field", &GameObjectWrapper::Is_In_Asteroid_Field);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Under_Effects_Of_Ability", &GameObjectWrapper::Is_Under_Effects_Of_Ability);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Build", &GameObjectWrapper::Build);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Make_Invulnerable", &GameObjectWrapper::Make_Invulnerable);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Teleport", &GameObjectWrapper::Teleport);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Teleport_And_Face", &GameObjectWrapper::Teleport_And_Face);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Hyperspace_Away", &GameObjectWrapper::Hyperspace_Away);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Cinematic_Hyperspace_In", &GameObjectWrapper::Cinematic_Hyperspace_In);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Cancel_Hyperspace", &GameObjectWrapper::Cancel_Hyperspace);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Lock_Build_Pad_Contents", &GameObjectWrapper::Lock_Build_Pad_Contents);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Bone_Position", &GameObjectWrapper::Get_Bone_Position);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Tactical_Superweapon_Ready", &GameObjectWrapper::Is_Tactical_Superweapon_Ready);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Fire_Tactical_Superweapon", &GameObjectWrapper::Fire_Tactical_Superweapon);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Set_Garrison_Spawn", &GameObjectWrapper::Set_Garrison_Spawn);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Prevent_Opportunity_Fire", &GameObjectWrapper::Prevent_Opportunity_Fire);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Hint", &GameObjectWrapper::Get_Hint);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Set_Cannot_Be_Killed", &GameObjectWrapper::Set_Cannot_Be_Killed);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Play_SFX_Event", &GameObjectWrapper::Play_SFX_Event);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Force_Test_Space_Conflict", &GameObjectWrapper::Force_Test_Space_Conflict);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Hide", &GameObjectWrapper::Hide);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Face_Immediate", &GameObjectWrapper::Face_Immediate);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Reset_Ability_Counter", &GameObjectWrapper::Reset_Ability_Counter);	
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Prevent_All_Fire", &GameObjectWrapper::Prevent_All_Fire);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Disable_Capture", &GameObjectWrapper::Disable_Capture);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Suspend_Locomotor", &GameObjectWrapper::Suspend_Locomotor);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Explore_Area", &GameObjectWrapper::Explore_Area);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Highlight", &GameObjectWrapper::Highlight);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Highlight_Small", &GameObjectWrapper::Highlight_Small);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Show_Emitter", &GameObjectWrapper::Show_Emitter);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Has_Attack_Target", &GameObjectWrapper::Has_Attack_Target);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Stop", &GameObjectWrapper::Stop);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Override_Max_Speed", &GameObjectWrapper::Override_Max_Speed);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Attach_Particle_Effect", &GameObjectWrapper::Attach_Particle_Effect);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Planet_Location", &GameObjectWrapper::Get_Planet_Location);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "In_End_Cinematic", &GameObjectWrapper::In_End_Cinematic);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Stop_SFX_Event", &GameObjectWrapper::Stop_SFX_Event);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Play_Cinematic_Engine_Flyby", &GameObjectWrapper::Play_Cinematic_Engine_Flyby);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Is_Planet_AI_Usable", &GameObjectWrapper::Get_Is_Planet_AI_Usable);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Enable_Behavior", &GameObjectWrapper::Enable_Behavior);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Cancel_Ability", &GameObjectWrapper::Cancel_Ability);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Can_Land_On_Planet", &GameObjectWrapper::Can_Land_On_Planet);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Set_Check_Contested_Space", &GameObjectWrapper::Set_Check_Contested_Space);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Attack_Target", &GameObjectWrapper::Get_Attack_Target);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Garrison", &GameObjectWrapper::Garrison);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Can_Garrison", &GameObjectWrapper::Can_Garrison);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Can_Garrison_Fire", &GameObjectWrapper::Can_Garrison_Fire);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Leave_Garrison", &GameObjectWrapper::Leave_Garrison);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Eject_Garrison", &GameObjectWrapper::Eject_Garrison);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Has_Garrison", &GameObjectWrapper::Has_Garrison);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Garrisoned_Units", &GameObjectWrapper::Get_Garrisoned_Units);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Good_Against", &GameObjectWrapper::Is_Good_Against);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Should_Switch_Weapons", &GameObjectWrapper::Should_Switch_Weapons);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_Current_Projectile_Type", &GameObjectWrapper::Get_Current_Projectile_Type);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Selectable", &GameObjectWrapper::Is_Selectable);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Ability_Autofire", &GameObjectWrapper::Is_Ability_Autofire);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Get_All_Projectile_Types", &GameObjectWrapper::Get_All_Projectile_Types);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Set_In_Limbo", &GameObjectWrapper::Set_In_Limbo);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_In_Garrison", &GameObjectWrapper::Is_In_Garrison);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Invade", &GameObjectWrapper::Invade);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Can_Move", &GameObjectWrapper::Can_Move);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Enable_Dynamic_LOD", &GameObjectWrapper::Enable_Dynamic_LOD);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Force_Ability_Recharge", &GameObjectWrapper::Force_Ability_Recharge);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectWrapper, "Is_Corrupted", &GameObjectWrapper::Is_Corrupted);
}

GameObjectWrapper::~GameObjectWrapper()
{
	Remove_Cached_Wrapper();
}

void GameObjectWrapper::Init(GameObjectClass *object)
{
	assert(!Object);
	Object = object;
	if (Object && !Object->Is_Delete_Pending())
	{
		SignalDispatcherClass::Get().Add_Listener(Object, this, PG_SIGNAL_OBJECT_DELETE_PENDING);
		Position = PositionWrapper::Create(Object->Get_Position());
	}
	else
	{
		Remove_Cached_Wrapper();
		Object = NULL;
	}
}

LuaTable* GameObjectWrapper::Lua_Is_Transport(LuaScriptClass * /*script*/, LuaTable * /*params*/)
{
	bool ret = false;

	if (Object) 
		ret = (Object->Behaves_Like(BEHAVIOR_TRANSPORT) != 0);
	
	return Return_Variable(new LuaBool(ret));
}

LuaTable* GameObjectWrapper::Release(LuaScriptClass *script, LuaTable * /*params*/)
{
	script;
	if (Object)
	{
		SignalDispatcherClass::Get().Remove_Listener(Object, this, PG_SIGNAL_OBJECT_DELETE_PENDING);
	}
	Remove_Cached_Wrapper();
	Object = 0;
	return NULL;
}

GameObjectClass *GameObjectWrapper::Get_Object(void) const
{
	return Object;
}

void GameObjectWrapper::To_String(std::string &outstr)
{
	if (!Object)
	{
		outstr = "NULL";
	}
	else
	{
		String_Printf(outstr, "GameObject: %8.8X, Type: %s", 
						(void *)Object, Object->Get_Original_Object_Type()->Get_Name()->c_str());
	}
}

bool GameObjectWrapper::Save(LuaScriptClass *script, ChunkWriterClass *writer) 
{
	assert(writer != NULL);
	bool ok = true;

	assert(Persistable);
	int range_count = ObjectInRangeList.size();

	writer->Begin_Chunk(GAMEOBJECTWRAPPER_DATA_CHUNK);
		WRITE_MICRO_CHUNK_MULTI_BASE_PTR					(SIGNAL_LISTENER_BASE_MICRO_CHUNK, 	SignalListenerClass);
		WRITE_MICRO_CHUNK_SMART_PTR						(GAMEOBJECTID_MICRO_CHUNK,				Object);
		WRITE_MICRO_CHUNK										(IN_RANGE_LIST_SIZE_MICRO_CHUNK,		range_count);

	writer->End_Chunk();

	for (int i = 0; i < (int)ObjectInRangeList.size(); i++)
	{
		writer->Begin_Chunk(IN_RANGE_ENTRY_CHUNK);
			LUA_WRITE_CHUNK_VALUE_PTR						(ENTRY_DISTANCE_CHUNK,					ObjectInRangeList[i].Distance, 		script);
			LUA_WRITE_CHUNK_VALUE_PTR						(ENTRY_FUNCTION_CHUNK,					ObjectInRangeList[i].Function,		script);
			LUA_WRITE_CHUNK_VALUE_PTR						(ENTRY_TYPE_CHUNK,						ObjectInRangeList[i].Type, 			script);
			LUA_WRITE_CHUNK_VALUE_PTR						(ENTRY_PLAYER_CHUNK,						ObjectInRangeList[i].Player,			script);
		writer->End_Chunk();
	}

	LUA_WRITE_CHUNK_VALUE_PTR								(OBJECT_POSITION_CHUNK,					Position,									script);

	return (ok);
}

bool GameObjectWrapper::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;

	int range_idx = 0;
	int range_count = 0;
	ObjectInRangeList.resize(0);

	while (reader->Open_Chunk()) {
		switch (reader->Cur_Chunk_ID()) {

			LUA_READ_CHUNK_VALUE_PTR								(OBJECT_POSITION_CHUNK,					Position,									script);
			case GAMEOBJECTWRAPPER_DATA_CHUNK:
				while (reader->Open_Micro_Chunk()) {
					switch (reader->Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK_MULTI_BASE_PTR				(SIGNAL_LISTENER_BASE_MICRO_CHUNK, 	SignalListenerClass);
						READ_MICRO_CHUNK_SMART_PTR						(GAMEOBJECTID_MICRO_CHUNK,				Object);
						READ_MICRO_CHUNK									(IN_RANGE_LIST_SIZE_MICRO_CHUNK,		range_count);
						default: assert(false); break;   // Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				ObjectInRangeList.resize(range_count);
				break;

			case IN_RANGE_ENTRY_CHUNK:
				while (reader->Open_Chunk()) {
					switch (reader->Cur_Chunk_ID()) {
						LUA_READ_CHUNK_VALUE_PTR						(ENTRY_DISTANCE_CHUNK,					ObjectInRangeList[range_idx].Distance,		script);
						LUA_READ_CHUNK_VALUE_PTR						(ENTRY_FUNCTION_CHUNK,					ObjectInRangeList[range_idx].Function,		script);
						LUA_READ_CHUNK_VALUE_PTR						(ENTRY_TYPE_CHUNK,						ObjectInRangeList[range_idx].Type,			script);
						LUA_READ_CHUNK_VALUE_PTR						(ENTRY_PLAYER_CHUNK,						ObjectInRangeList[range_idx].Player,		script);
						default: assert(false); break;   // Unknown Chunk
					}
					reader->Close_Chunk();
				}
				range_idx++;
				break;

			default: assert(false); break;   // Unknown Chunk
		}
		reader->Close_Chunk();
	}

	Script = script;
	SaveLoadClass::Register_Post_Load_Callback(Post_Load_Member_Callback<GameObjectWrapper>, this);

	assert(range_count == range_idx);
	return (ok);
}

void GameObjectWrapper::Post_Load_Callback(void)
{
	if (Object && Script)
	{
		WrapperCache->insert(std::make_pair(std::make_pair(Object, Script), this));
	}
}

void GameObjectWrapper::Debug_Validate_Wrapper_Cache(void)
{
#if 0
	WrapperCacheType::iterator it = WrapperCache->begin();
	for ( ; it != WrapperCache->end(); it++)
	{
		GameObjectClass *object = it->first.first;
		LuaScriptClass *script = it->first.second;
		GameObjectWrapper *wrapper = it->second;

		assert(wrapper->Script == script);
		assert(wrapper->Object == object);
	}
#endif
}

void GameObjectWrapper::Remove_Cached_Wrapper(void)
{
	if (Script)
	{
		WrapperCacheType::iterator it = WrapperCache->find(std::make_pair((GameObjectClass *)Object, Script));

		//The assert (ENFORCED_IF) we used to have here was hitting in all sorts of invalid cases.  It's quite
		//possible to call this function after the wrapper has already been removed from the cache in cases 
		//where the internal GameObject dies before the wrapper.
		assert(Object == NULL || it != WrapperCache->end());

		if (it != WrapperCache->end())
		{
			ENFORCED_IF(it->second == this)
			{
				WrapperCache->erase(it);
				Script = 0;
			}
		}
	}
}

void GameObjectWrapper::Init_Wrapper_Cache(void)
{
	if (!WrapperCache)
	{
		WrapperCache = new WrapperCacheType();
	}
}

void GameObjectWrapper::Shutdown_Wrapper_Cache(void)
{
	if (WrapperCache)
	{
		delete WrapperCache;
		WrapperCache = NULL;
	}
}

GameObjectWrapper *GameObjectWrapper::Create(GameObjectClass *obj, LuaScriptClass *script, bool persistable)
{
	FAIL_IF (obj && obj->Is_Delete_Pending())
	{
		obj = 0;
	}
	Debug_Validate_Wrapper_Cache();
	GameObjectWrapper *gobj;
	if (script)
	{
		std::pair<WrapperCacheType::iterator, bool> retval = WrapperCache->insert(std::make_pair(std::make_pair(obj, script), (GameObjectWrapper *)NULL));
		if (retval.second)
		{
			gobj = (GameObjectWrapper *) FactoryCreate();
			gobj->Init(obj);
			retval.first->second = gobj;
			gobj->Script = script;
		}
		else
		{
			gobj = retval.first->second;
			assert(gobj);
			assert(gobj->Script == script);
			assert(gobj->Object == obj);
		}
	}
	else
	{
		gobj = (GameObjectWrapper *) FactoryCreate();
		gobj->Init(obj);
	}

	gobj->Persistable = persistable;

	return gobj;
}

LuaTable *GameObjectWrapper::Get_Hull(LuaScriptClass *script, LuaTable *)
{
	if (!GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical())
	{
		script->Script_Error("GameObjectWrapper::Get_Hull -- this command is only supported in tactical modes.");
		return 0;
	}

	FAIL_IF (!Object) { return NULL; }

	return Return_Variable(new LuaNumber(Object->Get_Display_Health_Percent()));
}

LuaTable *GameObjectWrapper::Get_Final_Blow_Player(LuaScriptClass *script, LuaTable *)
{
	if (!Object)
	{
		return NULL;
	}

	PlayerClass *killer = NULL;
	int owner_id = Object->Get_Final_Blow_Player_ID();
	if (owner_id != -1)
	{
		killer = PlayerList.Get_Player_By_ID(owner_id);
	}
	else
	{
		killer = PlayerList.Get_Neutral_Player();
	}

	if (killer)
	{
		return Return_Variable(PlayerWrapper::Create(killer, script));
	}
	return NULL;
}

LuaTable *GameObjectWrapper::Get_Owner(LuaScriptClass *script, LuaTable *)
{
	Debug_Validate_Wrapper_Cache();
	if (!Object)
	{
		return NULL;
	}
	int owner_id = Object->Get_Owner();

	if (Object->Get_Behavior(BEHAVIOR_PLANET))
	{
		owner_id = Object->Get_Allegiance().Get_Current_Player_ID();
	}

	PlayerClass *owner = PlayerList.Get_Player_By_ID(owner_id);
	if (owner)
	{
		return Return_Variable(PlayerWrapper::Create(owner, script));
	}
	return NULL;
}


/**
 * Get the Game Scoring specific type associated with this object.
 * 
 * @param script lua script
 * 
 * @return GameObjectTypeWrapper
 * @since 12/4/2005 4:00:05 PM -- BMH
 */
LuaTable *GameObjectWrapper::Lua_Get_Game_Scoring_Type(LuaScriptClass *script, LuaTable *)
{
	if (Object)
	{
		const GameObjectTypeClass *retval = NULL;
		TeamBehaviorClass *tbehave = (TeamBehaviorClass *)Object->Get_Behavior(BEHAVIOR_TEAM);
		if (tbehave)
		{
			retval = tbehave->Get_Company_Type();
		}

		if (!retval && Object->Get_Behavior(BEHAVIOR_TRANSPORT))
		{
			TransportBehaviorClass *transport_behavior = static_cast< TransportBehaviorClass * >( Object->Get_Behavior( BEHAVIOR_TRANSPORT ) );
			retval = transport_behavior->Get_Dummy_Company_Object_Type( Object );
			if (!retval && Object->Get_Parent_Mode_ID() != INVALID_OBJECT_ID && GameModeManager.Get_Active_Mode())
			{
				GameModeClass *pmode = GameModeManager.Get_Parent_Game_Mode(GameModeManager.Get_Active_Mode());
				if (pmode)
				{
					GameObjectClass *tobj = pmode->Get_Object_Manager().Get_Object_From_ID(Object->Get_Parent_Mode_ID());
					if (tobj)
					{
						retval = tobj->Get_Original_Object_Type();
					}
				}
			}
		}

		if (!retval)
		{
			retval = Object->Get_Original_Object_Type();
		}
		return Return_Variable(GameObjectTypeWrapper::Create(const_cast<GameObjectTypeClass *>(retval), script));
	}
	return NULL;
}


LuaTable *GameObjectWrapper::Lua_Get_Type(LuaScriptClass *script, LuaTable *)
{
	Debug_Validate_Wrapper_Cache();
	if (Object)
	{
		return Return_Variable(GameObjectTypeWrapper::Create(const_cast<GameObjectTypeClass *>(Object->Get_Original_Object_Type()), script));
	}
	return NULL;
}

LuaTable *GameObjectWrapper::Lua_Set_Prefer_Ground_Over_Space(LuaScriptClass *script, LuaTable *params)
{
	if (!params || params->Value.size() == 0)
	{
		script->Script_Error("GameObjectWrapper::Lua_Set_Prefer_Ground_Over_Space -- Missing required boolean parameter.");
		return NULL;
	}

	LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);

	if (Object)
	{
		UnitAIBehaviorClass *behave = (UnitAIBehaviorClass *)Object->Get_Behavior(BEHAVIOR_UNIT_AI);
		if (behave)
		{
			behave->Set_Prefer_Ground_Over_Space(bval->Value);
			return NULL;
		}
	}
	script->Script_Error("GameObjectWrapper::Lua_Set_Prefer_Ground_Over_Space -- Object invalid or missing Unit_AI behavior.");
	return NULL;
}

LuaTable *GameObjectWrapper::Get_Shield(LuaScriptClass *script, LuaTable *)
{
	if (!GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical())
	{
		script->Script_Error("GameObjectWrapper::Get_Shield -- this command is only supported in tactical modes.");
		return 0;
	}

	if (!Object)
	{
		return Return_Variable(new LuaNumber(0.0f));
	}

	return Return_Variable(new LuaNumber(Object->Get_Shield_Percent()));
}

LuaTable *GameObjectWrapper::Get_Energy(LuaScriptClass *script, LuaTable *)
{
	if (!GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical())
	{
		script->Script_Error("GameObjectWrapper::Get_Energy -- this command is only supported in tactical modes.");
		return 0;
	}

	if (!Object)
	{
		return Return_Variable(new LuaNumber(0.0f));
	}

	return Return_Variable(new LuaNumber(Object->Get_Energy_Percent()));
}

LuaTable *GameObjectWrapper::Is_Category(LuaScriptClass *script, LuaTable *params)
{
	if (!Object)
	{
		return 0;
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Is_Category -- invalid number of parameters. Expected 1, got %d.", params->Value.size());
		return 0;
	}

	SmartPtr<LuaString> category_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!category_name)
	{
		script->Script_Error("GameObjectWrapper::Is_Category -- parameter 1 is not a valid string.");
		return 0;
	}

	GameObjectCategoryType category;
	if (!TheGameObjectCategoryTypeConverterPtr->String_To_Enum(category_name->Value, category))
	{
		script->Script_Error("GameObjectWrapper::Is_Category -- unrecognized category %s.", category_name->Value.c_str());
		return 0;
	}

	return Return_Variable(new LuaBool((Object->Get_Original_Object_Type()->Get_Category_Mask() & category) != 0));
}

LuaTable *GameObjectWrapper::Get_Parent_Object(LuaScriptClass *script, LuaTable *)
{
	if (!Object)
	{
		return 0;
	}

	if (!Object->Get_Parent_Container_Object())
	{
		return 0;
	}

	return Return_Variable(GameObjectWrapper::Create(Object->Get_Parent_Container_Object(), script));
}

LuaTable *GameObjectWrapper::Attack_Target(LuaScriptClass *script, LuaTable *params)
{
	//This is intentionally much less flexible than the taskforce version and does not return a blocking object.
	//It is intended primarily for event response.  Note that there's no safety check as to whether this script
	//is permitted to give attack orders to this object.  Until I work out some clean way of validating this we'll
	//just have to be careful not to abuse the command.

	if (!GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical())
	{
		script->Script_Error("GameObjectWrapper::Attack_Target -- this command is only supported in tactical modes.");
		return 0;
	}

	if (params->Value.size() < 1)
	{
		script->Script_Error("GameObjectWrapper::Attack_Target -- invalid number of parameters.  Expected at least 1, got %d.", params->Value.size());
		return 0;
	}

	if (!Object)
	{
		return 0;
	}

	if (Object->Is_Movement_Locked())
	{
		script->Script_Warning("GameObjectWrapper::Attack_Target -- object %s is locked into existing orders. Attack_Target fails.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return 0;
	}

	int pidx = 0;

	LuaTable::Pointer units = PG_Dynamic_Cast<LuaTable>(params->Value[pidx]);
	if (units)
	{
		pidx++;
	}

	if ((int)params->Value.size() <= pidx)
	{
		script->Script_Error("GameObjectWrapper::Attack_Target -- invalid number of parameters.  Expected %d, got %d.", pidx+1, params->Value.size());
		return 0;
	}

	AITargetLocationWrapper *dest = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[pidx]);
	GameObjectWrapper *dest_object = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[pidx]);

	if (!dest && !dest_object) 
	{
		script->Script_Error("GameObjectWrapper::Attack_Target -- Parameter 1 is not a valid destination for unit %s", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return NULL;
	}

	if ((dest && !dest->Get_Object()) || (dest_object && !dest_object->Get_Object()))
	{
		script->Script_Error("GameObjectWrapper::Attack_Target -- Target is already dead.");
		return 0;
	}

	if (!units)
	{
		units = Alloc_Lua_Table();
		units->Value.push_back(this);
	}

	GameObjectClass *target = 0;

	if (dest)
	{
		target = dest->Get_Object()->Get_Target_Game_Object();
	}
	else
	{
		target = dest_object->Get_Object();
	}

	if (!target)
	{
		script->Script_Error("GameObjectWrapper::Attack_Target -- destination is not valid for an attack command.");
		return 0;
	}

	MovementCoordinatorSystemClass *system = GameModeManager.Get_Active_Mode()->Get_Movement_Coordinator_System();
	MovementCoordinatorClass *coordinator = 0;

	if (!system)
	{
		script->Script_Error("GameObjectWrapper::Attack_Target -- unable to locate movement coordinator system.  This is very bad.");
		return 0;
	}

	coordinator = system->Create();

	if (!coordinator)
	{
		script->Script_Error("GameObjectWrapper::Attack_Target -- unable to create a movement coordinator for this attack.  This is also very bad.");
		return 0;
	}

	//
	// AJA 06/12/2006 - Copying Steve's fix for Lua Table order predictability here, since the
	// code below iterates a Lua Table and calls functions that may put the game state out of sync.
	// Lua table iteration is not multiplayer friendly since the tables are hashed by pointer value.
	// Here, we turn the table into a vector of gameobject pointers and sort the vector by object ID.
	//
  	//Translate the lua table into real game objects
  	static std::vector<GameObjectWrapper*> object_list;
  	object_list.resize(0);
  	for (unsigned int i = 0; i < units->Value.size(); ++i)
  	{
  		SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(units->Value[i]);
  		if (!object_wrapper)
  		{
  			script->Script_Warning("Find_Best_Local_Threat_Center -- invalid type for entry at index %d in object table.", i);
  			continue;
  		}
  
  		if (!object_wrapper->Get_Object())
  		{
  			//We'll treat this as resonable
  			continue;
  		}
  
  		object_list.push_back(object_wrapper);
		FrameSynchronizerClass::Print_Sync_Message_No_Stack(SYNC_LOG_LUA_CRC, "GameObjectWrapper::Attack_Target -- Added object ID %d\n", object_wrapper->Get_Object()->Get_ID());
	}
 	for (unsigned int bub1=0 ; bub1 < object_list.size() ; bub1++) {
 		for (unsigned int bub2=0 ; bub2 < object_list.size() - 1 ; bub2++) {
 			if (object_list[bub2]->Get_Object()->Get_ID() > object_list[bub2+1]->Get_Object()->Get_ID()) {
 				GameObjectWrapper *temp = object_list[bub2];
 				object_list[bub2] = object_list[bub2 + 1];
 				object_list[bub2 + 1] = temp;
 			}
 		}
 	}

	LuaTable *moved_units = Alloc_Lua_Table();
	std::vector<GameObjectWrapper*>::iterator it = object_list.begin();
	while ( it != object_list.end() )
	{
		GameObjectWrapper *object_wrapper = *it;
		FAIL_IF( object_wrapper == NULL )		{ ++it; continue; }
		GameObjectClass *object = object_wrapper->Get_Object();
		FAIL_IF( object == NULL )					{ ++it; continue; }

		object->Clear_Ability_State_For_Move();

		if ( object->Attack_Object(target, 0, true) == true )
		{
			if (system->Can_Object_Be_In_Formations(object))
			{
				system->Remove_Object_From_Coordinators(object);
	
				//Notify any blocking objects waiting on this object's current move - we'll behave as if it's finished since
				//it's now doing something completely different.
				SignalDispatcherClass::Get().Send_Signal(object, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL);
	
				coordinator->Add_Object(object);
				moved_units->Value.push_back(object_wrapper);
			}
			else if (object->Behaves_Like(BEHAVIOR_TEAM))
			{
				TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(object->Get_Behavior(BEHAVIOR_TEAM));
				FAIL_IF(!team) { return NULL; }
	
				//Notify any blocking objects waiting on this object's current move - we'll behave as if it's finished since
				//it's now doing something completely different.
				SignalDispatcherClass::Get().Send_Signal(object, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL);
	
				moved_units->Value.push_back(object_wrapper);
				for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
				{
					GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
					FAIL_IF(!team_member) { continue; }
	
					system->Remove_Object_From_Coordinators(team_member);
					SignalDispatcherClass::Get().Send_Signal(team_member, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL);
					coordinator->Add_Object(team_member);
				}
			}
			else
			{
				script->Script_Warning("GameObjectWrapper::Attack_Target -- unable to make object %s move.", object->Get_Type()->Get_Name()->c_str());
			}
		}
		else
		{
			script->Script_Warning("GameObjectWrapper::Attack_Target -- object %s unable to attack %s.", object->Get_Type()->Get_Name()->c_str(), 
										  target->Get_Type()->Get_Name()->c_str());
		}

		++it;
	}



	UnitMovementBlockStatus *bs = (UnitMovementBlockStatus *)UnitMovementBlockStatus::FactoryCreate();
	bs->Init(this, moved_units, target);

	FormationDestinationStruct destination;
	destination.Type = FDT_OBJECT;
	destination.Target = target;
	destination.Hardpoint = 0;
	destination.EnforceTopSpeed = false;

	coordinator->Set_Destination(destination);
	return Return_Variable(bs);
}

/**************************************************************************************************
* GameObjectWrapper::Is_Equal -- 
*
* In:				
*
* Out:		
*
* History: 11/11/2004 4:16PM JSY
**************************************************************************************************/
bool GameObjectWrapper::Is_Equal(const LuaVar *var) const
{
	SmartPtr<GameObjectWrapper> other_object = PG_Dynamic_Cast<GameObjectWrapper>(const_cast<LuaVar*>(var));
	if (!other_object)
	{
		return false;
	}

	return other_object->Get_Object() == Object;
}

void GameObjectWrapper::Receive_Signal(SignalGeneratorClass *, PGSignalType signal_type, SignalDataClass *)
{
	switch (signal_type)
	{
	case PG_SIGNAL_OBJECT_DELETE_PENDING:
		SignalDispatcherClass::Get().Remove_Listener(Object, this, PG_SIGNAL_OBJECT_DELETE_PENDING);
		SignalDispatcherClass::Get().Remove_Listener(Object, this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED);
		SignalDispatcherClass::Get().Remove_Listener(Object, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED);
		Remove_Cached_Wrapper();
		Object = 0;
		break;

	default:
		assert(false);
	}
}


/**************************************************************************************************
* GameObjectWrapper::Get_Rate_Of_Damage_Taken -- Script function to get the damage per second we were
*	suffering last we checked.
*
* In:				
*
* Out:		
*
* History: 2/16/2005 10:22AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Rate_Of_Damage_Taken(LuaScriptClass *script, LuaTable *)
{
	float damage_rate = 0.0f;

	if (!Object)
	{
		script->Script_Warning("GameObjectWrapper::Get_Rate_Of_Damage_Taken - object is already dead.");
	}
	else if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		//Desired behavior for teams is to sum the values for team members
		damage_rate = 0.0f;
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return 0; }
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			if (!team_member || !team_member->Behaves_Like(BEHAVIOR_DAMAGE_TRACKING))
			{
				continue;
			}
			DamageTrackingBehaviorClass *damage_tracking = static_cast<DamageTrackingBehaviorClass*>(team_member->Get_Behavior(BEHAVIOR_DAMAGE_TRACKING));
			FAIL_IF(!damage_tracking) { continue; }
			
			damage_rate += damage_tracking->Get_Rate_Of_Damage_Taken();
		}
	}
	else
	{
		DamageTrackingBehaviorClass *damage_tracking = static_cast<DamageTrackingBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_DAMAGE_TRACKING));
		if (damage_tracking)
		{
			damage_rate = damage_tracking->Get_Rate_Of_Damage_Taken();
		}
	}

	return Return_Variable(new LuaNumber(damage_rate));
}

/**************************************************************************************************
* GameObjectWrapper::Get_Time_Till_Dead -- Script function to get the time we estimate we have left
*	before we die
*
* In:				
*
* Out:		
*
* History: 2/16/2005 10:22AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Time_Till_Dead(LuaScriptClass *script, LuaTable *)
{
	float time_till_dead = BIG_FLOAT;

	if (!Object)
	{
		script->Script_Warning("GameObjectWrapper::Get_Time_Till_Dead - object is already dead.");
		time_till_dead = 0.0f;
	}
	else if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		//Desired behavior for teams is to sum the values for team members
		time_till_dead = 0.0f;
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return 0; }
		float total_health = 0.0f;
		float total_damage_rate = 0.0f;
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			if (!team_member || !team_member->Behaves_Like(BEHAVIOR_DAMAGE_TRACKING))
			{
				continue;
			}

			total_health += team_member->Get_Health();

			DamageTrackingBehaviorClass *damage_tracking = static_cast<DamageTrackingBehaviorClass*>(team_member->Get_Behavior(BEHAVIOR_DAMAGE_TRACKING));
			FAIL_IF(!damage_tracking) { continue; }
			
			total_damage_rate += damage_tracking->Get_Rate_Of_Damage_Taken();
		}

		time_till_dead = (total_damage_rate == 0.0f ? BIG_FLOAT : total_health / total_damage_rate);
	}
	else
	{
		DamageTrackingBehaviorClass *damage_tracking = static_cast<DamageTrackingBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_DAMAGE_TRACKING));
		if (damage_tracking)
		{
			time_till_dead = damage_tracking->Get_Estimated_Time_Till_Death();
		}
	}

	return Return_Variable(new LuaNumber(time_till_dead));
}

/**************************************************************************************************
* GameObjectWrapper::Set_Targeting_Priorities -- Script function to change the targeting priority set that
*	this unit uses
*
* In:				
*
* Out:		
*
* History: 2/16/2005 10:22AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Set_Targeting_Priorities(LuaScriptClass *script, LuaTable *params)
{
	if (!Object)
	{
		script->Script_Warning("GameObjectWrapper::Set_Targeting_Priorities - object is already dead.");
		return NULL;
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Set_Targeting_Priorities - invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> priority_set_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!priority_set_name)
	{
		script->Script_Error("GameObjectWrapper::Set_Targeting_Priorities - invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	TargetingPrioritySetClass *priority_set = TheTargetingPrioritySetManagerPtr->Get_Managed_Object(priority_set_name->Value);
	if (!priority_set)
	{
		script->Script_Error("GameObjectWrapper::Set_Targeting_Priorities - unknown targeting priority set %s.", priority_set_name->Value.c_str());
		return NULL;
	}

	TargetingInterfaceClass *targeting_behave = static_cast<TargetingInterfaceClass*>(Object->Get_Behavior(BEHAVIOR_TARGETING));
	if (targeting_behave)
	{
		targeting_behave->Set_Targeting_Priorities(priority_set);
	}

	if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }
			targeting_behave = static_cast<TargetingInterfaceClass*>(team_member->Get_Behavior(BEHAVIOR_TARGETING));
			if (targeting_behave)
			{
				targeting_behave->Set_Targeting_Priorities(priority_set);
			}
		}
	}
	
	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Set_Targeting_Stickiness_Time_Threshold -- Script function to change the time threshold
*	at which this unit will prefer to continue firing at a current target which is expected to die shortly rather
*	than switch to a higher priority target.
*
* In:				
*
* Out:		
*
* History: 2/16/2005 10:22AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Set_Targeting_Stickiness_Time_Threshold(LuaScriptClass *script, LuaTable *params)
{
	if (!Object)
	{
		script->Script_Warning("GameObjectWrapper::Set_Targeting_Stickiness_Time_Threshold - object is already dead.");
		return NULL;
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Set_Targeting_Stickiness_Time_Threshold - invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaNumber> time_threshold = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!time_threshold)
	{
		script->Script_Error("GameObjectWrapper::Set_Targeting_Stickiness_Time_Threshold - invalid type for parameter 1.  Expected number.");
		return NULL;
	}

	TargetingInterfaceClass *targeting_behave = static_cast<TargetingInterfaceClass*>(Object->Get_Behavior(BEHAVIOR_TARGETING));
	if (targeting_behave)
	{
		targeting_behave->Set_Targeting_Stickiness_Time_Threshold(time_threshold->Value);
	}

	if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }
			targeting_behave = static_cast<TargetingInterfaceClass*>(team_member->Get_Behavior(BEHAVIOR_TARGETING));
			if (targeting_behave)
			{
				targeting_behave->Set_Targeting_Stickiness_Time_Threshold(time_threshold->Value);
			}
		}
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Move_To -- Execute a move to a position (possibly defined by an object or taskforce)
*
* In:				
*
* Out:		
*
* History: 2/23/2005 2:07PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Move_To(LuaScriptClass *script, LuaTable *params)
{
	if (GameModeManager.Get_Sub_Type() == SUB_GAME_MODE_GALACTIC)
	{
		return Move_Galactic(script, params);
	}
	else
	{
		return Move_Internal(script, params, 0);
	}
}

/**************************************************************************************************
* GameObjectWrapper::Guard_Target -- Guard a position object or taskforce
*
* In:				
*
* Out:		
*
* History: 2/23/2005 2:07PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Guard_Target(LuaScriptClass *script, LuaTable *params)
{
	return Move_Internal(script, params, UNIT_AI_MOVE_TYPE_ESCORT);
}

/**************************************************************************************************
* GameObjectWrapper::Attack_Move -- Execute a move to a position (possibly defined by an object or taskforce).
*	Engage enemies encountered while travelling.
*
* In:				
*
* Out:		
*
* History: 2/23/2005 2:07PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Attack_Move(LuaScriptClass *script, LuaTable *params)
{
	return Move_Internal(script, params, UNIT_AI_MOVE_TYPE_ATTACK_ON_PATH);
}

/**************************************************************************************************
* GameObjectWrapper::Move_Internal -- Do all the work involved in executing a move, attack-move or guard
*
* In:				
*
* Out:		
*
* History: 2/23/2005 2:08PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Move_Internal(LuaScriptClass *script, LuaTable *params, unsigned int move_flags)
{
	if (!GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical())
	{
		script->Script_Error("GameObjectWrapper::Move_Internal -- this command is only supported in tactical modes.");
		return 0;
	}

	if (params->Value.size() == 0)
	{
		script->Script_Error("GameObjectWrapper::Move_Internal -- invalid number of parameters.  Expected 1 or 2, got %d.", params->Value.size());
		return 0;
	}

	if (!Object)
	{
		script->Script_Error("GameObjectWrapper::Move_Internal -- this object is already dead.");
		return 0;
	}

	if (Object->Is_Movement_Locked())
	{
		script->Script_Warning("GameObjectWrapper::Move_Internal -- object %s is locked into existing orders. Attack_Target fails.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return 0;
	}

	int pidx = 0;

	LuaTable::Pointer units = PG_Dynamic_Cast<LuaTable>(params->Value[pidx]);
	if (units)
	{
		pidx++;
	}

	if ((int)params->Value.size() <= pidx)
	{
		script->Script_Error("GameObjectWrapper::Move_Internal -- invalid number of parameters.  Expected %d, got %d.", pidx+1, params->Value.size());
		return 0;
	}

	//A target may be defined in several possible ways.
	SmartPtr<AITargetLocationWrapper> dest = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[pidx]);
	SmartPtr<GameObjectWrapper> dest_object = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[pidx]);
	SmartPtr<TaskForceClass> tf = PG_Dynamic_Cast<TaskForceClass>(params->Value[pidx]);
	SmartPtr<PositionWrapper> position_wrapper = PG_Dynamic_Cast<PositionWrapper>(params->Value[pidx]);

	if (!dest && !dest_object && !tf && !position_wrapper) 
	{
		script->Script_Error("GameObjectWrapper::Move_Internal -- Parameter 1 is not a valid destination for unit %s", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return 0;
	}

	if ((dest && !dest->Get_Object()) || (dest_object && !dest_object->Get_Object()))
	{
		script->Script_Error("GameObjectWrapper::Move_Internal -- Target is already dead.");
		return 0;
	}

	//In the case of a guard we'll actually guard the object passed; otherwise everything get's converted to a position.
	GameObjectClass *target = 0;
	Vector3 target_position = VECTOR3_NONE;
	int object_target_flags = (UNIT_AI_MOVE_TYPE_ESCORT | UNIT_AI_MOVE_TYPE_ATTACK_ON_PATH);

	if (!units)
	{
		units = Alloc_Lua_Table();
		units->Value.push_back(this);
	}

	if (dest)
	{
		if ((move_flags & object_target_flags) != 0)
		{
			target = dest->Get_Object()->Get_Target_Game_Object();
			if (target == Object || target == Object->Get_Parent_Container_Object())
			{
				target = 0;
			}
		}

		if (!target)
		{
			target_position = dest->Get_Object()->Get_Target_Position();
		}
	}
	else if (dest_object)
	{
		if ((move_flags & object_target_flags) != 0)
		{
			target = dest_object->Get_Object();
			if (target == Object || target == Object->Get_Parent_Container_Object())
			{
				target = 0;
			}
		}

		if (!target)
		{
			target_position = dest_object->Get_Object()->Get_Position();;
		}
	}
	else if (tf)
	{
		//Can't be attacking a taskforce can we?
		if ((move_flags & UNIT_AI_MOVE_TYPE_ESCORT) != 0)
		{
			for (int i = 0; i < tf->Get_Force_Count(); ++i)
			{
				GameObjectClass *tf_member = tf->Get_Member(i)->Get_Object();
				if (tf_member != Object && tf_member != Object->Get_Parent_Container_Object())
				{
					target = tf_member;
					break;
				}
			}
		}

		if (!target)
		{
			target_position = tf->Get_Position();
		}
	}
	else if (position_wrapper)
	{
		target_position = position_wrapper->Get_Position();
	}

	++pidx;
	SpaceCollisionType space_avoidance_mask = SCT_ALL;
	if (GameModeManager.Get_Sub_Type() == SUB_GAME_MODE_SPACE && static_cast<int>(params->Value.size()) > pidx)
	{
		SmartPtr<LuaString> lua_avoidance = PG_Dynamic_Cast<LuaString>(params->Value[pidx]);
		if (lua_avoidance)
		{
			if (!TheSpaceCollisionTypeConverterPtr->String_To_Enum(lua_avoidance->Value, space_avoidance_mask))
			{
				script->Script_Error("GameObjectWrapper::Move_Internal -- unknown space collision type %s for avoidance parameter.", lua_avoidance->Value.c_str());
				return NULL;
			}
			space_avoidance_mask = static_cast<SpaceCollisionType>(~space_avoidance_mask);
			++pidx;
		}
	}

	//Support threat avoidance on unit moves
	float threat_tolerance = BIG_FLOAT;
	if (static_cast<int>(params->Value.size()) > pidx)
	{
		SmartPtr<LuaNumber> lua_threat_tolerance = PG_Dynamic_Cast<LuaNumber>(params->Value[pidx]);
		if (lua_threat_tolerance)
		{
			threat_tolerance = lua_threat_tolerance->Value;
		}
	}

	//Go ahead and execute the move.  No blocking object required.
	MovementCoordinatorSystemClass *system = GameModeManager.Get_Active_Mode()->Get_Movement_Coordinator_System();
	MovementCoordinatorClass *coordinator = 0;

	if (!system)
	{
		script->Script_Error("GameObjectWrapper::Move_Internal -- unable to locate movement coordinator system.  This is very bad.");
		return NULL;
	}

	coordinator = system->Create();

	if (!coordinator)
	{
		script->Script_Error("GameObjectWrapper::Move_Internal -- unable to create a movement coordinator for this attack.  This is also very bad.");
		return NULL;
	}

	for (int i = 0; i < (int)units->Value.size(); i++)
	{
		GameObjectWrapper *wrap = PG_Dynamic_Cast<GameObjectWrapper>(units->Value[i]);
		FAIL_IF (!wrap)
		{
			script->Script_Error("GameObjectWrapper::Move_Internal -- Entry %d in unit array is not a gameobject.", i);
			continue;
		}
		GameObjectClass *object = wrap->Get_Object();

		object->Clear_Ability_State_For_Move();

		if (system->Can_Object_Be_In_Formations(object))
		{
			system->Remove_Object_From_Coordinators(object);

			//Notify any blocking objects waiting on this object's current move - we'll behave as if it's finished since
			//it's now doing something completely different.
			SignalDispatcherClass::Get().Send_Signal(object, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL);

			coordinator->Add_Object(object);
		}
		else if (object->Behaves_Like(BEHAVIOR_TEAM))
		{
			TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(object->Get_Behavior(BEHAVIOR_TEAM));
			FAIL_IF(!team) { return NULL; }

			//Notify any blocking objects waiting on this object's current move - we'll behave as if it's finished since
			//it's now doing something completely different.
			SignalDispatcherClass::Get().Send_Signal(object, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL);

			for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
			{
				GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
				FAIL_IF(!team_member) { continue; }

				system->Remove_Object_From_Coordinators(team_member);
				SignalDispatcherClass::Get().Send_Signal(team_member, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL);
				coordinator->Add_Object(team_member);
			}
		}
		else
		{
			script->Script_Warning("GameObjectWrapper::Move_Internal -- unable to make object %s move.", object->Get_Type()->Get_Name()->c_str());
		}
	}

	if (coordinator->Get_Member_Count() == 0)
	{
		script->Script_Warning("GameObjectWrapper::Move_Internal -- failed to move any objects");
		return NULL;
	}

	UnitMovementBlockStatus *bs = (UnitMovementBlockStatus *)UnitMovementBlockStatus::FactoryCreate();
	bs->Init(this, units, target);

	FormationDestinationStruct destination;

	if (target)
	{
		destination.Type = FDT_OBJECT;
		destination.Target = target;
	}
	else
	{
		destination.Type = FDT_POSITION;
		destination.Position = target_position;
	}

	destination.EnforceTopSpeed = false;
	destination.MovementType.Set_Type(move_flags);
	destination.SpaceObstacleFilter = space_avoidance_mask;
	destination.ThreatTolerance = threat_tolerance;
	
	coordinator->Set_Destination(destination);

	return Return_Variable(bs);
}

/**************************************************************************************************
* GameObjectWrapper::Fire_Special_Weapon -- Script function to attempt to fire a special weapon.  Valid only
*	for objects that are special weapons.
*
* In:				
*
* Out:		
*
* History: 2/23/2005 4:35PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Fire_Special_Weapon(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Fire_Special_Weapon -- invalid number of parameters: expected 2, got %d.", params->Value.size());
		return 0;
	}

	if (!Object)
	{
		script->Script_Error("GameObjectWrapper::Fire_Special_Weapon -- this object is already dead.");
		return 0;
	}

	SpecialWeaponBehaviorClass *space_weapon_behavior = static_cast<SpecialWeaponBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_SPECIAL_WEAPON));
	LobbingSuperweaponBehaviorClass *land_weapon_behavior = static_cast<LobbingSuperweaponBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_LOBBING_SUPERWEAPON));

	if (!space_weapon_behavior && !land_weapon_behavior && !Object->Behaves_Like(BEHAVIOR_DUMMY_STAR_BASE))
	{
		script->Script_Error("GameObjectWrapper::Fire_Special_Weapon -- object %s is not a special weapon", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return 0;
	}

	SmartPtr<AITargetLocationWrapper> ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);
	SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);

	if (!ai_target_wrapper && !object_wrapper)
	{
		script->Script_Error("GameObjectWrapper::Fire_Special_Weapon -- invalid target for special weapon %s.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return 0;
	}

	if ((ai_target_wrapper && !ai_target_wrapper->Get_Object()) || (object_wrapper && !object_wrapper->Get_Object()))
	{
		script->Script_Error("GameObjectWrapper::Fire_Special_Weapon -- target for special weapon %s is already dead.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return 0;
	}

	SmartPtr<PlayerWrapper> firing_player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[1]);
	if (!firing_player || !firing_player->Get_Object())
	{
		script->Script_Error("GameObjectWrapper::Fire_Special_Weapon -- invalid type for parameter 2.  Expected player.");
		return NULL;
	}

	//Unfortunately the interface is a little different depending on exactly how the weapon behaves. 
	if (Object->Behaves_Like(BEHAVIOR_DUMMY_STAR_BASE))
	{
		//A manually targeted hard point on a space station
		HardPointClass *hard_point = Object->Find_Hardpoint_That_Requires_Manual_Target();
		if (!hard_point)
		{
			script->Script_Warning("Fire_Special_Weapon -- space station %s has no special weapon.", Object->Get_Type()->Get_Name()->c_str());
			return NULL;
		}

		if (hard_point->Get_Readiness_Percentage() < 1.0f)
		{
			script->Script_Warning("GameObjectWrapper::Fire_Special_Weapon -- special weapon %s is not ready.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
			return NULL;
		}

		GameModeClass *game_mode = GameModeManager.Get_Game_Mode_By_Sub_Type(SUB_GAME_MODE_SPACE);
		if (game_mode)
		{
			BattlefieldModifiersClass *mods = game_mode->Get_Battlefield_Modifiers(firing_player->Get_Object()->Get_ID());
			if (mods)
			{
				if (!mods->Is_Special_Weapon_Enabled())
				{
					script->Script_Warning("Fire_Special_Weapon -- special weapons not available.  Purchase a special weapon upgrade to enable.");
					return 0;
				}
			}
		}

		GameObjectClass *target = ai_target_wrapper ? ai_target_wrapper->Get_Object()->Get_Target_Game_Object() : object_wrapper->Get_Object();
		if (!hard_point->Is_Valid_Manual_Target(target))
		{
			script->Script_Warning("GameObjectWrapper::Fire_Special_Weapon -- special weapon %s unable to fire at target %s.", Object->Get_Original_Object_Type()->Get_Name()->c_str(), target->Get_Type()->Get_Name()->c_str());
			return NULL;
		}

		hard_point->Assign_Manual_Target(target, firing_player->Get_Object()->Get_ID());
	}
	else if (space_weapon_behavior)
	{
		//A weapon fictionally located in the background that fires into space
		if (ai_target_wrapper && !ai_target_wrapper->Get_Object()->Get_Target_Game_Object())
		{
			script->Script_Error("GameObjectWrapper::Fire_Special_Weapon -- target %s is not valid for special weapon %s: this weapon may only be fired at objects.", ai_target_wrapper->Get_Object()->Get_Name().c_str(), Object->Get_Original_Object_Type()->Get_Name()->c_str());
			return 0;
		}

		if (space_weapon_behavior->Get_Readiness_Percentage(Object) < 1.0f)
		{
			script->Script_Warning("GameObjectWrapper::Fire_Special_Weapon -- special weapon %s is not ready.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
			return 0;
		}

		if (!space_weapon_behavior->Activate_Targeting_Mode(Object))
		{
			script->Script_Error("GameObjectWrapper::Fire_Special_Weapon -- Cannot activate targeting for special weapon %s.  What's going on?", Object->Get_Original_Object_Type()->Get_Name()->c_str());
			return 0;
		}

		GameModeClass *game_mode = GameModeManager.Get_Game_Mode_By_Sub_Type(SUB_GAME_MODE_SPACE);
		if (game_mode)
		{
			BattlefieldModifiersClass *mods = game_mode->Get_Battlefield_Modifiers(firing_player->Get_Object()->Get_ID());
			if (mods)
			{
				if (!mods->Is_Special_Weapon_Enabled())
				{
					script->Script_Warning("Fire_Special_Weapon -- special weapons not available.  Purchase a special weapon upgrade to enable.");
					return 0;
				}
			}
		}

		GameObjectClass *target = ai_target_wrapper ? ai_target_wrapper->Get_Object()->Get_Target_Game_Object() : object_wrapper->Get_Object();

		if (!space_weapon_behavior->Fire_At_Target(Object, target))
		{
			script->Script_Error("GameObjectWrapper::Fire_Special_Weapon -- Unknown and confusing error firing special weapon %s: everything seemed ok, but the weapon failed to fire.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
			return 0;
		}
	}
	else
	{
		//A ground based grenade-like weapon
		if (!Object->Get_Special_Weapon_Data())
		{
			script->Script_Error("GameObjectWrapper::Fire_Special_Weapon -- Object %s claims to be a special weapon but doesn't have a special weapon data pack.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
			return 0;
		}

		if (Object->Get_Special_Weapon_Data()->Get_Readiness_Percentage() < 1.0f)
		{
			script->Script_Warning("GameObjectWrapper::Fire_Special_Weapon -- special weapon %s is not ready.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
			return 0;
		}

		Vector3 target_position = ai_target_wrapper ? ai_target_wrapper->Get_Object()->Get_Target_Position() : object_wrapper->Get_Object()->Get_Position();

		land_weapon_behavior->Attack_Position(target_position);
	}

	// Destroy any objects owned by the owner player that have the "destroy when special weapon fired" dummy behavior.
	const DynamicVectorClass<GameObjectClass*> *destroy_objects = GAME_OBJECT_MANAGER.Find_Objects(BEHAVIOR_DUMMY_DESTROY_AFTER_SPECIAL_WEAPON_FIRED,
		firing_player->Get_Object()->Get_ID());
	ENFORCED_IF( destroy_objects != NULL )
	{
		for (int i = 0; i < destroy_objects->Get_Count(); ++i)
		{
			GameObjectClass *destroy_me = destroy_objects->Get_At(i);
			FAIL_IF( destroy_me == NULL )		{ continue; }
			destroy_me->Destroy();
		}
	}

	//For consistency with the taskforce versions of this functions we'll return a wrapper around ourselves.
	return Return_Variable(this);
}

/**************************************************************************************************
* GameObjectWrapper::Contains_Hero -- Script function to discover whether or not this object
*	is currently carrying any hero units
*
* In:				
*
* Out:		
*
* History: 2/23/2005 6:52PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Contains_Hero(LuaScriptClass *script, LuaTable *)
{
	if (!Object)
	{
		script->Script_Error("GameObjectWrapper::Contains_Hero -- this object is already dead.");
		return 0;
	}

	return Return_Variable(new LuaBool(Object->Contains_Named_Hero()));
}

/**************************************************************************************************
* GameObjectWrapper::Contains_Hero -- Script function to retrieve the collection of heroes carried by this unit
*
* In:				
*
* Out:		
*
* History: 2/23/2005 6:52PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Contained_Heroes(LuaScriptClass *script, LuaTable *)
{
	if (!Object)
	{
		script->Script_Error("GameObjectWrapper::Get_Contained_Heroes -- this object is already dead.");
		return 0;
	}

	LuaTable *heroes = Alloc_Lua_Table();
	FlagshipDataPackClass *flagship_data = Object->Get_Flagship_Data();

	if (flagship_data)
	{
		for (int i = 0; i < flagship_data->Get_Contained_Units()->Size(); ++i)
		{
			if (flagship_data->Get_Contained_Units()->Get_At(i)->Get_Type()->Is_Named_Hero())
			{
				heroes->Value.push_back(Create(flagship_data->Get_Contained_Units()->Get_At(i), script));
			}
		}
	}

	if (Object->Behaves_Like(BEHAVIOR_TRANSPORT))
	{
		DynamicVectorClass<GameObjectClass*> *units = Object->Get_Transport_Contents();
		FAIL_IF(!units) { return 0; }
		for (int i = 0; i < units->Size(); ++i)
		{
			if (units->Get_At(i)->Get_Type()->Is_Named_Hero())
			{
				heroes->Value.push_back(Create(units->Get_At(i), script));
			}
		}
	}

	return Return_Variable(heroes);
}

/**************************************************************************************************
* GameObjectWrapper::Are_Engines_Online -- Script function to determine whether or not this unit's engines
*	are currently functioning.
*
* In:				
*
* Out:		
*
* History: 2/23/2005 6:52PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Are_Engines_Online(LuaScriptClass *script, LuaTable *)
{
	if (!Object)
	{
		script->Script_Error("GameObjectWrapper::Are_Engines_Online -- this object is already dead.");
		return 0;
	}

	return Return_Variable(new LuaBool(Object->Are_Engines_Enabled()));
}


/**
 * Return the level of the starbase at this planet.
 * 
 * @param script lua script
 * 
 * @return level of the starbase
 * @since 3/21/2005 10:48:13 AM -- BMH
 */
LuaTable *GameObjectWrapper::Get_Starbase_Level(LuaScriptClass *script, LuaTable *)
{
	if (!Object)
	{
		script->Script_Error("GameObjectWrapper::Get_Starbase_Level -- this object is already dead.");
		return 0;
	}

	if (Object->Get_Behavior(BEHAVIOR_PLANET) == NULL || Object->Get_Planetary_Data() == NULL)
	{
		script->Script_Error("GameObjectWrapper::Get_Starbase_Level -- This object is not a planet.");
		return 0;
	}

	FAIL_IF(Object->Get_Planetary_Data()->Get_StarBase() == NULL) return 0;

	return Return_Variable(new LuaNumber((float)Object->Get_Planetary_Data()->Get_StarBase()->Get_Base_Level()));
}

/**************************************************************************************************
* GameObjectWrapper::Get_Build_Pad_Contents -- Script function to get a reference to the object (if any)
*	currently occupying a build pad.
*
* In:				
*
* Out:		
*
* History: 3/2/2005 10:00AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Build_Pad_Contents(LuaScriptClass *script, LuaTable *)
{
	if (!Object)
	{
		script->Script_Error("GameObjectWrapper::Get_Build_Pad_Contents -- this object is already dead.");
		return 0;
	}

	TacticalBuildObjectsBehaviorClass *build_behavior = static_cast<TacticalBuildObjectsBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TACTICAL_BUILD_OBJECTS));
	if (!build_behavior)
	{
		script->Script_Warning("GameObjectWrapper::Get_Build_Pad_Contents -- object %s is not a build pad.", Object->Get_Type()->Get_Name()->c_str());
		return 0;
	}

	GameObjectClass *built_object = build_behavior->Get_Constructed_Object(Object);
	if (!built_object)
	{
		built_object = build_behavior->Get_Object_Under_Construction(Object);
	
		if (!built_object)
		{
			return NULL;
		}
	}

	return Return_Variable(Create(built_object, script));
}

/**************************************************************************************************
* GameObjectWrapper::Get_Distance -- Script function to get the distance from this object to a position
*	(defined by another object, an AITargetLocation or a taskforce).
*
* In:				
*
* Out:		
*
* History: 3/2/2005 10:01AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Distance(LuaScriptClass *script, LuaTable *params)
{
	if (!Object)
	{
		script->Script_Error("GameObjectWrapper::Get_Distance -- this object is already dead.");
		return 0;
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Get_Distance -- invalid number of parameters.  Expceted 1, got %d.", params->Value.size());
		return 0;
	}

	Vector3 target_position;
	if (!Lua_Extract_Position(params->Value[0], target_position))
	{
		script->Script_Error("GameObjectWrapper::Get_Distance -- could not extract a position from parameter 1.");
	}

	float distance = (Object->Get_Position() - target_position).Length();

	return Return_Variable(new LuaNumber(distance));
}

/**************************************************************************************************
* GameObjectWrapper::Sell -- Script function to sell off an object built in tactical mode.
*
* In:				
*
* Out:		
*
* History: 3/2/2005 10:01AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Sell(LuaScriptClass *script, LuaTable *)
{
	if (!Object)
	{
		script->Script_Error("GameObjectWrapper::Sell -- this object is already dead.");
		return 0;
	}

	TacticalSellBehaviorClass *sell_behavior = static_cast<TacticalSellBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TACTICAL_SELL));
	if (!sell_behavior)
	{
		script->Script_Error("GameObjectWrapper::Sell -- object %s cannot be sold.", Object->Get_Type()->Get_Name()->c_str());
		return 0;
	}

	sell_behavior->Execute_Sell_This_Tactical_Object(Object);

	return 0;
}

LuaTable *GameObjectWrapper::Lock_Current_Orders(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	//We'll have the freestore listen on the signals to unlock the movement.  
	PlayerClass *player = Object->Get_Owner_Player();
	FAIL_IF(!player) { return NULL; }
	if (!player->Get_Is_AI_Controlled())
	{
		script->Script_Error("GameObjectWrapper::Lock_Current_Orders - object %s does not belong to an AI player; cannot lock.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return NULL;
	}

	TacticalAIManagerClass *tactical_manager = player->Get_AI_Player()->Get_Tactical_Manager_By_Mode(GameModeManager.Get_Sub_Type());
	AIFreeStoreClass *free_store = tactical_manager->Get_Execution_System()->Get_Free_Store();

	LocomotorInterfaceClass *locomotor = static_cast<LocomotorInterfaceClass*>(Object->Get_Behavior(BEHAVIOR_LOCO));
	if (!locomotor)
	{
		script->Script_Error("GameObjectWrapper::Lock_Current_Orders - %s is immobile; locking not supported.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	if (Object->Behaves_Like(BEHAVIOR_TEAM) && 
			GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_SPACE && 
			!locomotor->NMI_Is_Formation_Team_Locomotor())
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return NULL; }
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }

			if (team_member->Is_Movement_Locked())
			{
				script->Script_Warning("GameObjectWrapper::Lock_Current_Orders - %s is already locked.  Script cannot stack locks.", team_member->Get_Type()->Get_Name()->c_str());
				continue;
			}

			team_member->Set_Movement_Locked(true);

			SignalDispatcherClass::Get().Add_Listener(team_member, free_store, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED);
			SignalDispatcherClass::Get().Add_Listener(team_member, free_store, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED);		
		}
	}
	else if (locomotor->NMI_Use_Team_In_Formation())
	{
		GameObjectClass *parent = Object->Get_Parent_Container_Object();
		ENFORCED_IF(parent)
		{
			if (parent->Is_Movement_Locked())
			{
				script->Script_Warning("GameObjectWrapper::Lock_Current_Orders - %s is already locked.  Script cannot stack locks.", parent->Get_Original_Object_Type()->Get_Name()->c_str());
				return NULL;
			}

			parent->Set_Movement_Locked(true);

			SignalDispatcherClass::Get().Add_Listener(parent, free_store, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED);
			SignalDispatcherClass::Get().Add_Listener(parent, free_store, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED);
		}
	}
	else
	{
		if (Object->Is_Movement_Locked())
		{
			script->Script_Warning("GameObjectWrapper::Lock_Current_Orders - %s is already locked.  Script cannot stack locks.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
			return NULL;
		}

		Object->Set_Movement_Locked(true);

		SignalDispatcherClass::Get().Add_Listener(Object, free_store, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED);
		SignalDispatcherClass::Get().Add_Listener(Object, free_store, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED);
	}

	return NULL;
}


/**
 * Constructor
 * @since 4/26/2005 3:17:42 PM -- BMH
 */
GameObjectWrapper::ObjectInRangeItem::ObjectInRangeItem() : 
	EventSent(false)
{
}

/**
 * Cancel all object in range events for the passed in function.
 * 
 * @param script lua script
 * @param params lua params: event to cancel
 * 
 * @return none
 * @since 4/26/2005 1:15:11 PM -- BMH
 */
LuaTable *GameObjectWrapper::Cancel_Event_Object_In_Range(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() < 1)
	{
		script->Script_Error("GameObjectWrapper::Cancel_Event_Object_In_Range -- Invalid number of params.");
		return NULL;
	}

	LuaFunction::Pointer func = PG_Dynamic_Cast<LuaFunction>(params->Value[0]);

	if (!func)
	{
		script->Script_Error("GameObjectWrapper::Event_Object_In_Range -- Expected function object as parameter 1.");
		return NULL;
	}

	for (int i = 0; i < (int)ObjectInRangeList.size(); i++)
	{
		if (script->Compare_Lua_Functions(ObjectInRangeList[i].Function, func))
		{
			ObjectInRangeList.erase(ObjectInRangeList.begin() + i);
			i--;
			ObjectInRangeListModified = true;
		}
	}

	return NULL;
}

/**
 * Schedule an Object in range event on this object.
 * 
 * @param script lua script
 * @param params lua params: event to signal, distance, object_type, playerobject
 * 
 * @return true if event was scheduled.
 * @since 4/26/2005 10:56:26 AM -- BMH
 */
LuaTable *GameObjectWrapper::Event_Object_In_Range(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() < 2)
	{
		script->Script_Error("GameObjectWrapper::Event_Object_In_Range -- Invalid number of params.");
		return NULL;
	}

	LuaFunction::Pointer func = PG_Dynamic_Cast<LuaFunction>(params->Value[0]);
	LuaNumber::Pointer num = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);

	if (!func)
	{
		script->Script_Error("GameObjectWrapper::Event_Object_In_Range -- Expected function object as parameter 1.");
		return NULL;
	}

	if (!num)
	{
		script->Script_Error("GameObjectWrapper::Event_Object_In_Range -- Expected number as parameter 2.");
		return NULL;
	}

	ObjectInRangeListModified = true;
	ObjectInRangeList.resize(ObjectInRangeList.size()+1);
	ObjectInRangeList.back().Distance = num;
	ObjectInRangeList.back().Function = func;

	int param_idx = 2;
	SmartPtr<GameObjectTypeWrapper> type = NULL;
	SmartPtr<PlayerWrapper> player = NULL;
	if (param_idx < (int)params->Value.size())
	{
		type = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[param_idx]);
		player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[param_idx]);
		param_idx++;
		if (param_idx < (int)params->Value.size())
		{
			if (!type) type = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[param_idx]);
			if (!player) player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[param_idx]);
		}
	}

	ObjectInRangeList.back().Type = type;
	ObjectInRangeList.back().Player = player;

	return Return_Variable(new LuaBool(true));
}


/**
 * Generic wrapper service function.
 * 
 * @param script
 * @param params
 * 
 * @return 
 */
LuaTable *GameObjectWrapper::Service_Wrapper(LuaScriptClass *script, LuaTable *)
{
	if (!Object) { return NULL; }

	LuaTable::Pointer params = NULL;
	Box3Class box;
	box.Center = Object->Get_Position();
	for (int i = 0; i < (int)ObjectInRangeList.size() && Object; i++)
	{
		if (ObjectInRangeList[i].EventSent) continue;

		TreeCullClass *grid = Object->Get_Manager()->Get_All_Collidable_Objects_List();
		float dval = ObjectInRangeList[i].Distance->Value;
		box.Extent = Vector3(dval, dval, dval);
		grid->Box_Collect(box);

		ObjectInRangeListModified = false;

		CullLinkClass *link = grid->Get_First_Collected_Object();
		for ( ; link; link = link->Get_Next_Collected())
		{
			GameObjectClass *cull_object = static_cast<GameObjectClass*>(link->Get_Cull_Object());
			assert(cull_object != NULL);

			if (cull_object->Is_Delete_Pending())
				continue;

			if (cull_object->Get_Type()->Is_Decoration() || cull_object->Get_Behavior(BEHAVIOR_PROJECTILE) ||
				 cull_object->Get_Behavior(BEHAVIOR_PARTICLE))
				continue;

			if (cull_object == Object) continue;

			if (ObjectInRangeList[i].Player && cull_object->Get_Owner_Player()->Is_Ally(ObjectInRangeList[i].Player->Get_Object()) == false)
            continue;

			LuaFunction *func = ObjectInRangeList[i].Function;

			//Might need to trigger the script for objects that are fictionally contained within this unit
			if (cull_object->Get_Flagship_Data())
			{
				//Iterate backwards just in case this modifies the contained units
				const DynamicVectorClass<GameObjectClass*> *contained_units = cull_object->Get_Flagship_Data()->Get_Contained_Units();
				for (int j = contained_units->Size() - 1; j >= 0; --j)
				{
					GameObjectClass *contained_object = contained_units->Get_At(j);
					FAIL_IF(!contained_object) { continue; }
					if (ObjectInRangeList[i].Type && contained_object->Get_Type() != ObjectInRangeList[i].Type->Get_Object())
						continue;

					if (!params) params = Alloc_Lua_Table();
					params->Value.resize(0);
					params->Value.push_back(this);
					params->Value.push_back(GameObjectWrapper::Create(contained_object, script));

					script->Call_Function(func, params);

					//Need to handle the possibility that invoking script has scheduled this object for destruction (thus invalidating the wrapper)
					if (!Object)
					{
						break;
					}

					if (i >= (int)ObjectInRangeList.size() || func != ObjectInRangeList[i].Function)
					{
						assert(ObjectInRangeListModified);
						break;
					}
				}
			}

			//Need to handle the possibility that invoking script has scheduled this object for destruction (thus invalidating the wrapper)
			if (!Object)
			{
				break;
			}

			if (i >= (int)ObjectInRangeList.size() || func != ObjectInRangeList[i].Function)
			{
				assert(ObjectInRangeListModified);
				break;
			}

			if (ObjectInRangeList[i].Type && cull_object->Get_Type() != ObjectInRangeList[i].Type->Get_Object())
				continue;

			// Call the event;
			if (!params) params = Alloc_Lua_Table();
			params->Value.resize(0);
			params->Value.push_back(this);
			params->Value.push_back(GameObjectWrapper::Create(cull_object, script));

			script->Call_Function(func, params);

			//Need to handle the possibility that invoking script has scheduled this object for destruction (thus invalidating the wrapper)
			if (!Object)
			{
				break;
			}

			if (i >= (int)ObjectInRangeList.size() || func != ObjectInRangeList[i].Function)
			{
				assert(ObjectInRangeListModified);
				break;
			}
		}
		if (ObjectInRangeListModified) i = -1;
		else ObjectInRangeList[i].EventSent = true;
	}

	for (int i = 0; i < (int)ObjectInRangeList.size(); i++)
	{
		ObjectInRangeList[i].EventSent = false;
	}

	Free_Lua_Table(params);

	return NULL;
}

/**
 * Return the objects position to Lua
 * 
 * @param script lua script
 * 
 * @return objects position
 * @since 5/2/2005 2:14:30 PM -- BMH
 */
LuaTable *GameObjectWrapper::Lua_Get_Position(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }
	FAIL_IF(!Position) { return NULL; }

   Position->Set_Position(Object->Get_Position());
	return Return_Variable(Position);
}


/**
 * Add / remove this object from the freestore so the AI won't use it
 * as specified from the command parameters.
 * 
 * param bool -- true, remove the object from freestore,
 *               false, add it back to the freestore.
 * 
 * @param script lua script.
 * @param params lua params
 * 
 * @return none
 * @since 5/2/2005 8:02:30 PM -- BMH
 */
LuaTable *GameObjectWrapper::Prevent_AI_Usage(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() < 1)
	{
		script->Script_Error("GameObjectWrapper::Prevent_AI_Usage -- Expected bool parameter.");
		return NULL;
	}

	LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!bval)
	{
		script->Script_Error("GameObjectWrapper::Prevent_AI_Usage -- Expected bool parameter.");
		return NULL;
	}

	AIPlayerClass *ai_player = Object->Get_Owner_Player()->Get_AI_Player();

	if (!ai_player)
	{
		script->Script_Warning("GameObjectWrapper::Prevent_AI_Usage -- No AI Player active on the owner of this object.");
		return NULL;
	}

	TacticalAIManagerClass *tactical_manager = (*ai_player->Find_Tactical_Manager_By_Mode(GameModeManager.Get_Active_Mode()));
	if (!tactical_manager)
	{
		script->Script_Error("GameObjectWrapper::Prevent_AI_Usage -- Owner has no AI presence in this mode.  Ensure the player is one of the standard particpants in the battle or call Enable_As_Actor before using Prevent_AI_Usage.");
		return NULL;
	}

	if (!tactical_manager->Get_Execution_System())
	{
		script->Script_Error("GameObjectWrapper::Prevent_AI_Usage -- Owner AI is not yet initialized.  Wait a frame before calling this function.");
		return NULL;
	}

	AIFreeStoreClass *freestore = tactical_manager->Get_Execution_System()->Get_Free_Store();

	if (!freestore)
	{
		script->Script_Warning("GameObjectWrapper::Prevent_AI_Usage -- No freestore exists for this AI Player.");
		return NULL;
	}

	if (bval->Value)
	{
		freestore->Remove_Free_Store_Object(Object);
	}
	else
	{
		//Make sure to set the AI usable flag or the Add_Free_Store_Object call may do nothing at all.
		Object->Set_Is_AI_Usable(true);
		freestore->Add_Free_Store_Object(Object);
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Set_Importance -- Make an object important (or unimportant) to affect the likelihood
*	that the cinematic camera will focus on it
*
* In:				
*
* Out:		
*
* History: 5/3/2005 11:19AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Set_Importance(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Set_Importance -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	if (!Object)
	{
		script->Script_Warning("GameObjectWrapper::Set_Importance -- this object is dead!");
		return NULL;
	}

	SmartPtr<LuaNumber> importance = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!importance)
	{
		script->Script_Error("GameObjectWrapper::Set_Importance -- invalid type for parameter 1.  Expected number.");
		return NULL;
	}

	Object->Set_Importance(importance->Value);

	return NULL;
}


/**
 * Apply damage to the game object.
 * 
 * @param script lua script
 * @param params params -- 1) number, the amout of damage to apply.
 * 
 * @return none
 * @since 6/2/2005 11:10:50 AM -- BMH
 */
LuaTable *GameObjectWrapper::Take_Damage(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1 && params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Take_Damage -- invalid number of parameters.  Expected 1 or 2, got %d.", params->Value.size());
		return NULL;
	}

	if (!Object)
	{
		script->Script_Warning("GameObjectWrapper::Take_Damage -- this object is dead!");
		return NULL;
	}

	SmartPtr<LuaNumber> damage = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!damage)
	{
		script->Script_Error("GameObjectWrapper::Take_Damage -- invalid type for parameter 1.  Expected number.");
		return NULL;
	}

	const char *renderable_name = NULL;
	if (params->Value.size() == 2)
	{
		SmartPtr<LuaString> hard_point_name = PG_Dynamic_Cast<LuaString>(params->Value[1]);
		if (!hard_point_name)
		{
			script->Script_Error("GameObjectWrapper::Take_Damage -- invalid type for parameter 2.  Expected string.");
			return NULL;
		}

		const HardPointDataClass *hard_point_data = HardPointDataManager.Find_Hard_Point_Data(hard_point_name->Value);
		if (!hard_point_data)
		{
			script->Script_Error("GameObjectWrapper::Take_Damage -- unknown hard point %s.", hard_point_name->Value.c_str());
			return NULL;
		}
		renderable_name = hard_point_data->Get_Collision_Mesh_Name()->c_str();
	}

	if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		//Damaging the team itself has no effect, so pass the damage on to the individuals
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		ENFORCED_IF(team)
		{
			for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
			{
				GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
				FAIL_IF(!team_member) { continue; }
				team_member->Take_Damage(DAMAGE_DEBUG_CHEAT, damage->Value, NULL, renderable_name, NULL);
			}
		}
	}
	else
	{
		Object->Take_Damage(DAMAGE_DEBUG_CHEAT, damage->Value, NULL, renderable_name, NULL);
	}

	return NULL;
}


/**
 * Despawn the object
 * 
 * @param script lua script
 * 
 * @return none
 * @since 6/2/2005 11:11:09 AM -- BMH
 */
LuaTable *GameObjectWrapper::Despawn(LuaScriptClass *script, LuaTable *)
{
	if (!Object)
	{
		script->Script_Warning("GameObjectWrapper::Despawn -- this object is dead!");
		return NULL;
	}

	if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		//Just destroying the team does no good (and could lead to all sorts of unexpected things happening).
		//We need to destroy the team members, then the actual team will clean itself up.
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return NULL; }

		for (int i = team->Get_Team_Member_Count() - 1; i >= 0; --i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }

			//Don't count objects despawned through script as killed
			TheGameScoringManagerClass::Get().Stop_Listening_To_Unit(team_member);
			int parent_mode_id = team_member->Get_Parent_Mode_ID();
			if (parent_mode_id != INVALID_OBJECT_ID)
			{
				const GameObjectClass *parent_game_object = GameModeManager.Get_Parent_Mode_Object_By_ID(GameModeManager.Get_Active_Mode(), parent_mode_id);
				if (parent_game_object)
				{
					TheGameScoringManagerClass::Get().Stop_Listening_To_Unit(const_cast<GameObjectClass*>(parent_game_object));
				}
			}

			team_member->Destroy();
		}
	}
	else
	{
		//Don't count objects despawned through script as killed
		TheGameScoringManagerClass::Get().Stop_Listening_To_Unit(Object);
		int parent_mode_id = Object->Get_Parent_Mode_ID();
		if (parent_mode_id != INVALID_OBJECT_ID)
		{
			const GameObjectClass *parent_game_object = GameModeManager.Get_Parent_Mode_Object_By_ID(GameModeManager.Get_Active_Mode(), parent_mode_id);
			if (parent_game_object)
			{
				TheGameScoringManagerClass::Get().Stop_Listening_To_Unit(const_cast<GameObjectClass*>(parent_game_object));
			}
		}

		Object->Destroy();
	}

	return NULL;
}


/**
 * Mark the parent mode object as dead.
 * 
 * @param script lua script
 * 
 * @return none
 * @since 6/2/2005 7:30:26 PM -- BMH
 */
LuaTable *GameObjectWrapper::Mark_Parent_Mode_Object_For_Death(LuaScriptClass *script, LuaTable *)
{
	if (!Object)
	{
		script->Script_Warning("GameObjectWrapper::Mark_Parent_Mode_Object_For_Death -- this object is dead!");
		return NULL;
	}

	if (Object->Get_Parent_Mode_ID() != INVALID_OBJECT_ID)
	{
		Object->Get_Manager()->Mark_Parent_Mode_Object_For_Death(Object->Get_Parent_Mode_ID(), Object->Get_Type(), Object);
	}

	return NULL;
}



/**
 * Set this object as unselectable.
 * 
 * @param script lua script
 * @param params params -- 1) boolean true to enable selection, false to disable.
 * 
 * @return none
 * @since 6/4/2005 10:36:34 AM -- BMH
 */
LuaTable *GameObjectWrapper::Set_Selectable(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Set_Selectable -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	if (!Object)
	{
		script->Script_Warning("GameObjectWrapper::Set_Selectable -- this object is dead!");
		return NULL;
	}

	SmartPtr<LuaBool> selectable = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!selectable)
	{
		script->Script_Error("GameObjectWrapper::Set_Selectable -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	if (Object->Get_Behavior(BEHAVIOR_SELECTABLE) == NULL)
	{
		script->Script_Warning("GameObjectWrapper::Set_Selectable -- this object is is not selectable!");
		return NULL;
	}

	GAME_OBJECT_MANAGER.Deselect_Object(Object, Object->Get_Owner(), true);

	//Edit the data for all the team members too
	if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return NULL; }
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }
			if (team_member->Get_Selection_Data())
			{
				GAME_OBJECT_MANAGER.Deselect_Object(team_member, team_member->Get_Owner(), true);
				team_member->Get_Selection_Data()->Set_Selectable(selectable->Value);
			}
		}
	}

	Object->Get_Selection_Data()->Set_Selectable(selectable->Value);
	return NULL;
}

LuaTable *GameObjectWrapper::Get_Next_Starbase_Type(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (!Object->Behaves_Like(BEHAVIOR_PLANET))
	{
		script->Script_Error("GameObjectWrapper::Get_Next_Starbase_Type -- only valid on planets.");
		return NULL;
	}

	PlanetaryBehaviorClass *planet = static_cast<PlanetaryBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_PLANET));
	GameObjectTypeClass *base_type = const_cast<GameObjectTypeClass*>(planet->Get_Next_Buildable_Starbase_Type(Object));

	if (!base_type)
	{
		return NULL;
	}

	return Return_Variable(GameObjectTypeWrapper::Create(base_type, script));
}

/**
 * Change the owner of this object to the passed in player.
 * 
 * @param script lua script
 * @param params params -- 1) player object
 * 
 * @return none
 * @since 6/9/2005 2:08:12 PM -- BMH
 */
LuaTable *GameObjectWrapper::Change_Owner(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Change_Owner -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	if (!Object)
	{
		script->Script_Warning("GameObjectWrapper::Change_Owner -- this object is dead!");
		return NULL;
	}

	SmartPtr<PlayerWrapper> player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!player)
	{
		script->Script_Error("GameObjectWrapper::Change_Owner -- invalid type for parameter 1.  Expected PlayerWrapper.");
		return NULL;
	}

	if (Object->Behaves_Like(BEHAVIOR_PLANET))
	{
		PlanetaryBehaviorClass *planet_behave = static_cast<PlanetaryBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_PLANET));
		ENFORCED_IF(planet_behave)
		{
			planet_behave->Change_Faction(Object, player->Get_Object()->Get_Faction());
		}
	}
	else
	{
		Object->Change_Owner(player->Get_Object()->Get_ID());

		//Make sure the visual appearance of this object matches its new owner.
		if (Object->Behaves_Like(BEHAVIOR_CAPTURE_POINT))
		{
			//Owner-specific animation
			int faction_anim_subindex = Object->Get_Type()->Get_Anim_Subindex_For_Faction(Object->Get_Owner_Faction());
			if (faction_anim_subindex != -1)
			{
				Object->Set_Target_Animation_Type(ANIM_IDLE, static_cast<unsigned int>(faction_anim_subindex), true, 0.0f);
			}

			//Colorization
			CapturePointBehaviorClass *capture_point = static_cast<CapturePointBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_CAPTURE_POINT));
			ENFORCED_IF(capture_point)
			{
				RGBAClass color = capture_point->Get_Radar_Icon_Color();
				Vector3 vcolor(color.Red, color.Green, color.Blue);

				ModelClass *model = Object->Get_Model();
				if (model) 
				{
					model->Set_Unit_Colorization(vcolor);
				}

				capture_point->Set_Hologram_Color(Vector4(vcolor.X, vcolor.Y, vcolor.Z, 1.0f));

				//The object is fully owned by the new player, so fill it 100%
				capture_point->Set_Hologram_Fill(1.0f);
			}
		}
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Divert -- Send an object somewhere and then have it resume its previous orders
*
* In:				
*
* Out:		
*
* History: 6/9/2005 6:29PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Divert(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (!Object->Behaves_Like(BEHAVIOR_LOCO))
	{
		script->Script_Error("GameObjectWrapper::Divert -- this object cannot move");
		return NULL;
	}

	FormationClass *formation = Object->Get_Locomotor_Data()->Get_Formation();
	if (!formation)
	{
		script->Script_Warning("GameObjectWrapper::Divert -- unable to divert - object %s is between moves.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return Return_Variable(new LuaBool(false));
	}

	if (params->Value.size() != 1 && params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Divert -- invalid number of parameters.  Expected 1 or 2, got %d.");
		return NULL;
	}

	if (Object->Is_Movement_Locked())
	{
		script->Script_Warning("GameObjectWrapper::Divert -- unable to divert - object %s has had its movement locked.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return Return_Variable(new LuaBool(false));
	}

	FormationDestinationStruct destination;
	destination.Type = FDT_POSITION;
	//Tag this as a script issued diversion
	destination.MovementType.Set_Type(UNIT_AI_MOVE_TYPE_SCRIPT_DIVERSION);
	if (!Lua_Extract_Position(params->Value[0], destination.Position))
	{
		script->Script_Error("GameObjectWrapper::Divert -- invalid type for parameter 1; expected something which defines a position");
		return NULL;
	}

	if (params->Value.size() == 2)
	{
		SmartPtr<LuaNumber> lua_threat_tolerance = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
		if (!lua_threat_tolerance)
		{
			script->Script_Error("GameObjectWrapper::Divert -- invalid type for parameter 2; expected number.");
			return NULL;
		}

		destination.ThreatTolerance = lua_threat_tolerance->Value;
	}

	formation->Split_From_Formation(Object);
	formation = Object->Get_Locomotor_Data()->Get_Formation();

	formation->Override_Destination(destination);

	return Return_Variable(new LuaBool(true));
}

/**************************************************************************************************
* GameObjectWrapper::Get_AI_Power_Vs_Unit -- Assess my strength relative to another unit
*
* In:				
*
* Out:		
*
* History: 6/9/2005 6:30PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_AI_Power_Vs_Unit(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("Get_AI_Power_Vs_Unit -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	//Work back to a target game object
	SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	SmartPtr<AITargetLocationWrapper> target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);

	if (!object_wrapper && !target_wrapper)
	{
		script->Script_Error("Get_AI_Power_Vs_Unit -- invalid type for parameter 1.  Expected object or ai target.");
		return 0;
	}

	if ((object_wrapper && !object_wrapper->Get_Object()) || (target_wrapper && !target_wrapper->Get_Object()))
	{
		script->Script_Error("Get_AI_Power_Vs_Unit -- target is already dead.");
		return 0;
	}

	GameObjectClass *object = 0;
	if (object_wrapper)
	{
		object = object_wrapper->Get_Object();
	}
	else
	{
		object = target_wrapper->Get_Object()->Get_Target_Game_Object();
		if (!object)
		{
			script->Script_Error("Get_AI_Power_Vs_Unit -- ai target %s does not represent a game object (requried for this function).", target_wrapper->Get_Object()->Get_Name().c_str());
			return 0;
		}
	}

	const GameObjectTypeClass *enemy_type = 0;
	if (object->Behaves_Like(BEHAVIOR_TRANSPORT))
	{
		enemy_type = object->Get_Type();
	}
	else
	{
		enemy_type = object->Get_Company_Type();
	}

	const GameObjectTypeClass *my_type = 0;
	if (Object->Behaves_Like(BEHAVIOR_TRANSPORT))
	{
		my_type = Object->Get_Type();
	}
	else
	{
		my_type = Object->Get_Company_Type();
	}

	//Add up my force with appropriate RPS adjustment vs this enemy type.
	float my_contrast = my_type->Get_AI_Combat_Power_Metric() * Object->Get_Health_Percent();
	float enemy_contrast = enemy_type->Get_AI_Combat_Power_Metric() * object->Get_Health_Percent();

	int complete_category = enemy_type->Get_Category_Mask();
	int category = GET_FIRST_BIT_SET(complete_category);

	float best_factor = 0.0;
	while (category > -1) 
	{
		best_factor = Max(best_factor, TargetContrastClass::Get_Average_Contrast_Factor(my_type, NULL, 1 << category));

		// turn off that bit
		complete_category &= (~(1 << category));
		category = GET_FIRST_BIT_SET(complete_category);
	}

	my_contrast *= best_factor; 
	
	complete_category = my_type->Get_Category_Mask();
	category = GET_FIRST_BIT_SET(complete_category);

	best_factor = 0.0;
	while (category > -1) 
	{
		best_factor = Max(best_factor, TargetContrastClass::Get_Average_Contrast_Factor(enemy_type, NULL, 1 << category));

		// turn off that bit
		complete_category &= (~(1 << category));
		category = GET_FIRST_BIT_SET(complete_category);
	}

	enemy_contrast *= best_factor;

	return Return_Variable(new LuaNumber(my_contrast - enemy_contrast));
}

/**************************************************************************************************
* GameObjectWrapper::Has_Active_Orders -- Determine whether an object is doing something useful
*
* In:				
*
* Out:		
*
* History: 6/9/2005 6:30PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Has_Active_Orders(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (Object->Get_Target_Attack_Object(0))
	{
		return Return_Variable(new LuaBool(true));
	}

	if (Object->Behaves_Like(BEHAVIOR_LOCO))
	{
		LocomotorInterfaceClass *locomotor = static_cast<LocomotorInterfaceClass*>(Object->Get_Behavior(BEHAVIOR_LOCO));
		FAIL_IF(!locomotor) { return NULL; }

		MovementCoordinatorSystemClass *movement_system = GameModeManager.Get_Active_Mode()->Get_Movement_Coordinator_System();
		FAIL_IF(!movement_system) { return NULL; }
		FormationClass *formation = movement_system->Get_Object_Formation(Object);

		//No formation indicates a move order has been issued but not yet processed.
		//A formation order that's not finished could nevertheless have us paused in a state where we report we have no more movement.  Treat this
		//as a situation where we DO have active orders, since we're presumably going to resume whatever the move was once it becomes possible.
		if (!formation || !formation->Get_Has_Reached_Done())
		{
			return Return_Variable(new LuaBool(true));
		}

		const FormationDestinationStruct &destination = formation->Get_Destination();
		if (destination.MovementType.Has_Type(UNIT_AI_MOVE_TYPE_IDLE))
		{
			return Return_Variable(new LuaBool(false));
		}

		return Return_Variable(new LuaBool(!locomotor->NMI_Can_Start() || locomotor->NMI_Has_Movement_Left()));
	}

	return Return_Variable(new LuaBool(false));
}

/**************************************************************************************************
* GameObjectWrapper::Get_Contained_Object_Count -- Get the number of objects contained within this fleet, transport or flagship
*
* In:				
*
* Out:		
*
* History: 6/20/2005 1:42PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Contained_Object_Count(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	float object_count = 0.0f;

	if (Object->Get_Flagship_Data())
	{
		object_count = static_cast<float>(Object->Get_Flagship_Data()->Get_Contained_Units()->Size());
	}
	else if (Object->Behaves_Like(BEHAVIOR_TRANSPORT))
	{
		TransportBehaviorClass *transport = static_cast<TransportBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TRANSPORT));
		FAIL_IF(!transport) { return NULL; }
		object_count = static_cast<float>(transport->Get_Contained_Objects_Count(Object));
	}
	else if (Object->Behaves_Like(BEHAVIOR_FLEET))
	{
		FleetBehaviorClass *fleet = static_cast<FleetBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_FLEET));
		FAIL_IF(!fleet) { return NULL; }
		object_count = static_cast<float>(fleet->Get_Contained_Objects_Count(Object));
	}
	else if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return NULL; }
		object_count = static_cast<float>(team->Get_Team_Member_Count());
	}

	return Return_Variable(new LuaNumber(object_count));
}

/**************************************************************************************************
* GameObjectWrapper::Contains_Object_Type -- Determine whether this fleet, transport or flagship contains
*	an object of the given type.
*
* In:				
*
* Out:		
*
* History: 6/20/2005 1:42PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Contains_Object_Type(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Contains_Object_Type -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectTypeWrapper> type = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
	if (!type)
	{
		script->Script_Error("GameObjectWrapper::Contains_Object_Type -- invalid type for parameter 1.  Expected game object type.");
		return NULL;
	}

	return Return_Variable(new LuaBool(Object->Contains_Object_Type(type->Get_Object())));
}

/**************************************************************************************************
* GameObjectWrapper::Destroy_Contained_Objects -- Destroy some fraction of the contents of a fleet
*
* In:				
*
* Out:		
*
* History: 6/20/2005 1:42PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Destroy_Contained_Objects(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Destroy_Contained_Objects -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaNumber> destroy_fraction = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!destroy_fraction)
	{
		script->Script_Error("GameObjectWrapper::Destroy_Contained_Objects -- invalid type for parameter 1.  Expected number.");
		return NULL;
	}

	if (!Object->Behaves_Like(BEHAVIOR_FLEET))
	{
		script->Script_Error("GameObjectWrapper::Destroy_Contained_Objects -- this function may only be used on fleets.");
		return NULL;
	}

	FleetBehaviorClass *fleet = static_cast<FleetBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_FLEET));
	FAIL_IF(!fleet) { return NULL; }

	int original_count = static_cast<int>(fleet->Get_Contained_Objects_Count(Object));
	int survivor_count = static_cast<int>((1.0f - destroy_fraction->Value) * original_count + 0.5f);
	survivor_count = PGMath::Clamp(survivor_count, 0, original_count);

	while (original_count > survivor_count)
	{
		int destroy_index = SyncRandom.Get(0, original_count - 1);
		GameObjectClass *fleet_member = fleet->Get_Contained_Object(Object, destroy_index);
		FAIL_IF(!fleet_member) { return NULL; }
		fleet->Remove_Object_From_Fleet(fleet_member);
		fleet_member->Destroy();
		--original_count;
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Is_Ability_Ready -- Find out whether the named unit ability is recharged
*
* In:				
*
* Out:		
*
* History: 6/24/2005 6:13PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_Ability_Ready(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Is_Ability_Ready -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	FAIL_IF(!Object) { return NULL; }

	SmartPtr<LuaString> ability_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!ability_name)
	{
		script->Script_Error("GameObjectWrapper::Is_Ability_Ready -- invalid type for ability 1.  Expected string.");
		return NULL;
	}

	UnitAbilityType ability_type = ABILITY_NONE;
	if (!TheUnitAbilityTypeConverterPtr->String_To_Enum(ability_name->Value, ability_type))
	{
		script->Script_Error("GameObjectWrapper::Is_Ability_Ready -- unknown ability %s.", ability_name->Value.c_str());
		return NULL;
	}

	if (!Object->Has_Unit_Special_Ability(ability_type))
	{
		script->Script_Warning("GameObjectWrapper::Is_Ability_Ready -- Object %s does not have ability %s.", Object->Get_Type()->Get_Name()->c_str(), ability_name->Value.c_str());
		return Return_Variable(new LuaBool(false));
	}

	//JSY: Black market restricted abilities are not ready until they have been purchased.
	if (!Object->Is_Unit_Special_Ability_Available(ability_type))
	{
		return Return_Variable(new LuaBool(false));
	}

	if (Object->Is_Unit_Ability_Enabled(ability_type))
	{
		return Return_Variable(new LuaBool(true));
	}

	//I think it helps the AI if we report abilities as active while they're queued
	if (Object->Get_Tactical_Combatant_Data() && Object->Get_Tactical_Combatant_Data()->Get_Special_Ability_Unit_Ability_Type() == ability_type)
	{
		return Return_Variable(new LuaBool(true));
	}

	return Return_Variable(new LuaBool(false));
}

/**************************************************************************************************
* GameObjectWrapper::Has_Ability -- Find out whether this object has the named ability
*
* In:				
*
* Out:		
*
* History: 6/24/2005 6:13PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Has_Ability(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Has_Ability -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	FAIL_IF(!Object) { return NULL; }

	SmartPtr<LuaString> ability_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!ability_name)
	{
		script->Script_Error("GameObjectWrapper::Has_Ability -- invalid type for ability 1.  Expected string.");
		return NULL;
	}

	UnitAbilityType ability_type = ABILITY_NONE;
	if (!TheUnitAbilityTypeConverterPtr->String_To_Enum(ability_name->Value, ability_type))
	{
		script->Script_Error("GameObjectWrapper::Has_Ability -- unknown ability %s.", ability_name->Value.c_str());
		return NULL;
	}

	return Return_Variable(new LuaBool(Object->Has_Unit_Special_Ability(ability_type)));
}

/**************************************************************************************************
* GameObjectWrapper::Activate_Ability -- Execute the named ability.  Returns a blocking object that
*	is finished once the ability has completed.
*
* In:				
*
* Out:		
*
* History: 6/24/2005 6:13PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Activate_Ability(LuaScriptClass *script, LuaTable *params)
{
	if (GameModeManager.Get_Sub_Type() == SUB_GAME_MODE_GALACTIC)
	{
		return Activate_Ability_Galactic(script, params);
	}
	else if (Activate_Ability_Internal(script, params))
	{
		AbilityBlockStatus *blocking_object = static_cast<AbilityBlockStatus*>(AbilityBlockStatus::FactoryCreate());
		FAIL_IF(!blocking_object) { return NULL; }
		blocking_object->Init(this);
		blocking_object->Add_Object(Object);
		return Return_Variable(blocking_object);
	}
	else
	{
		return NULL;
	}
}

/**************************************************************************************************
* GameObjectWrapper::Set_Single_Ability_Autofire -- Control autofire mode for the named ability. Autofire defaults
*	to on for AI units
*
* In:				
*
* Out:		
*
* History: 6/24/2005 6:14PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Set_Single_Ability_Autofire(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Set_Single_Ability_Autofire -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> ability_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!ability_name)
	{
		script->Script_Error("GameObjectWrapper::Set_Single_Ability_Autofire -- invalid type for ability 1.  Expected string.");
		return NULL;
	}

	UnitAbilityType ability_type = ABILITY_NONE;
	if (!TheUnitAbilityTypeConverterPtr->String_To_Enum(ability_name->Value, ability_type))
	{
		script->Script_Error("GameObjectWrapper::Set_Single_Ability_Autofire -- unknown ability %s.", ability_name->Value.c_str());
		return NULL;
	}

	SmartPtr<LuaBool> enable = PG_Dynamic_Cast<LuaBool>(params->Value[1]);
	if (!enable)
	{
		script->Script_Error("GameObjectWrapper::Set_Single_Ability_Autofire -- invalid type for parameter 2.  Expected bool.");
		return NULL;
	}

	Object->Set_Ability_Autofire(ability_type, enable->Value, reinterpret_cast<int>(script));

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Set_All_Abilities_Autofire -- Control autofire mode for all abilities of the unit. Autofire defaults
*	to on for AI units
*
* In:				
*
* Out:		
*
* History: 6/24/2005 6:14PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Set_All_Abilities_Autofire(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Set_All_Abilities_Autofire -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaBool> enable = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!enable)
	{
		script->Script_Error("GameObjectWrapper::Set_All_Abilities_Autofire -- invalid type for parameter 1.  Expected bool.");
		return NULL;
	}

	for (int i = 0; i < ABILITY_TOTAL; ++i)
	{
		UnitAbilityType ability_type = static_cast<UnitAbilityType>(i);
		if (Object->Has_Unit_Special_Ability(ability_type))
		{
			Object->Set_Ability_Autofire(ability_type, enable->Value, reinterpret_cast<int>(script));
		}
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Activate_Ability_Internal -- Do the real work of activating a unit ability.  Public so that
*	TaskForceClass::Activate_Ability can make use of it.
*
* In:				
*
* Out:	Whether or not the execution of the ability is ongoing.  Some abilities are instantaneous effects or
*			mode switches and for these the return value will be false.
*
* History: 6/24/2005 6:15PM JSY
**************************************************************************************************/
bool GameObjectWrapper::Activate_Ability_Internal(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }
	if (params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Activate_Ability -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return false;
	}

	SmartPtr<LuaString> ability_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!ability_name)
	{
		script->Script_Error("GameObjectWrapper::Activate_Ability -- invalid type for ability 1.  Expected string.");
		return false;
	}

	UnitAbilityType ability_type = ABILITY_NONE;
	if (!TheUnitAbilityTypeConverterPtr->String_To_Enum(ability_name->Value, ability_type))
	{
		script->Script_Error("GameObjectWrapper::Activate_Ability -- unknown ability %s.", ability_name->Value.c_str());
		return false;
	}

	//Make sure we have the ability in question
	if(!Object->Has_Unit_Special_Ability(ability_type))
	{
		script->Script_Warning("GameObjectWrapper::Activate_Ability -- Object %s does not have ability %s.", Object->Get_Type()->Get_Name()->c_str(), ability_name->Value.c_str());
		return false;
	}

	//JSY: Black market restricted abilities are not ready until they have been purchased.
	if (!Object->Is_Unit_Special_Ability_Available(ability_type))
	{
		return false;
	}

	//Don't even try to activate an ability that's not ready.
	if (!Object->Is_Unit_Ability_Enabled(ability_type, true, true))
	{
		return false;
	}

	static DynamicVectorClass<GameObjectClass*> single_object_list;
	single_object_list.Truncate();

	switch (ability_type)
	{
	case ABILITY_INTERDICT:
	case ABILITY_DEPLOY:
	case ABILITY_BARRAGE:
	case ABILITY_HUNT:
	case ABILITY_CABLE_ATTACK:
	case ABILITY_FORCE_LIGHTNING:
	case ABILITY_FLAME_THROWER:
	case ABILITY_FORCE_TELEKINESIS:
	case ABILITY_STICKY_BOMB:
	case ABILITY_CAPTURE_VEHICLE:
	case ABILITY_JET_PACK:
	case ABILITY_TARGETED_HACK:
	case ABILITY_TARGETED_REPAIR:
	case ABILITY_MISSILE_SHIELD:
	case ABILITY_ENERGY_WEAPON:
	case ABILITY_LUCKY_SHOT:
	case ABILITY_CONCENTRATE_FIRE:
	case ABILITY_ION_CANNON_SHOT:
	case ABILITY_MAXIMUM_FIREPOWER:
	case ABILITY_SELF_DESTRUCT:
	case ABILITY_DEPLOY_TROOPERS:
	case ABILITY_EJECT_VEHICLE_THIEF:
	case ABILITY_SUPER_LASER:
	case ABILITY_BERSERKER:
	case ABILITY_FORCE_SIGHT:
	case ABILITY_PLACE_REMOTE_BOMB:
	case ABILITY_RADIOACTIVE_CONTAMINATE:
	case ABILITY_SABER_THROW:
	case ABILITY_LEECH_SHIELDS:
	case ABILITY_TACTICAL_BRIBE:
	case ABILITY_INFECTION:
	case ABILITY_PROXIMITY_MINES:
	case ABILITY_DRAIN_LIFE:
	case ABILITY_BUZZ_DROIDS:
	case ABILITY_SUMMON:
	case ABILITY_CORRUPT_SYSTEMS:
		//These abilities may cause us to do all sorts of horrid things to the objects movement.  We'll send a movement cancelled signal
		//so that any blocking objects that are waiting on this object's movement can just forget about it, rather than possibly becoming
		//stalled.
		SignalDispatcherClass::Get().Send_Signal(Object, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL);
		if (Object->Behaves_Like(BEHAVIOR_TEAM))
		{
			TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
			for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
			{
				SignalDispatcherClass::Get().Send_Signal(team->Get_Team_Member_By_Index(i), PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL);
			}
		}
		break;

	default:
		//These abilities may still have an effect on movement, but not to the extent that they freeze us or send us somewhere different
		//entirely.  Allow the active move (if any) to continue
		break;
	}

	//To avoid code duplication we'll make use of the events that already deal with these unit abilities.  We don't, however,
	//need to schedule the events
	switch (ability_type)
	{
	case ABILITY_ROCKET_ATTACK:
	case ABILITY_TURBO:
	case ABILITY_SPREAD_OUT:
	case ABILITY_INTERDICT:
	case ABILITY_DEFEND:
	case ABILITY_SPRINT:
	case ABILITY_SPOILER_LOCK:
	case ABILITY_POWER_TO_WEAPONS:
	case ABILITY_MISSILE_SHIELD:
	case ABILITY_INVULNERABILITY:
	case ABILITY_SELF_DESTRUCT:
	case ABILITY_SWAP_WEAPONS:
	case ABILITY_STEALTH:
	case ABILITY_FORCE_SIGHT:
	case ABILITY_FORCE_CLOAK:
	case ABILITY_LASER_DEFENSE:
	case ABILITY_FULL_SALVO:
	case ABILITY_SENSOR_JAMMING:
	case ABILITY_DETONATE_REMOTE_BOMB:
	case ABILITY_STIM_PACK:
	case ABILITY_DRAIN_LIFE:
	case ABILITY_BLAST:
		{
			//Second parameter is on/off
			SmartPtr<LuaBool> enable = PG_Dynamic_Cast<LuaBool>(params->Value[1]);
			if (!enable)
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability -- invalid type for parameter 2.  Ability %s expects a bool.", ability_name->Value.c_str());
				return false;
			}
			single_object_list.Add(Object);
			SetUnitAbilityModeEventClass event(enable->Value);
			//Set the player so that we don't get the 'invalid action' buzzer if it fails.
			event.Init(&single_object_list, ability_type, Object->Get_Owner());
			event.Set_UI_Request(false);
			event.Execute();
			return false;
		}

	case ABILITY_DEPLOY:
		{ 
			//Second parameter is on/off
			SmartPtr<LuaBool> enable = PG_Dynamic_Cast<LuaBool>(params->Value[1]);
			if (!enable)
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability -- invalid type for parameter 2.  Ability %s expects a bool.", ability_name->Value.c_str());
				return false;
			}

			//Deploy event toggles deployed status, so if our state already matches the desired state then take no action.
			if(enable->Value == Object->Is_Unit_Special_Ability_Active(ABILITY_DEPLOY))
			{
				return false;
			}

			single_object_list.Add(Object);
		
			SetUnitAbilityModeEventClass event;
			event.Init(&single_object_list, ABILITY_DEPLOY, Object->Get_Owner_Player_ID());
			event.Set_UI_Request(false);
			event.Execute();

			//Deploying takes time.  Return true so that the caller can block
			return true;
		}

	case ABILITY_DISTRACT:
	case ABILITY_BARRAGE:
	case ABILITY_FOW_REVEAL_PING:
	case ABILITY_WEAKEN_ENEMY:
		{
			//A barrage or distract command needs a target position
			Vector3 target_position;
			if (!Lua_Extract_Position(params->Value[1], target_position))
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability -- invalid type for parameter 2.  Ability %s expects a target position.", ability_name->Value.c_str());
				return false;
			}
			single_object_list.Add(Object);
			SetUnitAbilityModeEventClass event(true);
			//Set the player so that we don't get the 'invalid action' buzzer if it fails.
			event.Init
			(
				&single_object_list, 
				ability_type, 
				Object->Get_Owner(),
				target_position
			);
			event.Set_UI_Request(false);
			event.Execute();

			//Although a barrage takes time there's no obvious point at which it's over - the units just fire at a point in space
			//whether there's anything much there or not and will continue to do so until given new orders.  We therefore treat this
			//as an instantaneous ability.  For a distract the effect is instantaneous anyway.
			return false;
		}

	case ABILITY_LURE:
	case ABILITY_HUNT:
	case ABILITY_AVOID_DANGER:
		{
			//Second parameter is on/off
			SmartPtr<LuaBool> enable = PG_Dynamic_Cast<LuaBool>(params->Value[1]);
			if (!enable)
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability -- invalid type for parameter 2.  Ability %s expects a bool.", ability_name->Value.c_str());
				return false;
			}

			//Using UI code for reference here - it appears that the event expects the team leader, not the team.
			if (Object->Behaves_Like(BEHAVIOR_TEAM))
			{
				single_object_list.Add(Object->Get_Team_Data()->Get_Team_Leader());
			}
			else
			{
				single_object_list.Add(Object);
			}

		
			SetUnitAbilityModeEventClass event;
			//Set the player so that we don't get the 'invalid action' buzzer if it fails.
			event.Init
			(
				&single_object_list, 
				ability_type,  
				Object->Get_Owner(),
				VECTOR3_NONE,
				true
			);
			event.Set_UI_Request(false);
			event.Execute();
			return false;
			//Oksana  [7/24/2005]: now goes through the same interface as all other abilities...
		}

	case ABILITY_CABLE_ATTACK:
	case ABILITY_FORCE_LIGHTNING:
	case ABILITY_FLAME_THROWER:
	case ABILITY_FORCE_TELEKINESIS:
	case ABILITY_STICKY_BOMB:
	case ABILITY_CAPTURE_VEHICLE:
	case ABILITY_TRACTOR_BEAM:
	case ABILITY_TARGETED_HACK:
	case ABILITY_TARGETED_REPAIR:
	case ABILITY_ENERGY_WEAPON:
	case ABILITY_LUCKY_SHOT:
	case ABILITY_CONCENTRATE_FIRE:
	case ABILITY_ION_CANNON_SHOT:
	case ABILITY_MAXIMUM_FIREPOWER:
	case ABILITY_SUPER_LASER:
	case ABILITY_BERSERKER:
	case ABILITY_SABER_THROW:
	case ABILITY_LEECH_SHIELDS:
	case ABILITY_TACTICAL_BRIBE:
	case ABILITY_PLACE_REMOTE_BOMB:
	case ABILITY_INFECTION:
		{
			//These abilities are targeted on a game object
			SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[1]);
			SmartPtr<AITargetLocationWrapper> ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[1]);
			if (!object_wrapper && !ai_target_wrapper)
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability -- invalid type for parameter 2.  Ability %s expects a target object.", ability_name->Value.c_str());
				return false;
			}

			if ((object_wrapper && !object_wrapper->Get_Object()) || (ai_target_wrapper && (!ai_target_wrapper->Get_Object() || !ai_target_wrapper->Get_Object()->Get_Target_Game_Object())))
			{
				script->Script_Warning("GameObjectWrapper::Activate_Ability -- target object is already dead.");
				return false;
			}
			GameObjectClass *target = object_wrapper ? object_wrapper->Get_Object() : ai_target_wrapper->Get_Object()->Get_Target_Game_Object();

			//Get the ID of the special ability tied to this unit ability (there better be one)
			int ability_name_crc = 0;
			if (ability_type == Object->Get_Type()->Get_Unit_Ability())
			{
				ability_name_crc = Object->Get_Type()->Get_GUI_Activated_Ability_Name_CRC(PRIMARY_ABILITY_INDEX);
			}
			else
			{
				ability_name_crc = Object->Get_Type()->Get_GUI_Activated_Ability_Name_CRC(SECONDARY_ABILITY_INDEX);
			}

			if (ability_name_crc == 0)
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability -- unit ability %s is not hooked up to special ability logic for unit %s.  Check the GUI_Activated_Ability XML entry.", ability_name->Value.c_str(), Object->Get_Type()->Get_Name()->c_str());
				return false;
			}

			single_object_list.Add(Object);
			TacticalSpecialAbilityEventClass event;
			//Set the player so that we don't get the 'invalid action' buzzer if it fails.
			event.Set_Player_ID(Object->Get_Owner());
			event.Init(single_object_list, target, ability_name_crc, ability_type);
			event.Execute();

			//Make sure the event went through ok - it won't if the ability was not recharged or if the target was inappropriate.
			//Provided we did queue up the ability we should instruct the caller to block since these abilities may require movement
			//or have a duration.
			if (Object->Get_Tactical_Combatant_Data() && 
				Object->Get_Tactical_Combatant_Data()->Get_Special_Ability_Name_CRC() == static_cast<CRCValue>(ability_name_crc))
			{
				return true;
			}
			else
			{
				return false;
			}
		}

	case ABILITY_TARGETED_INVULNERABILITY:
		{
			//Grrrrr!!  This appears to be the only ability that behaves this way
			SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[1]);
			SmartPtr<AITargetLocationWrapper> ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[1]);
			if (!object_wrapper && !ai_target_wrapper)
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability -- invalid type for parameter 2.  Ability %s expects a target object.", ability_name->Value.c_str());
				return false;
			}

			if ((object_wrapper && !object_wrapper->Get_Object()) || (ai_target_wrapper && (!ai_target_wrapper->Get_Object() || !ai_target_wrapper->Get_Object()->Get_Target_Game_Object())))
			{
				script->Script_Warning("GameObjectWrapper::Activate_Ability -- target object is already dead.");
				return false;
			}
			GameObjectClass *target = object_wrapper ? object_wrapper->Get_Object() : ai_target_wrapper->Get_Object()->Get_Target_Game_Object();

			single_object_list.Add(Object);
			SetUnitAbilityModeEventClass event(true);
			//Set the player so that we don't get the 'invalid action' buzzer if it fails.
			event.Init
			(
				&single_object_list, 
				ability_type,  
				Object->Get_Owner(),
				target->Get_Position(),
				true,
				target->Get_ID()
			);
			event.Set_UI_Request(false);
			event.Execute();			
			return false;
		}

	case ABILITY_AREA_EFFECT_STUN:
	case ABILITY_AREA_EFFECT_HEAL:
	case ABILITY_AREA_EFFECT_CONVERT:
	case ABILITY_FORCE_WHIRLWIND:
	case ABILITY_HARMONIC_BOMB:
	case ABILITY_UNTARGETED_STICKY_BOMB:
	case ABILITY_REPLENISH_WINGMEN:
	case ABILITY_DEPLOY_TROOPERS:
	case ABILITY_EJECT_VEHICLE_THIEF:
	case ABILITY_STUN:
	case ABILITY_FORCE_CONFUSE:
	case ABILITY_CLUSTER_BOMB:
	case ABILITY_SHIELD_FLARE:
		{
			//Very easy - these are point blank area of effect abilities.  No second parameter required.

			//Using UI code for reference here - it appears that the event expects the team leader, not the team.
			if (Object->Behaves_Like(BEHAVIOR_TEAM))
			{
				single_object_list.Add(Object->Get_Team_Data()->Get_Team_Leader());
			}
			else
			{
				single_object_list.Add(Object);
			}

			SetUnitAbilityModeEventClass event;
			//Set the player so that we don't get the 'invalid action' buzzer if it fails.
			event.Init
			(
				&single_object_list, 
				ability_type,  
				Object->Get_Owner(),
				VECTOR3_NONE,
				// ability_type == Object->Get_Type()->Get_Unit_Ability()
				true
			);
			event.Set_UI_Request(false);
			event.Execute();
			return false;
		}

	case ABILITY_JET_PACK:
		{
			Vector3 target_position;
			if (!Lua_Extract_Position(params->Value[1], target_position))
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability -- invalid type for parameter 2.  Ability %s expects a target position.", ability_name->Value.c_str());
				return false;
			}

			if(!Object->Get_Unit_Ability_Data(ability_type))
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability -- object does not seem to have valid ability data.", ability_name->Value.c_str());
				return false;
			}

			single_object_list.Add(Object);
			SetUnitAbilityModeEventClass event(true);
			//Set the player so that we don't get the 'invalid action' buzzer if it fails.
			event.Init
			(
				&single_object_list, 
				ability_type,  
				Object->Get_Owner(),
				target_position
				// ability_type == Object->Get_Type()->Get_Unit_Ability()
			);
			event.Set_UI_Request(false);
			event.Execute();

			//Check whether the locomotor switch took effect
			if (Object->Behaves_Like(BEHAVIOR_LOCO))
			{
				LocomotorInterfaceClass *locomotor = static_cast<LocomotorInterfaceClass*>(Object->Get_Behavior(BEHAVIOR_LOCO));
				if (locomotor && !locomotor->Is_Primary_Locomotor())
				{
				//Wait until the jet pack move is finished - the notification is sent at the point that we switch the locomotor back to walk
					return true;
				}
			}
			return false;
		}

		case ABILITY_PROXIMITY_MINES:
		case ABILITY_RADIOACTIVE_CONTAMINATE:
		case ABILITY_BUZZ_DROIDS:
		case ABILITY_SUMMON:
		case ABILITY_CORRUPT_SYSTEMS:
		{
			Vector3 target_position;
			if (!Lua_Extract_Position(params->Value[1], target_position))
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability -- invalid type for parameter 2.  Ability %s expects a target position.", ability_name->Value.c_str());
				return false;
			}

			// Create a dummy target to go to.
			// It will be removed by the ability code when in range.
			GameObjectTypeClass* dummy_type = GameObjectTypeManager.Find_Object_Type("Dummy_Barrage_Target");
			assert(dummy_type);
			Vector3 facing(VECTOR3_NONE);
			GameObjectClass* dummy_target = GAME_OBJECT_MANAGER.Create_Object_Of_Type(dummy_type, Object->Get_Owner(), target_position, facing);

			//Get the ID of the special ability tied to this unit ability (there better be one)
			int ability_name_crc = 0;
			if (ability_type == Object->Get_Type()->Get_Unit_Ability())
			{
				ability_name_crc = Object->Get_Type()->Get_GUI_Activated_Ability_Name_CRC(PRIMARY_ABILITY_INDEX);
			}
			else
			{
				ability_name_crc = Object->Get_Type()->Get_GUI_Activated_Ability_Name_CRC(SECONDARY_ABILITY_INDEX);
			}

			if (ability_name_crc == 0)
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability -- unit ability %s is not hooked up to special ability logic for unit %s.  Check the GUI_Activated_Ability XML entry.", ability_name->Value.c_str(), Object->Get_Type()->Get_Name()->c_str());
				return false;
			}

			TacticalSpecialAbilityEventClass event;
			event.Set_Player_ID(Object->Get_Owner());
			single_object_list.Add(Object);
			event.Init(single_object_list, dummy_target, ability_name_crc, ability_type);
			event.Execute();
			return true;
		}
		break;

	default:
		script->Script_Error("GameObjectWrapper::Activate_Ability -- unsupported unit ability %s.", ability_name->Value.c_str());
		return false;
	}
}

/**************************************************************************************************
* GameObjectWrapper::Has_Property -- Check whether this object's property flags match the parameter
*
* In:				
*
* Out:	
*
* History: 6/24/2005 6:25PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Has_Property(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Has_Property -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> property_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!property_name)
	{
		script->Script_Error("GameObjectWrapper::Has_Property -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	GameObjectPropertiesType properties = GAME_OBJECT_PROPERTIES_NONE;
	if (!TheGameObjectPropertiesTypeConverterPtr->String_To_Enum(property_name->Value, properties))
	{
		script->Script_Error("GameObjectWrapper::Has_Property -- unrecognized property flags %s.", property_name->Value.c_str());
		return NULL;
	}

	return Return_Variable(new LuaBool((properties & Object->Get_Company_Type()->Get_Property_Mask()) != 0));
}


/**************************************************************************************************
* GameObjectWrapper::Unlock_Current_Orders -- Remove a movement lock placed on this object by the AI
*
* In:				
*
* Out:	
*
* History: 6/30/2005 10:47AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Unlock_Current_Orders(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	//We'll have the freestore listen on the signals to unlock the movement.  
	PlayerClass *player = Object->Get_Owner_Player();
	FAIL_IF(!player) { return NULL; }
	if (!player->Get_Is_AI_Controlled())
	{
		script->Script_Error("GameObjectWrapper::Unlock_Current_Orders - object %s does not belong to an AI player; cannot unlock.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return NULL;
	}

	if (Object->Is_Engaged_In_Hero_Clash())
	{
		script->Script_Error("GameObjectWrapper::Unlock_Current_Orders - object %s is locked by a hero clash.  Cannot unlock.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return NULL;
	}

	TacticalAIManagerClass *tactical_manager = player->Get_AI_Player()->Get_Tactical_Manager_By_Mode(GameModeManager.Get_Sub_Type());
	AIFreeStoreClass *free_store = tactical_manager->Get_Execution_System()->Get_Free_Store();

	if (GameModeManager.Get_Sub_Type() == SUB_GAME_MODE_LAND && Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return NULL; }
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }

			if (!team_member->Is_Movement_Locked())
			{
				script->Script_Warning("GameObjectWrapper::Unlock_Current_Orders - object %s is not locked.", team_member->Get_Type()->Get_Name()->c_str());
				continue;
			}

			team_member->Set_Movement_Locked(false);

			SignalDispatcherClass::Get().Remove_Listener(team_member, free_store, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED);
			SignalDispatcherClass::Get().Remove_Listener(team_member, free_store, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED);		
		}
	}
	else
	{
		if (!Object->Is_Movement_Locked())
		{
			script->Script_Warning("GameObjectWrapper::Unlock_Current_Orders - object %s is not locked.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
			return NULL;
		}

		Object->Set_Movement_Locked(false);

		SignalDispatcherClass::Get().Remove_Listener(Object, free_store, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED);
		SignalDispatcherClass::Get().Remove_Listener(Object, free_store, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED);
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Play_Animation -- Animation control for an object.  Play an animation once or
*	looped.  Optional 3rd parameter specifies a particular animation sub-index.
*
* In:				
*
* Out:	
*
* History: 7/05/2005 12:49PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Play_Animation(LuaScriptClass *script, LuaTable *params)
{
	const static float SCRIPT_ANIMATION_BLEND_TIME = 0.33f;

	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 2 && params->Value.size() != 3)
	{
		script->Script_Error("GameObjectWrapper::Play_Animation -- invalid number of parameters.  Expected 2 or 3, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> anim_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!anim_name)
	{
		script->Script_Error("GameObjectWrapper::Play_Animation -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	ModelAnimType anim_type;
	if (!TheModelAnimTypeConverterPtr->String_To_Enum(anim_name->Value, anim_type))
	{
		script->Script_Error("GameObjectWrapper::Play_Animation -- invalid animation type %s.", anim_name->Value.c_str());
		return NULL;
	}

	SmartPtr<LuaBool> loop = PG_Dynamic_Cast<LuaBool>(params->Value[1]);
	if (!loop)
	{
		script->Script_Error("GameObjectWrapper::Play_Animation -- invalid type for parameter 2.  Expected boolean.");
		return NULL;
	}

	unsigned int anim_index = ANIM_SUBINDEX_RANDOM;
	if (params->Value.size() == 3)
	{
		SmartPtr<LuaNumber> lua_index = PG_Dynamic_Cast<LuaNumber>(params->Value[2]);
		if (!lua_index)
		{
			script->Script_Error("GameObjectWrapper::Play_Animation -- invalid type for parameter 3.  Expected number.");
			return NULL;
		}
		anim_index = static_cast<unsigned int>(lua_index->Value + 0.5f);
	}

	if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return NULL; }
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }
			team_member->Set_Target_Animation_Type(anim_type, anim_index, loop->Value, SCRIPT_ANIMATION_BLEND_TIME);
		}
	}
	else
	{
		Object->Set_Target_Animation_Type(anim_type, anim_index, loop->Value, SCRIPT_ANIMATION_BLEND_TIME);
	}

	UnitAnimationBlockStatus *blocking_object = static_cast<UnitAnimationBlockStatus*>(UnitAnimationBlockStatus::FactoryCreate());
	FAIL_IF(!blocking_object) { return NULL; }
	blocking_object->Init(this, anim_type);

	return Return_Variable(blocking_object);
}

LuaTable *GameObjectWrapper::Is_On_Diversion(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (!Object->Behaves_Like(BEHAVIOR_LOCO))
	{
		script->Script_Error("GameObjectWrapper::Is_On_Diversion -- this object cannot move");
		return NULL;
	}

	FormationClass *formation = Object->Get_Locomotor_Data()->Get_Formation();
	if (!formation)
	{
		return Return_Variable(new LuaBool(false));
	}

	return Return_Variable(new LuaBool(formation->Get_Destination().MovementType.Has_Type(UNIT_AI_MOVE_TYPE_SCRIPT_DIVERSION)));
}

/**************************************************************************************************
* GameObjectWrapper::Get_Affiliated_Indigenous_Type -- Get hold of the friendly indigenous unit type
*	that has the strongest presence on this planet (invalid for other kinds of objects)
*
* In:				
*
* Out:	
*
* History: 7/28/2005 12:48PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Affiliated_Indigenous_Type(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (!Object->Behaves_Like(BEHAVIOR_PLANET))
	{
		script->Script_Error("GameObjectWrapper::Get_Affiliated_Indigenous_Type -- this function only applies to planets.");
		return NULL;
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Get_Affiliated_Indigenous_Type -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<PlayerWrapper> lua_player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!lua_player)
	{
		script->Script_Error("GameObjectWrapper::Get_Affiliated_Indigenous_Type -- invalid type for parameter 1.  Expected player.");
		return NULL;
	}

	PlayerClass *player = lua_player->Get_Object();
	FAIL_IF(!player) { return NULL; }

	const GameObjectTypeClass *affiliated_type = NULL;
	int indigenous_count = Object->Get_Type()->Get_Potential_Indigenous_Unit_Count();
	float best_indigenous_power = 0.0f;
	for (int i = 0; i < indigenous_count; ++i)
	{
		const GameObjectTypeClass *indigenous_type = Object->Get_Type()->Get_Potential_Indigenous_Unit_Type(i);
		FAIL_IF(!indigenous_type) { continue; }
		float indigenous_power = Object->Get_Type()->Get_Potential_Indigenous_Unit_Power(i);

		if (indigenous_power <= best_indigenous_power)
		{
			continue;
		}

		for (int j = 0; j < PlayerList.Get_Num_Players(); ++j)
		{
			PlayerClass *ally = PlayerList.Get_Player_By_Index(j);
			if (!ally)
			{
				continue;
			}
			
			if (!ally->Is_Ally(player))
			{
				continue;
			}

			if (indigenous_type->Is_Affiliated_With_Faction(ally->Get_Faction()))
			{
				affiliated_type = indigenous_type;
				best_indigenous_power = indigenous_power;
				break;
			}
		}
	}

	if (affiliated_type)
	{
		return Return_Variable(GameObjectTypeWrapper::Create(const_cast<GameObjectTypeClass*>(affiliated_type), script));
	}
	else
	{
		return NULL;
	}
}

/**************************************************************************************************
* GameObjectWrapper::Is_Planet_Destroyed -- Script function to check whether a planet has been vaporized
*
* In:				
*
* Out:	
*
* History: 8/2/2005 6:30PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_Planet_Destroyed(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (!Object->Behaves_Like(BEHAVIOR_PLANET))
	{
		script->Script_Error("GameObjectWrapper::Is_Planet_Destroyed -- this function is only valid on planet objects.");
		return NULL;
	}

	return Return_Variable(new LuaBool(Object->Get_Planetary_Data()->Planet_Has_Been_Destroyed()));
}

/**************************************************************************************************
* GameObjectWrapper::Turn_To_Face -- Script function to make an object face a given point (object, ai target, position etc.)
*
* In:				
*
* Out:	
*
* History: 8/3/2005 5:49PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Turn_To_Face(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Turn_To_Face -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	Vector3 face_position;
	if (!Lua_Extract_Position(params->Value[0], face_position))
	{
		script->Script_Error("GameObjectWrapper::Turn_To_Face -- could not deduce a position to face.");
		return NULL;
	}

	//Go ahead and execute the move.  No blocking object required.
	MovementCoordinatorSystemClass *system = GameModeManager.Get_Active_Mode()->Get_Movement_Coordinator_System();
	MovementCoordinatorClass *coordinator = 0;

	if (!system)
	{
		script->Script_Error("GameObjectWrapper::Turn_To_Face -- unable to locate movement coordinator system.  This is very bad.");
		return NULL;
	}

	coordinator = system->Create();

	if (!coordinator)
	{
		script->Script_Error("GameObjectWrapper::Turn_To_Face -- unable to create a movement coordinator for this attack.  This is also very bad.");
		return NULL;
	}

	if (system->Can_Object_Be_In_Formations(Object))
	{
		system->Remove_Object_From_Coordinators(Object);

		//Notify any blocking objects waiting on this object's current move - we'll behave as if it's finished since
		//it's now doing something completely different.
		SignalDispatcherClass::Get().Send_Signal(Object, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL);

		coordinator->Add_Object(Object);
	}
	else if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return NULL; }

		//Notify any blocking objects waiting on this object's current move - we'll behave as if it's finished since
		//it's now doing something completely different.
		SignalDispatcherClass::Get().Send_Signal(Object, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL);

		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }

			system->Remove_Object_From_Coordinators(team_member);
			SignalDispatcherClass::Get().Send_Signal(team_member, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL);
			coordinator->Add_Object(team_member);
		}
	}
	else
	{
		system->Destroy(coordinator);
		script->Script_Error("GameObjectWrapper::Move_Internal -- unable to make object %s move.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	UnitMovementBlockStatus *bs = (UnitMovementBlockStatus *)UnitMovementBlockStatus::FactoryCreate();
	bs->Init(this, Return_Variable(this));

	FormationDestinationStruct destination;
	destination.Type = FDT_FACING;
	FacingClass desired_facing = FacingClass(Object->Get_Position(), face_position);
	destination.ZAngle = desired_facing.Get()->Z;
	destination.EnforceTopSpeed = false;
	
	coordinator->Set_Destination(destination);

	return Return_Variable(bs);
}

/**************************************************************************************************
* GameObjectWrapper::Is_Ability_Active -- Script function to check whether a given ability is currently active
*
* In:				
*
* Out:	
*
* History: 8/5/2005 4:12PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_Ability_Active(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Is_Ability_Active -- invalid number of parameters.  Expected 1, got %d.");
		return NULL;
	}

	SmartPtr<LuaString> ability_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!ability_name)
	{
		script->Script_Error("GameObjectWrapper::Is_Ability_Active -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	UnitAbilityType ability_type = ABILITY_NONE;
	if (!TheUnitAbilityTypeConverterPtr->String_To_Enum(ability_name->Value, ability_type))
	{
		script->Script_Error("GameObjectWrapper::Is_Ability_Active -- unrecognized ability %s.", ability_name->Value.c_str());
		return NULL;
	}

	return Return_Variable(new LuaBool(Object->Is_Unit_Special_Ability_Active(ability_type)));
}

/**************************************************************************************************
* GameObjectWrapper::Is_In_Nebula -- Script function to check whether the object is inside a nebula
*
* In:				
*
* Out:	
*
* History: 8/9/2005 8:30PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_In_Nebula(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	return Return_Variable(new LuaBool(Object->Is_In_Nebula()));
}

/**************************************************************************************************
* GameObjectWrapper::Is_In_Ion_Storm -- Script function to check whether the object is inside an ion storm
*
* In:				
*
* Out:	
*
* History: 8/9/2005 8:30PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_In_Ion_Storm(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	return Return_Variable(new LuaBool(Object->Is_In_Ion_Storm()));
}

/**************************************************************************************************
* GameObjectWrapper::Is_In_Asteroid_Field -- Script function to check whether the object is inside an
*	asteroid field
*
* In:				
*
* Out:	
*
* History: 8/9/2005 8:30PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_In_Asteroid_Field(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	return Return_Variable(new LuaBool(Object->Is_In_Asteroid_Field()));
}

/**************************************************************************************************
* GameObjectWrapper::Is_Under_Effects_Of_Ability -- Script function to check whether the object is uner the effects
*	of the named unit ability
*
* In:				
*
* Out:	
*
* History: 8/10/2005 10:41AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_Under_Effects_Of_Ability(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Is_Under_Effects_Of_Ability -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	//Work back to ability type.
	SmartPtr<LuaString> ability_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!ability_name)
	{
		script->Script_Error("GameObjectWrapper::Is_Under_Effects_Of_Ability -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	UnitAbilityType ability = ABILITY_NONE;
	if (!TheUnitAbilityTypeConverterPtr->String_To_Enum(ability_name->Value, ability))
	{
		script->Script_Error("GameObjectWrapper::Is_Under_Effects_Of_Ability -- unknown ability %s.", ability_name->Value.c_str());
		return NULL;
	}

	//Logic varies some depending on ability type
	bool affected = false;
	switch (ability)
	{
	case ABILITY_ION_CANNON_SHOT:
		{
			if (Object->Behaves_Like(BEHAVIOR_ION_STUN_EFFECT))
			{
				IonStunEffectBehaviorClass *stunned_behavior = static_cast<IonStunEffectBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_ION_STUN_EFFECT));
				FAIL_IF(!stunned_behavior) { return NULL; }

				//The service of the stun behavior is only enabled when the object is actually stunned
				affected = stunned_behavior->Is_Service_Enabled();
			}
		}
		break;

	case ABILITY_TRACTOR_BEAM:
		{
			//Flag should have been set by the ability
			TacticalCombatantDataPackClass *td = Object->Get_Tactical_Combatant_Data();
			affected = td && td->Is_In_Tractor_Beam();
		}
		break;

	default:
		script->Script_Error("GameObjectWrapper::Is_Under_Effects_Of_Ability -- ability %s is not yet supported.  You'll need some programming help if you think it should be.", ability_name->Value.c_str());
		break;
	}
	
	return Return_Variable(new LuaBool(affected));
}

/**************************************************************************************************
* GameObjectWrapper::Build -- Request that a build pad start constructing a given object type
*
* In:				
*
* Out:	
*
* History: 8/10/2005 3:08PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Build(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1 && params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Build -- invaid number of parameters.  Expected 1 or 2, got %d.", params->Value.size());
		return NULL;
	}

	//Accept the type to build either as a string or a type wrapper
	SmartPtr<LuaString> type_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	SmartPtr<GameObjectTypeWrapper> lua_type = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
	const GameObjectTypeClass *type = NULL;
	if (lua_type)
	{
		type = lua_type->Get_Object();
	}
	else if (type_name)
	{
		type = GameObjectTypeManager.Find_Object_Type(type_name->Value);
		if (!type)
		{
			script->Script_Error("GameObjectWrapper::Build -- unknown game object type %s.", type_name->Value.c_str());
			return NULL;
		}
	}
	else
	{
		script->Script_Error("GameObjectWrapper::Build -- invalid type for parameter 1.  Expected type or type name.");
		return NULL;
	}

	TacticalBuildObjectsBehaviorClass *build_behave = static_cast<TacticalBuildObjectsBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TACTICAL_BUILD_OBJECTS));
	if (!build_behave)
	{
		script->Script_Error("GameObjectWrapper::Build -- Object %s is not a valid build location.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	//There's just no way we can build 2 things on the same pad.  Sorry.
	if (build_behave->Get_Object_Under_Construction(Object) || build_behave->Get_Constructed_Object(Object))
	{
		script->Script_Error("GameObjectWrapper::Build -- Object %s already has something built on it.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	if (params->Value.size() == 2)
	{
		//If there is a second parameter it should indicate whether or not we care to enforce standard game logic rules regarding this build
		SmartPtr<LuaBool> enforce_rules = PG_Dynamic_Cast<LuaBool>(params->Value[1]);
		if (!enforce_rules)
		{
			script->Script_Error("GameObjectWrapper::Build -- invalid type for parameter 2.  Expected boolean.");
			return NULL;
		}

		if (enforce_rules->Value)
		{
			//Check that the type is genuinely something this build pad can output
			bool can_build_here = false;
			int buildable_type_count = Object->Get_Type()->Get_Num_Tactical_Buildable_Object_Types(Object->Get_Owner_Faction());
			for (int i = 0; i < buildable_type_count; ++i)
			{
				if (Object->Get_Type()->Get_Tactical_Buildable_Object_Type(Object->Get_Owner_Faction(), i) == type)
				{
					can_build_here = true;
					break;
				}
			}

			//Verify that building is not obstructed e.g. by nearby enemy presence
			can_build_here &= build_behave->Is_New_Construction_Allowed_Here(Object);

			if (!can_build_here)
			{
				//Starting the build would violate game rules.  Report failure.
				script->Script_Warning("GameObjectWrapper::Build -- game rules prevent building %s at %s.", type->Get_Name()->c_str(), Object->Get_Type()->Get_Name()->c_str());
				return Return_Variable(new LuaBool(false));
			}
		}
	}

	//Go ahead and ask to build.  Don't charge credits and don't play HUD audio even if we're building units for the human player.
	if (!build_behave->Execute_Start_Building_Tactical_object(Object, type, Object->Get_Owner(), false, false))
	{
		script->Script_Error("GameObjectWrapper::Build -- unknown error starting build of %s at %s.  Perhaps there are some asserts to offer a clue?", type->Get_Name()->c_str(), Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	//Report that we started building
	return Return_Variable(new LuaBool(true));
}

/**************************************************************************************************
* GameObjectWrapper::Make_Invulnerable -- Allows for turning on and off invulnerability on an object
*
* In:				
*
* Out:	
*
* History: 8/11/2005 1:05PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Make_Invulnerable(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Make_Invulnerable -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	if (!GameModeManager.Get_Active_Mode() || !GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical())
	{
		script->Script_Error("GameObjectWrapper::Make_Invulnerable -- this function may only be used in tactical modes.");
		return NULL;
	}

	SmartPtr<LuaBool> on_off = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!on_off)
	{
		script->Script_Error("GameObjectWrapper::Make_Invulnerable -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	//By default when we attach an invulnerability behavior to an object it lasts forever.  We could potentially
	//update this function to also accept a time-out since it's possible to schedule automatic removal of the 
	//invulnerability

	//In the case of a team, switch invulnerability on all members
	if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return NULL; }
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }
			if (on_off->Value && !team_member->Behaves_Like(BEHAVIOR_INVULNERABLE))
			{
				team_member->Add_Behavior(BehaviorClass::Create_Behavior("INVULNERABLE"));
			}
			else if (!on_off->Value && team_member->Behaves_Like(BEHAVIOR_INVULNERABLE))
			{
				team_member->Remove_Behavior(static_cast<BehaviorClass*>(team_member->Get_Behavior(BEHAVIOR_INVULNERABLE)));
			}
		}
	}
	else
	{
		if (on_off->Value && !Object->Behaves_Like(BEHAVIOR_INVULNERABLE))
		{
			Object->Add_Behavior(BehaviorClass::Create_Behavior("INVULNERABLE"));
		}
		else if (!on_off->Value && Object->Behaves_Like(BEHAVIOR_INVULNERABLE))
		{
			Object->Remove_Behavior(static_cast<BehaviorClass*>(Object->Get_Behavior(BEHAVIOR_INVULNERABLE)));
		}
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Teleport -- Instantly move a unit to a give location
*
* In:				
*
* Out:	
*
* History: 8/11/2005 6:07PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Teleport(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (!GameModeManager.Get_Active_Mode() || !GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical())
	{
		script->Script_Error("GameObjectWrapper::Teleport -- this function may only be used in tactical modes.");
		return NULL;
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Teleport -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	Vector3 target_position;
	if (!Lua_Extract_Position(params->Value[0], target_position))
	{
		script->Script_Error("GameObjectWrapper:: Teleport -- unable to deduce a position from parameter 1.");
		return NULL;
	}

	Object->Remove_From_Garrisons_Immediately();
	Object->Teleport(target_position);

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Teleport -- Instantly move a unit to a give location
*
* In:				
*
* Out:	
*
* History: 8/11/2005 6:07PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Teleport_And_Face(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (!GameModeManager.Get_Active_Mode() || !GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical())
	{
		script->Script_Error("GameObjectWrapper::Teleport_And_Face -- this function may only be used in tactical modes.");
		return NULL;
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Teleport_And_Face -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	GameObjectClass* pobj = NULL;
	// Get the game object if one was passed in

	SmartPtr<PositionWrapper> position_wrapper = PG_Dynamic_Cast<PositionWrapper>(params->Value[0]);

	if(position_wrapper)
	{
		script->Script_Error("GameObjectWrapper:: Teleport_And_Face -- Expected game object for parameter 1, got position instead.");
		return NULL;
	}

	SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);

	if(object_wrapper)
	{
		pobj = object_wrapper->Get_Object();
		FAIL_IF( pobj == NULL )
		{
			script->Script_Error("GameObjectWrapper::Teleport_And_Face -- Invalid game object given for parameter 1.");
			return NULL;
		}
	}
	else
	{
		script->Script_Error("GameObjectWrapper:: Teleport_And_Face -- Expected game object for parameter 1.");
		return NULL;
	}

	Object->Remove_From_Garrisons_Immediately();
	Object->Teleport(pobj->Get_Position());
	Object->Set_Facing(pobj->Get_Facing());

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Hyperspace_Away -- Hyperspace an object out of a space battle, possibly destroying the
*	galactic representation
*
* In:				
*
* Out:	
*
* History: 8/11/2005 6:07PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Hyperspace_Away(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_SPACE)
	{
		script->Script_Error("GameObjectWrapper::Hyperspace_Away -- this function may only be used in space mode.");
		return NULL;
	}

	if (params->Value.size() > 1)
	{
		script->Script_Error("GameObjectWrapper::Hyperspace_Away -- invalid number of parameters.  Expected 0 or 1, got %d.", params->Value.size());
		return NULL;
	}

	bool destroy_parent = true;
	if (params->Value.size() > 0)
	{
		SmartPtr<LuaBool> lua_destroy_parent = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
		if (!destroy_parent)
		{
			script->Script_Error("GameObjectWrapper::Hyperspace_Away -- invalid type for parameter 1.  Expected boolean.");
			return NULL;
		}
		destroy_parent = lua_destroy_parent->Value;
	}

	if (!Object->Behaves_Like(BEHAVIOR_LOCO))
	{
		script->Script_Error("GameObjectWrapper::Hyperspace_Away -- object %s can't move, so it certainly can't hyperspace.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	//Hyperspace_Away is currently only going to have an effect for non-fighter space units (i.e. those that use
	//the SIMPLE_SPACE_LOCOMOTOR).
	LocomotorInterfaceClass *locomotor = static_cast<LocomotorInterfaceClass*>(Object->Get_Behavior(BEHAVIOR_LOCO));
	locomotor->Hyperspace_Away(destroy_parent);

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Hyperspace_In -- Hyperspace an object into a space battle
*
* In:				
*
* Out:	
*
* History: 8/11/2005 6:07PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Cinematic_Hyperspace_In(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_SPACE)
	{
		script->Script_Error("GameObjectWrapper::Cinematic_Hyperspace_In -- this function may only be used in space mode.");
		return NULL;
	}

	if (params->Value.size() < 1)
	{
		script->Script_Error("GameObjectWrapper::Cinematic_Hyperspace_In -- invalid number of parameters.  Expected interger value for delay frames");
		return NULL;
	}

	unsigned int delay_frames = 0;

	SmartPtr<LuaNumber> float_delay_frames  = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	
	if (!float_delay_frames)
	{
		script->Script_Error("GameObjectWrapper::Cinematic_Hyperspace_In -- invalid type for parameter 1.  Expected number.");
		return NULL;
	}
	
	delay_frames = static_cast<unsigned int>(float_delay_frames->Value + 0.5f);

	if (!Object->Behaves_Like(BEHAVIOR_LOCO))
	{
		script->Script_Error("GameObjectWrapper::Cinematic_Hyperspace_In -- object %s can't move, so it certainly can't hyperspace.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	if(Object->Get_Type()->Is_Hyperspace_Capable())
	{
		//Cinematic_Hyperspace_In is currently only going to have an effect for non-fighter space units (i.e. those that use
		//the SIMPLE_SPACE_LOCOMOTOR).
		LocomotorInterfaceClass *locomotor = static_cast<LocomotorInterfaceClass*>(Object->Get_Behavior(BEHAVIOR_LOCO));

		if(locomotor->Is_Hyperspace_Capable())
		{
			locomotor->Cinematic_Hyperspace_In(delay_frames);
		}
	}
	return NULL;
}


/**************************************************************************************************
* GameObjectWrapper::Hyperspace_In -- Hyperspace an object into a space battle
*
* In:				
*
* Out:	
*
* History: 8/11/2005 6:07PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Cancel_Hyperspace(LuaScriptClass * /*script*/, LuaTable * /*params*/)
{
	FAIL_IF(!Object) { return NULL; }

	if (GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_SPACE)
	{
		return NULL;
	}

	if (!Object->Behaves_Like(BEHAVIOR_LOCO))
	{
		return NULL;
	}

	//Cinematic_Hyperspace_In is currently only going to have an effect for non-fighter space units (i.e. those that use
	//the SIMPLE_SPACE_LOCOMOTOR).
	LocomotorInterfaceClass *locomotor = static_cast<LocomotorInterfaceClass*>(Object->Get_Behavior(BEHAVIOR_LOCO));
	locomotor->Cancel_Hyperspace();

	return NULL;
}


/**************************************************************************************************
* GameObjectWrapper::Lock_Build_Pad_Contents -- Script function to lock in the current contents of a build pad
*	so that they cannot be sold.  The lock expires when the current built object is destroyed.
*
* In:				
*
* Out:	
*
* History: 8/11/2005 7:53PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Lock_Build_Pad_Contents(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Lock_Build_Pad_Contents -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaBool> lock = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!lock)
	{
		script->Script_Error("GameObjectWrapper::Lock_Build_Pad_Contents -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	TacticalBuildObjectsDataPackClass *build_data = Object->Get_Tactical_Build_Objects_Data();
	if (!build_data)
	{
		script->Script_Error("GameObjectWrapper::Lock_Build_Pad_Contents -- %s is not a build pad.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	if (!build_data->Get_Unit_Under_Construction() && !build_data->Get_Constructed_Unit())
	{
		script->Script_Warning("GameObjectWrapper::Lock_Build_Pad_Contents -- cannot lock an empty build pad.");
		return NULL;
	}

	build_data->Set_Contents_Locked(lock->Value);

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Get_Bone_Position -- Script function to get the world position of the named bone
*
* In:				
*
* Out:	
*
* History: 8/15/2005 7:40PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Bone_Position(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Get_Bone_Position -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> bone_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!bone_name)
	{
		script->Script_Error("GameObjectWrapper::Get_Bone_Position -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	if (!Object->Get_Model() || !Object->Get_Model()->Get_HModel())
	{
		script->Script_Error("GameObjectWrapper::Get_Bone_Position -- object %s has no model.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	Vector3 bone_position;
	int bone_index = Object->Get_Model()->Get_HModel()->Find_Bone_Index(bone_name->Value.c_str());
	if (bone_index < 0)
	{
		script->Script_Error("GameObjectWrapper::Get_Bone_Position -- could not find named bone %s in model for object %s.", bone_name->Value.c_str(), Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	Mat3DClass bone_tm;
	Object->Get_Model()->Get_HModel()->Get_Bone_Transform(bone_index, &bone_tm);
	bone_tm.Get_Translation(&bone_position);

	return Return_Variable(PositionWrapper::Create(bone_position));
}

/**************************************************************************************************
* GameObjectWrapper::Move_Galactic -- Internal implementation of movement command for fleets in galactic mode
*
* In:				
*
* Out:	
*
* History: 8/16/2005 2:52PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Move_Galactic(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Move_Galactic -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<AITargetLocationWrapper> ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);
	SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);

	GameObjectClass *planet = NULL;
	if (ai_target_wrapper && ai_target_wrapper->Get_Object())
	{
		planet = ai_target_wrapper->Get_Object()->Get_Target_Game_Object();
	}
	else if (object_wrapper)
	{
		planet = object_wrapper->Get_Object();
	}

	if (!planet || !planet->Behaves_Like(BEHAVIOR_PLANET))
	{
		script->Script_Error("GameObjectWrapper::Move_Galactic -- invalid type for parameter 1.  Expected planet.");
		return NULL;
	}

	//Only fleets may be moved around in galactic
	if (!Object->Behaves_Like(BEHAVIOR_FLEET))
	{
		script->Script_Error("GameObjectWrapper::Move_Galactic -- this function is only valid on fleets.");
		return NULL;
	}

	//The blocking object takes care of finding the path and sending the fleet along its way
   MovementBlockStatus *bs = static_cast<MovementBlockStatus*>(MovementBlockStatus::FactoryCreate());
	bs->Init(script, this, Create(planet, script), GOAL_REACHABILITY_ANY);

	return Return_Variable(bs);
}

/**************************************************************************************************
* GameObjectWrapper::Activate_Ability_Galactic -- Internal implementation of ability activation for galactic
*	mode
*
* In:				
*
* Out:	
*
* History: 8/18/2005 4:49PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Activate_Ability_Galactic(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	//Allow for invoking this command on a fleet in which case we may want to specify what unit type
	//in the fleet is supposed to perform an ability
	if (params->Value.size() > 1)
	{
		script->Script_Error("GameObjectWrapper::Activate_Ability_Galactic -- invalid number of parameters.  Expected 0 or 1, got %d.", params->Value.size());
		return NULL;
	}

	const GameObjectTypeClass *type_filter = NULL;
	if (params->Value.size() > 0)
	{
		SmartPtr<GameObjectTypeWrapper> type_wrapper = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
		SmartPtr<LuaString> type_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (type_wrapper)
		{
			type_filter = type_wrapper->Get_Object();
		}
		else if (type_name)
		{
			type_filter = GameObjectTypeManager.Find_Object_Type(type_name->Value);
			if (!type_filter)
			{
				script->Script_Error("GameObjectWrapper::Activate_Ability_Galactic -- unknown object type %s.", type_name->Value.c_str());
				return NULL;
			}
		}
		else
		{
			script->Script_Error("GameObjectWrapper::Activate_Ability_Galactic -- invalid type for parameter 1.  Expected object type.");
			return NULL;
		}
	}

	//There's no selecting which ability to activate in galactic mode.  We just attempt to deploy the unit on a planet
	//and see if anything fires.
	GameObjectClass *planet = NULL;
	GameObjectClass *parent_object = Object->Get_Parent_Container_Object();
	if (!parent_object)
	{
		script->Script_Error("GameObjectWrapper::Activate_Ability_Galactic -- %s is not at a planet.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}
	else if (parent_object->Behaves_Like(BEHAVIOR_PLANET))
	{
		if (!Object->Behaves_Like(BEHAVIOR_FLEET))
		{
			script->Script_Error("GameObjectWrapper::Activate_Ability_Galactic -- %s is landed.  Must be in orbit to activate abilities.", Object->Get_Type()->Get_Name()->c_str());
			return NULL;
		}

		planet = parent_object;

		DynamicVectorClass<GameObjectClass*> fleet_contents;
		Object->Get_Fleet_Breakdown(fleet_contents, false);

		for (int i = 0; i < fleet_contents.Size(); ++i)
		{
			GameObjectClass *ship = fleet_contents[i];
			FAIL_IF(!ship) { continue; }

			if (type_filter && ship->Get_Original_Object_Type() != type_filter)
			{
				continue;
			}

			if (ship->Get_Behavior(BEHAVIOR_TRANSPORT))
			{
				TransportBehaviorClass *behave = (TransportBehaviorClass *)ship->Get_Behavior(BEHAVIOR_TRANSPORT);
				for (int i = 0; i < (int)behave->Get_Contained_Objects_Count(); i++)
				{
					GameObjectClass *unit = behave->Get_Contained_Object(ship, i);
					unit->On_Deployed_To_Planet(planet);
				}
			}
			else
			{
				ship->On_Deployed_To_Planet(planet);
			}
		}
	}
	else if (parent_object->Behaves_Like(BEHAVIOR_FLEET))
	{
		planet = parent_object->Get_Parent_Container_Object();
		if (!planet)
		{
			script->Script_Error("GameObjectWrapper::Activate_Ability_Galactic -- %s is not at a planet.", Object->Get_Type()->Get_Name()->c_str());
			return NULL;
		}

		if (Object->Get_Behavior(BEHAVIOR_TRANSPORT))
		{
			TransportBehaviorClass *behave = (TransportBehaviorClass *)Object->Get_Behavior(BEHAVIOR_TRANSPORT);
			for (int i = 0; i < (int)behave->Get_Contained_Objects_Count(); i++)
			{
				GameObjectClass *unit = behave->Get_Contained_Object(Object, i);
				unit->On_Deployed_To_Planet(planet);
			}
		}
		else
		{
			Object->On_Deployed_To_Planet(planet);
		}
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Fire_Tactical_Superweapon -- Script function to fire a tactical superweapon
*
* In:				
*
* Out:	
*
* History: 8/18/2005 4:49PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Fire_Tactical_Superweapon(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (!Object->Behaves_Like(BEHAVIOR_TACTICAL_SUPER_WEAPON))
	{
		script->Script_Error("GameObjectWrapper::Fire_Tactical_Superweapon -- %s is not a tactical superweapon.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	//JSY 12/12/2005 - Changed to directly start the attack sequence rather than calling User_Request_Next_Attack_Stage which would attempt to schedule
	//an event for the local player.
	TacticalSuperWeaponBehaviorClass *tsw_behavior = static_cast<TacticalSuperWeaponBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TACTICAL_SUPER_WEAPON));
	return Return_Variable(new LuaBool(tsw_behavior->Execute_Start_Attack_Power_Up_Sequence()));
}

/**************************************************************************************************
* GameObjectWrapper::Is_Tactical_Superweapon_Ready -- Script function to check whether a tactical superweapon
*	is ready to attack
*
* In:				
*
* Out:	
*
* History: 8/18/2005 4:51PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_Tactical_Superweapon_Ready(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (!Object->Behaves_Like(BEHAVIOR_TACTICAL_SUPER_WEAPON))
	{
		script->Script_Error("GameObjectWrapper::Fire_Tactical_Superweapon -- %s is not a tactical superweapon.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	TacticalSuperWeaponBehaviorClass *tsw_behavior = static_cast<TacticalSuperWeaponBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TACTICAL_SUPER_WEAPON));
	return Return_Variable(new LuaBool(tsw_behavior->Is_In_Range_And_Ready_To_Attack()));
}

/**************************************************************************************************
* GameObjectWrapper::Set_Garrison_Spawn -- Script function to set whether or not an object is allowed
*	to spit out garrison units
*
* In:				
*
* Out:	
*
* History: 8/18/2005 4:57PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Set_Garrison_Spawn(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Set_Garrison_Spawn -- invalid number of parameters.  Expected 1, got %d.");
		return NULL;
	}

	SmartPtr<LuaBool> on_off = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!on_off)
	{
		script->Script_Error("GameObjectWrapper::Set_Garrison_Spawn -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	if (!Object->Behaves_Like(BEHAVIOR_SPAWN_SQUADRON))
	{
		script->Script_Error("GameObjectWrapper::Set_Garrison_Spawn -- %s is set up to be capable of spawning garrisons.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	Object->Set_Garrison_Spawn_Enabled(on_off->Value);
	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Prevent_Opportunity_Fire -- Make it so this unit will not take shots of opportunity.
*	While in effect, other units will not take shots of opportunity on it either.
*
* In:				
*
* Out:	
*
* History: 8/20/2005 3:06PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Prevent_Opportunity_Fire(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Prevent_Opportunity_Fire -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaBool> disable = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!disable)
	{
		script->Script_Error("GameObjectWrapper::Prevent_Opportunity_Fire -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return NULL; }
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }
			team_member->Set_Is_Opportunity_Fire_Disabled(disable->Value);
		}		
	}

	Object->Set_Is_Opportunity_Fire_Disabled(disable->Value);

	return NULL;
}


/**************************************************************************************************
* GameObjectWrapper::Get_Hint -- Return the hint text for this object.
*
* In:				
*
* Out:	
*
* History: 8/22/2005 1:34:08 PM -- BMH
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Hint(LuaScriptClass *, LuaTable *)
{
	if (Object && Object->Get_Hint_Data())
	{
		return Return_Variable(new LuaString(Object->Get_Hint_Data()->Get_Hint_String()));
	}
	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Set_Cannot_Be_Killed -- Control whether or not this object can take fatal damage.
*	Damage that will not kill the object is applied as normal.
*
* In:				
*
* Out:	
*
* History: 8/23/2005 5:13 PM -- JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Set_Cannot_Be_Killed(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Set_Cannot_Be_Killed -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaBool> cannot_kill = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!cannot_kill)
	{
		script->Script_Error("GameObjectWrapper::Set_Cannot_Be_Killed -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return NULL; }
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }
			team_member->Set_Cannot_Be_Killed(cannot_kill->Value);
		}		
	}

	Object->Set_Cannot_Be_Killed(cannot_kill->Value);

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Play_SFX_Event -- Play a named 3d sound attached to this object
*
* In:				
*
* Out:	
*
* History: 8/24/2005 4:32 PM -- JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Play_SFX_Event(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1 && params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Play_SFX_Event -- invalid number of parameters.  Expected 1 or 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> sfx_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!sfx_name)
	{
		script->Script_Error("GameObjectWrapper::Play_SFX_Event -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	float fade_time = 0.0f;
	if (params->Value.size() == 2)
	{
		SmartPtr<LuaNumber> lua_fade_time = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
		if (!lua_fade_time)
		{
			script->Script_Error("GameObjectWrapper::Play_SFX_Event -- invalid type for parameter 2.  Expected number.");
			return NULL;
		}
		fade_time = lua_fade_time->Value;
	}

	SFXEventIDType sfx_id = TheSFXEventManager.Start_SFX_Event(sfx_name->Value, Object, fade_time);
	if (sfx_id == SFX_EVENT_ID_INVALID)
	{
		script->Script_Error("GameObjectWrapper::Play_SFX_Event -- could not find sfx event %s.", sfx_name->Value.c_str());
		return NULL;
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Force_Test_Space_Conflict -- Check for a space conflict.  This object had better
*	be either a fleet or a planet.
*
* In:				
*
* Out:	
*
* History: 8/24/2005 4:32 PM -- JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Force_Test_Space_Conflict(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	GameObjectClass *fleet = NULL;
	GameObjectClass *planet = NULL;

	if (Object->Behaves_Like(BEHAVIOR_FLEET))
	{
		fleet = Object;
		planet = Object->Get_Parent_Container_Object();
	}
	else if (Object->Behaves_Like(BEHAVIOR_PLANET))
	{
		planet = Object;
		const DynamicVectorClass<GameObjectClass*> *fleets = Object->Get_Planetary_Data()->Get_Orbiting_Fleets();
		if (fleets->Size() > 0)
		{
			fleet = fleets->Get_At(0);
		}
	}
	else
	{
		script->Script_Error("GameObjectWrapper::Force_Test_Space_Conflict -- bad object %s.  This function only works for fleets and planets.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	if (!planet || !planet->Behaves_Like(BEHAVIOR_PLANET))
	{
		return NULL;
	}

	if (!fleet || !fleet->Behaves_Like(BEHAVIOR_FLEET))
	{
		return NULL;
	}

	FleetBehaviorClass *fleet_behave = static_cast<FleetBehaviorClass*>(fleet->Get_Behavior(BEHAVIOR_FLEET));
	FAIL_IF(!fleet_behave) { return NULL; }

	fleet_behave->Unlink_Fleet_From_Planetary_Orbit(planet);
	if (!fleet_behave->Link_Fleet_To_Planetary_Orbit(planet))
	{
		return NULL;
	}

	FleetLocomotorBehaviorClass *fleet_locomotor = static_cast<FleetLocomotorBehaviorClass*>(fleet->Get_Behavior(BEHAVIOR_LOCO));
	FAIL_IF(!fleet_locomotor) { return NULL; }
	fleet_locomotor->Add_Nearby_Fleets_To_Planet(fleet, planet);

	fleet->Get_Manager()->Initiate_Space_Conflict(planet, fleet->Get_Owner());
	
	GenericSignalBlockStatus *signal_block = static_cast<GenericSignalBlockStatus*>(GenericSignalBlockStatus::FactoryCreate());
	FAIL_IF(!signal_block) { return NULL; }

	signal_block->Init(this, planet, PG_SIGNAL_OBJECT_SPACE_CONFLICT_END);

	return Return_Variable(signal_block);
}

/**************************************************************************************************
* GameObjectWrapper::Hide -- This hides or unhides the object from view
*	
*
* In:				
*
* Out:	
*
* History: 8/30/2005 12:32 PM -- JAR
**************************************************************************************************/
LuaTable *GameObjectWrapper::Hide(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (!params || params->Value.size() == 0)
	{
		script->Script_Error("GameObjectWrapper::Hide -- Missing required boolean parameter.");
		return NULL;
	}

	LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);

	if (!bval)
	{
		script->Script_Error("GameObjectWrapper::Hide -- Expected bool parameter.");
		return NULL;
	}

	if (!Object->Get_Model())
	{
		script->Script_Error("GameObjectWrapper::Hide -- object %s has no model.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	Object->Get_Model()->Hide_Model(bval->Value);
	Object->Set_Attached_SFX_Silent(bval->Value);

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Face_Immediate -- Causes an object to immediately face an object or position
*	
*
* In:				
*
* Out:	
*
* History: 8/30/2005 12:32 PM -- JAR
**************************************************************************************************/
LuaTable *GameObjectWrapper::Face_Immediate(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (!params || params->Value.size() == 0)
	{
		script->Script_Error("GameObjectWrapper::Face_Immediate -- Missing required position or gameobject parameter.");
		return NULL;
	}

	Vector3 target_position;

	if(!Lua_Extract_Position(params->Value[0], target_position))
	{
		script->Script_Error("GameObjectWrapper::Face_Immediate -- could not extract a position from parameter 1.");
		return NULL;
	}

	Object->Face_Position_Instantly(target_position);
	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Reset_Ability_Counter -- Causes an objects abilitiy counters to be reset
*	
*
* In:				
*
* Out:	
*
* History: 8/31/2005 16:00 PM -- JAR
**************************************************************************************************/
LuaTable *GameObjectWrapper::Reset_Ability_Counter(LuaScriptClass *script, LuaTable * /*params*/)
{
	FAIL_IF(!Object) { return NULL; }

	if( Object->Behaves_Like( BEHAVIOR_ABILITY_COUNTDOWN ) )
	{										
		AbilityCountdownBehaviorClass *countdown = static_cast<AbilityCountdownBehaviorClass *> (Object->Get_Behavior( BEHAVIOR_ABILITY_COUNTDOWN ));
		
		if(NULL == countdown)
		{
			script->Script_Error("GameObjectWrapper::Reset_Ability_Counter -- could not obtain a pointer to AbilityCountdownBehaviorClass");
			return NULL;
		}
		for(int ability = ABILITY_NONE + 1; ability < ABILITY_TOTAL; ability++)
		{
			if(countdown->Is_Ability_Recharged((UnitAbilityType)ability) == false)
			{
				countdown->Set_Countdown_Timer_Frames((UnitAbilityType)ability, 1);
			}
		}	
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Prevent_All_Fire -- Prevent an object from attacking at all
*	
*
* In:				
*
* Out:	
*
* History: 9/01/2005 3:55 PM -- JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Prevent_All_Fire(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Prevent_All_Fire -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaBool> disable = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!disable)
	{
		script->Script_Error("GameObjectWrapper::Prevent_All_Fire -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return NULL; }
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }
			TargetingInterfaceClass *targeting_behavior = static_cast<TargetingInterfaceClass*>(team_member->Get_Behavior(BEHAVIOR_TARGETING));
			WeaponBehaviorClass *weapon_behavior = static_cast<WeaponBehaviorClass*>(team_member->Get_Behavior(BEHAVIOR_WEAPON));
			if (disable->Value)
			{
				team_member->Disable_Weapon_Hard_Points();
				if (targeting_behavior)
				{
					targeting_behavior->Disable_Service();
				}
				if (weapon_behavior)
				{
					weapon_behavior->Disable_Service();
				}
			}
			else
			{
				team_member->Enable_Weapon_Hard_Points();
				if (targeting_behavior)
				{
					targeting_behavior->Enable_Service();
				}
				if (weapon_behavior)
				{
					weapon_behavior->Enable_Service();
				}
			}
		}		
	}

	TargetingInterfaceClass *targeting_behavior = static_cast<TargetingInterfaceClass*>(Object->Get_Behavior(BEHAVIOR_TARGETING));
	WeaponBehaviorClass *weapon_behavior = static_cast<WeaponBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_WEAPON));
	if (disable->Value)
	{
		Object->Disable_Weapon_Hard_Points();
		if (targeting_behavior)
		{
			targeting_behavior->Disable_Service();
		}
		if (weapon_behavior)
		{
			weapon_behavior->Disable_Service();
		}
	}
	else
	{
		Object->Enable_Weapon_Hard_Points();
		if (targeting_behavior)
		{
			targeting_behavior->Enable_Service();
		}
		if (weapon_behavior)
		{
			weapon_behavior->Enable_Service();
		}
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Disable_Capture -- Prevent an object from running its capture point behavior.  Not
*	valid on objects that are not capture points
*	
*
* In:				
*
* Out:	
*
* History: 9/05/2005 10:46 AM -- JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Disable_Capture(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Disable_Capture -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaBool> disable = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!disable)
	{
		script->Script_Error("GameObjectWrapper::Disable_Capture -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	CapturePointBehaviorClass *capture_behave = static_cast<CapturePointBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_CAPTURE_POINT));
	if (!capture_behave)
	{
		script->Script_Error("GameObjectWrapper::Disable_Capture -- %s isn't captureable to begin with - no capture point behavior.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	if (disable->Value)
	{
		capture_behave->Disable_Service();
	}
	else
	{
		capture_behave->Enable_Service();
	}

	return NULL;
}


/**************************************************************************************************
* GameObjectWrapper::Suspend_Locomotor -- Suspends an objects locomotor
*
* In:				
*
* Out:	
*
* History: 8/11/2005 6:07PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Suspend_Locomotor(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	// Make sure the object has a locomotor
	if (!Object->Behaves_Like(BEHAVIOR_LOCO))
	{
		// script->Script_Error("GameObjectWrapper::Suspend_Locomotor -- object %s can't move, so it can't have a suspended locomotor.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	// Make sure we can obtain a locomotor
	LocomotorInterfaceClass *locomotor = static_cast<LocomotorInterfaceClass*>(Object->Get_Behavior(BEHAVIOR_LOCO));

	if(NULL == locomotor)
	{
		script->Script_Error("GameObjectWrapper::Suspend_Locomotor -- Can't obtain locomotor for object %s", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	// Make sure we have the correct number of parameters
	if(params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Suspend_Locomotor -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	bool suspend = false;

	LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);

	// Make sure we were passed a bool
	if (!bval)
	{
		script->Script_Error("GameObjectWrapper::Suspend_Locomotor -- invalid type for parameter 1.  Expected bool.");
		return NULL;
	}
	
	suspend = bval->Value;

	if(suspend)
		locomotor->NMI_Suspend_Locomotor();
	else
		locomotor->NMI_Resume_Locomotor();
		
	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Explore_Area -- Wander around a target area until all of it ahs been recently viewed
*
* In:			
*
* Out:		
*
* History: 9/22/2005 1:54PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Explore_Area(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Explore_Area -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<AITargetLocationWrapper> ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);
	if (!ai_target_wrapper)
	{
		script->Script_Error("GameObjectWrapper::Explore_Area -- invalid type for parameter 1.  Expected ai target location.");
		return NULL;
	}

	if (!ai_target_wrapper->Get_Object())
	{
		script->Script_Error("GameObjectWrapper::Explore_Area -- target is dead.");
		return NULL;
	}

	ExploreAreaBlockStatus *bs = static_cast<ExploreAreaBlockStatus*>(ExploreAreaBlockStatus::FactoryCreate());
	FAIL_IF(!bs) { return NULL; }
	bs->Init(this, ai_target_wrapper->Get_Object());
	return Return_Variable(bs);
}

/**************************************************************************************************
* GameObjectWrapper::Highlight -- toggle on or off the highlight arrow on an object.
*
* In:			
*
* Out:		
*
* History: 9/22/2005 6:40PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Highlight(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() <=0 || params->Value.size() > 2)
	{
		script->Script_Error("GameObjectWrapper::Highlight -- invalid number of parameters.  Expected 1 or 2.");
		return NULL;
	}

	SmartPtr<LuaBool> on_off = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!on_off)
	{
		script->Script_Error("GameObjectWrapper::Highlight -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	float z_offset = 0.0f;
	if (params->Value.size() == 2)
	{
		SmartPtr<LuaNumber> offset_value = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
		if (!offset_value)
		{
			script->Script_Error("GameObjectWrapper::Highlight -- invalid type for parameter 2.  Expected number.");
			return NULL;
		}
		z_offset = offset_value->Value;
	}

	if (on_off->Value)
	{
		Object->Highlight(Vector3(0.0f, 0.0f, z_offset));
	}
	else
	{
		Object->Un_Highlight();
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Highlight -- toggle on or off the small highlight arrow on an object.
*
* In:			
*
* Out:		
*
* History: 11/15/2005 11:57:24 AM -- BMH
**************************************************************************************************/
LuaTable *GameObjectWrapper::Highlight_Small(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() <=0 || params->Value.size() > 2)
	{
		script->Script_Error("GameObjectWrapper::Highlight_Small -- invalid number of parameters.  Expected 1 or 2.");
		return NULL;
	}

	SmartPtr<LuaBool> on_off = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!on_off)
	{
		script->Script_Error("GameObjectWrapper::Highlight_Small -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	float z_offset = 0.0f;
	if (params->Value.size() == 2)
	{
		SmartPtr<LuaNumber> offset_value = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
		if (!offset_value)
		{
			script->Script_Error("GameObjectWrapper::Highlight_Small -- invalid type for parameter 2.  Expected number.");
			return NULL;
		}
		z_offset = offset_value->Value;
	}

	if (on_off->Value)
	{
		Object->Highlight(Vector3(0.0f, 0.0f, z_offset), true);
	}
	else
	{
		Object->Un_Highlight();
	}

	return NULL;
}


/**************************************************************************************************
* GameObjectWrapper::Show_Emitter -- toggle on or off emitters of a given type
*
* In:			
*
* Out:		
*
* History: 9/26/2005 11:16AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Show_Emitter(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Show_Emitter -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> emitter_type_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!emitter_type_name)
	{
		script->Script_Error("GameObjectWrapper::Show_Emitter -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	ModelClass::EmitterType emitter_type;
	if (!TheModelEmitterTypeConverterPtr->String_To_Enum(emitter_type_name->Value, emitter_type))
	{
		script->Script_Error("GameObjectWrapper::Show_Emitter -- unknown emitter type %s.", emitter_type_name->Value.c_str());
		return NULL;
	}

	SmartPtr<LuaBool> on_off = PG_Dynamic_Cast<LuaBool>(params->Value[1]);
	if (!on_off)
	{
		script->Script_Error("GameObjectWrapper::Show_Emitter -- invalid type for parameter 2.  Expected boolean.");
		return NULL;
	}

	if (Object->Get_Model())
	{
		Object->Get_Model()->Set_Emitter_Visibility(emitter_type, on_off->Value);
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Has_Attack_Target -- Script function to check whether a unit has something it's attacking
*
* In:			
*
* Out:		
*
* History: 9/30/2005 6:26PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Has_Attack_Target(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	return Return_Variable(new LuaBool(Object->Get_Target_Attack_Object(0) != NULL));
}

/**************************************************************************************************
* GameObjectWrapper::Has_Attack_Target -- Script function to check whether a unit has something it's attacking
*
* In:			
*
* Out:		
*
* History: 9/30/2005 6:26PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Stop(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	static std::vector< GameObjectClass * > objects;
	objects.resize(0);
	objects.push_back(Object);

	GameModeClass *active_mode = GameModeManager.Get_Active_Mode();
	if (!active_mode || !active_mode->Get_Movement_Coordinator_System())
	{
		script->Script_Error("GameObjectWrapper::Stop -- This command only works in tactical modes.");
		return NULL;
	}
	
	active_mode->Get_Movement_Coordinator_System()->Halt_Movement( objects );

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Override_Max_Speed -- Override this units max speed.
*
* In:			
*
* Out:		
*
* History: 10/17/2005 11:39:39 AM -- BMH
**************************************************************************************************/
LuaTable *GameObjectWrapper::Override_Max_Speed(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() == 0)
	{
		script->Script_Error("GameObjectWrapper::Override_Max_Speed -- Expected a number or boolean parameter.");
		return NULL;
	}
	LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	LuaNumber::Pointer nval = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (bval && bval->Value == false)
	{
		Object->Clear_Max_Movement_Speed_Override();
	}
	if (nval)
	{
		Object->Set_Max_Movement_Speed_Override(nval->Value);
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Attach_Particle_Effect -- Create a particle effect attached to this object.  Optional second
*	parameter allows for attachment to a particular bone.
*
* In:			
*
* Out:		
*
* History: 10/31/2005 10:21AM -- JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Attach_Particle_Effect(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1 && params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Attach_Particle_Effect -- Invalid number of parameters.  Expected 1 or 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectTypeWrapper> type_wrapper = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
	SmartPtr<LuaString> type_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);

	const GameObjectTypeClass *particle_type = NULL;
	if (type_wrapper)
	{
		particle_type = type_wrapper->Get_Object();
	}
	else if (type_name)
	{
		particle_type = GameObjectTypeManager.Find_Object_Type(type_name->Value);
	}
	else
	{
		script->Script_Error("GameObjectWrapper::Attach_Particle_Effect -- Invalid type for parameter 1.  Expected type or type name.");
		return NULL;
	}

	if (!particle_type)
	{
		script->Script_Error("GameObjectWrapper::Attach_Particle_Effect -- Unknown game object type supplied.");
		return NULL;
	}

	GameObjectClass *new_object = NULL;
	if (params->Value.size() == 2)
	{
		//Optional second parameter is bone name
		SmartPtr<LuaString> bone_name = PG_Dynamic_Cast<LuaString>(params->Value[1]);
		if (!bone_name)
		{
			script->Script_Error("GameObjectWrapper::Attach_Particle_Effect -- Invalid type for parameter 2. Expected string.");
			return NULL;
		}

		new_object = Object->Attach_Particle_Effect(particle_type, bone_name->Value.c_str());
	}
	else
	{
		new_object = Object->Attach_Particle_Effect(particle_type);
	}

	if (!new_object)
	{
		script->Script_Error("GameObjectWrapper::Attach_Particle_Effect -- Failed to create %s.  Some kind of internal game logic error.  Maybe check the logfile?", particle_type->Get_Name()->c_str());
		return NULL;
	}

	return Return_Variable(GameObjectWrapper::Create(new_object, script));
}

/**************************************************************************************************
* GameObjectWrapper::Get_Planet_Location -- Get the planet at which a galactic object is located (if any)
*
* In:			
*
* Out:		
*
* History: 11/2/2005 11:01AM -- JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Planet_Location(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (Object->Get_Manager()->Get_Game_Mode()->Get_Sub_Type() != SUB_GAME_MODE_GALACTIC)
	{
		script->Script_Error("GameObjectWrapper::Get_Planet_Location -- this function is only valid on galactic mode objects.");
		return NULL;
	}

	GameObjectClass *planet = Object;
	while (planet && !planet->Behaves_Like(BEHAVIOR_PLANET))
	{
		planet = planet->Get_Parent_Container_Object();
	}

	if (planet)
	{
		return Return_Variable(Create(planet, script));
	}
	else
	{
		return NULL;
	}
}
/**************************************************************************************************
* GameObjectWrapper::In_End_Cinematic -- Marks an object as being included in an end cinematic so it is not "cleaned up"
*
* In:			
*
* Out:		
*
* History: 11/11/2005 9:48AM -- JAR
**************************************************************************************************/
LuaTable *GameObjectWrapper::In_End_Cinematic(LuaScriptClass *script, LuaTable *params)
{
	if (!params || params->Value.size() == 0)
	{
		script->Script_Error("GameObjectWrapper::Lua_Is_In_End_Cinematic -- Missing required boolean parameter.");
		return NULL;
	}

	LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);

	if (Object)
	{
		Object->Set_Is_In_End_Cinematic(bval->Value);
		return NULL;
	}

	script->Script_Error("GameObjectWrapper::In_End_Cinematic -- Object invalid");
	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Stop_SFX_Event -- Stop playing a named SFX event attached to this object
*
* In:			
*
* Out:		
*
* History: 11/15/2005 11:48AM -- JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Stop_SFX_Event(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1 && params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Stop_SFX_Event -- invalid number of parameters.  Expected 1 or 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> sfx_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!sfx_name)
	{
		script->Script_Error("GameObjectWrapper::Stop_SFX_Event -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	float fade_time = 0.0f;
	if (params->Value.size() == 2)
	{
		SmartPtr<LuaNumber> lua_fade_time = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
		if (!lua_fade_time)
		{
			script->Script_Error("GameObjectWrapper::Stop_SFX_Event -- invalid type for parameter 2.  Expected number.");
			return NULL;
		}
		fade_time = lua_fade_time->Value;
	}

	const SFXEventClass *sfx_event = TheSFXEventManager.Find_SFX_Event(sfx_name->Value);
	if (!sfx_event)
	{
		script->Script_Error("GameObjectWrapper::Stop_SFX_Event -- unknown SFX event %s.", sfx_name->Value.c_str());
		return NULL;
	}

	if (!TheSFXEventManager.Stop_SFX_Event(sfx_event, Object, fade_time))
	{
		script->Script_Warning("GameObjectWrapper::Stop_SFX_Event -- failed to stop SFX event %s.  Maybe it wasn't playing?", sfx_name->Value.c_str());
	}

	return NULL;
}



/**************************************************************************************************
* GameObjectWrapper::Play_Cinematic_Engine_Flyby -- Sets the engine of an object on/off for cinematics
*
* In:			
*
* Out:		
*
* History: 11/11/2005 9:48AM -- JAR
**************************************************************************************************/
LuaTable *GameObjectWrapper::Play_Cinematic_Engine_Flyby(LuaScriptClass *script, LuaTable* /*params*/)
{
	if (Object)
	{
		const SFXEventClass *psfx = Object->Get_Type()->Get_SFXEvent_Ambient_Moving();  

		if(NULL != psfx)
		{
			TheSFXEventManager.Start_SFX_Event(psfx, Object);
		}
		return NULL;
	}

	script->Script_Error("GameObjectWrapper::Play_Cinematic_Engine_Flyby -- Object invalid");
	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Get_Is_Planet_AI_Usable -- Check whether this planet is legal for AI use
*
*
* In:		
*
* Out:	
*
*
* History: 11/17/2005 5:17 PM -- JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Is_Planet_AI_Usable(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (!Object->Behaves_Like(BEHAVIOR_PLANET))
	{
		script->Script_Error("GameObjectWrapper::Get_Is_Planet_AI_Usable -- object %s is not a planet.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	PlanetaryBehaviorClass *planet = static_cast<PlanetaryBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_PLANET));
	FAIL_IF(!planet) { return NULL; }

	return Return_Variable(new LuaBool(planet->Is_Usable_By_AI(Object)));
}

/**************************************************************************************************
* GameObjectWrapper::Enable_Behavior -- Enables or disables the servicing of a specific behavior
*
* In:			
*
* Out:		
*
* History: 11/11/2005 9:48AM -- JAR
**************************************************************************************************/
LuaTable *GameObjectWrapper::Enable_Behavior(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Enable_Behavior -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaNumber> lua_number = PG_Dynamic_Cast<LuaNumber>(params->Value[0]); 

	if(!lua_number)
	{
		script->Script_Error("GameObjectWrapper::Enable_Behavior -- invalid type for behavior.  Expected number.");
		return NULL;
	}
	
	int value = static_cast<int>(lua_number->Value);
	BehaviorType behavior_type = static_cast<BehaviorType>(value);

	if(behavior_type <= BEHAVIOR_NONE || behavior_type >= BEHAVIOR_COUNT)
	{
		script->Script_Error("GameObjectWrapper::Enable_Behavior -- invalid behavior");
		return NULL;
	}

	LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[1]);

	if(!bval)
	{
		script->Script_Error("GameObjectWrapper::Enable_Behavior -- invalid type for state.  Expected a bool.");
		return NULL;
	}

	bool state = bval->Value;

	Object->Enable_Behavior(behavior_type, state);

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Cancel_Ability -- Turn off a named ability (tactical only)
*
*
* In:		
*
* Out:	
*
*
* History: 11/22/2005 10:20 AM -- JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Cancel_Ability(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Cancel_Ability -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return false;
	}

	SmartPtr<LuaString> ability_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!ability_name)
	{
		script->Script_Error("GameObjectWrapper::Cancel_Ability -- invalid type for ability 1.  Expected string.");
		return false;
	}

	UnitAbilityType ability_type = ABILITY_NONE;
	if (!TheUnitAbilityTypeConverterPtr->String_To_Enum(ability_name->Value, ability_type))
	{
		script->Script_Error("GameObjectWrapper::Cancel_Ability -- unknown ability %s.", ability_name->Value.c_str());
		return false;
	}

	//Make sure we have the ability in question
	if(!Object->Has_Unit_Special_Ability(ability_type))
	{
		script->Script_Warning("GameObjectWrapper::Cancel_Ability -- Object %s does not have ability %s.", Object->Get_Type()->Get_Name()->c_str(), ability_name->Value.c_str());
		return false;
	}

	static DynamicVectorClass<GameObjectClass*> single_object_list;
	single_object_list.Truncate();
	single_object_list.Add(Object);

	SetUnitAbilityModeEventClass event(false);
	event.Init(&single_object_list, ability_type, Object->Get_Owner());
	event.Execute();

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Can_Land_On_Planet -- Script function to check the rules for whether or not
*	a transport (this) can land on a given planet.
*
*
* In:		
*
* Out:	
*
*
* History: 12/12/2005 9:17 PM -- JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Can_Land_On_Planet(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Can_Land_On_Planet -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectWrapper> planet_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	if (!planet_wrapper)
	{
		script->Script_Error("GameObjectWrapper::Can_Land_On_Planet -- invalid type for parameter 1.  Expected game object.");
		return NULL;
	}

	GameObjectClass *planet_object = planet_wrapper->Get_Object();

	if (!planet_object || !planet_object->Behaves_Like(BEHAVIOR_PLANET))
	{
		return Return_Variable(new LuaBool(false));
	}

	if (!Object->Behaves_Like(BEHAVIOR_TRANSPORT))
	{
		return Return_Variable(new LuaBool(false));
	}

	if (!planet_object->Get_Type()->Is_Planet_Surface_Accessible(planet_object))
	{
		return Return_Variable(new LuaBool(false));
	}

	if (planet_object->Get_Type()->Get_Planet_Object_Type_Is_Restricted(Object->Get_Original_Object_Type()))
	{
		return Return_Variable(new LuaBool(false));
	}

	PlanetaryBehaviorClass *planet_behavior = static_cast<PlanetaryBehaviorClass*>(planet_object->Get_Behavior(BEHAVIOR_PLANET));
	FAIL_IF(!planet_behavior) { return NULL; }

	return Return_Variable(new LuaBool(planet_behavior->Get_Total_Landed_Transports(planet_object) < TheGameConstants.Get_Max_Ground_Forces_On_Planet()));
}

/**************************************************************************************************
* GameObjectWrapper::Set_Check_Contested_Space -- 
*
* In:				
*
* Out:	
*
* History: 12/14/2005 4:09PM MLL
**************************************************************************************************/
LuaTable *GameObjectWrapper::Set_Check_Contested_Space(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Set_Check_Contested_Space -- invalid number of parameters.  Expected 1, got %d.");
		return NULL;
	}

	SmartPtr<LuaBool> on_off = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!on_off)
	{
		script->Script_Error("GameObjectWrapper::Set_Check_Contested_Space -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	Object->Set_Check_Contested_Space(on_off->Value);
	return NULL;
}



/**************************************************************************************************
* GameObjectWrapper::Get_Attack_Target -- Returns the object that this object is attacking
*
* In:		
*
* Out:	
*
*
* History: 05/10/2006 4:28PM JAC
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Attack_Target(LuaScriptClass *script, LuaTable *)
{
	if (!Object)
	{
		return 0;
	}

	GameObjectClass *target = Object->Get_Target_Attack_Object(NULL);

	if (target == NULL)
	{
		return 0;
	}

	return Return_Variable(GameObjectWrapper::Create(target, script));
}

/**************************************************************************************************
* GameObjectWrapper::Garrison -- Order this unit to go garrison somebody else
*
* In:		
*
* Out:	
*
*
* History: 05/24/2006 10:04AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Garrison(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (!Object->Behaves_Like(BEHAVIOR_GARRISON_UNIT))
	{
		script->Script_Warning("GameObjectWrapper::Garrison -- unit %s cannot be placed in a garrison: it has no GARRISON_UNIT behavior.", Object->Get_Original_Object_Type()->Get_Name()->c_str());
		return NULL;
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Garrison -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectWrapper> garrison_object = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	SmartPtr<AITargetLocationWrapper> garrison_ai_target = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);
	GameObjectClass *garrison = NULL;
	if (garrison_object)
	{
		garrison = garrison_object->Get_Object();
	}
	else if (garrison_ai_target && garrison_ai_target->Get_Object())
	{
		garrison = garrison_ai_target->Get_Object()->Get_Target_Game_Object();
	}
	else
	{
		script->Script_Error("GameObjectWrapper::Garrison -- invalid type for parameter 1.  Expected game object or AI target.");
	}

	if (!garrison)
	{
		script->Script_Error("GameObjectWrapper;:Garrison - target is already dead.");
		return NULL;
	}

	if (!garrison->Is_Ally(Object->Get_Owner()))
	{
		script->Script_Error("GameObjectWrapper;:Garrison - target is hostile.");
		return NULL;
	}

	if (!garrison->Behaves_Like(BEHAVIOR_GARRISON_STRUCTURE) && !garrison->Behaves_Like(BEHAVIOR_GARRISON_VEHICLE) && !garrison->Behaves_Like(BEHAVIOR_GARRISON_HOVER_TRANSPORT))
	{
		script->Script_Error("GameObjectWrapper::Garrison -- unit %s cannot have garrisons placed in it: it has no GARRISON_VEHICLE or GARRISON_STRUCTURE or GARRISON_HOVER_TRANSPORT behavior.", garrison->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	if (!garrison->Compatible_Garrison(Object))
	{
		script->Script_Warning("GameObjectWrapper::Garrison - unit %s cannot garrison unit %s: garrison masks do not match.", Object->Get_Original_Object_Type()->Get_Name()->c_str(), garrison->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	MoveToGarrisonEventClass event;
	event.Set_Player_ID(Object->Get_Owner());
	event.Init(garrison, Object, UNIT_AI_MOVE_TYPE_INVALID);
	event.Execute();

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Can_Garrison -- 
*
* In:		
*
* Out:	
*
*
* History: 05/24/2006 10:35AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Can_Garrison(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (!Object->Behaves_Like(BEHAVIOR_GARRISON_UNIT))
	{
		return Return_Variable(new LuaBool(false));
	}

	if (Object->Is_In_Limbo())
	{
		return Return_Variable(new LuaBool(false));
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Can_Garrison -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectWrapper> garrison_object = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	SmartPtr<AITargetLocationWrapper> garrison_ai_target = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);
	GameObjectClass *garrison = NULL;
	if (garrison_object)
	{
		garrison = garrison_object->Get_Object();
	}
	else if (garrison_ai_target && garrison_ai_target->Get_Object())
	{
		garrison = garrison_ai_target->Get_Object()->Get_Target_Game_Object();
	}
	else
	{
		script->Script_Error("GameObjectWrapper::Can_Garrison -- invalid type for parameter 1.  Expected game object or AI target.");
	}

	if (!garrison)
	{
		script->Script_Error("GameObjectWrapper;:Can_Garrison - target is already dead.");
		return NULL;
	}

	if (!garrison->Behaves_Like(BEHAVIOR_GARRISON_STRUCTURE) && !garrison->Behaves_Like(BEHAVIOR_GARRISON_VEHICLE) && !garrison->Behaves_Like(BEHAVIOR_GARRISON_HOVER_TRANSPORT))
	{
		return Return_Variable(new LuaBool(false));
	}

	PlayerClass *garrison_owner = garrison->Get_Owner_Player();
	if (garrison_owner && !garrison_owner->Is_Ally(Object->Get_Owner()) && !garrison_owner->Get_Faction()->Is_Neutral())
	{
		return Return_Variable(new LuaBool(false));
	}

	if (!garrison->Compatible_Garrison(Object))
	{
		return Return_Variable(new LuaBool(false));
	}

	if (garrison->Garrison_Slots_Available() - 
			garrison->Get_Garrison_Data()->Get_Garrison_Pending_Count() - 
			Object->Get_Original_Object_Type()->Get_Garrison_Value() <= 0)
	{
		return Return_Variable(new LuaBool(false));
	}

	return Return_Variable(new LuaBool(true));
}

/**************************************************************************************************
* GameObjectWrapper::Can_Garrison_Fire -- 
*
* In:		
*
* Out:	
*
*
* History: 05/24/2006 3:04PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Can_Garrison_Fire(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	return Return_Variable(new LuaBool(Object->Get_Type()->Get_Garrison_Bone_Indices()->Size() > 0));
}

/**************************************************************************************************
* GameObjectWrapper::Leave_Garrison -- Order this unit to leave any vehicle or structure it may
* be garrisoning
*
* In:		
*
* Out:	
*
*
* History: 05/25/2006 9:43AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Leave_Garrison(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	GarrisonUnitBehaviorClass *garrison_behavior = static_cast<GarrisonUnitBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_GARRISON_UNIT));
	if (!garrison_behavior)
	{
		return NULL;
	}

	if (!garrison_behavior->Un_Garrison())
	{
		//Just in case
		garrison_behavior->Clear_Garrison_Command();
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Eject_Garrison -- Order all units garrisoned inside this object to leave
*
* In:		
*
* Out:	
*
*
* History: 05/25/2006 9:43AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Eject_Garrison(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (Object->Behaves_Like(BEHAVIOR_GARRISON_VEHICLE))
	{
		GarrisonVehicleBehaviorClass *garrison_behavior = static_cast<GarrisonVehicleBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_GARRISON_VEHICLE));
		if (garrison_behavior)
		{
			garrison_behavior->Remove_All_Contained_Units(Object);
		}
	}
	else if(Object->Behaves_Like(BEHAVIOR_GARRISON_STRUCTURE))
	{
		GarrisonStructureBehaviorClass *garrison_behavior = static_cast<GarrisonStructureBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_GARRISON_STRUCTURE));
		if (garrison_behavior)
		{
			garrison_behavior->Remove_All_Contained_Units(Object);
		}
	}
	else if(Object->Behaves_Like(BEHAVIOR_GARRISON_HOVER_TRANSPORT))
	{
		GarrisonHoverTransportBehaviorClass *garrison_behavior = static_cast<GarrisonHoverTransportBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_GARRISON_HOVER_TRANSPORT));
		if (garrison_behavior)
		{
			garrison_behavior->Remove_All_Contained_Units(Object);
		}
	}

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Has_Garrison -- Does this object have units garrisoned inside it
*
* In:		
*
* Out:	
*
*
* History: 05/25/2006 10:49AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Has_Garrison(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (!Object->Behaves_Like(BEHAVIOR_GARRISON_VEHICLE) && !Object->Behaves_Like(BEHAVIOR_GARRISON_STRUCTURE) && !Object->Behaves_Like(BEHAVIOR_GARRISON_HOVER_TRANSPORT))
	{
		return Return_Variable(new LuaBool(false));
	}

	if (!Object->Get_Flagship_Data())
	{
		return Return_Variable(new LuaBool(false));
	}

	return Return_Variable(new LuaBool(Object->Get_Flagship_Data()->Get_Contained_Units()->Size() > 0));
}

/**************************************************************************************************
* GameObjectWrapper::Get_Garrisoned_Units -- Get a table of objects that are garrisoned inside this one
*
* In:		
*
* Out:	
*
*
* History: 05/26/2006 11:51AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Garrisoned_Units(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	LuaTable *garrison_table = Alloc_Lua_Table();

	if (!Object->Behaves_Like(BEHAVIOR_GARRISON_VEHICLE) && !Object->Behaves_Like(BEHAVIOR_GARRISON_STRUCTURE) && !Object->Behaves_Like(BEHAVIOR_GARRISON_HOVER_TRANSPORT))
	{
		return Return_Variable(garrison_table);
	}

	if (!Object->Get_Flagship_Data())
	{
		return Return_Variable(garrison_table);
	}

	const DynamicVectorClass<GameObjectClass*> *contained_units = Object->Get_Flagship_Data()->Get_Contained_Units();
	for (int i = 0; i < contained_units->Size(); ++i)
	{
		GameObjectClass *unit = contained_units->Get_At(i);
		FAIL_IF(!unit) { continue; }
		if (unit->Behaves_Like(BEHAVIOR_GARRISON_UNIT))
		{
			garrison_table->Value.push_back(Create(unit, script));
		}
	}

	return Return_Variable(garrison_table);
}

/**************************************************************************************************
* GameObjectWrapper::Is_Good_Against -- 
*
* In:		
*
* Out:	
*
*
* History: 06/01/2006 9:52AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_Good_Against(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	//Work back to a target game object
	SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	SmartPtr<AITargetLocationWrapper> target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);

	if (!object_wrapper && !target_wrapper)
	{
		script->Script_Error("Is_Good_Against -- invalid type for parameter 1.  Expected object or ai target.");
		return 0;
	}

	if ((object_wrapper && !object_wrapper->Get_Object()) || (target_wrapper && !target_wrapper->Get_Object()))
	{
		script->Script_Error("Is_Good_Against -- target is already dead.");
		return 0;
	}

	GameObjectClass *object = 0;
	if (object_wrapper)
	{
		object = object_wrapper->Get_Object();
	}
	else
	{
		object = target_wrapper->Get_Object()->Get_Target_Game_Object();
		if (!object)
		{
			script->Script_Error("Is_Good_Against -- ai target %s does not represent a game object (requried for this function).", target_wrapper->Get_Object()->Get_Name().c_str());
			return 0;
		}
	}

	const GameObjectTypeClass *enemy_type = 0;
	if (object->Behaves_Like(BEHAVIOR_TRANSPORT))
	{
		enemy_type = object->Get_Type();
	}
	else
	{
		enemy_type = object->Get_Company_Type();
	}

	const GameObjectTypeClass *my_type = 0;
	if (Object->Behaves_Like(BEHAVIOR_TRANSPORT))
	{
		my_type = Object->Get_Type();
	}
	else
	{
		my_type = Object->Get_Company_Type();
	}

	//Add up my force with appropriate RPS adjustment vs this enemy type.
	int complete_category = enemy_type->Get_Category_Mask();
	int category = GET_FIRST_BIT_SET(complete_category);

	float my_best_factor = 0.0;
	while (category > -1) 
	{
		my_best_factor = Max(my_best_factor, TargetContrastClass::Get_Average_Contrast_Factor(my_type, NULL, 1 << category));

		// turn off that bit
		complete_category &= (~(1 << category));
		category = GET_FIRST_BIT_SET(complete_category);
	}

	return Return_Variable(new LuaBool(my_best_factor > 1.0f));
}

/**************************************************************************************************
* GameObjectWrapper::Should_Switch_Weapons -- 
*
* In:		
*
* Out:	
*
*
* History: 06/02/2006 1:18PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Should_Switch_Weapons(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	GameObjectClass *firing_object = Object;
	if (Object->Behaves_Like(BEHAVIOR_TEAM))
	{
		firing_object = Object->Get_Team_Data()->Get_Team_Leader();
		if (!firing_object)
		{
			return Return_Variable(new LuaBool(false));
		}
	}

	if (firing_object->Get_Type()->Get_Num_Projectile_Types() < 2)
	{
		return Return_Variable(new LuaBool(false));
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Should_Switch_Weapons -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	//Work back to a target game object
	SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	SmartPtr<AITargetLocationWrapper> target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);

	if (!object_wrapper && !target_wrapper)
	{
		script->Script_Error("GameObjectWrapper::Should_Switch_Weapons -- invalid type for parameter 1.  Expected object or ai target.");
		return 0;
	}

	if ((object_wrapper && !object_wrapper->Get_Object()) || (target_wrapper && !target_wrapper->Get_Object()))
	{
		script->Script_Error("GameObjectWrapper::Should_Switch_Weapons -- target is already dead.");
		return 0;
	}

	GameObjectClass *object = NULL;
	if (object_wrapper)
	{
		object = object_wrapper->Get_Object();
	}
	else
	{
		object = target_wrapper->Get_Object()->Get_Target_Game_Object();
		if (!object)
		{
			script->Script_Error("GameObjectWrapper::Should_Switch_Weapons -- ai target %s does not represent a game object (requried for this function).", target_wrapper->Get_Object()->Get_Name().c_str());
			return 0;
		}
	}

	if (object->Behaves_Like(BEHAVIOR_TEAM))
	{
		object = object->Get_Team_Data()->Get_Team_Leader();
		if (!object)
		{
			return Return_Variable(new LuaBool(false));
		}
	}

	CRCValue target_armor_crc = 0;
	bool target_has_shield = false;
	if (object->Get_Shield_Percent() > 0.0f)
	{
		target_armor_crc = object->Get_Type()->Get_Shield_Armor_Type_Name_CRC();
		target_has_shield = true;
	}
	else
	{
		target_armor_crc = object->Get_Type()->Get_Armor_Type_Name_CRC();
	}

	const GameObjectTypeClass *current_projectile = firing_object->Get_Type()->Get_Projectile_Type(Object->Get_Current_Projectile_Index());
	FAIL_IF(!current_projectile) { return NULL; }
	float current_damage = current_projectile->Get_Projectile_Damage();
	current_damage *= TheGameConstants.Get_Damage_Type_Vs_Armor_Type(current_projectile->Get_Damage_Type_Name_CRC(), target_armor_crc);

	if (target_has_shield)
	{
		if (!current_projectile->Get_Projectile_Does_Shield_Damage())
		{
			current_damage = 0.0f;
		}
	}
	else
	{
		if (!current_projectile->Get_Projectile_Does_Hitpoint_Damage())
		{
			current_damage = 0.0f;
		}
	}

	for (int i = 0; i < firing_object->Get_Type()->Get_Num_Projectile_Types(); ++i)
	{
		if (firing_object->Get_Current_Projectile_Index() == i)
		{
			continue;
		}

		const GameObjectTypeClass *alternative_projectile = firing_object->Get_Type()->Get_Projectile_Type(i);
		FAIL_IF(!alternative_projectile) { continue; }

		if (target_has_shield)
		{
			if (!alternative_projectile->Get_Projectile_Does_Shield_Damage())
			{
				continue;
			}
		}
		else
		{
			if (!alternative_projectile->Get_Projectile_Does_Hitpoint_Damage())
			{
				continue;
			}
		}

		float alternative_damage = alternative_projectile->Get_Projectile_Damage();
		alternative_damage = Max(alternative_damage, alternative_projectile->Get_Projectile_Blast_Area_Damage());
		alternative_damage *= TheGameConstants.Get_Damage_Type_Vs_Armor_Type(alternative_projectile->Get_Damage_Type_Name_CRC(), target_armor_crc);

		if (alternative_damage > current_damage)
		{
			return Return_Variable(new LuaBool(true));
		}
	}

	return Return_Variable(new LuaBool(false));
}

/**************************************************************************************************
* GameObjectWrapper::Get_Current_Projectile_Type -- 
*
* In:		
*
* Out:	
*
*
* History: 06/08/2006 2:43PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_Current_Projectile_Type(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	GameObjectClass *firing_object = Object;
	if (firing_object->Behaves_Like(BEHAVIOR_TEAM))
	{
		firing_object = Object->Get_Team_Data()->Get_Team_Leader();
	}

	if (!firing_object)
	{
		return NULL;
	}

	int projectile_index = firing_object->Get_Current_Projectile_Index();
	if (projectile_index < 0 || projectile_index >= firing_object->Get_Type()->Get_Num_Projectile_Types())
	{
		return NULL;
	}

	GameObjectTypeClass *projectile_type = firing_object->Get_Type()->Get_Projectile_Type(projectile_index);
	return Return_Variable(GameObjectTypeWrapper::Create(projectile_type, script));
}

/**************************************************************************************************
* GameObjectWrapper::Is_Selectable -- 
*
* In:		
*
* Out:	
*
*
* History: 07/07/2006 11:06AM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_Selectable(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	SelectionDataPackClass *select_data = Object->Get_Selection_Data();
	return Return_Variable(new LuaBool(select_data && select_data->Is_Selectable()));
}

/**************************************************************************************************
* GameObjectWrapper::Is_Ability_Autofire -- 
*
* In:		
*
* Out:	
*
*
* History: 07/11/2006 5:444PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_Ability_Autofire(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Is_Ability_Autofire -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> ability_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!ability_name)
	{
		script->Script_Error("GameObjectWrapper::Set_Single_Ability_Autofire -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	UnitAbilityType ability_type = ABILITY_NONE;
	if (!TheUnitAbilityTypeConverterPtr->String_To_Enum(ability_name->Value, ability_type))
	{
		script->Script_Error("GameObjectWrapper::Set_Single_Ability_Autofire -- unknown ability %s.", ability_name->Value.c_str());
		return NULL;
	}

	return Return_Variable(new LuaBool(Object->Is_Unit_Ability_Autofire(ability_type)));
}

/**************************************************************************************************
* GameObjectWrapper::Get_All_Projectile_Types -- Get a table of all projectile types that an object could
*	conceivably fire, whether from hard points or as its built-in attack
*
* In:		
*
* Out:	
*
*
* History: 07/20/2006 3:39PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Get_All_Projectile_Types(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	GameObjectClass *firing_object = Object;
	if (firing_object->Behaves_Like(BEHAVIOR_TEAM))
	{
		firing_object = Object->Get_Team_Data()->Get_Team_Leader();
	}

	if (!firing_object)
	{
		return NULL;
	}

	LuaTable *projectile_table = Alloc_Lua_Table();

	//Collect projectile types from hard points
	int hard_point_count = Object->Get_Total_Hard_Points();
	for (int i = 0; i < hard_point_count; ++i)
	{
		HardPointClass *hard_point = Object->Get_Hard_Point_By_Index(i);
		FAIL_IF(!hard_point) { continue; }
		if (hard_point->Is_Destroyed())
		{
			continue;
		}

		//Silly Create function won't accept const GameObjectTypeClass*
		GameObjectTypeClass *projectile_type = const_cast<GameObjectTypeClass*>(hard_point->Get_Data()->Get_Fire_Projectile_Type());
		if (!projectile_type)
		{
			continue;
		}

		projectile_table->Value.push_back(GameObjectTypeWrapper::Create(projectile_type, script));
	}

	//Collect projectile types inherent to the object itself
	int projectile_count = firing_object->Get_Type()->Get_Num_Projectile_Types();
	for (int i = 0; i < projectile_count; ++i)
	{
		GameObjectTypeClass *projectile_type = firing_object->Get_Type()->Get_Projectile_Type(i);
		FAIL_IF(!projectile_type) { continue; }
		projectile_table->Value.push_back(GameObjectTypeWrapper::Create(projectile_type, script));
	}
	return Return_Variable(projectile_table);
}

/**************************************************************************************************
* GameObjectWrapper::Set_In_Limbo -- Place an object in a state where it is invisible, uncontrollable
*	and ignored by other objects
*
* In:		
*
* Out:	
*
*
* History: 07/28/2006 4:05PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Set_In_Limbo(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Set_In_Limbo -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaBool> limbo = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!limbo)
	{
		script->Script_Error("GameObjectWrapper::Set_In_Limbo -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	Object->Set_In_Limbo(limbo->Value);

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Is_In_Garrison -- 
*
* In:		
*
* Out:	
*
*
* History: 07/31/2006 3:13PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Is_In_Garrison(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (Object->Is_Team_Member())
	{
		GameObjectClass *team_object = Object->Get_Parent_Container_Object();
		return Return_Variable(new LuaBool(team_object->Get_Garrison_Data() && team_object->Get_Garrison_Data()->State == GARRISON_STATE_INSIDE));
	}
	else
	{
		return Return_Variable(new LuaBool(Object->Get_Garrison_Data() && Object->Get_Garrison_Data()->State == GARRISON_STATE_INSIDE));
	}
}

/**************************************************************************************************
* GameObjectWrapper::Invade -- Deploy land forces on an enemy planet
*
* In:		
*
* Out:	
*
*
* History: 8/16/2006 1:38PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Invade(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 0)
	{
		script->Script_Error("GameObjectWrapper::Invade -- invalid number of parameters.  Expected 0, got %d.", params->Value.size());
		return NULL;
	}

	//Only fleets may perform invasions
	if (!Object->Behaves_Like(BEHAVIOR_FLEET))
	{
		script->Script_Error("GameObjectWrapper::Invade -- this function is only valid on fleets.");
		return NULL;
	}

	//Fleet must be at a planet
	GameObjectClass *planet = Object;
	while (planet && !planet->Behaves_Like(BEHAVIOR_PLANET))
	{
		planet = planet->Get_Parent_Container_Object();
	}

	if (!planet)
	{
		script->Script_Warning("GameObjectWrapper:Invade -- this fleet is not at a planet.  Cannot initiate invasion.");
		return NULL;
	}

	//Planet must belong to an enemy
	PlayerClass *planet_owner = planet->Get_Owner_Player();
	PlayerClass *fleet_owner = Object->Get_Owner_Player();
	if (!planet_owner || !fleet_owner || !planet_owner->Is_Enemy(fleet_owner))
	{
		script->Script_Warning("GameObjectWrapper:Invade -- fleet owner %s is not an enemy of planet %s owner %s.  Cannot initiate invasion", 
										fleet_owner->Get_Faction()->Get_Name()->c_str(),
										planet->Get_Type()->Get_Name()->c_str(),
										planet_owner->Get_Faction()->Get_Name()->c_str());
		return NULL;
	}

	//Fleet must contain at least one transport
	DynamicVectorClass<GameObjectClass*> invasion_list;
	DynamicVectorClass<GameObjectClass*> fleet_contents;
	Object->Get_Fleet_Breakdown(fleet_contents, false);
	for (int i = 0; i < fleet_contents.Size(); ++i)
	{
		GameObjectClass *ship = fleet_contents[i];
		FAIL_IF(!ship) { continue; }
		if (ship->Behaves_Like(BEHAVIOR_TRANSPORT))
		{
			invasion_list.Add(ship);
		}
	}

	if (invasion_list.Size() == 0)
	{
		script->Script_Warning("GameObjectWrapper:Invade -- fleet contains no land units.  Cannot initiate invasion");
		return NULL;
	}

	InvadeEventClass invade_event;
	invade_event.Init(&invasion_list, planet);
	invade_event.Execute();

	return NULL;
}


/**************************************************************************************************
* GameObjectWrapper::Can_Move -- Check whether this object will respond to a movement command
*
* In:		
*
* Out:	
*
*
* History: 8/16/2006 1:38PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Can_Move(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (GameModeManager.Get_Sub_Type() == SUB_GAME_MODE_GALACTIC)
	{
		//In galactic fleets that are currently stationed at planets are the only things that can be moved
		if (!Object->Behaves_Like(BEHAVIOR_FLEET))
		{
			return Return_Variable(new LuaBool(false));
		}

		GameObjectClass *planet = Object->Get_Parent_Container_Object();
		if (!planet || !planet->Behaves_Like(BEHAVIOR_PLANET))
		{
			return Return_Variable(new LuaBool(false));
		}

		return Return_Variable(new LuaBool(true));
	}
	else
	{
		//In tactical modes we'll query the movement system
		MovementCoordinatorSystemClass *move_system = GameModeManager.Get_Active_Mode()->Get_Movement_Coordinator_System();
		if (!move_system)
		{
			return Return_Variable(new LuaBool(false));
		}

		if (!move_system->Can_Object_Be_In_Formations(Object))
		{
			return Return_Variable(new LuaBool(false));
		}

		if (Object->Is_Movement_Locked())
		{
			return Return_Variable(new LuaBool(false));
		}

		return Return_Variable(new LuaBool(true));
	}
}


/**************************************************************************************************
* GameObjectWrapper::Enable_Dynamic_LOD -- Enable or disable dynamic LOD code.
*
* In:		
*
* Out:	
*
*
* History: 8/16/2006 1:38PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Enable_Dynamic_LOD(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectWrapper::Enable_Dynamic_LOD -- command takes a single boolean value.");
		return NULL;
	}

	SmartPtr<LuaBool> dynamic_lod_enabled = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!dynamic_lod_enabled)
	{
		script->Script_Error("GameObjectWrapper::Enable_Dynamic_LOD -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	if (Object->Get_Model())
		Object->Get_Model()->Enable_Dynamic_LOD(dynamic_lod_enabled->Value);

	return NULL;
}

/**************************************************************************************************
* GameObjectWrapper::Force_Ability_Recharge -- 
*
* In:		
*
* Out:	
*
*
* History: 8/29/2006 2:24PM JSY
**************************************************************************************************/
LuaTable *GameObjectWrapper::Force_Ability_Recharge(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1 && params->Value.size() != 2)
	{
		script->Script_Error("GameObjectWrapper::Force_Ability_Recharge -- invalid number of parameters.  Expected 1 or 2, got %d.", params->Value.size());
		return NULL;
	}

	if (!Object->Behaves_Like(BEHAVIOR_ABILITY_COUNTDOWN))
	{
		script->Script_Error("GameObjectWrapper::Force_Ability_Recharge -- %s has no ability timer; cannot force it to recharging state.", Object->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	AbilityCountdownBehaviorClass *countdown_behavior = static_cast<AbilityCountdownBehaviorClass*>(Object->Get_Behavior(BEHAVIOR_ABILITY_COUNTDOWN));
	FAIL_IF(!countdown_behavior) { return NULL; }

	SmartPtr<LuaString> ability_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!ability_name)
	{
		script->Script_Error("GameObjectWrapper::Force_Ability_Recharge -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	UnitAbilityType ability_type = ABILITY_NONE;
	if (!TheUnitAbilityTypeConverterPtr->String_To_Enum(ability_name->Value, ability_type))
	{
		script->Script_Error("GameObjectWrapper::Force_Ability_Recharge -- unknown ability %s.", ability_name->Value.c_str());
		return NULL;
	}

	//This will start the countdown with the default recharge time
	countdown_behavior->Start_Ability_Countdown_Timer(ability_type);

	//Support an optional user-specified recharge time (which must be set after the timer has been started)
	if (params->Value.size() == 2)
	{
		SmartPtr<LuaNumber> recharge_time = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
		if (!recharge_time)
		{
			script->Script_Error("GameObjectWrapper::Force_Ability_Recharge -- invalid type for parameter 2.  Expected number.");
			return NULL;
		}

		if (recharge_time->Value < 0.0f)
		{
			script->Script_Error("GameObjectWrapper::Force_Ability_Recharge -- invalid value for parameter 2.  Expected non-negative number");
			return NULL;
		}

		countdown_behavior->Set_Countdown_Timer_Seconds(ability_type, recharge_time->Value);
	}

	return NULL;
}

LuaTable *GameObjectWrapper::Is_Corrupted(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	return Return_Variable(new LuaBool(Object->Planet_Is_Corrupted()));
}
