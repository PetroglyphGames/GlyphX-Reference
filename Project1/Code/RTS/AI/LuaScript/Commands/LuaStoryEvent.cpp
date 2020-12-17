// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaStoryEvent.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaStoryEvent.cpp $
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


#include "LuaStoryEvent.h"
#include "GameModeManager.h"
#include "StoryMode/StoryMode.h"
#include "AI/LuaScript/GameObjectWrapper.h"

PG_IMPLEMENT_RTTI(LuaStoryEventClass, LuaUserVar);

LuaTable *LuaStoryEventClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() == 0 || params->Value.size() > 2)
	{
		script->Script_Error("StoryEvent -- invalid number of parameters.  Expected 1 or 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> trigger_string = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!trigger_string)
	{
		script->Script_Error("StoryEvent -- invalid type for parameter 1.  Expected string");
		return NULL;
	}

	GameObjectClass *planet = 0;
	if (params->Value.size() == 2)
	{
		SmartPtr<GameObjectWrapper> lua_planet = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[1]);
		if (!lua_planet)
		{
			script->Script_Error("StoryEvent -- invalid type for parameter 2.  Expected game object.");
			return NULL;
		}
		planet = lua_planet->Get_Object();
	}

	GameModeManager.Get_Active_Mode()->Get_Story_Mode().Story_Event(STORY_AI_NOTIFICATION, 0, &trigger_string->Value, planet);

	return NULL;
}

PG_IMPLEMENT_RTTI(LuaAddObjectiveClass, LuaUserVar);

LuaTable *LuaAddObjectiveClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 2)
	{
		script->Script_Error("Add_Objective -- invalid number of parameters.  Expected 2, got %d.");
		return NULL;
	}

	SmartPtr<LuaString> objective_tag = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!objective_tag)
	{
		script->Script_Error("Add_Objective -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	SmartPtr<LuaBool> suggestion = PG_Dynamic_Cast<LuaBool>(params->Value[1]);
	if (!suggestion)
	{
		script->Script_Error("Add_Objective -- invalid type for parameter 2.  Expected boolean.");
		return NULL;
	}

	if (!GameModeManager.Get_Active_Mode())
	{
		script->Script_Error("Add_Objective -- no game mode currently playing.");
		return NULL;
	}

	GameModeManager.Get_Active_Mode()->Get_Story_Mode().Add_Objective(objective_tag->Value, NULL, suggestion->Value);

	return NULL;
}