// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindTarget.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindTarget.cpp $
//
//    Original Author: James Yarrow
//
//            $Author: Brian_Hayes $
//
//            $Change: 641502 $
//
//          $DateTime: 2017/05/09 13:45:27 $
//
//          $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */

#pragma hdrstop
#include "FindTarget.h"

#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "AI/AIPlayer.h"
#include "AI/TacticalAIManager.h"
#include "AI/AITargetLocation.h"
#include "AI/Perception/AIPerceptionSystem.h"
#include "AI/Perception/PerceptionContext.h"
#include "PerceptionFunction.h"
#include "AI/Planning/AIPlanningSystem.h"
#include "AI/Planning/TaskForce.h"
#include "AI/Planning/PlanBehavior.h"
#include "AI/Planning/Reachability.h"
#include "GameModeManager.h"
#include "GameObject.h"
#include "GameObjectType.h"
#include "DiscreteDistribution.h"
#include "ThePerceptionFunctionManager.h"
#include "EnumConversion.h"
#include "AI/Goal/AIGoalReachabilityType.h"
#include "AI/Goal/AIGoalSystem.h"

PG_IMPLEMENT_RTTI(FindTargetClass, LuaUserVar);


FindTargetClass::FindTargetClass()
{
	LUA_REGISTER_MEMBER_FUNCTION(FindTargetClass, "Reachable_Target", &FindTargetClass::Reachable_Target);
	LUA_REGISTER_MEMBER_FUNCTION(FindTargetClass, "Best_Of", &FindTargetClass::Best_Of);
}

LuaTable *FindTargetClass::Reachable_Target(LuaScriptClass *script, LuaTable *params)
{
	const PerceptionFunctionClass *function;
	AIGoalApplicationType application_type;
	AIGoalReachabilityType reachability_type;
	float best_choice_scale_factor;
	float max_distance;

	// param 1: playerwrapper.
	// param 2: perception function name
	// param 3: goal application type string
	// param 4: reachability type string
	// param 5: The probability of selecting the target with highest desire
	// param 6: The source from which the find target should search for relative targets.
	// param 7: The maximum direct from source to target.
	if (params->Value.size() < 4 || params->Value.size() > 7)
	{
		script->Script_Error("FindTarget -- Invalid number of parameters: expected between 4 and 7, got %d.", params->Value.size());
		return NULL;
	}

	//Debug_Printf("<><><><><> FindTargetClass called from %s\n",script->Get_Name().c_str());

	//First parameter should be a task force belonging to an AI player
	SmartPtr<PlayerWrapper> player = LUA_SAFE_CAST(PlayerWrapper, params->Value[0]);
	if (!player || !player->Get_Object() || !player->Get_Object()->Get_AI_Player())
	{
		script->Script_Error("FindTarget -- Parameter 1 is not a valid AI player.");
		return NULL;
	}

	//Second parameter should be the name of the perception function used to pick a target
	LuaString *function_name = PG_Dynamic_Cast<LuaString>(params->Value[1]);
	if (!function_name)
	{
		script->Script_Error("FindTarget -- Parameter 2 is not a valid string");
		return NULL;
	}

	function = ThePerceptionFunctionManagerPtr->Get_Managed_Object(function_name->Value);
	if (!function)
	{
		script->Script_Error("FindTarget -- unrecognized perception function %s.", function_name->Value.c_str());
		return NULL;
	}

	//third parameter should be goal application type
	LuaString *application_name = PG_Dynamic_Cast<LuaString>(params->Value[2]);
	if (!application_name)
	{
		script->Script_Error("FindTarget -- Parameter 3 is not a valid string.");
		return NULL;
	}

	if (!TheAIGoalApplicationTypeConverterPtr->String_To_Enum(application_name->Value, application_type))
	{
		script->Script_Error("FindTarget -- unrecognized goal application %s.", application_name->Value.c_str());
		return NULL;
	}

	if (application_type == AI_GOAL_APPLICATION_TYPE_GLOBAL)
	{
		script->Script_Error("FindTarget -- can't find a target for a global goal!");
		return NULL;
	}

	//fourth parameter should be the reachability type.
	LuaString *reachability_name = PG_Dynamic_Cast<LuaString>(params->Value[3]);
	if (!reachability_name)
	{
		script->Script_Error("FindTarget -- Parameter 4 is not a valid string.");
		return NULL;
	}

	if (!TheAIGoalReachabilityTypeConverterPtr->String_To_Enum(reachability_name->Value, reachability_type))
	{
		script->Script_Error("FindTarget -- unrecognized goal reachability %s.", reachability_name->Value.c_str());
		return NULL;
	}

	if (reachability_type == GOAL_REACHABILITY_INVALID)
	{
		script->Script_Error("FindTarget -- can't find a target for a global goal!");
		return NULL;
	}

	if (params->Value.size() == 4)
	{
		best_choice_scale_factor = 0.0;
	}
	else
	{
		//Fifth parameter is probability of selecting best target
		LuaNumber *lua_prob_best = PG_Dynamic_Cast<LuaNumber>(params->Value[4]);
		if (!lua_prob_best)
		{
			script->Script_Error("FindTarget -- fifth parameter is not a valid number.");
			return NULL;
		}

		if (lua_prob_best->Value <= 0.0f || lua_prob_best->Value > 1.0f)
		{
			script->Script_Error("FindTarget -- Probability of selecting best target is not in (0,1].");
			return NULL;
		}

		best_choice_scale_factor = 1.0f / lua_prob_best->Value - 1.0f;
	}

	SubGameModeType mode = SUB_GAME_MODE_INVALID;
	switch (reachability_type)
	{
	case GOAL_REACHABILITY_FRIENDLY_ONLY:			
	case GOAL_REACHABILITY_FRIENDLY_IGNORE_THREAT:
	case GOAL_REACHABILITY_ENEMY_DESTINATION:	
	case GOAL_REACHABILITY_ENEMY_UNDEFENDED:		
	case GOAL_REACHABILITY_SINGLE_HOP_DISCONNECTED:
	case GOAL_REACHABILITY_ANY:						
		mode = SUB_GAME_MODE_GALACTIC;
		break;

	case GOAL_REACHABILITY_NO_THREAT:
	case GOAL_REACHABILITY_LOW_THREAT:
	case GOAL_REACHABILITY_MEDIUM_THREAT:
	case GOAL_REACHABILITY_HIGH_THREAT:
	case GOAL_REACHABILITY_ANY_THREAT:
		mode = GameModeManager.Get_Sub_Type();
		break;
	};

	TacticalAIManagerClass *tactical_manager = player->Get_Object()->Get_AI_Player()->Get_Tactical_Manager_By_Mode(mode);
	if (!tactical_manager)
	{
		script->Script_Error("FindTarget -- could not locate tactical AI manager.");
		return NULL;
	}

	//Sixth parameter is the source of the find target
	AITargetLocationClass *source = NULL;
	if (params->Value.size() > 5)
	{
		AITargetLocationWrapper *ai_target = LUA_SAFE_CAST(AITargetLocationWrapper, params->Value[5]);
		if (!ai_target || !ai_target->Get_Object())
		{
			GameObjectWrapper *wrapper = LUA_SAFE_CAST(GameObjectWrapper, params->Value[5]);
			if (!wrapper || !wrapper->Get_Object())
			{
				script->Script_Error("FindTarget -- Parameter 6 is not a valid AITargetLocationWrapper.");
				return NULL;
			}
			source = tactical_manager->Get_Goal_System()->Find_Target(wrapper->Get_Object());
		}
		else
		{
			source = const_cast<AITargetLocationClass *>(ai_target->Get_Object());
		}
	}

	if (params->Value.size() < 7)
	{
		max_distance = 0.0;
	}
	else
	{
		//Seventh parameter is maximum distance to target.  If this is not set then distance will not be considered.
		LuaNumber *lua_max_distance = PG_Dynamic_Cast<LuaNumber>(params->Value[6]);
		if (!lua_max_distance)
		{
			script->Script_Error("FindTarget -- Seventh parameter is not a valid number.");
			return NULL;
		}
		max_distance = lua_max_distance->Value;
	}

	max_distance *= max_distance;
	AIPerceptionSystemClass *perception_system = tactical_manager->Get_Perception_System();

	//Track the targets that meet requirements

	float best_desire = 0.0f;
	const AITargetLocationClass *best_target = 0;
	DiscreteDistributionClass<const AITargetLocationClass *> good_targets;

	// get the list of goal targets
	const AIPerceptionSystemClass::GoalTargetVectorType &target_vector = perception_system->Get_Goal_Target_List();

	PerceptionTokenType self_token = player->Get_Object()->Get_Evaluator_Token();
	PerceptionTokenType enemy_token = PERCEPTION_TOKEN_INVALID;
	PerceptionTokenType human_token = PERCEPTION_TOKEN_INVALID;
	if (perception_system->Get_Manager()->Get_Enemy())
	{
		enemy_token = perception_system->Get_Manager()->Get_Enemy()->Get_Evaluator_Token();
	}
	PlayerClass *human = PlayerList.Get_Human_Player();
	if (human)
	{
		human_token = human->Get_Evaluator_Token();
	}

	for (unsigned int i = 0; i < target_vector.size(); ++i)
	{
		const AITargetLocationClass *target = target_vector[i];
		assert(target);

		// does the goal type of this function match the target's properties?
		if (!target->Matches_Application_Type(application_type, player->Get_Object()->Get_AI_Player()))
			continue;

		// Test target reachability...
		if (target->Get_Target_Reachability() && source)
		{
			if (target->Get_Target_Reachability()->Get_Cost_To_Target(source, reachability_type, NULL) == BIG_FLOAT)
				continue;
		}

		// perceptual setup for evaluation
		PerceptionContextClass perception_context(perception_system);
		perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_SELF, self_token);
		if (enemy_token != PERCEPTION_TOKEN_INVALID)
		{
			perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_ENEMY, enemy_token);
		}
		if (human_token != PERCEPTION_TOKEN_INVALID)
		{
			perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_HUMAN, human_token);
		}
		perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_TARGET, target->Get_Evaluator());
		perception_context.Set_Target(target);
		perception_context.Set_Player(player->Get_Object());

		// compute a desire value for this (goal, location) pair
		float desire = 0;

		if (!function->Evaluate(perception_context, desire))
			continue;

		if (max_distance > 0)
		{
			if ((source->Get_Target_Position() - target->Get_Target_Position()).Length2() > max_distance)
			{
				continue;
			}
		}

		// Always track the best option.  Also hang on to
		// other possible targets that meet the minimum desire requirement.
		if (desire > best_desire)
		{
			good_targets.Add_Element(best_target, best_desire);
			best_target = target;
			best_desire = desire;
		}
		else
		{
			good_targets.Add_Element(target, desire);
		}
	}

	if (good_targets.Num_Elements() > 0 && best_choice_scale_factor != 0.0)
	{
		// More than one target met the requirements.  Add in the best target to the mix
		// with appropriately skewed probability and smaple.
		good_targets.Add_Element(best_target, good_targets.Get_Total_Weight() / best_choice_scale_factor);
		best_target = good_targets.Sample();

		return Return_Variable(AITargetLocationWrapper::Create(best_target, script));
	}
	else if (best_target)
	{
		// 1 or 0 targets met the requirement, or else we were explicitly
		// asked to only ever select the best.
		return Return_Variable(AITargetLocationWrapper::Create(best_target, script));
	}
	else
	{
		return NULL;
	}
}

/**
 * Finds a target
 * 
 * @param script		The LuaScript object we're running
 * @param params		Lua parameters to the function.  These are:
 *							- The taskforce searching for a target
 *							- The perception function name
 *							- The desire level required for consideration as a possible target
 *							- The probability of selecting the target with highest desire
 * @since 5/18/2004 11:40AM -- JSY
 */
LuaTable *FindTargetClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	TaskForceClass *taskforce = 0;
	PerceptionFunctionClass *function = 0;
	AIGoalApplicationType application_type;
	float best_choice_scale_factor;
	float max_distance;

	//Check the parameters and extract the C++ values.
	bool result = Validate_Call(script, params, taskforce, 
									function, application_type, 
										best_choice_scale_factor,
										max_distance);

	if (!result)
		return NULL;

	max_distance *= max_distance;
	TacticalAIManagerClass *tactical_manager = taskforce->Get_Plan()->Get_Planning_System()->Get_Manager();
	AIPerceptionSystemClass *perception_system = tactical_manager->Get_Perception_System();

	//Track the targets that meet requirements

	float best_desire = 0.0f;
	const AITargetLocationClass *best_target = 0;
	DiscreteDistributionClass<const AITargetLocationClass *> good_targets;

	// get the list of goal targets
	const AIPerceptionSystemClass::GoalTargetVectorType &target_vector = perception_system->Get_Goal_Target_List();

	PlayerClass *self = taskforce->Get_Plan()->Get_Player();
	PerceptionTokenType self_token = self->Get_Evaluator_Token();
	PerceptionTokenType enemy_token = PERCEPTION_TOKEN_INVALID;
	PerceptionTokenType human_token = PERCEPTION_TOKEN_INVALID;
	if (perception_system->Get_Manager()->Get_Enemy())
	{
		enemy_token = perception_system->Get_Manager()->Get_Enemy()->Get_Evaluator_Token();
	}
	PlayerClass *human = PlayerList.Get_Human_Player();
	if (human)
	{
		human_token = human->Get_Evaluator_Token();
	}

	for (unsigned int i = 0; i < target_vector.size(); ++i)
	{
		const AITargetLocationClass *target = target_vector[i];
		assert(target);
		
		// does the goal type of this function match the target's properties?
		if (!target->Matches_Application_Type(application_type, taskforce->Get_Plan()->Get_Player()->Get_AI_Player()))
			continue;
		
		// perceptual setup for evaluation
		PerceptionContextClass perception_context(perception_system);
		perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_SELF, self_token);
		if (enemy_token != PERCEPTION_TOKEN_INVALID)
		{
			perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_ENEMY, enemy_token);
		}
		if (human_token != PERCEPTION_TOKEN_INVALID)
		{
			perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_HUMAN, human_token);
		}
		perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_TARGET, target->Get_Evaluator());

		perception_context.Set_Target(target);
		perception_context.Set_Player(self);
		
		// compute a desire value for this (goal, location) pair
		float desire = 0;
		
		if (!function->Evaluate(perception_context, desire))
			continue;

		if (max_distance > 0)
		{
			if ((taskforce->Get_Position() - target->Get_Target_Position()).Length2() > max_distance)
			{
				continue;
			}
		}

		// Always track the best option.  Also hang on to
		// other possible targets that meet the minimum desire requirement.
		if (desire > best_desire)
		{
			good_targets.Add_Element(best_target, best_desire);
			best_target = target;
			best_desire = desire;
		}
		else
		{
			good_targets.Add_Element(target, desire);
		}
	}

	if (good_targets.Num_Elements() > 0 && best_choice_scale_factor != 0.0)
	{
		// More than one target met the requirements.  Add in the best target to the mix
		// with appropriately skewed probability and smaple.
		good_targets.Add_Element(best_target, good_targets.Get_Total_Weight() / best_choice_scale_factor);
		best_target = good_targets.Sample();

		if (best_target->Get_Target_Game_Object())
		{
			return Return_Variable(GameObjectWrapper::Create(best_target->Get_Target_Game_Object(), script));
		}
		else
		{
			return Return_Variable(AITargetLocationWrapper::Create(best_target, script));
		}
	}
	else if (best_target)
	{
		// 1 or 0 targets met the requirement, or else we were explicitly
		// asked to only ever select the best.
		if (best_target->Get_Target_Game_Object())
		{
			return Return_Variable(GameObjectWrapper::Create(best_target->Get_Target_Game_Object(), script));
		}
		else
		{
			return Return_Variable(AITargetLocationWrapper::Create(best_target, script));
		}
	}
	else
	{
		return NULL;
	}
}

bool FindTargetClass::Validate_Call(LuaScriptClass *script, 
									LuaTable *params, 
									TaskForceClass *&taskforce,
									PerceptionFunctionClass *&function, 
									AIGoalApplicationType &application_type,
									float &best_choice_scale_factor,
									float &max_distance)
{
	if (params->Value.size() < 3 || params->Value.size() > 5)
	{
		script->Script_Error("FindTarget -- Invalid number of parameters: expected 2 or 3, got %d.", params->Value.size());
		return false;
	}

	//First parameter should be a task force belonging to an AI player
	taskforce = LUA_SAFE_CAST(TaskForceClass, params->Value[0]);
	if (!taskforce)
	{
		script->Script_Error("FindTarget -- Parameter 1 is not a valid taskforce.");
		return false;
	}

	//Second parameter should be the name of the perception function used to pick a target
	LuaString *function_name = PG_Dynamic_Cast<LuaString>(params->Value[1]);
	if (!function_name)
	{
		script->Script_Error("FindTarget -- Parameter 2 is not a valid string");
		return false;
	}

	function = ThePerceptionFunctionManagerPtr->Get_Managed_Object(function_name->Value);
	if (!function)
	{
		script->Script_Error("FindTarget -- unrecognized perception function %s.", function_name->Value.c_str());
		return false;
	}

	//third parameter should be goal application type
	LuaString *application_name = PG_Dynamic_Cast<LuaString>(params->Value[2]);
	if (!application_name)
	{
		script->Script_Error("FindTarget -- Parameter 3 is not a valid string.");
		return false;
	}

	if (!TheAIGoalApplicationTypeConverterPtr->String_To_Enum(application_name->Value, application_type))
	{
		script->Script_Error("FindTarget -- unrecognized goal application %s.", application_name->Value.c_str());
		return false;
	}

	if (application_type == AI_GOAL_APPLICATION_TYPE_GLOBAL)
	{
		script->Script_Error("FindTarget -- can't find a target for a global goal!");
		return false;
	}

	if (params->Value.size() == 3)
	{
		best_choice_scale_factor = 0.0;
		return true;
	}

	//Fourth parameter is probability of selecting best target
	LuaNumber *lua_prob_best = PG_Dynamic_Cast<LuaNumber>(params->Value[3]);
	if (!lua_prob_best)
	{
		script->Script_Error("FindTarget -- third parameter is not a valid number.");
		return false;
	}

	if (lua_prob_best->Value <= 0.0 || lua_prob_best->Value > 1.0f)
	{
		script->Script_Error("FindTarget -- Probability of selecting best target is not in (0,1].");
		return false;
	}

	best_choice_scale_factor = 1.0f / lua_prob_best->Value - 1.0f;

	if (params->Value.size() == 4)
	{
		max_distance = 0.0;
		return true;
	}

	//Fifth parameter is maximum distance to target.  If this is not set then distance will not be considered.
	LuaNumber *lua_max_distance = PG_Dynamic_Cast<LuaNumber>(params->Value[4]);
	if (!lua_max_distance)
	{
		script->Script_Error("FindTarget -- Fourth parameter is not a valid number.");
		return false;
	}

	max_distance = lua_max_distance->Value;

	return true;
}

/**************************************************************************************************
* FindTargetClass::Best_Of -- Given a list of targets find the one that scores best when evaluated
*	by some perception function
*
* In:			
*
* Out:		
*
* History: 3/3/2005 5:13PM JSY
**************************************************************************************************/
LuaTable *FindTargetClass::Best_Of(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 3)
	{
		script->Script_Error("Find_Best_Of -- invalid number of parameters.  Expected 3, got %d.");
		return 0;
	}

	SmartPtr<TaskForceClass> tf = PG_Dynamic_Cast<TaskForceClass>(params->Value[0]);
	if (!tf)
	{
		script->Script_Error("Find_Best_Of -- invalid type for parameter 1.  Expected taskforce.");
		return 0;
	}

	SmartPtr<LuaTable> target_list = PG_Dynamic_Cast<LuaTable>(params->Value[1]);
	if (!target_list)
	{
		script->Script_Error("Find_Best_Of -- invalid type for parameter 2.  Expected table of targets.");
		return 0;
	}

	SmartPtr<LuaString> function_name = PG_Dynamic_Cast<LuaString>(params->Value[2]);
	if (!function_name)
	{
		script->Script_Error("Find_Best_Of -- invalid type for parameter 3.  Expected string.");
		return 0;
	}

	PerceptionFunctionClass *perception = ThePerceptionFunctionManagerPtr->Get_Managed_Object(function_name->Value);
	if (!perception)
	{
		script->Script_Error("Find_Best_Of -- unrecognized perception function %s.", function_name->Value.c_str());
		return 0;
	}

	//Go through each target evaluating the specified perception.  We're going to encounter problems if the
	//targets are not all of the same type.
	TacticalAIManagerClass *tactical_manager = tf->Get_Plan()->Get_Planning_System()->Get_Manager();
	AIPerceptionSystemClass *perception_system = tactical_manager->Get_Perception_System();
	PlayerClass *player = tf->Get_Plan()->Get_Player();
	PerceptionTokenType self_token = player->Get_Evaluator_Token();
	PerceptionTokenType enemy_token = PERCEPTION_TOKEN_INVALID;
	PerceptionTokenType human_token = PERCEPTION_TOKEN_INVALID;
	if (perception_system->Get_Manager()->Get_Enemy())
	{
		enemy_token = perception_system->Get_Manager()->Get_Enemy()->Get_Evaluator_Token();
	}
	PlayerClass *human = PlayerList.Get_Human_Player();
	if (human)
	{
		human_token = human->Get_Evaluator_Token();
	}

	float best_score = 0.0f;
	int best_index = -1;
	for (unsigned int i = 0; i < target_list->Value.size(); ++i)
	{
		PerceptionContextClass perception_context(perception_system);
		perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_SELF, self_token);
		if (enemy_token != PERCEPTION_TOKEN_INVALID)
		{
			perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_ENEMY, enemy_token);
		}
		if (human_token != PERCEPTION_TOKEN_INVALID)
		{
			perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_HUMAN, human_token);
		}
		perception_context.Set_Player(player);
		SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(target_list->Value[i]);
		SmartPtr<AITargetLocationWrapper> ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(target_list->Value[i]);

		if (object_wrapper)
		{
			GameObjectClass *object_target = object_wrapper->Get_Object();
			if (!object_wrapper->Get_Object())
			{
				script->Script_Warning("Find_Best_OF -- target in target list is already dead.");
				continue;
			}

			if (object_target->Behaves_Like(BEHAVIOR_PLANET))
			{
				perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_TARGET, object_target->Get_Planetary_Data()->Get_Evaluator_Token());
			}
			else
			{
				perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_TARGET, AIPerceptionSystemClass::Build_Game_Object_Token_String(object_target));
			}
		}
		else if (ai_target_wrapper)
		{
			if (!ai_target_wrapper->Get_Object())
			{
				script->Script_Warning("Find_Best_OF -- target in target list is already dead.");
				continue;
			}

			perception_context.Add_Context(PERCEPTION_TOKEN_VARIABLE_TARGET, ai_target_wrapper->Get_Object()->Get_Evaluator());
			perception_context.Set_Target(ai_target_wrapper->Get_Object());
		}
		else
		{
			script->Script_Warning("Find_Best_Of -- unsupported entry in target list: these should be game objects or AI targets.");
			continue;
		}

		float score = 0.0f;
		perception->Evaluate(perception_context, score);

		if (score > best_score)
		{
			best_score = score;
			best_index = static_cast<int>(i);
		}
	}

	if (best_index == -1)
	{
		return 0;
	}
	else
	{
		return Return_Variable(target_list->Value[best_index]);
	}
}