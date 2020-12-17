// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceAmbushBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceAmbushBlock.cpp $
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


#include "SpaceAmbushBlock.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "GameObject.h"
#include "PGSignal/SignalDispatcher.h"
#include "AI/Planning/TaskForce.h"
#include "AI/Planning/PlanBehavior.h"
#include "AI/Movement/MovementCoordinatorSystem.h"
#include "AI/Movement/MovementCoordinator.h"
#include "GameConstants.h"
#include "AI/Perception/AIPerceptionSystem.h"
#include "AI/Perception/TacticalPerceptionGrid.h"
#include "AI/Planning/TaskForceDefinition.h"
#include "AI/AILog.h"
#include "AI/Planning/PlanEventManager.h"

PG_IMPLEMENT_RTTI(SpaceAmbushBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_SPACE_AMBUSH_BLOCK, SpaceAmbushBlockStatus);
static const int AMBUSH_UPDATE_INTERVAL_FRAMES = 150;


/**************************************************************************************************
* SpaceAmbushBlockStatus::SpaceAmbushBlockStatus -- constructor
*
* In:			
*
* Out:		
*
* History: 2/25/2005 11:30AM JSY
**************************************************************************************************/
SpaceAmbushBlockStatus::SpaceAmbushBlockStatus() :
	IsFinished(false),
	Target(0),
	OffsetDirection(-1),
	OffsetDistance(0.0f),
	ThreatTolerance(BIG_FLOAT),
	LastMoveUpdateFrame(0),
	Stragglers(0),
	AmbushReady(false),
	MoveList(0)
{
}

SpaceAmbushBlockStatus::~SpaceAmbushBlockStatus()
{
	delete Stragglers;
	delete MoveList;
}

/**************************************************************************************************
* SpaceAmbushBlockStatus::Init -- Setup this blocking object
*
* In:			
*
* Out:		
*
* History: 2/25/2005 11:30AM JSY
**************************************************************************************************/
void SpaceAmbushBlockStatus::Init(LuaUserVar *command, GameObjectClass *target, int offset_direction, float offset_distance, float threat_tolerance)
{
	FAIL_IF(!PG_Dynamic_Cast<TaskForceClass>(command)) { return; }
	FAIL_IF(!target) { return; }

	Set_Command(command);
	Target = target;
	OffsetDirection = offset_direction;
	OffsetDistance = offset_distance;
	ThreatTolerance = threat_tolerance;
	IsFinished = false;
	Stragglers = 0;
	AmbushReady = false;

	Update_Movement(true);
}

/**************************************************************************************************
* SpaceAmbushBlockStatus::Find_Ambush_Point -- Decided where tp send the taskforce in preparation for
*	an ambush
*
* In:			
*
* Out:		
*
* History: 2/25/2005 11:30AM JSY
**************************************************************************************************/
Vector3 SpaceAmbushBlockStatus::Find_Ambush_Point()
{
	FAIL_IF(!Target) { return VECTOR3_INVALID; }

	//Direction is relative to target's current facing
	FacingClass facing = Target->Get_Facing();
	if (facing == FacingClass())
	{
		facing = FacingClass(0.0f, 90.0f, 0.0f);
	}	

	Vector2 target_face_direction = facing.Get_XY_Vector();
	Vector3 direction;
	switch (OffsetDirection)
	{
	case AIDataPackClass::FIRE_DIRECTION_LEFT:
		direction = Vector3(-target_face_direction.Y, target_face_direction.X, 0.0f);
		break;
	case AIDataPackClass::FIRE_DIRECTION_RIGHT:
		direction = Vector3(target_face_direction.Y, -target_face_direction.X, 0.0f);
		break;
	case AIDataPackClass::FIRE_DIRECTION_FRONT:
		direction = Vector3(target_face_direction);
		break;
	case AIDataPackClass::FIRE_DIRECTION_BACK:
		direction = -Vector3(target_face_direction);
		break;
	default:
		assert(false);
		return VECTOR3_INVALID;
	}

	const GameObjectTypeClass *target_type = 0;
	if (Target->Behaves_Like(BEHAVIOR_TRANSPORT))
	{
		target_type = Target->Get_Type();
	}
	else
	{
		target_type = Target->Get_Original_Object_Type();
	}

	float region_size = target_type->Get_Space_FOW_Reveal_Range();
	if (region_size <= 0.0f)
	{
		region_size = TheGameConstants.Get_AI_Space_Evaluator_Region_Size();
	}
	Vector3 sneak_target = Target->Get_Position() + OffsetDistance * direction;
	FRect threat_region = FRect(0.0f, 0.0f, region_size, region_size);
	bool found_sneak_spot = false;

	GameModeClass *space_mode = GameModeManager.Get_Game_Mode_By_Sub_Type(SUB_GAME_MODE_SPACE);
	FAIL_IF(!space_mode) { return VECTOR3_INVALID; }

	TaskForceClass *tf = static_cast<TaskForceClass*>(Get_Command());
	PlayerClass *player = tf->Get_Plan()->Get_Player();

	//Travel in the desired direction until we find a safe spot.  That's where we'll go.
	while (Box3Class::Overlap_Test(GameModeManager.Get_Active_Mode()->Get_Map_Bounds(), sneak_target))
	{
		threat_region.X = sneak_target.X - region_size / 2.0f;
		threat_region.Y = sneak_target.Y - region_size / 2.0f;

		float force = AIPerceptionSystemClass::Get_Perception_Grid()->Get_Force_Unnormalized(threat_region, GAME_OBJECT_CATEGORY_ALL, player, false, 0.0, NULL, NULL);
		if (force <= ThreatTolerance)
		{
			found_sneak_spot = true;
			break;
		}
		sneak_target += region_size * direction;
	}

	if (found_sneak_spot)
	{
		return sneak_target;
	}
	else
	{
		return VECTOR3_INVALID;
	}
}

/**************************************************************************************************
* SpaceAmbushBlockStatus::Update_Movement -- Periodically reissue move orders to keep units safe
*	and heading to the right place
*
* In:			
*
* Out:		
*
* History: 2/25/2005 11:30AM JSY
**************************************************************************************************/
void SpaceAmbushBlockStatus::Update_Movement(bool init)
{
	LastMoveUpdateFrame = GameModeManager.Get_Frame_Timer();

	TaskForceClass *tf = static_cast<TaskForceClass*>(Get_Command());
	
	Vector3 ambush_point = Find_Ambush_Point();
	if (ambush_point == VECTOR3_INVALID)
	{
		AIMESSAGE( ("Failed to find suitable point to ambush target %s with taskforce %s.  Aborting ambush block.", Target->Get_Type()->Get_Name()->c_str(), tf->Get_Definition()->Get_Name().c_str()) );
		IsFinished = true;
		AmbushReady = false;
		return;
	}

	//Very like the space movement block
	MovementCoordinatorSystemClass *system = GameModeManager.Get_Active_Mode()->Get_Movement_Coordinator_System();
	MovementCoordinatorClass *coordinator = 0;

	if (system)
	{
		coordinator = system->Create();
	}

	if (init)
	{
		Init_Move_List();
		SignalDispatcherClass::Get().Add_Listener(Target, this, PG_SIGNAL_OBJECT_DELETE_PENDING);
	}

	FAIL_IF(!Stragglers) { IsFinished = true; AmbushReady = false; return; }
	FAIL_IF(!MoveList) { IsFinished = true; AmbushReady = false; return; }

	MultiLinkedListIterator<GameObjectClass> it(MoveList);
	for (; !it.Is_Done(); it.Next())
	{
		GameObjectClass *object = it.Current_Object();
		if (!object) 
		{
			continue;
		}

		// if this object can be coordinated, stick it in the coordinator
		if (coordinator && system->Can_Object_Be_In_Formations(object))
		{
			Stragglers->Add(object);
			system->Remove_Object_From_Coordinators(object);
			coordinator->Add_Object(object);
		}
	}

	if (coordinator && coordinator->Get_Member_Count())
	{
		// set up the coordinator target/destination
		FormationDestinationStruct destination;
		destination.Type = FDT_POSITION;
		destination.Position = ambush_point;
		destination.ThreatTolerance = ThreatTolerance;

		coordinator->Set_Destination(destination);
	}
	else 
	{
		if (coordinator) 
		{
			system->Destroy(coordinator);
			coordinator = 0;
		}
	}
}

/**************************************************************************************************
* SpaceAmbushBlockStatus::Receive_Signal -- Handle notifications concerning taskforce members
*
* In:			
*
* Out:		
*
* History: 2/25/2005 11:30AM JSY
**************************************************************************************************/
void SpaceAmbushBlockStatus::Receive_Signal(SignalGeneratorClass *gen, PGSignalType type, SignalDataClass *)
{
	GameObjectClass *obj = static_cast<GameObjectClass*>(gen);

	switch (type) 
	{
		case PG_SIGNAL_OBJECT_MOVEMENT_BEGIN:
			break;

		case PG_SIGNAL_OBJECT_MOVEMENT_FINISHED:
			{
				//Don't quit listening to signals - they're still relevant because we may start moving again if not
				//everybody gets here in time.
				FAIL_IF(!Stragglers) { IsFinished = true; AmbushReady = false; break; }
				Stragglers->Remove(obj);

				TaskForceClass *tf = static_cast<TaskForceClass*>(Get_Command());
				SmartPtr<LuaTable> event_params = Return_Variable(GameObjectWrapper::Create(obj, tf->Get_Plan()->Get_Script()));
				tf->Signal_Event(PLAN_EVENT_UNIT_MOVE_FINISHED, event_params);
				Free_Lua_Table(event_params);

				if (Stragglers->Is_Empty()) 
				{
					AmbushReady = true;
					IsFinished = true;
				}
				break;
			}
		case PG_SIGNAL_OBJECT_MOVEMENT_CANCELED:
		case PG_SIGNAL_OBJECT_OWNER_CHANGED:
		case PG_SIGNAL_OBJECT_DELETE_PENDING:
			{
				SignalDispatcherClass::Get().Remove_Listener( obj, this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED );
				SignalDispatcherClass::Get().Remove_Listener( obj, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED );
				SignalDispatcherClass::Get().Remove_Listener( obj, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
				SignalDispatcherClass::Get().Remove_Listener( obj, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );

				if (obj == Target)
				{
					Target = 0;
					AmbushReady = true;
					IsFinished = true;
				}
				else
				{
					GameObjectClass *object = static_cast<GameObjectClass*>(gen);

					FAIL_IF(!MoveList) { IsFinished = true; AmbushReady = false; break; }
					FAIL_IF(!Stragglers) { IsFinished = true; AmbushReady = false; break; }

					Stragglers->Remove(object);
					MoveList->Remove(object);

					if (Stragglers->Is_Empty() || MoveList->Is_Empty())
					{
						IsFinished = true;
						AmbushReady = !MoveList->Is_Empty();
					}
				}
				break;
			}
		default:
			assert(false);
			return;
	}
}

/**************************************************************************************************
* SpaceAmbushBlockStatus::Is_Finished -- Script function to determine whether the operation is complete.
*	Also runs the update logic.
*
* In:			
*
* Out:		
*
* History: 2/25/2005 11:30AM JSY
**************************************************************************************************/
LuaTable *SpaceAmbushBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	if (!IsFinished)
	{
		int frames_since_update = GameModeManager.Get_Frame_Timer() - LastMoveUpdateFrame;
		if (frames_since_update > AMBUSH_UPDATE_INTERVAL_FRAMES)
		{
			Update_Movement(false);
		}
	}

	return Return_Variable(new LuaBool(IsFinished));
}

/**************************************************************************************************
* SpaceAmbushBlockStatus::Result -- Script function to determine whether the operation was successful.
*
* In:			
*
* Out:		
*
* History: 2/25/2005 11:30AM JSY
**************************************************************************************************/
LuaTable *SpaceAmbushBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(AmbushReady));
}

void SpaceAmbushBlockStatus::Init_Move_List()
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

enum
{
	CHUNK_ID_AMBUSH_BASE_CLASS,
	CHUNK_ID_AMBUSH_BLOCK_DATA,
	CHUNK_ID_AMBUSH_STRAGGLERS,

	CHUNK_ID_AMBUSH_LAST_UPDATE_FRAME,
	CHUNK_ID_AMBUSH_READY,
	CHUNK_ID_AMBUSH_IS_FINISHED,
	CHUNK_ID_AMBUSH_TARGET,
	CHUNK_ID_AMBUSH_DIRECTION,
	CHUNK_ID_AMBUSH_DISTANCE,
	CHUNK_ID_AMBUSH_THREAT_TOLERANCE,
	CHUNK_ID_AMBUSH_LISTENER_BASE,
};

/**************************************************************************************************
* SpaceAmbushBlockStatus::Save -- Writes this blocking object to file
*
* In:			
*
* Out:		
*
* History: 2/25/2005 1:40PM JSY
**************************************************************************************************/
bool SpaceAmbushBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(CHUNK_ID_AMBUSH_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(CHUNK_ID_AMBUSH_BLOCK_DATA);
	WRITE_MICRO_CHUNK(					CHUNK_ID_AMBUSH_LAST_UPDATE_FRAME,	LastMoveUpdateFrame);
	WRITE_MICRO_CHUNK(					CHUNK_ID_AMBUSH_READY,					AmbushReady);
	WRITE_MICRO_CHUNK(					CHUNK_ID_AMBUSH_IS_FINISHED,			IsFinished);
	WRITE_MICRO_CHUNK_OBJECT_PTR(		CHUNK_ID_AMBUSH_TARGET,					Target);
	WRITE_MICRO_CHUNK(					CHUNK_ID_AMBUSH_DIRECTION,				OffsetDirection);
	WRITE_MICRO_CHUNK(					CHUNK_ID_AMBUSH_DISTANCE,				OffsetDistance);
	WRITE_MICRO_CHUNK(					CHUNK_ID_AMBUSH_THREAT_TOLERANCE,	ThreatTolerance);
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_AMBUSH_LISTENER_BASE,		SignalListenerClass);
	ok &= writer->End_Chunk();

	WRITE_CHUNK_MLL_PTR_LIST(CHUNK_ID_AMBUSH_STRAGGLERS, Stragglers);

	return ok;
}

/**************************************************************************************************
* SpaceAmbushBlockStatus::Load -- Reads this blocking object from file
*
* In:			
*
* Out:		
*
* History: 2/25/2005 1:40PM JSY
**************************************************************************************************/
bool SpaceAmbushBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
			READ_CHUNK_MLL_PTR_LIST(CHUNK_ID_AMBUSH_STRAGGLERS, Stragglers);

		case CHUNK_ID_AMBUSH_BASE_CLASS:
			ok &= BlockingStatus::Load(script, reader);
			break;

		case CHUNK_ID_AMBUSH_BLOCK_DATA:
			while (reader->Open_Micro_Chunk())
			{
				switch (reader->Cur_Micro_Chunk_ID())
				{
					READ_MICRO_CHUNK(						CHUNK_ID_AMBUSH_LAST_UPDATE_FRAME,	LastMoveUpdateFrame);
					READ_MICRO_CHUNK(						CHUNK_ID_AMBUSH_READY,					AmbushReady);
					READ_MICRO_CHUNK(						CHUNK_ID_AMBUSH_IS_FINISHED,			IsFinished);
					READ_MICRO_CHUNK_OBJECT_PTR(		CHUNK_ID_AMBUSH_TARGET,					Target);
					READ_MICRO_CHUNK(						CHUNK_ID_AMBUSH_DIRECTION,				OffsetDirection);
					READ_MICRO_CHUNK(						CHUNK_ID_AMBUSH_DISTANCE,				OffsetDistance);
					READ_MICRO_CHUNK(						CHUNK_ID_AMBUSH_THREAT_TOLERANCE,	ThreatTolerance);
					READ_MICRO_CHUNK_MULTI_BASE_PTR(	CHUNK_ID_AMBUSH_LISTENER_BASE,		SignalListenerClass);

				default:
					ok = false;
					assert(false);
					break;
				}
				reader->Close_Micro_Chunk();
			}
			break;

		default:
			ok = false;
			assert(false);
			break;
		}

		reader->Close_Chunk();
	}

	return ok;
}