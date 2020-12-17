// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/IsCampaignGame.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/IsCampaignGame.cpp $
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


#include "IsCampaignGame.h"
#include "GameModeManager.h"

PG_IMPLEMENT_RTTI(IsCampaignGameClass, LuaUserVar);

/**************************************************************************************************
* IsCampaignGameClass::Function_Call - Script function to determine whether we're currently playing a 
*	campaign game
*
* In:		script
*			params 
*
* Out:	
*
* History: 3/14/2005 5:58 PM -- JSY
**************************************************************************************************/
LuaTable *IsCampaignGameClass::Function_Call(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(GameModeManager.Get_Sub_Type() == SUB_GAME_MODE_GALACTIC || GameModeManager.Get_Parent_Game_Mode(GameModeManager.Get_Active_Mode())));
}