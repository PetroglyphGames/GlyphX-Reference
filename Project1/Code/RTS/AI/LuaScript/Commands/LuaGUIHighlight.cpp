// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaGUIHighlight.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaGUIHighlight.cpp $
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

#include "LuaGUIHighlight.h"
#include "CommandBar.h"
#include "AI/LuaScript/GameObjectWrapper.h"

PG_IMPLEMENT_RTTI(AddRadarBlipClass, LuaUserVar);
PG_IMPLEMENT_RTTI(RemoveRadarBlipClass, LuaUserVar);
PG_IMPLEMENT_RTTI(AddPlanetHighlightClass, LuaUserVar);
PG_IMPLEMENT_RTTI(RemovePlanetHighlightClass, LuaUserVar);


/**************************************************************************************************
* AddRadarBlipClass::Function_Call -- Script command to add a named blip to the radar map.
*
* In:				
*
* Out:	
*
* History: 9/13/2005 4:57PM JSY
**************************************************************************************************/
LuaTable *AddRadarBlipClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (GameModeManager.Get_Sub_Type() == SUB_GAME_MODE_GALACTIC)
	{
		if (params->Value.size() != 1)
		{
			script->Script_Error("Add_Radar_Blip -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
			return NULL;
		}

		SmartPtr<GameObjectWrapper> radar_object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
		if (!radar_object_wrapper || !radar_object_wrapper->Get_Object() || !radar_object_wrapper->Get_Object()->Behaves_Like(BEHAVIOR_PLANET))
		{
			script->Script_Error("Add_Radar_Blip -- invalid type for parameter 1.  Expected planet.");
			return NULL;
		}

		TheCommandBar.Flash_Galactic_Radar(radar_object_wrapper->Get_Object(), true);
	}
	else
	{
		if (params->Value.size() != 2)
		{
			script->Script_Error("Add_Radar_Blip -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
			return NULL;
		}

		Vector3 radar_position;
		if (!Lua_Extract_Position(params->Value[0], radar_position))
		{
			script->Script_Error("Add_Radar_Blip -- could not deduce a position from parameter 1.");
			return NULL;
		}

		GameObjectClass *radar_object = NULL;
		SmartPtr<GameObjectWrapper> radar_object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
		if (radar_object_wrapper)
		{
			radar_object = radar_object_wrapper->Get_Object();
		}

		SmartPtr<LuaString> blip_tag = PG_Dynamic_Cast<LuaString>(params->Value[1]);
		if (!blip_tag)
		{
			script->Script_Error("Add_Radar_Blip -- invalid type for parameter 2.  Expected strng.");
			return NULL;
		}

		int blip_id = static_cast<int>(CRCClass::Calculate_CRC(blip_tag->Value.c_str(), blip_tag->Value.length()));

		TheCommandBar.Add_Radar_Blip(radar_position, -1, blip_id, false, radar_object);
	}

	return NULL;
}

/**************************************************************************************************
* RemoveRadarBlipClass::Function_Call -- Script command to remove a named blip on the radar map.
*
* In:				
*
* Out:	
*
* History: 9/13/2005 4:57PM JSY
**************************************************************************************************/
LuaTable *RemoveRadarBlipClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("Remove_Radar_Blip -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	if (GameModeManager.Get_Sub_Type() == SUB_GAME_MODE_GALACTIC)
	{
		SmartPtr<GameObjectWrapper> radar_object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
		if (!radar_object_wrapper || !radar_object_wrapper->Get_Object() || !radar_object_wrapper->Get_Object()->Behaves_Like(BEHAVIOR_PLANET))
		{
			script->Script_Error("Add_Radar_Blip -- invalid type for parameter 1.  Expected planet.");
			return NULL;
		}

		TheCommandBar.Flash_Galactic_Radar(radar_object_wrapper->Get_Object(), false);
	}
	else
	{
		SmartPtr<LuaString> blip_tag = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (!blip_tag)
		{
			script->Script_Error("Remove_Radar_Blip -- invalid type for parameter 1.  Expected strng.");
			return NULL;
		}

		int blip_id = static_cast<int>(CRCClass::Calculate_CRC(blip_tag->Value.c_str(), blip_tag->Value.length()));

		TheCommandBar.Remove_Radar_Blip(blip_id);
	}

	return NULL;
}

/**************************************************************************************************
* RemovePlanetHighlightClass::Function_Call -- Script command to add a named highlight cursor to a planet
*
* In:				
*
* Out:	
*
* History: 9/13/2005 6:22PM JSY
**************************************************************************************************/
LuaTable *AddPlanetHighlightClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 2)
	{
		script->Script_Error("Add_Planet_Highlight -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectWrapper> planet_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	if (!planet_wrapper)
	{
		script->Script_Error("Add_Planet_Highlight -- invalid type for parameter 1.  Expected game object.");
		return NULL;
	}

	SmartPtr<LuaString> highlight_tag = PG_Dynamic_Cast<LuaString>(params->Value[1]);
	if (!highlight_tag)
	{
		script->Script_Error("Add_Planet_Highlight -- invalid type for parameter 2.  Expected string.");
		return NULL;
	}

	TheCommandBar.Flash_Planet_Component(planet_wrapper->Get_Object()->Get_Type()->Get_Name()->c_str(), FLASH_TROOPS, -1, highlight_tag->Value.c_str());
	return NULL;
}

/**************************************************************************************************
* RemovePlanetHighlightClass::Function_Call -- Script command to remove a named highlight cursor
*
* In:				
*
* Out:	
*
* History: 9/13/2005 6:22PM JSY
**************************************************************************************************/
LuaTable *RemovePlanetHighlightClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("Remove_Planet_Highlight -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> highlight_tag = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!highlight_tag)
	{
		script->Script_Error("Remove_Planet_Highlight -- invalid type for parameter 1.  Expected strng.");
		return NULL;
	}

	TheCommandBar.Hide_Tutorial_Cursor(highlight_tag->Value.c_str());

	return NULL;
}