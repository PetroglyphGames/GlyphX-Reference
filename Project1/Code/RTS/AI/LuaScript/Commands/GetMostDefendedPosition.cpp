// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GetMostDefendedPosition.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GetMostDefendedPosition.cpp $
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

#include "GetMostDefendedPosition.h"
#include "AI/AIHintZone.h"
#include "GameModeManager.h"
#include "GameMode.h"
#include "AI/Perception/AIPerceptionSystem.h"
#include "AI/Perception/TacticalPerceptionGrid.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/PositionWrapper.h"
#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "AI/AITargetLocation.h"
#include "GameConstants.h"

PG_IMPLEMENT_RTTI(GetMostDefendedPositionClass, LuaUserVar);

LuaTable *GetMostDefendedPositionClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	SubGameModeType mode = GameModeManager.Get_Sub_Type();

	if (mode == SUB_GAME_MODE_GALACTIC)
	{
		script->Script_Error("Get_Most_Defended_Position -- this script function may only be used in tactical modes.");
		return NULL;
	}

	if (params->Value.size() != 2)
	{
		script->Script_Error("Get_Most_Defended_Position -- invalid number of parameters.  Expected 2, got %d.");
		return NULL;
	}

	Vector3 position;
	AIHintZoneClass *hint_zone = 0;
	SmartPtr<AITargetLocationWrapper> ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);
	if (ai_target_wrapper && ai_target_wrapper->Get_Object())
	{
		hint_zone = ai_target_wrapper->Get_Object()->Get_Hint_Zone();
	}

	SmartPtr<PlayerWrapper> player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[1]);
	if (!player)
	{
		script->Script_Error("Get_Most_Defended_Position -- invalid type for parameter 2.  Expected player.");
		return NULL;
	}

	if (!hint_zone)
	{
		if (!Lua_Extract_Position(params->Value[0], position))
		{
			script->Script_Error("Get_Most_Defended_Position -- invalid type for parameter 1.  Expected object that can define a position.");
			return NULL;
		}

		if (mode == SUB_GAME_MODE_LAND)
		{
			hint_zone = GameModeManager.Get_Active_Mode()->Get_Hint_Zone_At_Location(position);
			if (!hint_zone)
			{
				script->Script_Warning("Get_Most_Defended_Position -- cannot translate target to hint zone; map may not be zoned.");
				return NULL;
			}
		}
	}

	FAIL_IF (!AIPerceptionSystemClass::Get_Perception_Grid()) { return NULL; }
	
	Vector3 defended_position;
	
	if (mode == SUB_GAME_MODE_LAND)
	{
		defended_position = AIPerceptionSystemClass::Get_Perception_Grid()->Get_Most_Defended_Position(hint_zone->Get_Threat_Cells(), GAME_OBJECT_CATEGORY_ALL, player->Get_Object());
	}
	else
	{
		FRect query_rect;
		float query_size = TheGameConstants.Get_AI_Space_Evaluator_Region_Size();
		query_rect.X = position.X - query_size / 2.0f;
		query_rect.Y = position.Y - query_size / 2.0f;
		query_rect.Width = query_size;
		query_rect.Height = query_size;

		defended_position = AIPerceptionSystemClass::Get_Perception_Grid()->Get_Most_Defended_Position(query_rect, GAME_OBJECT_CATEGORY_ALL, player->Get_Object());
	}


	return Return_Variable(PositionWrapper::Create(defended_position));
}