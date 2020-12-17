// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/EvaluateTypeList.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/EvaluateTypeList.cpp $
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


#include "EvaluateTypeList.h"

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
#include "AI/LuaScript/Commands/WeightedTypeList.h"
#include "AI/Perception/Evaluators/PerceptionEvaluationState.h"
#include "AI/Perception/TacticalPerceptionGrid.h"
#include "AI/Perception/Evaluators/Tactical/TacticalLocationPerceptualEvaluator.h"
#include "AI/AIHintZone.h"


PG_IMPLEMENT_RTTI(EvaluateTypeListClass, LuaUserVar);

// target = planet or NULL
// type_list = WeightedTypeList
// FoundTypes = vector of bools
//	FoundTypes = Evaluate_Type_List(PlayerObject, target, type_list)

LuaTable *EvaluateTypeListClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 3)
	{
		script->Script_Error("Evaluate_Type_List: Invalid number of parameters.  Expected 3, got %d.", params->Value.size());
		return NULL;
	}

	PlayerWrapper *player = LUA_SAFE_CAST(PlayerWrapper, params->Value[0]);
	if (!player)
	{
		script->Script_Error("Evaluate_Type_List: Invalid parameter type for parameter 1. Expected Player Wrapper.");
		return NULL;
	}

	AITargetLocationWrapper *ai_target = LUA_SAFE_CAST(AITargetLocationWrapper, params->Value[1]);
	if (!ai_target) {
		//This is not an error case: we may be evaluating a list for a global goal, and this is the easiest place to handle it
		return NULL;
	}

	if (!ai_target->Get_Object())
	{
		script->Script_Error("Evaluate_Type_List: AI target location is defunct.");
		return NULL;
	}

	WeightedTypeListClass *tlist = LUA_SAFE_CAST(WeightedTypeListClass, params->Value[2]);
	if (tlist == NULL) {
		script->Script_Error("Evaluate_Type_List: Invalid parameter type for parameter 3. Expected weighted type list.");
		return NULL;
	}

	if (ai_target->Get_Object()->Get_Perception_System()->Get_Manager()->Get_Game_Mode() == SUB_GAME_MODE_GALACTIC)
	{
		return Galactic_Evaluate(script, player, ai_target, tlist); 
	}
	else
	{
		return Tactical_Evaluate(script, player, ai_target, tlist);
	}
}

/**************************************************************************************************
* EvaluateTypeListClass::Galactic_Evaluate -- Implementation of script function for galactic mode
*
* In:				
*
* Out:		
*
* History: 11/22/2004 3:14PM JSY
**************************************************************************************************/
LuaTable *EvaluateTypeListClass::Galactic_Evaluate(LuaScriptClass *script, PlayerWrapper *player, AITargetLocationWrapper *target, WeightedTypeListClass *tlist)
{
	if (!target->Get_Object() || !target->Get_Object()->Get_Target_Game_Object())
	{
		script->Script_Error("Evaluate_Type_List: parameter 2 should be a planet target.");
		return NULL;
	}

	AIPerceptionSystemClass *perception_system = target->Get_Object()->Get_Perception_System();

	// build a perception context for the query we're about to make		
	PerceptionContextClass context( perception_system );
	context.Add_Context( PERCEPTION_TOKEN_VARIABLE_TARGET, target->Get_Object()->Get_Evaluator() );
	context.Set_Target(target->Get_Object());
	context.Set_Player(player->Get_Object());

	// Variable_Target.EnemyForce.SpaceTotalUnnormalized
	PerceptionTokenVector tokens;
	tokens.push_back( PERCEPTION_TOKEN_VARIABLE_TARGET );
	tokens.push_back( PERCEPTION_TOKEN_ENEMY_FORCE );
	tokens.push_back( PERCEPTION_TOKEN_GROUND_UNITS_BITFIELD );

	PerceptionEvaluationStateClass evaluation_state( &context, tokens );

	int num_types = 0;
	int i = 0;
	for (i = 0; i < tlist->Get_Weight_Count(); i++) {

		float dval;
		if (tlist->Is_Category_At_Index(i)) {
			dval = AIPerceptionSystemClass::Enum_Value_To_Parameter_Value(tlist->Get_Category_At_Index(i));
			evaluation_state.Add_Parameter(PERCEPTION_TOKEN_PARAMETER_CATEGORY, dval);
		} else {
			dval = AIPerceptionSystemClass::String_To_Parameter_Value(tlist->Get_Type_At_Index(i)->Get_Name());
			evaluation_state.Add_Parameter(PERCEPTION_TOKEN_PARAMETER_TYPE, dval);
			num_types++;
		}
	}

	// make the query
	float query_result = 0.0;
	perception_system->Evaluate_Perception( evaluation_state, query_result );

	unsigned long bit_result = (unsigned long)query_result;

	int t = 0, s = 0;
	LuaTable *retval = Alloc_Lua_Table();
	for (i = 0; i < tlist->Get_Weight_Count(); i++) {
		bool setval = false;
		if (tlist->Is_Category_At_Index(i)) {
			setval = ((bit_result & (1 << (s + num_types))) != 0);
			s++;
		} else {
			setval = ((bit_result & (1 << t)) != 0);
			t++;
		}
		retval->Value.push_back(new LuaBool(setval));
	}

	tokens.pop_back();
	tokens.push_back( PERCEPTION_TOKEN_SPACE_UNITS_BITFIELD );

	PerceptionEvaluationStateClass evaluation_state2( &context, tokens );

	num_types = 0;
	for (i = 0; i < tlist->Get_Weight_Count(); i++) {

		float dval;
		if (tlist->Is_Category_At_Index(i)) {
			dval = AIPerceptionSystemClass::Enum_Value_To_Parameter_Value(tlist->Get_Category_At_Index(i));
			evaluation_state2.Add_Parameter(PERCEPTION_TOKEN_PARAMETER_CATEGORY, dval);
		} else {
			dval = AIPerceptionSystemClass::String_To_Parameter_Value(tlist->Get_Type_At_Index(i)->Get_Name());
			evaluation_state2.Add_Parameter(PERCEPTION_TOKEN_PARAMETER_TYPE, dval);
			num_types++;
		}
	}

	// make the query
	query_result = 0.0;
	perception_system->Evaluate_Perception( evaluation_state2, query_result );

	bit_result = (unsigned long)query_result;

	t = 0; s = 0;
	for (i = 0; i < tlist->Get_Weight_Count(); i++) {
		bool setval = false;
		if (tlist->Is_Category_At_Index(i)) {
			setval = ((bit_result & (1 << (s + num_types))) != 0);
			s++;
		} else {
			setval = ((bit_result & (1 << t)) != 0);
			t++;
		}
		LuaBool *bval = PG_Dynamic_Cast<LuaBool>(retval->Value[i]);
		bval->Value = (bval->Value || setval);
	}

	return Return_Variable(retval);	
}

/**************************************************************************************************
* EvaluateTypeListClass::Tactical_Evaluate -- Implementation of script function for tactical mode
*
* In:				
*
* Out:		
*
* History: 11/22/2004 3:23PM JSY
**************************************************************************************************/
LuaTable *EvaluateTypeListClass::Tactical_Evaluate(LuaScriptClass *, PlayerWrapper *player, AITargetLocationWrapper *target, WeightedTypeListClass *tlist)
{
	float result = 0.0f;
	
	PerceptionContextClass context(target->Get_Object()->Get_Perception_System());
	context.Add_Context(PERCEPTION_TOKEN_VARIABLE_TARGET, target->Get_Object()->Get_Evaluator());
	context.Set_Player(player->Get_Object());
	context.Set_Target(target->Get_Object());

	static PerceptionTokenVector tokens;
	tokens.resize(0);
	tokens.push_back(PERCEPTION_TOKEN_VARIABLE_TARGET);
	if (target->Get_Object()->Get_Target_Game_Object())
	{
		tokens.push_back(PERCEPTION_TOKEN_LOCATION);
	}
	tokens.push_back(PERCEPTION_TOKEN_ENEMY_UNITS_BITFIELD);

	PerceptionEvaluationStateClass evaluation_state(&context, tokens);

	//Create the parameter set for the call.
	int num_types = 0;
	for (int i = 0; i < tlist->Get_Weight_Count(); i++) {

		if (tlist->Is_Category_At_Index(i)) 
		{
			float param = AIPerceptionSystemClass::Enum_Value_To_Parameter_Value(tlist->Get_Category_At_Index(i));
			evaluation_state.Add_Parameter(PERCEPTION_TOKEN_PARAMETER_CATEGORY, param);
		} 
		else
		{
			float param = AIPerceptionSystemClass::String_To_Parameter_Value(tlist->Get_Type_At_Index(i)->Get_Name());
			evaluation_state.Add_Parameter(PERCEPTION_TOKEN_PARAMETER_TYPE, param);
			++num_types;
		}
	}

	target->Get_Object()->Get_Perception_System()->Evaluate_Perception(evaluation_state, result);

	int bit_result = *reinterpret_cast<int*>(&result);

	//The perception query arranges the result so that types are first, followed by categories. 
	//We need to put the results back in the order the script expects.
	LuaTable *retval = Alloc_Lua_Table();
	for (int i = 0, s = 0, t = 0; i < tlist->Get_Weight_Count(); ++i) 
	{
		bool setval = false;
		if (tlist->Is_Category_At_Index(i)) 
		{
			setval = ((bit_result & (1 << (s + num_types))) != 0);
			++s;
		} 
		else 
		{
			setval = ((bit_result & (1 << t)) != 0);
			++t;
		}
		retval->Value.push_back(new LuaBool(setval));
	}

	return Return_Variable(retval);
}