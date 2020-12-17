// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/UtilityCommands.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/UtilityCommands.cpp $
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

#include "Always.h"

#include "UtilityCommands.h"
#include "GetEvent.h"
#include "LuaScript.h"
#include "LuaNetworkDebugger.h"

/**
 * Prints script messages from lua.
 * @since 4/23/2004 2:36:13 PM -- BMH
 */
class LuaScriptMessage : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		LuaString *str = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (str) 
		{
			script->Script_Message(str->Value.c_str());
		}
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaScriptMessage, LuaUserVar);

class LuaDebugBreak : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *)
	{
		script;
#ifndef NDEBUG
		script->Debug_Attach(false);
		LuaScriptClass::Debug_Set_Script_Context(script);
		script->Debug_Break_Thread(script->Get_Current_Thread_Id());
#endif
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaDebugBreak, LuaUserVar);


static stdext::hash_map<std::string, bool> _write_map;
class LuaCustomScriptMessage : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() != 2)
		{
			script->Script_Error("LuaCustomScriptMessage -- Expected 2 parameters, got %d", params->Value.size());
			return NULL;
		}
		LuaString *filename = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		LuaString *outstr = PG_Dynamic_Cast<LuaString>(params->Value[1]);
		if (!filename || !outstr)
		{
			script->Script_Error("LuaCustomScriptMessage -- expected two string parameters.");
			return NULL;
		}

		std::pair<stdext::hash_map<std::string, bool>::iterator, bool> retval = _write_map.insert(std::make_pair(filename->Value, true));
		if (retval.second)
		{
			DeleteFile(filename->Value.c_str());
		}
		Debug_Print_Alt(filename->Value.c_str(), "%s\n", outstr->Value.c_str());
		script->Script_Message(outstr->Value.c_str());
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaCustomScriptMessage, LuaUserVar);

class DumpCallStack : public LuaUserVar
{
public:
	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *)
	{
		return Return_Variable(new LuaString(script->Debug_Dump_Callstack()));
	}
};

/**
 * Lua ScriptExit command.  Causes the script and all it's threads
 * to terminate.
 * @since 4/23/2004 2:36:53 PM -- BMH
 */
class LuaScriptExit : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable * /*params*/)
	{
		script->Set_Exit();
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaScriptExit, LuaUserVar);

/**
 * Create thread function.  Creates a lua thread and adds it
 * to the service table.
 * @since 8/8/2005 10:03:09 AM -- BMH
 */
class LuaCreateThread : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	LuaCreateThread()
	{
		LUA_REGISTER_MEMBER_FUNCTION(LuaCreateThread, "Create", &LuaCreateThread::Function_Call);
		LUA_REGISTER_MEMBER_FUNCTION(LuaCreateThread, "Get_Current_ID", &LuaCreateThread::Get_Current_ID);
		LUA_REGISTER_MEMBER_FUNCTION(LuaCreateThread, "Get_Name", &LuaCreateThread::Get_Name);
		LUA_REGISTER_MEMBER_FUNCTION(LuaCreateThread, "Kill", &LuaCreateThread::Kill);
		LUA_REGISTER_MEMBER_FUNCTION(LuaCreateThread, "Kill_All", &LuaCreateThread::Kill_All);
		LUA_REGISTER_MEMBER_FUNCTION(LuaCreateThread, "Is_Thread_Active", &LuaCreateThread::Is_Thread_Active);
	}
	virtual LuaTable* Get_Current_ID(LuaScriptClass *script, LuaTable *)
	{
		return Return_Variable(new LuaNumber((float)script->Get_Current_Thread_Id()));
	}
	virtual LuaTable* Get_Name(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaCreateThread::Get_Name -- Expected a thread id parameter.");
			return NULL;
		}
		LuaNumber::Pointer num = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
		if (!num)
		{
			script->Script_Error("LuaCreateThread::Get_Name -- Expected a number for thread id parameter.");
			return NULL;
		}

		const std::string *thread_name = script->Get_Thread_Name((int)num->Value);
		if (!thread_name)
		{
			return NULL;
		}

		return Return_Variable(new LuaString(*thread_name));
	}
	virtual LuaTable* Kill_All(LuaScriptClass *script, LuaTable *)
	{
		for (int i = 0; i < script->Get_Thread_Count(); i++)
		{
			if (i == script->Get_Current_Thread_Id()) continue;

			script->Kill_Thread(i);
		}
		return NULL;
	}
	virtual LuaTable* Kill(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaCreateThread::Kill -- Expected a thread id parameter.");
			return NULL;
		}
		LuaNumber::Pointer num = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
		if (!num)
		{
			script->Script_Error("LuaCreateThread::Kill -- Expected a number for thread id parameter.");
			return NULL;
		}

		FAIL_IF(script->Get_Current_Thread_Id() == (int)num->Value) { return NULL; }

		script->Kill_Thread((int)num->Value);
		return NULL;
	}
	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() < 1)
		{
			script->Script_Error("LuaCreateThread -- Expected a LuaFunction parameter.");
			return NULL;
		}

		LuaString::Pointer func_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (!func_name)
		{
			script->Script_Error("LuaCreateThread -- Expected a string for function name.");
			return NULL;
		}

		SmartPtr<LuaFunction> func = LUA_SAFE_CAST(LuaFunction, script->Map_Global_From_Lua(func_name->Value.c_str()));
		if (!func)
		{
			script->Script_Error("LuaCreateThread -- \"%s\" does not evaluate to a lua function.", func_name->Value.c_str());
			return NULL;
		}

		return Return_Variable(new LuaNumber((float)script->Create_Thread_Function(func_name->Value.c_str(), 
			params->Value.size() > 1 ? params->Value[1] : NULL)));
	}
	virtual LuaTable *Is_Thread_Active(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() != 1)
		{
			script->Script_Error("LuaCreateThread::Is_Thread_Active -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
			return NULL;
		}

		LuaNumber::Pointer lua_thread_id = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
		if (!lua_thread_id)
		{
			script->Script_Error("LuaCreateThread::Is_Thread_Active -- Expected a thread id.");
			return NULL;
		}

		int thread_id = static_cast<int>(lua_thread_id->Value);
		if (thread_id < 0 || thread_id >= script->Get_Thread_Count())
		{
			return Return_Variable(new LuaBool(false));
		}

		return Return_Variable(new LuaBool(script->Get_Thread_Name(thread_id) != NULL));
	}
};
PG_IMPLEMENT_RTTI(LuaCreateThread, LuaUserVar);

class LuaStringCompare : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	virtual LuaTable* Function_Call(LuaScriptClass *, LuaTable *params)
	{
		bool eq = false;
		if (params->Value.size() > 1) {
			LuaString::Pointer str1 = LUA_SAFE_CAST(LuaString, params->Value[0]);
			LuaString::Pointer str2 = LUA_SAFE_CAST(LuaString, params->Value[1]);
			if (str1 && str2) {
				eq = _stricmp(str1->Value.c_str(), str2->Value.c_str()) == 0;
			}
		}
		return Return_Variable(new LuaBool(eq));
	}
};
PG_IMPLEMENT_RTTI(LuaStringCompare, LuaUserVar);

class LuaDebugPrint : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	virtual LuaTable* Function_Call(LuaScriptClass * /*script*/, LuaTable *params)
	{
		LuaString *str = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (str) 
		{
			Debug_Print("%s", str->Value.c_str());
		}
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaDebugPrint, LuaUserVar);

class LuaConsolePrint : public LuaUserVar // Note: alternate syntax is lc(param)
{
public:
	PG_DECLARE_RTTI();
	virtual LuaTable* Function_Call(LuaScriptClass * /*script*/, LuaTable *params)
	{
		char output[256];
		output[0]=0; // terminate the string

		LuaNumber::Pointer floatval = LUA_SAFE_CAST(LuaNumber, params->Value[0]);
		if (floatval) 
		{
			sprintf(output,"type = number, value = %f",floatval->Value);
		}
		else
		{
			LuaString::Pointer strval = LUA_SAFE_CAST(LuaString, params->Value[0]);
			if (strval)
			{
				sprintf(output,"type = string, value = \"%s\"",strval->Value.c_str());
			}
			else
			{
				LuaTable::Pointer tableval = LUA_SAFE_CAST(LuaTable, params->Value[0]);
				if (tableval)
				{
					sprintf(output,"type = table; has %d elements.",tableval->Value.size());
				}
				else
				{
					LuaBool::Pointer boolval = LUA_SAFE_CAST(LuaBool, params->Value[0]);
					if (boolval)
					{
						sprintf(output,"type = bool; value = %s",boolval->Value?"true":"false");
					}
					else
					{
						sprintf(output,"type = ???, cannot be evaluated with lc()");
					}
				}
			}
		}

		LuaNetworkDebuggerClass::Send_Output_To_Debugger(output,LuaNetworkDebuggerClass::DEST_LUA_CONSOLE);
		return NULL;
	}
};
PG_IMPLEMENT_RTTI(LuaConsolePrint, LuaUserVar);


typedef stdext::hash_map<std::string, SmartPtr<LuaVar> > GlobalTableType;
GlobalTableType GlobalTable;

enum {
	CHUNK_ID_GLOBAL_VALUE_DATA,
	CHUNK_ID_GLOBAL_VALUE_KEY,
	CHUNK_ID_GLOBAL_VALUE_VALUE,
};

void Lua_Global_Table_Reset( void )
{
	GlobalTable.clear();
}

bool Lua_Global_Table_Save( ChunkWriterClass *writer )
{
	bool ok = true;

	GlobalTableType::iterator it = GlobalTable.begin();
	for (; it != GlobalTable.end(); it++)
	{
		ok &= writer->Begin_Chunk(CHUNK_ID_GLOBAL_VALUE_DATA);
			WRITE_MICRO_CHUNK_STRING	(	CHUNK_ID_GLOBAL_VALUE_KEY, 	it->first);
		ok &= writer->End_Chunk();

		LUA_WRITE_CHUNK_VALUE_PTR(CHUNK_ID_GLOBAL_VALUE_VALUE, it->second, NULL);
	}

	return ok;
}

bool Lua_Global_Table_Load( ChunkReaderClass *reader )
{
	bool ok = true;
	std::string key_str;

	GlobalTable.clear();

	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case CHUNK_ID_GLOBAL_VALUE_DATA:
				while (reader->Open_Micro_Chunk()) {
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK_STRING	(	CHUNK_ID_GLOBAL_VALUE_KEY, 	key_str);
						default: assert(false); break;   // Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			LUA_READ_CHUNK_VALUE_PTR(CHUNK_ID_GLOBAL_VALUE_VALUE, GlobalTable[key_str], NULL);
			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}
	return ok;
}

class GlobalValue : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	GlobalValue()
	{
		LUA_REGISTER_MEMBER_FUNCTION(GlobalValue, "Get", &GlobalValue::Get);
		LUA_REGISTER_MEMBER_FUNCTION(GlobalValue, "Set", &GlobalValue::Set);
	}

	LuaTable* Get(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() != 1) {
			script->Script_Error("GlobalValue::Get -- Invalid number of parameters %d should be 1", params->Value.size());
			return NULL;
		}
		LuaString *str = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (!str) {
			script->Script_Error("GlobalValue::Get -- Invalid parameter 1, should be a string.");
			return NULL;
		}

		GlobalTableType::iterator it = GlobalTable.find(str->Value);
		if (it == GlobalTable.end())
		{
			return NULL;
		}
		return Return_Variable(it->second);
	}

	LuaTable* Set(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() != 2) {
			script->Script_Error("GlobalValue::Set -- Invalid number of parameters %d should be 2", params->Value.size());
			return NULL;
		}
		LuaString *str = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (!str) {
			script->Script_Error("GlobalValue::Set -- Invalid parameter 1, should be a string.");
			return NULL;
		}
		if (PG_Is_Type<LuaUserVar>(params->Value[1])) {
			script->Script_Error("GlobalValue::Set -- Invalid parameter 2, cannot be a Lua User Variable.");
			return NULL;
		}
		
		GlobalTable[str->Value] = params->Value[1];
		return NULL;
	}

	LuaTable* Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		return Get(script, params);
	}

};
PG_IMPLEMENT_RTTI(GlobalValue, LuaUserVar);



class ThreadValue : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_THREAD_VALUE, ThreadValue);
	ThreadValue()
	{
		LUA_REGISTER_MEMBER_FUNCTION(ThreadValue, "Get", &ThreadValue::Get);
		LUA_REGISTER_MEMBER_FUNCTION(ThreadValue, "Set", &ThreadValue::Set);
		LUA_REGISTER_MEMBER_FUNCTION(ThreadValue, "Reset", &ThreadValue::Reset);
	}
	LuaTable* Get(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() != 1) {
			script->Script_Error("ThreadValue::Get -- Invalid number of parameters %d should be 1", params->Value.size());
			return NULL;
		}
		LuaString *str = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (!str) {
			script->Script_Error("ThreadValue::Get -- Invalid parameter 1, should be a string.");
			return NULL;
		}
		int idx = script->Get_Current_Thread_Id();
		if (idx < 0) return NULL;
		if (idx+1 > (int)ThreadMapVec.size()) ThreadMapVec.resize(idx+1);

		ThreadMapType::iterator it = ThreadMapVec[idx].find(str->Value);
		if (it == ThreadMapVec[idx].end()) return NULL;
		return Return_Variable(it->second);
	}
	LuaTable* Set(LuaScriptClass *script, LuaTable *params)
	{
		if (params->Value.size() != 2) {
			script->Script_Error("ThreadValue::Set -- Invalid number of parameters %d should be 2", params->Value.size());
			return NULL;
		}
		LuaString *str = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (!str) {
			script->Script_Error("ThreadValue::Set -- Invalid parameter 1, should be a string.");
			return NULL;
		}

		int idx = script->Get_Current_Thread_Id();
		if (idx < 0) return NULL;
		if (idx+1 > (int)ThreadMapVec.size()) ThreadMapVec.resize(idx+1);
      
		(ThreadMapVec[idx])[str->Value.c_str()] = params->Value[1];
		return NULL;
	}
	LuaTable* Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		return Get(script, params);
	}
	LuaTable* Reset(LuaScriptClass *, LuaTable *)
	{
		ThreadMapVec.clear();
		return NULL;
	}

	enum {
		CHUNK_ID_THREAD_VALUE_DATA,
		CHUNK_ID_THREAD_VALUE_COUNT,
		CHUNK_ID_THREAD_VALUE_VARIABLE_TABLE,
		CHUNK_ID_THREAD_VALUE_VARIABLE_KEY,
		CHUNK_ID_THREAD_VALUE_VARIABLE_VALUE,
	};
	
	bool Save( LuaScriptClass *script, ChunkWriterClass *writer )
	{
		bool ok = true;
		int i = 0;
	
		int thread_count = (int)ThreadMapVec.size();
		if (thread_count) {
			ok &= writer->Begin_Chunk(CHUNK_ID_THREAD_VALUE_DATA);
				WRITE_MICRO_CHUNK				(	CHUNK_ID_THREAD_VALUE_COUNT, thread_count);
			ok &= writer->End_Chunk();
	
			for (i = 0; i < thread_count; i++) {
				ThreadMapType &tab = ThreadMapVec[i];

				ok &= writer->Begin_Chunk(CHUNK_ID_THREAD_VALUE_VARIABLE_TABLE);
				ThreadMapIterator it = tab.begin();
				for (; it != tab.end(); it++) {
					WRITE_MICRO_CHUNK_STRING	(	CHUNK_ID_THREAD_VALUE_VARIABLE_KEY, it->first);
				}
				ok &= writer->End_Chunk();

				it = tab.begin();
				for (; it != tab.end(); it++) {
					LUA_WRITE_CHUNK_VALUE_PTR	(	CHUNK_ID_THREAD_VALUE_VARIABLE_VALUE, it->second, script);
				}
			}
		}
		return ok;
	}
	
	bool Load( LuaScriptClass *script, ChunkReaderClass *reader )
	{
		bool ok = true;
		int thread_count = 0;
		int thread_index = -1;

		ThreadMapVec.clear();
		ThreadMapIterator map_it;
	
		while (reader->Open_Chunk()) {
			switch ( reader->Cur_Chunk_ID() )
			{
				case CHUNK_ID_THREAD_VALUE_DATA:
					while (reader->Open_Micro_Chunk()) {
						switch ( reader->Cur_Micro_Chunk_ID() )
						{
							READ_MICRO_CHUNK				(	CHUNK_ID_THREAD_VALUE_COUNT, thread_count);
							default: assert(false); break;   // Unknown Chunk
						}
						reader->Close_Micro_Chunk();
					}
					ThreadMapVec.resize(thread_count);
					break;
	
	
				case CHUNK_ID_THREAD_VALUE_VARIABLE_TABLE:
					{
						thread_index++;
						std::string key_str;
						while (reader->Open_Micro_Chunk()) {
							switch ( reader->Cur_Micro_Chunk_ID() )
							{
								READ_MICRO_CHUNK_STRING	(	CHUNK_ID_THREAD_VALUE_VARIABLE_KEY, key_str);
								default: assert(false); break;   // Unknown Chunk
							}
							ThreadMapVec[thread_index][key_str] = LuaVar::Pointer();
							reader->Close_Micro_Chunk();
						}
						map_it = ThreadMapVec[thread_index].begin();
						break;
					}

				LUA_READ_CHUNK_VALUE_PTR	(	CHUNK_ID_THREAD_VALUE_VARIABLE_VALUE, (map_it++)->second, script);

				default: assert(false); break;	// Unknown Chunk
			}
			reader->Close_Chunk();
		}
		return ok;
	}

private:
	typedef stdext::hash_map<std::string, SmartPtr<LuaVar> > ThreadMapType;
	typedef stdext::hash_map<std::string, SmartPtr<LuaVar> >::iterator ThreadMapIterator;
	std::vector<ThreadMapType>		ThreadMapVec;

};
PG_IMPLEMENT_RTTI(ThreadValue, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_THREAD_VALUE, ThreadValue);

class LuaMessagePopup : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *, LuaTable *params)
	{
#ifndef NDEBUG
		LuaString *str = PG_Dynamic_Cast<LuaString>(params->Value[0]);
		if (str)
		{
			Message_Popup(str->Value.c_str());
		}
#else
		params;
#endif

		return 0;
	}
};
PG_IMPLEMENT_RTTI(LuaMessagePopup, LuaUserVar);

class LuaGetThreadID : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *)
	{
		return Return_Variable(new LuaNumber(static_cast<float>(script->Get_Current_Thread_Id())));
	}
};
PG_IMPLEMENT_RTTI(LuaGetThreadID, LuaUserVar);

void UtilityCommandsClass::Register_Commands(LuaScriptClass *script)
{
	if (script->Pool_Is_Fresh_Load())
	{
		script->Map_Global_To_Lua(new LuaStringCompare(), "StringCompare");
		script->Map_Global_To_Lua(new LuaScriptExit(), "_ScriptExit");
		script->Map_Global_To_Lua(new LuaScriptMessage(), "_ScriptMessage");
		script->Map_Global_To_Lua(new LuaDebugBreak(), "_DebugBreak");
		script->Map_Global_To_Lua(new LuaCustomScriptMessage(), "_CustomScriptMessage");
		script->Map_Global_To_Lua(ThreadValue::FactoryCreate(), "ThreadValue");
		script->Map_Global_To_Lua(new GlobalValue(), "GlobalValue");
		script->Map_Global_To_Lua(new LuaMessagePopup(), "_MessagePopup");
		script->Map_Global_To_Lua(new LuaDebugPrint(), "_OuputDebug");
		script->Map_Global_To_Lua(new LuaGetThreadID(), "GetThreadID");
		script->Map_Global_To_Lua(new DumpCallStack(), "DumpCallStack");
		script->Map_Global_To_Lua(new LuaCreateThread(), "Create_Thread");
		script->Map_Global_To_Lua(new LuaCreateThread(), "Thread");
		script->Map_Global_To_Lua(new LuaConsolePrint(), "lc");

		LuaUserVar *hand = GetEvent::FactoryCreate();
		script->Map_Global_To_Lua(hand, "GetEvent");
		script->Set_Thread_Event_Handler((GetEvent *)hand);
	}
}
