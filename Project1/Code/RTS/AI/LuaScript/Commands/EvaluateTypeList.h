// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/EvaluateTypeList.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/EvaluateTypeList.h $
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

#ifndef _EVALUATE_TYPE_LIST_H_
#define _EVALUATE_TYPE_LIST_H_

#include "AI/LuaScript/LuaRTSUtilities.h"

class PlayerWrapper;
class AITargetLocationWrapper;
class WeightedTypeListClass;

class EvaluateTypeListClass : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	virtual LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params);

private:
	LuaTable *Galactic_Evaluate(LuaScriptClass *script, PlayerWrapper *player, AITargetLocationWrapper *target, WeightedTypeListClass *tlist);
	LuaTable *Tactical_Evaluate(LuaScriptClass *script, PlayerWrapper *player, AITargetLocationWrapper *target, WeightedTypeListClass *tlist);
};


#endif //_EVALUATE_TYPE_LIST_H_
