// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GetNextBaseType.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GetNextBaseType.cpp $
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


#include "GetNextBaseType.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/GameObjectTypeWrapper.h"
#include "GameObject.h"
#include "GameObjectType.h"
#include "PlanetaryBehavior.h"
#include "Faction.h"
#include "GameObjectTypeManager.h"

PG_IMPLEMENT_RTTI(GetNextStarbaseTypeClass, LuaUserVar);
PG_IMPLEMENT_RTTI(GetNextGroundbaseTypeClass, LuaUserVar);

LuaTable *GetNextStarbaseTypeClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GetNextStarbaseType : invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectWrapper> planet = LUA_SAFE_CAST(GameObjectWrapper, params->Value[0]);
	if (!planet || !planet->Get_Object()->Behaves_Like(BEHAVIOR_PLANET))
	{
		script->Script_Error("GetNextStarbaseType : invalid parameter type.  Expected planet game object.");
		return NULL;
	}

	PlanetaryBehaviorClass *behave = static_cast<PlanetaryBehaviorClass*>(planet->Get_Object()->Get_Behavior(BEHAVIOR_PLANET));

	const GameObjectTypeClass *next_base = behave->Get_Next_Buildable_Starbase_Type(planet->Get_Object());

	if (next_base)
	{
		return Return_Variable(GameObjectTypeWrapper::Create(const_cast<GameObjectTypeClass*>(next_base), script));
	}

	return NULL;
}

LuaTable *GetNextGroundbaseTypeClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("GetNextGroundbaseType : invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectWrapper> planet = LUA_SAFE_CAST(GameObjectWrapper, params->Value[0]);
	if (!planet || !planet->Get_Object()->Behaves_Like(BEHAVIOR_PLANET))
	{
		script->Script_Error("GetNextGroundbaseType : invalid parameter type.  Expected planet game object.");
		return NULL;
	}

	PlanetaryBehaviorClass *behave = static_cast<PlanetaryBehaviorClass*>(planet->Get_Object()->Get_Behavior(BEHAVIOR_PLANET));

	const GameObjectTypeClass *next_base = behave->Get_Next_Buildable_Ground_Base_Type(planet->Get_Object());

	if (next_base)
	{
		return Return_Variable(GameObjectTypeWrapper::Create(const_cast<GameObjectTypeClass*>(next_base), script));
	}

	return NULL;
}