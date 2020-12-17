// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/PurgeGoals.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/PurgeGoals.cpp $
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

#pragma hdrstop


#include "PurgeGoals.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "Player.h"
#include "AI/AIPlayer.h"
#include "AI/TacticalAIManager.h"
#include "AI/Planning/AIPlanningSystem.h"
#include "AI/Planning/PlanBehavior.h"
#include "GameModeManager.h"

PG_IMPLEMENT_RTTI(PurgeGoalsClass, LuaUserVar);

/**************************************************************************************************
* PurgeGoalsClass::Function_Call -- abandon all the given player's goals (except the one running the plan that
*												invoked this call.
*
* In:				
*
* Out:		Always 0	
*
* History: 11/11/2004 1:34PM JSY
**************************************************************************************************/
LuaTable *PurgeGoalsClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("Purge_Goals -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return 0;
	}

	SmartPtr<PlayerWrapper> lua_player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!lua_player)
	{
		script->Script_Error("Purge_Goals -- invalid type for parameter 1.  Expected a player.");
		return 0;
	}

	PlayerClass *player = lua_player->Get_Object();
	if (!player)
	{
		return 0;
	}

	AIPlayerClass *ai_player = player->Get_AI_Player();
	if (!ai_player)
	{
		script->Script_Error("Purge_Goals -- player does not appear to be AI.");
		return 0;
	}

	//Turns out it's probably neater and easier to purge plans rather than goals...
	TacticalAIManagerClass *manager = ai_player->Get_Tactical_Manager_By_Mode(GameModeManager.Get_Sub_Type());
	if (!manager)
	{
		script->Script_Error("Purge_Goals -- AI player has no tactical manager for current mode.  How???");
		return 0;
	}
	AIPlanningSystemClass *planning_system = manager->Get_Planning_System();
	if (!planning_system)
	{
		script->Script_Error("Purge_Goals -- AI player has no planning system for current mode.  Oh dear.");
		return 0;
	}

	//The script pointer I have should identify which plan is in fact me.
	ReferenceListIterator<PlanBehaviorClass> it(&planning_system->Get_Active_Plans());
	while (!it.Is_Done())
	{
		PlanBehaviorClass *plan = it.Current_Object();
		if (!plan)
		{
			it.Next();
			continue;
		}

		if (plan->Get_Script() == script)
		{
			it.Next();
			continue;
		}

		if (!plan->Is_Goal_System_Removable())
		{
			it.Next();
			continue;
		}

		plan->Abandon_Plan();
		it.Remove_Current_Object();
	}

	return 0;
}