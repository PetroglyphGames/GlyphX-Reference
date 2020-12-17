// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceMoveObject.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceMoveObject.cpp $
//
//    Original Author: Brian Hayes
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


#include "SpaceMoveObject.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "GameObject.h"
#include "PGSignal/SignalDispatcher.h"
#include "AI/Planning/TaskForce.h"
#include "AI/Planning/PlanBehavior.h"
#include "AI/Planning/PlanEventManager.h"
#include "AI/Planning/AIPlanningSystem.h"
#include "AI/TacticalAIManager.h"
#include "MoveObjectToObjectEvent.h"
#include "OutgoingEventQueue.h"
#include "AI/PathFinding/GalacticPath.h"
#include "AI/PathFinding/GalacticPathFinder.h"
#include "Faction.h"
#include "AI/Goal/AIGoalReachabilityType.h"
#include "AI/Goal/InstantiatedGoal.h"
#include "AI/Goal/AIGoalProposalFunction.h"
#include "AI/Goal/AIGoalType.h"
#include "AI/Movement/MovementCoordinatorSystem.h"
#include "AI/Movement/MovementCoordinator.h"
#include "AI/AITargetLocation.h"
#include "GameObjectManager.h"
#include "HardPoint.h"
#include "TargetingInterface.h"
#include "AI/Movement/Formation.h"

/**
 * Keeps track of the progress of a Movement Command.
 * @since 4/29/2004 6:33:23 PM -- BMH
 */
SpaceMovementBlockStatus::SpaceMovementBlockStatus() :
	IsFinished(false),
	Repeat(false),
	Attack(false),
	HardPoint(0),
	ThreatTolerance(BIG_FLOAT),
	MoveFlags(0),
	Stragglers(0),
	MoveList(0),
	Avoidance(SCT_ALL)
{
}

SpaceMovementBlockStatus::~SpaceMovementBlockStatus()
{
	delete Stragglers;
	delete MoveList;
}

/**
 * Init the BlockingStatus object.
 * 
 * @param command Command that spawned the blocking status.
 * @param fleet
 * @since 4/29/2004 6:34:04 PM -- BMH
 */
void SpaceMovementBlockStatus::Init(LuaUserVar *command, 
												AITargetLocationWrapper *dest, 
												bool attack, 
												bool repeat, 
												HardPointType hard_point_type,
												float threat_tolerance,
												int move_flags,
												SpaceCollisionType avoidance)
{
	TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(command);
	FAIL_IF(!tf) {
		IsFinished = true;
		return;
	}

	Set_Command(command);
	Destination = dest->Get_Object()->Get_Target_Game_Object();
	Repeat = repeat;
	ThreatTolerance = threat_tolerance;
	MoveFlags = move_flags;
	Avoidance = avoidance;

	if (Destination) 
	{
		Attack = attack;
		SignalDispatcherClass::Get().Add_Listener( Destination, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );
		SignalDispatcherClass::Get().Add_Listener( Destination, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
		Execute_Move(Destination, true, hard_point_type);
	}
	else
	{
		Execute_Move(dest->Get_Object()->Get_Target_Position(), true);
	}
}

void SpaceMovementBlockStatus::Init(LuaUserVar *command, 
												GameObjectClass *dest, 
												bool attack, bool repeat, 
												HardPointType hard_point_type,
												float threat_tolerance,
												int move_flags,
												SpaceCollisionType avoidance)
{
	FAIL_IF(!dest)
	{
		IsFinished = true;
		return;
	}
		
	TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(command);
	FAIL_IF(!tf) {
		IsFinished = true;
		return;
	}

	Set_Command(command);
	Destination = dest;
	Attack = attack;
	Repeat = repeat;
	ThreatTolerance = threat_tolerance;
	MoveFlags = move_flags;
	Avoidance = avoidance;

	SignalDispatcherClass::Get().Add_Listener( Destination, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );
	SignalDispatcherClass::Get().Add_Listener( Destination, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
	Execute_Move(Destination, true, hard_point_type);
}


/**
 * Execute a ship move to position order.
 * 
 * @param pos    Position to move to.
 * @since 9/14/2004 2:11:56 PM -- BMH
 */
void SpaceMovementBlockStatus::Execute_Move(const Vector3 &pos, bool init)
{
	MovementCoordinatorSystemClass *system = GameModeManager.Get_Active_Mode()->Get_Movement_Coordinator_System();
	MovementCoordinatorClass *coordinator = NULL;

	// make a new coordinator
	if ( system )
	{
		coordinator = system->Create();
	}

	if (init)
	{
		Init_Move_List();
	}

	FAIL_IF(!Stragglers) { IsFinished = true; return; }
	FAIL_IF(!MoveList) {IsFinished = true; return; }

	MultiLinkedListIterator<GameObjectClass> it(MoveList);
	for (; !it.Is_Done(); it.Next())
	{
		GameObjectClass *object = it.Current_Object();
		if (!object) continue;

		// if this object can be coordinated, stick it in the coordinator
		if ( coordinator && system->Can_Object_Be_In_Formations( object ) )
		{
			Stragglers->Add(object);
			system->Remove_Object_From_Coordinators( object );
			coordinator->Add_Object( object );
		}
	}

	if ( coordinator && coordinator->Get_Member_Count() )
	{
		// set up the coordinator target/destination
		FormationDestinationStruct destination;
		destination.Type = FDT_POSITION;
		destination.Position = pos;
		destination.MovementType.Set_Type(MoveFlags);
		destination.SpaceObstacleFilter = Avoidance;
		if (ThreatTolerance != BIG_FLOAT)
		{
			destination.ThreatTolerance = ThreatTolerance;
		}

		coordinator->Set_Destination( destination );
	}
	else 
	{
		if (coordinator) {
			system->Destroy(coordinator);
			coordinator = NULL;
		}
	}
}


bool SpaceMovementBlockStatus::Internal_Is_Finished(void)
{
	if (HardPoint && HardPoint->Is_Destroyed())
	{
		if (Repeat)
		{
			Execute_Move(Destination, false, HardPoint->Get_Type());
		}
		else
		{
			IsFinished = true;
		}
	}

	return IsFinished;
}

LuaTable* SpaceMovementBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	TaskForceClass *tf = (TaskForceClass *)Get_Command();

	return Return_Variable(new LuaBool(Internal_Is_Finished() || tf->Get_Force_Count() == 0));
}

enum {
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_MOVEMENT_BLOCK_DATA,
	CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED,
	CHUNK_ID_MOVEMENT_BLOCK_DESTINATION,
	CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID,
	CHUNK_ID_MOVEMENT_ATTACK,
	CHUNK_ID_MOVEMENT_BLOCK_REPEAT,
	CHUNK_ID_MOVEMENT_BLOCK_THREAT_TOLERANCE,
	CHUNK_ID_MOVEMENT_BLOCK_HARD_POINT,
	CHUNK_ID_MOVEMENT_BLOCK_MOVE_FLAGS,
	CHUNK_ID_MOVEMENT_BLOCK_STRAGGLERS,
	CHUNK_ID_MOVEMENT_BLOCK_MOVE_LIST,
	CHUNK_ID_MOVEMENT_BLOCK_AVOIDANCE,
};

bool SpaceMovementBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();
	
	ok &= writer->Begin_Chunk(CHUNK_ID_MOVEMENT_BLOCK_DATA);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED, IsFinished);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_ATTACK, Attack);
		WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
		WRITE_MICRO_CHUNK_SMART_PTR(CHUNK_ID_MOVEMENT_BLOCK_DESTINATION, Destination);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_REPEAT, Repeat);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_THREAT_TOLERANCE, ThreatTolerance);
		WRITE_MICRO_CHUNK_OBJECT_PTR(CHUNK_ID_MOVEMENT_BLOCK_HARD_POINT, HardPoint);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_MOVE_FLAGS, MoveFlags);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_AVOIDANCE, Avoidance);
	ok &= writer->End_Chunk();

	WRITE_CHUNK_MLL_PTR_LIST(CHUNK_ID_MOVEMENT_BLOCK_STRAGGLERS, Stragglers);
	WRITE_CHUNK_MLL_PTR_LIST(CHUNK_ID_MOVEMENT_BLOCK_MOVE_LIST, MoveList);

	return (ok);
}

bool SpaceMovementBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;
	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case CHUNK_ID_BASE_CLASS:
				ok &= BlockingStatus::Load(script, reader);
				break;

			case CHUNK_ID_MOVEMENT_BLOCK_DATA:
				while (reader->Open_Micro_Chunk()) {
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED, IsFinished);
						READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_ATTACK, Attack);
						READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
						READ_MICRO_CHUNK_SMART_PTR(CHUNK_ID_MOVEMENT_BLOCK_DESTINATION, Destination);
						READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_REPEAT, Repeat);
						READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_THREAT_TOLERANCE, ThreatTolerance);
						READ_MICRO_CHUNK_CONST_OBJECT_PTR(CHUNK_ID_MOVEMENT_BLOCK_HARD_POINT, HardPoint);
						READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_MOVE_FLAGS, MoveFlags);
						READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_AVOIDANCE, Avoidance);
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			READ_CHUNK_MLL_PTR_LIST(CHUNK_ID_MOVEMENT_BLOCK_STRAGGLERS, Stragglers);
			READ_CHUNK_MLL_PTR_LIST(CHUNK_ID_MOVEMENT_BLOCK_MOVE_LIST, MoveList);

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}
	return (ok);
}

LuaTable* SpaceMovementBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(ResultObject);
}


/// signal dispatcher interface
void SpaceMovementBlockStatus::Receive_Signal(SignalGeneratorClass *gen, PGSignalType type, SignalDataClass *)
{
	//Don't return early from this function!  Script calls may release all references to the object, so
	//we need to have the behavior wrapped in an Add_Ref/Release_Ref pair
	Add_Ref();

	GameObjectClass *obj = (GameObjectClass *)gen;

	switch (type) {
		case PG_SIGNAL_OBJECT_MOVEMENT_BEGIN:
			break;

		case PG_SIGNAL_OBJECT_MOVEMENT_FINISHED:
			{
				GameObjectClass *object = static_cast<GameObjectClass*>(gen);
				FAIL_IF(!Stragglers) { IsFinished = true; break; }
				Stragglers->Remove(object);

				TaskForceClass *tf = static_cast<TaskForceClass*>(Get_Command());
				SmartPtr<LuaTable> event_params = Return_Variable(GameObjectWrapper::Create(object, tf->Get_Plan()->Get_Script()));
				tf->Signal_Event(PLAN_EVENT_UNIT_MOVE_FINISHED, event_params);
				Free_Lua_Table(event_params);

				bool is_guard = ((MoveFlags & UNIT_AI_MOVE_TYPE_ESCORT) != 0);

				if (Stragglers->Is_Empty())
				{
					if (!Destination) 
					{
						IsFinished = true;
					}
					else if (!is_guard && 
								(!Destination->Is_Enemy(tf->Get_Plan()->Get_Player()) ||
								!TargetingInterfaceClass::Is_Suitable_Target(Destination)))
					{
						IsFinished = true;
					}
					else if (!Destination->Get_Type()->Get_Last_State_Visible_Under_FOW() && 
								!Destination->Get_Type()->Get_Initial_State_Visible_Under_FOW())
					{
						if (GameModeManager.Get_Active_Mode()->Is_Fogged(tf->Get_Plan()->Get_Player()->Get_ID(), Destination, true))
						{
							IsFinished = true;
						}
					}
				}

				break;
			}
		case PG_SIGNAL_OBJECT_MOVEMENT_CANCELED:
		case PG_SIGNAL_OBJECT_OWNER_CHANGED:
		case PG_SIGNAL_OBJECT_DELETE_PENDING:
			{
				SignalDispatcherClass::Get().Remove_Listener( obj, this, PG_SIGNAL_OBJECT_MOVEMENT_BEGIN );
				SignalDispatcherClass::Get().Remove_Listener( obj, this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED );
				SignalDispatcherClass::Get().Remove_Listener( obj, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED );
				SignalDispatcherClass::Get().Remove_Listener( obj, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
				SignalDispatcherClass::Get().Remove_Listener( obj, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );

				bool is_guard = ((MoveFlags & UNIT_AI_MOVE_TYPE_ESCORT) != 0);

				TaskForceClass *tf = (TaskForceClass *)Get_Command();
				if (obj == Destination)
				{
					if (type == PG_SIGNAL_OBJECT_OWNER_CHANGED)
					{
						bool is_now_enemy = Destination->Is_Enemy(tf->Get_Plan()->Get_Player());
						if (Attack && !is_now_enemy)
						{
							IsFinished = true;
							Destination = 0;
							HardPoint = 0;	
						}

						if (is_guard && is_now_enemy)
						{
							IsFinished = true;
							Destination = 0;
							HardPoint = 0;	
						}
					}
					else
					{
						IsFinished = true;
						Destination = 0;
						HardPoint = 0;
						tf->Get_Plan()->Broadcast_Signal(PLAN_EVENT_CURRENT_TARGET_DESTROYED, 0);
					}
				}
				else
				{
					GameObjectClass *object = static_cast<GameObjectClass*>(gen);

					FAIL_IF(!MoveList) { IsFinished = true; break; }
					FAIL_IF(!Stragglers) { IsFinished = true; break; }

					Stragglers->Remove(object);
					MoveList->Remove(object);

					if (Stragglers->Is_Empty())
					{
						if (!Destination)
						{
							IsFinished = true;
						}
						else if (!is_guard && 
									(!Destination->Is_Enemy(tf->Get_Plan()->Get_Player()) ||
									!TargetingInterfaceClass::Is_Suitable_Target(Destination)))
						{
							IsFinished = true;
						}
						else if (!Destination->Get_Type()->Get_Last_State_Visible_Under_FOW() && 
									!Destination->Get_Type()->Get_Initial_State_Visible_Under_FOW())
						{
							TaskForceClass *tf = (TaskForceClass *)Get_Command();
							if (GameModeManager.Get_Active_Mode()->Is_Fogged(tf->Get_Plan()->Get_Player()->Get_ID(), Destination, true))
							{
								IsFinished = true;
							}
						}
					}

					if (MoveList->Is_Empty())
					{
						IsFinished = true;
					}
				}
				break;
			}
		default:
			return;
	}

	Release_Ref();
}

const HardPointClass *SpaceMovementBlockStatus::Find_Best_Hard_Point(HardPointType hard_point_type)
{
	FAIL_IF(!Destination) { return 0; }

	float min_health = BIG_FLOAT;
	const HardPointClass *best_hard_point = 0;
	for (int i = 0; i < Destination->Get_Total_Hard_Points(); ++i)
	{
		const HardPointClass *hard_point = Destination->Get_Hard_Point_By_Index(i);
		if (hard_point->Get_Type() != hard_point_type ||
			!hard_point->Is_Targetable())
		{
			continue;
		}

		if (hard_point->Get_Health_Percent() < min_health)
		{
			best_hard_point = hard_point;
			min_health = hard_point->Get_Health_Percent();
		}
	}

	return best_hard_point;
}

void SpaceMovementBlockStatus::Init(LuaUserVar *command, const Vector3 &pos, float threat_tolerance, int move_flags, SpaceCollisionType avoidance)
{
	TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(command);
	FAIL_IF(!tf) {
		IsFinished = true;
		return;
	}

	Set_Command(command);

	ThreatTolerance = threat_tolerance;
	MoveFlags = move_flags;
	Avoidance = avoidance;

	Execute_Move(pos, true);
}

void SpaceMovementBlockStatus::Execute_Move(GameObjectClass *target, bool init, HardPointType hard_point_type)
{
	MovementCoordinatorSystemClass *system = GameModeManager.Get_Active_Mode()->Get_Movement_Coordinator_System();
	MovementCoordinatorClass *coordinator = NULL;

	// make a new coordinator
	if (system)
	{
		coordinator = system->Create();
	}

	if (Attack &&
		hard_point_type > HARD_POINT_INVALID &&
		hard_point_type < HARD_POINT_TOTAL)
	{
		HardPoint = Find_Best_Hard_Point(hard_point_type);
		if (!HardPoint)
		{
			Destination = 0;
			IsFinished = true;
			return;
		}
	}

	if (init)
	{
		Init_Move_List();
	}

	FAIL_IF(!Stragglers) { IsFinished = true; return; }
	FAIL_IF(!MoveList) {IsFinished = true; return; }

	MultiLinkedListIterator<GameObjectClass> it(MoveList);
	for (; !it.Is_Done(); it.Next())
	{
		GameObjectClass *object = it.Current_Object();
		if (!object) continue;

		bool update_movement = true;
		if (Attack && (object->Behaves_Like(BEHAVIOR_TEAM) || object->Behaves_Like(BEHAVIOR_TARGETING)) )
		{
			update_movement = object->Attack_Object( target, HardPoint ? HardPoint->Get_Name_CRC() : 0, true ); // True = direct player command
		}

		if ( update_movement == true )
		{
			// if this object can be coordinated, stick it in the coordinator
			if ( coordinator && system->Can_Object_Be_In_Formations( object ) )
			{
				system->Remove_Object_From_Coordinators( object );
				coordinator->Add_Object( object );
				Stragglers->Add(object);
			}
		}
		else
		{
			it.Remove_Current_Object();
		}
	}

	if ( coordinator && coordinator->Get_Member_Count() )
	{
		// set up the coordinator target/destination
		FormationDestinationStruct destination;
		destination.Type = FDT_OBJECT;
		destination.Target = target;
		destination.Hardpoint = Attack ? HardPoint : NULL;
		destination.MovementType.Set_Type(MoveFlags);
		destination.SpaceObstacleFilter = Avoidance;
		if (ThreatTolerance != BIG_FLOAT)
		{
			destination.ThreatTolerance = ThreatTolerance;
		}

		coordinator->Set_Destination( destination );
		coordinator = NULL;
	} else if (coordinator) {
		system->Destroy(coordinator);
		coordinator = NULL;
	}
}

void SpaceMovementBlockStatus::Init_Move_List()
{
	assert(!Stragglers);
	assert(!MoveList);
	MovementCoordinatorSystemClass *system = GameModeManager.Get_Active_Mode()->Get_Movement_Coordinator_System();
	FAIL_IF(!system) { return; }

	Stragglers = new MultiLinkedListClass<GameObjectClass>();
	MoveList = new MultiLinkedListClass<GameObjectClass>();

	TaskForceClass *tf = static_cast<TaskForceClass*>(Get_Command());

	for (int i = 0; i < (int)tf->Get_Member_Count(); i++) 
	{
		GameObjectWrapper *wrap = tf->Get_Member(i);
		if (!wrap) continue;

		GameObjectClass *object = wrap->Get_Object();
		if (!object) continue;

		if (object->Get_Original_Object_Type()->Is_Special_Weapon_In_Space())
			continue;

		if (object->Is_Movement_Locked())
			continue;

		// if this object can be coordinated, stick it in the coordinator
		if (system->Can_Object_Be_In_Formations(object))
		{
			SignalDispatcherClass::Get().Send_Signal( object, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL );

			SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_MOVEMENT_BEGIN );
			SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED );
			SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED );
			SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
			SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );

			MoveList->Add(object);
		}
	}
}

LUA_IMPLEMENT_FACTORY(LUA_CHUNK_SPACE_MOVEMENT_BLOCK, SpaceMovementBlockStatus);
PG_IMPLEMENT_RTTI(SpaceMovementBlockStatus, BlockingStatus);



