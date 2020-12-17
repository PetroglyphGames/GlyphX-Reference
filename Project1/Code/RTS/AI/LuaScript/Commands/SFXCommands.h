// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SFXCommands.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SFXCommands.h $
//
//    Original Author: Brian Hayes
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

#ifndef __SFX_COMMANDS_H___
#define __SFX_COMMANDS_H___

#include "AI/LuaScript/LuaRTSUtilities.h"

class LuaSFXCommandsClassClass : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	LuaSFXCommandsClassClass();
	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *params);
	LuaTable *Allow_Localized_SFXEvents(LuaScriptClass *script, LuaTable *params);
	LuaTable *Allow_Unit_Reponse_VO(LuaScriptClass *script, LuaTable *params);
	LuaTable *Allow_HUD_VO(LuaScriptClass *script, LuaTable *params);
	LuaTable *Allow_Ambient_VO(LuaScriptClass *script, LuaTable *params);
	LuaTable *Allow_Enemy_Sighted_VO(LuaScriptClass *script, LuaTable *params);
};


#endif //__SFX_COMMANDS_H___
