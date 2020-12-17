// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/StoryPlotWrapper.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/StoryPlotWrapper.cpp $
//
//    Original Author: James Yarrow
//
//            $Author: Brian_Hayes $
//
//            $Change: 641502 $
//
//          $DateTime: 2017/05/09 13:45:27 $
//
//          $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */

#pragma hdrstop

#include "StoryPlotWrapper.h"
#include "StoryEventWrapper.h"
#include "StoryMode/StorySubPlot.h"

PG_IMPLEMENT_RTTI(StoryPlotWrapper, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_STORYPLOT_WRAPPER, StoryPlotWrapper);
MEMORY_POOL_INSTANCE(StoryPlotWrapper, LUA_WRAPPER_POOL_SIZE);

StoryPlotWrapper *StoryPlotWrapper::Create(StorySubPlotClass *plot, LuaScriptClass *script)
{
	FAIL_IF(!plot) { return NULL; }
	FAIL_IF(!script) { return NULL; }

	StoryPlotWrapper *plot_wrapper = (StoryPlotWrapper *)StoryPlotWrapper::FactoryCreate();
	plot_wrapper->Plot = plot;
	plot_wrapper->Script = script;
	return plot_wrapper;
}

StoryPlotWrapper::StoryPlotWrapper() :
	Plot(NULL),
	Script(NULL)
{
	LUA_REGISTER_MEMBER_FUNCTION(StoryPlotWrapper, "Is_Valid", &StoryPlotWrapper::Is_Valid);
	LUA_REGISTER_MEMBER_FUNCTION(StoryPlotWrapper, "Get_Event", &StoryPlotWrapper::Get_Event);
	LUA_REGISTER_MEMBER_FUNCTION(StoryPlotWrapper, "Activate", &StoryPlotWrapper::Activate);
	LUA_REGISTER_MEMBER_FUNCTION(StoryPlotWrapper, "Suspend", &StoryPlotWrapper::Suspend);
	LUA_REGISTER_MEMBER_FUNCTION(StoryPlotWrapper, "Reset", &StoryPlotWrapper::Reset);
}

void StoryPlotWrapper::Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *)
{
	Plot = NULL;
}

LuaTable *StoryPlotWrapper::Get_Event(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Plot) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("StoryPlotWrapper::Get_Event -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> event_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!event_name)
	{
		script->Script_Error("StoryPlotWrapper::Get_Event -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	StoryEventClass *story_event = Plot->Get_Event(event_name->Value.c_str());
	if (!story_event)
	{
		script->Script_Error("StoryPlotWrapper::Get_Event -- cannot find event %s in plot %s.", event_name->Value.c_str(), Plot->Get_Name().c_str());
		return NULL;
	}

	return Return_Variable(StoryEventWrapper::Create(story_event, script));
}

LuaTable *StoryPlotWrapper::Activate(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Plot) { return NULL; }

	Plot->Set_Active(true);

	//Blocking object waiting on plot?
	return NULL;
}

LuaTable *StoryPlotWrapper::Suspend(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Plot) { return NULL; }

	Plot->Set_Active(false);

	return NULL;
}

LuaTable *StoryPlotWrapper::Reset(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Plot) { return NULL; }

	Plot->Reset_All_Events();

	return NULL;
}

enum
{
	SIGNAL_LISTENER_BASE_MICRO_CHUNK,
	PLOT_MICRO_CHUNK,
};

bool StoryPlotWrapper::Save(LuaScriptClass *, ChunkWriterClass *writer)
{
	bool ok = true;
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(SIGNAL_LISTENER_BASE_MICRO_CHUNK, SignalListenerClass);
	WRITE_MICRO_CHUNK_OBJECT_PTR(PLOT_MICRO_CHUNK, Plot);
	return ok;
}

bool StoryPlotWrapper::Load(LuaScriptClass *, ChunkReaderClass *reader)
{
	bool ok = true;
	while (reader->Open_Micro_Chunk())
	{
		switch (reader->Cur_Micro_Chunk_ID())
		{
			READ_MICRO_CHUNK_MULTI_BASE_PTR(SIGNAL_LISTENER_BASE_MICRO_CHUNK, SignalListenerClass);
			READ_MICRO_CHUNK_OBJECT_PTR(PLOT_MICRO_CHUNK, Plot);

		default:
			ok = false;
			assert(false);
			break;
		}

		reader->Close_Micro_Chunk();
	}

	return ok;
}