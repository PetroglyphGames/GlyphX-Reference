// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindNearest.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindNearest.cpp $
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


#include "FindNearest.h"

#include "GameObject.h"
#include "GameObjectTypeManager.h"
#include "AI/Perception/TacticalPerceptionGrid.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "AI/Planning/TaskForce.h"
#include "AI/Perception/AIPerceptionSystem.h"
#include "Player.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "GameObjectPropertiesType.h"
#include "DynamicEnum.h"
#include "AI/Movement/ObjectTrackingSystem.h"
#include "GameObjectManager.h"

PG_IMPLEMENT_RTTI(FindNearestClass, LuaUserVar);
PG_IMPLEMENT_RTTI(FindNearestSpaceFieldClass, LuaUserVar);

LuaTable *FindNearestClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() < 1 || params->Value.size() > 4)
	{
		script->Script_Error("Find_Nearest - invalid number of parameters.  Expected between 1 and 4, got %d.", params->Value.size());
		return 0;
	}

	if (!GameModeManager.Get_Active_Mode() || !GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical())
	{
		script->Script_Error("Find_Nearest - this function may only be used in tactical modes.");
		return 0;
	}

	SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);
	SmartPtr<AITargetLocationWrapper> ai_target_wrapper = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[0]);
	SmartPtr<TaskForceClass> tf = PG_Dynamic_Cast<TaskForceClass>(params->Value[0]);

	if (!object_wrapper && !ai_target_wrapper && !tf)
	{
		script->Script_Error("Find_Nearest - invalid type for parameter 1; expected game object, ai target or taskforce.");
		return 0;
	}

	GameObjectClass *exclude_object = 0;
	Vector3 from_position;
	if (object_wrapper && object_wrapper->Get_Object())
	{
		exclude_object = object_wrapper->Get_Object();
		from_position = exclude_object->Get_Position();
	}
	else if (ai_target_wrapper && ai_target_wrapper->Get_Object())
	{
		exclude_object = ai_target_wrapper->Get_Object()->Get_Target_Game_Object();
		from_position = ai_target_wrapper->Get_Object()->Get_Target_Position();
	}
	else if (tf)
	{
		from_position = tf->Get_Position();
	}
	else
	{
		script->Script_Error("Find_Nearest - parameter 1 is already dead; cannot extract a position from it.");
		return 0;
	}

	const GameObjectTypeClass *filter_type = 0;
	PlayerClass *allegiance_player = 0;
	bool match_allegiance = true;
	GameObjectPropertiesType property_filter = GAME_OBJECT_PROPERTIES_ALL;
	GameObjectCategoryType category_filter = GAME_OBJECT_CATEGORY_ALL;
	if (params->Value.size() > 1)
	{
		unsigned int parameter_index = 1;
		SmartPtr<LuaString> type_name = PG_Dynamic_Cast<LuaString>(params->Value[parameter_index]);
		if (type_name)
		{
			if (!TheGameObjectPropertiesTypeConverterPtr->String_To_Enum(type_name->Value.c_str(), property_filter))
			{
				if (!TheGameObjectCategoryTypeConverterPtr->String_To_Enum(type_name->Value.c_str(), category_filter))
				{
					filter_type = GameObjectTypeManager.Find_Object_Type(type_name->Value);
					if (!filter_type)
					{
						script->Script_Error("Find_Nearest - unknown game object type, properties mask or catefory %s.", type_name->Value.c_str());
						return 0;
					}
				}
			}
			++parameter_index;
		}

		if (params->Value.size() > parameter_index + 1)
		{
			SmartPtr<PlayerWrapper> lua_allegiance_player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[parameter_index]);
			if (lua_allegiance_player)
			{
				allegiance_player = lua_allegiance_player->Get_Object();
			}

			SmartPtr<LuaBool> lua_match_allegiance = PG_Dynamic_Cast<LuaBool>(params->Value[parameter_index + 1]);
			if (lua_match_allegiance)
			{
				match_allegiance = lua_match_allegiance->Value;
				parameter_index += 2;
			}
		}
	}

	GameObjectClass *best_object = 0;
	float min_distance2 = BIG_FLOAT;

	for (int i = 0; i < PlayerList.Get_Num_Players(); ++i)
	{
		PlayerClass *player = PlayerList.Get_Player_By_Index(i);
		if (!player)
		{
			continue;
		}

		if (player->Is_Neutral())
		{
			continue;
		}


		if (allegiance_player && player->Is_Ally(allegiance_player) != match_allegiance)
		{
			continue;
		}

		MultiLinkedListIterator<GameObjectClass> it(AIPerceptionSystemClass::Get_Perception_Grid()->Get_Complete_Object_List(i));
		
		for (; !it.Is_Done(); it.Next())
		{
			GameObjectClass *object = it.Current_Object();

			if (object->Is_Dead())
			{
				continue;
			}

			if (object == exclude_object)
			{
				continue;
			}

			if (filter_type && object->Get_Original_Object_Type() != filter_type)
			{
				continue;
			}

			if (property_filter != GAME_OBJECT_PROPERTIES_ALL && (property_filter & object->Get_Original_Object_Type()->Get_Property_Mask()) == 0)
			{
				continue;
			}

			if ((category_filter & object->Get_Original_Object_Type()->Get_Category_Mask()) == 0)
			{
				continue;
			}

			if (allegiance_player && GameModeManager.Get_Active_Mode()->Is_Fogged(allegiance_player->Get_ID(), object, true))
			{
				continue;
			}

			float distance2 = (object->Get_Position() - from_position).Length2();
			if (distance2 < min_distance2)
			{
				min_distance2 = distance2;
				best_object = object;
			}
		}
	}

	if (best_object)
	{
		return Return_Variable(GameObjectWrapper::Create(best_object, script));
	}

	return 0;
}

/**************************************************************************************************
* FindNearestSpaceFieldClass::Function_Call -- Script function to find the nearst asteroid field,
*	nebula or ion storm.
*
* In:			
*
* Out:		
*
* History: 8/31/2005 9:53AM JSY
**************************************************************************************************/
LuaTable *FindNearestSpaceFieldClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_SPACE)
	{
		script->Script_Error("Find_Nearest_Space_Field -- this function may only be used in space mode.");
		return NULL;
	}

	if (params->Value.size() < 1 || params->Value.size() > 2)
	{
		script->Script_Error("Find_Nearest_Space_Field -- invalid number of paraneters.  Expected 1 or 21, got %d.", params->Value.size());
		return NULL;
	}

	//First parameter is position we must be nearest to
	Vector3 source_position;
	if (!Lua_Extract_Position(params->Value[0], source_position))
	{
		script->Script_Error("Find_Nearest_Space_Field -- could not deduce source position from parameter 1");
		return NULL;
	}

	//Optional 2nd parameter is collision mask for field types to find.
	SpaceCollisionType space_field_mask = static_cast<SpaceCollisionType>(SCT_ASTEROID_FIELD | SCT_ION_STORM | SCT_NEBULA);
	if (params->Value.size() == 2)
	{
		SmartPtr<LuaString> collision_string = PG_Dynamic_Cast<LuaString>(params->Value[1]);
		if (!collision_string)
		{
			script->Script_Error("Find_Nearest_Space_Field -- invalid type for parameter 2.  Expected string.");
			return NULL;
		}

		if (!TheSpaceCollisionTypeConverterPtr->String_To_Enum(collision_string->Value, space_field_mask))
		{
			script->Script_Error("Find_Nearest_Space_Field -- unknown field type %s.", collision_string->Value.c_str());
			return NULL;
		}
	}

	//Find all possible fields
	static std::vector<ObjectIDType> object_list;
	object_list.resize(0);
	ObjectTrackingSystemClass *tracking_system = GameModeManager.Get_Active_Mode()->Get_Object_Tracking_System();
	tracking_system->Build_Layer_Objects(SLT_STATIC_OBJECT, object_list);

	float best_distance2 = BIG_FLOAT;
	GameObjectClass *best_object = NULL;
	for (unsigned int i = 0; i < object_list.size(); ++i)
	{
		GameObjectClass *object = GAME_OBJECT_MANAGER.Get_Object_From_ID(object_list[i]);
		if (!object)
		{
			continue;
		}

		//Check we have a space field of the appropriate type
		const GameObjectTypeClass *type = object->Get_Type();
		if ((type->Is_Nebula() && (space_field_mask & SCT_NEBULA) != 0) ||
				(type->Is_Asteroid_Field() && (space_field_mask & SCT_ASTEROID_FIELD) != 0) ||
				(type->Is_Ion_Storm() && (space_field_mask & SCT_ION_STORM) != 0))
		{
			float distance2 = (object->Get_Position() - source_position).Project_XY().Length2();
			if (distance2 < best_distance2)
			{
				best_distance2 = distance2;
				best_object = object;
			}
		}
	}

	if (best_object)
	{
		return Return_Variable(GameObjectWrapper::Create(best_object, script));
	}
	else
	{
		return NULL;
	}
}