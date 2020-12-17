// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ProjectByUnitRange.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ProjectByUnitRange.cpp $
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

#include "ProjectByUnitRange.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "GameObject.h"
#include "AI/LuaScript/PositionWrapper.h"
#include "GameModeManager.h"
#include "GameMode.h"
#include "HardPoint.h"

PG_IMPLEMENT_RTTI(ProjectByUnitRangeClass, LuaUserVar);

LuaTable *ProjectByUnitRangeClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 2)
	{
		script->Script_Error("Project_By_Unit_Range -- invalid number of parameters.  Expected 2 got %d.", params->Value.size());
		return NULL;
	}

	if (!GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical())
	{
		script->Script_Error("Project_By_Unit_Range -- this function is only valid in tactical.");
		return NULL;
	}

	SmartPtr<GameObjectWrapper> target = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	if (!target)
	{
		script->Script_Error("Project_By_Unit_Range -- invalid type for parameter 1.  Expected game object.");
		return NULL;
	}

	if (!target->Get_Object())
	{
		script->Script_Error("Project_By_Unit_Range -- target is already dead.");
		return NULL;
	}

	Vector3 project_through_position;
	if (!Lua_Extract_Position(params->Value[1], project_through_position))
	{
		script->Script_Error("Project_By_Unit_Range -- unable to extract a position from parameter 2.");
		return NULL;
	}

	Vector3 project_direction = project_through_position - target->Get_Object()->Get_Position();

	if (project_direction == VECTOR3_NONE)
	{
		script->Script_Warning("Project_By_Unit_Range -- unable to project: positions are identical.");
		return NULL;
	}

	project_direction.Fast_Normalize();

	GameObjectClass *targeting_object = target->Get_Object();
	if (targeting_object->Behaves_Like(BEHAVIOR_TEAM))
	{
		targeting_object = targeting_object->Get_Team_Data()->Get_Team_Leader();
	}

	if (!targeting_object)
	{
		return Return_Variable(PositionWrapper::Create(project_through_position));
	}

	float project_range = targeting_object->Get_Type()->Get_Targeting_Max_Attack_Distance(targeting_object);
	for (int i = 0; i < targeting_object->Get_Total_Hard_Points(); ++i)
	{
		HardPointClass *hard_point = targeting_object->Get_Hard_Point_By_Index(i);
		FAIL_IF(!hard_point) { continue; }
		if (hard_point->Is_Destroyable() && hard_point->Is_Destroyed())
		{
			continue;
		}

		if (hard_point->Get_Type() < HARD_POINT_WEAPON_FIRST || hard_point->Get_Type() > HARD_POINT_WEAPON_LAST)
		{
			continue;
		}

		project_range = Max(project_range, hard_point->Get_Max_Range());
	}

	Vector3 projected_position = project_through_position + project_range * project_direction;

	Box3Class world_bounds = GameModeManager.Get_Active_Mode()->Get_Map_Bounds();
	projected_position.X = PGMath::Clamp(projected_position.X, world_bounds.Get_XMin(), world_bounds.Get_XMax());
	projected_position.Y = PGMath::Clamp(projected_position.Y, world_bounds.Get_YMin(), world_bounds.Get_YMax());

	return Return_Variable(PositionWrapper::Create(projected_position));
}