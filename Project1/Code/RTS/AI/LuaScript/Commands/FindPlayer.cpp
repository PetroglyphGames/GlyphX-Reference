// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindPlayer.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindPlayer.cpp $
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

#include "FindPlayer.h"

#include "FactionList.h"
#include "PlayerList.h"
#include "AI/LuaScript/PlayerWrapper.h"

PG_IMPLEMENT_RTTI(FindPlayerClass, LuaUserVar);

/**************************************************************************************************
* FindPlayerClass::Function_Call -- get hold of a script wrapper for a player by faction name.
*	Doesn't work so well in tactical multiplayer where there may be multiple players of the same faction.
*
* In:			
*
* Out:		
*
* History: 8/8/2005 4:16PM JSY
**************************************************************************************************/
LuaTable *FindPlayerClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("Find_Player -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> faction_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!faction_name)
	{
		script->Script_Error("Find_Player -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	if (_stricmp(faction_name->Value.c_str(), "local") == 0)
	{
		return Return_Variable(PlayerWrapper::Create(PlayerList.Get_Local_Player(), script));
	}

	const FactionClass *faction = TheFactionTypeConverterPtr->Get_Managed_Object(faction_name->Value);
	if (!faction)
	{
		script->Script_Error("Find_Player -- unknown faction %s.", faction_name->Value.c_str());
		return NULL;
	}

	PlayerClass *player = PlayerList.Get_Player_Of_Faction(faction);
	if (player)
	{
		return Return_Variable(PlayerWrapper::Create(player, script));
	}
	else
	{
		return NULL;
	}
}