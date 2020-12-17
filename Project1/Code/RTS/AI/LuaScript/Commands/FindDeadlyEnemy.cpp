// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindDeadlyEnemy.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindDeadlyEnemy.cpp $
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


#include "FindDeadlyEnemy.h"

#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/AITargetLocation.h"
#include "AI/Planning/TaskForce.h"
#include "GameObjectManager.h"
#include "DamageTrackingBehavior.h"
#include "hash_map"

#include "TeamBehavior.h"

PG_IMPLEMENT_RTTI(FindDeadlyEnemyClass, LuaUserVar);

/**************************************************************************************************
* FindDeadlyEnemyClass::Function_Call -- Entry point from script
*
* In:			
*
* Out:		
*
* History: 9/16/2004 10:05AM JSY
**************************************************************************************************/
LuaTable *FindDeadlyEnemyClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	//Accept as parameters:
	//	- a taskforce
	//	- an AI target that wraps a game object
	//	- an AI target of any type, a range, and the player who's asking.
	if (params->Value.size() != 1 &&
		params->Value.size() != 3)
	{
		script->Script_Error("FindDeadlyEnemy - invalid number of parameters.  Got %d, expected 1 or 3.", params->Value.size());
		return 0;
	}

	GameObjectClass *enemy = 0;

	if (params->Value.size() == 1)
	{
		//Either the taskforce or the game object target
		SmartPtr<TaskForceClass> tf = PG_Dynamic_Cast<TaskForceClass>(params->Value[0]);
		SmartPtr<AITargetLocationWrapper> target = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);
		SmartPtr<GameObjectWrapper> object = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);

		if (tf)
		{
			enemy = Find_Deadly_Enemy(tf);
		}
		else if (target && target->Get_Object() && target->Get_Object()->Get_Target_Game_Object())
		{
			enemy = Find_Deadly_Enemy(target->Get_Object()->Get_Target_Game_Object());
		}
		else if (object && object->Get_Object())
		{
			enemy = Find_Deadly_Enemy(object->Get_Object());
		}
		else
		{
			script->Script_Error("FindDeadlyEnemy - invalid type for parameter 1. Expected taskforce or AI ship target location.");
			return 0;
		}
	}
	else
	{
		SmartPtr<AITargetLocationWrapper> target = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);
		if (!target || !target->Get_Object())
		{
			script->Script_Error("FindDeadlyEnemy - invalid type for parameter 1. Expected AI target location.");
			return 0;
		}

		SmartPtr<LuaNumber> range = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
		if (!range)
		{
			script->Script_Error("FindDeadlyEnemy - invalid type for parameter 2.  Expected number.");
			return 0;
		}

		if (range->Value <= 0.0f)
		{
			script->Script_Error("FindDeadlyEnemy - invalid value for parameter 2.  Expected positive number.");
			return 0;
		}

		SmartPtr<PlayerWrapper> player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[2]);
		if (!player || !player->Get_Object())
		{
			script->Script_Error("FindDeadlyEnemy - invalid type for parameter 3. Expected player.");
			return 0;
		}

		enemy = Find_Deadly_Enemy(target->Get_Object(), range->Value, player->Get_Object());
	}

	if (enemy)
	{
		return Return_Variable(GameObjectWrapper::Create(enemy, script));
	}
	else
	{
		return 0;
	}
}

struct ThreatComparisonStruct : public binary_function<std::pair<int,float>,std::pair<int,float>,bool>
{
	bool operator()(const std::pair<int,float> &lhs, const std::pair<int,float> &rhs) const { return lhs.second < rhs.second; }
};

/**************************************************************************************************
* FindDeadlyEnemyClass::Find_Deadly_Enemy -- Find the most dangerous enemy of a single game object
*
* In:			Object whose enemy we'd like to find
*
* Out:			Object that's done most damage to the input object recently
*
* History: 9/16/2004 10:08AM JSY
**************************************************************************************************/
GameObjectClass *FindDeadlyEnemyClass::Find_Deadly_Enemy(GameObjectClass *target)
{
	FAIL_IF(!target) { return 0; }

	const stdext::hash_map<int,float> *damage_map = 0;
	stdext::hash_map<int,float> internal_damage_map;

	if (target->Behaves_Like(BEHAVIOR_DAMAGE_TRACKING))
	{
		DamageTrackingBehaviorClass *damage_tracking = static_cast<DamageTrackingBehaviorClass*>(target->Get_Behavior(BEHAVIOR_DAMAGE_TRACKING));
		if (!damage_tracking)
		{
			return 0;
		}
		damage_map = &damage_tracking->Get_Threatening_Objects();
	}
	else if (target->Behaves_Like(BEHAVIOR_TEAM))
	{
		Add_Object_To_Damage_Tracking_Map(target, internal_damage_map);
		damage_map = &internal_damage_map;
	}

	if (damage_map)
	{
		stdext::hash_map<int, float>::const_iterator i = std::max_element(damage_map->begin(), damage_map->end(), ThreatComparisonStruct());

		if (i != damage_map->end())
		{
			return GAME_OBJECT_MANAGER.Get_Object_From_ID(i->first);
		}
	}

	return 0;
}

/**************************************************************************************************
* FindDeadlyEnemyClass::Find_Deadly_Enemy -- Find the most dangerous enemy of a taskforce
*
* In:			Taskforce whose enemy we'd like to find
*
* Out:			Object that's done most damage to the input taskforce recently
*
* History: 9/16/2004 10:08AM JSY
**************************************************************************************************/
GameObjectClass *FindDeadlyEnemyClass::Find_Deadly_Enemy(TaskForceClass *taskforce)
{
	//Mash together the per-object DOT tables for all the units in the taskforce
	stdext::hash_map<int, float> taskforce_enemies;
	for (unsigned int i = 0; i < taskforce->Get_Member_Count(); ++i)
	{
		GameObjectClass *unit = taskforce->Get_Member(i)->Get_Object();
		FAIL_IF(!unit) { continue; }
		Add_Object_To_Damage_Tracking_Map(unit, taskforce_enemies);
	}

	//Now we can go ahead and find the one that's done the most damage
	stdext::hash_map<int, float>::const_iterator most_deadly = std::max_element(taskforce_enemies.begin(),
																				taskforce_enemies.end(),
																				ThreatComparisonStruct());

	if (most_deadly != taskforce_enemies.end())
	{
		return GAME_OBJECT_MANAGER.Get_Object_From_ID(most_deadly->first);
	}

	return 0;
}

/**************************************************************************************************
* FindDeadlyEnemyClass::Find_Deadly_Enemy -- Find the most dangerous enemy of friendly units within
*	a given area
*
* In:			Target location defining the center point of the area
*				Radius of circle defining area
*				Player whose enemy we want to find
*
* Out:			Object that's done most damage recently to the player's forces within the input region
*
* History: 9/16/2004 10:49AM JSY
**************************************************************************************************/
GameObjectClass *FindDeadlyEnemyClass::Find_Deadly_Enemy(const AITargetLocationClass *target, float range, PlayerClass *for_player)
{
	const Vector3 &target_pos = target->Get_Target_Position();

	//Find friendly forces that support damage tracking within range 
	const DynamicVectorClass<GameObjectClass*> *friendly_units = GAME_OBJECT_MANAGER.Find_Objects(BEHAVIOR_DAMAGE_TRACKING, 
																									for_player->Get_ID(), 
																									PlayerClass::INVALID_PLAYER_ID, 
																									target_pos, 
																									range);

	//Mash together the per-object DOT tables for all the units in the taskforce
	stdext::hash_map<int, float> taskforce_enemies;
	for (int i = 0; i < friendly_units->Size(); ++i)
	{
		GameObjectClass *unit = friendly_units->Get_At(i);
		FAIL_IF(!unit) { continue; }
		Add_Object_To_Damage_Tracking_Map(unit, taskforce_enemies);
	}

	//Now we can go ahead and find the one that's done the most damage
	stdext::hash_map<int, float>::const_iterator most_deadly = std::max_element(taskforce_enemies.begin(),
																				taskforce_enemies.end(),
																				ThreatComparisonStruct());

	if (most_deadly != taskforce_enemies.end())
	{
		return GAME_OBJECT_MANAGER.Get_Object_From_ID(most_deadly->first);
	}

	return 0;
}

/**************************************************************************************************
* FindDeadlyEnemyClass::Add_Object_To_Damage_Tracking_Map -- Mash an object (possibly a team) into a damage map
*
* In:			
*
* Out:			
*
* History: 2/16/2005 12:07PM JSY
**************************************************************************************************/
void FindDeadlyEnemyClass::Add_Object_To_Damage_Tracking_Map(GameObjectClass *object, stdext::hash_map<int,float> &damage_map)
{
	FAIL_IF(!object) { return; }

	if (object->Behaves_Like(BEHAVIOR_DAMAGE_TRACKING))
	{
		DamageTrackingBehaviorClass *damage_tracking = static_cast<DamageTrackingBehaviorClass*>(object->Get_Behavior(BEHAVIOR_DAMAGE_TRACKING));
		FAIL_IF(!damage_tracking) { return; }

		for (stdext::hash_map<int, float>::const_iterator it = damage_tracking->Get_Threatening_Objects().begin();
				it != damage_tracking->Get_Threatening_Objects().end();
				++it)
		{
			stdext::hash_map<int, float>::iterator existing_entry = damage_map.find(it->first);

			if (existing_entry != damage_map.end())
			{
				existing_entry->second += it->second;
			}
			else
			{
				damage_map.insert(*it);
			}
		}
	}
	else if (object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return; }
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }

			Add_Object_To_Damage_Tracking_Map(team_member, damage_map);
		}
	}
}