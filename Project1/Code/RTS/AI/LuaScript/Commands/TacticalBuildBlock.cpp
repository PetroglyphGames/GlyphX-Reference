// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/TacticalBuildBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/TacticalBuildBlock.cpp $
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


#include "TacticalBuildBlock.h"

#include "PGSignal/SignalDispatcher.h"
#include "GameObject.h"
#include "AI/Planning/TaskForce.h"

PG_IMPLEMENT_RTTI(TacticalBuildBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_TACTICAL_BUILD_BLOCK, TacticalBuildBlockStatus);

/**************************************************************************************************
* TacticalBuildBlockStatus::Init -- Setup this blocking object
*
* In:			
*
* Out:		
*
* History: 3/3/2005 6:32PM JSY
**************************************************************************************************/
void TacticalBuildBlockStatus::Init(LuaUserVar *command)
{
	assert(PG_Dynamic_Cast<TaskForceClass>(command));
	Set_Command(command);
	PendingBuilds = new MultiLinkedListClass<GameObjectClass>();
}

/**************************************************************************************************
* TacticalBuildBlockStatus::Add_Tactical_Build -- Add to the list of builds tracked by this blocking object
*
* In:			
*
* Out:		
*
* History: 3/3/2005 6:33PM JSY
**************************************************************************************************/
void TacticalBuildBlockStatus::Add_Tactical_Build(GameObjectClass *pad)
{
	FAIL_IF(!pad) { return; }
	FAIL_IF(!PendingBuilds) { return; }

	SignalDispatcherClass::Get().Add_Listener(pad, this, PG_SIGNAL_OBJECT_TACTICAL_CONSTRUCTION_COMPLETE);
	PendingBuilds->Add(pad);
}

/**************************************************************************************************
* TacticalBuildBlockStatus::Is_Finished -- Script function to determine whether this block is done
*
* In:			
*
* Out:		
*
* History: 3/3/2005 6:33PM JSY
**************************************************************************************************/
LuaTable *TacticalBuildBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(PendingBuilds && PendingBuilds->Is_Empty()));
}

/**************************************************************************************************
* TacticalBuildBlockStatus::Receive_Signal -- Handle signal when a build we're waiting on finishes
*
* In:			
*
* Out:		
*
* History: 3/3/2005 6:34PM JSY
**************************************************************************************************/
void TacticalBuildBlockStatus::Receive_Signal(SignalGeneratorClass *gen, PGSignalType, SignalDataClass *)
{
	FAIL_IF(!PendingBuilds) { return; }
	PendingBuilds->Remove(static_cast<GameObjectClass*>(gen));
	SignalDispatcherClass::Get().Remove_Listener(gen, this, PG_SIGNAL_OBJECT_TACTICAL_CONSTRUCTION_COMPLETE);
}

enum
{
	CHUNK_ID_TACTICAL_BUILD_BLOCK_BASE_CLASS,
	CHUNK_ID_TACTICAL_BUILD_BLOCK_DATA,
	CHUNK_ID_TACTICAL_BUILD_BLOCK_LISTENER_BASE,
	CHUNK_ID_TACTICAL_BUILD_BLOCK_BUILD_LIST,
};

/**************************************************************************************************
* TacticalBuildBlockStatus::Save -- Write this blocking object to file
*
* In:			
*
* Out:		
*
* History: 3/3/2005 6:34PM JSY
**************************************************************************************************/
bool TacticalBuildBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(CHUNK_ID_TACTICAL_BUILD_BLOCK_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(CHUNK_ID_TACTICAL_BUILD_BLOCK_DATA);
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_TACTICAL_BUILD_BLOCK_LISTENER_BASE, SignalListenerClass);
	ok &= writer->End_Chunk();

	WRITE_CHUNK_MLL_PTR_LIST(CHUNK_ID_TACTICAL_BUILD_BLOCK_BUILD_LIST, PendingBuilds);

	return ok;
}

/**************************************************************************************************
* TacticalBuildBlockStatus::Load -- Read this blocking object from file
*
* In:			
*
* Out:		
*
* History: 3/3/2005 6:34PM JSY
**************************************************************************************************/
bool TacticalBuildBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
			READ_CHUNK_MLL_PTR_LIST(CHUNK_ID_TACTICAL_BUILD_BLOCK_BUILD_LIST, PendingBuilds);

		case CHUNK_ID_TACTICAL_BUILD_BLOCK_BASE_CLASS:
			ok &= BlockingStatus::Load(script, reader);
			break;

		case CHUNK_ID_TACTICAL_BUILD_BLOCK_DATA:
			while (reader->Open_Micro_Chunk())
			{
				switch (reader->Cur_Micro_Chunk_ID())
				{
					READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_TACTICAL_BUILD_BLOCK_LISTENER_BASE, SignalListenerClass);

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