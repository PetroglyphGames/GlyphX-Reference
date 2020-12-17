// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceReinforceBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceReinforceBlock.cpp $
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

#pragma hdrstop

#include "Always.h"

#include "SpaceReinforceBlock.h"
#include "GameObjectManager.h"
#include "GameModeManager.h"
#include "PGSignal/SignalDispatcher.h"
#include "GameObject.h"
#include "AI/Planning/TaskForce.h"
#include "AI/Planning/PlanBehavior.h"
#include "Player.h"
#include "SpaceMode.h"
#include "GameRandom.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/Planning/AIPlanningSystem.h"
#include "AI/TacticalAIManager.h"
#include "AI/Execution/AIExecutionSystem.h"
#include "AI/Execution/AIFreeStore.h"
#include "AI/AIPlayer.h"

PG_IMPLEMENT_RTTI(SpaceReinforceBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_SPACE_REINFORCE_BLOCK, SpaceReinforceBlockStatus);

static const int AI_SPACE_REINFORCEMENT_ATTEMPTS_PER_UPDATE = 10;
static const float AI_SPACE_REINFORCEMENT_RADIUS_EXPANSION = 500.0f;

SpaceReinforceBlockStatus::SpaceReinforceBlockStatus() :
	Script(NULL),
	PlayerID(-1),
	TimeOutFrame(-1),
	IncomingShipCount(0),
	Radius(0.0f),
	ObeyReinforceRules(true)
{
}

void SpaceReinforceBlockStatus::Init(LuaUserVar *command, const std::vector<int> &reinforcement_indices, const Vector3 &target_position, float time_out, LuaScriptClass *script, int player_id)
{
	Set_Command(command);

	Script = script;
	PlayerID = player_id;
	Radius = 0.0f;

	ReinforcementIndices = reinforcement_indices;
	TargetPosition = target_position;
	TargetPosition.Z = 0.0f;
	if (time_out != BIG_FLOAT)
	{
		TimeOutFrame = FrameSynchronizer.Get_Current_Frame() + static_cast<int>(time_out * FrameSynchronizer.Get_Logical_FPS());
	}

	//Give ourselves a head start if we start out in a bad spot.
	SpaceModeClass *space_mode = static_cast<SpaceModeClass*>(GameModeManager.Get_Active_Mode());
	if (space_mode->Point_Overlaps_Reinforcement_Modifying_Objects(PlayerID, TargetPosition))
	{
		Radius = 2.0f * AI_SPACE_REINFORCEMENT_RADIUS_EXPANSION;
	}
}

LuaTable *SpaceReinforceBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	if (ReinforcementIndices.size() > 0)
	{
		Attempt_Reinforcement();
	}

	bool timed_out = (TimeOutFrame >= 0 && FrameSynchronizer.Get_Current_Frame() >= TimeOutFrame);
	bool all_reinforced = (ReinforcementIndices.size() == 0 && IncomingShipCount <= 0);

	return Return_Variable(new LuaBool(timed_out || all_reinforced));
}

LuaTable *SpaceReinforceBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	if (!ReinforceUnits)
	{
		return Return_Variable(new LuaBool(ReinforcementIndices.size() == 0));
	}

	LuaVar::Pointer val = ReinforceUnits;
	ReinforceUnits = 0;
	return Return_Variable(val);
}

void SpaceReinforceBlockStatus::Receive_Signal(SignalGeneratorClass *gen, PGSignalType, SignalDataClass *)
{
	--IncomingShipCount;
	assert(IncomingShipCount >= 0);

	GameObjectClass *object = static_cast<GameObjectClass*>(gen);
	TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(Get_Command());

	if (!tf)
	{
		assert(Script);
		if (!ReinforceUnits)
		{
			ReinforceUnits = Alloc_Lua_Table();
		}

		ReinforceUnits->Value.push_back(GameObjectWrapper::Create(object, Script));
		AIPlayerClass *ai_player = object->Get_Owner_Player()->Get_AI_Player();
		if (ai_player)
		{
			AIFreeStoreClass *freestore = (*ai_player->Find_Tactical_Manager_By_Mode(GameModeManager.Get_Active_Mode()))->Get_Execution_System()->Get_Free_Store();
			freestore->Remove_Free_Store_Object(object);
		}
		return;
	}


	SmartPtr<LuaTable> params = Return_Variable(GameObjectWrapper::Create(object, tf->Get_Plan()->Get_Script()));
	tf->Add_Force(tf->Get_Plan()->Get_Script(), params);
	AIFreeStoreClass *free_store = tf->Get_Plan()->Get_Planning_System()->Get_Manager()->Get_Execution_System()->Get_Free_Store();
	free_store->Remove_Free_Store_Object(object);
	Free_Lua_Table(params);
}

void SpaceReinforceBlockStatus::Attempt_Reinforcement()
{
	FAIL_IF(GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_SPACE) { return; }
	SpaceModeClass *space_mode = static_cast<SpaceModeClass*>(GameModeManager.Get_Active_Mode());
	FAIL_IF(!space_mode) { return; }
	PlayerClass *player = PlayerList.Get_Player_By_ID(PlayerID);
	FAIL_IF(!player) { return; }

	const GameObjectTypeClass *type = GAME_OBJECT_MANAGER.Get_Reinforcement_Type(ReinforcementIndices.back(), PlayerID);
	if (!type)
	{
		//We should have already filtered out bad indices in skirmish modes!
		assert(!GameModeManager.Is_Multiplayer_Tactical_Game());
		ReinforcementIndices.pop_back();
		return;
	}

	//If we're following reinforcement rules then check pop cap 
	if (ObeyReinforceRules && !GAME_OBJECT_MANAGER.Is_Room_For_Reinforcements(PlayerID, type))
	{
		return;
	}

	FacingClass reinforcement_facing = FacingClass(space_mode->Get_Player_Reinforcement_Facing(player->Get_Index()));

	Vector2 x_axis = -reinforcement_facing.Get_XY_Vector();
	Vector2 y_axis = Vector2(-x_axis.Y, x_axis.X);

	Vector3 reinforce_position;
	if (!ObeyReinforceRules)
	{
		reinforce_position = TargetPosition;
		Radius += AI_SPACE_REINFORCEMENT_RADIUS_EXPANSION;
	}
	else
	{
		Vector3 radial_vector = -Vector3(reinforcement_facing.Get_XY_Vector());
		Box3Class world_bounds = space_mode->Get_Playable_Bounds();
		bool found_spot = false;
		for (int i = 0; i < AI_SPACE_REINFORCEMENT_ATTEMPTS_PER_UPDATE; ++i)
		{
			reinforce_position = TargetPosition + Radius * radial_vector;

			reinforce_position.X = PGMath::Clamp(reinforce_position.X, world_bounds.Get_XMin(), world_bounds.Get_XMax());
			reinforce_position.Y = PGMath::Clamp(reinforce_position.Y, world_bounds.Get_YMin(), world_bounds.Get_YMax());

			if (space_mode->Can_Land_Reinforcements(player->Get_ID(), reinforce_position, type))
			{
				found_spot = true;
				break;
			}

			if (Radius == 0.0f)
			{
				break;
			}

			radial_vector.Rotate_Z(2.0f * FLOAT_M_PI / AI_SPACE_REINFORCEMENT_ATTEMPTS_PER_UPDATE);
		}

		if (!found_spot)
		{
			Radius += AI_SPACE_REINFORCEMENT_RADIUS_EXPANSION;
			return;
		}
	}

	int reinforcement_id = ReinforcementIndices.back();
	if (GameModeManager.Is_Multiplayer_Tactical_Game() == false)
	{
		ObjectPersistenceClass *persist_info = GameModeManager.Get_Parent_Mode_Persistent_Objects(space_mode);
		FAIL_IF(!persist_info) { return; }
		//Campaign
		reinforcement_id = persist_info->Get_Reinforcements(PlayerID)[ReinforcementIndices.back()].ObjectID;
	}

	GameObjectClass *new_object = GAME_OBJECT_MANAGER.Space_Reinforce(reinforcement_id, reinforce_position, PlayerID);
	FAIL_IF(!new_object) { return; }
	TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(Get_Command());
	if (!tf)
	{
		AIPlayerClass *ai_player = new_object->Get_Owner_Player()->Get_AI_Player();
		if (ai_player)
		{
			TacticalAIManagerClass *manager = (*ai_player->Find_Tactical_Manager_By_Mode(GameModeManager.Get_Active_Mode()));
			if (manager)
			{
				AIExecutionSystemClass *exec = manager->Get_Execution_System();
				if (exec)
				{
					AIFreeStoreClass *freestore = exec->Get_Free_Store();
					if (freestore)
					{
						freestore->Remove_Free_Store_Object(new_object);
					}
				}
			}
		}
	}

	SignalDispatcherClass::Get().Add_Listener(new_object, this, PG_SIGNAL_OBJECT_REINFORCEMENTS_UNLOADED);
	++IncomingShipCount;
	ReinforcementIndices.pop_back();

	if (ReinforcementIndices.size() > 0)
	{
		if (space_mode->Point_Overlaps_Reinforcement_Modifying_Objects(PlayerID, TargetPosition))
		{
			Radius = 2.0f * AI_SPACE_REINFORCEMENT_RADIUS_EXPANSION;
		}
		else
		{
			Radius = 0.0f;
		}
	}
}

enum
{
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_REINFORCE_BLOCK_DATA,
	CHUNK_ID_REINFORCE_BLOCK_LISTENER_BASE_ID,
	CHUNK_ID_REINFORCE_BLOCK_INDICES,
	CHUNK_ID_REINFORCE_BLOCK_POSITION,
	CHUNK_ID_REINFORCE_BLOCK_INCOMING_SHIPS,
	CHUNK_ID_REINFORCE_BLOCK_TIME_OUT_FRAME,
	CHUNK_ID_REINFORCE_BLOCK_BOX_EXTENT,
	CHUNK_ID_REINFORCE_BLOCK_ATTEMPTS_TO_EXPANSION,
	CHUNK_ID_REINFORCE_BLOCK_UNITS,
	CHUNK_ID_REINFORCE_BLOCK_PLAYER_ID,
	CHUNK_ID_REINFORCE_BLOCK_OBEY_RULES,
	CHUNK_ID_REINFORCE_BLOCK_RADIUS,
};

bool SpaceReinforceBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(CHUNK_ID_REINFORCE_BLOCK_DATA);
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_REINFORCE_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
	WRITE_MICRO_CHUNK_VECTOR3(CHUNK_ID_REINFORCE_BLOCK_POSITION, TargetPosition);
	WRITE_MICRO_CHUNK(CHUNK_ID_REINFORCE_BLOCK_INCOMING_SHIPS, IncomingShipCount);
	WRITE_MICRO_CHUNK(CHUNK_ID_REINFORCE_BLOCK_TIME_OUT_FRAME, TimeOutFrame);
	WRITE_MICRO_CHUNK(CHUNK_ID_REINFORCE_BLOCK_RADIUS, Radius);
	WRITE_MICRO_CHUNK(CHUNK_ID_REINFORCE_BLOCK_PLAYER_ID, PlayerID);
	WRITE_MICRO_CHUNK(CHUNK_ID_REINFORCE_BLOCK_OBEY_RULES, ObeyReinforceRules);
	ok &= writer->End_Chunk();

	WRITE_STL_VECTOR(CHUNK_ID_REINFORCE_BLOCK_INDICES, ReinforcementIndices);
	LUA_WRITE_CHUNK_VALUE_PTR(CHUNK_ID_REINFORCE_BLOCK_UNITS,						ReinforceUnits, 			script);

	return ok;
}

bool SpaceReinforceBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
			READ_STL_VECTOR(CHUNK_ID_REINFORCE_BLOCK_INDICES, ReinforcementIndices);
			LUA_READ_CHUNK_VALUE_PTR(CHUNK_ID_REINFORCE_BLOCK_UNITS,	ReinforceUnits, script);

		case CHUNK_ID_BASE_CLASS:
			ok &= BlockingStatus::Load(script, reader);
			break;

		case CHUNK_ID_REINFORCE_BLOCK_DATA:
			while (reader->Open_Micro_Chunk())
			{
				switch (reader->Cur_Micro_Chunk_ID())
				{
					READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_REINFORCE_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
					READ_MICRO_CHUNK_VECTOR3(CHUNK_ID_REINFORCE_BLOCK_POSITION, TargetPosition);
					READ_MICRO_CHUNK(CHUNK_ID_REINFORCE_BLOCK_INCOMING_SHIPS, IncomingShipCount);
					READ_MICRO_CHUNK(CHUNK_ID_REINFORCE_BLOCK_TIME_OUT_FRAME, TimeOutFrame);
					READ_MICRO_CHUNK(CHUNK_ID_REINFORCE_BLOCK_PLAYER_ID, PlayerID);
					READ_MICRO_CHUNK(CHUNK_ID_REINFORCE_BLOCK_OBEY_RULES, ObeyReinforceRules);
					READ_MICRO_CHUNK(CHUNK_ID_REINFORCE_BLOCK_RADIUS, Radius);

				case CHUNK_ID_REINFORCE_BLOCK_ATTEMPTS_TO_EXPANSION:
				case CHUNK_ID_REINFORCE_BLOCK_BOX_EXTENT:
					//Obsolete chunks.
					break;

				default:
					assert(false);
					ok = false;
					break;
				}

				reader->Close_Micro_Chunk();
			}
			break;

		default:
			assert(false);
			ok = false;
			break;
		}

		reader->Close_Chunk();
	}

	Script = script;

	return ok;
}