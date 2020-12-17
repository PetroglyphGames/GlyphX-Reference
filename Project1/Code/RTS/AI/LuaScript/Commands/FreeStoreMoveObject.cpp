// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FreeStoreMoveObject.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FreeStoreMoveObject.cpp $
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


#include "FreeStoreMoveObject.h"
#include "AI/LuaScript/GameObjectWrapper.h"
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
#include "Player.h"
#include "AIPlayer.h"
#include "AI/Goal/AIGoalReachabilityType.h"
#include "AI/Goal/InstantiatedGoal.h"
#include "AI/Goal/AIGoalProposalFunction.h"
#include "AI/Goal/AIGoalType.h"
#include "AI/Execution/AIExecutionSystem.h"
#include "AI/Execution/AIExecutionPath.h"
#include "PlanetaryBehavior.h"
#include "GameObjectManager.h"


/**
 * Keeps track of the progress of a Movement Command.
 * @since 4/29/2004 6:33:23 PM -- BMH
 */
FreeStoreMovementBlockStatus::FreeStoreMovementBlockStatus() :
	IsFinished(false)
,	DoAdvance(false)
,	Player(NULL)
,	ThreatThreshold(0)
{
}

/**
 * Init the BlockingStatus object.
 * 
 * @param command Command that spawned the blocking status.
 * @param fleet
 * @since 4/29/2004 6:34:04 PM -- BMH
 */
void FreeStoreMovementBlockStatus::Init(GameObjectWrapper *fleet, GameObjectWrapper *dest, PlayerClass *player, float threat_threshold)
{
	ResultObject = fleet;
	Destination = dest;
	Player = player;
	ThreatThreshold = threat_threshold;
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_MOVEMENT_BEGIN );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_SPACE_CONFLICT_BEGIN );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_SPACE_CONFLICT_END );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_GROUND_CONFLICT_BEGIN );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_GROUND_CONFLICT_END );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_DELETE_PENDING );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_RETREAT );

	Advance();
}

const GameObjectClass *FreeStoreMovementBlockStatus::Get_Next_Hop(void)
{
	//1/4/2006 JSY - If there are *any* battles scheduled we must neither advance nor report
	//finished - in general this won't be an issue because the game is normally paused when battles occur,
	//but there are certain conditions where the game is allowed to continue even with a battle
	//scheduled in order to allow a story sequence to complete.  It's these cases we're guarding
	//against here since we may actually be involved in the pending conflict!
	if (GAME_OBJECT_MANAGER.Get_Num_Space_Conflicts() > 0 ||
			GAME_OBJECT_MANAGER.Get_Num_Land_Invasions() > 0)
	{
		//Set the DoAdvance flag so that we'll try again later.
		DoAdvance = true;
		return NULL;
	}

	if (!ResultObject || !ResultObject->Get_Object()) {
		IsFinished = true;
		Debug_Print("FreeStoreMovementBlockStatus::Get_Next_Hop -- ResultObject no longer valid.");
		return NULL;
	}

	GameObjectClass *from = ResultObject->Get_Object()->Get_Parent_Container_Object();
	if (!from || !from->Behaves_Like(BEHAVIOR_PLANET)) {
		IsFinished = true;
		Debug_Print("FreeStoreMovementBlockStatus::Get_Next_Hop -- Fleet: %8.8X doesn't appear to be on a planet",
																 ResultObject->Get_Object());
		return NULL;
	}

	if (from == Destination->Get_Object()) {
		IsFinished = true;
		return NULL;
	}

	AIExecutionPathFinderClass *pathfinder = Player->Get_AI_Player()->Get_Tactical_Manager_By_Mode(SUB_GAME_MODE_GALACTIC)->
		Get_Execution_System()->Get_Path_Finder();
	assert(pathfinder);

	const AIExecutionPathClass &path = pathfinder->Find_Path(from, Destination->Get_Object());
	if (path.Is_Safe_Path(ThreatThreshold) == false)
	{
		IsFinished = true;
		Debug_Print("FreeStoreMovementBlockStatus::Get_Next_Hop -- Unable to find a path from %s to %s.",
																	from->Get_Type()->Get_Name()->c_str(), 
																	Destination->Get_Object()->Get_Type()->Get_Name()->c_str());
		return NULL;
	}

	const GameObjectClass *dest_planet = static_cast<const GameObjectClass*>(path.Get_Hop(1));
	PlanetaryBehaviorClass *planet_behave = static_cast<PlanetaryBehaviorClass*>(dest_planet->Get_Behavior(BEHAVIOR_PLANET));
	ENFORCED_IF(planet_behave)
	{
		PlayerClass *orbit_player = planet_behave->Get_Occupying_Orbit_Player(dest_planet);
		if (orbit_player && orbit_player != Player)
		{
			IsFinished = true;
			return NULL;
		}
	}

	return dest_planet;
}

bool FreeStoreMovementBlockStatus::Advance(void)
{
	DoAdvance = false;
	const GameObjectClass *hop = Get_Next_Hop();
	if (!hop) return false;

	MoveObjectToObjectEventClass event;
	
	event.Init(ResultObject->Get_Object()->Get_ID(), hop->Get_ID());
	event.Execute();

	return true;
}

bool FreeStoreMovementBlockStatus::Internal_Is_Finished(void)
{
	if (DoAdvance)
		Advance();

	return IsFinished;
}

LuaTable* FreeStoreMovementBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(Internal_Is_Finished()));
}

enum {
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_MOVEMENT_BLOCK_DATA,
	CHUNK_ID_MOVEMENT_BLOCK_DOADVANCE,
	CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED,
	CHUNK_ID_MOVEMENT_BLOCK_RESULT,
	CHUNK_ID_MOVEMENT_BLOCK_DESTINATION,
	CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID,
	CHUNK_ID_MOVEMENT_BLOCK_PLAYER,
};

bool FreeStoreMovementBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();
	
	ok &= writer->Begin_Chunk(CHUNK_ID_MOVEMENT_BLOCK_DATA);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_DOADVANCE, DoAdvance);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED, IsFinished);
		WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
		WRITE_MICRO_CHUNK_OBJECT_PTR(CHUNK_ID_MOVEMENT_BLOCK_PLAYER, Player);
	ok &= writer->End_Chunk();
	LUA_WRITE_CHUNK_VALUE_PTR	(	CHUNK_ID_MOVEMENT_BLOCK_RESULT, ResultObject, script);
	LUA_WRITE_CHUNK_VALUE_PTR	(	CHUNK_ID_MOVEMENT_BLOCK_DESTINATION, Destination, script);

	return (ok);
}

bool FreeStoreMovementBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
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
						READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_DOADVANCE, DoAdvance);
						READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED, IsFinished);
						READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
						READ_MICRO_CHUNK_OBJECT_PTR(CHUNK_ID_MOVEMENT_BLOCK_PLAYER, Player);
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;
			LUA_READ_CHUNK_VALUE_PTR	(	CHUNK_ID_MOVEMENT_BLOCK_RESULT, ResultObject, script);
			LUA_READ_CHUNK_VALUE_PTR	(	CHUNK_ID_MOVEMENT_BLOCK_DESTINATION, Destination, script);
			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}
	return (ok);
}

LuaTable* FreeStoreMovementBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(ResultObject);
}

/// signal dispatcher interface
void FreeStoreMovementBlockStatus::Receive_Signal(SignalGeneratorClass *, PGSignalType type, SignalDataClass *)
{
	switch (type) {
		case PG_SIGNAL_OBJECT_MOVEMENT_BEGIN:
			break;

		case PG_SIGNAL_OBJECT_MOVEMENT_CANCELED:
			{
				IsFinished = true;
				Debug_Print("FreeStoreMovementBlockStatus::Receive_Signal -- Object Move Canceled.  Unable to path to %s.",
																		 Destination->Get_Object()->Get_Type()->Get_Name()->c_str());
				break;
			}
		case PG_SIGNAL_OBJECT_MOVEMENT_FINISHED:
			{
				//If Advance fails then we've reached the end of our journey and 
				//should quit listening for signals.
				DoAdvance = true;
				break;
			}
		case PG_SIGNAL_OBJECT_RETREAT:
		case PG_SIGNAL_OBJECT_DELETE_PENDING:
			{
				IsFinished = true;
				ResultObject = 0;
				break;
			}
		case PG_SIGNAL_OBJECT_SPACE_CONFLICT_BEGIN:
			{
				DoAdvance = IsFinished = false;
			}
			break;
		case PG_SIGNAL_OBJECT_SPACE_CONFLICT_END:
			{
				DoAdvance = true;
			}
			break;

		default:
			return;
	}
}

GameObjectClass *FreeStoreMovementBlockStatus::Get_Fleet_Object(void) const
{
	if (ResultObject) {
		return ResultObject->Get_Object();
	}
	return NULL;
}


LUA_IMPLEMENT_FACTORY(LUA_CHUNK_FREE_MOVEMENT_BLOCK, FreeStoreMovementBlockStatus);

PG_IMPLEMENT_RTTI(FreeStoreMovementBlockStatus, BlockingStatus);


