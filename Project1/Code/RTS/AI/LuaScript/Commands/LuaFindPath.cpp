// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaFindPath.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaFindPath.cpp $
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


#include "LuaFindPath.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "AI/AITargetLocation.h"
#include "GameObject.h"
#include "Player.h"
#include "AI/AIPlayer.h"
#include "AI/TacticalAIManager.h"
#include "AI/Execution/AIExecutionPath.h"
#include "AI/Execution/AIExecutionSystem.h"

PG_IMPLEMENT_RTTI(LuaFindPathClass, LuaUserVar);

LuaTable *LuaFindPathClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 3)
	{
		script->Script_Error("Find_Path -- invalid number of parameters.  Expected 3, got %d.", params->Value.size());
		return NULL;
	}

	if (GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_GALACTIC)
	{
		script->Script_Error("Find_Path -- this script function is only supported in galactic mode.");
		return NULL;
	}

	//Parameter 1 should be player
	SmartPtr<PlayerWrapper> player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!player || !player->Get_Object())
	{
		script->Script_Error("Find_Path -- invalid parameter type for parameter 1.  Expected player.");
		return NULL;
	}

	//Parameter 2 should be path origin
	GameObjectClass *from = 0;
	SmartPtr<AITargetLocationWrapper> from_ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[1]);
	SmartPtr<GameObjectWrapper> from_object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[1]);
	if (from_ai_target_wrapper && from_ai_target_wrapper->Get_Object())
	{
		from = from_ai_target_wrapper->Get_Object()->Get_Target_Game_Object();
	}
	else if (from_object_wrapper)
	{
		from = from_object_wrapper->Get_Object();
	}

	if (!from)
	{
		script->Script_Error("Find_Path -- invalid parameter for parameter 2.  This needs to be a valid planet (or wrapper round one)");
		return NULL;
	}

	//Parameter 3 should be path destination
	GameObjectClass *to = 0;
	SmartPtr<AITargetLocationWrapper> to_ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[2]);
	SmartPtr<GameObjectWrapper> to_object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[2]);
	if (to_ai_target_wrapper && to_ai_target_wrapper->Get_Object())
	{
		to = to_ai_target_wrapper->Get_Object()->Get_Target_Game_Object();
	}
	else if (to_object_wrapper)
	{
		to = to_object_wrapper->Get_Object();
	}

	if (!to)
	{
		script->Script_Error("Find_Path -- invalid parameter for parameter 3.  This needs to be a valid planet (or wrapper round one)");
		return NULL;
	}

	AIPlayerClass *ai_player = player->Get_Object()->Get_AI_Player();
	if (!ai_player)
	{
		script->Script_Error("Find_Path -- can't do this from the point of view of non-ai players");
		return NULL;
	}

	TacticalAIManagerClass *manager = ai_player->Get_Tactical_Manager_By_Mode(SUB_GAME_MODE_GALACTIC);
	FAIL_IF(!manager) { return NULL; }
	AIExecutionSystemClass *execution_system = manager->Get_Execution_System();
	FAIL_IF(!execution_system) { return NULL; }
	AIExecutionPathFinderClass *path_finder = execution_system->Get_Path_Finder();
	FAIL_IF(!path_finder) { return NULL; }
	const AIExecutionPathClass &path = path_finder->Find_Path(from, to);

	LuaTable *result = Alloc_Lua_Table();

	for (int i = 0; i < path.Get_Hop_Count(); ++i)
	{
		GameObjectWrapper *node_wrapper = GameObjectWrapper::Create(static_cast<GameObjectClass*>(path.Get_Hop(i)), script);
		result->Value.push_back(node_wrapper);
	}

	return Return_Variable(result);
}