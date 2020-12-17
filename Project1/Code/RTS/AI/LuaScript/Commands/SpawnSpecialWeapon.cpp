// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpawnSpecialWeapon.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpawnSpecialWeapon.cpp $
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

#include "Always.h"

#include "SpawnSpecialWeapon.h"

#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "GameModeManager.h"
#include "GameObjectManager.h"
#include "GameObjectTypeManager.h"

PG_IMPLEMENT_RTTI(SpawnSpecialWeaponClass, LuaUserVar);

/**************************************************************************************************
* SpawnSpecialWeaponClass::Function_Call -- Script command to spawn in a special weapon of a given type under
*	the ownership of a given player
*
* In:			
*
* Out:		
*
* History: 7/28/2006 8:12PM -- JSY
**************************************************************************************************/
LuaTable *SpawnSpecialWeaponClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_SPACE)
	{
		script->Script_Error("Spawn_Special_Weapon -- this command may only be used in space mode.");
		return NULL;
	}

	if (params->Value.size() != 2)
	{
		script->Script_Error("Spawn_Special_Weapon -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	//First parameter is weapon type
	SmartPtr<LuaString> weapon_type_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!weapon_type_name)
	{
		script->Script_Error("Spawn_Special_Weapon -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	//Make sure this object exists and is a usable special weapon
	GameObjectTypeClass *weapon_type = GameObjectTypeManager.Find_Object_Type(weapon_type_name->Value);
	if (!weapon_type)
	{
		script->Script_Error("Spawn_Special_Weapon -- request to spawn weapon of unknown  type %s.", weapon_type_name->Value.c_str());
		return NULL;
	}

	if (!weapon_type->Is_Special_Weapon_In_Space())
	{
		script->Script_Error("Spawn_Special_Weapon -- request to spawn weapon of type %s which is not a valid space special weapon type.", weapon_type_name->Value.c_str());
		return NULL;
	}

	if (weapon_type->Get_Special_Weapon_Index() < 0)
	{
		script->Script_Error("Spawn_Special_Weapon -- weapon type %s has invalid weapon index %d.  Weapon will not be usable, spawn aborted.", weapon_type_name->Value.c_str(), weapon_type->Get_Special_Weapon_Index());
		return NULL;
	}

	//Second parameter is player who will control the special weapon
	SmartPtr<PlayerWrapper> player_wrapper = PG_Dynamic_Cast<PlayerWrapper>(params->Value[1]);
	if (!player_wrapper)
	{
		script->Script_Error("Spawn_Special_Weapon -- invalid type for parameter 2.  Expected player.");
		return NULL;
	}

	PlayerClass *player = player_wrapper->Get_Object();
	if (!player)
	{
		script->Script_Error("Spawn_Special_Weapon -- attempt to spawn weapon for unknown player who has been removed from the game.");
		return NULL;
	}

	//The logic for actually creating and hooking up the special weapon is borrowed from GameObjectManagerClass::Create_Starting_Forces

	// Figure out where to place this special weapon. There needs to be an object saved in the space map
	// that has the "Special Weapon Source" behavior on it (typically the main planet).
	const DynamicVectorClass<GameObjectClass*> *source_objects = GAME_OBJECT_MANAGER.Find_Objects(BEHAVIOR_DUMMY_SPECIAL_WEAPON_SOURCE);
	if (source_objects->Size() == 0)
	{
		script->Script_Error("Spawn_Special_Weapon -- unable to find an object in this map with SPECIAL_WEAPON_SOURCE behavior.  Cannot place special weapon.");
		return NULL;
	}
	
	GameObjectClass *source_object = source_objects->Get_At(0);
	FAIL_IF(!source_object) { return NULL; }
	Vector3 position = source_object->Get_Position();
	Vector3 facing = source_object->Get_Facing_Vector();

	// Create the special weapon and place it at the center of the planet.
	GameObjectClass *new_structure = GAME_OBJECT_MANAGER.Create_Object_Of_Type(weapon_type, player->Get_ID(), position, facing);
	FAIL_IF(!new_structure) { return NULL; }

	// Associate this new object with a special weapon button on the tactical GUI.
	// Being associated through a special weapon index allows the button click to trigger this object.
	GameModeManager.Get_Active_Mode()->Add_Special_Weapon(new_structure);

	//Return the new special weapon to script
	return Return_Variable(GameObjectWrapper::Create(new_structure, script));								
}