// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/PlayLightningEffect.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/PlayLightningEffect.cpp $
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

#include "PlayLightningEffect.h"
#include "LightningTypeManager.h"

PG_IMPLEMENT_RTTI(PlayLightningEffectClass, LuaUserVar);
PG_IMPLEMENT_RTTI(LightningEffectBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_LIGHTNING_BLOCK, LightningEffectBlockStatus);

/**************************************************************************************************
* PlayLightningEffectClass::Function_Call -- Script function to play the named lightning effect between
*	the two locations passed as parameters
*
* In:				
*
* Out:	
*
* History: 8/15/2005 7:18PM JSY
**************************************************************************************************/
LuaTable *PlayLightningEffectClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 3)
	{
		script->Script_Error("Play_Lightning_Effect -- invalid number of parameters.  Expected 3, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> effect_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!effect_name)
	{
		script->Script_Error("Play_Lightning_Effect -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	Vector3 source;
	Vector3 target;
	if (!Lua_Extract_Position(params->Value[1], source))
	{
		script->Script_Error("Play_Lightning_Effect -- invalid type for parameter 2.  Expected position of some sort.");
		return NULL;
	}

	if (!Lua_Extract_Position(params->Value[2], target))
	{
		script->Script_Error("Play_Lightning_Effect -- invalid type for parameter 3.  Expected position of some sort.");
		return NULL;
	}

	FAIL_IF(!GameModeManager.Get_Active_Mode()) { return NULL; }

	//Ask for auto-release since the blocking object may be discarded by script so we can't rely on it to be around to clean up
	//the effect.
	LightningEffectClass *effect = TheLightningTypeManager.Create_Lightning_Effect(source, target, effect_name->Value, true);
	if (!effect)
	{
		script->Script_Error("Play_Lightning_Effect -- hmm, somehow we failed to create effect %s.  Maybe it's not the right name?", effect_name->Value.c_str());
		return NULL;
	}

	SceneClass *scene = GameModeManager.Get_Active_Mode()->Get_Scene();
	ENFORCED_IF(scene)
	{
		alScene *al_scene = scene->Get_alScene();
		ENFORCED_IF(al_scene)
		{
			al_scene->Add_Renderable(effect);
		}
	}

	LightningEffectBlockStatus *bs = static_cast<LightningEffectBlockStatus*>(LightningEffectBlockStatus::FactoryCreate());
	FAIL_IF(!bs) { return NULL; }
	bs->Init(NULL, effect);
	return Return_Variable(bs);
}

/**************************************************************************************************
* LightningEffectBlockStatus::Is_Finished -- Check whether this lightning effect is done.
*
* In:				
*
* Out:	
*
* History: 8/15/2005 7:18PM JSY
**************************************************************************************************/
LuaTable *LightningEffectBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(!Effect || Effect->Is_Done()));
}