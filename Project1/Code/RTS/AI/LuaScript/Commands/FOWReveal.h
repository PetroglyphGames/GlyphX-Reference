// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FOWReveal.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FOWReveal.h $
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

#ifndef __FOW_REVEAL_H___
#define __FOW_REVEAL_H___

#include "AI/LuaScript/LuaRTSUtilities.h"

class LuaFOWRevealCommandClass : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	LuaFOWRevealCommandClass();
	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *params);
	LuaTable *Disable_Rendering(LuaScriptClass *script, LuaTable *params);
	LuaTable *Temporary_Reveal(LuaScriptClass *script, LuaTable *params);
	LuaTable *Reveal_All(LuaScriptClass *script, LuaTable *params);
	LuaTable *Reveal(LuaScriptClass *script, LuaTable *params);
};


#endif //__FOW_REVEAL_H___
