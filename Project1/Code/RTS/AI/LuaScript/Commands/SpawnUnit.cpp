// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpawnUnit.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpawnUnit.cpp $
//
//    Original Author: Brian Hayes
//
//            $Author: Brian_Hayes $
//
//            $Change: 637819 $
//
//          $DateTime: 2017/03/22 10:16:16 $
//
//          $Revision: #1 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */
#pragma hdrstop

#include "SpawnUnit.h"
#include "Facing.h"
#include "GameModeManager.h"
#include "GameObjectManager.h"
#include "AI/LuaScript/GameObjectTypeWrapper.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "PlanetaryBehavior.h"
#include "ProductionBehavior.h"
#include "GameObjectTypeManager.h"
#include "FleetManagementEvent.h"
#include "AI/TacticalAIManager.h"
#include "AI/Execution/AIExecutionSystem.h"
#include "AI/Execution/AIFreeStore.h"
#include "AI/AIPlayer.h"



PG_IMPLEMENT_RTTI(LuaSpawnUnitCommandClass, LuaUserVar);
PG_IMPLEMENT_RTTI(LuaCreateGenericObjectClass, LuaUserVar);


// Params : object_type, position, player
LuaTable *LuaSpawnUnitCommandClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
	{
		return Galactic_Spawn_Unit(script, params);
	}

	if (params->Value.size() < 3)
	{
		script->Script_Error("LuaSpawnUnitCommandClass -- Invalid number of parameters.");
		return NULL;
	}

	GameObjectTypeWrapper *type_wrapper = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
	PlayerWrapper *player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[2]);

	if (!player)
	{
		script->Script_Error("LuaSpawnUnitCommandClass -- Invalid parameter for player.");
		return NULL;
	}

	if (!type_wrapper)
	{
		script->Script_Error("LuaSpawnUnitCommandClass -- Invalid parameter for type.");
		return NULL;
	}

	Vector3 land_pos(VECTOR3_INVALID);
	Lua_Extract_Position(params->Value[1], land_pos);
	if (land_pos == VECTOR3_INVALID)
	{
		script->Script_Error("LuaSpawnUnitCommandClass -- Invalid position parameter.");
		return NULL;
	}

	DynamicVectorClass<GameObjectClass*> squad_objects(true);
	FacingClass face(land_pos, VECTOR3_NONE);
	if (GameModeManager.Get_Sub_Type() == SUB_GAME_MODE_SPACE && type_wrapper->Get_Object()->Behaves_Like(BEHAVIOR_DUMMY_GROUND_COMPANY))
	{
		//Use Spawn_Transport_In_Space to support special transport units in space mode
		GameObjectClass *single_object = GameModeManager.Get_Active_Mode()->Get_Object_Manager().Spawn_Transport_In_Space(type_wrapper->Get_Object(), player->Get_Object()->Get_ID(),
																																								land_pos, *face.Get());
		if (single_object)
		{
			squad_objects.Add(single_object);
		}
	}
	else
	{
		GameModeManager.Get_Active_Mode()->Get_Object_Manager().Create_And_Place_Company(type_wrapper->Get_Object(), land_pos, *face.Get(), 
																													player->Get_Object()->Get_ID(), NULL, &squad_objects);
	}

	LuaTable::Pointer table = Alloc_Lua_Table();
	for (int i = 0; i < squad_objects.Get_Count(); i++)
	{
		table->Value.push_back(GameObjectWrapper::Create(squad_objects[i], script));
	}
	return Return_Variable(table);
}


LuaTable *LuaSpawnUnitCommandClass::Galactic_Spawn_Unit(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() < 3)
	{
		script->Script_Error("Galactic_Spawn_Unit -- Invalid number of parameters.");
		return NULL;
	}

	GameObjectTypeWrapper *type_wrapper = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
	PlayerWrapper *player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[2]);

	if (!player)
	{
		script->Script_Error("Galactic_Spawn_Unit -- Invalid parameter for player.");
		return NULL;
	}

	if (!type_wrapper)
	{
		script->Script_Error("Galactic_Spawn_Unit -- Invalid parameter for type.");
		return NULL;
	}

	GameObjectWrapper *planet = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[1]);
	if (!planet || !planet->Get_Object() || !planet->Get_Object()->Get_Behavior(BEHAVIOR_PRODUCTION))
	{
		script->Script_Error("Galactic_Spawn_Unit -- Invalid planet parameter.");
		return NULL;
	}

	ProductionBehaviorClass *pbehavior = static_cast<ProductionBehaviorClass *> (planet->Get_Object()->Get_Behavior( BEHAVIOR_PRODUCTION ));
	assert(pbehavior);

	// Now create the object at the planet - could return a NULL object if a special type
	GameObjectClass *built_object = pbehavior->Create_And_Place_Object_Type_At_Location(type_wrapper->Get_Object(), planet->Get_Object(), player->Get_Object()->Get_ID());
	if (!built_object) 
	{
		script->Script_Warning("Galactic_Spawn_Unit -- Create returned NULL for %s at planet: %s.", type_wrapper->Get_Object()->Get_Name()->c_str(), planet->Get_Object()->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	GameObjectTypeClass *fleet_type = GameObjectTypeManager.Find_Object_Type( "Galactic_Fleet" );
	bool is_fleet = (built_object->Get_Type() == fleet_type);

	if (is_fleet) 
	{
		built_object = built_object->Get_Fleet_Data()->Get_Starship_List()->Get_At(0);
	}

	AIPlayerClass *ai_player = player->Get_Object()->Get_AI_Player();
	if (ai_player)
	{
		AIFreeStoreClass *freestore = (*ai_player->Find_Tactical_Manager_By_Mode(GameModeManager.Get_Active_Mode()))->Get_Execution_System()->Get_Free_Store();
		freestore->Remove_Free_Store_Object(built_object);
	}

	LuaTable::Pointer table = Alloc_Lua_Table();
	table->Value.push_back(GameObjectWrapper::Create(built_object, script));
	return Return_Variable(table);

}

LuaTable *LuaCreateGenericObjectClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 3)
	{
		script->Script_Error("Create_Generic_Object -- Invalid number of parameters.  Expected 3, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> type_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	SmartPtr<GameObjectTypeWrapper> type_wrapper = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
	SmartPtr<PlayerWrapper> player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[2]);

	if (!player)
	{
		script->Script_Error("Create_Generic_Object -- Invalid type for parameter 3.  Expected player.");
		return NULL;
	}

	const GameObjectTypeClass *type = NULL;
	if (type_wrapper)
	{
		type = type_wrapper->Get_Object();
	}
	else if (type_name)
	{
		type = GameObjectTypeManager.Find_Object_Type(type_name->Value);
	}

	if (!type)
	{
		script->Script_Error("Create_Generic_Object -- Invalid type or unnown type name for parameter 1.");
		return NULL;
	}

	Vector3 create_position;
	if (!Lua_Extract_Position(params->Value[1], create_position))
	{
		script->Script_Error("Create_Generic_Object -- Could not deduce a creation position from parameter 2.");
		return NULL;
	}

	GameObjectClass *new_object = GAME_OBJECT_MANAGER.Create_Object_Of_Type(type, player->Get_Object()->Get_ID(), create_position, VECTOR3_NONE);
	if (!new_object)
	{
		script->Script_Error("Create_Generic_Object -- Internal game logic error creating %s.  Possibly check _logfile.txt.");
		return NULL;
	}

	return Return_Variable(GameObjectWrapper::Create(new_object, script));
}
