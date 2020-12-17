// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindMarker.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindMarker.cpp $
//
//    Original Author: Brian Hayes
//
//            $Author: Brian_Hayes $
//
//            $Change: 641502 $
//
//          $DateTime: 2017/05/09 13:45:27 $
//
//          $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */
#pragma hdrstop

#include "FindMarker.h"
#include "GameModeManager.h"
#include "GameObjectManager.h"
#include "GameObjectTypeManager.h"
#include "AI/LuaScript/GameObjectTypeWrapper.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/GameObjectWrapper.h"



PG_IMPLEMENT_RTTI(LuaFindMarkerCommandClass, LuaUserVar);
PG_IMPLEMENT_RTTI(LuaFindAllHintsCommandClass, LuaUserVar);


// Params : object_type, position, player
LuaTable *LuaFindMarkerCommandClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{

	if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
	{
		script->Script_Error("LuaFindMarkerCommandClass -- Command only valid in a tactical game.");
		return NULL;
	}

	if (params->Value.size() < 1)
	{
		script->Script_Error("LuaFindMarkerCommandClass -- Invalid number of parameters.");
		return NULL;
	}

	SmartPtr<GameObjectTypeWrapper> lua_type = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
	LuaString::Pointer str = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!str && !lua_type)
	{
		script->Script_Error("LuaFindMarkerCommandClass -- Expected type or name for parameter 1");
		return NULL;
	}
	GameObjectTypeClass *type = lua_type ? lua_type->Get_Object() : NULL;
	if (str)
	{
		type = GameObjectTypeManager.Find_Object_Type(str->Value);
	}
	if (!type)
	{
		script->Script_Error("LuaFindMarkerCommandClass -- Unable to resolve type name");
		return NULL;
	}

	LuaString::Pointer hint;
	if (params->Value.size() > 1)
	{
		hint = PG_Dynamic_Cast<LuaString>(params->Value[1]);
	}

	GameObjectClass *found_object = NULL;
	MultiLinkedListIterator<GameObjectClass> it(&GameModeManager.Get_Active_Mode()->Get_Object_Manager().Get_Hint_Objects());
	while (!it.Is_Done())
	{
		if (it.Current_Object()->Get_Original_Object_Type() == type)
		{
			if (hint && hint->Value.size())
			{
				if (_stricmp(hint->Value.c_str(), it.Current_Object()->Get_Hint_Data()->Get_Hint_String().c_str()) == 0)
				{
					found_object = it.Current_Object();
					break;
				}
			}
			else
			{
				found_object = it.Current_Object();
				break;
			}
		}
		it.Next();
	}

	if (found_object)
	{
		return Return_Variable(GameObjectWrapper::Create(found_object, script));
	}

	return NULL;
}

/**************************************************************************************************
* LuaFindAllHintsCommandClass::Function_Call -- Script function to get hold of all objects that have
*	been marked up with a given hint.
*
* In:				
*
* Out:	
*
* History: 8/17/2005 9:19AM JSY
**************************************************************************************************/
LuaTable *LuaFindAllHintsCommandClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("Find_All_Objects_With_Hint -- invalid number of parameters.  Expected 1, got %d.");
		return NULL;
	}

	SmartPtr<LuaString> hint_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!hint_name)
	{
		script->Script_Error("Find_All_Objects_With_Hint -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	SmartPtr<LuaTable> all_objects = Alloc_Lua_Table();
	MultiLinkedListIterator<GameObjectClass> it(&GameModeManager.Get_Active_Mode()->Get_Object_Manager().Get_Hint_Objects());
	for ( ; !it.Is_Done(); it.Next())
	{
		if (_stricmp(hint_name->Value.c_str(), it.Current_Object()->Get_Hint_Data()->Get_Hint_String().c_str()) == 0)
		{
			all_objects->Value.push_back(GameObjectWrapper::Create(it.Current_Object(), script));
		}
	}

	return Return_Variable(all_objects);
}
