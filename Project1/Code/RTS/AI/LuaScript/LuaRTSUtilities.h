// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/LuaRTSUtilities.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/LuaRTSUtilities.h $
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

#ifndef _LUA_RTS_UTILITIES_H_
#define _LUA_RTS_UTILITIES_H_

#include "LuaScript.h"

enum LuaRTSChunkId
{
	LUA_CHUNK_TASK_FORCE = LUA_USER_APP_BEGIN,
	LUA_CHUNK_GAMEOBJECT_WRAPPER,
	LUA_CHUNK_PLAYER_WRAPPER,
	LUA_CHUNK_GAMEOBJECTTYPE_WRAPPER,
	LUA_CHUNK_PRODUCTION_BLOCK,
	LUA_CHUNK_PRODUCEOBJECT_CHUNK,
	LUA_CHUNK_FORMATION_BLOCK,
	LUA_CHUNK_MOVEMENT_BLOCK,
	LUA_CHUNK_POLITICAL_CONTROL_BLOCK,
	LUA_CHUNK_INVASION_BLOCK,
	LUA_CHUNK_PRODUCE_FORCE_BLOCK,
	LUA_CHUNK_LAND_UNITS_BLOCK,
	LUA_CHUNK_FOREVER_BLOCK,
	LUA_CHUNK_BUDGET,
	LUA_CHUNK_BUDGET_BLOCK,
	LUA_CHUNK_AI_TARGET_LOCATION_WRAPPER,
	LUA_CHUNK_GIVE_DESIRE_BLOCK,
	LUA_CHUNK_BASE_LEVEL_BLOCK,
	LUA_CHUNK_WEIGHTED_TYPE_LIST,
	LUA_CHUNK_FREE_MOVEMENT_BLOCK,
	LUA_CHUNK_GALACTIC_TASK_FORCE,
	LUA_CHUNK_SPACE_TASK_FORCE,
	LUA_CHUNK_SPACE_MOVEMENT_BLOCK,
	LUA_CHUNK_LAND_TASK_FORCE,
	LUA_CHUNK_LAND_MOVEMENT_BLOCK,
	LUA_CHUNK_REINFORCE_BLOCK,
	LUA_CHUNK_APPLY_MARKUP_BLOCK,
	LUA_CHUNK_BOMBING_BLOCK,
	LUA_CHUNK_SPACE_AMBUSH_BLOCK,
	LUA_CHUNK_TACTICAL_BUILD_BLOCK,
	LUA_CHUNK_SPACE_REINFORCE_BLOCK,
	LUA_CHUNK_POSITION_WRAPPER,
	LUA_CHUNK_STORYPLOT_WRAPPER,
	LUA_CHUNK_STORYEVENT_WRAPPER,
	LUA_CHUNK_DISCRETE_DISTRIBUTION,
	LUA_CHUNK_UNIT_MOVEMENT_BLOCK,
	LUA_CHUNK_ABILITY_BLOCK,
	LUA_CHUNK_UNIT_ANIMATION_BLOCK,
	LUA_CHUNK_GENERIC_SIGNAL_BLOCK,
	LUA_CHUNK_FOW_CELLS,
	LUA_CHUNK_LIGHTNING_BLOCK,
	LUA_CHUNK_BINK_MOVIE_BLOCK,
	LUA_CHUNK_EXPLORE_AREA_BLOCK,	
	LUA_CHUNK_PERCEPTION_EVALUATOR_WRAPPER,
};

#ifndef NDEBUG
void Lua_Callback_Log_Message(const char *message);
void Lua_Callback_Log_Warning(const char *warning_message);
void Lua_Callback_Log_Error_Callback(const char *error_message);
void Lua_Callback_Register_Script_File(const char *full_file_name);
void Lua_Callback_Unregister_Script_File(const char *full_file_name);
#endif //NDEBUG

void Lua_System_Initialize();
void Lua_System_Shutdown();
void Lua_System_Reset();
void Lua_Validate_All_Scripts();

bool Lua_Extract_Position(LuaVar *var, Vector3 &position, Vector3 *facing = NULL);

#endif //_LUA_RTS_UTILITIES_H_