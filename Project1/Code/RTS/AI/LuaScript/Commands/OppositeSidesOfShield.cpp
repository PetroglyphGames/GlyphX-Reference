// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/OppositeSidesOfShield.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/OppositeSidesOfShield.cpp $
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

#include "OppositeSidesOfShield.h"
#include "LandMode.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/GameObjectWrapper.h"

PG_IMPLEMENT_RTTI(OppositeSidesOfShieldClass, LuaUserVar);

LuaTable *OppositeSidesOfShieldClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 2 && params->Value.size() != 4)
	{
		script->Script_Error("Are_On_Opposite_Sides_Of_Shield -- invalid number of parameters.  Expected 2 or 4, got %d.", params->Value.size());
		return NULL;
	}

	Vector3 position_a;
	if (!Lua_Extract_Position(params->Value[0], position_a))
	{
		script->Script_Error("Are_On_Opposite_Sides_Of_Shield -- could not extract a position from parameter 1.");
		return NULL;
	}

	GameObjectClass *shooter = NULL;
	GameObjectClass *target = NULL;
	GameObjectWrapper *object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	if (object_wrapper)
	{
		shooter = object_wrapper->Get_Object();
	}
	object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[1]);
	if (object_wrapper)
	{
		target = object_wrapper->Get_Object();
	}

	Vector3 position_b;
	if (!Lua_Extract_Position(params->Value[1], position_b))
	{
		script->Script_Error("Are_On_Opposited_Sides_Of_Shield -- could not extract a position from parameter 2.");
		return NULL;
	}

	PlayerClass *player = NULL;
	bool friendly = false;
	if (params->Value.size() == 4)
	{
		SmartPtr<PlayerWrapper> lua_player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[2]);
		if (!lua_player)
		{
			script->Script_Error("Are_On_Opposite_Sides_Of_Shield -- invalid type for parameter 3.  Expected player.");
			return NULL;
		}

		SmartPtr<LuaBool> lua_friendly = PG_Dynamic_Cast<LuaBool>(params->Value[3]);
		if (!lua_friendly)
		{
			script->Script_Error("Are_On_Opposite_Sides_Of_Shield -- invalid type for parameter 4.  Expected boolean.");
			return NULL;
		}

		player = lua_player->Get_Object();
		friendly = lua_friendly->Value;
	}

	LandModeClass *land_mode = static_cast<LandModeClass*>(GameModeManager.Get_Game_Mode_By_Sub_Type(SUB_GAME_MODE_LAND));
	if (!land_mode)
	{
		script->Script_Error("Are_On_Opposite_Sides_Of_Shield -- this function is only valid in land mode.");
		return NULL;
	}

	return Return_Variable(new LuaBool(land_mode->Are_On_Opposite_Sides_Of_Shield(shooter, target, position_a, position_b, player, friendly)));
}