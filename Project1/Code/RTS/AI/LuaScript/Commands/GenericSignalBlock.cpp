// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GenericSignalBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GenericSignalBlock.cpp $
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

#include "GenericSignalBlock.h"

PG_IMPLEMENT_RTTI(GenericSignalBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_GENERIC_SIGNAL_BLOCK, GenericSignalBlockStatus);

void GenericSignalBlockStatus::Init(LuaUserVar *command, SignalGeneratorClass *gen, PGSignalType signal_type)
{
	Set_Command(command);
	SignalDispatcherClass::Get().Add_Listener(gen, this, signal_type);
}

enum
{
	CHUNK_ID_GENERIC_SIGNAL_BLOCK_BASE_CLASS,
	CHUNK_ID_GENERIC_SIGNAL_BLOCK_DATA,
	CHUNK_ID_GENERIC_SIGNAL_BLOCK_LISTENER_BASE,
	CHUNK_ID_GENERIC_SIGNAL_BLOCK_IS_FINISHED
};

bool GenericSignalBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(CHUNK_ID_GENERIC_SIGNAL_BLOCK_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(CHUNK_ID_GENERIC_SIGNAL_BLOCK_DATA);
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_GENERIC_SIGNAL_BLOCK_LISTENER_BASE, SignalListenerClass);
	WRITE_MICRO_CHUNK(CHUNK_ID_GENERIC_SIGNAL_BLOCK_IS_FINISHED, IsFinished);
	ok &= writer->End_Chunk();

	return ok;
}

bool GenericSignalBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
		case CHUNK_ID_GENERIC_SIGNAL_BLOCK_BASE_CLASS:
			ok &= BlockingStatus::Load(script, reader);
			break;

		case CHUNK_ID_GENERIC_SIGNAL_BLOCK_DATA:
			while (reader->Open_Micro_Chunk())
			{
				switch (reader->Cur_Micro_Chunk_ID())
				{
					READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_GENERIC_SIGNAL_BLOCK_LISTENER_BASE, SignalListenerClass);
					READ_MICRO_CHUNK(CHUNK_ID_GENERIC_SIGNAL_BLOCK_IS_FINISHED, IsFinished);

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