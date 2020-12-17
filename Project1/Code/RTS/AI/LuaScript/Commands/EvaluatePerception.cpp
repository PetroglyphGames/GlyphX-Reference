// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/EvaluatePerception.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/EvaluatePerception.cpp $
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


#include "EvaluatePerception.h"

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

PG_IMPLEMENT_RTTI(EvaluatePerceptionClass, LuaUserVar);

/**************************************************************************************************
* EvaluatePerceptionClass::Function_Call
*
* In:		script
*			params --	(1st) Name of perception function to evaluate
*							(2nd) Player for which this evaluation is taking place
*							(3rd) AI target location or game object representing a planet or NULL
*
* Out:	Result of evaluation
*
* History: 6/30/2004 9:28 AM -- JSY
**************************************************************************************************/
LuaTable *EvaluatePerceptionClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	//Verify all the parameters.
	if (params->Value.size() != 2 && params->Value.size() != 3)
	{
		script->Script_Error("Evaluate_Perception: Invalid number of parameters.  Expected 2 or 3, got %d.", params->Value.size());
		return NULL;
	}

	LuaString *perception_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!perception_name)
	{
		script->Script_Error("Evaluate_Perception: Invalid parameter type for parameter 1. Expected string.");
		return NULL;
	}

	PerceptionFunctionClass *function = ThePerceptionFunctionManagerPtr->Get_Managed_Object(perception_name->Value);
	if (!function)
	{
		script->Script_Error("Evaluate_Perception: Unrecognised perception function %s.", perception_name->Value.c_str());
		return NULL;
	}

	SmartPtr<PlayerWrapper> player = LUA_SAFE_CAST(PlayerWrapper, params->Value[1]);
	if (!player)
	{
		script->Script_Error("Evaluate_Perception: Invalid parameter type for parameter 2.  Expected player object.");
		return NULL;
	}

	//Third parameter could be either an AI target location or a game object.
	SmartPtr<AITargetLocationWrapper> target_wrapper = 0;
	SmartPtr<GameObjectWrapper> object_wrapper = 0;
	if (params->Value.size() == 3)
	{
		target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[2]);
		object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[2]);

		if (!target_wrapper && !object_wrapper)
		{
			script->Script_Error("Evaluate_Perception: Invalid parameter type for parameter 3.  Expected AI target location or game object.");
			return NULL;
		}

		if ((target_wrapper && !target_wrapper->Get_Object()) || (object_wrapper && !object_wrapper->Get_Object()) )
		{
			script->Script_Error("Evaluate_Perception: target is already dead.");
			return NULL;
		}
	}

	AIPlayerClass *ai_player = player->Get_Object()->Get_AI_Player();
	if (!ai_player)
	{
		script->Script_Error("Evaluate_Perception: Cannot evaluate perceptions for players with no AI.");
		return NULL;		
	}

	TacticalAIManagerClass *tactical_manager = ai_player->Get_Tactical_Manager_By_Mode(GameModeManager.Get_Active_Mode()->Get_Sub_Type());
	if (!tactical_manager || !tactical_manager->Get_Are_Systems_Initialized())
	{
		script->Script_Warning("Evaluate_Perception: Cannot evaluate perception - AI not yet initialized.  Ask again later.");
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
	if (target_wrapper)
	{
		context.Add_Context(PERCEPTION_TOKEN_VARIABLE_TARGET, target_wrapper->Get_Object()->Get_Evaluator());
		context.Set_Target(target_wrapper->Get_Object());
	}
	else if (object_wrapper)
	{
		if (object_wrapper->Get_Object()->Behaves_Like(BEHAVIOR_PLANET))
		{
			context.Add_Context(PERCEPTION_TOKEN_VARIABLE_TARGET, object_wrapper->Get_Object()->Get_Planetary_Data()->Get_Evaluator_Token());
		}
		else
		{
			context.Add_Context(PERCEPTION_TOKEN_VARIABLE_TARGET, AIPerceptionSystemClass::Build_Game_Object_Token_String(object_wrapper->Get_Object()));
		}
	}

	context.Set_Player(player->Get_Object());

	float result = 0.0;
	if (!function->Evaluate(context, result))
	{
		script->Script_Error("Evaluate_Perception: Error evaluating perception function %s.", function->Get_Name().c_str());
		return NULL;
	}

	return Return_Variable(new LuaNumber(static_cast<float>(result)));
}