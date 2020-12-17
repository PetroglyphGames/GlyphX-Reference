// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/BudgetWrapper.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/BudgetWrapper.cpp $
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

#pragma hdrstop

#include "BudgetWrapper.h"
#include "BlockingStatus.h"
#include "AI/Planning/PlanBehavior.h"
#include "AI/Goal/InstantiatedGoal.h"
#include "MathConstant.h"
#include "AI/Goal/AIGoalCategoryType.h"
#include "AI/Goal/AIGoalSystem.h"
#include "AI/Goal/AIBudget.h"
#include "DynamicEnum.h"


class BudgetBlockStatus : public BlockingStatus
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_BUDGET_BLOCK, BudgetBlockStatus);

	BudgetBlockStatus();

	void Init(LuaUserVar *command, PlanBehaviorClass *plan, float resources_to_wait_for, bool block_on_spendable);
	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *);
	virtual LuaTable *Result(LuaScriptClass *, LuaTable *);
	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

private:

	bool BlockOnSpendable;
	PlanBehaviorClass *Plan;
	float ResourcesToWaitFor;
};

PG_IMPLEMENT_RTTI(BudgetBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_BUDGET_BLOCK, BudgetBlockStatus);

BudgetBlockStatus::BudgetBlockStatus() :
BlockOnSpendable(false),
Plan(0),
ResourcesToWaitFor(0.0f)
{
}

void BudgetBlockStatus::Init(LuaUserVar *command, PlanBehaviorClass *plan, float resources_to_wait_for, bool block_on_spendable)
{
	Set_Command(command);
	Plan = plan;
	ResourcesToWaitFor = resources_to_wait_for;
	BlockOnSpendable = block_on_spendable;
}

LuaTable *BudgetBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	if (BlockOnSpendable)
	{
		return Return_Variable(new LuaBool(Plan->Get_Goal()->Get_Spendable_Resources() - ResourcesToWaitFor >= -FLOAT_EPSILON));
	}
	else
	{
		return Return_Variable(new LuaBool(Plan->Get_Goal()->Get_Unallocated_Resource_Value() - ResourcesToWaitFor >= -FLOAT_EPSILON));
	}
}

LuaTable *BudgetBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////

PG_IMPLEMENT_RTTI(BudgetWrapper, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_BUDGET, BudgetWrapper);

BudgetWrapper::BudgetWrapper(PlanBehaviorClass *plan) :
Plan(plan)
{
	LUA_REGISTER_MEMBER_FUNCTION(BudgetWrapper, "Get_Unallocated_Resources", &BudgetWrapper::Get_Unallocated_Resources);
	LUA_REGISTER_MEMBER_FUNCTION(BudgetWrapper, "Get_Spendable_Resources", &BudgetWrapper::Get_Spendable_Resources);
	LUA_REGISTER_MEMBER_FUNCTION(BudgetWrapper, "Allocate_Resources", &BudgetWrapper::Allocate_Resources);
	LUA_REGISTER_MEMBER_FUNCTION(BudgetWrapper, "Wait_For_Spendable_Resources", &BudgetWrapper::Wait_For_Spendable_Resources);
	LUA_REGISTER_MEMBER_FUNCTION(BudgetWrapper, "Wait_For_Unallocated_Resources", &BudgetWrapper::Wait_For_Unallocated_Resources);
	LUA_REGISTER_MEMBER_FUNCTION(BudgetWrapper, "Give_Resources_To_Goal", &BudgetWrapper::Give_Resources_To_Goal);
	LUA_REGISTER_MEMBER_FUNCTION(BudgetWrapper, "Take_Resources_From_Goal", &BudgetWrapper::Take_Resources_From_Goal);
	LUA_REGISTER_MEMBER_FUNCTION(BudgetWrapper, "Flush_Unallocated_Resources", &BudgetWrapper::Flush_Unallocated_Resources);
	LUA_REGISTER_MEMBER_FUNCTION(BudgetWrapper, "Flush_All_Resources", &BudgetWrapper::Flush_All_Resources);
	LUA_REGISTER_MEMBER_FUNCTION(BudgetWrapper, "Flush_Category", &BudgetWrapper::Flush_Category);
}

LuaTable *BudgetWrapper::Get_Unallocated_Resources(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaNumber(static_cast<float>(Plan->Get_Goal()->Get_Unallocated_Resource_Value())));
}

LuaTable *BudgetWrapper::Get_Spendable_Resources(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaNumber(static_cast<float>(Plan->Get_Goal()->Get_Spendable_Resources())));
}

LuaTable *BudgetWrapper::Allocate_Resources(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("BudgetWrapper::Allocate_Resources -- Invalid number of parameters: %d should be 1.\n", 
										params->Value.size());
		return NULL;
	}

	LuaNumber *resources = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!resources)
	{
		script->Script_Error("BudgetWrapper::Allocate_Resources -- Invalid parameter type for parameter 1.  Should be number.\n");
		return NULL;
	}

	Plan->Get_Goal()->Allocate_Resources(resources->Value);

	return NULL;
}

LuaTable *BudgetWrapper::Flush_All_Resources(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 0)
	{
		script->Script_Error("BudgetWrapper::Release_Resources -- Invalid number of parameters: %d should be 0.\n", 
										params->Value.size());
		return NULL;
	}

	Plan->Get_Goal()->Release_Resources(Plan->Get_Goal()->Compute_Budget());
	Plan->Get_Goal()->Flush_Unallocated_Resources();

	return NULL;
}

LuaTable *BudgetWrapper::Flush_Unallocated_Resources(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 0)
	{
		script->Script_Error("BudgetWrapper::Flush_Unallocated_Resources -- Invalid number of parameters: %d should be 0.\n", 
										params->Value.size());
		return NULL;
	}

	Plan->Get_Goal()->Flush_Unallocated_Resources();

	return NULL;
}

LuaTable *BudgetWrapper::Wait_For_Spendable_Resources(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("BudgetWrapper::Wait_For_Spendable_Resources -- Invalid number of parameters: %d should be 1.\n", 
										params->Value.size());
		return NULL;
	}

	LuaNumber *resources = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!resources)
	{
		script->Script_Error("BudgetWrapper::Wait_For_Spendable_Resources -- Invalid parameter type for parameter 1.  Should be number.\n");
		return NULL;
	}

	BudgetBlockStatus *budget_block = static_cast<BudgetBlockStatus*>(BudgetBlockStatus::FactoryCreate());

	budget_block->Init(this, Plan, resources->Value, true);

	return Return_Variable(budget_block);
}

LuaTable *BudgetWrapper::Wait_For_Unallocated_Resources(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("BudgetWrapper::Wait_For_Unallocated_Resources -- Invalid number of parameters: %d should be 1.\n", 
										params->Value.size());
		return NULL;
	}

	LuaNumber *resources = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!resources)
	{
		script->Script_Error("BudgetWrapper::Wait_For_Uncallocated_Resources -- Invalid parameter type for parameter 1.  Should be number.\n");
		return NULL;
	}

	BudgetBlockStatus *budget_block = static_cast<BudgetBlockStatus*>(BudgetBlockStatus::FactoryCreate());

	budget_block->Init(this, Plan, resources->Value, false);

	return Return_Variable(budget_block);
}

void BudgetWrapper::Mark_Resources_For_Spend(float resources)
{
	Plan->Get_Goal()->Mark_Resources_For_Spend(resources);
}

void BudgetWrapper::Spend_Resources(float resources)
{
	Plan->Get_Goal()->Spend_Resources(resources);
}

LuaTable *BudgetWrapper::Give_Resources_To_Goal(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 2)
	{
		script->Script_Error("BudgetWrapper::Give_Resources_To_Goal -- Invalid number of parameters: %d should be 2.\n", 
										params->Value.size());
		return NULL;
	}

	LuaNumber *resources = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!resources)
	{
		script->Script_Error("BudgetWrapper::Give_Resources_To_Goal -- Invalid parameter type for parameter 1.  Should be number.\n");
		return NULL;
	}

	if (resources->Value < 0)
	{
		script->Script_Error("BudgetWrapper::Give_Resources_To_Goal -- Invalid value %.2f for parameter 1.  Minimum value is zero.  Use Take_Resources_From_Goal instead.\n", resources->Value);
		return NULL;
	}

	LuaString *goal_name = PG_Dynamic_Cast<LuaString>(params->Value[1]);
	if (!goal_name)
	{
		script->Script_Error("BudgetWrapper::Give_Resources_To_Goal -- Invalid parameter type for parameter 2.  Should be string.\n");
		return NULL;
	}

	bool success = Plan->Get_Goal()->Transfer_Resources(resources->Value, goal_name->Value);

	return Return_Variable(new LuaBool(success));
}

LuaTable *BudgetWrapper::Take_Resources_From_Goal(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 2)
	{
		script->Script_Error("BudgetWrapper::Take_Resources_From_Goal -- Invalid number of parameters: %d should be 2.", 
										params->Value.size());
		return NULL;
	}

	LuaNumber *resources = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!resources)
	{
		script->Script_Error("BudgetWrapper::Take_Resources_From_Goal -- Invalid parameter type for parameter 1.  Should be number.");
		return NULL;
	}

	if (resources->Value < 0)
	{
		script->Script_Error("BudgetWrapper::Take_Resources_From_Goal -- Invalid value %.2f for parameter 1.  Minimum value is zero.  Use Give_Resources_To_Goal instead.", resources->Value);
		return NULL;
	}

	LuaString *goal_name = PG_Dynamic_Cast<LuaString>(params->Value[1]);
	if (!goal_name)
	{
		script->Script_Error("BudgetWrapper::Take_Resources_From_Goal -- Invalid parameter type for parameter 2.  Should be string.");
		return NULL;
	}

	bool success = Plan->Get_Goal()->Transfer_Resources(-resources->Value, goal_name->Value);

	return Return_Variable(new LuaBool(success));
}

LuaTable *BudgetWrapper::Flush_Category(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("BudgetWrapper::Flush_Category -- Invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> category_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!category_name)
	{
		script->Script_Error("BudgetWrapper::Flush_Category -- invalid type for parameter 1.  Expected string");
		return NULL;
	}

	AIGoalCategoryType category;
	if (!TheAIGoalCategoryTypeConverterPtr->String_To_Enum(category_name->Value, category))
	{
		script->Script_Error("BudgetWrapper::Flush_Category -- unrecognized goal category %s.", category_name->Value.c_str());
		return NULL;
	}

	Plan->Get_Goal()->Get_Goal_System()->Get_Budget().Flush_Category(category);

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////

enum
{
	BUDGET_DATA_CHUNK,
		BUDGET_PLAN_CHUNK,

	BUDGET_BLOCK_BASE_CLASS_CHUNK,
	BUDGET_BLOCK_DATA_CHUNK,
		BUDGET_BLOCK_BLOCK_ON_SPENDABLE_CHUNK,
		BUDGET_BLOCK_RESOURCES_CHUNK,
		BUDGET_BLOCK_PLAN_CHUNK,
};

bool BudgetWrapper::Save(LuaScriptClass *, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(BUDGET_DATA_CHUNK);
	WRITE_MICRO_CHUNK_OBJECT_PTR(BUDGET_PLAN_CHUNK, Plan);
	ok &= writer->End_Chunk();

	return ok;
}

bool BudgetWrapper::Load(LuaScriptClass *, ChunkReaderClass *reader)
{
	bool ok = reader->Open_Chunk();

	while (reader->Open_Micro_Chunk())
	{
		switch (reader->Cur_Micro_Chunk_ID())
		{
			READ_MICRO_CHUNK_OBJECT_PTR(BUDGET_PLAN_CHUNK, Plan);

		default:
			ok = false;
			break;
		}

		reader->Close_Micro_Chunk();
	}

	reader->Close_Chunk();

	return ok;
}

bool BudgetBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(BUDGET_BLOCK_BASE_CLASS_CHUNK);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(BUDGET_BLOCK_DATA_CHUNK);
	WRITE_MICRO_CHUNK(BUDGET_BLOCK_BLOCK_ON_SPENDABLE_CHUNK, BlockOnSpendable);
	WRITE_MICRO_CHUNK(BUDGET_BLOCK_RESOURCES_CHUNK, ResourcesToWaitFor);
	WRITE_MICRO_CHUNK_OBJECT_PTR(BUDGET_BLOCK_PLAN_CHUNK, Plan);
	ok &= writer->End_Chunk();

	return ok;
}

bool BudgetBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
		case BUDGET_BLOCK_BASE_CLASS_CHUNK:
			ok &= BlockingStatus::Load(script, reader);
			break;

		case BUDGET_BLOCK_DATA_CHUNK:
			while (reader->Open_Micro_Chunk())
			{
				switch (reader->Cur_Micro_Chunk_ID())
				{
					READ_MICRO_CHUNK(BUDGET_BLOCK_BLOCK_ON_SPENDABLE_CHUNK, BlockOnSpendable);
					READ_MICRO_CHUNK(BUDGET_BLOCK_RESOURCES_CHUNK, ResourcesToWaitFor);
					READ_MICRO_CHUNK_OBJECT_PTR(BUDGET_BLOCK_PLAN_CHUNK, Plan);

				default:
					ok = false;
					break;
				}

				reader->Close_Micro_Chunk();
			}
			break;
		}
		reader->Close_Chunk();
	}

	return ok;
}