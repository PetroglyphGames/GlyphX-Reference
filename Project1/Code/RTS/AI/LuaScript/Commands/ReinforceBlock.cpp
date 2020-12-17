// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ReinforceBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ReinforceBlock.cpp $
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


#include "ReinforceBlock.h"
#include "RTSSignals.h"
#include "AI/Planning/TaskForce.h"
#include "GameObject.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/Planning/PlanBehavior.h"
#include "AI/Planning/AIPlanningSystem.h"
#include "AI/TacticalAIManager.h"
#include "AI/Execution/AIExecutionSystem.h"
#include "AI/Execution/AIFreeStore.h"
#include "AI/Goal/InstantiatedGoal.h"
#include "AI/AIPlayer.h"
#include "GameObjectManager.h"
#include "ObjectPersistence.h"
#include "AI/LuaScript/GameObjectTypeWrapper.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "PlanetaryBehavior.h"
#include "ProductionBehavior.h"
#include "FleetManagementEvent.h"
#include "GameObjectTypeManager.h"
#include "SpaceReinforceBlock.h"
#include "LandMode.h"
#include "AI/AILog.h"
#include "AI/AIHintZone.h"
#include "CellArray.h"


PG_IMPLEMENT_RTTI(ReinforceBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_REINFORCE_BLOCK, ReinforceBlockStatus);

std::vector<Vector3> LuaReinforceCommandClass::LandingLocations;


ReinforceBlockStatus::~ReinforceBlockStatus()
{ 
	Remove_Land_Point(); 
}

void ReinforceBlockStatus::Remove_Land_Point(void)
{
	for (int i = 0; i < (int)LuaReinforceCommandClass::LandingLocations.size(); i++)
	{
		if (LandPoint == LuaReinforceCommandClass::LandingLocations[i])
		{
			LuaReinforceCommandClass::LandingLocations.erase(LuaReinforceCommandClass::LandingLocations.begin() + i);
			break;
		}
	}
	LandPoint = VECTOR3_INVALID;
}

/**************************************************************************************************
* ReinforceBlockStatus::Result -- Return a table of all the units brought down.
*
* In:			
*
* Out:		
*
* History: 5/5/2005 10:09:43 AM -- BMH
**************************************************************************************************/
LuaTable *ReinforceBlockStatus::Result(LuaScriptClass *, LuaTable *)
{ 
	if (!ReinforceUnits)
	{
		return Return_Variable(new LuaBool(ResultValue));
	}

	LuaVar::Pointer val = ReinforceUnits;
	ReinforceUnits = 0;
	return Return_Variable(val);
}


/**************************************************************************************************
* ReinforceBlockStatus::Receive_Signal -- handle notification that a transport has unloaded
*
* In:			
*
* Out:		
*
* History: 12/1/2004 4:49PM JSY
**************************************************************************************************/
void ReinforceBlockStatus::Receive_Signal(SignalGeneratorClass *gen, PGSignalType, SignalDataClass *data)
{
	SignalDispatcherClass::Get().Remove_Listener(gen, this, PG_SIGNAL_OBJECT_REINFORCEMENTS_UNLOADED);

	ReinforcementsUnloadedDataClass *reinforce_data = PG_Dynamic_Cast<ReinforcementsUnloadedDataClass>(data);
	FAIL_IF(!reinforce_data) { return; }

	--TransportCount;

	TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(Get_Command());
	if (!tf)
	{
		assert(Script);
		if (!ReinforceUnits)
		{
			ReinforceUnits = Alloc_Lua_Table();
		}

		for (int i = 0; i < reinforce_data->Reinforcements.Size(); i++)
		{
			GameObjectClass *tf_member = reinforce_data->Reinforcements[i];
			FAIL_IF(!tf_member) { continue; }

			if (tf_member->Is_Team_Member())
			{
				tf_member = tf_member->Get_Parent_Container_Object();
			}

			int t = 0;
			for (t = 0; t < (int)ReinforceUnits->Value.size(); t++)
			{
				GameObjectWrapper *wrap = PG_Dynamic_Cast<GameObjectWrapper>(ReinforceUnits->Value[t]);
				FAIL_IF(!wrap) continue;
				if (wrap->Get_Object() == tf_member) break;
			}
			if (t != (int)ReinforceUnits->Value.size()) continue;

			ReinforceUnits->Value.push_back(GameObjectWrapper::Create(tf_member, Script));
			AIPlayerClass *ai_player = tf_member->Get_Owner_Player()->Get_AI_Player();
			if (ai_player)
			{
				AIFreeStoreClass *freestore = (*ai_player->Find_Tactical_Manager_By_Mode(GameModeManager.Get_Active_Mode()))->Get_Execution_System()->Get_Free_Store();
				freestore->Remove_Free_Store_Object(tf_member);
			}
		}
		return;
	}

	//AIFreeStoreClass *free_store = tf->Get_Plan()->Get_Planning_System()->Get_Manager()->Get_Execution_System()->Get_Free_Store();

	//Because every team member is in the list of created objects we get, but we only add the teams to
	//the taskforce, we're going to track what we've already added to avoid duplicates
	static std::vector<GameObjectClass*> added_objects;
	added_objects.resize(0);

	for (int i = 0; i < reinforce_data->Reinforcements.Size(); ++i)
	{
		GameObjectClass *tf_member = reinforce_data->Reinforcements[i];
		FAIL_IF(!tf_member) { continue; }

		//Move from inidividual units up to the team.
		if (tf_member->Get_Parent_Container_Object())
		{
			tf_member = tf_member->Get_Parent_Container_Object();

			//See whether we've put this fellow in already.  This is only possible
			//if he's a team.
			bool already_added = false;
			for (unsigned int j = 0; j < added_objects.size(); ++j)
			{
				if (added_objects[j] == tf_member)
				{
					already_added = true;
					break;
				}
			}

			if (already_added)
			{
				continue;
			}
		}

      SmartPtr<LuaTable> params = Return_Variable(GameObjectWrapper::Create(tf_member, tf->Get_Plan()->Get_Script()));
		tf->Add_Force(tf->Get_Plan()->Get_Script(), params);
		added_objects.push_back(tf_member);
		tf->Get_Plan()->Get_Goal()->Get_Potential_Plan()->Add_Reinforcement_As_Free_Store_Unit(tf_member);
		Free_Lua_Table(params);

		//This object will have been added to the freestore on creation, but as a reinforcement he's
		//already tied to a taskforce.  Rather than fix up all the reservation mess we'll just remove him
		//again.  He'll get put on the freestore for real when this taskforce releases him.
		//free_store->Remove_Free_Store_Object(tf_member);
	}
}

/**************************************************************************************************
* ReinforceBlockStatus::Init -- Set up the blocking object
*
* In:			command that spawned the block (this had better be a taskforce)
*				number of transports we're waiting on
*				was the reinforce command that spawned this block successful?
*
* Out:		
*
* History: 12/1/2004 4:49PM JSY
**************************************************************************************************/
void ReinforceBlockStatus::Init(LuaUserVar *command, int transport_count, bool result, LuaScriptClass *script, const Vector3 &land_point)
{
	ResultValue = result;
	Set_Command(command);
	TransportCount = transport_count;
	CanPlanBeAbandoned = true;
	Script = script;
	LandPoint = land_point;
	TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(Get_Command());
	if (tf)
	{
		CanPlanBeAbandoned = tf->Get_Plan()->Is_Goal_System_Removable();
		tf->Get_Plan()->Set_As_Goal_System_Removable(false);
	}
}

/**************************************************************************************************
* ReinforceBlockStatus::Is_Finished -- Lua function that specifies whether our block is over
*
* In:		
*
* Out:		
*
* History: 12/1/2004 4:49PM JSY
**************************************************************************************************/
LuaTable *ReinforceBlockStatus::Is_Finished(LuaScriptClass *, LuaTable*)
{ 
	bool is_finished = (TransportCount == 0);

	if (is_finished)
	{
		Remove_Land_Point();
		//Possibly unlock abandonment of this plan this reinforce command was executed by
		TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(Get_Command());
		if (tf)
		{
			tf->Get_Plan()->Set_As_Goal_System_Removable(CanPlanBeAbandoned);
		}
	}
	return Return_Variable(new LuaBool(is_finished)); 
}

enum
{
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_REINFORCE_BLOCK_DATA,
	CHUNK_ID_REINFORCE_BLOCK_LISTENER_BASE_ID,
	CHUNK_ID_REINFORCE_BLOCK_TRANSPORT_COUNT,
	CHUNK_ID_REINFORCE_BLOCK_CAN_PLAN_BE_ABANDONED,
	CHUNK_ID_REINFORCE_BLOCK_RESULT,
	CHUNK_ID_REINFORCE_BLOCK_UNITS,
};

/**************************************************************************************************
* ReinforceBlockStatus::Save -- Write this blocking object to file
*
* In:		
*
* Out:		
*
* History: 12/2/2004 3:19PM JSY
**************************************************************************************************/
bool ReinforceBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(CHUNK_ID_REINFORCE_BLOCK_DATA);
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(	CHUNK_ID_REINFORCE_BLOCK_LISTENER_BASE_ID,		SignalListenerClass);
	WRITE_MICRO_CHUNK(						CHUNK_ID_REINFORCE_BLOCK_TRANSPORT_COUNT,			TransportCount);
	WRITE_MICRO_CHUNK(						CHUNK_ID_REINFORCE_BLOCK_CAN_PLAN_BE_ABANDONED,	CanPlanBeAbandoned);
	WRITE_MICRO_CHUNK(						CHUNK_ID_REINFORCE_BLOCK_RESULT,						ResultValue);
	ok &= writer->End_Chunk();

	LUA_WRITE_CHUNK_VALUE_PTR(				CHUNK_ID_REINFORCE_BLOCK_UNITS,						ReinforceUnits, 			script);

	return ok;
}

/**************************************************************************************************
* ReinforceBlockStatus::Load -- Read this blocking object from file
*
* In:		
*
* Out:		
*
* History: 12/2/2004 3:19PM JSY
**************************************************************************************************/
bool ReinforceBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;
	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
		case CHUNK_ID_BASE_CLASS:
			ok &= BlockingStatus::Load(script, reader);
			break;

		case CHUNK_ID_REINFORCE_BLOCK_DATA:
			while (reader->Open_Micro_Chunk())
			{
				switch (reader->Cur_Micro_Chunk_ID())
				{
					READ_MICRO_CHUNK_MULTI_BASE_PTR(	CHUNK_ID_REINFORCE_BLOCK_LISTENER_BASE_ID,		SignalListenerClass);
					READ_MICRO_CHUNK(						CHUNK_ID_REINFORCE_BLOCK_TRANSPORT_COUNT,			TransportCount);
					READ_MICRO_CHUNK(						CHUNK_ID_REINFORCE_BLOCK_CAN_PLAN_BE_ABANDONED,	CanPlanBeAbandoned);
					READ_MICRO_CHUNK(						CHUNK_ID_REINFORCE_BLOCK_RESULT,						ResultValue);

				default:
					assert(false);
					break;
				}

				reader->Close_Micro_Chunk();
			}
			break;

		LUA_READ_CHUNK_VALUE_PTR(				CHUNK_ID_REINFORCE_BLOCK_UNITS,						ReinforceUnits, 			script);

		default:
			assert(false);
			break;
		}

		reader->Close_Chunk();
	}

	Script = script;

	return ok;
}


PG_IMPLEMENT_RTTI(LuaReinforceCommandClass, LuaUserVar);

/**
 * Reinforce lua script command
 * Params: object_type, position, player
 * 
 * @param script lua script
 * @param params params
 * 
 * @return reinforce block status.
 * @since 5/2/2005 1:14:01 PM -- BMH
 */
LuaTable *LuaReinforceCommandClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
	{
		script->Script_Error("LuaReinforceCommandClass::Reinforce -- Command only valid in a tactical game.");
		return NULL;
	}

	if (params->Value.size() < 3)
	{
		script->Script_Error("LuaReinforceCommandClass::Reinforce -- Invalid number of parameters.");
		return NULL;
	}

	GameObjectTypeWrapper *type_wrapper = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[0]);
	PlayerWrapper *player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[2]);

	if (!player)
	{
		script->Script_Error("LuaReinforceCommandClass::Reinforce -- Invalid parameter for player.");
		return NULL;
	}

	if (!type_wrapper)
	{
		script->Script_Error("LuaReinforceCommandClass::Reinforce -- Invalid parameter for type.");
		return NULL;
	}
	LuaBool::Pointer delete_after = NULL;
	LuaBool::Pointer obey_zones = NULL;
	if (params->Value.size() > 3)
	{
		delete_after = PG_Dynamic_Cast<LuaBool>(params->Value[3]);
		if (params->Value.size() > 4)
		{
			obey_zones = PG_Dynamic_Cast<LuaBool>(params->Value[4]);
		}
	}

	Vector3 land_pos(VECTOR3_INVALID);
	bool create_only = PG_Dynamic_Cast<LuaBool>(params->Value[1]) ? true : false;
	if (!create_only)
	{
		Lua_Extract_Position(params->Value[1], land_pos);
		if (land_pos == VECTOR3_INVALID)
		{
			script->Script_Error("LuaReinforceCommandClass::Reinforce -- Invalid position parameter.");
			return NULL;
		}
	}

	GameModeClass *this_mode = GameModeManager.Get_Active_Mode();
	GameModeClass *parent_mode = GameModeManager.Get_Parent_Game_Mode(this_mode);

	SpaceReinforceBlockStatus *sbs = NULL;
	ReinforceBlockStatus *bs = NULL;
	if (!create_only && this_mode->Get_Sub_Type() == SUB_GAME_MODE_SPACE)
	{
		sbs = static_cast<SpaceReinforceBlockStatus*>(SpaceReinforceBlockStatus::FactoryCreate());
		FAIL_IF(!sbs) { return NULL; }

		if (obey_zones)
		{
			sbs->Set_Obey_Reinforce_Rules(obey_zones->Value);
		}
	}
	else if (!create_only)
	{
		if (obey_zones && obey_zones->Value)
		{
			//If we're following reinforcement rules then check pop cap 
			if (!GAME_OBJECT_MANAGER.Is_Room_For_Reinforcements(player->Get_Object()->Get_ID(), type_wrapper->Get_Object()))
			{
				return NULL;
			}
		}

		land_pos = Find_Reinforcement_Landing_Location(player->Get_Object(), land_pos, 1000.0f, 
																				const_cast<GameObjectTypeClass *>(type_wrapper->Get_Object()), obey_zones->Value);
		if (land_pos == VECTOR3_INVALID)
		{
			return NULL;
		}

		bs = static_cast<ReinforceBlockStatus*>(ReinforceBlockStatus::FactoryCreate());
		FAIL_IF(!bs) { return NULL; }

		bs->Init(NULL, 1, true, script, land_pos);
	}

	FAIL_IF (create_only && !parent_mode) { return NULL; }

	if (parent_mode)
	{
		GameObjectClass *build_planet = NULL;
		ConflictInfoStruct *conflict_info = NULL;
		if (this_mode->Get_Sub_Type() == SUB_GAME_MODE_SPACE)
		{
			conflict_info = parent_mode->Get_Object_Manager().Get_Space_Conflict_Info();
			build_planet = parent_mode->Get_Object_Manager().Get_Object_From_ID(conflict_info->LocationID);
		}
		else if (this_mode->Get_Sub_Type() == SUB_GAME_MODE_LAND)
		{
			conflict_info = parent_mode->Get_Object_Manager().Get_Land_Invasion_Info();
			build_planet = parent_mode->Get_Object_Manager().Get_Object_From_ID(conflict_info->LocationID);
		}
		FAIL_IF(!build_planet || !conflict_info) return NULL;

		// Find a unit in galactic of the same type and kill it.
		ReferenceListIterator<GameObjectClass> it(GALACTIC_GAME_OBJECT_MANAGER.Get_Owned_List(player->Get_Object()->Get_ID()));
		for ( ; !it.Is_Done(); it.Next())
		{
			GameObjectClass *object = it.Current_Object();

			FAIL_IF(!object) continue;
			if (object->Get_Original_Object_Type() != type_wrapper->Get_Object()) continue;

			GameObjectClass *pobj = object->Get_Parent_Container_Object();
			if (pobj && pobj->Get_Behavior(BEHAVIOR_FLEET))
			{
				pobj = pobj->Get_Parent_Container_Object();
			}

			// Don't destroy the objects on this planet.
			if (pobj && pobj->Get_Behavior(BEHAVIOR_PLANET) && pobj == build_planet) continue;

			object->Destroy();
			break;
		}

		// if we have to create this hero search the respawn list at each planet and
		// remove the respawn event for this player.
		for (int i = 0; i < GALACTIC_GAME_OBJECT_MANAGER.Get_Object_Count(); i++)
		{
			GameObjectClass *object = GALACTIC_GAME_OBJECT_MANAGER.Get_Object_At_Index(i);
			if (!object || !object->Behaves_Like(BEHAVIOR_PLANET)) continue;

			PlanetaryBehaviorClass *behave = (PlanetaryBehaviorClass *)object->Get_Behavior(BEHAVIOR_PLANET);
			behave->Remove_Scheduled_Object_Type_Creation(type_wrapper->Get_Object());
		}

		ProductionBehaviorClass *pbehavior = static_cast<ProductionBehaviorClass *> (build_planet->Get_Behavior( BEHAVIOR_PRODUCTION ));
		assert(pbehavior);

		// Now create the object at the planet - could return a NULL object if a special type
		GameObjectClass *built_object = pbehavior->Create_And_Place_Object_Type_At_Location(type_wrapper->Get_Object(), build_planet, player->Get_Object()->Get_ID());
		FAIL_IF (!built_object) 
		{
			return NULL;
		}

		GameObjectTypeClass *fleet_type = GameObjectTypeManager.Find_Object_Type( "Galactic_Fleet" );
		bool is_fleet = (built_object->Get_Type() == fleet_type);

		if (!is_fleet && built_object->Get_Parent_Container_Object() && built_object->Get_Parent_Container_Object()->Get_Behavior(BEHAVIOR_PLANET))
		{
			FleetManagementEventClass event;
			event.Init(build_planet->Get_ID(), INVALID_OBJECT_ID, built_object->Get_ID(), 
							FleetManagementEventClass::ACTION_TO_ORBIT);
			event.Set_Player_ID(player->Get_Object()->Get_ID());
			event.Execute();

			event = FleetManagementEventClass();
			event.Init(build_planet->Get_ID(), INVALID_OBJECT_ID, built_object->Get_ID(), 
							FleetManagementEventClass::ACTION_ADD);
			event.Set_Player_ID(player->Get_Object()->Get_ID());
			event.Execute();
		}

		if (is_fleet) 
		{
			built_object = built_object->Get_Fleet_Data()->Get_Starship_List()->Get_At(0);
		}

		built_object->Set_Destroy_After_Tactical(delete_after ? delete_after->Value : true);

		ObjectPersistenceClass::PersistentUnit new_unit;
		new_unit.ObjectID = built_object->Get_ID();
		ObjectPersistenceClass *persist_objects = GameModeManager.Get_Parent_Mode_Persistent_Objects(this_mode);
		DynamicVectorClass<ObjectPersistenceClass::PersistentUnit> &persist_reinforcements = persist_objects->Get_Reinforcements(player->Get_Object()->Get_ID());
		int ref_idx = persist_reinforcements.Get_Count();
		persist_reinforcements.Add(new_unit);

		AIPlayerClass *ai_player = player->Get_Object()->Get_AI_Player();
		if (ai_player)
		{
			TacticalAIManagerClass *tactical_manager = ai_player->Get_Tactical_Manager_By_Mode(this_mode->Get_Sub_Type());
			if (tactical_manager && tactical_manager->Get_Execution_System())
			{
				AIFreeStoreClass *freestore = tactical_manager->Get_Execution_System()->Get_Free_Store();
				ENFORCED_IF(freestore)
				{
					freestore->Append_Reinforce_Reservation(ref_idx, (PotentialPlanClass *)-1);
				}
			}
		}

		if (create_only)
		{
			return NULL;
		}

		if (this_mode->Get_Sub_Type() == SUB_GAME_MODE_SPACE)
		{
			std::vector<int> reinforcement_indices;
			reinforcement_indices.push_back(ref_idx);

			sbs->Init(NULL, reinforcement_indices, land_pos, BIG_FLOAT, script, player->Get_Object()->Get_ID());
		}
		else
		{
			GAME_OBJECT_MANAGER.Reinforce(built_object->Get_ID(), land_pos, player->Get_Object()->Get_ID());
			SignalDispatcherClass::Get().Add_Listener(built_object, bs, PG_SIGNAL_OBJECT_REINFORCEMENTS_UNLOADED);
		}
	}
	else
	{
		player->Get_Object()->Add_Skirmish_Reinforcement(type_wrapper->Get_Object());

		if (this_mode->Get_Sub_Type() == SUB_GAME_MODE_SPACE)
		{
			std::vector<int> reinforcement_indices;
			reinforcement_indices.push_back((int)type_wrapper->Get_Object()->Get_Name_CRC());

			sbs->Init(NULL, reinforcement_indices, land_pos, BIG_FLOAT, script, player->Get_Object()->Get_ID());
		}
		else
		{
			GameObjectClass *transport = GAME_OBJECT_MANAGER.Reinforce((int)type_wrapper->Get_Object()->Get_Name_CRC(), land_pos, player->Get_Object()->Get_ID());
			SignalDispatcherClass::Get().Add_Listener(transport, bs, PG_SIGNAL_OBJECT_REINFORCEMENTS_UNLOADED);
		}
	}

	if (sbs)
	{
		return Return_Variable(sbs);
	}
	else
	{
		return Return_Variable(bs);
	}
}



Vector3 LuaReinforceCommandClass::Find_Reinforcement_Landing_Location(PlayerClass *player, const Vector3 &target_position, float distance_to_check, const GameObjectTypeClass *type, bool obey_zones)
{
	FAIL_IF(GameModeManager.Get_Sub_Type() != SUB_GAME_MODE_LAND) { return VECTOR3_INVALID; }

	LandModeClass *land_mode = static_cast<LandModeClass*>(GameModeManager.Get_Active_Mode());
	FAIL_IF(!land_mode) { return VECTOR3_INVALID; }

	int player_id = player->Get_ID();

	if (obey_zones)
	{
		static const float ANGLE_DELTA = FLOAT_M_PI / 4.0f;

		//Find landing locations and sort them by distance from the requested landing location
		ReferenceListIterator<GameObjectClass> it(GAME_OBJECT_MANAGER.Get_Reinforcement_Modifying_Objects());

		typedef std::multimap<float, GameObjectClass*> ReinforcementPointMap;
		ReinforcementPointMap sorted_reinforcement_points;

		float distance_limit2 = distance_to_check * distance_to_check;
		for( ; !it.Is_Done(); it.Next())
		{
			GameObjectClass *reinforcement_point = it.Current_Object();
			FAIL_IF(!reinforcement_point) { continue; }
			if (reinforcement_point->Get_Type()->Get_Reinforcement_Enabling_Radius() <= 0.0f)
			{
				continue;
			}

			if (!reinforcement_point->Is_Ally(player))
			{
				continue;
			}

			float score = (target_position - reinforcement_point->Get_Position()).Project_XY().Length2();

			if (score > distance_limit2)
			{
				continue;
			}

			sorted_reinforcement_points.insert(std::make_pair(score, reinforcement_point));
		}

		for (ReinforcementPointMap::iterator i = sorted_reinforcement_points.begin(); 
				i != sorted_reinforcement_points.end(); 
				++i)
		{
			GameObjectClass *reinforcement_point = i->second;

			float try_radius = reinforcement_point->Get_Type()->Get_Reinforcement_Enabling_Radius() / 2.0f;

			for (float angle = 0.0f; angle < FLOAT_M_2PI; angle += ANGLE_DELTA)
			{
				Vector3 try_position = Vector3(1.0f, 0.0f, 0.0f);
				try_position.Rotate_Z(angle);
				try_position *= try_radius;
				try_position += reinforcement_point->Get_Position();

				//Don't ignore existing landing transports - we want immediate response
				if (land_mode->Can_Land_Reinforcements(player_id, try_position, type, false, false))
				{
					return try_position;
				}
			}
		}

		return VECTOR3_INVALID;
	}
	else
	{
		static const float ANGLE_DELTA = FLOAT_M_PI / 8.0f;
		static const float RADIUS_DELTA = 50.0f;

		for (float try_radius = 0.0f; try_radius < distance_to_check; try_radius += RADIUS_DELTA)
		{
			for (float angle = 0.0f; angle < FLOAT_M_2PI; angle += ANGLE_DELTA)
			{
				Vector3 try_position = Vector3(1.0f, 0.0f, 0.0f);
				try_position.Rotate_Z(angle);
				try_position *= try_radius;
				try_position += target_position;
				//Don't ignore existing landing transports - we want immediate response
				if (land_mode->Can_Land_Reinforcements(player_id, try_position, type, true, false))
				{
					return try_position;
				}

				if (try_radius == 0.0f)
				{
					break;
				}
			}
		}
	}

	//We exhausted all possibilities and there wasn't anywhere good enough
	return VECTOR3_INVALID;
}

