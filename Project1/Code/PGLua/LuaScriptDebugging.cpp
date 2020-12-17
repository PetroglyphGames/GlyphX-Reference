// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScriptDebugging.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScriptDebugging.cpp $
//
//    Original Author: Brian Hayes
//
//            $Author: Steve_Tall $
//
//            $Change: 747006 $
//
//          $DateTime: 2020/10/20 16:15:42 $
//
//          $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */

#pragma hdrstop

#include "Always.h"

#include "LuaScript.h"
#include "LuaScriptVariable.h"
#include "SaveLoad.h"
#include "Text.h"
#include "MegaFileManager.h"
#include "UtilityCommands.h"
#include <algorithm>

extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

SmartPtr<LuaDebugCallbackClass>						LuaScriptClass::LuaDebugCallbacks;
bool															LuaScriptClass::DebugShouldAttachAll = false;


int LuaScriptClass::Debug_Get_Callstack_Depth(int thread_id)
{
	lua_Debug dbg;
	lua_State *L = State;
	if (thread_id != -1) {
		L = ThreadData[thread_id].Thread;
	}
	if (!L)
	{
		return -1;
	}

	int i = 0;
	int status = 1;
	for (; status == 1; i++) {
		status = lua_getstack(L, i, &dbg);
	}
	return i-1;
}

int LuaScriptClass::Debug_Get_Current_Top_Most_Line_Number(int thread_id)
{
	int s = Debug_Get_Callstack_Depth(thread_id);
	if (s > 0) {
		return Debug_Get_Current_Line_For_Callstack_Level(thread_id, s-1);
	}
	return -1;
}

int LuaScriptClass::Debug_Get_Current_Line_For_Callstack_Level(int thread_id, int level)
{
	lua_Debug dbg;
	lua_State *L = State;
	if (thread_id != -1) {
		L = ThreadData[thread_id].Thread;
	}
	int status = lua_getstack(L, level, &dbg);
	if (status != 1) return -1;

	status = lua_getinfo(L, "l", &dbg);
	if (status != 1) return -1;

	return dbg.currentline;
}
#if (0)
const char *LuaScriptClass::Debug_Get_Name_For_Callstack_Level(int thread_id, int level)
{
	lua_Debug dbg;
	lua_State *L = State;
	if (thread_id != -1) {
		L = ThreadData[thread_id].Thread;
	}
	int status = lua_getstack(L, level, &dbg);
	if (status != 1) return NULL;

	status = lua_getinfo(L, "S", &dbg);
	if (status != 1) return NULL;

	return &dbg.short_src[0];
}
#endif
const std::string & LuaScriptClass::Debug_Dump_Callstack(void)
{
	static std::string outstr;
	outstr.resize(0);

	std::string tstr;

	if (State) {
		outstr = "LuaScriptThread: Main State\n";
		Debug_Print_Current_Callstack(State, tstr);
		outstr += tstr;
	}

	for (int i = 0; i < (int)ThreadData.size(); i++) {
		if (ThreadData[i].Thread) {
			outstr += "LuaScriptThread: " + ThreadData[i].Thread_Name + "\n";
			Debug_Print_Current_Callstack(ThreadData[i].Thread, tstr);
			outstr += tstr;
		}
	}
	return outstr;
}

void LuaScriptClass::Debug_Print_Current_Callstack(lua_State *L, std::string &outstr)
{
	static lua_Debug dbg;
	std::string tstr;
	outstr.resize(0);

	int i = 0;
	int status = 1;
	for (; status == 1; i++) {
		status = lua_getstack(L, i, &dbg);
	}
	int sdepth = i-1;

	for (i = 1; i <= sdepth; i++) {
		status = lua_getstack(L, i, &dbg);
		if (status != 1) break;

		status = lua_getinfo(L, "lSn", &dbg);
		if (status != 1) break;

		String_Printf(tstr, "LuaScript::CALLSTACK -- %2d: [%s:%s] %s:%d\n", i, dbg.namewhat, dbg.name, 
						  dbg.source ? dbg.source+1 : dbg.source, dbg.currentline);
		outstr += tstr;
	}
}

void LuaScriptClass::Debug_Print_Current_Callstack(lua_State *L, bool warning)
{
	static lua_Debug dbg;

	int i = 0;
	int status = 1;
	for (; status == 1; i++) {
		status = lua_getstack(L, i, &dbg);
	}
	int sdepth = i-1;

	for (i = 1; i <= sdepth; i++) {
		status = lua_getstack(L, i, &dbg);
		if (status != 1) break;

		status = lua_getinfo(L, "lSn", &dbg);
		if (status != 1) break;

		std::string callstack_message;
		String_Printf(callstack_message, "LuaScript::CALLSTACK -- %2d: [%s:%s] %s:%d", i, dbg.namewhat, dbg.name, dbg.source ? dbg.source+1 : dbg.source, dbg.currentline);
		if (warning) {
			if (LogWarningCallback)
			{
				LogWarningCallback(callstack_message.c_str());
			}
		} else {
			if (LogErrorCallback)
			{
				LogErrorCallback(callstack_message.c_str());
			}
		}
	}
}

/**
 * Network Debugging support.
 * 2/9/2006 9:13:46 PM -- BMH
 */

struct DebugBreakpointStruct
{
	int					ScriptID;
	int					ThreadID;
	std::string			FileName;
	int					Line;
	std::string			Condition;
};

struct DebugContextStruct : public RefCountClass
{
	std::string CurrentContext;
};

enum DebugBreakTypeEnum
{
	DEBUG_BREAK_NONE,
	DEBUG_BREAK_ALL,
	DEBUG_BREAK_THREAD,
	DEBUG_STEP_IN_TO,
	DEBUG_STEP_OVER,
	DEBUG_STEP_OUT,
};

// static LuaScriptClass members.
typedef std::vector<DebugBreakpointStruct> DebugBreakpointListType;
typedef std::vector<SmartPtr<DebugContextStruct> > DebugContextStackType;
typedef stdext::hash_map<std::string, int> LocalsTableType;
DebugBreakpointListType 			DebugBreakpointList;
DebugContextStackType 				DebugContextStack;
bool										DebugIsSuspended = false;
DebugBreakTypeEnum					DebugBreakType = DEBUG_BREAK_NONE;
int										DebugContextThreadID = -1;
LuaScriptClass							*DebugContextScript = NULL;
int 										DebugCurrentStackContext = 0;
LocalsTableType						LocalsTable;

void LuaScriptClass::Debug_On_Debugger_Connect(void)
{
	if (LuaDebugCallbacks && DebugContextScript && DebugIsSuspended)
	{
		LuaDebugCallbacks->Script_Suspended(DebugContextScript);
	}
}

void LuaScriptClass::Reset_Debugging(void)
{
	DebugBreakpointList.clear();
	DebugContextStack.clear();
	DebugIsSuspended = false;
	DebugCurrentStackContext = 0;
	DebugBreakType = DEBUG_BREAK_NONE;
	DebugContextThreadID = -1;
	DebugContextScript = NULL;

	ActiveScriptListType::iterator it = ActiveScriptList.begin();
	for (; it != ActiveScriptList.end(); it++)
	{
		it->second->Debug_Detach();
	}
}

// static function
void LuaScriptClass::Debug_Add_Breakpoint(LuaScriptClass *script, int thread_id, const std::string & filename, int line, const std::string &condition)
{
	if (thread_id != -1)
	{
		FAIL_IF(!script) { return; }
		FAIL_IF(thread_id < 0 || thread_id >= script->Get_Thread_Count()) { return; }
		FAIL_IF(script->ThreadData[thread_id].Thread == NULL) { return; }
	}

	if (script == NULL)
	{
		Debug_Enable_Attach_All();
	}

	DebugBreakpointList.resize(DebugBreakpointList.size() + 1);
	DebugBreakpointList.back().ScriptID = script ? script->Get_Script_ID() : -1;
	DebugBreakpointList.back().ThreadID = thread_id;
	DebugBreakpointList.back().FileName = filename;
	DebugBreakpointList.back().Condition = condition;
	DebugBreakpointList.back().Line = line;
}

void LuaScriptClass::Debug_Remove_Breakpoint(LuaScriptClass *script, int thread_id, const std::string & filename, int line)
{
	if (thread_id != -1)
	{
		FAIL_IF(!script) { return; }
		FAIL_IF(thread_id < 0 || thread_id >= script->Get_Thread_Count()) { return; }
		FAIL_IF(script->ThreadData[thread_id].Thread == NULL) { return; }
	}

	int script_id = script ? script->Get_Script_ID() : -1;

	bool global_break_point = false;
	for (int i = 0; i < (int)DebugBreakpointList.size(); i++)
	{
		if ((DebugBreakpointList[i].ScriptID == script_id || DebugBreakpointList[i].ScriptID == -1) &&
			 (thread_id == -1 || DebugBreakpointList[i].ThreadID == -1 || thread_id == DebugBreakpointList[i].ThreadID) &&
			 (_stricmp(DebugBreakpointList[i].FileName.c_str(), filename.c_str()) == 0) &&
			 DebugBreakpointList[i].Line == line)
		{
			DebugBreakpointList.erase(DebugBreakpointList.begin()+i);
			i--;
			continue;
		}
		if (DebugBreakpointList[i].ScriptID == -1)
		{
			global_break_point = true;
		}
	}

	if (global_break_point == false)
	{
		Debug_Disable_Attach_All();
	}
}


void LuaScriptClass::Debug_Get_Loaded_Child_Scripts(std::vector<std::string> &scripts)
{
	if (State == NULL)
	{
		return;
	}

	LuaMap::Pointer tab = LUA_SAFE_CAST(LuaMap, Map_Global_From_Lua("_LOADED", true));
	scripts.resize(0);
	if (tab)
	{
		LuaMapType::iterator it = tab->Value.begin();
		for (; it != tab->Value.end(); it++)
		{
			LuaString::Pointer strval = PG_Dynamic_Cast<LuaString>(it->first);
			if (strval)
			{
				std::string fullname;
				Generate_Full_Path_Name(strval->Value, fullname);
				scripts.push_back(fullname);
			}
		}
	}
}

void LuaScriptClass::Debug_Disable_Attach_All(void)
{
	if (DebugShouldAttachAll == false) return;

	ActiveScriptListType::iterator it = ActiveScriptList.begin();
	for (; it != ActiveScriptList.end(); it++)
	{
		if (it->second->DebugTarget == 2)
		{
			it->second->Debug_Detach();
		}
	}
	DebugShouldAttachAll = false;
}

void LuaScriptClass::Debug_Enable_Attach_All(void)
{
	if (DebugShouldAttachAll) return;

	ActiveScriptListType::iterator it = ActiveScriptList.begin();
	for (; it != ActiveScriptList.end(); it++)
	{
		it->second->Debug_Attach(true);
	}
	DebugShouldAttachAll = true;
}

// member function.
void LuaScriptClass::Debug_Attach(bool auto_attach)
{
	if (State == NULL)
		return;

	if (DebugTarget)
	{
		if (DebugTarget == 2 && auto_attach == false)
		{
			DebugTarget = 1;
		}
		return;
	}
	DebugTarget = auto_attach ? 2 : 1;
	lua_sethook(State, Debug_Single_Step_Hook, LUA_MASKLINE, 0);
	for (int i = 0; i < (int)ThreadData.size(); i++)
	{
		if (ThreadData[i].Thread)
		{
			lua_sethook(ThreadData[i].Thread, Debug_Single_Step_Hook, LUA_MASKLINE, 0);
		}
	}
}

// member function.
void LuaScriptClass::Debug_Detach(void)
{
	DebugTarget = 0;
	if (State == NULL)
		return;
	lua_sethook(State, NULL, 0, 0);
	for (int i = 0; i < (int)ThreadData.size(); i++)
	{
		if (ThreadData[i].Thread)
		{
			lua_sethook(ThreadData[i].Thread, NULL, 0, 0);
		}
	}
}

// member function.
void LuaScriptClass::Debug_Attach_Thread(int thread_id)
{
	FAIL_IF(thread_id < 0 || thread_id >= static_cast<int>(ThreadData.size())) { return; }
	FAIL_IF(ThreadData[thread_id].Thread == NULL) { return; }

	lua_sethook(ThreadData[thread_id].Thread, Debug_Single_Step_Hook, LUA_MASKLINE, 0);
}

static DebugBreakTypeEnum string_break_type = DEBUG_BREAK_NONE;
void LuaScriptClass::Debug_Pre_Execute_String(void)
{
	string_break_type = DebugBreakType;
	DebugBreakType = DEBUG_BREAK_NONE;
}

void LuaScriptClass::Debug_Post_Execute_String(void)
{
	DebugBreakType = string_break_type;
}

// static function
void LuaScriptClass::Debug_Continue(void)
{
	FAIL_IF(DebugIsSuspended == false) { return; }
	DebugBreakType = DEBUG_BREAK_NONE;
	DebugIsSuspended = false;
}

// static function
void LuaScriptClass::Debug_Step_In_To(void)
{
	FAIL_IF(DebugIsSuspended == false) { return; }
	DebugBreakType = DEBUG_STEP_IN_TO;
	DebugIsSuspended = false;
}

// static function
void LuaScriptClass::Debug_Step_Over(void)
{
	FAIL_IF(DebugIsSuspended == false) { return; }
	DebugBreakType = DEBUG_STEP_OVER;
	DebugIsSuspended = false;
}

// static function
void LuaScriptClass::Debug_Step_Out(void)
{
	FAIL_IF(DebugIsSuspended == false) { return; }
	DebugBreakType = DEBUG_STEP_OUT;
	DebugIsSuspended = false;
}

// static function
void LuaScriptClass::Debug_Break_All(void)
{
	FAIL_IF(DebugIsSuspended == true) { return; }
	FAIL_IF(DebugBreakType == DEBUG_BREAK_ALL) { return; }

	DebugBreakType = DEBUG_BREAK_ALL;
}

// static function
void LuaScriptClass::Debug_Break_Thread(int thread_id)
{
	FAIL_IF(DebugContextScript == NULL) { return; }

	if (thread_id != -1)
	{
		FAIL_IF(thread_id < 0 || thread_id >= static_cast<int>(DebugContextScript->ThreadData.size())) { return; }
		FAIL_IF(DebugContextScript->ThreadData[thread_id].Thread == NULL) { return; }
	}

	DebugContextThreadID = thread_id;
	DebugBreakType = DEBUG_BREAK_THREAD;
	DebugIsSuspended = false;
}

// static function
void LuaScriptClass::Debug_Set_Script_Context(LuaScriptClass *script)
{
	if (script == DebugContextScript)
	{
		return;
	}
	DebugContextStack.resize(0);
	DebugIsSuspended = false;
	DebugBreakType = DEBUG_BREAK_NONE;
	DebugContextThreadID = -1;
	DebugContextScript = script;
	Debug_Break_All();
}

static void _Debug_Build_Context_String(std::string &outstr, lua_Debug *dbg, bool for_callstack)
{
	if (dbg->source && dbg->source[0] == '@')
	{
		dbg->source++;
	}
	String_Printf(outstr, "%s:%d:%s:%s:%s", 
					  dbg->source ? dbg->source : "",
					  for_callstack ? dbg->currentline : dbg->linedefined, 
					  dbg->what ? dbg->what : "",
					  dbg->namewhat ? dbg->namewhat : "",
					  dbg->name ? dbg->name : "");
}

// static function
static void _Debug_Build_Context_Stack(lua_State *L, DebugContextStackType &cstack, bool for_callstack)
{
	cstack.resize(0);
	int i = 0;
	int status = 1;
	lua_Debug dbg;
	for (; status == 1; i++) 
	{
		status = lua_getstack(L, i, &dbg);
		lua_getinfo(L, "lSn", &dbg);
		DebugContextStruct *ts = new DebugContextStruct;
		_Debug_Build_Context_String(ts->CurrentContext, &dbg, for_callstack);
		cstack.insert(cstack.begin(), ts);
	}
}

// member function
void LuaScriptClass::Break_For_Step_Out(void)
{
	// Not our thread context so continue.
	if (Get_Current_Thread_Id() != DebugContextThreadID) 
		return;

	DebugContextStackType curstack;
	lua_State *cur_state = DebugContextThreadID == -1 ? State : ThreadData[DebugContextThreadID].Thread;
	_Debug_Build_Context_Stack(cur_state, curstack, false);

	for (int i = 0; i < (int)curstack.size(); i++)
	{
		// 
		if (i >= (int)DebugContextStack.size())
		{
			// Current script stack is deeper than our context so don't break on this line.
			return;
		}
		if (curstack[i]->CurrentContext != DebugContextStack[i]->CurrentContext)
		{
			// We've somehow descended into another callstack tree.
			// Shouldn't ever happen, but we should break here.
			break;
		}
	}

	if (i == (int)DebugContextStack.size())
	{
		// Callstacks are equal so continue until we exit this function.
		return;
	}

	// We're now in the caller of our context function so go ahead and break.
	DebugContextStack = curstack;
	DebugBreakType = DEBUG_BREAK_NONE;
	DebugCurrentStackContext = ((int)DebugContextStack.size()-1);
	DebugIsSuspended = true;
}

// member function
void LuaScriptClass::Break_For_Step_Over(void)
{
	// Not our thread context so continue.
	if (Get_Current_Thread_Id() != DebugContextThreadID) 
		return;

	DebugContextStackType curstack;
	lua_State *cur_state = DebugContextThreadID == -1 ? State : ThreadData[DebugContextThreadID].Thread;
	_Debug_Build_Context_Stack(cur_state, curstack, false);

	for (int i = 0; i < (int)curstack.size(); i++)
	{
		// 
		if (i >= (int)DebugContextStack.size())
		{
			// Current script stack is deeper than our context so don't break on this line.
			return;
		}
		if (curstack[i]->CurrentContext != DebugContextStack[i]->CurrentContext)
		{
			// We've somehow descended into another callstack tree.
			// Shouldn't ever happen.
			return;
		}
	}
	// Current script callstack is either equal or contained within our Context callstack
	// so go ahead and break here.  Re-assign our Context stack if we've left a function.
	if (i != (int)DebugContextStack.size())
	{
		DebugContextStack = curstack;
	}
	DebugBreakType = DEBUG_BREAK_NONE;
	DebugCurrentStackContext = ((int)DebugContextStack.size()-1);
	DebugIsSuspended = true;
}

// static func
void LuaScriptClass::Debug_Get_Var_From_Lua(lua_State *L, int &ltype, std::string &value, bool pop_var)
{
	ltype = lua_type(L, -1);
	switch (ltype) {
		case LUA_TNUMBER:
			{
				int n = (int) lua_tonumber(L,-1); // was a lua_tostring, but on numbers it changes stack value! clobbers keys when using lua_next. -Eric_Y
				char sn[16];
				sprintf(sn,"%d",n);  // String_Printf didn't work in this case, was getting garbage strings! -Eric_Y
				value = sn;
				break;
			}
		case LUA_TSTRING:
			value = lua_tostring(L, -1);
			break;
		case LUA_TBOOLEAN:
			value = lua_toboolean(L, -1) ? "true" : "false";
			break;
		case LUA_TTABLE:
			String_Printf(value, "table: %p", lua_topointer(L, -1));
			break;
		case LUA_TFUNCTION:
			String_Printf(value, "function: %p", lua_topointer(L, -1));
			break;
		case LUA_TUSERDATA:
		case LUA_TLIGHTUSERDATA:
			// is there a metafield?
			if (luaL_callmeta(L, -1, "__tostring"))
			{
				// use it's value
				value.assign(lua_tostring(L, -1), lua_strlen(L, -1));
				lua_pop(L, 1);
			}
			else
			{
				String_Printf(value, "userdata: %p", lua_topointer(L, -1));
			}
			break;
		case LUA_TTHREAD:
			String_Printf(value, "thread: %p", (void *)lua_tothread(L, -1));
			break;
		default:
			assert(false);
		case LUA_TNIL:
			value = "nil";
			break;
	}

	if (pop_var)
		lua_pop(L, 1);
}

void LuaScriptClass::Debug_Get_Var_From_Lua(const std::string &name, int &ltype, std::string &value)
{
	LocalsTableType::iterator it = LocalsTable.find(name);
	lua_State *cur_state = State;

	const char *local_name = NULL;
	if (it != LocalsTable.end() && DebugIsSuspended && DebugContextScript == this)
	{
		cur_state = DebugContextThreadID == -1 ? State : ThreadData[DebugContextThreadID].Thread;
		lua_Debug ar;
		int slevel = ((int)DebugContextStack.size()-1) - DebugCurrentStackContext;
		if (lua_getstack(cur_state, slevel, &ar))
		{
			local_name = lua_getlocal(cur_state, &ar, it->second);
			if (local_name && it->first != local_name)
			{
				local_name = NULL;
				lua_pop(cur_state, 1);
			}
		}
	}
	if (!local_name)
	{
		lua_pushlstring(cur_state, name.c_str(), name.size());
		lua_gettable(cur_state, LUA_GLOBALSINDEX);
	}
	Debug_Get_Var_From_Lua(cur_state, ltype, value);
}

void LuaScriptClass::Debug_Get_Table_From_Lua(const std::string &name,std::vector<int> &descension,LuaTableMemberList &table_members)
{
	LocalsTableType::iterator it = LocalsTable.find(name);
	lua_State *cur_state = State;

	const char *local_name = NULL;
	if (it != LocalsTable.end() && DebugIsSuspended && DebugContextScript == this)
	{
		cur_state = DebugContextThreadID == -1 ? State : ThreadData[DebugContextThreadID].Thread;
		lua_Debug ar;
		int slevel = ((int)DebugContextStack.size()-1) - DebugCurrentStackContext;
		if (lua_getstack(cur_state, slevel, &ar))
		{
			local_name = lua_getlocal(cur_state, &ar, it->second);
			if (local_name && it->first != local_name)
			{
				local_name = NULL;
				lua_pop(cur_state, 1);
			}
		}
	}
	if (!local_name)
	{
		lua_pushlstring(cur_state, name.c_str(), name.size());
		lua_gettable(cur_state, LUA_GLOBALSINDEX);
	}
	for (int i = 0; i < (int)descension.size(); i++)
	{
		FAIL_IF (Descend_Into_Table(cur_state, descension[i]) == false)
		{
			// Fixup lua stack.
			lua_pop(cur_state, i);
			return;
		}
	}

	Debug_Dump_Table_Members(cur_state, table_members);

	// Fixup lua stack.
	lua_pop(cur_state,1); // pop the table value off the stack
	lua_pop(cur_state, 2 * ((int)descension.size())); // pop the value/key pairs off

	return;
}


bool LuaScriptClass::Descend_Into_Table(lua_State *L, int index)
{
	int t = lua_gettop(L);

	// first key
	lua_pushnil(L);
	for (int i = 0; lua_next(L, t) != 0; i++)
	{
		if (i == index) 
		{
			if (lua_type(L, -1) == LUA_TTABLE)
				return true;

			lua_pop(L, 2);
			return false;
		}

		lua_pop(L,1); // pop the value (leave the key for iteration)
	}

	return false;
}


void LuaScriptClass::Debug_Dump_Table_Members(lua_State *L, LuaTableMemberList &table_members)
{
	// table is in the stack at the top
	int t = lua_gettop(L);

	// first key
	lua_pushnil(L);
	while (lua_next(L, t) != 0) 
	{
		// `key' is at index -2 and `value' at index -1

		// Map the value
		table_members.resize(table_members.size()+1);
		Debug_Get_Var_From_Lua(L, table_members.back().value_type, table_members.back().value_string, true);

		// don't pop the key since we need it to iterate.
		Debug_Get_Var_From_Lua(L, table_members.back().key_type, table_members.back().key_value, false);
	}
}


void LuaScriptClass::Debug_Build_Locals_Table(void)
{
	FAIL_IF (DebugIsSuspended == false) { return; }

	lua_State *cur_state = DebugContextThreadID == -1 ? State : ThreadData[DebugContextThreadID].Thread;

	LocalsTable.clear();
	lua_Debug ar;
	int slevel = ((int)DebugContextStack.size()-1) - DebugCurrentStackContext;
	if (lua_getstack(cur_state, slevel, &ar))
	{
		int i = 1;
		const char *name = NULL;
		while ((name = lua_getlocal(cur_state, &ar, i)) != NULL)
		{
			LocalsTable[name] = i;
			lua_pop(cur_state, 1);
			i++;
		}
	}
}

void LuaScriptClass::Debug_Dump_Local_Variable_Names(std::vector<std::string> &names)
{
	FAIL_IF (DebugIsSuspended == false) { return; }

	LocalsTableType::iterator it = LocalsTable.begin();
	names.resize(0);
	for ( ; it != LocalsTable.end(); it++)
	{
		names.push_back(it->first);
	}
}

void LuaScriptClass::Debug_Set_Current_Callstack_Depth(int level)
{
	FAIL_IF(DebugIsSuspended == false) { return; }
	FAIL_IF(level < 0 || level >= (int)DebugContextStack.size()) return;

	DebugCurrentStackContext = level;
	Debug_Build_Locals_Table();
}


void LuaScriptClass::Debug_Get_Callstack(std::vector<std::string> &call_stack)
{
	FAIL_IF(DebugIsSuspended == false) { return; }

	lua_State *cur_state = DebugContextThreadID == -1 ? State : ThreadData[DebugContextThreadID].Thread;
	DebugContextStackType curstack;
	_Debug_Build_Context_Stack(cur_state, curstack, true);

	call_stack.resize(0);

	for (int i = 0; i < (int)curstack.size(); i++)
	{
		call_stack.push_back(curstack[i]->CurrentContext);
	}
}

// static function
void LuaScriptClass::Debug_Single_Step_Hook(lua_State *L, lua_Debug *dbg)
{
	if (dbg->event == LUA_HOOKTAILRET) return;

	LuaScriptClass *script = LuaScriptClass::Get_Script_From_State(L);
	FAIL_IF(!script) { return; }

	DebugIsSuspended = false;
	script->Set_Current_Thread(L);

	if (DebugContextScript == script || (DebugBreakType == DEBUG_BREAK_ALL && DebugContextScript == NULL))
	{
		switch (DebugBreakType)
		{
			case DEBUG_BREAK_ALL:
				DebugContextScript = script;
				_Debug_Build_Context_Stack(L, DebugContextStack, false);
				DebugContextThreadID = script->Get_Current_Thread_Id();
				DebugCurrentStackContext = ((int)DebugContextStack.size()-1);
				DebugIsSuspended = true;
				DebugBreakType = DEBUG_BREAK_NONE;
				break;

			case DEBUG_STEP_OVER:
				script->Break_For_Step_Over();
				break;

			case DEBUG_BREAK_THREAD:
			case DEBUG_STEP_IN_TO:
				// Only care about our thread context
				if (script->Get_Current_Thread_Id() == DebugContextThreadID) 
				{
					_Debug_Build_Context_Stack(L, DebugContextStack, false);
					DebugContextThreadID = script->Get_Current_Thread_Id();
					DebugCurrentStackContext = ((int)DebugContextStack.size()-1);
					DebugIsSuspended = true;
					DebugBreakType = DEBUG_BREAK_NONE;
				}
				break;

			case DEBUG_STEP_OUT:
				script->Break_For_Step_Out();
				break;

			case DEBUG_BREAK_NONE:
				break;

			default:
				assert(false);
				return;
		}
	}

	int status = lua_getstack(L, 0, dbg);
	FAIL_IF (status == 0) { return; }

	bool break_point = false;
	lua_getinfo(L, "lSn", dbg);
	if (dbg->source && dbg->source[0] == '@')
	{
		dbg->source++;
	}
	std::string sname(dbg->source);

	// search for a breakpoint hit.
	for (int i = 0; i < (int)DebugBreakpointList.size(); i++)
	{
		if ((DebugBreakpointList[i].ScriptID == -1 || 
			  DebugBreakpointList[i].ScriptID == script->Get_Script_ID()) &&
			 (DebugBreakpointList[i].ThreadID == -1 || 
			  DebugBreakpointList[i].ThreadID == script->Get_Current_Thread_Id()) &&
			 dbg->currentline == DebugBreakpointList[i].Line &&
			 _stricmp(DebugBreakpointList[i].FileName.c_str(), sname.c_str()) == 0)
		{
			_Debug_Build_Context_Stack(L, DebugContextStack, false);
			DebugContextThreadID = script->Get_Current_Thread_Id();
			DebugContextScript = script;
			DebugIsSuspended = true;
			DebugCurrentStackContext = ((int)DebugContextStack.size()-1);
			DebugBreakType = DEBUG_BREAK_NONE;
			break_point = true;
			if (LuaDebugCallbacks)
			{
				LuaDebugCallbacks->Script_Suspended(script);
			}
			break;
		}
	}

	if (LuaDebugCallbacks && DebugIsSuspended && break_point == false)
	{
		LuaDebugCallbacks->Script_Suspended(script);
	}

	MSG msg;
	while (DebugIsSuspended)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE|PM_NOYIELD)) 
		{
			if (!TranslateAccelerator(msg.hwnd, NULL, &msg)) 
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		if (LuaDebugCallbacks)
		{
			LuaDebugCallbacks->Suspended_Service();
		}
	}
}

LuaTable *LuaScriptClass::Debug_Should_Issue_Event_Alert(LuaScriptClass *, LuaTable *)
{
	return 0;
//    if (CurrentThreadId < 0 || CurrentThreadId >= static_cast<int>(ThreadData.size()))
//    {
//       return 0;
//    }
//
//    return Return_Variable(new LuaBool(ThreadData[CurrentThreadId].EventAlert));
}

