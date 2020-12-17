// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/BombingBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/BombingBlock.cpp $
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


#include "BombingBlock.h"
#include "GameObject.h"
#include "PGSignal/SignalDispatcher.h"
#include "AI/Planning/TaskForce.h"

PG_IMPLEMENT_RTTI(BombingBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_BOMBING_BLOCK, BombingBlockStatus);

/**************************************************************************************************
* BombingBlockStatus::Init -- Setup this blocking object to track the status of the bombing run
*
* In:			taskforce that started the bombing run
*				player that started the bombing run
*				vector of active bombers
*
* Out:		
*
* History: 2/2/2005 11:35AM JSY
**************************************************************************************************/
void BombingBlockStatus::Init(LuaUserVar *command, int player_id, const DynamicVectorClass<GameObjectClass*> &bombers)
{
	TaskForceClass *tf = PG_Dynamic_Cast<TaskForceClass>(command);
	FAIL_IF(!tf) { return; }

	Set_Command(command);

	OriginalBomberCount = bombers.Size();
	SurvivingBomberCount = OriginalBomberCount;
	ActiveBomberCount = OriginalBomberCount;

	FAIL_IF(OriginalBomberCount == 0) { return; }

	for (int i = 0; i < bombers.Size(); ++i)
	{
		if (bombers[i]->Get_Owner() == player_id)
		{
			SignalDispatcherClass::Get().Add_Listener(bombers[i], this, PG_SIGNAL_OBJECT_DELETE_PENDING);
		}
	}
}

/**************************************************************************************************
* BombingBlockStatus::Is_Finished -- This blocking object is done when all bombers have either died or left
*	the map
*
* In:		
*
* Out:		
*
* History: 2/2/2005 11:37AM JSY
**************************************************************************************************/
LuaTable *BombingBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(ActiveBomberCount == 0));
}

/**************************************************************************************************
* BombingBlockStatus::Result -- The result of this block is the fraction of bombers that survived
*
* In:		
*
* Out:		
*
* History: 2/2/2005 11:37AM JSY
**************************************************************************************************/
LuaTable *BombingBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaNumber(static_cast<float>(SurvivingBomberCount) / OriginalBomberCount));
}

/**************************************************************************************************
* BombingBlockStatus::Receive_Signal -- Handle a bomber being destroyed
*
* In:		
*
* Out:		
*
* History: 2/2/2005 11:37AM JSY
**************************************************************************************************/
void BombingBlockStatus::Receive_Signal(SignalGeneratorClass *gen, PGSignalType, SignalDataClass *)
{
	SignalDispatcherClass::Get().Remove_Listener(gen, this, PG_SIGNAL_OBJECT_DELETE_PENDING);

	//It's important to know whether the bomber is being deleted because it was killed or just because it
	//left the map.
	GameObjectClass *bomber = static_cast<GameObjectClass*>(gen);
	--ActiveBomberCount;
	if (bomber->Is_Dead())
	{
		--SurvivingBomberCount;
	}
}

enum
{
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_BOMBING_BLOCK_DATA,
	CHUNK_ID_BOMBING_BLOCK_LISTENER_BASE_ID,
	CHUNK_ID_BOMBING_BLOCK_ORIGINAL_BOMBER_COUNT,
	CHUNK_ID_BOMBING_BLOCK_SURVIVING_BOMBER_COUNT,
	CHUNK_ID_BOMBING_BLOCK_ACTIVE_BOMBER_COUNT,
};

/**************************************************************************************************
* BombingBlockStatus::Save -- Write this blocking object to file
*
* In:		
*
* Out:		
*
* History: 2/2/2005 11:38AM JSY
**************************************************************************************************/
bool BombingBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(CHUNK_ID_BOMBING_BLOCK_DATA);
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(	CHUNK_ID_BOMBING_BLOCK_LISTENER_BASE_ID,			SignalListenerClass);
	WRITE_MICRO_CHUNK(						CHUNK_ID_BOMBING_BLOCK_ORIGINAL_BOMBER_COUNT,	OriginalBomberCount);
	WRITE_MICRO_CHUNK(						CHUNK_ID_BOMBING_BLOCK_SURVIVING_BOMBER_COUNT,	SurvivingBomberCount);
	WRITE_MICRO_CHUNK(						CHUNK_ID_BOMBING_BLOCK_ACTIVE_BOMBER_COUNT,		ActiveBomberCount);
	ok &= writer->End_Chunk();

	return ok;
}

/**************************************************************************************************
* BombingBlockStatus::Load -- Read this blocking object to file
*
* In:		
*
* Out:		
*
* History: 2/2/2005 11:39AM JSY
**************************************************************************************************/
bool BombingBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;
	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
		case CHUNK_ID_BASE_CLASS:
			ok &= BlockingStatus::Load(script, reader);
			break;

		case CHUNK_ID_BOMBING_BLOCK_DATA:
			while (reader->Open_Micro_Chunk())
			{
				switch (reader->Cur_Micro_Chunk_ID())
				{
					READ_MICRO_CHUNK_MULTI_BASE_PTR(	CHUNK_ID_BOMBING_BLOCK_LISTENER_BASE_ID,			SignalListenerClass);
					READ_MICRO_CHUNK(						CHUNK_ID_BOMBING_BLOCK_ORIGINAL_BOMBER_COUNT,	OriginalBomberCount);
					READ_MICRO_CHUNK(						CHUNK_ID_BOMBING_BLOCK_SURVIVING_BOMBER_COUNT,	SurvivingBomberCount);
					READ_MICRO_CHUNK(						CHUNK_ID_BOMBING_BLOCK_ACTIVE_BOMBER_COUNT,		ActiveBomberCount);

				default:
					assert(false);
					break;
				}

				reader->Close_Micro_Chunk();
			}
			break;

		default:
			assert(false);
			break;
		}

		reader->Close_Chunk();
	}

	return ok;
}