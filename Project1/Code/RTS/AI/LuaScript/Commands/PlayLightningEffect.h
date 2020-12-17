// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/PlayLightningEffect.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/PlayLightningEffect.h $
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

#ifndef _PLAY_LIGHTNING_EFFECT_H_
#define _PLAY_LIGHTNING_EFFECT_H_

#include "LuaScriptVariable.h"
#include "BlockingStatus.h"
#include "LightningEffect.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class PlayLightningEffectClass : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	virtual LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params);
};

class LightningEffectClass;

class LightningEffectBlockStatus : public BlockingStatus
{
public: 
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_LIGHTNING_BLOCK, LightningEffectBlockStatus);

	void Init(LuaUserVar *command, LightningEffectClass *effect)					{ Set_Command(command); Effect = effect; }
	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *);
	virtual LuaTable *Result(LuaScriptClass *, LuaTable *)							{ return NULL; }

	//Leave Save/Load using base class implementation.  Lightning effects are not saveable, so if we save a game
	//while one is going off we just want the block to finish immediately when we load again.

private:

	SmartPtr<LightningEffectClass> Effect;
};

#endif //_PLAY_LIGHTNING_EFFECT_H_