// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SFXCommands.cpp#3 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SFXCommands.cpp $
//
//    Original Author: Brian Hayes
//
//            $Author: Brian_Hayes $
//
//            $Change: 641585 $
//
//          $DateTime: 2017/05/10 10:42:50 $
//
//          $Revision: #3 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */
#pragma hdrstop

#include "SFXCommands.h"
#include "SFXEventManager.h"


PG_IMPLEMENT_RTTI(LuaSFXCommandsClassClass, LuaUserVar);

LuaSFXCommandsClassClass::LuaSFXCommandsClassClass()
{
	LUA_REGISTER_MEMBER_FUNCTION(LuaSFXCommandsClassClass, "Allow_Localized_SFXEvents", &LuaSFXCommandsClassClass::Allow_Localized_SFXEvents);
	LUA_REGISTER_MEMBER_FUNCTION(LuaSFXCommandsClassClass, "Allow_Unit_Reponse_VO", &LuaSFXCommandsClassClass::Allow_Unit_Reponse_VO);
	LUA_REGISTER_MEMBER_FUNCTION(LuaSFXCommandsClassClass, "Allow_HUD_VO", &LuaSFXCommandsClassClass::Allow_HUD_VO);
	LUA_REGISTER_MEMBER_FUNCTION(LuaSFXCommandsClassClass, "Allow_Ambient_VO", &LuaSFXCommandsClassClass::Allow_Ambient_VO);
	LUA_REGISTER_MEMBER_FUNCTION(LuaSFXCommandsClassClass, "Allow_Enemy_Sighted_VO", &LuaSFXCommandsClassClass::Allow_Enemy_Sighted_VO);
}


LuaTable *LuaSFXCommandsClassClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	script; params;
	return Return_Variable(this);
}


LuaTable *LuaSFXCommandsClassClass::Allow_Localized_SFXEvents(LuaScriptClass *script, LuaTable *params)
{
	script;
	bool enable = true;
	if (params->Value.size())
	{
		LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
		enable = bval->Value;
	}

	TheSFXEventManager.Allow_Localized_SFXEvents(enable, SFXEVENT_SYSTEM_ALLOW_LUA_SCRIPT_SFX_COMMAND);
	return NULL;
}


LuaTable *LuaSFXCommandsClassClass::Allow_Unit_Reponse_VO(LuaScriptClass *script, LuaTable *params)
{
	script;
	bool enable = true;
	if (params->Value.size())
	{
		LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
		enable = bval->Value;
	}

	TheSFXEventManager.Allow_Unit_Reponse_VO(enable, SFXEVENT_SYSTEM_ALLOW_LUA_SCRIPT_SFX_COMMAND);
	return NULL;
}


LuaTable *LuaSFXCommandsClassClass::Allow_HUD_VO(LuaScriptClass *script, LuaTable *params)
{
	script;
	bool enable = true;
	if (params->Value.size())
	{
		LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
		enable = bval->Value;
	}

	TheSFXEventManager.Allow_HUD_VO(enable, SFXEVENT_SYSTEM_ALLOW_LUA_SCRIPT_SFX_COMMAND);
	return NULL;
}

LuaTable *LuaSFXCommandsClassClass::Allow_Ambient_VO(LuaScriptClass *script, LuaTable *params)
{
	script;
	bool enable = true;
	if (params->Value.size())
	{
		LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
		enable = bval->Value;
	}

	TheSFXEventManager.Allow_Ambient_VO(enable, SFXEVENT_SYSTEM_ALLOW_LUA_SCRIPT_SFX_COMMAND);
	return NULL;
}

LuaTable *LuaSFXCommandsClassClass::Allow_Enemy_Sighted_VO(LuaScriptClass *, LuaTable *params)
{
	bool enable = true;
	if (params->Value.size())
	{
		LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
		enable = bval->Value;
	}

	//Set the sighted enemy flag without playing the sound to prevent it from being played naturally.
	if (GameModeManager.Get_Active_Mode())
	{
		GameModeManager.Get_Active_Mode()->Force_Set_Sighted_Enemy(!enable);
	}

	return NULL;
}
