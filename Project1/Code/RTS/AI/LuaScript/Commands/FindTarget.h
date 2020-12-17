// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindTarget.h#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindTarget.h $
//
//    Original Author: James Yarrow
//
//            $Author: Brian_Hayes $
//
//            $Change: 641502 $
//
//          $DateTime: 2017/05/09 13:45:27 $
//
//          $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */

#ifndef __FINDTARGET_H__
#define __FINDTARGET_H__

#include "AI/LuaScript/LuaRTSUtilities.h"
#include "AI/Goal/AIGoalApplicationType.h"

class PerceptionFunctionClass;
class TaskForceClass;

//Script command to search for a target object for a given goal
class FindTargetClass : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	FindTargetClass();
	LuaTable *Reachable_Target(LuaScriptClass *script, LuaTable *params);
	virtual LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params);
	LuaTable *Best_Of(LuaScriptClass *script, LuaTable *params);

private:

	bool Validate_Call(LuaScriptClass *script, 
						LuaTable *params, 
						TaskForceClass *&taskforce, 
						PerceptionFunctionClass *&function,
						AIGoalApplicationType &application_type,
						float &best_choice_scale_factor,
						float &max_distance);
};

#endif //#ifndef __FINDTARGET_H__