// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/AbilityBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/AbilityBlock.cpp $
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

#include "AbilityBlock.h"

#include "AI/Planning/TaskForce.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/Planning/PlanBehavior.h"
#include "AI/Planning/PlanEventManager.h"

PG_IMPLEMENT_RTTI(AbilityBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_ABILITY_BLOCK, AbilityBlockStatus);

void AbilityBlockStatus::Init(LuaUserVar *command)
{
	Set_Command(command);
}

void AbilityBlockStatus::Add_Object(GameObjectClass *object)
{
	++UnfinishedAbilityCount;
	SignalDispatcherClass::Get().Add_Listener(object, this, PG_SIGNAL_OBJECT_ABILITY_FINISHED);
	SignalDispatcherClass::Get().Add_Listener(object, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED);
	SignalDispatcherClass::Get().Add_Listener(object, this, PG_SIGNAL_OBJECT_DELETE_PENDING);
}

void AbilityBlockStatus::Receive_Signal(SignalGeneratorClass *generator, PGSignalType, SignalDataClass *)
{
	--UnfinishedAbilityCount;
	SignalDispatcherClass::Get().Remove_Listener(generator, this, PG_SIGNAL_OBJECT_ABILITY_FINISHED);
	SignalDispatcherClass::Get().Remove_Listener(generator, this, PG_SIGNAL_OBJECT_DELETE_PENDING);
	SignalDispatcherClass::Get().Remove_Listener(generator, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED);
}

enum
{
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_ABILITY_BLOCK_DATA,
	CHUNK_ID_ABILITY_BLOCK_LISTENER_BASE_ID,
	CHUNK_ID_ABILITY_BLOCK_UNFINISHED_COUNT,
};

bool AbilityBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(CHUNK_ID_ABILITY_BLOCK_DATA);
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_ABILITY_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
	WRITE_MICRO_CHUNK(CHUNK_ID_ABILITY_BLOCK_UNFINISHED_COUNT, UnfinishedAbilityCount);
	ok &= writer->End_Chunk();

	return ok;
}

bool AbilityBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
		case CHUNK_ID_BASE_CLASS:
			ok &= BlockingStatus::Load(script, reader);
			break;

		case CHUNK_ID_ABILITY_BLOCK_DATA:
			while (reader->Open_Micro_Chunk())
			{
				switch (reader->Cur_Micro_Chunk_ID())
				{
					READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_ABILITY_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
					READ_MICRO_CHUNK(CHUNK_ID_ABILITY_BLOCK_UNFINISHED_COUNT, UnfinishedAbilityCount);

				default:
					assert(false);
					ok = false;
					break;
				}

				reader->Close_Micro_Chunk();
			}
			break;

		default:
			assert(false);
			ok = false;
			break;
		}

		reader->Close_Chunk();
	}

	return ok;
}

