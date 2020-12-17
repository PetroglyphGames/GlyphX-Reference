// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/GameObjectTypeWrapper.cpp#3 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/GameObjectTypeWrapper.cpp $
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
#include "Assert.h"
#include "GameObjectTypeWrapper.h"
#include "GameObjectTypeManager.h"
#include "GameObjectType.h"
#include "ChunkFile.h"
#include "AI/Planning/PotentialPlan.h"
#include "CRC.h"
#include "PlayerWrapper.h"
#include "GameObjectWrapper.h"

enum {
	GAMEOBJECTTYPEID_MICRO_CHUNK,
};

PG_IMPLEMENT_RTTI(GameObjectTypeWrapper, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_GAMEOBJECTTYPE_WRAPPER, GameObjectTypeWrapper);
MEMORY_POOL_INSTANCE(GameObjectTypeWrapper, LUA_WRAPPER_POOL_SIZE);

GameObjectTypeWrapper::WrapperCacheType *GameObjectTypeWrapper::WrapperCache = NULL;


GameObjectTypeWrapper::GameObjectTypeWrapper() : 
	Script(NULL)
{
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Get_Build_Cost", &GameObjectTypeWrapper::Get_Build_Cost);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Get_Combat_Rating", &GameObjectTypeWrapper::Get_Combat_Rating);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Is_Valid", &GameObjectTypeWrapper::Is_Valid);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Is_Hero", &GameObjectTypeWrapper::Lua_Is_Hero);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Get_Name", &GameObjectTypeWrapper::Lua_Get_Name);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Get_Base_Level", &GameObjectTypeWrapper::Get_Base_Level);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Get_Tech_Level", &GameObjectTypeWrapper::Get_Tech_Level);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Is_Affiliated_With", &GameObjectTypeWrapper::Is_Affiliated_With);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Is_Build_Locked", &GameObjectTypeWrapper::Is_Build_Locked);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Is_Obsolete", &GameObjectTypeWrapper::Is_Obsolete);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Get_Tactical_Build_Cost", &GameObjectTypeWrapper::Get_Tactical_Build_Cost);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Get_Score_Cost_Credits", &GameObjectTypeWrapper::Get_Score_Cost_Credits);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Get_Max_Range", &GameObjectTypeWrapper::Get_Max_Range);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Get_Min_Range", &GameObjectTypeWrapper::Get_Min_Range);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Get_Bribe_Cost", &GameObjectTypeWrapper::Get_Bribe_Cost);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Is_Affected_By_Missile_Shield", &GameObjectTypeWrapper::Is_Affected_By_Missile_Shield);
	LUA_REGISTER_MEMBER_FUNCTION(GameObjectTypeWrapper, "Is_Affected_By_Laser_Defense", &GameObjectTypeWrapper::Is_Affected_By_Laser_Defense);
}

GameObjectTypeWrapper::~GameObjectTypeWrapper()
{
	Remove_Cached_Wrapper();
}

void GameObjectTypeWrapper::Init(GameObjectTypeClass *object)
{
	assert(!Object);
	Object = object;
}

GameObjectTypeClass *GameObjectTypeWrapper::Get_Object(void) const
{
	return Object;
}

bool GameObjectTypeWrapper::Save(LuaScriptClass *, ChunkWriterClass *writer) 
{
	assert(writer != NULL);
	bool ok = true;

	WRITE_MICRO_CHUNK_TYPE_PTR						(GAMEOBJECTTYPEID_MICRO_CHUNK,			Object);

	return (ok);
}

bool GameObjectTypeWrapper::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	GameObjectTypeClass *obj = NULL;
	while (reader->Open_Micro_Chunk()) {
		switch (reader->Cur_Micro_Chunk_ID()) {
			READ_MICRO_CHUNK_TYPE_PTR						(GAMEOBJECTTYPEID_MICRO_CHUNK,			obj);
			default: assert(false); break;   // Unknown Chunk
		}
		reader->Close_Micro_Chunk();
	}

	Object = obj;
	Script = script;

	if (Object && script)
	{
		WrapperCache->insert(std::make_pair(std::make_pair(obj, script), this));
	}

	return ok;
}

void GameObjectTypeWrapper::Remove_Cached_Wrapper(void)
{
	if (Script)
	{
		WrapperCacheType::iterator it = WrapperCache->find(std::make_pair((GameObjectTypeClass *)Object, Script));
		if (it != WrapperCache->end() && it->second == this)
		{
			WrapperCache->erase(it);
			Script = 0;
		}
	}
}

void GameObjectTypeWrapper::Init_Wrapper_Cache(void)
{
	if (!WrapperCache)
	{
		WrapperCache = new WrapperCacheType();
	}
}

void GameObjectTypeWrapper::Shutdown_Wrapper_Cache(void)
{
	if (WrapperCache)
	{
		delete WrapperCache;
		WrapperCache = NULL;
	}
}

GameObjectTypeWrapper *GameObjectTypeWrapper::Create(GameObjectTypeClass *obj, LuaScriptClass *script)
{
	FAIL_IF(!obj) { return NULL; }

	GameObjectTypeWrapper *gobj;
	if (script)
	{
		std::pair<WrapperCacheType::iterator, bool> retval = WrapperCache->insert(std::make_pair(std::make_pair(obj, script), (GameObjectTypeWrapper *)NULL));
		if (retval.second)
		{
			gobj = (GameObjectTypeWrapper *) FactoryCreate();
			gobj->Init(obj);
			retval.first->second = gobj;
			gobj->Script = script;
		}
		else
		{
			gobj = retval.first->second;
			assert(gobj);
			assert(gobj->Script == script);
		}
	}
	else
	{
		gobj = (GameObjectTypeWrapper *) FactoryCreate();
		gobj->Init(obj);
	}

	return gobj;
}

LuaTable *GameObjectTypeWrapper::Lua_Is_Hero(LuaScriptClass *, LuaTable *)
{
	const GameObjectTypeClass *htype = PotentialPlanClass::Get_Hero_Type_From_Build_Type(Object);

	if (!htype || (htype->Is_Named_Hero() == false && htype->Is_Special_Weapon_In_Space() == false))
		return Return_Variable(new LuaBool(false));

	return Return_Variable(new LuaBool(true));
}

LuaTable *GameObjectTypeWrapper::Get_Base_Level(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaNumber((float)Object->Get_Base_Level()));
}

LuaTable *GameObjectTypeWrapper::Lua_Get_Name(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaString(*Object->Get_Name()));
}

LuaTable *GameObjectTypeWrapper::Get_Combat_Rating(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaNumber(static_cast<float>(Object->Get_AI_Combat_Power_Metric())));
}

LuaTable *GameObjectTypeWrapper::Get_Build_Cost(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaNumber(static_cast<float>(Object->Get_Build_Cost_Credits())));
}

LuaTable *GameObjectTypeWrapper::Get_Tactical_Build_Cost(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaNumber(static_cast<float>(Object->Get_Tactical_Build_Cost_Credits())));
}

LuaTable *GameObjectTypeWrapper::Get_Score_Cost_Credits(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaNumber(static_cast<float>(Object->Get_Score_Cost_Credits())));
}

bool GameObjectTypeWrapper::Is_Equal(const LuaVar *lua_var) const
{
	SmartPtr<GameObjectTypeWrapper> other_type = PG_Dynamic_Cast<GameObjectTypeWrapper>(const_cast<LuaVar*>(lua_var));
	if (!other_type)
	{
		return false;
	}

	return other_type->Get_Object() == Object;
}

LuaTable *GameObjectTypeWrapper::Get_Tech_Level(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaNumber(static_cast<float>(Object->Get_Tech_Level())));
}

LuaTable *GameObjectTypeWrapper::Is_Build_Locked(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectTypeWrapper::Is_Build_Locked -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<PlayerWrapper> player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!player)
	{
		script->Script_Error("GameObjectTypeWrapper::Is_Build_Locked -- invalid type for parameter 1.  Expected player");
		return NULL;
	}

	bool is_initially_locked = Object->Is_Build_Initially_Locked();
	bool has_been_unlocked = player->Get_Object()->Is_Object_Type_On_Build_Unlocked_List(Object);

	return Return_Variable(new LuaBool(is_initially_locked && !has_been_unlocked));
}

LuaTable *GameObjectTypeWrapper::Is_Obsolete(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectTypeWrapper::Is_Obsolete -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<PlayerWrapper> player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!player)
	{
		script->Script_Error("GameObjectTypeWrapper::Is_Obsolete -- invalid type for parameter 1.  Expected player");
		return NULL;
	}

	return Return_Variable(new LuaBool(player->Get_Object()->Is_Object_Type_On_Build_Locked_List(Object)));
}

LuaTable *GameObjectTypeWrapper::Is_Affiliated_With(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GameObjectTypeWrapper::Is_Affiliated_With -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<PlayerWrapper> player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!player)
	{
		script->Script_Error("GameObjectTypeWrapper::Is_Affiliated_With -- invalid type for parameter 1.  Expected player");
		return NULL;
	}

	return Return_Variable(new LuaBool(Object->Is_Affiliated_With_Faction(player->Get_Object()->Get_Faction())));
}

LuaTable *GameObjectTypeWrapper::Get_Max_Range(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaNumber(Object->Get_Targeting_Max_Attack_Distance_Raw()));
}

LuaTable *GameObjectTypeWrapper::Get_Min_Range(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaNumber(Object->Get_Targeting_Min_Attack_Distance()));
}

/**************************************************************************************************
* GameObjectTypeWrapper::Get_Bribe_Cost -- Get the cost to bribe an object of this type.  Optionally
*	accepts the object that will perform the bribe and applies relevant modifiers
*
* In:		
*			
*
* Out:	
*
* History: 6/5/2006 4:38PM JSY
**************************************************************************************************/
LuaTable *GameObjectTypeWrapper::Get_Bribe_Cost(LuaScriptClass *script, LuaTable *params)
{
	float bribe_cost = static_cast<float>(Object->Get_Tactical_Bribe_Cost());

	if (params->Value.size() == 0)
	{
		return Return_Variable(new LuaNumber(bribe_cost));
	}

	GameObjectClass *planet = GameModeManager.Get_Current_Conflict_Location();
	if (!planet)
	{
		return Return_Variable(new LuaNumber(bribe_cost));
	}

	SmartPtr<GameObjectWrapper> bribing_object = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	if (!bribing_object)
	{
		script->Script_Error("GameObjectTypeWrapper::Get_Bribe_Cost -- invalid type for parameter 1.  Expected game object wrapper.");
		return NULL;
	}

	if (!bribing_object->Get_Object())
	{
		script->Script_Error("GameObjectTypeWrapper::Get_Bribe_Cost -- bribing object is dead.");
		return NULL;
	}

	PlanetaryDataPackClass* planet_data = static_cast<PlanetaryDataPackClass*>(planet->Get_Planetary_Data());
	FAIL_IF(!planet_data) { return NULL; }

	// Units on corrupted planets usually are more easily bribed.
	if (planet_data->Planet_Is_Corrupted() == true)
	{
		bribe_cost *= bribing_object->Get_Object()->Get_Type()->Bribe_Modifier_On_Corrupted_Planets();
	}
	else
	{
		bribe_cost *= bribing_object->Get_Object()->Get_Type()->Bribe_Modifier_On_Uncorrupted_Planets();
	}

	return Return_Variable(new LuaNumber(bribe_cost));
}

/**************************************************************************************************
* GameObjectTypeWrapper::Is_Affected_By_Missile_Shield -- 
*
* In:		
*			
*
* Out:	
*
* History: 6/8/2006 2:46PM JSY
**************************************************************************************************/
LuaTable *GameObjectTypeWrapper::Is_Affected_By_Missile_Shield(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(Object->Get_Projectile_Category() == PROJECTILE_CATEGORY_ROCKET ||
													Object->Get_Projectile_Category() == PROJECTILE_CATEGORY_MPTL_ROCKET));
}

/**************************************************************************************************
* GameObjectTypeWrapper::Is_Affected_By_Laser_Defense -- 
*
* In:		
*			
*
* Out:	
*
* History: 6/8/2006 3:42PM JSY
**************************************************************************************************/
LuaTable *GameObjectTypeWrapper::Is_Affected_By_Laser_Defense(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(Object->Get_Projectile_Category() == PROJECTILE_CATEGORY_MISSILE ||
													Object->Get_Projectile_Category() == PROJECTILE_CATEGORY_ROCKET ||
													Object->Get_Projectile_Category() == PROJECTILE_CATEGORY_MPTL_ROCKET));
}
