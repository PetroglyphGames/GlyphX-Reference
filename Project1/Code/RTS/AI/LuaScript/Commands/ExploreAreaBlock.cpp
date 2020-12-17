// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ExploreAreaBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ExploreAreaBlock.cpp $
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

#include "ExploreAreaBlock.h"
#include "AI/Planning/TaskForce.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "TeamBehavior.h"
#include "AI/Movement/MovementCoordinatorSystem.h"
#include "AI/Movement/MovementCoordinator.h"
#include "AI/AITargetLocation.h"
#include "AI/Perception/AIPerceptionSystem.h"
#include "AI/Perception/TacticalPerceptionGrid.h"
#include "AI/AIHintZone.h"

PG_IMPLEMENT_RTTI(ExploreAreaBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_EXPLORE_AREA_BLOCK, ExploreAreaBlockStatus);

/**************************************************************************************************
* ExploreAreaBlockStatus::ExploreAreaBlockStatus -- 
*
* In:			
*
* Out:		
*
* History: 9/22/2005 1:36PM JSY
**************************************************************************************************/
ExploreAreaBlockStatus::ExploreAreaBlockStatus() :
	MoveList(NULL),
	Target(NULL),
	NeedsNewDestination(false),
	IsFinished(false),
	StartTime(0)
{
}

/**************************************************************************************************
* ExploreAreaBlockStatus::~ExploreAreaBlockStatus -- 
*
* In:			
*
* Out:		
*
* History: 9/22/2005 1:38PM JSY
**************************************************************************************************/
ExploreAreaBlockStatus::~ExploreAreaBlockStatus()
{
	delete MoveList;
	MoveList = NULL;
}

/**************************************************************************************************
* ExploreAreaBlockStatus::Init -- Set up this blocking object 
*
* In:			unit or taskforce to do the exploring
*				target area to explore.
*
* Out:		
*
* History: 9/22/2005 1:38PM JSY
**************************************************************************************************/
void ExploreAreaBlockStatus::Init(LuaUserVar *command, const AITargetLocationClass *target)
{
	Set_Command(command);
	Target = target;
	NeedsNewDestination = false;
	IsFinished = false;
	StartTime = GameModeManager.Get_Frame_Timer();

	Init_Move_List();
	Move_To_New_Destination();
}

/**************************************************************************************************
* ExploreAreaBlockStatus::Is_Finished -- Script function to determine whether this order is finished
*	and update it if not.
*
* In:			
*
* Out:		
*
* History: 9/22/2005 1:39PM JSY
**************************************************************************************************/
LuaTable *ExploreAreaBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	//May need a new move order
	if (NeedsNewDestination)
	{
		Move_To_New_Destination();
	}

	return Return_Variable(new LuaBool(IsFinished));
}

/**************************************************************************************************
* ExploreAreaBlockStatus::Init_Move_List -- Gather up the set of objects that this command is going to
*	move around.
*
* In:			
*
* Out:		
*
* History: 9/22/2005 1:39PM JSY
**************************************************************************************************/
void ExploreAreaBlockStatus::Init_Move_List()
{
	assert(!MoveList);
	MoveList = new MultiLinkedListClass<GameObjectClass>();

	TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(Get_Command());
	GameObjectWrapper *object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(Get_Command());

	if (tf)
	{
		for (int i = 0; i < (int)tf->Get_Member_Count(); i++) 
		{
			GameObjectWrapper *wrap = tf->Get_Member(i);
			if (!wrap) continue;

			Add_Object(wrap->Get_Object());
		}
	}
	else if (object_wrapper)
	{
		Add_Object(object_wrapper->Get_Object());
	}
}

/**************************************************************************************************
* ExploreAreaBlockStatus::Add_Object -- Add a single object to the move list.
*
* In:			
*
* Out:		
*
* History: 9/22/2005 1:40PM JSY
**************************************************************************************************/
void ExploreAreaBlockStatus::Add_Object(GameObjectClass *object)
{
	if (!object) return;

	if (object->Is_Movement_Locked())
		return;

	MovementCoordinatorSystemClass *system = GameModeManager.Get_Active_Mode()->Get_Movement_Coordinator_System();
	FAIL_IF(!system) { return; }

	if (system->Can_Object_Be_In_Formations(object))
	{
		SignalDispatcherClass::Get().Send_Signal( object, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL );

		SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED );
		SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED );
		SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
		SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );

		MoveList->Add(object);
	}
	else if (object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return; }
		for (int j = 0; j < team->Get_Team_Member_Count(); ++j)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(j);
			FAIL_IF(!team_member) { continue; }

			SignalDispatcherClass::Get().Send_Signal( team_member, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL );

			SignalDispatcherClass::Get().Add_Listener( team_member, this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED );
			SignalDispatcherClass::Get().Add_Listener( team_member, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED );
			SignalDispatcherClass::Get().Add_Listener( team_member, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
			SignalDispatcherClass::Get().Add_Listener( team_member, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );

			MoveList->Add(team_member);
		}
	}
}

/**************************************************************************************************
* ExploreAreaBlockStatus::Move_To_New_Destination -- Find an unexplored location and start heading there
*
* In:			
*
* Out:		
*
* History: 9/22/2005 1:40PM JSY
**************************************************************************************************/
void ExploreAreaBlockStatus::Move_To_New_Destination()
{
	if (!MoveList || MoveList->Is_Empty())
	{
		IsFinished = true;
		return;
	}

	PlayerClass *for_player = MoveList->Get_Head()->Get_Owner_Player();
	FAIL_IF(!for_player) { IsFinished = true; return; }
	FAIL_IF(!Target) { IsFinished = true; return; }

	FormationDestinationStruct destination;
	destination.Type = FDT_POSITION;

	TacticalPerceptionGridClass *perception_grid = AIPerceptionSystemClass::Get_Perception_Grid();
	FAIL_IF(!perception_grid) { IsFinished = true; return; }
	AIHintZoneClass *hint_zone = Target->Get_Hint_Zone();
	if (hint_zone)
	{
		destination.Position = perception_grid->Get_Most_Fogged_Position(hint_zone->Get_Threat_Cells(), StartTime, for_player);
	}
	else
	{
		destination.Position = perception_grid->Get_Most_Fogged_Position(Target->Get_Target_Region(), StartTime, for_player);
	}

	if (destination.Position == VECTOR3_INVALID)
	{
		IsFinished = true;
		return;
	}

	MultiLinkedListIterator<GameObjectClass> it(MoveList);

	MovementCoordinatorSystemClass *system = GameModeManager.Get_Active_Mode()->Get_Movement_Coordinator_System();
	FAIL_IF(!system) { IsFinished = true; return; }
	MovementCoordinatorClass *coordinator = system->Create();
	FAIL_IF(!coordinator) { IsFinished = true; return; }

	for ( ; !it.Is_Done(); it.Next())
	{
		system->Remove_Object_From_Coordinators(it.Current_Object());
		coordinator->Add_Object(it.Current_Object());	
	}

	coordinator->Set_Destination(destination);
	NeedsNewDestination = false;
}

/**************************************************************************************************
* ExploreAreaBlockStatus::Receive_Signal -- Handle notifications concerning the moving objects
*
* In:			
*
* Out:		
*
* History: 9/22/2005 1:40PM JSY
**************************************************************************************************/
void ExploreAreaBlockStatus::Receive_Signal(SignalGeneratorClass *gen, PGSignalType signal_type, SignalDataClass *)
{
	switch (signal_type)
	{
		case PG_SIGNAL_OBJECT_MOVEMENT_CANCELED:
		case PG_SIGNAL_OBJECT_OWNER_CHANGED:
		case PG_SIGNAL_OBJECT_DELETE_PENDING:
			//This unit is lost to us
			SignalDispatcherClass::Get().Remove_Listener( gen, this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED );
			SignalDispatcherClass::Get().Remove_Listener( gen, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED );
			SignalDispatcherClass::Get().Remove_Listener( gen, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
			SignalDispatcherClass::Get().Remove_Listener( gen, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );
			if (MoveList)
			{
				MoveList->Remove(static_cast<GameObjectClass*>(gen));
			}
			break;

		case PG_SIGNAL_OBJECT_MOVEMENT_FINISHED:
			//Someone reached the destination.  Next update we'll start heading somewhere new.
			NeedsNewDestination = true;
			break;

		default:
			assert(false);
			break;
	}
}

enum
{
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_EXPLORE_BLOCK_DATA,
	CHUNK_ID_EXPLORE_BLOCK_LISTENER_BASE_ID,
	CHUNK_ID_EXPLORE_BLOCK_TARGET,
	CHUNK_ID_EXPLORE_BLOCK_NEED_DESTINATION,
	CHUNK_ID_EXPLORE_BLOCK_FINISHED,
	CHUNK_ID_EXPLORE_BLOCK_START_TIME,
	CHUNK_ID_EXPLORE_BLOCK_MOVE_LIST,
};

/**************************************************************************************************
* ExploreAreaBlockStatus::Save -- Write this blocking object to file.
*
* In:			
*
* Out:		
*
* History: 9/22/2005 1:40PM JSY
**************************************************************************************************/
bool ExploreAreaBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(CHUNK_ID_EXPLORE_BLOCK_DATA);
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_EXPLORE_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
	WRITE_MICRO_CHUNK_OBJECT_PTR(CHUNK_ID_EXPLORE_BLOCK_TARGET, Target);
	WRITE_MICRO_CHUNK(CHUNK_ID_EXPLORE_BLOCK_NEED_DESTINATION, NeedsNewDestination);
	WRITE_MICRO_CHUNK(CHUNK_ID_EXPLORE_BLOCK_FINISHED, IsFinished);
	WRITE_MICRO_CHUNK(CHUNK_ID_EXPLORE_BLOCK_START_TIME, StartTime);
	ok &= writer->End_Chunk();

	WRITE_CHUNK_MLL_PTR_LIST(CHUNK_ID_EXPLORE_BLOCK_MOVE_LIST, MoveList);

	return ok;
}

/**************************************************************************************************
* ExploreAreaBlockStatus::Load -- Read this blocking object from file.
*
* In:			
*
* Out:		
*
* History: 9/22/2005 1:40PM JSY
**************************************************************************************************/
bool ExploreAreaBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;
	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
		case CHUNK_ID_BASE_CLASS:
			ok &= BlockingStatus::Load(script, reader);
			break;

		case CHUNK_ID_EXPLORE_BLOCK_DATA:
			while (reader->Open_Micro_Chunk())
			{
				switch (reader->Cur_Micro_Chunk_ID())
				{
					READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_EXPLORE_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
					READ_MICRO_CHUNK_OBJECT_PTR(CHUNK_ID_EXPLORE_BLOCK_TARGET, Target);
					READ_MICRO_CHUNK(CHUNK_ID_EXPLORE_BLOCK_NEED_DESTINATION, NeedsNewDestination);
					READ_MICRO_CHUNK(CHUNK_ID_EXPLORE_BLOCK_FINISHED, IsFinished);
					READ_MICRO_CHUNK(CHUNK_ID_EXPLORE_BLOCK_START_TIME, StartTime);

				default:
					ok = false;
					assert(false);
					break;
				}

				reader->Close_Micro_Chunk();
			}
			break;
			
			READ_CHUNK_MLL_PTR_LIST(CHUNK_ID_EXPLORE_BLOCK_MOVE_LIST, MoveList);

		default:
			ok = false;
			assert(false);
			break;
		}

		reader->Close_Chunk();
	}

	return ok;
}