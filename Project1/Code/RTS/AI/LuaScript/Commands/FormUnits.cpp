// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FormUnits.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FormUnits.cpp $
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
#include "FormUnits.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "GameObject.h"
#include "PGSignal/SignalDispatcher.h"


PG_IMPLEMENT_RTTI(FleetMergeResult, SignalDataClass);

/**
 * Keeps track of the progress of a Formation Command.
 * @since 4/29/2004 6:33:23 PM -- BMH
 */
FormationBlockStatus::FormationBlockStatus()
{
}

/**
 * Init the BlockingStatus object.
 * 
 * @param command Command that spawned the blocking status.
 * @param fleet
 * @since 4/29/2004 6:34:04 PM -- BMH
 */
void FormationBlockStatus::Init(LuaUserVar *command, GameObjectWrapper *fleet)
{
	Set_Command(command);
	ResultObject = fleet;
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_FORMATION_MERGE );
	SignalDispatcherClass::Get().Add_Listener( fleet->Get_Object(), this, PG_SIGNAL_OBJECT_DELETE_PENDING );
}

void FormationBlockStatus::Add_Merge_Object(GameObjectClass *obj)
{
	MergeObjectIDs.push_back(obj->Get_ID());
}

LuaTable* FormationBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(MergeObjectIDs.size() == 0));
}

bool FormationBlockStatus::Internal_Is_Finished(void)
{
	return (MergeObjectIDs.size() == 0);
}

//    std::vector<int>                       MergeObjectIDs;
//    SmartPtr<GameObjectWrapper>            ResultObject; // return a pointer to the fleet
enum {
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_FORMATIONBLOCK_DATA,
	CHUNK_ID_FORMATIONBLOCK_MERGE_IDS,
	CHUNK_ID_FORMATIONBLOCK_RESULT,
	CHUNK_ID_FORMATIONBLOCK_LISTENER_BASE_ID,
};

bool FormationBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	WRITE_STL_VECTOR(CHUNK_ID_FORMATIONBLOCK_MERGE_IDS, MergeObjectIDs);

	ok &= writer->Begin_Chunk(CHUNK_ID_FORMATIONBLOCK_DATA);
		WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_FORMATIONBLOCK_LISTENER_BASE_ID, SignalListenerClass);
	ok &= writer->End_Chunk();

	LUA_WRITE_CHUNK_VALUE_PTR	(	CHUNK_ID_FORMATIONBLOCK_RESULT, ResultObject, script);

	return (ok);
}

bool FormationBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;
	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case CHUNK_ID_BASE_CLASS:
				ok &= BlockingStatus::Load(script, reader);
				break;

			LUA_READ_CHUNK_VALUE_PTR	(	CHUNK_ID_FORMATIONBLOCK_RESULT, 		ResultObject, script);
			READ_STL_VECTOR				(	CHUNK_ID_FORMATIONBLOCK_MERGE_IDS, 	MergeObjectIDs);
			case CHUNK_ID_FORMATIONBLOCK_DATA:
				while (reader->Open_Micro_Chunk()) {
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_FORMATIONBLOCK_LISTENER_BASE_ID, SignalListenerClass);
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

LuaTable* FormationBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(ResultObject);
}

/// signal dispatcher interface
void FormationBlockStatus::Receive_Signal(SignalGeneratorClass *, PGSignalType type, SignalDataClass *data)
{
	FleetMergeResult *prod = PG_Dynamic_Cast<FleetMergeResult>(data);

	switch (type) {
		case PG_SIGNAL_OBJECT_FORMATION_MERGE:
			{
				if (!prod) return;
				std::vector<int>::iterator it = std::find(MergeObjectIDs.begin(), MergeObjectIDs.end(), prod->ObjectID);
				if (it != MergeObjectIDs.end()) MergeObjectIDs.erase(it);
			}
			break;
		case PG_SIGNAL_OBJECT_DELETE_PENDING:
			{
				MergeObjectIDs.resize(0);
				ResultObject = 0;
			}
			break;
		default:
			return;
	}
}

LUA_IMPLEMENT_FACTORY(LUA_CHUNK_FORMATION_BLOCK, FormationBlockStatus);
PG_IMPLEMENT_RTTI(FormationBlockStatus, BlockingStatus);



