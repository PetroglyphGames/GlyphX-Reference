// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/LuaRTSUtilities.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/LuaRTSUtilities.cpp $
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

#include "Always.h"

#include "LuaRTSUtilities.h"

#include "AI/AILog.h"
#include "AI/TheAIDataManager.h"
#include "GameObjectWrapper.h"
#include "PlayerWrapper.h"
#include "GameObjectTypeWrapper.h"
#include "AITargetLocationWrapper.h"
#include "MegaFileManager.h"
#include "AI/Planning/TaskForce.h"
#include "PositionWrapper.h"

#ifndef NDEBUG

/**************************************************************************************************
* Lua_Callback_Log_Message -- Debug callback for logging messages from the script system
*
* In:				
*
* Out:		
*
* History: 1/18/2006 10:04AM JSY
**************************************************************************************************/
void Lua_Callback_Log_Message(const char *message)
{
	AILOG_CONTEXT_BEGIN("SCRIPT", 0);
	AIMESSAGE((message));
	AILOG_CONTEXT_END();
}

/**************************************************************************************************
* Lua_Callback_Log_Warning -- Debug callback for logging warnings from the script system
*
* In:				
*
* Out:		
*
* History: 1/18/2006 10:04AM JSY
**************************************************************************************************/
void Lua_Callback_Log_Warning(const char *warning_message)
{
	AIWARNING((warning_message));
}

/**************************************************************************************************
* Lua_Callback_Log_Error -- Debug callback for logging errors from the script system
*
* In:				
*
* Out:		
*
* History: 1/18/2006 10:04AM JSY
**************************************************************************************************/
void Lua_Callback_Log_Error(const char *error_message)
{
	AIERROR((error_message));
}

/**************************************************************************************************
* Lua_Callback_Register_Script_File -- Debug callback for registering a script file to support hot reloading
*
* In:				
*
* Out:		
*
* History: 1/18/2006 10:04AM JSY
**************************************************************************************************/
void Lua_Callback_Register_Script_File(const char *full_file_name)
{
	TheAIDataManagerClass::Get().Register_AI_File(full_file_name);
}

/**************************************************************************************************
* Lua_Callback_Register_Script_File -- Debug callback for unregistering a script file to support hot reloading
*
* In:				
*
* Out:		
*
* History: 1/18/2006 10:05AM JSY
**************************************************************************************************/
void Lua_Callback_Unregister_Script_File(const char *full_file_name)
{
	TheAIDataManagerClass::Get().Unregister_AI_File(full_file_name);
}

#endif //NDEBUG


/**************************************************************************************************
* Lua_System_Initialize -- Handles initialization of both library and engine level LUA systems
*
* In:				
*
* Out:		
*
* History: 1/18/2006 10:05AM JSY
**************************************************************************************************/
void Lua_System_Initialize()
{
	LuaScriptClass::System_Initialize();

	PlayerWrapper::Init_Wrapper_Cache();
	GameObjectTypeWrapper::Init_Wrapper_Cache();
	GameObjectWrapper::Init_Wrapper_Cache();
	AITargetLocationWrapper::Init_Wrapper_Cache();

#ifndef NDEBUG
	LuaScriptClass::Install_Log_Message_Callback(Lua_Callback_Log_Message);
	LuaScriptClass::Install_Log_Error_Callback(Lua_Callback_Log_Error);
	LuaScriptClass::Install_Log_Warning_Callback(Lua_Callback_Log_Warning);
	LuaScriptClass::Install_Register_Script_Callback(Lua_Callback_Register_Script_File);
	LuaScriptClass::Install_Unregister_Script_Callback(Lua_Callback_Unregister_Script_File);

	// Lua_Validate_All_Scripts();
#endif  //NDEBUG
}

/**************************************************************************************************
* Lua_System_Shutdown -- Handles shutdown of both library and engine level LUA systems
*
* In:				
*
* Out:		
*
* History: 1/18/2006 10:05AM JSY
**************************************************************************************************/
void Lua_System_Shutdown()
{
	LuaScriptClass::System_Shutdown();

	AITargetLocationWrapper::Shutdown_Wrapper_Cache();
	GameObjectWrapper::Shutdown_Wrapper_Cache();
	GameObjectTypeWrapper::Shutdown_Wrapper_Cache();
	PlayerWrapper::Shutdown_Wrapper_Cache();

	LuaScriptClass::Install_Log_Message_Callback(NULL);
	LuaScriptClass::Install_Log_Error_Callback(NULL);
	LuaScriptClass::Install_Log_Warning_Callback(NULL);
	LuaScriptClass::Install_Register_Script_Callback(NULL);
	LuaScriptClass::Install_Unregister_Script_Callback(NULL);
}

/**************************************************************************************************
* Lua_System_Reset -- Convenience function since we seem to need to reset LUA from a handful of places
*
* In:				
*
* Out:		
*
* History: 1/18/2006 10:05AM JSY
**************************************************************************************************/
void Lua_System_Reset()
{
	//Just shutdown and reinitialize
	Lua_System_Shutdown();
	Lua_System_Initialize();
}

/**************************************************************************************************
* Lua_Validate_All_Scripts -- Forces loading (and therefore basic error checking) of all engine level scripts.
*
* In:				
*
* Out:		
*
* History: 1/18/2006 10:07AM JSY
**************************************************************************************************/
void Lua_Validate_All_Scripts()
{
	//We need the megafile manager in place to do this
	FAIL_IF(!TheMegaFileManager) { return; }

	static std::string ALL_SCRIPT_PATHS[] = {
														"./Data/Scripts/Library/",
														"./Data/Scripts/GameObject/",
														"./Data/Scripts/FreeStore/",
														"./Data/Scripts/Miscellaneous/",
														"./Data/Scripts/Story/",
														"./Data/Scripts/Evaluators/",
														"./Data/Scripts/AI/",
														"./Data/Scripts/AI/SpaceMode/",
														"./Data/Scripts/AI/LandMode/",
														"./Data/Scripts/AI/StoryArc/",
														"./Data/Scripts/Interventions/",
														"./Data/Scripts/AI/PerceptualEquations/", };

	for (int i = 0; i < ARRAY_SIZE(ALL_SCRIPT_PATHS); i++) 
	{
		LuaScriptClass::Add_Script_Path(ALL_SCRIPT_PATHS[i].c_str());
	}

	LuaScriptClass::Validate_All_Scripts();
}

/**************************************************************************************************
* Lua_Extract_Position -- Utility function to extract a world position from a LuaVar (if possible)
*
* In:			LuaVar to extract position from
*				Output position.
*
* Out:		Did we successfully determine a position?
*
* History: 1/18/2006 10:09AM JSY
**************************************************************************************************/
bool Lua_Extract_Position(LuaVar *var, Vector3 &position, Vector3 *facing /*= NULL*/)
{
	SmartPtr<AITargetLocationWrapper> ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(var);
	SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(var);
	SmartPtr<TaskForceClass> tf = PG_Dynamic_Cast<TaskForceClass>(var);
	SmartPtr<PositionWrapper> position_wrapper = PG_Dynamic_Cast<PositionWrapper>(var);

	if (object_wrapper)
	{
		if (!object_wrapper->Get_Object())
		{
			return false;
		}

		position = object_wrapper->Get_Object()->Get_Position();
		if (facing)
		{
			*facing = *object_wrapper->Get_Object()->Get_Facing().Get();
		}
	}
	else if (ai_target_wrapper)
	{
		if (!ai_target_wrapper->Get_Object())
		{
			return false;
		}

		position = ai_target_wrapper->Get_Object()->Get_Target_Position();
		if (facing && ai_target_wrapper->Get_Object()->Get_Target_Game_Object())
		{
			*facing = *ai_target_wrapper->Get_Object()->Get_Target_Game_Object()->Get_Facing().Get();
		}
	}
	else if (tf)
	{
		position = tf->Get_Position();
	}
	else if (position_wrapper)
	{
		position = position_wrapper->Get_Position();
	}
	else
	{
		return false;
	}

	return true;
}