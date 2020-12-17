// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ProduceForce.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ProduceForce.cpp $
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
#include "ProduceForce.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "GameObject.h"
#include "PGSignal/SignalDispatcher.h"
#include "AI/Execution/AIBuildTask.h"
#include "SaveLoad.h"
#include "AI/Planning/TaskForce.h"
#include "AI/Planning/PlanBehavior.h"
#include "AI/Planning/AIPlanningSystem.h"
#include "TacticalAIManager.h"
#include "AI/Goal/InstantiatedGoal.h"

/**
 * Keeps track of the progress of a ProduceForce Command.
 * @since 4/29/2004 6:33:23 PM -- BMH
 */
ProduceForceBlockStatus::ProduceForceBlockStatus()
{
}

ProduceForceBlockStatus::~ProduceForceBlockStatus()
{
	for (unsigned int i = 0; i < TaskList.size(); ++i)
	{
		TaskList[i]->Set_Task_As_Finished();
	}
}

/**
 * Init the BlockingStatus object.
 * 
 * @param command Command that spawned the blocking status.
 * @param fleet
 * @since 4/29/2004 6:34:04 PM -- BMH
 */
void ProduceForceBlockStatus::Init(LuaUserVar *command)
{
	Set_Command(command);
}

LuaTable* ProduceForceBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(Internal_Is_Finished()));
}

bool ProduceForceBlockStatus::Internal_Is_Finished(void)
{
	for (int i = 0; i < (int) TaskList.size(); i++) {
		AIBuildTaskClass *task = TaskList[i];
		if (task->Is_Task_Finished()) {
			TaskList.erase(TaskList.begin()+i);
			i--; continue;
		}
	}

	if (TaskList.size() == 0) {
		return true;
	}

	return false;
}

void ProduceForceBlockStatus::Add_Build_Task(AIBuildTaskClass *task)
{
	TaskList.push_back(task);
}

enum {
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_PRODUCEFORCE_DATA,
	CHUNK_ID_PRODUCEFORCE_TASK_IDS,
	CHUNK_ID_PRODUCEFORCE_LISTENER_BASE_ID,
};

bool ProduceForceBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;

	//Clean out finished build tasks before the save, since it may be that no-one else is going
	//to save them for us.
	for (int i = 0; i < (int) TaskList.size(); i++) {
		AIBuildTaskClass *task = TaskList[i];
		if (task->Is_Task_Finished()) {
			TaskList.erase(TaskList.begin()+i);
			i--; continue;
		}
	}

	ok &= writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
		ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	WRITE_SMART_POINTER_STL_VECTOR(CHUNK_ID_PRODUCEFORCE_TASK_IDS, TaskList);
	ok &= writer->Begin_Chunk(CHUNK_ID_PRODUCEFORCE_DATA);
		WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_PRODUCEFORCE_LISTENER_BASE_ID, SignalListenerClass);
	ok &= writer->End_Chunk();

	return (ok);
}

bool ProduceForceBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;
	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case CHUNK_ID_BASE_CLASS:
				ok &= BlockingStatus::Load(script, reader);
				break;

			READ_SMART_POINTER_STL_VECTOR(CHUNK_ID_PRODUCEFORCE_TASK_IDS, TaskList);

			case CHUNK_ID_PRODUCEFORCE_DATA:
				while (reader->Open_Micro_Chunk()) {
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_PRODUCEFORCE_LISTENER_BASE_ID, SignalListenerClass);
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

LUA_IMPLEMENT_FACTORY(LUA_CHUNK_PRODUCE_FORCE_BLOCK, ProduceForceBlockStatus);
PG_IMPLEMENT_RTTI(ProduceForceBlockStatus, BlockingStatus);



