// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/EnableDistanceFog.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/EnableDistanceFog.cpp $
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

#include "Always.h"

#include "EnableDistanceFog.h"
#include "GameModeManager.h"
#include "RtsScene.h"

PG_IMPLEMENT_RTTI(EnableDistanceFogClass, LuaUserVar);

/**************************************************************************************************
* EnableDistanceFogClass::Function_Call -- Toggle fogging of objects in the distance
*
* In:			
*
* Out:		
*
* History: 8/4/2006 6:19PM -- JSY
**************************************************************************************************/
LuaTable *EnableDistanceFogClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	//Single boolean parameter
	if (params->Value.size() != 1)
	{
		script->Script_Error("Enable_Distance_Fog -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaBool> enable = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
	if (!enable)
	{
		script->Script_Error("Enable_Distance_Fog -- invalid type for parameter 1.  Expected boolean.");
		return NULL;
	}

	//Make sure we're in a game mode that supports this functionality
	GameModeClass *mode = GameModeManager.Get_Active_Mode();
	if (!mode)
	{
		script->Script_Error("Enable_Distance_Fog -- no active game mode!");
		return NULL;
	}
	
	RtsSceneClass *rts_scene = PG_Dynamic_Cast<RtsSceneClass>(mode->Get_Scene());
	if (!rts_scene)
	{
		script->Script_Error("Enable_Distance_Fog -- this command is not supported in the current game mode.");
		return NULL;
	}

	rts_scene->Set_Environment_Fog_Enabled(enable->Value);

	return NULL;
}

