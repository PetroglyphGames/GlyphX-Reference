// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/GetEvent.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/GetEvent.cpp $
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
#include "LuaScript.h"
#include "GetEvent.h"

PG_IMPLEMENT_RTTI(GetEvent, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_SCRIPT_GETEVENT, GetEvent);



/**
 * Save / Load chunk ids
 */
enum GetEventChunks {
	CHUNK_GETEVENT_QUEUE
};

/**
 * Constructor
 * @since 5/28/2004 12:10:12 PM -- BMH
 */
GetEvent::GetEvent()
{
	LUA_REGISTER_MEMBER_FUNCTION(GetEvent, "Params", &GetEvent::Lua_Params);
	LUA_REGISTER_MEMBER_FUNCTION(GetEvent, "Reset", &GetEvent::Lua_Reset);
}

/**
 * Schedule an event callback in Lua.
 * 
 * @param event     Lua Function pointer that will process the event.
 * @param thread_id Thread id of lua thread that will execute the event.
 * @param params    optional parameters to be passed to the event.
 * 
 * @return true if the event was scheduled.
 * @since 5/28/2004 12:11:49 PM -- BMH
 */
bool GetEvent::Signal_Event(LuaFunction *event, int thread_id, LuaTable *params)
{
	assert(thread_id >= 0);
	if (thread_id < 0)
		return false;

	if (thread_id >= (int)Events.size()) {
		Events.resize(thread_id+1);
	}
	if (thread_id >= (int)Params.size()) {
		Params.resize(thread_id+1);
	}

	ThreadEventQueue &elist = Events[thread_id];
	elist.push_back(event);
	ThreadParamQueue &plist = Params[thread_id];
	plist.push_back(params);
	return true;
}

/**
 * Reset the event and params queue.
 * 
 * @param script lua script.
 * 
 * @return NULL
 * @since 4/24/2005 3:36:46 PM -- BMH
 */
LuaTable* GetEvent::Lua_Reset(LuaScriptClass *, LuaTable *)
{
	Events.resize(0);
	Params.resize(0);
	return NULL;
}

/**
 * Lua function to query any event parameters.
 * 
 * @param script   lua script
 * @param LuaTable lua parameters
 * 
 * @return the parameters for the event.
 * @since 5/28/2004 12:12:43 PM -- BMH
 */
LuaTable* GetEvent::Lua_Params(LuaScriptClass *script, LuaTable *)
{
	if (script->Get_Current_Thread_Id() == -1 || script->Get_Current_Thread_Id() >= (int)Params.size()) {
		return NULL;
	}

	ThreadParamQueue &plist = Params[script->Get_Current_Thread_Id()];
	if (plist.size()) {
		LuaTable *retval = Return_Variable(plist.front());
		plist.pop_front();
		return retval;
	}

	return NULL;
}

/**
 * Get any scheduled events.
 * 
 * @param script   lua script
 * @param LuaTable lua parameters
 * 
 * @return Event callback pointer if one was scheduled.
 * @since 5/28/2004 12:13:33 PM -- BMH
 */
LuaTable* GetEvent::Function_Call(LuaScriptClass *script, LuaTable *)
{
	if (script->Get_Current_Thread_Id() == -1 || script->Get_Current_Thread_Id() >= (int)Events.size()) {
		return NULL;
	}

	ThreadEventQueue &elist = Events[script->Get_Current_Thread_Id()];
	if (elist.size()) {
		LuaTable *retval = Return_Variable(elist.front());
		elist.pop_front();
		return retval;
	}

	return NULL;
}

enum {
	CHUNK_ID_GETEVENT_THREAD_COUNT,
	CHUNK_ID_GETEVENT_THREAD_DATA,
	CHUNK_ID_GETEVENT_THREAD_EVENT_COUNT,
	CHUNK_ID_GETEVENT_THREAD_EVENT_DATA,
	CHUNK_ID_GETEVENT_THREAD_PARAM_COUNT,
	CHUNK_ID_GETEVENT_THREAD_PARAM_DATA,
};

bool GetEvent::Save(LuaScriptClass *script, ChunkWriterClass *writer) 
{
	assert(writer != NULL);
	bool ok = true;
	int thread_count = (int)Events.size();
	assert(Params.size() == Events.size());

	int i = 0;

	ok &= writer->Begin_Chunk(CHUNK_ID_GETEVENT_THREAD_DATA);
		WRITE_MICRO_CHUNK                (CHUNK_ID_GETEVENT_THREAD_COUNT,        thread_count);
		for (i = 0; i < thread_count; i++) {
			int event_count = (int)Events[i].size();
			WRITE_MICRO_CHUNK                (CHUNK_ID_GETEVENT_THREAD_EVENT_COUNT,        event_count);
			int param_count = (int)Params[i].size();
			WRITE_MICRO_CHUNK                (CHUNK_ID_GETEVENT_THREAD_PARAM_COUNT,        param_count);
		}
	ok &= writer->End_Chunk();

	for (i = 0; i < thread_count; i++) {
		int event_count = (int)Events[i].size();
		int t = 0;
		ThreadEventQueueIterator tit = Events[i].begin();
		for (t = 0; t < event_count; t++, tit++) {
			LUA_WRITE_CHUNK_VALUE_PTR(CHUNK_ID_GETEVENT_THREAD_EVENT_DATA, (*tit), script);
		}
		int param_count = (int)Params[i].size();
		ThreadParamQueueIterator pit = Params[i].begin();
		for (t = 0; t < param_count; t++, pit++) {
			LUA_WRITE_CHUNK_VALUE_PTR(CHUNK_ID_GETEVENT_THREAD_PARAM_DATA, (*pit), script);
		}
	}
	return (ok);
}

bool GetEvent::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;
	int thread_count = 0;
	int thread_index = 0;
	int event_count = 0;
	int param_count = 0;
	ThreadEventQueueIterator tit;
	ThreadParamQueueIterator pit;

	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case CHUNK_ID_GETEVENT_THREAD_DATA:
				while (reader->Open_Micro_Chunk()) {
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						case CHUNK_ID_GETEVENT_THREAD_COUNT:
							reader->Read(&thread_count, sizeof(thread_count));
							Params.resize(thread_count);
							Events.resize(thread_count);
							break;
			
						case CHUNK_ID_GETEVENT_THREAD_EVENT_COUNT:
							reader->Read(&event_count, sizeof(event_count));
							Events[thread_index].resize(event_count);
							break;
			
						case CHUNK_ID_GETEVENT_THREAD_PARAM_COUNT:
							reader->Read(&param_count, sizeof(param_count));
							Params[thread_index].resize(param_count);
							break;
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				thread_index = 0;
				if (thread_index < thread_count) {
					tit = Events[thread_index].begin();
					pit = Params[thread_index].begin();
				}
				break;

			case CHUNK_ID_GETEVENT_THREAD_EVENT_DATA:
				ok &= Lua_Load_Variable(reader, (SmartPtr<LuaVar> &)(*tit), script);
				tit++;
				break;

			case CHUNK_ID_GETEVENT_THREAD_PARAM_DATA:
				ok &= Lua_Load_Variable(reader, (SmartPtr<LuaVar> &)(*pit), script);
				pit++;
				break;
			default: assert(false); break;	// Unknown Chunk
		}
		if (thread_index < thread_count && tit == Events[thread_index].end()) {
			thread_index++;
			if (thread_index < thread_count) {
				tit = Events[thread_index].begin();
				pit = Params[thread_index].begin();
			}
		}
		reader->Close_Chunk();
	}

	return (ok);
}

bool GetEvent::Has_Event_Pending(LuaFunction *event, int thread_id)
{
	if (thread_id >= static_cast<int>(Events.size()) || thread_id < 0)
	{
		return false;
	}

	for (ThreadEventQueueIterator i = Events[thread_id].begin(); i != Events[thread_id].end(); ++i)
	{
		if (LuaScriptClass::Compare_Lua_Functions(NULL, *i, event))
		{
			return true;
		}
	}

	return false;
}
