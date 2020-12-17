// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LandUnits.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LandUnits.cpp $
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
#include "LandUnits.h"
#include "FormUnits.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "GameObject.h"
#include "PGSignal/SignalDispatcher.h"
#include "AI/Planning/GalacticTaskforce.h"
#include "AI/Planning/PlanBehavior.h"


/**
 * Keeps track of the progress of a Formation Command.
 * @since 4/29/2004 6:33:23 PM -- BMH
 */
LandUnitsBlockStatus::LandUnitsBlockStatus()
{
}

/**
 * Init the BlockingStatus object.
 * 
 * @param command    Command that spawned the blocking status.
 * @param fleet      The result object to return to Lua.
 * @param land_units true if you want to land units
 *                   false if you want to launch units into orbit.
 * @since 8/29/2004 2:55:58 PM -- BMH
 */
void LandUnitsBlockStatus::Init(LuaUserVar *command, GameObjectWrapper *fleet, bool)
{
	Set_Command(command);
	ResultObject = fleet;
	if (fleet) {
		SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_DELETE_PENDING );
	}
}

void LandUnitsBlockStatus::Add_Merge_Object(GameObjectClass *obj, bool land_units /*= true*/)
{
	if (!land_units) {
		SignalDispatcherClass::Get().Add_Listener( obj, this, PG_SIGNAL_OBJECT_FORMATION_ADD );
	}
	else
	{
		SignalDispatcherClass::Get().Add_Listener( obj, this, PG_SIGNAL_OBJECT_FORMATION_LAND );
	}

	MergeObjectIDs.push_back(obj->Get_ID());
}

LuaTable* LandUnitsBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(MergeObjectIDs.size() == 0));
}

bool LandUnitsBlockStatus::Internal_Is_Finished(void)
{
	return (MergeObjectIDs.size() == 0);
}

enum {
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_LANDUNITSBLOCK_DATA,
	CHUNK_ID_LANDUNITSBLOCK_MERGE_IDS,
	CHUNK_ID_LANDUNITSBLOCK_RESULT,
	CHUNK_ID_LANDUNITSBLOCK_LISTENER_BASE_ID,
};

bool LandUnitsBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
		ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	WRITE_STL_VECTOR(CHUNK_ID_LANDUNITSBLOCK_MERGE_IDS, MergeObjectIDs);

	ok &= writer->Begin_Chunk(CHUNK_ID_LANDUNITSBLOCK_DATA);
		WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_LANDUNITSBLOCK_LISTENER_BASE_ID, SignalListenerClass);
	ok &= writer->End_Chunk();

	LUA_WRITE_CHUNK_VALUE_PTR	(	CHUNK_ID_LANDUNITSBLOCK_RESULT, ResultObject, script);

	return (ok);
}

bool LandUnitsBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;
	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case CHUNK_ID_BASE_CLASS:
				ok &= BlockingStatus::Load(script, reader);
				break;

			READ_STL_VECTOR(CHUNK_ID_LANDUNITSBLOCK_MERGE_IDS, MergeObjectIDs);
			LUA_READ_CHUNK_VALUE_PTR	(	CHUNK_ID_LANDUNITSBLOCK_RESULT, ResultObject, script);
			case CHUNK_ID_LANDUNITSBLOCK_DATA:
				while (reader->Open_Micro_Chunk()) {
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_LANDUNITSBLOCK_LISTENER_BASE_ID, SignalListenerClass);
						default: assert(false); break;   // Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;
			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}
	return (ok);
}

LuaTable* LandUnitsBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(ResultObject);
}

/// signal dispatcher interface
void LandUnitsBlockStatus::Receive_Signal(SignalGeneratorClass *_ship, PGSignalType type, SignalDataClass *)
{
	GameObjectClass *ship = (GameObjectClass *)_ship;

	switch (type) {
		case PG_SIGNAL_OBJECT_FORMATION_LAND:
			{
				std::vector<int>::iterator it = std::find(MergeObjectIDs.begin(), MergeObjectIDs.end(),ship->Get_ID());
				if (it != MergeObjectIDs.end()) MergeObjectIDs.erase(it);
			}
			break;
		case PG_SIGNAL_OBJECT_DELETE_PENDING:
			{
				MergeObjectIDs.resize(0);
				ResultObject = 0;
			}
			break;

		case PG_SIGNAL_OBJECT_FORMATION_ADD:
			{
				std::vector<int>::iterator it = std::find(MergeObjectIDs.begin(), MergeObjectIDs.end(), ship->Get_ID());
				if (it != MergeObjectIDs.end()) MergeObjectIDs.erase(it);
				GalacticTaskForceClass *tf = PG_Dynamic_Cast<GalacticTaskForceClass>(Get_Command());
				if (tf->Get_Fleet_Object() == NULL) {
					assert(ship && ship->Get_Parent_Container_Object() && 
							 ship->Get_Parent_Container_Object()->Behaves_Like(BEHAVIOR_FLEET));
					tf->Set_Fleet_Object(GameObjectWrapper::Create(ship->Get_Parent_Container_Object(), tf->Get_Plan()->Get_Script()));
				}
				SignalDispatcherClass::Get().Remove_Listener( ship, this, PG_SIGNAL_OBJECT_FORMATION_ADD );
			}
			break;

		default:
			return;
	}
}

LUA_IMPLEMENT_FACTORY(LUA_CHUNK_LAND_UNITS_BLOCK, LandUnitsBlockStatus);
PG_IMPLEMENT_RTTI(LandUnitsBlockStatus, BlockingStatus);




