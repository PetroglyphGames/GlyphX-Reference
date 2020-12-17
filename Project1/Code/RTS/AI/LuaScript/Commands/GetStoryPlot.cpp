// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GetStoryPlot.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GetStoryPlot.cpp $
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

#include "GetStoryPlot.h"

#include "AI/LuaScript/StoryPlotWrapper.h"
#include "StoryMode/StoryMode.h"
#include "StoryMode/StorySubPlot.h"
#include "GameModeManager.h"
#include "GameMode.h"

PG_IMPLEMENT_RTTI(GetStoryPlotClass, LuaUserVar);

LuaTable *GetStoryPlotClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("Get_Story_Plot -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> plot_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!plot_name)
	{
		script->Script_Error("Get_Story_Plot -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	StorySubPlotClass *plot = GameModeManager.Get_Active_Mode()->Get_Story_Mode().Get_Sub_Plot(plot_name->Value.c_str());
	if (!plot)
	{
		script->Script_Warning("Get_Story_Plot -- plot %s not found.", plot_name->Value.c_str());
		return NULL;
	}

	return Return_Variable(StoryPlotWrapper::Create(plot, script));
}