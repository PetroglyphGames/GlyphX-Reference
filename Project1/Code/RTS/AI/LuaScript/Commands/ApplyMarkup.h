// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ApplyMarkup.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ApplyMarkup.h $
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

#ifndef _APPLY_MARKUP_H_
#define _APPLY_MARKUP_H_

#include "AI/LuaScript/LuaRTSUtilities.h"

class GameObjectClass;
class PlayerClass;
class AITargetLocationClass;
class PerceptualEvaluatorClass;
class MarkupBlockStatus;

class ApplyMarkupClass : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	virtual LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params);

protected:
	
	void Apply_Markup(PerceptualEvaluatorClass *evaluator, float value, MarkupBlockStatus *blocking_status);

	void Apply_Markup(const AITargetLocationClass *target, float value, MarkupBlockStatus *blocking_status);
	void Apply_Markup(PlayerClass *for_player, GameObjectClass *target, float value, MarkupBlockStatus *blocking_status);
	void Apply_Markup(PlayerClass *for_player, PlayerClass *target, float value, MarkupBlockStatus *blocking_status);
	void Apply_Markup(PlayerClass *for_player, LuaTable *target_list, float value, MarkupBlockStatus *blocking_status);
};

#endif