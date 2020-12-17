// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/EvaluateGalacticContext.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/EvaluateGalacticContext.cpp $
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


#include "EvaluateGalacticContext.h"

#include "AI/Perception/AIPerceptionSystem.h"
#include "AI/TacticalAIManager.h"
#include "AI/AIPlayer.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "Player.h"
#include "PerceptionFunction.h"
#include "ThePerceptionFunctionManager.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "GameObject.h"
#include "GameObjectType.h"
#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "AI/AITargetLocation.h"
#include "AI/Perception/PerceptionContext.h"
#include "GameModeManager.h"
#include "AI/AIPlayerType.h"
#include "GameObjectManager.h"

PG_IMPLEMENT_RTTI(EvaluateGalacticContextClass, LuaUserVar);

/**************************************************************************************************
* EvaluateGalacticContextClass::Function_Call - Evaluate the given perception in the context of the
*	parent galactic mode.  Only valid in tactical battles spawned from galactic.  The perception target 
*	is mapped to the planet at which the current tactical is taking place
*
* In:		script
*			params --	(1st) Name of perception function to evaluate
*							(2nd) Player for which this evaluation is taking place
*
* Out:	Result of evaluation
*
* History: 3/14/2005 5:50 PM -- JSY
**************************************************************************************************/
LuaTable *EvaluateGalacticContextClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (!GameModeManager.Get_Parent_Game_Mode(GameModeManager.Get_Active_Mode()))
	{
		script->Script_Warning("Evaluate_In_Galactic_Context: This function may only be used in tactical games spawned from campaign mode.");
		return NULL;
	}

	//Verify all the parameters.
	if (params->Value.size() != 2)
	{
		script->Script_Error("Evaluate_In_Galactic_Context: Invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	LuaString *perception_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!perception_name)
	{
		script->Script_Error("Evaluate_In_Galactic_Context: Invalid parameter type for parameter 1. Expected string.");
		return NULL;
	}

	PerceptionFunctionClass *function = ThePerceptionFunctionManagerPtr->Get_Managed_Object(perception_name->Value);
	if (!function)
	{
		script->Script_Error("Evaluate_In_Galactic_Context: Unrecognised perception function %s.", perception_name->Value.c_str());
		return NULL;
	}

	SmartPtr<PlayerWrapper> player = LUA_SAFE_CAST(PlayerWrapper, params->Value[1]);
	if (!player)
	{
		script->Script_Error("Evaluate_In_Galactic_Context: Invalid parameter type for parameter 2.  Expected player object.");
		return NULL;
	}

	AIPlayerClass *ai_player = player->Get_Object()->Get_AI_Player();
	TacticalAIManagerClass *tactical_manager = ai_player->Get_Tactical_Manager_By_Mode(SUB_GAME_MODE_GALACTIC);
	if (!tactical_manager)
	{
		script->Script_Error("Evaluate_In_Galactic_Context: AI player %s has no galactic AI.", ai_player->Get_AI_Player_Type()->Get_Name().c_str());
		return NULL;
	}
	AIPerceptionSystemClass *perception_system = tactical_manager->Get_Perception_System();

	//Set up the context depending on exactly what we got passed
	PerceptionContextClass context(perception_system);
	context.Add_Context(PERCEPTION_TOKEN_VARIABLE_SELF, player->Get_Object()->Get_Evaluator_Token());
	if (perception_system->Get_Manager()->Get_Enemy())
	{
		context.Add_Context(PERCEPTION_TOKEN_VARIABLE_ENEMY, perception_system->Get_Manager()->Get_Enemy()->Get_Evaluator_Token());
	}
	PlayerClass *human = PlayerList.Get_Human_Player();
	if (human)
	{
		context.Add_Context(PERCEPTION_TOKEN_VARIABLE_HUMAN, human->Get_Evaluator_Token());
	}

	ConflictInfoStruct *conflict = GameModeManager.Get_Current_Conflict_Info();
	FAIL_IF(!conflict) { return NULL; }
	const GameObjectClass *planet = GameModeManager.Get_Parent_Mode_Object_By_ID(GameModeManager.Get_Active_Mode(), conflict->LocationID);

	context.Add_Context(PERCEPTION_TOKEN_VARIABLE_TARGET, planet->Get_Planetary_Data()->Get_Evaluator_Token());

	context.Set_Player(player->Get_Object());

	float result = 0.0;
	if (!function->Evaluate(context, result))
	{
		script->Script_Error("Evaluate_Perception: Error evaluating perception function %s.", function->Get_Name().c_str());
		return NULL;
	}

	return Return_Variable(new LuaNumber(static_cast<float>(result)));
}