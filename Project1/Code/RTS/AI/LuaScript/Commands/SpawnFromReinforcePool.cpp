// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpawnFromReinforcePool.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpawnFromReinforcePool.cpp $
//
//    Original Author: James Yarrow
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

#include "SpawnFromReinforcePool.h"

#include "AI/LuaScript/GameObjectTypeWrapper.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "Player.h"
#include "GameObjectManager.h"
#include "GameModeManager.h"

PG_IMPLEMENT_RTTI(SpawnFromReinforcePoolClass, LuaUserVar);

/**************************************************************************************************
* SpawnFromReinforcePoolClass::Function_Call -- Spawn an object directly from a player's reinforcement
*	pool.  The reinforcement is used up and, where relevant, the tactical object is connected to the parent
*	mode instance.  We skip the whole landing sequence deal though.
*
* In:				
*
* Out:	
*
* History: 7/13/2006 4:55PM JSY
**************************************************************************************************/
LuaTable *SpawnFromReinforcePoolClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 3)
	{
		script->Script_Error("Spawn_From_Reinforcement_Pool -- Invalid number of parameters.  Expected 3, got %d", params->Value.size());
		return NULL;
	}

	//Extract parameters
	SmartPtr<GameObjectTypeWrapper> type_wrapper = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
	SmartPtr<PlayerWrapper> player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[2]);

	if (!player || !player->Get_Object())
	{
		script->Script_Error("Spawn_From_Reinforcement_Pool -- Invalid parameter for player.");
		return NULL;
	}

	if (!type_wrapper)
	{
		script->Script_Error("Spawn_From_Reinforcement_Pool -- Invalid parameter for type.");
		return NULL;
	}

	Vector3 land_pos = VECTOR3_INVALID;
	Lua_Extract_Position(params->Value[1], land_pos);
	if (land_pos == VECTOR3_INVALID)
	{
		script->Script_Error("Spawn_From_Reinforcement_Pool -- Invalid position parameter.");
		return NULL;
	}

	//Make sure there's a reinforcement of the requested type available
	GameModeClass *this_mode = GameModeManager.Get_Active_Mode();
	GameModeClass *parent_mode = GameModeManager.Get_Parent_Game_Mode(this_mode);
	int link_to_parent_mode_id = INVALID_OBJECT_ID;
	if (parent_mode)
	{
		//Find an object of the given type in the parent mode reinforcement list
		ObjectPersistenceClass *persist_objects = GameModeManager.Get_Parent_Mode_Persistent_Objects(this_mode);
		FAIL_IF(!persist_objects) { return NULL; }
		DynamicVectorClass<ObjectPersistenceClass::PersistentUnit> &reinforcements = persist_objects->Get_Reinforcements(player->Get_Object()->Get_ID());
		for (int i = 0; i < reinforcements.Size(); ++i)
		{
			int reinforcement_parent_id = reinforcements[i].ObjectID;
			if (reinforcement_parent_id == INVALID_OBJECT_ID)
			{
				continue;
			}

			GameObjectClass *reinforcement_parent_object = GALACTIC_GAME_OBJECT_MANAGER.Get_Object_From_ID(reinforcement_parent_id);
			if (!reinforcement_parent_object)
			{
				continue;
			}

			//Claim the reinforcement
			if (reinforcement_parent_object->Get_Original_Object_Type() == type_wrapper->Get_Object())
			{
				link_to_parent_mode_id = reinforcement_parent_id;
				reinforcements[i].ObjectID = INVALID_OBJECT_ID;
				break;
			}
		}

		//Didn't find a match?
		if (link_to_parent_mode_id == INVALID_OBJECT_ID)
		{
			script->Script_Warning("Spawn_From_Reinforcement_Pool -- player has no objects of type %s in reinforcement pool.", type_wrapper->Get_Object()->Get_Name()->c_str());
			return NULL;
		}
	}
	else
	{
		//Skirmish is easy!
		int reinf_index = player->Get_Object()->Get_Skirmish_Reinforcement_Index(type_wrapper->Get_Object());
		if (reinf_index == -1)
		{
			script->Script_Warning("Spawn_From_Reinforcement_Pool -- player has no objects of type %s in reinforcement pool.", type_wrapper->Get_Object()->Get_Name()->c_str());
			return NULL;
		}

		player->Get_Object()->Remove_Skirmish_Reinforcement(reinf_index);
	}

	//Create the object.  Since this is pulled from the reinforcement pool it should be tracked for unit cap purposes
	DynamicVectorClass<GameObjectClass*> squad_objects(true);
	FacingClass face(land_pos, VECTOR3_NONE);
	GameModeManager.Get_Active_Mode()->Get_Object_Manager().Create_And_Place_Company(type_wrapper->Get_Object(), land_pos, *face.Get(), 
																												player->Get_Object()->Get_ID(), NULL, &squad_objects, true);
	LuaTable::Pointer table = Alloc_Lua_Table();
	for (int i = 0; i < squad_objects.Get_Count(); i++)
	{
		//Connect to parent mode transport if relevant
		if (link_to_parent_mode_id != INVALID_OBJECT_ID)
		{
			squad_objects[i]->Set_Parent_Mode_ID(link_to_parent_mode_id);
		}
		table->Value.push_back(GameObjectWrapper::Create(squad_objects[i], script));
	}
	return Return_Variable(table);	
}