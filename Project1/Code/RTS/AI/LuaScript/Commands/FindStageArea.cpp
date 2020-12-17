// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindStageArea.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindStageArea.cpp $
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
#include "FindStageArea.h"
#include "GameObject.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "LuaScript.h"
#include "AI/Goal/AIGoalReachabilityType.h"
#include "AI/Goal/InstantiatedGoal.h"
#include "AI/Goal/AIGoalProposalFunction.h"
#include "AI/Goal/AIGoalType.h"
#include "PlanetaryBehavior.h"
#include "GameObjectTypeManager.h"
#include "GameObjectType.h"
#include "GameObjectManager.h"
#include "Faction.h"
#include "AIPlayer.h"
#include "AITargetLocation.h"
#include "TacticalAIManager.h"
#include "AI/Planning/Reachability.h"
#include "AI/Planning/TaskForce.h"
#include "AI/Planning/PlanBehavior.h"
#include "AI/PathFinding/GalacticPath.h"
#include "AI/PathFinding/GalacticPathFinder.h"
#include "AI/Perception/AIPerceptionSystem.h"


PG_IMPLEMENT_RTTI(FindStageAreaClass, LuaUserVar);


/**
 * Lua function to find an appropriate staging location based production
 * facility location and target location.
 * 
 * @param script lua script
 * @param params lua params.  Player object of the player.  Target planet.
 * 
 * @return Planet where the units should stage at.
 * @since 5/28/2004 2:20:05 PM -- BMH
 */
LuaTable* FindStageAreaClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	SmartPtr<PlayerWrapper> player = LUA_SAFE_CAST(PlayerWrapper, params->Value[0]);
	if (!player) {
		script->Script_Error("FindStageAreaClass -- Expected a player object for parameter 1");
		return NULL;
	}

	SmartPtr<AITargetLocationWrapper> twrapper = LUA_SAFE_CAST(AITargetLocationWrapper, params->Value[1]);
	SmartPtr<GameObjectWrapper> planet = LUA_SAFE_CAST(GameObjectWrapper, params->Value[1]);
	if (!twrapper && !planet) {
		script->Script_Error("FindStageAreaClass -- Expected a AITargetLocationWrapper/GameObjectWrapper for parameter 2");
		return NULL;
	}

	AITargetLocationClass *target = NULL;
	if (twrapper)
		target = const_cast<AITargetLocationClass *>(twrapper->Get_Object());
	else
	{
		const AIPerceptionSystemClass::GoalTargetVectorType &tvec = player->Get_Object()->Get_AI_Player()->
			Get_Tactical_Manager_By_Mode(SUB_GAME_MODE_GALACTIC)->Get_Perception_System()->Get_Goal_Target_List();

		GameObjectClass *cont = planet->Get_Object();
		while (cont && cont->Behaves_Like(BEHAVIOR_PLANET) == false) {
			cont = cont->Get_Parent_Container_Object();
		}
	
		if (!cont) {
			script->Script_Error("FindStageAreaClass -- Couldn't find a parent planet for target");
			return NULL;
		}
		PlanetaryBehaviorClass *behave = (PlanetaryBehaviorClass *)cont->Get_Behavior(BEHAVIOR_PLANET);
		target = tvec[behave->Get_AI_Target_Index()];
	}


	SmartPtr<TaskForceClass> tf = LUA_SAFE_CAST(TaskForceClass, params->Value[2]);

	AITargetLocationClass *stage = Find_Stage(player->Get_Object(), target, tf);

	if (!stage) return NULL;

	if (twrapper)
	{
		return Return_Variable(AITargetLocationWrapper::Create(stage, script));
	}

   return Return_Variable(GameObjectWrapper::Create(stage->Get_Target_Game_Object(), script));
}


/**
 * Find the stage location.  Iterate over all of our production facilities.  
 * Run a find path to target from the facility.  Keep track of the number of
 * times a planet is evaluated and weight that into the selection algorithm.
 * 
 * @param player Player who is searching for the stage location.
 * @param target Enemy target location.
 * 
 * @return planet that will be the stage location.
 * @since 5/28/2004 2:22:54 PM -- BMH
 */
AITargetLocationClass *FindStageAreaClass::Find_Stage
(
	#ifndef NDEBUG
		PlayerClass *player, 
	#else
		PlayerClass *,
	#endif
	AITargetLocationClass *target, TaskForceClass *tf)
{
	assert(player && player->Get_AI_Player());

	AIGoalReachabilityType type = GOAL_REACHABILITY_FRIENDLY_ONLY;
	if (tf) {
		type = tf->Get_Plan()->Get_Goal()->Get_Source()->Get_Goal()->Get_Reachability();
	}

	return target->Get_Target_Reachability()->Get_Stage_Location(type, CELL_PASSABILITY_INVALID);
}