// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/GetEvent.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/GetEvent.h $
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

#ifndef __GETEVENT_H__
#define __GETEVENT_H__

#include "Always.h"
#include "LuaScriptVariable.h"



/**
 * Lua GetEvent.  Handles callback events reported to Lua.
 * @since 5/22/2004 5:01:05 PM -- BMH
 */
class GetEvent : public LuaUserVar
{
public:
	/**
	 * Queue Typedefs
	 */
	typedef std::list<SmartPtr<LuaFunction> > ThreadEventQueue;
	typedef std::list<SmartPtr<LuaFunction> >::iterator ThreadEventQueueIterator;
	typedef std::vector<ThreadEventQueue> EventQueue;

	typedef std::list<SmartPtr<LuaTable> > ThreadParamQueue;
	typedef std::list<SmartPtr<LuaTable> >::iterator ThreadParamQueueIterator;
	typedef std::vector<ThreadParamQueue> ParamQueue;

	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_SCRIPT_GETEVENT, GetEvent);

	GetEvent();

	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *);
	LuaTable* Lua_Params(LuaScriptClass *script, LuaTable *);
	LuaTable* Lua_Reset(LuaScriptClass *script, LuaTable *);
	virtual bool Save(LuaScriptClass *, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *, ChunkReaderClass *reader);

	bool Signal_Event(LuaFunction *event, int thread_id, LuaTable *params);

	bool Has_Event_Pending(LuaFunction *event, int thread_id);

private:
	EventQueue			Events;
	ParamQueue			Params;
};


#endif // __GETEVENT_H__