// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/CheckStoryFlag.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/CheckStoryFlag.cpp $
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

#include "CheckStoryFlag.h"

#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "AI/Perception/AIPerceptionSystem.h"
#include "AI/AITargetLocation.h"
#include "AI/AIPlayer.h"
#include "AI/TacticalAIManager.h"
#include "Player.h"

PG_IMPLEMENT_RTTI(CheckStoryFlagClass, LuaUserVar);

LuaTable *CheckStoryFlagClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 4)
	{
		script->Script_Error("Check_AI_Story_Flag -- invalid number of parameters.  Expected between 4, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<PlayerWrapper> lua_player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!lua_player)
	{
		script->Script_Error("Check_AI_Story_Flag -- invalid type for parameter 1.  Expected player");
		return NULL;
	}

	SmartPtr<LuaString> flag_name = PG_Dynamic_Cast<LuaString>(params->Value[1]);
	if (!flag_name)
	{
		script->Script_Error("Check_AI_Story_Flag -- invalid type for parameter 2.  Expected string");
		return NULL;
	}

	GameObjectClass *target = NULL;

	SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[2]);
	SmartPtr<AITargetLocationWrapper> ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[2]);
	if (object_wrapper)
	{
		target = object_wrapper->Get_Object();
		if (!target)
		{
			script->Script_Error("Check_AI_Story_Flag -- target specified is dead; unable to check flag.");
			return NULL;
		}
	}
	else if (ai_target_wrapper)
	{
		if (!ai_target_wrapper->Get_Object())
		{
			script->Script_Error("Check_AI_Story_Flag -- target specified is dead; unable to check flag.");
			return NULL;
		}

		target = ai_target_wrapper->Get_Object()->Get_Target_Game_Object();
		if (!target)
		{
			script->Script_Error("Check_AI_Story_Flag -- target %s is not a game object; unable to check story flag.", ai_target_wrapper->Get_Object()->Get_Name().c_str());
			return NULL;
		}
	}

	SmartPtr<LuaBool> lua_reset = PG_Dynamic_Cast<LuaBool>(params->Value[3]);
	if (!lua_reset)
	{
		script->Script_Error("Check_AI_Story_Flag -- invalid type for parameter 4; expected boolean.");
		return NULL;
	}

	TacticalAIManagerClass *tactical_manager = lua_player->Get_Object()->Get_AI_Player()->Get_Tactical_Manager_By_Mode(GameModeManager.Get_Sub_Type());
	if (!tactical_manager)
	{
		script->Script_Error("Check_AI_Story_Flag -- could not locate tactical AI manager.");
		return NULL;
	}

	AIPerceptionSystemClass *perception_system = tactical_manager->Get_Perception_System();

	bool flag_set = perception_system->Get_AI_Story_Arc_Trigger(flag_name->Value, target);

	if (flag_set && lua_reset->Value)
	{
		perception_system->Set_AI_Story_Arc_Trigger(flag_name->Value, target, false);
	}

	return Return_Variable(new LuaBool(flag_set));
}