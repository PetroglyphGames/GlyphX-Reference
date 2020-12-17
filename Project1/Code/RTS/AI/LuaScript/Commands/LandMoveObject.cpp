// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LandMoveObject.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LandMoveObject.cpp $
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


#include "LandMoveObject.h"
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
#include "TeamBehavior.h"
#include "AI/Pathfinding/LandZonePathFinder.h"
#include "AI/Pathfinding/LandZonePath.h"
#include "AI/AILog.h"
#include "AI/AIHintZone.h"
#include "AI/Movement/Formation.h"
#include "TargetingInterface.h"

/**
 * Keeps track of the progress of a Movement Command.
 * @since 4/29/2004 6:33:23 PM -- BMH
 */
LandMovementBlockStatus::LandMovementBlockStatus() :
	IsFinished(false),
	ThreatTolerance(BIG_FLOAT),
	DoZonePath(false),
	DoAdvance(false),
	DestinationPosition(VECTOR3_NONE),
	MoveFlags(0),
	Attack(false),
	Stragglers(0),
	MoveList(0)
{
}

LandMovementBlockStatus::~LandMovementBlockStatus()
{
	delete Stragglers;
	delete MoveList;
}

void LandMovementBlockStatus::Init(LuaUserVar *command, GameObjectClass *dest, float threat_tolerance, bool do_zone_path, bool attack, int move_flags)
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
	DestinationObject = dest;
	ThreatTolerance = threat_tolerance;
	DoZonePath = do_zone_path;
	MoveFlags = move_flags;
	Attack = attack;

	SignalDispatcherClass::Get().Add_Listener( DestinationObject, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );
	SignalDispatcherClass::Get().Add_Listener( DestinationObject, this, PG_SIGNAL_OBJECT_DELETE_PENDING );

	if (DoZonePath)
	{
		AIPerceptionSystemClass *perception_system = tf->Get_Plan()->Get_Planning_System()->Get_Manager()->Get_Perception_System();
		StartZone = perception_system->Find_Hint_Zone_At_Location(tf->Get_Position());
		EndZone = perception_system->Find_Hint_Zone_At_Location(DestinationObject->Get_Position());
	}

	if (!DoZonePath || !StartZone || !EndZone || StartZone == EndZone)
	{
		Execute_Move(DestinationObject, true);
	}
	else
	{
		Do_Zone_Advance(true);
	}
}

/**
 * Execute a ship move to position order.
 * 
 * @param pos    Position to move to.
 * @since 9/14/2004 2:11:56 PM -- BMH
 */
void LandMovementBlockStatus::Execute_Move(const Vector3 &pos, bool init)
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
	FAIL_IF(!MoveList) { IsFinished = true; return; }

	MultiLinkedListIterator<GameObjectClass> it(MoveList);
	for (; !it.Is_Done(); it.Next())
	{
		GameObjectClass *object = it.Current_Object();
		if (!object) continue;

		AIMESSAGE( ("Issuing Move order for %s, ID:%d.", object->Get_Type()->Get_Name()->c_str(), object->Get_ID()) );

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
		// set up the coordinator target/DestinationObject
		FormationDestinationStruct destination;
		destination.Type = FDT_POSITION;
		destination.Position = pos;
		destination.MovementType.Set_Type(MoveFlags);
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

/**
 * Execute a ship attack target order.
 * 
 * @param target Target to attack.
 * @since 9/14/2004 2:12:16 PM -- BMH
 */
void LandMovementBlockStatus::Execute_Move(GameObjectClass *target, bool init)
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
	FAIL_IF(!MoveList) { IsFinished = true; return; }

	MultiLinkedListIterator<GameObjectClass> it(MoveList);
	for (; !it.Is_Done(); it.Next())
	{
		GameObjectClass *object = it.Current_Object();
		if (!object) continue;

		bool attacking = false;
		bool update_movement = true;
		if (Attack && (object->Behaves_Like(BEHAVIOR_TEAM) || object->Behaves_Like(BEHAVIOR_TARGETING))) {
			update_movement = object->Attack_Object( target, 0, true ); // True = direct player command
			attacking = true;
		}

		if ( update_movement == true )
		{
			// if this object can be coordinated, stick it in the coordinator
			if ( coordinator && system->Can_Object_Be_In_Formations( object ) )
			{
				Stragglers->Add(object);
				system->Remove_Object_From_Coordinators( object );
				coordinator->Add_Object( object );
			}
		}
	}

	if ( coordinator && coordinator->Get_Member_Count() )
	{
		// set up the coordinator target/DestinationObject
		FormationDestinationStruct destination;
		destination.Type = FDT_OBJECT;
		destination.Target = target;
		destination.Hardpoint = 0;
		destination.MovementType.Set_Type(MoveFlags);
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

bool LandMovementBlockStatus::Internal_Is_Finished(void)
{
	if (DoAdvance)
	{
		Do_Zone_Advance(false);
	}

	return IsFinished;
}

LuaTable* LandMovementBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	TaskForceClass *tf = (TaskForceClass *)Get_Command();

	return Return_Variable(new LuaBool(Internal_Is_Finished() || tf->Get_Force_Count() == 0));
}

enum {
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_MOVEMENT_BLOCK_DATA,
	CHUNK_ID_MOVEMENT_BLOCK_DOADVANCE,
	CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED,
	CHUNK_ID_MOVEMENT_BLOCK_DESTINATION,
	CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID,
	CHUNK_ID_MOVEMENT_BLOCK_THREAT_TOLERANCE,
	CHUNK_ID_MOVEMENT_BLOCK_ATTACK,
	CHUNK_ID_MOVEMENT_BLOCK_MOVE_FLAGS,
	CHUNK_ID_MOVEMENT_BLOCK_DO_ZONE_PATH,
	CHUNK_ID_MOVEMENT_BLOCK_DESTINATION_POSITION,
	CHUNK_ID_MOVEMENT_BLOCK_START_ZONE,
	CHUNK_ID_MOVEMENT_BLOCK_END_ZONE,
	CHUNK_ID_MOVEMENT_BLOCK_STRAGGLERS,
	CHUNK_ID_MOVEMENT_BLOCK_MOVE_LIST,
};

bool LandMovementBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();
	
	ok &= writer->Begin_Chunk(CHUNK_ID_MOVEMENT_BLOCK_DATA);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED, IsFinished);
		WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
		WRITE_MICRO_CHUNK_SMART_PTR(CHUNK_ID_MOVEMENT_BLOCK_DESTINATION, DestinationObject);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_THREAT_TOLERANCE, ThreatTolerance);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_DOADVANCE, DoAdvance);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_ATTACK, Attack);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_MOVE_FLAGS, MoveFlags);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_DO_ZONE_PATH, DoZonePath);
		WRITE_MICRO_CHUNK_VECTOR3(CHUNK_ID_MOVEMENT_BLOCK_DESTINATION_POSITION, DestinationPosition);
		WRITE_MICRO_CHUNK_SMART_PTR(CHUNK_ID_MOVEMENT_BLOCK_START_ZONE, StartZone);
		WRITE_MICRO_CHUNK_SMART_PTR(CHUNK_ID_MOVEMENT_BLOCK_END_ZONE, EndZone);
	ok &= writer->End_Chunk();

	WRITE_CHUNK_MLL_PTR_LIST(CHUNK_ID_MOVEMENT_BLOCK_STRAGGLERS, Stragglers);
	WRITE_CHUNK_MLL_PTR_LIST(CHUNK_ID_MOVEMENT_BLOCK_MOVE_LIST, MoveList);

	return (ok);
}

bool LandMovementBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
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
							READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
							READ_MICRO_CHUNK_SMART_PTR(CHUNK_ID_MOVEMENT_BLOCK_DESTINATION, DestinationObject);
							READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_THREAT_TOLERANCE, ThreatTolerance);
							READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_DOADVANCE, DoAdvance);
							READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_ATTACK, Attack);
							READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_MOVE_FLAGS, MoveFlags);
							READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_DO_ZONE_PATH, DoZonePath);
							READ_MICRO_CHUNK_VECTOR3(CHUNK_ID_MOVEMENT_BLOCK_DESTINATION_POSITION, DestinationPosition);
							READ_MICRO_CHUNK_SMART_PTR(CHUNK_ID_MOVEMENT_BLOCK_START_ZONE, StartZone);
							READ_MICRO_CHUNK_SMART_PTR(CHUNK_ID_MOVEMENT_BLOCK_END_ZONE, EndZone);
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

LuaTable* LandMovementBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return NULL;
}


/// signal dispatcher interface
void LandMovementBlockStatus::Receive_Signal(SignalGeneratorClass *gen, PGSignalType type, SignalDataClass *)
{
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
	
				if (object->Is_Team_Member())
				{
					bool all_finished = true;
					TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(object->Get_Parent_Container_Object()->Get_Behavior(BEHAVIOR_TEAM));
					for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
					{
						if (Stragglers->Is_In_List(team->Get_Team_Member_By_Index(i)))
						{
							all_finished = false;
						}
					}

					if (all_finished)
					{
						SmartPtr<LuaTable> event_params = Return_Variable(GameObjectWrapper::Create(object->Get_Parent_Container_Object(), tf->Get_Plan()->Get_Script()));
						tf->Signal_Event(PLAN_EVENT_UNIT_MOVE_FINISHED, event_params);
						Free_Lua_Table(event_params);
					}
				}
				else
				{
					SmartPtr<LuaTable> event_params = Return_Variable(GameObjectWrapper::Create(object, tf->Get_Plan()->Get_Script()));
					tf->Signal_Event(PLAN_EVENT_UNIT_MOVE_FINISHED, event_params);
					Free_Lua_Table(event_params);
				}

				bool is_guard = ((MoveFlags & UNIT_AI_MOVE_TYPE_ESCORT) != 0);

				if (Stragglers->Is_Empty())
				{
					if (StartZone && EndZone)
					{
						DoAdvance = true;
					}
					else if (!DestinationObject)
					{
						IsFinished = true;
					}
					else if (!is_guard &&
								(!DestinationObject->Is_Enemy(tf->Get_Plan()->Get_Player()) ||
								!TargetingInterfaceClass::Is_Suitable_Target(DestinationObject)))
					{
						IsFinished = true;
					}
					else if (!DestinationObject->Get_Type()->Get_Last_State_Visible_Under_FOW() && 
								!DestinationObject->Get_Type()->Get_Initial_State_Visible_Under_FOW())
					{
						if (GameModeManager.Get_Active_Mode()->Is_Fogged(tf->Get_Plan()->Get_Player()->Get_ID(), DestinationObject, true))
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

				TaskForceClass *tf = static_cast<TaskForceClass*>(Get_Command());
				bool is_guard = ((MoveFlags & UNIT_AI_MOVE_TYPE_ESCORT) != 0);

				if (obj == DestinationObject)
				{
					if (type == PG_SIGNAL_OBJECT_OWNER_CHANGED)
					{
						bool is_now_enemy = DestinationObject->Is_Enemy(tf->Get_Plan()->Get_Player());
						if (Attack && !is_now_enemy)
						{
							IsFinished = true;
							DestinationObject = 0;
						}

						if (is_guard && is_now_enemy)
						{
							IsFinished = true;
							DestinationObject = 0;
						}
					}
					else
					{
						IsFinished = true;
						DestinationObject = 0;
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
						if (StartZone && EndZone)
						{
							DoAdvance = true;
						}
						else if (!DestinationObject)
						{
							IsFinished = true;
						}
						else if (!is_guard &&
									(!DestinationObject->Is_Enemy(tf->Get_Plan()->Get_Player()) ||
									!TargetingInterfaceClass::Is_Suitable_Target(DestinationObject)))
						{
							IsFinished = true;
						}
						else if (!DestinationObject->Get_Type()->Get_Last_State_Visible_Under_FOW() && 
									!DestinationObject->Get_Type()->Get_Initial_State_Visible_Under_FOW())
						{
							TaskForceClass *tf = static_cast<TaskForceClass*>(Get_Command());
							if (GameModeManager.Get_Active_Mode()->Is_Fogged(tf->Get_Plan()->Get_Player()->Get_ID(), DestinationObject, true))
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
}

void LandMovementBlockStatus::Init(LuaUserVar *command, const Vector3 &pos, float threat_tolerance, bool do_zone_path, int move_flags)
{
	TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(command);
	FAIL_IF(!tf) 
	{
		IsFinished = true;
		return;
	}

	Set_Command(command);

	ThreatTolerance = threat_tolerance;
	DoZonePath = do_zone_path;
	DestinationPosition = pos;
	MoveFlags = move_flags;
	Attack = false;

	if (DoZonePath)
	{
		AIPerceptionSystemClass *perception_system = tf->Get_Plan()->Get_Planning_System()->Get_Manager()->Get_Perception_System();
		StartZone = perception_system->Find_Hint_Zone_At_Location(tf->Get_Position());
		EndZone = perception_system->Find_Hint_Zone_At_Location(DestinationPosition);
	}

	if (!DoZonePath || !StartZone || !EndZone || StartZone == EndZone)
	{
		Execute_Move(DestinationPosition, true);
	}
	else
	{
		Do_Zone_Advance(true);
	}
}

void LandMovementBlockStatus::Do_Zone_Advance(bool init)
{
	DoAdvance = false;
		
	if (StartZone == EndZone)
	{
		StartZone = 0;
		EndZone = 0;

		if (DestinationObject)
		{
			Execute_Move(DestinationObject, false);
		}
		else
		{
			Execute_Move(DestinationPosition, false);
		}
		return;
	}

	TaskForceClass *tf = static_cast<TaskForceClass*>(Get_Command());

	LandZonePathFindCallPropertiesStruct config;
	config.Player = tf->Get_Plan()->Get_Player();
	config.StartZone = StartZone->Get_Hint_Zone();
	config.EndZone = EndZone->Get_Hint_Zone();
	config.ThreatThreshold = 2.0f * ThreatTolerance;
	config.ThreatWeight = 1.0f;
	config.PassabilityMask = tf->Get_Passability_Mask();;
	LandZonePathClass path;
	if (!LandZonePathFinderClass::Get().Find_Path(config, path))
	{
		AIERROR( ("Unable to find a path for taskforce %s.", static_cast<TaskForceClass*>(Get_Command())->Get_Definition()->Get_Name().c_str()) );
		IsFinished = true;
		return;
	}

	if (path.Get_Path_Node_Count() == 2)
	{
		StartZone = 0;
		EndZone = 0;
		if (DestinationObject)
		{
			Execute_Move(DestinationObject, init);
		}
		else
		{
			Execute_Move(DestinationPosition, init);
		}
	}
	else
	{
		Execute_Move(path.Get_Path_Node(1)->Get_Center_Point(), init);
		StartZone = StartZone->Get_Perception_System()->Find_Hint_Zone(const_cast<AIHintZoneClass*>(path.Get_Path_Node(1)));
	}
}

void LandMovementBlockStatus::Init_Move_List()
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

		if (object->Is_Movement_Locked())
			continue;

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
		else if (object->Behaves_Like(BEHAVIOR_TEAM))
		{
			TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(object->Get_Behavior(BEHAVIOR_TEAM));
			FAIL_IF(!team) { continue; }
			for (int j = 0; j < team->Get_Team_Member_Count(); ++j)
			{
				GameObjectClass *team_member = team->Get_Team_Member_By_Index(j);
				FAIL_IF(!team_member) { continue; }

				SignalDispatcherClass::Get().Send_Signal( team_member, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED, NULL );

				SignalDispatcherClass::Get().Add_Listener( team_member, this, PG_SIGNAL_OBJECT_MOVEMENT_BEGIN );
				SignalDispatcherClass::Get().Add_Listener( team_member, this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED );
				SignalDispatcherClass::Get().Add_Listener( team_member, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED );
				SignalDispatcherClass::Get().Add_Listener( team_member, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
				SignalDispatcherClass::Get().Add_Listener( team_member, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );

				MoveList->Add(team_member);
			}
		}
	}
}

LUA_IMPLEMENT_FACTORY(LUA_CHUNK_LAND_MOVEMENT_BLOCK, LandMovementBlockStatus);
PG_IMPLEMENT_RTTI(LandMovementBlockStatus, BlockingStatus);



