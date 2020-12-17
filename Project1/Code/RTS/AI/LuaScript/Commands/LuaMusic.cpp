// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaMusic.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaMusic.cpp $
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

#include "LuaMusic.h"

#include "MusicEventManager.h"
#include "GameModeManager.h"

PG_IMPLEMENT_RTTI(LuaPlayMusicClass, LuaUserVar);
PG_IMPLEMENT_RTTI(LuaStopAllMusicClass, LuaUserVar);
PG_IMPLEMENT_RTTI(LuaResumeModeBasedMusicClass, LuaUserVar);

/**************************************************************************************************
* LuaPlayMusicClass::Function_Call -- Play a named music event
*
* In:				
*
* Out:	
*
* History: 9/06/2005 10:57AM JSY
**************************************************************************************************/
LuaTable *LuaPlayMusicClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("Play_Music -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> music_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!music_name)
	{
		script->Script_Error("Play_Music -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	TheMusicEventManager.Start_Music_Event(music_name->Value, MUSIC_MODE_SCRIPTED, false, true);

	return NULL;
}

/**************************************************************************************************
* LuaStopAllMusicClass::Function_Call -- Stop music from playing
*
* In:				
*
* Out:	
*
* History: 9/06/2005 10:57AM JSY
**************************************************************************************************/
LuaTable *LuaStopAllMusicClass::Function_Call(LuaScriptClass *, LuaTable *)
{
	TheMusicEventManager.System_Kill_All_Active_Events();

	return NULL;
}

/**************************************************************************************************
* LuaResumeModeBasedMusicClass::Function_Call -- Restart the normal music for this mode.
*
* In:				
*
* Out:	
*
* History: 9/06/2005 4:59PM JSY
**************************************************************************************************/
LuaTable *LuaResumeModeBasedMusicClass::Function_Call(LuaScriptClass *, LuaTable *)
{
	GameModeManager.Start_Current_Mode_Based_Ambient_Music(true);

	return NULL;
}