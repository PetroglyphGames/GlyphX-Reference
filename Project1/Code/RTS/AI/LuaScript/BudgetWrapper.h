// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/BudgetWrapper.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/BudgetWrapper.h $
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

#ifndef _BUDGET_WRAPPER_H_
#define _BUDGET_WRAPPER_H_

#include "LuaRTSUtilities.h"

class PlanBehaviorClass;

class BudgetWrapper : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_BUDGET, BudgetWrapper);

	BudgetWrapper(PlanBehaviorClass *plan = 0);

	LuaTable *Get_Unallocated_Resources(LuaScriptClass *, LuaTable *);
	LuaTable *Get_Spendable_Resources(LuaScriptClass *, LuaTable *);

	LuaTable *Allocate_Resources(LuaScriptClass *script, LuaTable *params);
	LuaTable *Flush_Unallocated_Resources(LuaScriptClass *script, LuaTable *params);
	LuaTable *Flush_All_Resources(LuaScriptClass *script, LuaTable *params);

	LuaTable *Give_Resources_To_Goal(LuaScriptClass *script, LuaTable *params);
	LuaTable *Take_Resources_From_Goal(LuaScriptClass *script, LuaTable *params);
	
	LuaTable *Wait_For_Spendable_Resources(LuaScriptClass *script, LuaTable *params);
	LuaTable *Wait_For_Unallocated_Resources(LuaScriptClass *script, LuaTable *params);

	LuaTable *Flush_Category(LuaScriptClass *script, LuaTable *params);

	void Mark_Resources_For_Spend(float resources);
	void Spend_Resources(float resources);

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

	void Set_Plan(PlanBehaviorClass *plan) {Plan = plan;}

	virtual LuaTable *Is_Pool_Safe(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(false)); }

private:

	PlanBehaviorClass *Plan;
};

#endif //_LUA_BUDGET_H_