// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GiveDesireBonus.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GiveDesireBonus.cpp $
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


#include "GiveDesireBonus.h"
#include "Player.h"
#include "AI/AIPlayer.h"
#include "AI/TacticalAIManager.h"
#include "BlockingStatus.h"
#include "AI/Goal/AIGoalSystem.h"
#include "AI/AITargetLocation.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/Goal/TheAIGoalTypeManager.h"
#include "GameObject.h"
#include "GameObjectType.h"
#include "FrameSynchronizer.h"
#include "AI/Goal/AIGoalType.h"


class GiveDesireBlockStatus : public BlockingStatus
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_GIVE_DESIRE_BLOCK, GiveDesireBlockStatus);

	GiveDesireBlockStatus();

	void Init(AIGoalSystemClass *goal_system, const AIGoalTypeClass *goal_type, const AITargetLocationClass *target, float bonus, float time_limit);
	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *);
	virtual LuaTable *Result(LuaScriptClass *, LuaTable *) {return NULL;}
	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

private:	

	int ExpiryFrame;
	AIGoalSystemClass *GoalSystem;
	const AIGoalTypeClass *GoalType;
	const AITargetLocationClass *Target;
	float Bonus;
};

PG_IMPLEMENT_RTTI(GiveDesireBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_GIVE_DESIRE_BLOCK, GiveDesireBlockStatus);


GiveDesireBlockStatus::GiveDesireBlockStatus() :
ExpiryFrame(0),
GoalSystem(0),
GoalType(0),
Target(0),
Bonus(0.0f)
{
}

void GiveDesireBlockStatus::Init(AIGoalSystemClass *goal_system, const AIGoalTypeClass *goal_type, 
											const AITargetLocationClass *target, float bonus, float time_limit)
{
	ExpiryFrame = FrameSynchronizer.Get_Current_Frame() + static_cast<int>(time_limit * FrameSynchronizer.Get_Logical_FPS());
	GoalSystem = goal_system;
	GoalType = goal_type;
	Target = target;
	Bonus = bonus;
}

LuaTable *GiveDesireBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	if (GoalSystem->Give_Desire_Bonus(GoalType, Target, Bonus))
	{
		return Return_Variable(new LuaBool(true));
	}

	return Return_Variable(new LuaBool(FrameSynchronizer.Get_Current_Frame() >= ExpiryFrame));
}

enum
{
	GIVE_DESIRE_BASE_CLASS_CHUNK,
	GIVE_DESIRE_DATA_CHUNK,
	EXPIRY_FRAME_MICRO_CHUNK,
	GOAL_SYSTEM_MICRO_CHUNK,
	GOAL_TYPE_NAME_MICRO_CHUNK,
	TARGET_MICRO_CHUNK,
	BONUS_MICRO_CHUNK,
};

bool GiveDesireBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = true;
	ok &= writer->Begin_Chunk(GIVE_DESIRE_BASE_CLASS_CHUNK);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(GIVE_DESIRE_DATA_CHUNK);
	WRITE_MICRO_CHUNK(EXPIRY_FRAME_MICRO_CHUNK, ExpiryFrame);
	WRITE_MICRO_CHUNK(BONUS_MICRO_CHUNK, Bonus);
	WRITE_MICRO_CHUNK_OBJECT_PTR(GOAL_SYSTEM_MICRO_CHUNK, GoalSystem);
	WRITE_MICRO_CHUNK_OBJECT_PTR(TARGET_MICRO_CHUNK, Target);
	WRITE_MICRO_CHUNK_STRING(GOAL_TYPE_NAME_MICRO_CHUNK, GoalType->Get_Name());
	ok &= writer->End_Chunk();

	return ok;
}

bool GiveDesireBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;
	std::string goal_type_name;

	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
		case GIVE_DESIRE_BASE_CLASS_CHUNK:
			ok &= BlockingStatus::Load(script, reader);
			break;
		case GIVE_DESIRE_DATA_CHUNK:
			{
				while (reader->Open_Micro_Chunk())
				{
					switch (reader->Cur_Micro_Chunk_ID())
					{
						READ_MICRO_CHUNK(EXPIRY_FRAME_MICRO_CHUNK, ExpiryFrame);
						READ_MICRO_CHUNK(BONUS_MICRO_CHUNK, Bonus);
						READ_MICRO_CHUNK_OBJECT_PTR(GOAL_SYSTEM_MICRO_CHUNK, GoalSystem);
						READ_MICRO_CHUNK_OBJECT_PTR(TARGET_MICRO_CHUNK, Target);
						READ_MICRO_CHUNK_STRING(GOAL_TYPE_NAME_MICRO_CHUNK, goal_type_name);
					default:
						ok = false;
						break;
					}

					reader->Close_Micro_Chunk();
				}
				break;
			}
		default:
			ok = false;
			break;
		}
		reader->Close_Chunk();
	}

	GoalType = TheAIGoalTypeManagerPtr->Get_Managed_Object(goal_type_name);
	if (!GoalType)
	{
		ok = false;
	}

	return ok;
}

////////////////////////////////////////////////////////////////////////////////////////////////

PG_IMPLEMENT_RTTI(GiveDesireBonusClass, LuaUserVar);

LuaTable *GiveDesireBonusClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	//Verify parameters
	if (params->Value.size() != 5)
	{
		script->Script_Error("Give_Desire_Bonus : invalid number of parameters.  Expected 4, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<PlayerWrapper> player = LUA_SAFE_CAST(PlayerWrapper, params->Value[0]);
	if (!player)
	{
		script->Script_Error("Give_Desire_Bonus : invalid type for parameter 1.  Expected player.");
		return NULL;
	}

	LuaString *goal_name = PG_Dynamic_Cast<LuaString>(params->Value[1]);
	if (!goal_name)
	{
		script->Script_Error("Give_Desire_Bonus : invalid type for parameter 2.  Expected string.");
		return NULL;
	}

	//Accept either a game object or an AITargetLocation for parameter 3.
	SmartPtr<AITargetLocationWrapper> target_wrapper = LUA_SAFE_CAST(AITargetLocationWrapper, params->Value[2]);
	SmartPtr<GameObjectWrapper> object;
	if (!target_wrapper)
	{
		object = LUA_SAFE_CAST(GameObjectWrapper, params->Value[2]);

		if (!object)
		{
			script->Script_Error("Give_Desire_Bonus : invalid type for parameter 2.  Expected AI target location or game object.");
			return NULL;
		}
	}

	LuaNumber *bonus = PG_Dynamic_Cast<LuaNumber>(params->Value[3]);
	if (!bonus)
	{
		script->Script_Error("Give_Desire_Bonus : invalid type for parameter 3.  Expected number.");
		return NULL;
	}

	LuaNumber *time_limit = PG_Dynamic_Cast<LuaNumber>(params->Value[4]);
	if (!time_limit)
	{
		script->Script_Error("Give_Desire_Bonus : invalid type for parameter 4.  Expected number.");
		return NULL;
	}

	//Find the goal type.
	AIGoalTypeClass *goal_type = TheAIGoalTypeManagerPtr->Get_Managed_Object(goal_name->Value);
	if (!goal_type)
	{
		script->Script_Error("Give_Desire_Bonus : unrecognised goal type %s.", goal_name->Value.c_str());
		return NULL;
	}

	AIPlayerClass *ai_player = player->Get_Object()->Get_AI_Player();
	TacticalAIManagerClass *manager = ai_player->Get_Tactical_Manager_By_Mode(GameModeManager.Get_Active_Mode()->Get_Sub_Type());
	AIGoalSystemClass *goal_system = manager->Get_Goal_System();

	const AITargetLocationClass *target;
	if (target_wrapper)
	{
		target = target_wrapper->Get_Object();
	}
	else
	{
		target = goal_system->Find_Target(object->Get_Object());

		if (!target)
		{
			script->Script_Error("Give_Desire_Bonus : No AI target location corresponds to game object of type %s.", object->Get_Object()->Get_Type()->Get_Name()->c_str());
			return NULL;
		}
	}

	GiveDesireBlockStatus *bs = static_cast<GiveDesireBlockStatus*>(GiveDesireBlockStatus::FactoryCreate());
	bs->Init(goal_system, goal_type, target, bonus->Value, time_limit->Value);

	return Return_Variable(bs);
}