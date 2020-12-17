// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaGameMessage.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaGameMessage.cpp $
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

#include "LuaGameMessage.h"

#include "GameText.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "CommandBar.h"

PG_IMPLEMENT_RTTI(LuaGameMessageClass, LuaUserVar);

/**************************************************************************************************
* LuaGameMessageClass::Function_Call -- Generate an in-game message.
*
* In:				
*
* Out:		
*
* History: 6/20/2005 1:42PM JSY
**************************************************************************************************/
LuaTable *LuaGameMessageClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	//Currently we just accept the text id of the string to display.  Maybe we should also
	//accept the color?
	if (params->Value.size() != 1)
	{
		script->Script_Error("Game_Message -- invalid number of parameters.  Expected 1, got %d.");
		return NULL;
	}

	if (!TextDisplayCallback)
	{
		script->Script_Error("Game_Message -- no method for text display found.");
		return NULL;
	}

	SmartPtr<LuaString> text_id = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!text_id)
	{
		script->Script_Error("Game_Message -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	TheCommandBar.Add_Advisor_Text(TheGameText.Get(text_id->Value), RGBAClass(255, 255, 255), true);
	//TextDisplayCallback(TheGameText.Get(text_id->Value), RGBAClass(), "i_button_temporary.tga", NULL, NULL);

	return NULL;
}