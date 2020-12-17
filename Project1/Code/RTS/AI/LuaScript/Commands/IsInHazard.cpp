// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/IsInHazard.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/IsInHazard.cpp $
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

#include "IsInHazard.h"
#include "GameObject.h"
#include "AI/Movement/ObjectTrackingSystem.h"
#include "GameObjectManager.h"
#include "OrientedBox2.h"

PG_IMPLEMENT_RTTI(IsPointInNebulaClass, LuaUserVar);
PG_IMPLEMENT_RTTI(IsPointInIonStormClass, LuaUserVar);
PG_IMPLEMENT_RTTI(IsPointInAsteroidFieldClass, LuaUserVar);

/**************************************************************************************************
* Get_Static_Colliders -- Helper function to get the list of objects whose oriented boxes contain
*	the point defined by the lua parameter
*
* In:			
*
* Out:		
*
* History: 8/9/2005 7:55PM JSY
**************************************************************************************************/
void Get_Static_Colliders(LuaScriptClass *script, LuaTable *params, std::vector<GameObjectClass*> &objects)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("Get_Static_Colliders -- invalid number of parameters.  Expected 1, got %d.");
		return;
	}

	Vector3 query_position;
	Vector2 projected_position;
	if (!Lua_Extract_Position(params->Value[0], query_position))
	{
		script->Script_Error("Get_Static_Colliders -- invalid type for parameter 1.  Expected something that can define a position.");
		return;
	}
	projected_position = query_position.Project_XY();

	GameModeClass *mode = GameModeManager.Get_Active_Mode();
	FAIL_IF(!mode) { return; }
	if (!mode->Get_Object_Tracking_System())
	{
		script->Script_Error("Get_Static_Colliders -- function is nor supported in this game mode.");
		return;
	}

	static std::vector<ObjectIDType> object_ids;
	object_ids.resize(0);
	mode->Get_Object_Tracking_System()->Build_Layer_Objects(SLT_STATIC_OBJECT, object_ids);

	for (unsigned int i = 0; i < object_ids.size(); ++i)
	{
		GameObjectClass *possible_collision = GAME_OBJECT_MANAGER.Get_Object_From_ID(object_ids[i]);
		FAIL_IF(!possible_collision) { continue; }

		OrientedBox2Class oriented_box = possible_collision->Build_Oriented_Box();

		if (oriented_box.Contains(projected_position))
		{
			objects.push_back(possible_collision);
		}
	}
}

/**************************************************************************************************
* IsPointInNebulaClass::Function_Call -- Script function to discover whether a point is inside a nebula 
*
* In:			
*
* Out:		
*
* History: 8/9/2005 7:55PM JSY
**************************************************************************************************/
LuaTable *IsPointInNebulaClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	static std::vector<GameObjectClass*> objects;
	objects.resize(0);
	Get_Static_Colliders(script, params, objects);
	for (unsigned int i = 0; i < objects.size(); ++i)
	{
		if (objects[i]->Get_Type()->Is_Nebula())
		{
			return Return_Variable(new LuaBool(true));
		}
	}

	return Return_Variable(new LuaBool(false));
}

/**************************************************************************************************
* IsPointInIonStormClass::Function_Call -- Script function to discover whether a point is inside an ion storm
*
* In:			
*
* Out:		
*
* History: 8/9/2005 7:55PM JSY
**************************************************************************************************/
LuaTable *IsPointInIonStormClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	static std::vector<GameObjectClass*> objects;
	objects.resize(0);
	Get_Static_Colliders(script, params, objects);
	for (unsigned int i = 0; i < objects.size(); ++i)
	{
		if (objects[i]->Get_Type()->Is_Ion_Storm())
		{
			return Return_Variable(new LuaBool(true));
		}
	}

	return Return_Variable(new LuaBool(false));
}

/**************************************************************************************************
* IsPointInAsteroidFieldClass::Function_Call -- Script function to discover whether a point is inside an
*	asteroid field
*
* In:			
*
* Out:		
*
* History: 8/9/2005 7:55PM JSY
**************************************************************************************************/
LuaTable *IsPointInAsteroidFieldClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	static std::vector<GameObjectClass*> objects;
	objects.resize(0);
	Get_Static_Colliders(script, params, objects);
	for (unsigned int i = 0; i < objects.size(); ++i)
	{
		if (objects[i]->Get_Type()->Is_Asteroid_Field())
		{
			return Return_Variable(new LuaBool(true));
		}
	}

	return Return_Variable(new LuaBool(false));
}