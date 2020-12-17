// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GlobalCommands.cpp#3 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GlobalCommands.cpp $
//
//    Original Author: Brian Hayes
//
//            $Author: Brian_Hayes $
//
//            $Change: 747268 $
//
//          $DateTime: 2020/10/27 14:46:31 $
//
//          $Revision: #3 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */

#pragma hdrstop
#include "GlobalCommands.h"
#include "ProduceObject.h"
#include "FindPlanet.h"
#include "FindTarget.h"
#include "ForeverBlock.h"
#include "FindStageArea.h"
#include "EvaluatePerception.h"
#include "GiveDesireBonus.h"
#include "GetNextBaseType.h"
#include "BaseBlock.h"
#include "EvaluateTypeList.h"
#include "WeightedTypeList.h"
#include "FindDeadlyEnemy.h"
#include "PurgeGoals.h"
#include "ApplyMarkup.h"
#include "LuaFindPath.h"
#include "LuaStoryEvent.h"
#include "EvaluateGalacticContext.h"
#include "IsCampaignGame.h"
#include "FindNearest.h"
#include "GetMostDefendedPosition.h"
#include "ProjectByUnitRange.h"
#include "ReinforceBlock.h"
#include "GameObjectTypeManager.h"
#include "AI/LuaScript/GameObjectTypeWrapper.h"
#include "SpawnUnit.h"
#include "FindMarker.h"
#include "GetStoryPlot.h"
#include "CheckStoryFlag.h"
#include "LuaGameMessage.h"
#include "LuaDiscreteDistribution.h"
#include "OppositeSidesOfShield.h"
#include "CameraFXManager.h"
#include "CinematicsManager.h"
#include "GameObjectManager.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/PositionWrapper.h"
#include "GameObjectPropertiesType.h"
#include "DynamicEnum.h"
#include "FindPlayer.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "IsInHazard.h"
#include "FOWReveal.h"
#include "PlayLightningEffect.h"
#include "AssembleFleet.h"
#include "TransportLandingBehavior.h"
#include "BinkPlayer.h"
#include "LuaMusic.h"
#include "WeatherSystem.h"
#include "LuaGUIHighlight.h"
#include "Audio.h"  
#include "CommandBar.h"
#include "SFXEventManager.h"
#include "../PGRender/RTSScene.h"
#include "WeatherAudioManager.h"
#include "SpaceRetreatCoordinator.h"
#include "LocomotorInterface.h"
#include "FactionList.h"
#include "AI/Movement/Collision/CollisionSystem.h"
#include "FindBestLocalThreatCenter.h"
#include "SFXCommands.h"
#include "CancelFastForward.h"
#include "SpawnFromReinforcePool.h"
#include "SpawnSpecialWeapon.h"
#include "EnableDistanceFog.h"
#include "ScheduledEventQueue.h"

class LuaStartCinematicCamera : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass* script, LuaTable* params)
	{
		bool resume_sound = true;

		if (params->Value.size() > 0)
		{
			LuaBool::Pointer boolval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);

			if (!boolval)
			{
				script->Script_Error("LuaStartCinematicCamera -- invalid type for parameter 1.  Expected bool.");
				return NULL;
			}	

			resume_sound = boolval->Value;
		}

		CameraFXManager.Start_Cinematic_Mode(resume_sound);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaStartCinematicCamera, LuaUserVar);

class LuaEndCinematicCamera : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass* /*script*/, LuaTable* /*params*/)
	{
		CameraFXManager.End_Cinematic_Mode();
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaEndCinematicCamera, LuaUserVar);


class LuaSuspendAI : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaSuspendAI -- Expected float parameter 1");
			return NULL;
		}

		bool suspended = false;

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if (!floatval)
		{
			script->Script_Error("LuaSuspendAI -- Invalid parameter 1, expected a float");
		}
		else
		{
			suspended = floatval->Value > 0.0f ? true : false;
		}
			
		CameraFXManager.Suspend_AI(suspended);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaSuspendAI, LuaUserVar);

class LuaCreatePosition : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 3)
		{
			script->Script_Error("LuaCreatePosition -- Expected 3 float parameters");
			return NULL;
		}

		LuaNumber::Pointer floatvalx = LUA_SAFE_CAST(LuaNumber, params->Value[0]);
		LuaNumber::Pointer floatvaly = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
		LuaNumber::Pointer floatvalz = LUA_SAFE_CAST(LuaNumber, params->Value[2]);

		if (!floatvalx || !floatvaly || !floatvalz)
		{
			script->Script_Error("LuaCreatePosition -- Invalid parameter, expected 3 floats");
			return NULL;
		}

		Vector3 t(floatvalx->Value, floatvaly->Value, floatvalz->Value);
		return Return_Variable(PositionWrapper::Create(t));
	}
};
PG_IMPLEMENT_RTTI(LuaCreatePosition, LuaUserVar);

class LuaLockControls : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		/*
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaLockControls -- Command only valid in a tactical game.");
			return NULL;
		}
		*/
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaLockControls -- Expected float parameter 1");
			return NULL;
		}

		bool locked = false;

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if (!floatval)
		{
			script->Script_Error("LuaLockControls -- Invalid parameter 1, expected a float");
		}
		else
		{
			locked = floatval->Value > 0.0f ? true : false;
		}
			
		CameraFXManager.Lock_Controls(locked);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaLockControls, LuaUserVar);

class LuaRotateCameraTo : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaRotateCameraTo -- Command only valid in a tactical game.");
			return NULL;
		}

		if (params->Value.size() < 2)
		{
			script->Script_Error("LuaRotateCameraTo -- Expected float parameter 1, float parameter 2");
			return NULL;
		}

		float angle = 0.0f;
		float time = 0.0f;
		bool shortest = true;

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if (!floatval)
		{
			script->Script_Error("LuaRotateCameraTo -- Invalid parameter 1, expected a float");
		}
		else
		{
			angle = floatval->Value;
		}

		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);

		if (!floatval)
		{
			script->Script_Error("LuaRotateCameraTo -- Invalid parameter 2, expected a float");
		}
		else
		{
			time = floatval->Value;
		}

		if (params->Value.size() > 2)
		{
			floatval = LUA_SAFE_CAST(LuaNumber, params->Value[2]);

			if (!floatval)
			{
				script->Script_Error("LuaRotateCameraTo -- Invalid parameter 3, expected a float");
			}
			else
			{
				shortest = floatval->Value > 0.0f ? true : false;
			}
			
		}

		CameraFXManager.Rotate_Camera_To(angle, time, shortest);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaRotateCameraTo, LuaUserVar);

class LuaRotateCameraBy : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaRotateCameraBy -- Command only valid in a tactical game.");
			return NULL;
		}

		if (params->Value.size() < 2)
		{
			script->Script_Error("LuaRotateCameraBy -- Expected float parameter 1, float parameter 2");
			return NULL;
		}

		float angle = 0.0f;
		float time = 0.0f;

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if (!floatval)
		{
			script->Script_Error("LuaRotateCameraBy -- Invalid parameter 1, expected a float");
		}
		else
		{
			angle = floatval->Value;
		}

		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);

		if (!floatval)
		{
			script->Script_Error("LuaRotateCameraBy -- Invalid parameter 2, expected a float");
		}
		else
		{
			time = floatval->Value;
		}

		CameraFXManager.Rotate_Camera_By(angle, time);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaRotateCameraBy, LuaUserVar);

class LuaZoomCamera : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaZoomCamera -- Command only valid in a tactical game.");
			return NULL;
		}

		if (params->Value.size() < 2)
		{
			script->Script_Error("LuaZoomCamera -- Expected float parameter 1, float parameter 2");
			return NULL;
		}

		float zoom = .5f;
		bool immediate = false;

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if (!floatval)
		{
			script->Script_Error("LuaZoomCamera -- Invalid parameter 1, expected a float");
		}
		else
		{
			zoom = floatval->Value;
		}

		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);

		if (!floatval)
		{
			script->Script_Error("LuaZoomCamera -- Invalid parameter 2, expected a float");
		}
		else
		{
			immediate = floatval->Value > 0.0 ? true : false;
		}

		CameraFXManager.Zoom_Camera(zoom, immediate);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaZoomCamera, LuaUserVar);


class LuaCameraToFollow : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaScrollCameraTo -- Command only valid in a tactical game.");
			return NULL;
		}

		GameObjectClass* pobj = NULL;
		bool immediate = true;
	
		if(params->Value.size() > 0)
		{
			SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);

			if(object_wrapper)
			{
				pobj = object_wrapper->Get_Object();
			}
		}

		if(params->Value.size() > 1)
		{
			LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
			
			if (floatval)
			{
				immediate = floatval->Value > 0.0 ? true : false;
			}
		}

		CameraFXManager.Camera_To_Follow(pobj, immediate);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaCameraToFollow, LuaUserVar);




class LuaScrollCameraTo : public LuaUserVar
{
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaScrollCameraTo -- Command only valid in a tactical game.");
			return NULL;
		}
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaScrollCameraTo -- Expected position parameter 1");
			return NULL;
		}

		Vector3 dest(VECTOR3_INVALID);
		GameObjectClass* pobj = NULL;

		Lua_Extract_Position(params->Value[0], dest);

		if (dest == VECTOR3_INVALID)
		{
			script->Script_Warning("LuaScrollCameraTo -- Invalid position.");
			return NULL;
		}

		if(params->Value.size() > 1)
		{
			SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[1]);

			if(object_wrapper)
			{
				pobj = object_wrapper->Get_Object();
			}
		}

		CameraFXManager.Scroll_Camera_To(dest, pobj);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaScrollCameraTo, LuaUserVar);


/**
 * Creates a letterbox view of the scene over time
 * @since 7/25/2005 3:07:09 PM -- JAR
 */
class LuaFadeOn : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass*, LuaTable*)
	{
		CameraFXManager.Set_Faded_Out();
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaFadeOn, LuaUserVar);

/**
 * Creates a letterbox view of the scene over time
 * @since 7/25/2005 3:07:09 PM -- JAR
 */
class LuaFadeOff : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *, LuaTable*)
	{
		CameraFXManager.Set_Faded_In();
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaFadeOff, LuaUserVar);

/**
 * Creates a letterbox view of the scene over time
 * @since 7/25/2005 3:07:09 PM -- JAR
 */
class LuaLetterBoxOn : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass*, LuaTable*)
	{
		CameraFXManager.Set_Letter_Box_On();
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaLetterBoxOn, LuaUserVar);

/**
 * Creates a letterbox view of the scene over time
 * @since 7/25/2005 3:07:09 PM -- JAR
 */
class LuaLetterBoxOff : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass*, LuaTable*)
	{
		CameraFXManager.Set_Letter_Box_Off();
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaLetterBoxOff, LuaUserVar);

/**
 * Creates a letterbox view of the scene over time
 * @since 7/25/2005 3:07:09 PM -- JAR
 */
class LuaLetterBoxIn : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaLetterBoxIn -- Expected float parameter 1");
			return NULL;
		}

		float letterboxtime = .5f;

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if (!floatval)
		{
			script->Script_Error("LuaLetterBoxIn -- Invalid parameter 1, expected a float");
		}
		else
		{
			letterboxtime = floatval->Value;
		}

		CameraFXManager.Letter_Box_In(letterboxtime);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaLetterBoxIn, LuaUserVar);


/**
 * removes a letterbox view of the scene over time
 * @since 7/25/2005 3:07:09 PM -- JAR
 */
class LuaLetterBoxOut : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaLetterBoxOut -- Expected float parameter 1");
			return NULL;
		}

		float letterboxtime = .5f;

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if (!floatval)
		{
			script->Script_Error("LuaLetterBoxOut -- Invalid parameter 1, expected a float");
		}
		else
		{
			letterboxtime = floatval->Value;
		}

		CameraFXManager.Letter_Box_Out(letterboxtime);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaLetterBoxOut, LuaUserVar);

/**
 * Fades the scene in from faded out
 * @since 7/25/2005 3:07:09 PM -- JAR
 */
class LuaFadeScreenIn : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaFadeScreenIn -- Expected float parameter 1");
			return NULL;
		}

		float fadetime = .5f;

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if (!floatval)
		{
			script->Script_Error("LuaFadeScreenIn -- Invalid parameter 1, expected a float");
		}
		else
		{
			fadetime = floatval->Value;
		}

		CameraFXManager.Fade_In(fadetime);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaFadeScreenIn, LuaUserVar);


/**
 * Fades the scene out from faded int
 * @since 7/31/2005 9:07:45 PM -- JAR
 */
class LuaFadeScreenOut : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaFadeScreenOut -- Expected float parameter 1");
			return NULL;
		}

		float fadetime = .5f;

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if (!floatval)
		{
			script->Script_Error("LuaFadeScreenOut -- Invalid parameter 1, expected a float");
		}
		else
		{
			fadetime = floatval->Value;
		}

		CameraFXManager.Fade_Out(fadetime);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaFadeScreenOut, LuaUserVar);


/**
 * Points the camera at the object.
 * @since 5/5/2005 11:07:09 AM -- BMH
 */
class LuaPointCamera : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaPointCamera -- Command only valid in a tactical game.");
			return NULL;
		}
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaPointCamera -- Expected position parameter 1");
			return NULL;
		}
		Vector3 dest(VECTOR3_INVALID);
		Lua_Extract_Position(params->Value[0], dest);
		if (dest == VECTOR3_INVALID)
		{
			script->Script_Warning("LuaPointCamera -- Invalid position.");
			return NULL;
		}

		Vector2 point = dest.Project_XY();
		CameraFXManager.Point_Camera_At(point);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaPointCamera, LuaUserVar);


/**
 * Finds an object_type of the name passed in.
 * @since 5/2/2005 3:24:38 PM -- BMH
 */
class LuaFindObjectType : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaFindObjectType -- Expected type name for parameter 1");
			return NULL;
		}
		LuaString::Pointer str = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (!str)
		{
			script->Script_Error("LuaFindObjectType -- Expected type name for parameter 1");
			return NULL;
		}

		GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(str->Value);
		if (type)
		{
			return Return_Variable(GameObjectTypeWrapper::Create(type, script));
		}
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaFindObjectType, LuaUserVar);


/**
 * Iterate all objects and return a table of objects matching the query.
 * @since 8/8/2005 1:59:50 PM -- BMH
 */
class FindAllObjectsOfType : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 1)
		{
			script->Script_Error("FindAllObjectsOfType -- Expected a filter parameter.");
			return NULL;
		}

		unsigned int parameter_index = 0;
		GameObjectPropertiesType property_filter = GAME_OBJECT_PROPERTIES_ALL;
		GameObjectCategoryType category_filter = GAME_OBJECT_CATEGORY_ALL;
		GameObjectTypeClass *filter_type = NULL;
		PlayerClass *player = NULL;

		while (parameter_index < params->Value.size())
		{
			SmartPtr<LuaString> type_name = PG_Dynamic_Cast<LuaString>(params->Value[parameter_index]);
			SmartPtr<GameObjectTypeWrapper> type_wrap = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[parameter_index]);
			SmartPtr<PlayerWrapper> player_wrap = PG_Dynamic_Cast<PlayerWrapper>(params->Value[parameter_index]);
			bool matched = false;

			if (type_name && property_filter == GAME_OBJECT_PROPERTIES_ALL)
			{
				matched = TheGameObjectPropertiesTypeConverterPtr->String_To_Enum(type_name->Value.c_str(), property_filter);
			}
			if (!matched && type_name && category_filter == GAME_OBJECT_CATEGORY_ALL)
			{
				matched = TheGameObjectCategoryTypeConverterPtr->String_To_Enum(type_name->Value.c_str(), category_filter);
			}
			if (type_wrap && filter_type == NULL)
			{
				matched = true;
				filter_type = type_wrap->Get_Object();
			}
			if (!matched && type_name && filter_type == NULL)
			{
				filter_type = GameObjectTypeManager.Find_Object_Type(type_name->Value);
				matched = filter_type ? true : false;
			}
			if (player_wrap)
			{
				matched = true;
				player = player_wrap->Get_Object();
			}

			if (!matched)
			{
				script->Script_Error("FindAllObjectsOfType -- unknown filter parameter %d.", parameter_index+1);
				return 0;
			}
			++parameter_index;
		}

		if (!player && !filter_type && category_filter == GAME_OBJECT_CATEGORY_ALL && property_filter == GAME_OBJECT_PROPERTIES_ALL)
		{
			script->Script_Error("FindAllObjectsOfType -- Invalid parameters.  You must pass a filter.");
			return 0;
		}

		LuaTable::Pointer table = Alloc_Lua_Table();

		ReferenceListIterator<GameObjectClass> object_list = GameModeManager.Get_Active_Mode()->Get_Object_Manager().Get_Object_Iterator();
		for (object_list.First(); object_list.Is_Done() == false; object_list.Next())
		{
			GameObjectClass *object = object_list.Current_Object();

			if (player && player != object->Get_Owner_Player())
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

			if ((category_filter != GAME_OBJECT_CATEGORY_ALL) && (category_filter & object->Get_Original_Object_Type()->Get_Category_Mask()) == 0)
			{
				continue;
			}

			table->Value.push_back(GameObjectWrapper::Create(object, script));
		}

		return Return_Variable(table);
	}
};
PG_IMPLEMENT_RTTI(FindAllObjectsOfType, LuaUserVar);

/**
 * activate the mission retry dialog
 * @since 6/15/2005 10:19:05 AM -- BMH
 */
class ActivateRetryDialog : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *, LuaTable *)
	{
		StoryEventClass::Activate_Retry_Dialog();
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(ActivateRetryDialog, LuaUserVar);

/*
void Set_Cinematic_Target_Key(Vector3& target_pos, float x_dist, float y_pitch, float z_yaw, bool euler, GameObjectClass* pobj, bool use_object_rotation, bool cinematic_animation);
*/
class LuaSetCinematicTargetKey : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		/*
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaSetCinematicTargetKey -- Command only valid in a tactical game.");
			return NULL;
		}
		*/
		if (params->Value.size() < 8)
		{
			script->Script_Error("LuaSetCinematicTargetKey -- Expected 8 parameters");
			return NULL;
		}

		// Parameters for set cinematic camera
		Vector3 target_pos(VECTOR3_INVALID);
		float x_dist = 0.0f;
		float y_pitch = 0.0f;
		float z_yaw = 0.0f;
		bool euler = false;
		GameObjectClass* pobj = NULL;
		bool use_object_rotation = false;
		bool cinematic_animation = false;
		
		// Get the target key position 
		Lua_Extract_Position(params->Value[0], target_pos);

		if(target_pos == VECTOR3_INVALID)
		{
			script->Script_Warning("LuaSetCinematicTargetKey -- Invalid parameter 1, expected a position.");
			return NULL;
		}
		// Get the x offset or distance from the target 
		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
		
		if (!floatval)
			script->Script_Error("LuaSetCinematicTargetKey -- Invalid parameter 2, expected a float");
		else
			x_dist = floatval->Value;
		
		// Get the y offset or pitch 
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[2]);
		
		if (!floatval)
			script->Script_Error("LuaSetCinematicTargetKey -- Invalid parameter 3, expected a float");
		else
			y_pitch = floatval->Value;
	
		// Get the z offset or yaw 
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[3]);
			
		if (!floatval)
			script->Script_Error("LuaSetCinematicTargetKey -- Invalid parameter 4, expected a float");
		else
			z_yaw = floatval->Value;
		
		// Get whether the offsets are angles or axis offsets
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[4]);

		if (!floatval)
			script->Script_Error("LuaSetCinematicTargetKey -- Invalid parameter 5, expected a float");
		else
			euler = floatval->Value > 0.0f ? true : false;	

		// Get the game object if one was passed in
		SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[5]);

		if(object_wrapper)
		{
			pobj = object_wrapper->Get_Object();
		}
		
		// Get whether to track a cinematic animation
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[6]);
		
		if (!floatval)
			script->Script_Error("LuaSetCinematicTargetKey -- Invalid parameter 7, expected a float");
		else
			use_object_rotation = floatval->Value > 0.0f ? true : false;
		
		// Get whether to track a cinematic animation
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[7]);
		
		if (!floatval)
			script->Script_Error("LuaSetCinematicTargetKey -- Invalid parameter 8, expected a float");
		else
			cinematic_animation = floatval->Value > 0.0f ? true : false;
		
		CameraFXManager.Set_Cinematic_Target_Key(target_pos, x_dist, y_pitch, z_yaw, euler, pobj, use_object_rotation, cinematic_animation);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaSetCinematicTargetKey, LuaUserVar);

/*
void Transition_Cinematic_Target_Key(Vector3& target_pos, float time, float x_dist, float y_pitch, float z_yaw, bool euler, GameObjectClass* pobj, bool use_object_rotation, bool cinematic_animation);
*/
class LuaTransitionCinematicTargetKey : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		/*
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaTransitionCinematicTargetKey -- Command only valid in a tactical game.");
			return NULL;
		}
		*/
		if (params->Value.size() < 9)
		{
			script->Script_Error("LuaTransitionCinematicTargetKey -- Expected 9 parameters");
			return NULL;
		}

		// Parameters for set cinematic camera
		Vector3 target_pos(VECTOR3_INVALID);
		float time = 0.0f;
		float x_dist = 0.0f;
		float y_pitch = 0.0f;
		float z_yaw = 0.0f;
		bool euler = false;
		GameObjectClass* pobj = NULL;
		bool use_object_rotation = false;
		bool cinematic_animation = false;
		
		// Get the target key position 
		Lua_Extract_Position(params->Value[0], target_pos);

		if(target_pos == VECTOR3_INVALID)
		{
			script->Script_Warning("LuaTransitionCinematicTargetKey -- Invalid parameter 1, expected a position.");
			return NULL;
		}

		// Get the time for the transition 
		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
		
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicTargetKey -- Invalid parameter 2, expected a float");
		else
			time = floatval->Value;

		// Get the x offset or distance from the target 
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[2]);
		
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicTargetKey -- Invalid parameter 3, expected a float");
		else
			x_dist = floatval->Value;
		
		// Get the y offset or pitch 
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[3]);
		
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicTargetKey -- Invalid parameter 4, expected a float");
		else
			y_pitch = floatval->Value;
	
		// Get the z offset or yaw 
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[4]);
			
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicTargetKey -- Invalid parameter 5, expected a float");
		else
			z_yaw = floatval->Value;
		
		// Get whether the offsets are angles or axis offsets
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[5]);

		if (!floatval)
			script->Script_Error("LuaTransitionCinematicTargetKey -- Invalid parameter 6, expected a float");
		else
			euler = floatval->Value > 0.0f ? true : false;	

		// Get the game object if one was passed in
		SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[6]);

		if(object_wrapper)
		{
			pobj = object_wrapper->Get_Object();
		}
		
		// Get whether to track a cinematic animation
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[7]);
		
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicTargetKey -- Invalid parameter 8, expected a float");
		else
			use_object_rotation = floatval->Value > 0.0f ? true : false;
		
		// Get whether to track a cinematic animation
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[8]);
		
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicTargetKey -- Invalid parameter 9, expected a float");
		else
			cinematic_animation = floatval->Value > 0.0f ? true : false;
		
		CameraFXManager.Transition_Cinematic_Target_Key(target_pos, time, x_dist, y_pitch, z_yaw, euler, pobj, use_object_rotation, cinematic_animation);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaTransitionCinematicTargetKey, LuaUserVar);

/*
void Set_Cinematic_Camera_Key(Vector3& target_pos, float x_dist, float y_pitch, float z_yaw, bool euler, GameObjectClass* pobj, bool use_object_rotation, bool cinematic_animation);
*/
class LuaSetCinematicCameraKey : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		/*
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaSetCinematicCameraKey -- Command only valid in a tactical game.");
			return NULL;
		}
		*/
		if (params->Value.size() < 8)
		{
			script->Script_Error("LuaSetCinematicCameraKey -- Expected 8 parameters");
			return NULL;
		}

		// Parameters for set cinematic camera
		Vector3 target_pos(VECTOR3_INVALID);
		float x_dist = 0.0f;
		float y_pitch = 0.0f;
		float z_yaw = 0.0f;
		bool euler = false;
		GameObjectClass* pobj = NULL;
		bool use_object_rotation = false;
		bool cinematic_animation = false;
		
		// Get the target key position 
		Lua_Extract_Position(params->Value[0], target_pos);

		if(target_pos == VECTOR3_INVALID)
		{
			script->Script_Warning("LuaSetCinematicCameraKey -- Invalid parameter 1, expected a position.");
			return NULL;
		}
		// Get the x offset or distance from the target 
		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
		
		if (!floatval)
			script->Script_Error("LuaSetCinematicCameraKey -- Invalid parameter 2, expected a float");
		else
			x_dist = floatval->Value;
		
		// Get the y offset or pitch 
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[2]);
		
		if (!floatval)
			script->Script_Error("LuaSetCinematicCameraKey -- Invalid parameter 3, expected a float");
		else
			y_pitch = floatval->Value;
	
		// Get the z offset or yaw 
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[3]);
			
		if (!floatval)
			script->Script_Error("LuaSetCinematicCameraKey -- Invalid parameter 4, expected a float");
		else
			z_yaw = floatval->Value;
		
		// Get whether the offsets are angles or axis offsets
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[4]);

		if (!floatval)
			script->Script_Error("LuaSetCinematicCameraKey -- Invalid parameter 5, expected a float");
		else
			euler = floatval->Value > 0.0f ? true : false;	

		// Get the game object if one was passed in
		SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[5]);

		if(object_wrapper)
		{
			pobj = object_wrapper->Get_Object();
		}
		
		// Get whether to track a cinematic animation
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[6]);
		
		if (!floatval)
			script->Script_Error("LuaSetCinematicCameraKey -- Invalid parameter 7, expected a float");
		else
			use_object_rotation = floatval->Value > 0.0f ? true : false;
		
		// Get whether to track a cinematic animation
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[7]);
		
		if (!floatval)
			script->Script_Error("LuaSetCinematicCameraKey -- Invalid parameter 8, expected a float");
		else
			cinematic_animation = floatval->Value > 0.0f ? true : false;
		
		CameraFXManager.Set_Cinematic_Camera_Key(target_pos, x_dist, y_pitch, z_yaw, euler, pobj, use_object_rotation, cinematic_animation);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaSetCinematicCameraKey, LuaUserVar);

/*
void Transition_Cinematic_Camera_Key(Vector3& target_pos, float time, float x_dist, float y_pitch, float z_yaw, bool euler, GameObjectClass* pobj, bool use_object_rotation, bool cinematic_animation);
*/
class LuaTransitionCinematicCameraKey : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		/*
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaTransitionCinematicCameraKey -- Command only valid in a tactical game.");
			return NULL;
		}
		*/
		if (params->Value.size() < 9)
		{
			script->Script_Error("LuaTransitionCinematicCameraKey -- Expected 9 parameters");
			return NULL;
		}

		// Parameters for set cinematic camera
		Vector3 target_pos(VECTOR3_INVALID);
		float time = 0.0f;
		float x_dist = 0.0f;
		float y_pitch = 0.0f;
		float z_yaw = 0.0f;
		bool euler = false;
		GameObjectClass* pobj = NULL;
		bool use_object_rotation = false;
		bool cinematic_animation = false;
		
		// Get the target key position 
		Lua_Extract_Position(params->Value[0], target_pos);

		if(target_pos == VECTOR3_INVALID)
		{
			script->Script_Warning("LuaTransitionCinematicCameraKey -- Invalid parameter 1, expected a position.");
			return NULL;
		}

		// Get the time for the transition 
		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
		
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicCameraKey -- Invalid parameter 2, expected a float");
		else
			time = floatval->Value;

		// Get the x offset or distance from the target 
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[2]);
		
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicCameraKey -- Invalid parameter 3, expected a float");
		else
			x_dist = floatval->Value;
		
		// Get the y offset or pitch 
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[3]);
		
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicCameraKey -- Invalid parameter 4, expected a float");
		else
			y_pitch = floatval->Value;
	
		// Get the z offset or yaw 
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[4]);
			
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicCameraKey -- Invalid parameter 5, expected a float");
		else
			z_yaw = floatval->Value;
		
		// Get whether the offsets are angles or axis offsets
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[5]);

		if (!floatval)
			script->Script_Error("LuaTransitionCinematicCameraKey -- Invalid parameter 6, expected a float");
		else
			euler = floatval->Value > 0.0f ? true : false;	

		// Get the game object if one was passed in
		SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[6]);

		if(object_wrapper)
		{
			pobj = object_wrapper->Get_Object();
		}
		
		// Get whether to track a cinematic animation
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[7]);
		
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicCameraKey -- Invalid parameter 8, expected a float");
		else
			use_object_rotation = floatval->Value > 0.0f ? true : false;
		
		// Get whether to track a cinematic animation
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[8]);
		
		if (!floatval)
			script->Script_Error("LuaTransitionCinematicCameraKey -- Invalid parameter 9, expected a float");
		else
			cinematic_animation = floatval->Value > 0.0f ? true : false;
		
		CameraFXManager.Transition_Cinematic_Camera_Key(target_pos, time, x_dist, y_pitch, z_yaw, euler, pobj, use_object_rotation, cinematic_animation);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaTransitionCinematicCameraKey, LuaUserVar);

/*
void Hide_Object(GameObjectClass* pobj, bool state);
*/
class LuaHideObject : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaTransitionCinematicTargetKey -- Command only valid in a tactical game.");
			return NULL;
		}
		if (params->Value.size() < 2)
		{
			script->Script_Error("LuaHideObject -- Expected gameobject pointer parameter 1, float parameter 2");
			return NULL;
		}

		// Parameters for set cinematic camera
		GameObjectClass* pobj = NULL;
		bool hide = true;

		
		SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);

		if(object_wrapper)
		{
			pobj = object_wrapper->Get_Object();
		}
		else
		{
			script->Script_Error("LuaHideObject -- Invalid parameter 1, expected a game object");
		}
		
		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
		
		if(!floatval)
		{
			script->Script_Error("LuaHideObject -- Invalid parameter 2, expected a float");
		}		
		else
		{
			hide = floatval->Value > 0.0f ? true : false;
		}
	

		CameraFXManager.Hide_Object(pobj, hide);
		pobj->Set_Attached_SFX_Silent(hide);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaHideObject, LuaUserVar);

/*
void Hide_Object(GameObjectClass* pobj, bool state);
*/
class LuaHideSubObject : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaHideSubObject -- Command only valid in a tactical game.");
			return NULL;
		}
		if (params->Value.size() < 3)
		{
			script->Script_Error("LuaHideSubObject -- Expected gameobject pointer parameter 1, float parameter 2, string parameter 3");
			return NULL;
		}

		// Parameters for set cinematic camera
		GameObjectClass* pobj = NULL;
		bool hide = true;

		
		SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);

		if(object_wrapper)
		{
			pobj = object_wrapper->Get_Object();
		}
		else
		{
			script->Script_Error("LuaHideSubObject -- Invalid parameter 1, expected a game object");
		}
		 
		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
		
		if(!floatval)
		{
			script->Script_Error("LuaHideSubObject -- Invalid parameter 2, expected a float");
		}		
		else
		{
			hide = floatval->Value > 0.0f ? true : false;
		}
	
		SmartPtr<LuaString> sub_object_name = PG_Dynamic_Cast<LuaString>(params->Value[2]);
		
		if(!sub_object_name)
		{
			script->Script_Error("LuaHideSubObject -- invalid type for parameter 3.  Expected string.");
		}	

		CameraFXManager.Hide_Sub_Object(pobj, hide, sub_object_name->Value.c_str());
		pobj->Set_Attached_SFX_Silent(hide);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaHideSubObject, LuaUserVar);


/*
void Transition_To_Tactical_Camera(float time);
*/
class LuaTransitionToTacticalCamera : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		/*
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaTransitionCinematicTargetKey -- Command only valid in a tactical game.");
			return NULL;
		}
		*/
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaTransitionToTacticalCamera -- Expected float parameter 1");
			return NULL;
		}

		// Parameters for set cinematic camera
		float time = 1.0f;

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);
		
		if(!floatval)
		{
			script->Script_Error("LuaTransitionToTacticalCamera -- Invalid parameter 1, expected a float");
		}		
		else
		{
			time = floatval->Value;
		}

		CameraFXManager.Transition_To_Tactical_Camera(time);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaTransitionToTacticalCamera, LuaUserVar);

/*
void CinematicsManagerClass::Create_Cinematic_Transport(const char* shuttle_type_name, int player_id, Vector3 pos, float zangle, int mode, float delta, float idle_time, bool persist, const char* hint)

*/
class LuaCreateCinematicTransport : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		// Check to make sure we are in a tactical game
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaCreateCinematicTransport -- Command only valid in a tactical game.");
			return NULL;
		}
		// Check to make sure this is a land tactical game
		if (GameModeManager.Get_Active_Mode()->Get_Sub_Type() != SUB_GAME_MODE_LAND)
		{
			script->Script_Error("LuaCreateCinematicTransport -- Command only valid in a land tactical game.");
			return NULL;
		}
		// Check to see that we have the correct number of parameters passed
		if(params->Value.size() < 8)
		{
			script->Script_Error("LuaCreateCinematicTransport -- Expected 8 parameters");
			return NULL;
		}
		// Get the first parameter, a string that names the transport type
		SmartPtr<LuaString> transport_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (!transport_name)
		{
			script->Script_Error("LuaCreateCinematicTransport -- invalid type for parameter 1.  Expected string.");
			return NULL;
		}	
		
		// Get the second parameter, the player id
		int player_id = 0;
		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);

		if(!floatval)
			script->Script_Error("LuaCreateCinematicTransport -- Invalid parameter 2, expected a float value for player_id");
		else
			player_id = static_cast<int>(floatval->Value);
		
		// Get the third parameter, the position the transport will land
		Vector3 transport_position(VECTOR3_INVALID);

		Lua_Extract_Position(params->Value[2], transport_position);

		if(transport_position == VECTOR3_INVALID)
		{
			script->Script_Warning("LuaCreateCinematicTransport -- Invalid parameter 3, expected a position value.");
			return NULL;
		}

		// Get the fourth parameter, the z facing angle of the transport
		float z_angle = 0.0f;
	
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[3]);

		if(!floatval)
			script->Script_Error("LuaCreateCinematicTransport -- Invalid parameter 4, expected a float value for z_angle");
		else
			z_angle = floatval->Value;	

		// Get the fifth parameter, the mode the shuttle starts in 
		int mode = TRANSPORT_PHASE_LANDING;

		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[4]);

		if(!floatval)
			script->Script_Error("LuaCreateCinematicTransport -- Invalid parameter 5, expected a float value for transport landing mode");
		else
			mode = static_cast<int>(floatval->Value);

		// Get the sixth parameter, the delta into the landing animation
		float anim_delta = 0.0f;
		
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[5]);

		if(!floatval)
			script->Script_Error("LuaCreateCinematicTransport -- Invalid parameter 6, expected a float value for anim delta");
		else
			anim_delta = floatval->Value;	

		// Get the seventh parameter, the time the transport will stay on the ground
		float idle_time = 2.0f;
		
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[6]);

		if(!floatval)
			script->Script_Error("LuaCreateCinematicTransport -- Invalid parameter 7, expected a float value for idle time");
		else
			idle_time = floatval->Value;	

		// Get the eight parameter, whether the shuttle will leave
		bool persist = false;

		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[7]);

		if(!floatval)
			script->Script_Error("LuaCreateCinematicTransport -- Invalid parameter 8, expected a float value for transport persisting");
		else
			persist = floatval->Value > 0.0f ? true : false;	

		// Get the ninth paramter if it was passed in 
		if(params->Value.size() > 8)
		{
			SmartPtr<LuaString> hint_name = PG_Dynamic_Cast<LuaString>(params->Value[8]);
			if (!hint_name)
			{
				script->Script_Error("LuaCreateCinematicTransport -- invalid type for parameter 9.  Expected string.");
				GameObjectClass *obj = CameraFXManager.Create_Cinematic_Transport(transport_name->Value.c_str(), player_id, transport_position, z_angle, mode, anim_delta, idle_time, persist);  
			
				if(NULL != obj)
				{
					return Return_Variable(GameObjectWrapper::Create(obj, script));
				}
			}	
			else
			{
				GameObjectClass *obj = CameraFXManager.Create_Cinematic_Transport(transport_name->Value.c_str(), player_id, transport_position, z_angle, mode, anim_delta, idle_time, persist, hint_name->Value.c_str());
			
				if(NULL != obj)
				{
					return Return_Variable(GameObjectWrapper::Create(obj, script));
				}
			}
		}
		else
		{
			GameObjectClass *obj = CameraFXManager.Create_Cinematic_Transport(transport_name->Value.c_str(), player_id, transport_position, z_angle, mode, anim_delta, idle_time, persist);  
		
			if(NULL != obj)
			{
				return Return_Variable(GameObjectWrapper::Create(obj, script));
			}
		}

		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaCreateCinematicTransport, LuaUserVar);



class LuaEnableFog : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaEnableFog -- Command only valid in a land tactical game.");
			return NULL;
		}
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaEnableFog -- Expected bool parameter 1");
			return NULL;
		}

		LuaBool::Pointer boolval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);

		if (!boolval)
		{
			script->Script_Error("LuaEnableFog -- invalid type for parameter 1.  Expected bool.");
			return NULL;
		}	

		bool enable = boolval->Value;

		CameraFXManager.Enable_Fog(enable);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaEnableFog, LuaUserVar);




/*
void Promote_To_Space_Cinematic_Layer(GameObjectClass* pobj, bool state);
*/
class LuaPromoteToSpaceCinematicLayer : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaPromoteToSpaceCinematicLayer -- Command only valid in a tactical game.");
			return NULL;
		}
		// Check to make sure this is a land tactical game
		if (GameModeManager.Get_Active_Mode()->Get_Sub_Type() != SUB_GAME_MODE_LAND && GameModeManager.Get_Active_Mode()->Get_Sub_Type() != SUB_GAME_MODE_SPACE)
		{
			script->Script_Error("LuaPromoteToSpaceCinematicLayer -- Command only valid in a land or space tactical game.");
			return NULL;
		}
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaPromoteToSpaceCinematicLayer -- Expected gameobject parameter 1");
			return NULL;
		}

		// Parameters for set cinematic camera
		GameObjectClass* pobj = NULL;
	
		SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[0]);

		if(object_wrapper)
		{
			pobj = object_wrapper->Get_Object();
		}
		else
		{
			script->Script_Error("LuaPromoteToSpaceCinematicLayer -- Invalid parameter 1, expected a game object");
		}
		
		if(NULL != pobj)
		{
			Vector3 pos = pobj->Get_Position();
			pos.Z += 20000.0f;
			pobj->Teleport(pos);
		}
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaPromoteToSpaceCinematicLayer, LuaUserVar);


class BinkMovieBlockStatus : public BlockingStatus
{
public: 
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_BINK_MOVIE_BLOCK, BinkMovieBlockStatus);

	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *)
	{
		return Return_Variable(new LuaBool(BinkPlayer.Is_Playing() == false));
	}
	virtual LuaTable *Result(LuaScriptClass *, LuaTable *)							{ return NULL; }

	//Leave Save/Load using base class implementation.  Lightning effects are not saveable, so if we save a game
	//while one is going off we just want the block to finish immediately when we load again.
};

PG_IMPLEMENT_RTTI(BinkMovieBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_BINK_MOVIE_BLOCK, BinkMovieBlockStatus);


/*
**BinkPlayer.Open_Bink_Movie(const char *name, bool looping, void(*post_movie_callback)(void), bool hide_game, bool manual_blit)
*/
class LuaPlayBinkMovie : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaPlayBinkMovie -- Expected type name for parameter 1");
			return NULL;
		}
		LuaString::Pointer str = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (!str)
		{
			script->Script_Error("LuaPlayBinkMovie -- Expected type string for parameter 1");
			return NULL;
		}

		BinkPlayer.Open_Bink_Movie(str->Value.c_str());
	
		return Return_Variable(BinkMovieBlockStatus::FactoryCreate());
	}
};
PG_IMPLEMENT_RTTI(LuaPlayBinkMovie, LuaUserVar);


/*
**BinkPlayer.Stop_Bink_Movie(void)
*/
class LuaStopBinkMovie : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass * /*script*/, LuaTable * /*params*/)
	{
		if(!BinkPlayer.Is_Playing())
		{
			return NULL;
		}
	
		BinkPlayer.Stop_Bink_Movie();
	
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaStopBinkMovie, LuaUserVar);

/**
 * Force weather into next pattern
 * @since 9/10/2005 2:11:54 PM -- BMH
 */
class LuaForceWeather : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *, LuaTable * /*params*/)
	{
		if(GameModeManager.Get_Active_Mode()->Get_Sub_Type() == SUB_GAME_MODE_LAND )
		{
			TheWeatherSystem.Force_Weather();
		}
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaForceWeather, LuaUserVar);


/**
 * Force weather into next pattern
 * @since 9/10/2005 2:11:54 PM -- BMH
 */
class LuaResumeHyperspaceIn : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *, LuaTable * /*params*/)
	{
		if(GameModeManager.Get_Active_Mode()->Get_Sub_Type() == SUB_GAME_MODE_SPACE )
		{
			CinematicsManager.Release_Hyperspace_Cinematic();
		}
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaResumeHyperspaceIn, LuaUserVar);





/**
 * Force weather into next pattern
 * @since 9/10/2005 2:11:54 PM -- BMH
 */
class LuaCinematicZoom : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable * params)
	{

		if (params->Value.size() < 2)
		{
			script->Script_Error("LuaCinematicZoom -- Expected float param 1 time, float param 2 delta");
			return NULL;
		}

		/*
		if(!GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical())
		{
			script->Script_Error("LuaCinematicZoom -- This command only works in tactical modes");
			return NULL;
		}
		*/
		if(CinematicsManager.Get_Cinematic_Mode() != CINEMATIC_MODE_STORY)
		{
			script->Script_Error("LuaCinematicZoom -- This command only when the cinematic manager is in story mode");
			return NULL;
		}
		
		float time = 0.0f;
		float delta = 1.0f;
		
		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if(!floatval)
		{
			script->Script_Error("LuaCinematicZoom -- Invalid parameter 1, expected a float");
			return NULL;
		}
		else
		{
			time = floatval->Value;
		}
		
		floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);

		if(!floatval)
		{
			script->Script_Error("LuaCinematicZoom -- Invalid parameter 2, expected a float");
			return NULL;
		}
		else
		{
			delta = floatval->Value;
		}
		
		CinematicsManager.Cinematic_Zoom(time, delta);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaCinematicZoom, LuaUserVar);





/**
 * Force weather into next pattern
 * @since 9/10/2005 2:11:54 PM -- BMH
 */
class LuaStopAllSpeech : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *, LuaTable * /*params*/)
	{
		if( TheAudio.Get_Speech() != NULL )
		{
			TheAudio.Get_Speech()->System_Stop_All();
		}
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaStopAllSpeech, LuaUserVar);


/**
 * Force weather into next pattern
 * @since 9/10/2005 2:11:54 PM -- BMH
 */
class LuaRemoveAllText : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *, LuaTable * /*params*/)
	{
		TheCommandBar.Remove_All_Text();
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaRemoveAllText, LuaUserVar);



class LuaAllowLocalizedSFX : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaAllowLocalizedSFX -- Command only valid in a land tactical game.");
			return NULL;
		}
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaAllowLocalizedSFX -- Expected bool parameter 1");
			return NULL;
		}

		LuaBool::Pointer boolval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);

		if (!boolval)
		{
			script->Script_Error("LuaAllowLocalizedSFX -- invalid type for parameter 1.  Expected bool.");
			return NULL;
		}	

		bool enable = boolval->Value;

		TheSFXEventManager.Allow_Localized_SFXEvents(enable, SFXEVENT_SYSTEM_ALLOW_LUA_SCRIPT_GLOBAL_COMMAND);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaAllowLocalizedSFX, LuaUserVar);




class LuaMasterVolumeRestore : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass* /*script*/, LuaTable* /*params*/)
	{
		TheAudio.Volume_Slider_Restore(VOLUME_SLIDER_MASTER);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaMasterVolumeRestore, LuaUserVar);


class LuaGetGameMode : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *, LuaTable *)
	{
		return Return_Variable(new LuaString(TheSubGameModeTypeConverterPtr->Enum_To_String(GameModeManager.Get_Sub_Type())));
	}
};
PG_IMPLEMENT_RTTI(LuaGetGameMode, LuaUserVar);




class LuaSetCinematicEnvironment : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode() == NULL)
		{
			script->Script_Error("LuaSetCinematicEnvironment -- No active game mode.");
			return NULL;
		}

		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaSetCinematicEnvironment -- Command only valid in a tactical game.");
			return NULL;
		}

		RtsSceneClass*	pRTSScene = static_cast<RtsSceneClass*>(GameModeManager.Get_Active_Mode()->Get_Scene());

		if(NULL == pRTSScene)
		{
			script->Script_Error("LuaSetCinematicEnvironment -- Unable to retrieve RTS scene from game mode.");
			return NULL;
		}

		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaSetCinematicEnvironment -- Expected bool parameter 1");
			return NULL;
		}

		LuaBool::Pointer boolval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);

		if (!boolval)
		{
			script->Script_Error("LuaSetCinematicEnvironment -- invalid type for parameter 1.  Expected bool.");
			return NULL;
		}	

		pRTSScene->Set_Cinematic_Environment(boolval->Value);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaSetCinematicEnvironment, LuaUserVar);

class LuaSetNewEnvironment : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode() == NULL)
		{
			script->Script_Error("LuaSetNewEnvironment -- No active game mode.");
			return NULL;
		}

		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaSetNewEnvironment -- Command only valid in a tactical game.");
			return NULL;
		}

		RtsSceneClass*	pRTSScene = static_cast<RtsSceneClass*>(GameModeManager.Get_Active_Mode()->Get_Scene());

		if(NULL == pRTSScene)
		{
			script->Script_Error("LuaSetNewEnvironment -- Unable to retrieve RTS scene from game mode.");
			return NULL;
		}

		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaSetNewEnvironment -- Expected int parameter 1");
			return NULL;
		}

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if (!floatval)
		{
			script->Script_Error("LuaSetNewEnvironment -- invalid type for parameter 1.  Expected int.");
			return NULL;
		}	

		int idx = static_cast<int>(floatval->Value);

		GameModeManager.Get_Active_Mode()->Set_Current_Environment(idx);
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaSetNewEnvironment, LuaUserVar);


class LuaStartCinematicMode : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *, LuaTable *)
	{
		CinematicsManager.Init_Movie_Mode();
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaStartCinematicMode, LuaUserVar);


class LuaEndCinematicMode : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *, LuaTable *)
	{
		CinematicsManager.End_Cinematic_Mode();
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaEndCinematicMode, LuaUserVar);


class LuaWeatherAudioPause : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
		{
			script->Script_Error("LuaWeatherAudioPause -- Command only valid in a land tactical game.");
			return NULL;
		}
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaWeatherAudioPause -- Expected bool parameter 1");
			return NULL;
		}

		LuaBool::Pointer boolval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);

		if (!boolval)
		{
			script->Script_Error("LuaWeatherAudioPause -- invalid type for parameter 1.  Expected bool.");
			return NULL;
		}	

		bool enable = boolval->Value;

		if(enable)
		{
			TheWeatherAudioManager.System_Pause();
		}
		else
		{
			TheWeatherAudioManager.System_Resume();
		}
		
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaWeatherAudioPause, LuaUserVar);



class LuaStartCinematicSpaceRetreat : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		GameModeClass* pgamemode = GameModeManager.Get_Active_Mode();
		
		if(NULL == pgamemode)
		{
			script->Script_Error("LuaStartCinematicSpaceRetreat -- Unable to retrieve current game mode.");
			return NULL;
		}

		if (pgamemode->Get_Sub_Type() != SUB_GAME_MODE_SPACE)
		{
			script->Script_Error("LuaStartCinematicSpaceRetreat -- Command only valid in a space tactical game.");
			return NULL;
		}

		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaStartCinematicSpaceRetreat -- Expected player id for parameter 1");
			return NULL;
		}
	
		int player_id = 0;

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);

		if(!floatval)
			script->Script_Error("LuaStartCinematicSpaceRetreat -- Invalid parameter 1, expected an int value for player_id");
		else
			player_id = static_cast<int>(floatval->Value);

		int delay = 0;

		if (params->Value.size() > 1)
		{
			floatval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
		
			if(!floatval)
				script->Script_Error("LuaStartCinematicSpaceRetreat -- Invalid parameter 2, expected an float value for delay time");
			else
				delay = static_cast<int>(floatval->Value * 30.0f); // assume 30 fps
		} 
		SpaceRetreatCoordinatorClass* spaceretreatcoordinator = reinterpret_cast<SpaceRetreatCoordinatorClass*>(pgamemode->Get_Retreat_Coordinator());

		if(NULL == spaceretreatcoordinator)
		{
			script->Script_Error("LuaStartCinematicSpaceRetreat -- Unable to locate valid retreat coordinator");
			return NULL;
		}

		if(!spaceretreatcoordinator->Is_Space_Retreat_Coordinator())
		{
			script->Script_Error("LuaStartCinematicSpaceRetreat -- Unable to locate valid space retreat coordinator");
			return NULL;
		}
		
		spaceretreatcoordinator->Build_Retreatable_Unit_List( player_id );
		spaceretreatcoordinator->Set_Units_To_Retreat_Positions();

		DynamicVectorClass<GameObjectClass*> retreatingunitlist = spaceretreatcoordinator->Get_Retreating_Unit_List();

		int unit_count = retreatingunitlist.Get_Count();
		
		PlayerClass *player = PlayerList.Get_Player_By_ID( player_id );
		assert( player != NULL );
		const FactionClass *player_faction = player->Get_Faction();
		assert( player_faction != NULL );

		int delay_inc = player_faction->Get_Space_Retreat_Unit_Increment_Wait_Frames();

		for(int i = 0; i < unit_count; ++i)
		{
			GameObjectClass* pobj = retreatingunitlist.Get_At(i);
			if(NULL != pobj)
			{
				pobj->Set_Is_In_End_Cinematic(true);
				LocomotorInterfaceClass *locomotor = static_cast<LocomotorInterfaceClass*>(pobj->Get_Behavior(BEHAVIOR_LOCO));
				
				if(NULL != locomotor)
					locomotor->Hyperspace_Away(false, delay);
			
				delay += delay_inc;
			}
		}
	
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaStartCinematicSpaceRetreat, LuaUserVar);



class LuaDoEndCinematicCleanup : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *, LuaTable *)
	{
		// Get a list of all non-neutral player objects 
		const FactionClass *neutral_faction = FactionList.Get_Neutral_Faction();
		assert( neutral_faction != NULL );

		PlayerClass *neutral_player = PlayerList.Get_Player_Of_Faction( neutral_faction );
		assert( neutral_player != NULL );

		// Find all objects not owned by the neutral player with a locomotor behavior
		const DynamicVectorClass<GameObjectClass *> *object_list = GAME_OBJECT_MANAGER.Find_Objects
		( 
			BEHAVIOR_LOCO,
			PlayerClass::INVALID_PLAYER_ID, 
			neutral_player->Get_ID()
		);

		assert( object_list != NULL );

		for ( int list_index = 0; list_index < object_list->Get_Size(); list_index ++ ) 
		{
			GameObjectClass *object = (*object_list)[ list_index ];
			assert( object != NULL );
			if ( object != NULL )
			{
				object->Cinematic_Disable();
			}
		}
		if(GameModeManager.Get_Active_Mode() == NULL)
		{

			CollisionSystemClass* collision_system = GameModeManager.Get_Active_Mode()->Get_Collision_System();

			if(collision_system)
				collision_system->Service();
		}
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaDoEndCinematicCleanup, LuaUserVar);


class LuaIsMultiplayerMode : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass* /*script*/, LuaTable* /*params*/)
	{
		return(Return_Variable(new LuaBool(GameModeManager.Is_Multiplayer_Mode() || ScheduledEventQueue.Is_Playing_Back())));
	}
};
PG_IMPLEMENT_RTTI(LuaIsMultiplayerMode, LuaUserVar);

class LuaGetTime : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaGetTime()
	{
		LUA_REGISTER_MEMBER_FUNCTION(LuaGetTime, "Frame", &LuaGetTime::Frame);
		LUA_REGISTER_MEMBER_FUNCTION(LuaGetTime, "Galactic_Time", &LuaGetTime::Galactic_Time);
	}
	virtual LuaTable* Function_Call(LuaScriptClass *, LuaTable *)
	{
		return Return_Variable(new LuaNumber(GameModeManager.Get_Frame_Timer() * FrameSynchronizer.Get_Inv_Logical_FPS()));
	}
	virtual LuaTable* Frame(LuaScriptClass *, LuaTable *)
	{
		return Return_Variable(new LuaNumber(FrameSynchronizer.Get_Seconds_Elapsed()));
	}
	virtual LuaTable* Galactic_Time(LuaScriptClass *, LuaTable *)
	{
		FAIL_IF(GameModeManager.Get_Game_Mode_By_Sub_Type(SUB_GAME_MODE_GALACTIC) == NULL) { return NULL; }

		return Return_Variable(new LuaNumber(GameModeManager.Get_Game_Mode_By_Sub_Type(SUB_GAME_MODE_GALACTIC)->Get_Frame_Timer() * FrameSynchronizer.Get_Inv_Logical_FPS()));
	}


};
PG_IMPLEMENT_RTTI(LuaGetTime, LuaUserVar);

class LuaGameRandom : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaGameRandom()
	{
		LUA_REGISTER_MEMBER_FUNCTION(LuaGameRandom, "Get_Float", &LuaGameRandom::Get_Float);
		LUA_REGISTER_MEMBER_FUNCTION(LuaGameRandom, "Free_Random", &LuaGameRandom::Free_Random);
	}

	virtual LuaTable* Get_Float(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() == 0)
		{
			return Return_Variable(new LuaNumber(SyncRandom.Get_Float(0.0f, 1.0f)));
		}

		LuaNumber::Pointer minval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);
		if (!minval)
		{
			script->Script_Error("GameRandom::Get_Float -- Invalid parameter 1, expected a number");
			return Return_Variable(new LuaNumber(SyncRandom.Get_Float(0.0f, 1.0f)));
		}

		if (params->Value.size() == 1)
		{
			return Return_Variable(new LuaNumber(SyncRandom.Get_Float(0.0f, minval->Value)));
		}

		LuaNumber::Pointer maxval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
		if (!maxval)
		{
			script->Script_Error("GameRandom::Get_Float -- Invalid parameter 2, expected a number");
			return Return_Variable(new LuaNumber(SyncRandom.Get_Float(0.0f, minval->Value)));
		}

		return Return_Variable(new LuaNumber(SyncRandom.Get_Float(minval->Value, maxval->Value)));
	}
	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() == 0)
		{
			return Return_Variable(new LuaNumber((float)SyncRandom.Get(0, 0xffff)));
		}

		LuaNumber::Pointer minval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);
		if (!minval)
		{
			script->Script_Error("GameRandom -- Invalid parameter 1, expected a number");
			return Return_Variable(new LuaNumber((float)SyncRandom.Get(0, 0xffff)));
		}

		int v1 = (int)minval->Value;
		if (params->Value.size() == 1)
		{
			return Return_Variable(new LuaNumber((float)SyncRandom.Get(0, v1)));
		}

		LuaNumber::Pointer maxval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
		if (!maxval)
		{
			script->Script_Error("GameRandom -- Invalid parameter 2, expected a number");
			return Return_Variable(new LuaNumber((float)SyncRandom.Get(0, v1)));
		}
		int v2 = (int)maxval->Value;
		return Return_Variable(new LuaNumber((float)SyncRandom.Get(v1, v2)));
	}

	LuaTable *Free_Random(LuaScriptClass *script, LuaTable *params)
	{
		if (GameModeManager.Is_Multiplayer_Mode())
		{
			script->Script_Warning("GameRandom::Free_Random -- cannot generate non-synchronized random numbers from script in MP.  Falling back to synchronized random.");
			return Function_Call(script, params);
		}

		if (params->Value.size() == 0)
		{
			return Return_Variable(new LuaNumber((float)SyncRandom.Get(0, 0xffff)));
		}

		LuaNumber::Pointer minval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);
		if (!minval)
		{
			script->Script_Error("GameRandom::Free_Random -- Invalid parameter 1, expected a number");
			return Return_Variable(new LuaNumber((float)FreeRandom.Get(0, 0xffff)));
		}

		int v1 = (int)minval->Value;
		if (params->Value.size() == 1)
		{
			return Return_Variable(new LuaNumber((float)FreeRandom.Get(0, v1)));
		}

		LuaNumber::Pointer maxval = LUA_SAFE_CAST(LuaNumber, params->Value[1]);
		if (!maxval)
		{
			script->Script_Error("GameRandom::Free_Random -- Invalid parameter 2, expected a number");
			return Return_Variable(new LuaNumber((float)FreeRandom.Get(0, v1)));
		}
		int v2 = (int)maxval->Value;
		return Return_Variable(new LuaNumber((float)FreeRandom.Get(v1, v2)));
	}
};
PG_IMPLEMENT_RTTI(LuaGameRandom, LuaUserVar);

/**
 * Register the Lua commands for the global commands
 * 
 * @param script script to register commands with
 * @since 4/30/2004 12:03:24 PM -- BMH
 */
void GlobalCommandsClass::Register_Commands(LuaScriptClass *script)
{
	if (script->Pool_Is_Fresh_Load())
	{
		script->Map_Global_To_Lua(ProduceObjectClass::FactoryCreate(), "_ProduceObject");
		script->Map_Global_To_Lua(new FindPlanetClass(), "FindPlanet");
		script->Map_Global_To_Lua(new FindTargetClass(), "FindTarget");
		script->Map_Global_To_Lua(ForeverBlockStatus::FactoryCreate(), "BlockForever");
		script->Map_Global_To_Lua(new FindStageAreaClass(), "_FindStageArea");
		script->Map_Global_To_Lua(new EvaluatePerceptionClass(), "EvaluatePerception");
		script->Map_Global_To_Lua(new GiveDesireBonusClass(), "GiveDesireBonus");
		script->Map_Global_To_Lua(new GetNextStarbaseTypeClass(), "GetNextStarbaseType");
		script->Map_Global_To_Lua(new GetNextGroundbaseTypeClass(), "GetNextGroundbaseType");
		script->Map_Global_To_Lua(new WaitForGroundbaseClass, "WaitForGroundbase");
		script->Map_Global_To_Lua(new WaitForStarbaseClass, "WaitForStarbase");
		script->Map_Global_To_Lua(new EvaluateTypeListClass(), "EvaluateTypeList");
		script->Map_Global_To_Lua(WeightedTypeListClass::FactoryCreate(), "WeightedTypeList");
		script->Map_Global_To_Lua(new FindDeadlyEnemyClass(), "FindDeadlyEnemy");
		script->Map_Global_To_Lua(new FindPlanetClass(), "Find_First_Object");
		script->Map_Global_To_Lua(new PurgeGoalsClass(), "Purge_Goals");
		script->Map_Global_To_Lua(new ApplyMarkupClass(), "Apply_Markup");
		script->Map_Global_To_Lua(new LuaFindPathClass(), "Find_Path");
		script->Map_Global_To_Lua(new LuaStoryEventClass(), "Story_Event");
		script->Map_Global_To_Lua(new EvaluateGalacticContextClass(), "Evaluate_In_Galactic_Context");
		script->Map_Global_To_Lua(new IsCampaignGameClass(), "Is_Campaign_Game");
		script->Map_Global_To_Lua(new FindNearestClass(), "Find_Nearest");
		script->Map_Global_To_Lua(new GetMostDefendedPositionClass(), "Get_Most_Defended_Position");
		script->Map_Global_To_Lua(new ProjectByUnitRangeClass(), "Project_By_Unit_Range");
		script->Map_Global_To_Lua(new LuaReinforceCommandClass(), "Reinforce_Unit");
		script->Map_Global_To_Lua(new LuaFindObjectType(), "Find_Object_Type");
		script->Map_Global_To_Lua(new LuaSpawnUnitCommandClass(), "Spawn_Unit");
		script->Map_Global_To_Lua(new LuaFindMarkerCommandClass(), "Find_Hint");
		script->Map_Global_To_Lua(new LuaPointCamera(), "Point_Camera_At");
		script->Map_Global_To_Lua(new GetStoryPlotClass(), "Get_Story_Plot");
		script->Map_Global_To_Lua(new CheckStoryFlagClass(), "Check_Story_Flag");
		script->Map_Global_To_Lua(new ActivateRetryDialog(), "Activate_Retry_Dialog");
		script->Map_Global_To_Lua(new LuaGameMessageClass(), "Game_Message");
		script->Map_Global_To_Lua(LuaDiscreteDistributionClass::FactoryCreate(), "DiscreteDistribution");
		script->Map_Global_To_Lua(new OppositeSidesOfShieldClass(), "Are_On_Opposite_Sides_Of_Shield");
		script->Map_Global_To_Lua(new LuaFadeOn(), "Fade_On");
		script->Map_Global_To_Lua(new LuaFadeOff(), "Fade_Off");
		script->Map_Global_To_Lua(new LuaLetterBoxOn(), "Letter_Box_On");
		script->Map_Global_To_Lua(new LuaLetterBoxOff(), "Letter_Box_Off");
		script->Map_Global_To_Lua(new LuaLetterBoxIn(), "Letter_Box_In");
		script->Map_Global_To_Lua(new LuaLetterBoxOut(), "Letter_Box_Out");
		script->Map_Global_To_Lua(new LuaFadeScreenIn(), "Fade_Screen_In");
		script->Map_Global_To_Lua(new LuaFadeScreenOut(), "Fade_Screen_Out");
		script->Map_Global_To_Lua(new LuaScrollCameraTo(), "Scroll_Camera_To");
		script->Map_Global_To_Lua(new LuaCameraToFollow(), "Camera_To_Follow");
		script->Map_Global_To_Lua(new LuaZoomCamera(), "Zoom_Camera");
		script->Map_Global_To_Lua(new LuaRotateCameraBy(), "Rotate_Camera_By");
		script->Map_Global_To_Lua(new LuaRotateCameraTo(), "Rotate_Camera_To");
		script->Map_Global_To_Lua(new LuaLockControls(), "Lock_Controls");
		script->Map_Global_To_Lua(new LuaSuspendAI(), "Suspend_AI");
		script->Map_Global_To_Lua(new FindAllObjectsOfType(), "Find_All_Objects_Of_Type");
		script->Map_Global_To_Lua(new FindPlayerClass(), "Find_Player");
		script->Map_Global_To_Lua(new IsPointInNebulaClass(), "Is_Point_In_Nebula");
		script->Map_Global_To_Lua(new IsPointInIonStormClass(), "Is_Point_In_Ion_Storm");
		script->Map_Global_To_Lua(new IsPointInAsteroidFieldClass(), "Is_Point_In_Asteroid_Field");
		script->Map_Global_To_Lua(new LuaFOWRevealCommandClass(), "FogOfWar");
		script->Map_Global_To_Lua(new PlayLightningEffectClass(), "Play_Lightning_Effect");
		script->Map_Global_To_Lua(new AssembleFleetClass(), "Assemble_Fleet");
		script->Map_Global_To_Lua(new LuaFindAllHintsCommandClass, "Find_All_Objects_With_Hint");
		script->Map_Global_To_Lua(new LuaStartCinematicCamera(), "Start_Cinematic_Camera");
		script->Map_Global_To_Lua(new LuaEndCinematicCamera(), "End_Cinematic_Camera");
		script->Map_Global_To_Lua(new LuaSetCinematicTargetKey(), "Set_Cinematic_Target_Key");
		script->Map_Global_To_Lua(new LuaTransitionCinematicTargetKey(), "Transition_Cinematic_Target_Key");
		script->Map_Global_To_Lua(new LuaSetCinematicCameraKey(), "Set_Cinematic_Camera_Key");
		script->Map_Global_To_Lua(new LuaTransitionCinematicCameraKey(), "Transition_Cinematic_Camera_Key");
		script->Map_Global_To_Lua(new LuaTransitionToTacticalCamera(), "Transition_To_Tactical_Camera");
		script->Map_Global_To_Lua(new LuaCinematicZoom(), "Cinematic_Zoom");
		script->Map_Global_To_Lua(new LuaCreateCinematicTransport(), "Create_Cinematic_Transport");
		script->Map_Global_To_Lua(new LuaHideObject(), "Hide_Object");
		script->Map_Global_To_Lua(new LuaHideSubObject(), "Hide_Sub_Object");
		script->Map_Global_To_Lua(new FindNearestSpaceFieldClass(), "Find_Nearest_Space_Field");
		script->Map_Global_To_Lua(new LuaEnableFog(), "Enable_Fog");
		script->Map_Global_To_Lua(new LuaPromoteToSpaceCinematicLayer(), "Promote_To_Space_Cinematic_Layer");
		script->Map_Global_To_Lua(new LuaPlayBinkMovie(), "Play_Bink_Movie");
		script->Map_Global_To_Lua(new LuaStopBinkMovie(), "Stop_Bink_Movie");
		script->Map_Global_To_Lua(new LuaPlayMusicClass(), "Play_Music");
		script->Map_Global_To_Lua(new LuaStopAllMusicClass(), "Stop_All_Music");
		script->Map_Global_To_Lua(new LuaResumeModeBasedMusicClass(), "Resume_Mode_Based_Music");
		script->Map_Global_To_Lua(new LuaForceWeather(), "Force_Weather");
		script->Map_Global_To_Lua(new AddRadarBlipClass(), "Add_Radar_Blip");
		script->Map_Global_To_Lua(new RemoveRadarBlipClass(), "Remove_Radar_Blip");
		script->Map_Global_To_Lua(new AddPlanetHighlightClass(), "Add_Planet_Highlight");
		script->Map_Global_To_Lua(new RemovePlanetHighlightClass(), "Remove_Planet_Highlight");
		script->Map_Global_To_Lua(new LuaResumeHyperspaceIn(), "Resume_Hyperspace_In");
		script->Map_Global_To_Lua(new LuaStopAllSpeech(), "Stop_All_Speech");
		script->Map_Global_To_Lua(new LuaRemoveAllText(), "Remove_All_Text");
		script->Map_Global_To_Lua(new LuaAllowLocalizedSFX(), "Allow_Localized_SFX");	
		script->Map_Global_To_Lua(new LuaMasterVolumeRestore(), "Master_Volume_Restore");
		script->Map_Global_To_Lua(new LuaGetGameMode(), "Get_Game_Mode");
		script->Map_Global_To_Lua(new LuaSetCinematicEnvironment(), "Set_Cinematic_Environment");
		script->Map_Global_To_Lua(new LuaSetNewEnvironment(), "Set_New_Environment");
		script->Map_Global_To_Lua(new LuaStartCinematicMode(), "Start_Cinematic_Mode");
		script->Map_Global_To_Lua(new LuaEndCinematicMode(), "End_Cinematic_Mode");
		script->Map_Global_To_Lua(new LuaCreateGenericObjectClass(), "Create_Generic_Object");
		script->Map_Global_To_Lua(new LuaWeatherAudioPause(), "Weather_Audio_Pause");
		script->Map_Global_To_Lua(new LuaStartCinematicSpaceRetreat(), "Start_Cinematic_Space_Retreat");
		script->Map_Global_To_Lua(new LuaDoEndCinematicCleanup(), "Do_End_Cinematic_Cleanup");
		script->Map_Global_To_Lua(new FindBestLocalThreatCenterClass(), "Find_Best_Local_Threat_Center");
		script->Map_Global_To_Lua(new LuaSFXCommandsClassClass(), "SFXManager");
		script->Map_Global_To_Lua(new LuaAddObjectiveClass, "Add_Objective");
		script->Map_Global_To_Lua(new LuaIsMultiplayerMode(), "Is_Multiplayer_Mode");
		script->Map_Global_To_Lua(new CancelFastForwardClass(), "Cancel_Fast_Forward");
		script->Map_Global_To_Lua(new LuaGetTime, "GetCurrentTime");
		script->Map_Global_To_Lua(new LuaGameRandom, "GameRandom");
		script->Map_Global_To_Lua(new SpawnFromReinforcePoolClass, "Spawn_From_Reinforcement_Pool");
		script->Map_Global_To_Lua(new SpawnSpecialWeaponClass, "Spawn_Special_Weapon");
		script->Map_Global_To_Lua(new EnableDistanceFogClass(), "Enable_Distance_Fog");
		script->Map_Global_To_Lua(new LuaCreatePosition(), "Create_Position");
	}
}
