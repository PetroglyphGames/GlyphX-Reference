// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/MoveObject.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/MoveObject.cpp $
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


#include "MoveObject.h"
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
#include "AI/Goal/AIGoalReachabilityType.h"
#include "AI/Goal/InstantiatedGoal.h"
#include "AI/Goal/AIGoalProposalFunction.h"
#include "AI/Goal/AIGoalType.h"
#include "PlanetaryBehavior.h"
#include "GameObjectManager.h"
#include "FleetBehavior.h"
#include "FleetLocomotorBehavior.h"

REGISTER_PLAN_EVENT(PLAN_EVENT_SPACE_CONFLICT_BEGIN, "Space_Conflict_Begin");
REGISTER_PLAN_EVENT(PLAN_EVENT_SPACE_CONFLICT_END, "Space_Conflict_End");


/**
 * Keeps track of the progress of a Movement Command.
 * @since 4/29/2004 6:33:23 PM -- BMH
 */
MovementBlockStatus::MovementBlockStatus() :
	IsFinished(false)
,	DoAdvance(false)
{
}

/**
 * Init the BlockingStatus object.
 * 
 * @param command Command that spawned the blocking status.
 * @param fleet
 * @since 4/29/2004 6:34:04 PM -- BMH
 */
void MovementBlockStatus::Init(LuaUserVar *command, GameObjectWrapper *fleet, GameObjectWrapper *dest, AIGoalReachabilityType reachability)
{
	assert(PG_Dynamic_Cast<TaskForceClass>(command) || PG_Dynamic_Cast<LuaScriptClass>(command));

	Set_Command(command);
	ResultObject = fleet;
	Destination = dest;
	Reachability = reachability;
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_MOVEMENT_BEGIN );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_SPACE_CONFLICT_BEGIN );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_DELETE_PENDING );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_RETREAT );


	Advance();
}

const GameObjectClass *MovementBlockStatus::Get_Next_Hop(void)
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

	LuaScriptClass *script = PG_Dynamic_Cast<LuaScriptClass>(Get_Command());
	if (!script)
	{
		TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(Get_Command());
		if (tf)
		{
			script = tf->Get_Plan()->Get_Script();

			//If the taskforce is empty, quit moving even though the fleet may still be valid (retreated)
			if (tf->Get_Force_Count() == 0)
			{
				IsFinished = true;
				return NULL;
			}
		}
	}

	if (!ResultObject || !ResultObject->Get_Object()) {
		IsFinished = true;
		if (script) {
			script->Script_Warning("MovementBlockStatus::Get_Next_Hop -- ResultObject no longer valid.");
			ResultObject = NULL;
		}
		return NULL;
	}

	GameObjectClass *from = ResultObject->Get_Object()->Get_Parent_Container_Object();
	if (!from || !from->Behaves_Like(BEHAVIOR_PLANET)) {
		IsFinished = true;
		if (script) {
			script->Script_Warning("MovementBlockStatus::Get_Next_Hop -- Fleet: %8.8X doesn't appear to be on a planet",
																 ResultObject->Get_Object());
		}
		return NULL;
	}

	if (from == Destination->Get_Object()) {
		IsFinished = true;
		return NULL;
	}

	GalacticPathFindCallPropertiesStruct path_find_props(ResultObject->Get_Object()->Get_Owner_Player());
	path_find_props.StartPlanet = from;
	path_find_props.EndPlanet = Destination->Get_Object();
	path_find_props.IsHostilePassable = (Reachability == GOAL_REACHABILITY_ANY);
	path_find_props.IsNeutralPassable = (Reachability == GOAL_REACHABILITY_ANY);
	path_find_props.IsDefendedPassable = (Reachability == GOAL_REACHABILITY_ANY);
	path_find_props.AllowBadFactionIfStartEnd = (Reachability >= GOAL_REACHABILITY_ENEMY_DESTINATION);
	path_find_props.AllowBadFactionIfNoForce = (Reachability >= GOAL_REACHABILITY_ENEMY_UNDEFENDED);

	//If the source of this move was not a taskforce then it probably comes from a non-standard AI script
	//or else a story script and we should allow it complete freedom of movement.
	path_find_props.EnforceAIPlanetRestrictions = PG_Is_Type<TaskForceClass>(Get_Command());
	
	GalacticPathClass path;

	if (!GalacticPathFinderClass::Get().Find_Path(path_find_props, path))
	{
		IsFinished = true;
		if (script)
		{
			script->Script_Warning("MovementBlockStatus::Get_Next_Hop -- Unable to find a path from %s to %s.",
																		from->Get_Type()->Get_Name()->c_str(), 
																		Destination->Get_Object()->Get_Type()->Get_Name()->c_str());
		}
		return NULL;
	}

	const GameObjectClass *dest_planet = path.Get_Path_Node(1);

	PlanetaryBehaviorClass *planet_behave = static_cast<PlanetaryBehaviorClass*>(dest_planet->Get_Behavior(BEHAVIOR_PLANET));
	ENFORCED_IF(planet_behave)
	{
		PlayerClass *orbit_player = planet_behave->Get_Occupying_Orbit_Player(dest_planet);
		if (orbit_player && orbit_player != ResultObject->Get_Object()->Get_Owner_Player())
		{
			//If this is an intermediate and unfriendly planet then we should schedule a bribe payment if possible
			if (dest_planet != Destination->Get_Object() && ResultObject->Get_Object()->Get_Owner_Faction()->Get_Benefits_From_Corruption())
			{
				ResultObject->Get_Object()->Get_Fleet_Data()->Set_Should_AI_Fleet_Pay_Bribe(true);
			}
			else if (Reachability < GOAL_REACHABILITY_ENEMY_DESTINATION)
			{
				IsFinished = true;
				return NULL;
			}
		}
	}

	return dest_planet;
}

bool MovementBlockStatus::Advance(void)
{
	DoAdvance = false;
	const GameObjectClass *hop = Get_Next_Hop();
	if (!hop)
	{
		if (!ResultObject)
		{
			return false;
		}

		//It's possible that we skipped a conflict on arrival by paying a bribe but then, for whatever reason, discovered
		//that we were unable to move off the planet.  To prevent this from leaving us in an unpleasant state where enemy
		//factions share orbit without conflict we must perform an extra test for a space battle.
		GameObjectClass *fleet = ResultObject->Get_Object();
		if (!fleet)
		{
			return false;
		}

		FleetBehaviorClass *fleet_behave = static_cast<FleetBehaviorClass*>(fleet->Get_Behavior(BEHAVIOR_FLEET));
		FAIL_IF(!fleet_behave) { return false; }

		GameObjectClass *planet = fleet_behave->Get_Planet_Orbited();
		if (!planet)
		{
			return false;
		}

		fleet_behave->Unlink_Fleet_From_Planetary_Orbit(planet);
		if (!fleet_behave->Link_Fleet_To_Planetary_Orbit(planet))
		{
			return false;
		}

		FleetLocomotorBehaviorClass *fleet_locomotor = static_cast<FleetLocomotorBehaviorClass*>(fleet->Get_Behavior(BEHAVIOR_LOCO));
		FAIL_IF(!fleet_locomotor) { return NULL; }
		fleet_locomotor->Add_Nearby_Fleets_To_Planet(fleet, planet);

		fleet->Get_Manager()->Initiate_Space_Conflict(planet, fleet->Get_Owner());

		return false;
	}

	MoveObjectToObjectEventClass event;
	
	event.Init(ResultObject->Get_Object()->Get_ID(), hop->Get_ID());
	event.Execute();

	return true;
}

bool MovementBlockStatus::Internal_Is_Finished(void)
{
	if (DoAdvance)
		Advance();

	return IsFinished;
}

LuaTable* MovementBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(Internal_Is_Finished() || !ResultObject->Get_Object()));
}

enum {
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_MOVEMENT_BLOCK_DATA,
	CHUNK_ID_MOVEMENT_BLOCK_DOADVANCE,
	CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED,
	CHUNK_ID_MOVEMENT_BLOCK_RESULT,
	CHUNK_ID_MOVEMENT_BLOCK_DESTINATION,
	CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID,
	CHUNK_ID_MOVEMENT_BLOCK_REACHABILITY,
};

bool MovementBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
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
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_REACHABILITY, Reachability);
	ok &= writer->End_Chunk();
	LUA_WRITE_CHUNK_VALUE_PTR	(	CHUNK_ID_MOVEMENT_BLOCK_RESULT, ResultObject, script);
	LUA_WRITE_CHUNK_VALUE_PTR	(	CHUNK_ID_MOVEMENT_BLOCK_DESTINATION, Destination, script);

	return (ok);
}

bool MovementBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
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
						READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_REACHABILITY, Reachability);
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

LuaTable* MovementBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(ResultObject);
}

/// signal dispatcher interface
void MovementBlockStatus::Receive_Signal(SignalGeneratorClass *gen, PGSignalType type, SignalDataClass *data)
{
	switch (type) {
		case PG_SIGNAL_OBJECT_MOVEMENT_BEGIN:
			break;

		case PG_SIGNAL_OBJECT_MOVEMENT_CANCELED:
			{
				IsFinished = true;
				TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(Get_Command());
				if (tf)
				{
					tf->Get_Plan()->Get_Script()->Script_Error("MovementBlockStatus::Receive_Signal -- Object Move Canceled.  Unable to path to %s.",
																				Destination->Get_Object()->Get_Type()->Get_Name()->c_str());
				}
				break;
			}
		case PG_SIGNAL_OBJECT_MOVEMENT_FINISHED:
			{
				TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(Get_Command());
				if (tf)
				{
					tf->Signal_Event(PLAN_EVENT_REACHED_SYSTEM_ON_PATH, 0);
				}
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
				GameObjectClass *planet = (GameObjectClass *)data;
				TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(Get_Command());
				if (tf)
				{
					tf->Signal_Event(PLAN_EVENT_SPACE_CONFLICT_BEGIN, NULL);
				}
				SignalDispatcherClass::Get().Add_Listener( planet, this, PG_SIGNAL_OBJECT_SPACE_CONFLICT_END );
			}
			break;
		case PG_SIGNAL_OBJECT_SPACE_CONFLICT_END:
			{
				TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(Get_Command());
				if (tf)
				{
					tf->Signal_Event(PLAN_EVENT_SPACE_CONFLICT_END, NULL);
				}
				SignalDispatcherClass::Get().Remove_Listener(gen, this, PG_SIGNAL_OBJECT_SPACE_CONFLICT_END);

				//Careful! Units can end up in space combat without being at a planet because of the way we collect nearby units
				//when starting a conflict.
				if (ResultObject && ResultObject->Get_Object() && ResultObject->Get_Object()->Get_Parent_Container_Object() == gen)
				{
					DoAdvance = true;
				}
			}
			break;
		default:
			return;
	}
}

LUA_IMPLEMENT_FACTORY(LUA_CHUNK_MOVEMENT_BLOCK, MovementBlockStatus);
PG_IMPLEMENT_RTTI(MovementBlockStatus, BlockingStatus);



