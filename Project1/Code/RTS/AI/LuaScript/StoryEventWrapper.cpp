// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/StoryEventWrapper.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/StoryEventWrapper.cpp $
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

#include "StoryEventWrapper.h"

#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/GameObjectTypeWrapper.h"
#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "StoryMode/StoryEvent.h"
#include "GameText.h"
#include "StoryDialogManager.h"

PG_IMPLEMENT_RTTI(StoryEventWrapper, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_STORYEVENT_WRAPPER, StoryEventWrapper);
MEMORY_POOL_INSTANCE(StoryEventWrapper, LUA_WRAPPER_POOL_SIZE);

StoryEventWrapper::StoryEventWrapper() :
	Event(NULL),
	Script(NULL)
{
	LUA_REGISTER_MEMBER_FUNCTION(StoryEventWrapper, "Is_Valid", &StoryEventWrapper::Is_Valid);
	LUA_REGISTER_MEMBER_FUNCTION(StoryEventWrapper, "Set_Event_Parameter", &StoryEventWrapper::Set_Event_Parameter);
	LUA_REGISTER_MEMBER_FUNCTION(StoryEventWrapper, "Set_Reward_Parameter", &StoryEventWrapper::Set_Reward_Parameter);
	LUA_REGISTER_MEMBER_FUNCTION(StoryEventWrapper, "Add_Dialog_Text", &StoryEventWrapper::Add_Dialog_Text);
	LUA_REGISTER_MEMBER_FUNCTION(StoryEventWrapper, "Clear_Dialog_Text", &StoryEventWrapper::Clear_Dialog_Text);
	LUA_REGISTER_MEMBER_FUNCTION(StoryEventWrapper, "Set_Dialog", &StoryEventWrapper::Set_Dialog);
	LUA_REGISTER_MEMBER_FUNCTION(StoryEventWrapper, "Set_Reward_Type", &StoryEventWrapper::Set_Reward_Type);
}

StoryEventWrapper *StoryEventWrapper::Create(StoryEventClass *event, LuaScriptClass *script)
{
	FAIL_IF(!event) { return NULL; }
	FAIL_IF(!script) { return NULL; }

	StoryEventWrapper *event_wrapper = (StoryEventWrapper *)StoryEventWrapper::FactoryCreate();
	event_wrapper->Event = event;
	event_wrapper->Script = script;
	return event_wrapper;
}

void StoryEventWrapper::Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *)
{
	Event = NULL;
}

LuaTable *StoryEventWrapper::Set_Event_Parameter(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Event) { return NULL; }

	if (params->Value.size() < 2)
	{
		script->Script_Error("StoryEventWrapper::Set_Event_Parameter -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaNumber> lua_event_param_index = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!lua_event_param_index)
	{
		script->Script_Error("StoryEventWrapper::Set_Event_Parameter -- invalid type for parameter 1. Expected number.");
		return NULL;
	}
	int event_param_index = static_cast<int>(lua_event_param_index->Value + 0.5f);

	static std::vector<std::string> event_parameters;
	event_parameters.resize(0);
	Extract_String_Parameters(params->Value.begin() + 1, params->Value.end(), event_parameters);

	Event->Set_Param(event_param_index, &event_parameters);
	return NULL;
}

void StoryEventWrapper::Extract_String_Parameters(std::vector< SmartPtr<LuaVar> >::iterator begin, 
																  std::vector< SmartPtr<LuaVar> >::iterator end, 
																  std::vector<std::string> &parameters)
{
	for (std::vector< SmartPtr<LuaVar> >::iterator i = begin; i != end; ++i)
	{
		SmartPtr<GameObjectWrapper> object_param = PG_Dynamic_Cast<GameObjectWrapper>(*i);
		SmartPtr<GameObjectTypeWrapper> object_type_param = PG_Dynamic_Cast<GameObjectTypeWrapper>(*i);
		SmartPtr<LuaString> string_param = PG_Dynamic_Cast<LuaString>(*i);
		SmartPtr<LuaNumber> number_param = PG_Dynamic_Cast<LuaNumber>(*i);

		if (object_param)
		{
			FAIL_IF(!object_param->Get_Object()) { continue; }
			parameters.push_back(*object_param->Get_Object()->Get_Original_Object_Type()->Get_Name());
		}
		else if (object_type_param)
		{
			FAIL_IF(!object_type_param->Get_Object()) { continue; }
			parameters.push_back(*object_type_param->Get_Object()->Get_Name());
		}
		else if (string_param)
		{
			parameters.push_back(string_param->Value);
		}
		else if (number_param)
		{
			parameters.push_back(std::string());
			String_Printf(parameters.back(), "%d", static_cast<int>(number_param->Value + 0.5f));
		}
		else
		{
			assert(false);
			continue;
		}
	}
}

LuaTable *StoryEventWrapper::Set_Reward_Parameter(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Event) { return NULL; }

	if (params->Value.size() != 2)
	{
		script->Script_Error("StoryEventWrapper::Set_Reward_Parameter -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaNumber> lua_event_param_index = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	if (!lua_event_param_index)
	{
		script->Script_Error("StoryEventWrapper::Set_Reward_Parameter -- invalid type for parameter 1. Expected number.");
		return NULL;
	}
	int event_param_index = static_cast<int>(lua_event_param_index->Value + 0.5f);

	static std::vector<std::string> event_parameters;
	event_parameters.resize(0);
	Extract_String_Parameters(params->Value.begin() + 1, params->Value.end(), event_parameters);

	if (event_parameters.size() != 1)
	{
		script->Script_Error("StoryEventWrapper::Set_Reward_Parameter -- must set exactly one parameter per parameter index.");
		return NULL;
	}

	Event->Set_Reward_Param(event_param_index, &event_parameters[0]);

	return NULL;
}

LuaTable *StoryEventWrapper::Clear_Dialog_Text(LuaScriptClass *, LuaTable *)
{
	FAIL_IF(!Event) { return NULL; }

	std::vector<std::wstring> *extra_text = Event->Get_Story_Dialog_Extra_Text();
	if (extra_text)
	{
		extra_text->resize(0);
	}

	return NULL;
}

LuaTable *StoryEventWrapper::Add_Dialog_Text(LuaScriptClass *script, LuaTable *params)
{
	const static int MAX_DYNAMIC_STRINGS = 2;

	FAIL_IF(!Event) { return NULL; }

	if (params->Value.size() > MAX_DYNAMIC_STRINGS + 1)
	{
		script->Script_Error("StoryEventWrapper::Add_Dialog_Text -- invalid number of parameters.  Expected 2.");
		return NULL;
	}

	SmartPtr<LuaString> text_id = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!text_id)
	{
		script->Script_Error("StoryEventWrapper::Add_Dialog_Text -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	std::wstring display_string;
	std::wstring dynamic_strings[MAX_DYNAMIC_STRINGS];

	int dynamic_string_count = static_cast<int>(params->Value.size()) - 1;
	for (int i = 0; i < dynamic_string_count; ++i)
	{
		SmartPtr<GameObjectWrapper> object_param = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[i + 1]);
		SmartPtr<GameObjectTypeWrapper> object_type_param = PG_Dynamic_Cast<GameObjectTypeWrapper>(params->Value[i + 1]);
		SmartPtr<LuaString> string_param = PG_Dynamic_Cast<LuaString>(params->Value[i + 1]);
		SmartPtr<LuaNumber> number_param = PG_Dynamic_Cast<LuaNumber>(params->Value[i + 1]);

		if (object_param)
		{
			FAIL_IF(!object_param->Get_Object()) { return NULL; }
			dynamic_strings[i] = object_param->Get_Object()->Get_Original_Object_Type()->Get_Display_Name();
		}
		else if (object_type_param)
		{
			FAIL_IF(!object_type_param->Get_Object()) { return NULL; }
			dynamic_strings[i] = object_type_param->Get_Object()->Get_Display_Name();
		}
		else if (string_param)
		{
			TheGameText.Get(string_param->Value, dynamic_strings[i]);
		}
		else if (number_param)
		{
			String_Printf(dynamic_strings[i], L"%d", static_cast<int>(number_param->Value + 0.5f));
		}
		else
		{
			script->Script_Error("StoryEventWrapper::Add_Dialog_Text -- invalid type for parameter %d.", i + 1);
			continue;
		}
	}

	String_Printf(display_string, TheGameText.Get(text_id->Value).c_str(), 
												dynamic_strings[0].c_str(),
												dynamic_strings[1].c_str());

	std::vector<std::wstring> *extra_text = Event->Get_Story_Dialog_Extra_Text();
	FAIL_IF(!extra_text) { return NULL; }
	extra_text->push_back(display_string);

	return NULL;
}

LuaTable *StoryEventWrapper::Set_Dialog(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Event) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("StoryEventWrapper::Set_Dialog -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> dialog_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!dialog_name)
	{
		script->Script_Error("StoryEventWrapper::Set_Dialog -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	Event->Set_Story_Dialog(&dialog_name->Value);
	StoryDialogManagerClass::Add_Story(dialog_name->Value.c_str());

	return NULL;
}

LuaTable *StoryEventWrapper::Set_Reward_Type(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Event) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("StoryEventWrapper::Set_Reward_Type -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> reward_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!reward_name)
	{
		script->Script_Error("StoryEventWrapper::Set_Reward_Type -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	StoryRewardEnum reward_type = (StoryRewardEnum)StoryEventClass::Lookup_Enum(reward_name->Value.c_str());

	Event->Set_Reward_Type(reward_type);

	return NULL;
}

enum
{
	SIGNAL_LISTENER_BASE_MICRO_CHUNK,
	EVENT_MICRO_CHUNK,
};

bool StoryEventWrapper::Save(LuaScriptClass *, ChunkWriterClass *writer)
{
	bool ok = true;
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(SIGNAL_LISTENER_BASE_MICRO_CHUNK, SignalListenerClass);
	WRITE_MICRO_CHUNK_OBJECT_PTR(EVENT_MICRO_CHUNK, Event);
	return ok;
}

bool StoryEventWrapper::Load(LuaScriptClass *, ChunkReaderClass *reader)
{
	bool ok = true;
	while (reader->Open_Micro_Chunk())
	{
		switch (reader->Cur_Micro_Chunk_ID())
		{
			READ_MICRO_CHUNK_MULTI_BASE_PTR(SIGNAL_LISTENER_BASE_MICRO_CHUNK, SignalListenerClass);
			READ_MICRO_CHUNK_OBJECT_PTR(EVENT_MICRO_CHUNK, Event);

		default:
			ok = false;
			assert(false);
			break;
		}

		reader->Close_Micro_Chunk();
	}

	return ok;
}