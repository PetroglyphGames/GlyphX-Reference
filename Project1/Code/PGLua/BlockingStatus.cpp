// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/BlockingStatus.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/BlockingStatus.cpp $
//
//    Original Author: Brian Hayes
//
//            $Author: Brian_Hayes $
//
//            $Change: 641585 $
//
//          $DateTime: 2017/05/10 10:42:50 $
//
//          $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */

#pragma hdrstop
#include "BlockingStatus.h"

PG_IMPLEMENT_RTTI(BlockingStatus, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_NEVER_BLOCK, BlockingStatus);

/**
 * Constructor
 * 
 * @param command Command for this blocking status.
 * @since 4/22/2004 2:01:58 PM -- BMH
 */
BlockingStatus::BlockingStatus()
{
	LUA_REGISTER_MEMBER_FUNCTION(BlockingStatus, "IsFinished", &BlockingStatus::Is_Finished);
	LUA_REGISTER_MEMBER_FUNCTION(BlockingStatus, "Result", &BlockingStatus::Result);
}

/**
 * Default IsFinished virtual function.
 * 
 * @param script script for this object
 * @param params params for this object
 * 
 * @return just return true to signal the command is finished.
 * @since 4/22/2004 2:00:21 PM -- BMH
 */
LuaTable* BlockingStatus::Is_Finished(LuaScriptClass * /*script*/, LuaTable * /*params*/)
{
	return Return_Variable(new LuaBool(true));
}

/**
 * Default Result virtual function.
 * 
 * @param script script for this object
 * @param params params for this object
 * 
 * @return just return true as a result.
 * @since 4/27/2004 2:20:32 PM -- BMH
 */
LuaTable* BlockingStatus::Result(LuaScriptClass * /*script*/, LuaTable * /*params*/)
{
	return Return_Variable(new LuaBool(true));
}

/**
 * Get the command object associated with this blocking status object.
 * 
 * @return Command object for this blocking status object.
 * @since 4/22/2004 2:01:26 PM -- BMH
 */
LuaUserVar *BlockingStatus::Get_Command(void) const
{
	return Command;
}

/**
 * Set the command associated with this BlockingStatus
 * 
 * @param command Command value
 * @since 4/22/2004 6:58:04 PM -- BMH
 */
void BlockingStatus::Set_Command(LuaUserVar *command)
{
	Command = command;
}

enum {
	CHUNK_ID_BLOCKING_STATUS_COMMAND,
};

bool BlockingStatus::Save(LuaScriptClass * script, ChunkWriterClass * writer)
{
	bool ok = true;
	LUA_WRITE_CHUNK_VALUE_PTR	(	CHUNK_ID_BLOCKING_STATUS_COMMAND, Command, script);
	return ok;
}

bool BlockingStatus::Load(LuaScriptClass * script, ChunkReaderClass * reader)
{
	bool ok = true;
	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			LUA_READ_CHUNK_VALUE_PTR	(	CHUNK_ID_BLOCKING_STATUS_COMMAND, Command, script);
			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}
	return (ok);
}
