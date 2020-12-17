// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/PlayerWrapper.cpp#5 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/PlayerWrapper.cpp $
//
//    Original Author: Brian Hayes
//
//            $Author: Brian_Hayes $
//
//            $Change: 747267 $
//
//          $DateTime: 2020/10/27 14:46:23 $
//
//          $Revision: #5 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */

#pragma hdrstop
#include "PlayerWrapper.h"
#include "PlayerList.h"
#include "Faction.h"
#include "AI/AIPlayer.h"
#include "AI/TacticalAIManager.h"
#include "AI/Goal/AIGoalSystem.h"
#include "AI/Goal/AIBudget.h"
#include "DiscreteDistribution.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/GameObjectTypeWrapper.h"
#include "GameObjectManager.h"
#include "GameObjectTypeManager.h"
#include "BattlefieldModifiers.h"
#include "EnumConversion.h"
#include "PlanetaryBombardManager.h"

enum {
	PLAYERID_MICRO_CHUNK,
};

PG_IMPLEMENT_RTTI(PlayerWrapper, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_PLAYER_WRAPPER, PlayerWrapper);
MEMORY_POOL_INSTANCE(PlayerWrapper, LUA_WRAPPER_POOL_SIZE);

PlayerWrapper::WrapperCacheType *PlayerWrapper::WrapperCache = NULL;

PlayerWrapper::PlayerWrapper() : 
	Object(NULL)
,	Script(NULL)
{
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Is_Neutral", &PlayerWrapper::Lua_Is_Neutral);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Get_ID", &PlayerWrapper::Lua_Get_ID);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Get_Name", &PlayerWrapper::Lua_Get_Name);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Is_Valid", &PlayerWrapper::Is_Valid);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Give_Money", &PlayerWrapper::Lua_Give_Money);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Set_Tech_Level", &PlayerWrapper::Lua_Set_Tech_Level);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Release_Credits_For_Tactical", &PlayerWrapper::Lua_Release_Credits_For_Tactical);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Get_Credits", &PlayerWrapper::Lua_Get_Credits);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Get_GameSpy_Stats_Player_ID", &PlayerWrapper::Lua_Get_GameSpy_Stats_Player_ID);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Get_Enemy", &PlayerWrapper::Lua_Get_Enemy);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Get_Faction_Name", &PlayerWrapper::Lua_Get_Faction_Name);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Get_Tech_Level", &PlayerWrapper::Lua_Get_Tech_Level);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Retreat", &PlayerWrapper::Lua_Retreat);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Give_Random_Sliceable_Tech", &PlayerWrapper::Lua_Give_Random_Sliceable_Tech);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Unlock_Tech", &PlayerWrapper::Lua_Unlock_Tech);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Lock_Tech", &PlayerWrapper::Lua_Lock_Tech);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Is_Human", &PlayerWrapper::Lua_Is_Human);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Enable_As_Actor", &PlayerWrapper::Lua_Enable_As_Actor);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Is_Enemy", &PlayerWrapper::Lua_Is_Enemy);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Is_Ally", &PlayerWrapper::Lua_Is_Ally);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Select_Object", &PlayerWrapper::Lua_Select_Object);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Disable_Bombing_Run", &PlayerWrapper::Disable_Bombing_Run);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Enable_Advisor_Hints", &PlayerWrapper::Enable_Advisor_Hints);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Get_Difficulty", &PlayerWrapper::Get_Difficulty);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Set_Black_Market_Tutorial", &PlayerWrapper::Lua_Set_Black_Market_Tutorial);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Set_Sabotage_Tutorial", &PlayerWrapper::Lua_Set_Sabotage_Tutorial);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Make_Ally", &PlayerWrapper::Make_Ally);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Make_Enemy", &PlayerWrapper::Make_Enemy);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Disable_Orbital_Bombardment", &PlayerWrapper::Disable_Orbital_Bombardment);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Remove_Orbital_Bombardment", &PlayerWrapper::Remove_Orbital_Bombardment);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Get_Clan_ID", &PlayerWrapper::Lua_Get_Clan_ID);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Get_Team", &PlayerWrapper::Lua_Get_Team);
	LUA_REGISTER_MEMBER_FUNCTION(PlayerWrapper, "Get_Space_Station", &PlayerWrapper::Get_Space_Station);

}

PlayerWrapper::~PlayerWrapper()
{
	Remove_Cached_Wrapper();
}

void PlayerWrapper::Init(PlayerClass *object)
{
	assert(!Object);
	Object = object;
}

LuaTable* PlayerWrapper::Lua_Give_Money(LuaScriptClass *script, LuaTable *params)
{
	if (!params || params->Value.size() == 0)
	{
		script->Script_Error("PlayerWrapper::Lua_Give_Money -- Missing required parameter.");
		return NULL;
	}

	LuaNumber::Pointer num = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!num)
	{
		script->Script_Error("PlayerWrapper::Lua_Give_Money -- Parameter 1 must be a number.");
		return NULL;
	}
	Object->Add_Credits(num->Value, true);
	return NULL;
}

LuaTable* PlayerWrapper::Lua_Set_Tech_Level(LuaScriptClass *script, LuaTable *params)
{
	if (!params || params->Value.size() == 0)
	{
		script->Script_Error("PlayerWrapper::Lua_Set_Tech_Level -- Missing required parameter.");
		return NULL;
	}
	LuaNumber::Pointer num = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!num)
	{
		script->Script_Error("PlayerWrapper::Lua_Set_Tech_Level -- Parameter 1 must be a number.");
		return NULL;
	}
	Object->Set_Tech_Level(static_cast<int>(num->Value));
	return NULL;
}

LuaTable* PlayerWrapper::Lua_Get_Name(LuaScriptClass *, LuaTable *)
{
	if (!Object) return NULL;

	return Return_Variable(new LuaString(To_MultiByte(Object->Get_Player_Name()->c_str())));
}

LuaTable* PlayerWrapper::Lua_Get_GameSpy_Stats_Player_ID(LuaScriptClass *, LuaTable *)
{
	if (!Object) return Return_Variable(new LuaNumber(-1));

	return Return_Variable(new LuaNumber((float)Object->Get_GameSpy_Stats_Player_Index()));
}

LuaTable* PlayerWrapper::Lua_Get_ID(LuaScriptClass *, LuaTable *)
{
	if (!Object) return Return_Variable(new LuaNumber(-1));

	return Return_Variable(new LuaNumber((float)Object->Get_ID()));
}

LuaTable* PlayerWrapper::Lua_Get_Clan_ID(LuaScriptClass *, LuaTable *)
{
	if (!Object) return Return_Variable(new LuaNumber(-1));

	return Return_Variable(new LuaNumber((float)Object->Get_Clan_ID()));
}

LuaTable* PlayerWrapper::Lua_Get_Team(LuaScriptClass *, LuaTable *)
{
	if (!Object) return Return_Variable(new LuaNumber(-1));

	return Return_Variable(new LuaNumber((float)Object->Get_Team()));
}

LuaTable* PlayerWrapper::Lua_Is_Neutral(LuaScriptClass *, LuaTable *)
{
	if (!Object) return NULL;

   return Return_Variable(new LuaBool(Object->Get_Faction()->Is_Neutral()));
}

PlayerClass *PlayerWrapper::Get_Object(void) const
{
	return Object;
}

bool PlayerWrapper::Save(LuaScriptClass * /*script*/, ChunkWriterClass *writer) 
{
	assert(writer != NULL);
	bool ok = true;

	WRITE_MICRO_CHUNK_OBJECT_PTR		(PLAYERID_MICRO_CHUNK,			Object);

	return (ok);
}

bool PlayerWrapper::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;

	while (reader->Open_Micro_Chunk()) {
		switch (reader->Cur_Micro_Chunk_ID()) {
			READ_MICRO_CHUNK_OBJECT_PTR		(PLAYERID_MICRO_CHUNK,			Object);
			default: assert(false); break;   // Unknown Chunk
		}
		reader->Close_Micro_Chunk();
	}

	Script = script;
	SaveLoadClass::Register_Post_Load_Callback(Post_Load_Member_Callback<PlayerWrapper>, this);

	return (ok);
}

void PlayerWrapper::Post_Load_Callback(void)
{
	if (Object && Script)
	{
		WrapperCache->insert(std::make_pair(std::make_pair(Object, Script), this));
	}
}

void PlayerWrapper::Init_Wrapper_Cache(void)
{
	if (!WrapperCache)
	{
		WrapperCache = new WrapperCacheType();
	}
}

void PlayerWrapper::Shutdown_Wrapper_Cache(void)
{
	if (WrapperCache)
	{
		delete WrapperCache;
		WrapperCache = NULL;
	}
}

void PlayerWrapper::Remove_Cached_Wrapper(void)
{
	if (Script)
	{
		WrapperCacheType::iterator it = WrapperCache->find(std::make_pair((PlayerClass *)Object, Script));
		if (it != WrapperCache->end() && it->second == this)
		{
			WrapperCache->erase(it);
			Script = 0;
		}
	}
}

PlayerWrapper *PlayerWrapper::Create(PlayerClass *obj, LuaScriptClass *script)
{
	FAIL_IF(!obj) return NULL;
	PlayerWrapper *player;
	if (script)
	{
		std::pair<WrapperCacheType::iterator, bool> retval = WrapperCache->insert(std::make_pair(std::make_pair(obj, script), (PlayerWrapper *)NULL));
		if (retval.second)
		{
			player = (PlayerWrapper *) FactoryCreate();
			player->Init(obj);
			retval.first->second = player;
			player->Script = script;
		}
		else
		{
			player = retval.first->second;
			assert(player);
			assert(player->Script == script);
		}
	}
	else
	{
		player = (PlayerWrapper *) FactoryCreate();
		player->Init(obj);
	}

	return player;
}

bool PlayerWrapper::Is_Equal(const LuaVar *lua_var) const
{
	SmartPtr<PlayerWrapper> other_player = PG_Dynamic_Cast<PlayerWrapper>(const_cast<LuaVar*>(lua_var));
	if (!other_player)
	{
		SmartPtr<LuaVoid> nil_object = PG_Dynamic_Cast<LuaVoid>(const_cast<LuaVar*>(lua_var));
		if (nil_object)
		{
			return Object == nil_object->Value;
		}
		return false;
	}

	return other_player->Get_Object() == Object;
}

/**************************************************************************************************
* PlayerWrapper::Lua_Release_Credits_For_Tactical -- Script function to divert credits from galactic
*	mode budget to be spent in tactical mode.
*
* In:				
*
* Out:		
*
* History: 2/28/2005 10:28AM JSY
**************************************************************************************************/
LuaTable *PlayerWrapper::Lua_Release_Credits_For_Tactical(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return 0; }

	if (GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_LAND)
	{
		script->Script_Error("PlayerWrapper::Release_Credits_For_Tactical - this script function is only valid in land tactical");
		return 0;
	}

	if (params->Value.size() > 1)
	{
		script->Script_Error("PlayerWrapper::Release_Credits_For_Tactical - invalid number of parameters.  Expected 0 or 1, got %d.", params->Value.size());
		return 0;
	}

	float credits_to_release = -1.0f;
	if (params->Value.size() == 1)
	{
		SmartPtr<LuaNumber> lua_credits = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
		if (!lua_credits)
		{
			script->Script_Error("PlayerWrapper::Release_Credits_For_Tactical - invalid type for parameter 1.  Expected number.");
			return 0;
		}
		credits_to_release = lua_credits->Value;
	}

	AIPlayerClass *ai_player = Object->Get_AI_Player();
	if (!ai_player)
	{
		script->Script_Error("PlayerWrapper::Release_Credits_For_Tactical - this action is not possible: player is not AI controlled.");
		return 0;
	}

	TacticalAIManagerClass *tactical_manager = ai_player->Get_Tactical_Manager_By_Mode(SUB_GAME_MODE_GALACTIC);
	if (!tactical_manager)
	{
		script->Script_Warning("PlayerWrapper::Release_Credits_For_Tactical - player has no associated galactic AI.");
		return 0;
	}

	FAIL_IF(!tactical_manager->Get_Goal_System()) { return 0; }

	tactical_manager->Get_Goal_System()->Get_Budget().Release_Additional_Resources_For_Tactical(credits_to_release);

	return 0;
}

/**************************************************************************************************
* PlayerWrapper::Lua_Get_Credits -- Script function to check this player's current credits.
*
* In:				
*
* Out:		
*
* History: 2/28/2005 10:28AM JSY
**************************************************************************************************/
LuaTable *PlayerWrapper::Lua_Get_Credits(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	return Return_Variable(new LuaNumber(Object->Get_Credits()));
}

LuaTable *PlayerWrapper::Lua_Get_Enemy(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	PlayerClass *enemy = 0;
	if (Object->Get_AI_Player())
	{
		enemy = Object->Get_AI_Player()->Get_Enemy();
	}
	else
	{
		enemy = Object->Get_Enemy();
	}

	if (enemy)
	{
		return Return_Variable(Create(enemy, script));
	}
	else
	{
		return NULL;
	}
}

LuaTable *PlayerWrapper::Lua_Get_Faction_Name(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	return Return_Variable(new LuaString(*Object->Get_Faction()->Get_Name()));
}

LuaTable *PlayerWrapper::Lua_Get_Tech_Level(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	return Return_Variable(new LuaNumber(static_cast<float>(Object->Get_Tech_Level())));
}

LuaTable *PlayerWrapper::Lua_Retreat(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (GameModeManager.Get_Active_Mode()->Get_Sub_Type() == SUB_GAME_MODE_GALACTIC)
	{
		TheAutoResolver->Player_Retreats(Object->Get_ID());
	}
	else
	{
		RetreatCoordinatorClass *retreat_coordinator = GameModeManager.Get_Active_Mode()->Get_Retreat_Coordinator();
		if (!retreat_coordinator)
		{
			script->Script_Error("PlayerWrapper::Lua_Retreat -- No retreat coordinator!  What's going on?");
			return Return_Variable(new LuaBool(false));
		}

		if (retreat_coordinator->Is_Retreat_Currently_Allowed(Object->Get_ID()) != RetreatCoordinatorClass::REQUEST_OK)
		{
			return Return_Variable(new LuaBool(false));
		}

		retreat_coordinator->Request_Start_Player_Retreat(Object->Get_ID(), false);
	}
	return Return_Variable(new LuaBool(true));
}

LuaTable *PlayerWrapper::Lua_Give_Random_Sliceable_Tech(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	DiscreteDistributionClass<const GameObjectTypeClass*> slice_distribution;
	int total_num_types = GameObjectTypeManager.Get_Total_Object_Types();
	for (int type_idx = 0; type_idx < total_num_types; ++type_idx)
	{
		// Get this type.
		const GameObjectTypeClass *type = GameObjectTypeManager.Get_Game_Object_Type(type_idx);
		FAIL_IF( type == NULL )				{ continue; }

		// Can the player's faction build this kind of object?
		if ( type->Is_Affiliated_With_Faction(Object->Get_Faction()) == false )
			continue;

		// Is it buildable at this tech level?
		if ( static_cast<int>(type->Get_Tech_Level()) > Object->Get_Tech_Level() )
			continue;

		// Is it initially locked?
		if ( type->Is_Build_Initially_Locked() == false )
			continue;

		// Does the player already know how to build it?
		if ( Object->Is_Object_Type_On_Build_Unlocked_List(type) == true )
			continue;

		// Can it be unlocked by a slicer?
		if ( type->Can_Be_Unlocked_By_Slicer() == false )
			continue;

		// Alright, this is a candidate for slicing!
		slice_distribution.Add_Element(type, 1.0f / type->Get_Slice_Cost_Credits());
	}

	if (slice_distribution.Num_Elements() == 0)
	{
		if (Object->Get_Tech_Level() == 0)
		{
			Object->Increment_Tech_Level();
			return Lua_Give_Random_Sliceable_Tech(script, params);
		}
		else
		{
			return NULL;
		}
	}

	GameObjectTypeClass *random_tech = const_cast<GameObjectTypeClass*>(slice_distribution.Sample());

	if (slice_distribution.Num_Elements() == 1)
	{
		Object->Increment_Tech_Level();
	}
	else
	{
		Object->Add_Object_Type_To_Build_Unlocked_List(random_tech);
	}

	return Return_Variable(GameObjectTypeWrapper::Create(random_tech, script));
}

LuaTable *PlayerWrapper::Lua_Unlock_Tech(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("PlayerWrapper::Lua_Unlock_Tech -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectTypeWrapper> type_wrapper = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
	if (!type_wrapper)
	{
		script->Script_Error("PlayerWrapper::Lua_Unlock_Tech -- invalid type for parameter 1.  Expected game object type.");
		return NULL;
	}

	GameObjectTypeClass *unlock_type = type_wrapper->Get_Object();
	FAIL_IF(!unlock_type) { return NULL; }

	if (!Object->Is_Object_Type_On_Build_Locked_List(unlock_type) && Object->Is_Object_Type_On_Build_Unlocked_List(unlock_type))
	{
		script->Script_Warning("PlayerWrapper::Lua_Unlock_Tech -- type %s is already unlocked.",unlock_type->Get_Name()->c_str());
		return NULL;
	}

	Object->Remove_Object_Type_From_Build_Locked_List(unlock_type);
	Object->Add_Object_Type_To_Build_Unlocked_List(unlock_type);

	return NULL;
}

LuaTable *PlayerWrapper::Lua_Lock_Tech(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("PlayerWrapper::Lua_Lock_Tech -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectTypeWrapper> type_wrapper = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
	if (!type_wrapper)
	{
		script->Script_Error("PlayerWrapper::Lua_Lock_Tech -- invalid type for parameter 1.  Expected game object type.");
		return NULL;
	}

	GameObjectTypeClass *lock_type = type_wrapper->Get_Object();
	FAIL_IF(!lock_type) { return NULL; }

	if (Object->Is_Object_Type_On_Build_Locked_List(lock_type))
	{
		script->Script_Warning("PlayerWrapper::Lua_Lock_Tech -- type %s is already locked.", lock_type->Get_Name()->c_str());
		return NULL;
	}

	Object->Remove_Object_Type_From_Build_Unlocked_List(lock_type);
	Object->Add_Object_Type_To_Build_Locked_List(lock_type);

	return NULL;
}

LuaTable *PlayerWrapper::Lua_Is_Human(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	return Return_Variable(new LuaBool(Object->Is_Human()));
}

LuaTable *PlayerWrapper::Lua_Enable_As_Actor(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (Object->Get_AI_Player() == NULL)
	{
		script->Script_Error("PlayerWrapper::Lua_Enable_As_Actor -- This player has no AI attached to it.");
		return NULL;
	}

	Object->Get_AI_Player()->Create_Tactical_Manager(GameModeManager.Get_Active_Mode());

	return NULL;
}

LuaTable *PlayerWrapper::Lua_Is_Enemy(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("PlayerWrapper::Lua_Is_Enemy -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return Return_Variable(new LuaBool(false));
	}

	SmartPtr<PlayerWrapper> player_wrapper = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!player_wrapper || player_wrapper->Get_Object() == NULL)
	{
		script->Script_Error("PlayerWrapper::Lua_Is_Enemy -- invalid type for parameter 1.  Expected player object.");
		return Return_Variable(new LuaBool(false));
	}

	return Return_Variable(new LuaBool(Object->Is_Enemy(player_wrapper->Get_Object())));
}

LuaTable *PlayerWrapper::Lua_Is_Ally(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("PlayerWrapper::Lua_Is_Ally -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return Return_Variable(new LuaBool(false));
	}

	SmartPtr<PlayerWrapper> player_wrapper = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!player_wrapper || player_wrapper->Get_Object() == NULL)
	{
		script->Script_Error("PlayerWrapper::Lua_Is_Ally -- invalid type for parameter 1.  Expected player object.");
		return Return_Variable(new LuaBool(false));
	}

	return Return_Variable(new LuaBool(Object->Is_Ally(player_wrapper->Get_Object())));
}


LuaTable *PlayerWrapper::Lua_Select_Object(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("PlayerWrapper::Lua_Select_Object -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	if (!object_wrapper || object_wrapper->Get_Object() == NULL)
	{
		script->Script_Error("PlayerWrapper::Lua_Select_Object -- invalid type for parameter 1.  Expected game object.");
		return NULL;
	}

	int player_id = Object->Get_ID();
	int game_object_id = object_wrapper->Get_Object()->Get_ID();
	GAME_OBJECT_MANAGER.Select_Object(game_object_id, player_id);
	ReferenceListClass<GameObjectClass> *selected_objects = GAME_OBJECT_MANAGER.Get_Selection_List(player_id);
	ENFORCED_IF( selected_objects != NULL )
	{
		// Build a DynamicVector of game object pointers for those selected objects and update the GameplayUI's
		// list of selected objects to match, if this is the local player.
		if ( PlayerList.Get_Local_Player_ID() == player_id  && GamePlayUISelectionListUpdateCallback != NULL )
		{
			DynamicVectorClass<GameObjectClass*> selected(true);
			ReferenceListIterator<GameObjectClass> it(selected_objects);
			for ( it.First(); !it.Is_Done(); it.Next() )
			{
				GameObjectClass *object = it.Current_Object();
				FAIL_IF( object == NULL )				{ continue; }
				selected.Add(object);
			}

			GamePlayUISelectionListUpdateCallback( &selected, false );
		}
	}

	return NULL;
}

/**************************************************************************************************
* PlayerWrapper::Disable_Bombing_Run -- Script function to prevent bombing runs for this player
*	(regardless of actual bomber presence).  Note however that calling Disable(false) does not guarantee that a
*	bombing run will be possible since that *does* depend on bomber presence
*
* In:				
*
* Out:		
*
* History: 10/19/2005 3:05PM JSY
**************************************************************************************************/
LuaTable *PlayerWrapper::Disable_Bombing_Run(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return false; }

	if (GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_LAND)
	{
		script->Script_Error("PlayerWrapper::Disable_Bombing_Run -- bombing runs are only relevant to land mode (which this mode isn't)!");
		return NULL;
	}
	
	if (params->Value.size() != 1)
	{
		script->Script_Error("PlayerWrapper::Disable_Bombing_Run -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaBool> on_off = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!on_off)
	{
		script->Script_Error("PlayerWrapper::Disable_Bombing_Run -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	BattlefieldModifiersClass *mods = GameModeManager.Get_Active_Mode()->Get_Battlefield_Modifiers(Object->Get_ID());
	ENFORCED_IF(mods)
	{
		mods->BombingRunEnableMods.Set_Modifier(on_off->Value, ModifierSourceClass());
	}

	return NULL;
}

/**************************************************************************************************
* PlayerWrapper::Enable_Advisor_Hints -- Script function to switch on or off periodic advisor hints
*	for this player.
*
* In:				
*
* Out:		
*
* History: 10/21/2005 10:27AM JSY
**************************************************************************************************/
LuaTable *PlayerWrapper::Enable_Advisor_Hints(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return false; }

	if (params->Value.size() != 2)
	{
		script->Script_Error("PlayerWrapper::Enable_Advisor_Hints -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> mode_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!mode_name)
	{
		script->Script_Error("PlayerWrapper::Enable_Advisor_Hints -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	SubGameModeType mode = SUB_GAME_MODE_INVALID;
	if (!TheSubGameModeTypeConverterPtr->String_To_Enum(mode_name->Value, mode))
	{
		script->Script_Error("PlayerWrapper::Enable_Advisor_Hints -- unknown game mode %s.", mode_name->Value.c_str());
		return NULL;
	}

	SmartPtr<LuaBool> on_off = PG_Dynamic_Cast<LuaBool>(params->Value[1]);
	if (!on_off)
	{
		script->Script_Error("PlayerWrapper::Enable_Advisor_Hints -- invalid type for parameter 2.  Expected boolean.");
		return NULL;
	}

	Object->Set_Advisor_Hints_Enabled(mode, on_off->Value);

	return NULL;
}

/**************************************************************************************************
* PlayerWrapper::Get_Difficulty -- Examine the difficulty level of this player (as a string)
*
* In:				
*
* Out:		
*
* History: 10/21/2005 3:30PM JSY
**************************************************************************************************/
LuaTable *PlayerWrapper::Get_Difficulty(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	DifficultyLevelType difficulty = DIFFICULTY_LEVEL_NORMAL;
	if (Object->Get_AI_Player())
	{
		difficulty = Object->Get_AI_Player()->Get_Difficulty();
	}

	return Return_Variable(new LuaString(TheDifficultyLevelTypeConverterPtr->Enum_To_String(difficulty)));
}

/**************************************************************************************************
* PlayerWrapper::Lua_Set_Black_Market_Tutorial -- 
*
* In:				
*
* Out:		
*
* History: 6/22/2006 4:31PM MLL
**************************************************************************************************/
LuaTable* PlayerWrapper::Lua_Set_Black_Market_Tutorial(LuaScriptClass *script, LuaTable *params)
{
	if (!params || params->Value.size() == 0)
	{
		script->Script_Error("PlayerWrapper::Lua_Set_Black_Market_Tutorial -- Missing required parameter.");
		return NULL;
	}
	SmartPtr<LuaBool> toggle = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!toggle)
	{
		script->Script_Error("PlayerWrapper::Lua_Set_Black_Market_Tutorial -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	Object->Set_Black_Market_Tutorial(toggle->Value); 
	return NULL;
}



/**************************************************************************************************
* PlayerWrapper::Lua_Set_Sabotage_Tutorial -- 
*
* In:				
*
* Out:		
*
* History: 6/22/2006 4:53PM JAC
**************************************************************************************************/
LuaTable* PlayerWrapper::Lua_Set_Sabotage_Tutorial(LuaScriptClass *script, LuaTable *params)
{
	if (!params || params->Value.size() == 0)
	{
		script->Script_Error("PlayerWrapper::Lua_Set_Sabotage_Tutorial -- Missing required parameter.");
		return NULL;
	}
	SmartPtr<LuaBool> toggle = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!toggle)
	{
		script->Script_Error("PlayerWrapper::Lua_Set_Sabotage_Tutorial -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	Object->Set_Sabotage_Tutorial(toggle->Value); 
	return NULL;
}

/**************************************************************************************************
* PlayerWrapper::Make_Ally -- Script function to make this player consider another as an ally
*
* In:				
*
* Out:		
*
* History: 7/27/2006 2:17PM JSY
**************************************************************************************************/
LuaTable *PlayerWrapper::Make_Ally(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("PlayerWrapper::Make_Ally -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<PlayerWrapper> ally = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!ally)
	{
		script->Script_Error("PlayerWrapper::Make_Ally -- invalid type for parameter 1.  Expected player.");
		return NULL;
	}

	if (!ally->Get_Object())
	{
		script->Script_Error("PlayerWrapper::Make_Ally -- attempt to ally with mystery player who appears to have been removed from the game.");
		return NULL;
	}

	Object->Set_Ally_Status_To_Other_Player(ally->Get_Object()->Get_ID(), PLAYER_ALLY_FRIEND);

	return NULL;
}

/**************************************************************************************************
* PlayerWrapper::Make_Enemy -- Script function to make this player consider another as an enemy
*
* In:				
*
* Out:		
*
* History: 7/27/2006 2:17PM JSY
**************************************************************************************************/
LuaTable *PlayerWrapper::Make_Enemy(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("PlayerWrapper::Make_Enemy -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<PlayerWrapper> enemy = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!enemy)
	{
		script->Script_Error("PlayerWrapper::Make_Enemy -- invalid type for parameter 1.  Expected player.");
		return NULL;
	}

	if (!enemy->Get_Object())
	{
		script->Script_Error("layerWrapper::Make_Enemy -- attempt to make an enemy of mystery player who appears to have been removed from the game.");
		return NULL;
	}

	Object->Set_Ally_Status_To_Other_Player(enemy->Get_Object()->Get_ID(), PLAYER_ALLY_ENEMY);

	return NULL;
}

/**************************************************************************************************
* PlayerWrapper::Disable_Orbital_Bombardment -- Script function to prevent orbital bombardment for this player
*	(regardless of actual capital ship presence).  Note however that calling Disable(false) does not guarantee that a
*	bombardment will be possible since that *does* depend on capital ship presence
*
* In:				
*
* Out:		
*
* History: 7/31/2006 4:33PM JSY
**************************************************************************************************/
LuaTable *PlayerWrapper::Disable_Orbital_Bombardment(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return false; }

	if (GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_LAND)
	{
		script->Script_Error("PlayerWrapper::Disable_Orbital_Bombardment -- orbital bombardment is only relevant to land mode (which this mode isn't)!");
		return NULL;
	}
	
	if (params->Value.size() != 1)
	{
		script->Script_Error("PlayerWrapper::Disable_Orbital_Bombardment -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaBool> on_off = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!on_off)
	{
		script->Script_Error("PlayerWrapper::Disable_Orbital_Bombardment -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	BattlefieldModifiersClass *mods = GameModeManager.Get_Active_Mode()->Get_Battlefield_Modifiers(Object->Get_ID());
	ENFORCED_IF(mods)
	{
		mods->PlanetaryBombardEnableMods.Set_Modifier(!on_off->Value, ModifierSourceClass());
	}

	return NULL;
}

LuaTable *PlayerWrapper::Remove_Orbital_Bombardment(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return false; }

	if (GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_LAND)
	{
		script->Script_Error("PlayerWrapper::Remove_Orbital_Bombardment -- orbital bombardment is only relevant to land mode (which this mode isn't)!");
		return NULL;
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("PlayerWrapper::Remove_Orbital_Bombardment -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaBool> on_off = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!on_off)
	{
		script->Script_Error("PlayerWrapper::Remove_Orbital_Bombardment -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}


	PlanetaryBombardManagerClass* bombard = GameModeManager.Get_Active_Mode()->Get_Land_Planetary_Bombard_Manager();
	if (bombard)
	{
		bombard->Set_Bombing_Run_Valid(Object->Get_ID(), !on_off->Value);
	}

	return NULL;
}

LuaTable *PlayerWrapper::Get_Space_Station(LuaScriptClass *script, LuaTable *)
{
	FAIL_IF(!Object) { return NULL; }

	if (GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_SPACE)
	{
		script->Script_Error("PlayerWrapper::Get_Space_Station -- this command may only be used in space mode.");
		return NULL;
	}

	for (int i = 0; i < PlayerList.Get_Num_Players(); ++i)
	{
		PlayerClass *ally = PlayerList.Get_Player_By_Index(i);
		if (!ally)
		{
			continue;
		}

		if (!ally->Is_Ally(Object))
		{
			continue;
		}

		ReferenceListIterator<GameObjectClass> it(GAME_OBJECT_MANAGER.Get_Owned_List(ally->Get_ID()));
			
		for (; !it.Is_Done(); it.Next())
		{
			GameObjectClass *object = it.Current_Object();
			FAIL_IF(!object) { continue; }

			//Ignore possible capturable stations
			if (object->Behaves_Like(BEHAVIOR_DUMMY_STAR_BASE) && !object->Behaves_Like(BEHAVIOR_CAPTURE_POINT))
			{
				return Return_Variable(GameObjectWrapper::Create(object, script));
			}
		}
	}

	return NULL;
}

