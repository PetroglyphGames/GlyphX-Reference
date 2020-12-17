// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/InvasionBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/InvasionBlock.cpp $
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
#include "AI/LuaScript/Commands/InvasionBlock.h"

#include "AI/Planning/TaskForce.h"
#include "AI/Planning/PlanEventManager.h"
#include "PGSignal/SignalDispatcher.h"
#include "GameObject.h"

REGISTER_PLAN_EVENT(PLAN_EVENT_GROUND_CONFLICT_BEGIN, "Ground_Conflict_Begin");
REGISTER_PLAN_EVENT(PLAN_EVENT_GROUND_CONFLICT_END, "Ground_Conflict_End");

PG_IMPLEMENT_RTTI(InvasionResultClass, SignalDataClass);

InvasionBlockStatus::InvasionBlockStatus() :
Success(false),
IsFinished(false)
{
}

/**
 * Initializes the blocking status object
 * 
 * @param planet					The planet we're invading.
 *
 * @since 5/18/2004 11:40AM -- JSY
 */
void InvasionBlockStatus::Init(TaskForceClass *command, GameObjectClass *planet)
{
	Set_Command(command);
	//Listen for the planet to tell us that the invasion is over.
	SignalDispatcherClass::Get().Add_Listener(planet, this, PG_SIGNAL_OBJECT_ACTION_COMPLETE);
	SignalDispatcherClass::Get().Add_Listener(planet, this, PG_SIGNAL_OBJECT_GROUND_CONFLICT_BEGIN);
	SignalDispatcherClass::Get().Add_Listener(planet, this, PG_SIGNAL_OBJECT_GROUND_CONFLICT_END);
}

LuaTable *InvasionBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(IsFinished));
}

/**
 * Get the result of the blocking operation
 * 
 * @param script			Unused
 * @param params			Unused
 * return					The result of the invasion.  True if the invaders won,
 *								false if they were repelled
 * @since 5/18/2004 11:40AM -- JSY
 */
LuaTable *InvasionBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(Success));
}

void InvasionBlockStatus::Receive_Signal(SignalGeneratorClass *, PGSignalType type, SignalDataClass *data)
{
	InvasionResultClass *result = PG_Dynamic_Cast<InvasionResultClass>(data);

	//If we have an appropriate signal, set the flag to end the blocking,
	//hang on to the result of the invasion and quit listening for signals.
	switch (type) {
		case PG_SIGNAL_OBJECT_ACTION_COMPLETE:
			if (result)
			{
				Success = result->Success;
				IsFinished = true;
				SignalDispatcherClass::Get().Remove_Listener_From_All(this);
			}
			break;

		case PG_SIGNAL_OBJECT_GROUND_CONFLICT_BEGIN:
			{
				TaskForceClass *tf = (TaskForceClass *)Get_Command();
				tf->Signal_Event(PLAN_EVENT_GROUND_CONFLICT_BEGIN, NULL);
			}
			break;
		case PG_SIGNAL_OBJECT_GROUND_CONFLICT_END:
			{
				TaskForceClass *tf = (TaskForceClass *)Get_Command();
				tf->Signal_Event(PLAN_EVENT_GROUND_CONFLICT_END, NULL);
			}
			break;
	}
}
//    bool Success;
//    bool IsFinished;
enum {
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_INVADE_BLOCK_DATA,
	CHUNK_ID_INVADE_BLOCK_SUCCESS,
	CHUNK_ID_INVADE_BLOCK_ISFINISHED,
	CHUNK_ID_INVADE_BLOCK_LISTENER_BASE_ID,
};

bool InvasionBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
		ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();
	
	ok &= writer->Begin_Chunk(CHUNK_ID_INVADE_BLOCK_DATA);
		WRITE_MICRO_CHUNK(CHUNK_ID_INVADE_BLOCK_SUCCESS, Success);
		WRITE_MICRO_CHUNK(CHUNK_ID_INVADE_BLOCK_ISFINISHED, IsFinished);
		WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_INVADE_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
	ok &= writer->End_Chunk();

	return (ok);
}

bool InvasionBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;
	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case CHUNK_ID_BASE_CLASS:
				ok &= BlockingStatus::Load(script, reader);
				break;

			case CHUNK_ID_INVADE_BLOCK_DATA:
				while (reader->Open_Micro_Chunk()) {
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK(CHUNK_ID_INVADE_BLOCK_SUCCESS, Success);
						READ_MICRO_CHUNK(CHUNK_ID_INVADE_BLOCK_ISFINISHED, IsFinished);
						READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_INVADE_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
						default: assert(false); break;	// Unknown Chunk
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

LUA_IMPLEMENT_FACTORY(LUA_CHUNK_INVASION_BLOCK, InvasionBlockStatus);
PG_IMPLEMENT_RTTI(InvasionBlockStatus, BlockingStatus);