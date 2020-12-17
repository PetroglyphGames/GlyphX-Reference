// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/BlockingStatus.h#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/BlockingStatus.h $
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

#ifndef __BLOCKINGSTATUS_H__
#define __BLOCKINGSTATUS_H__

#include "LuaScriptVariable.h"

/**
 * Base class of a Blocking Status.  BlockingStatus objects are returned
 * from calls that block.  The BlockingStatus object can then be checked
 * later to determine if the blocking call has completed or not.
 * @since 4/22/2004 1:58:11 PM -- BMH
 */
class BlockingStatus : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_NEVER_BLOCK, BlockingStatus);
	BlockingStatus();

	LuaUserVar *Get_Command(void) const;
	void Set_Command(LuaUserVar *command);
	virtual LuaTable* Is_Finished(LuaScriptClass *script, LuaTable *params);
	virtual LuaTable* Result(LuaScriptClass *, LuaTable *);
	virtual bool Save(LuaScriptClass * /*script*/, ChunkWriterClass * /*writer*/);
	virtual bool Load(LuaScriptClass * /*script*/, ChunkReaderClass * /*reader*/);

	virtual LuaTable *Is_Pool_Safe(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(false)); }

private:
	SmartPtr<LuaUserVar>				Command;
};

#endif // __BLOCKINGSTATUS_H__


