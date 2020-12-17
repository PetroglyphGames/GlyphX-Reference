// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/StoryEventWrapper.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/StoryEventWrapper.h $
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

#ifndef _STORY_EVENT_WRAPPER_H_
#define _STORY_EVENT_WRAPPER_H_

#include "LuaRTSUtilities.h"
#include "PGSignal/SignalListener.h"

class StoryEventWrapper : public LuaUserVar, public SignalListenerClass, public PooledObjectClass<StoryEventWrapper, LUA_WRAPPER_POOL_SIZE>
{
public:

	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_STORYEVENT_WRAPPER, StoryEventWrapper);

	StoryEventWrapper();

	static StoryEventWrapper *Create(StoryEventClass *event, LuaScriptClass *script);

	LuaTable *Set_Event_Parameter(LuaScriptClass *script, LuaTable *params);
	LuaTable *Set_Reward_Parameter(LuaScriptClass *script, LuaTable *params);
	LuaTable *Add_Dialog_Text(LuaScriptClass *script, LuaTable *params);
	LuaTable *Clear_Dialog_Text(LuaScriptClass *script, LuaTable *params);
	LuaTable *Set_Dialog(LuaScriptClass *script, LuaTable *params);
	LuaTable *Set_Reward_Type(LuaScriptClass *script, LuaTable *params);

	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *);

	LuaTable *Is_Valid(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(Event != NULL) ); }

	virtual bool Save(LuaScriptClass *, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *, ChunkReaderClass *reader);

	virtual LuaTable *Is_Pool_Safe(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(false)); }


private:

	void Extract_String_Parameters(std::vector< SmartPtr<LuaVar> >::iterator begin, 
												std::vector< SmartPtr<LuaVar> >::iterator end, 
												std::vector<std::string> &parameters);

	StoryEventClass *Event;
	LuaScriptClass *Script;
};

#endif //_STORY_EVENT_WRAPPER_H_