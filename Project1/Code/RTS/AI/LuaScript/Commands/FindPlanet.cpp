// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindPlanet.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindPlanet.cpp $
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
#include "FindPlanet.h"
#include "GameObject.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "GameObjectTypeManager.h"
#include "GameObjectType.h"
#include "GameObjectManager.h"


PG_IMPLEMENT_RTTI(FindPlanetClass, LuaUserVar);


FindPlanetClass::FindPlanetClass()
{
	LUA_REGISTER_MEMBER_FUNCTION(FindPlanetClass, "Get_All_Planets", &FindPlanetClass::Get_All_Planets);
}

/**
 * LuaFunction to find the planet of the given name.
 * 
 * @param script LuaScript object
 * @param params Lua parameters passed into this function.  
 * 
 * @return Null or a GameObjectWrapper of the planet.
 * @since 4/30/2004 11:52:40 AM -- BMH
 */
LuaTable* FindPlanetClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() == 0) {
		script->Script_Error("FindPlanet -- No planet name set");
		return NULL;
	}

	LuaString *str = PG_Dynamic_Cast<LuaString>(params->Value[0]);

	if (!str) {
		script->Script_Error("FindPlanet -- Parameter 1 is not a valid string");
		return NULL;
	}

	GameObjectClass *planet = Get_Planet(str->Value.c_str());
	if (planet) {
		return Return_Variable(GameObjectWrapper::Create(planet, script));
	}
	return NULL;
}

/**
 * Finds the GameObjectClass for the given planet name.
 * 
 * @param name   Name of the planet.
 * 
 * @return GameObjectClass pointer to the planet, or NULL if not found.
 * @since 4/28/2004 2:24:04 PM -- BMH
 */
GameObjectClass *FindPlanetClass::Get_Planet(const char *name)
{
	GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
	if (!type) return NULL;
	
	const DynamicVectorClass<GameObjectClass *> *planets = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(type);
	if (planets->Count())
	{
		return (*planets)[0];
	}
	return NULL;
}

LuaTable *FindPlanetClass::Get_All_Planets(LuaScriptClass *script, LuaTable *)
{
	const DynamicVectorClass<GameObjectClass*> *planets = GALACTIC_GAME_OBJECT_MANAGER.Find_Objects(BEHAVIOR_PLANET);

	SmartPtr<LuaTable> result = Alloc_Lua_Table();

	for (int i = 0; i < planets->Size(); ++i)
	{
		result->Value.push_back(GameObjectWrapper::Create(planets->Get_At(i), script));
	}

	return Return_Variable(result);
}