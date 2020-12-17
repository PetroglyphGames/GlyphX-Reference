// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScript.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScript.cpp $
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

#include "LuaScript.h"
#include "LuaScriptVariable.h"
#include "SaveLoad.h"
#include "Text.h"
#include "MegaFileManager.h"
#include "UtilityCommands.h"
#include "../SecuROM/securom_api.h"
#include <algorithm>
#include "LuaNetworkDebugger.h"
#include "LuaScriptWrapper.h"
#include "LuaExternalFunction.h"

extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

std::vector<std::string>	LuaScriptClass::ScriptPaths;
std::string						LuaScriptClass::ScriptPathString("./?.lua;./?.lc");
int								LuaScriptClass::NextScriptID = 1;
LuaScriptClass::ScriptPoolListType			LuaScriptClass::ScriptPool;
LuaScriptClass::ActiveScriptListType		LuaScriptClass::ActiveScriptList;

LuaScriptClass::LogCallbackType						LuaScriptClass::LogMessageCallback = NULL;
LuaScriptClass::LogCallbackType						LuaScriptClass::LogErrorCallback = NULL;
LuaScriptClass::LogCallbackType						LuaScriptClass::LogWarningCallback = NULL;
LuaScriptClass::FileRegistrationCallbackType		LuaScriptClass::RegisterScriptCallback = NULL;
LuaScriptClass::FileRegistrationCallbackType		LuaScriptClass::UnregisterScriptCallback = NULL;

bool LuaScriptClass::ResetPerformed = false;

PG_IMPLEMENT_RTTI(LuaScriptClass, LuaUserVar);

/**
 * Lua Error Handler
 * 
 * @param L      Lua state
 * 
 * @return number of params we're returning to lua.
 * @since 4/23/2005 5:33:05 PM -- BMH
 */
int LuaScriptClass::Lua_Error_Handler(lua_State *L)
{
	if (!LogErrorCallback)
	{
		return 0;
	}

	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; i++)
	{
		const char *str = lua_tostring(L, i);
		if (str) 
		{
			Debug_Print_Current_Callstack(L, false);
			std::string error_message;
			error_message.reserve(8192);
			String_Printf(error_message, "LuaScriptClass::ERROR -- %s", str);
			LogErrorCallback(error_message.c_str());
		}
	}
#ifndef NDEBUG
	static lua_Debug dbg;
	if (lua_getstack(L, 1, &dbg) == 1 && lua_getinfo(L, "lSn", &dbg) == 1)
	{
		Assert_Handler("Error encountered executing script function - see AI log for details", dbg.short_src ? dbg.short_src : "Unknown file", static_cast<unsigned int>(dbg.currentline));
	}
	else
	{
		Assert_Handler("Error encountered executing script function - see AI log for details", "Unknown file", 0);
	}
#endif

	return 0;
}

/**
 * Lua Compile error handler.
 * 
 * @param L      lua state
 * 
 * @return 1 to attempt to reload the script, 0 to continue.
 * @since 3/22/2006 3:54:56 PM -- BMH
 */
int LuaScriptClass::Lua_Compile_Error(lua_State *L)
{
	if (!LogErrorCallback)
	{
		return 0;
	}

	int nargs = lua_gettop(L);
	const char *str = NULL;
	for (int i = 1; i <= nargs; i++)
	{
		str = lua_tostring(L, i);
		if (str) 
		{
			Debug_Print_Current_Callstack(L, false);
			std::string error_message;
			error_message.reserve(8192);
			String_Printf(error_message, "LuaScriptClass::ERROR -- %s", str);
			LogErrorCallback(error_message.c_str());
		}
	}

	if (str)
	{
		Debug_Print("LuaScriptClass::ALERT -- %s\n", str);
		LuaScriptClass *script = LuaScriptClass::Get_Script_From_State(L);
		script->LastError = str;
	}

#ifndef NDEBUG
	static lua_Debug dbg;
	if (lua_getstack(L, 1, &dbg) == 1 && lua_getinfo(L, "lSn", &dbg) == 1)
	{
		Assert_Handler("Script crash or parse error - see AI log for details!", dbg.short_src ? dbg.short_src : "Unknown file", static_cast<unsigned int>(dbg.currentline));
	}
	else
	{
		Assert_Handler("Script crash or parse error - see AI log for details!", "Unknown file", 0);
	}
#endif

	LuaScriptClass *script = LuaScriptClass::Get_Script_From_State(L);
	std::string error_message;
	error_message.reserve(8192);
	String_Printf(error_message, "LuaScript Error in %s.lua!\n\n%s", 
					  script->Get_Name().c_str(), script->LastError.c_str());
	error_message += "\n\nDo you wish to reload the script now?";
	script->LastError.clear();
	return Custom_Popup(error_message.c_str(), MB_YESNO) == IDYES ? 1 : 0;
}

/**
 * Lua Alert handler.
 * 
 * @param L      lua state
 * 
 * @return number of params we're returning to lua.
 * @since 4/23/2005 5:33:23 PM -- BMH
 */
int LuaScriptClass::Lua_Alert_Handler(lua_State *L)
{
	if (!LogErrorCallback)
	{
		return 0;
	}

	int nargs = lua_gettop(L);
	const char *str = NULL;
	for (int i = 1; i <= nargs; i++)
	{
		str = lua_tostring(L, i);
		if (str) 
		{
			Debug_Print_Current_Callstack(L, false);
			std::string error_message;
			error_message.reserve(8192);
			String_Printf(error_message, "LuaScriptClass::ERROR -- %s", str);
			LogErrorCallback(error_message.c_str());
		}
	}

	if (str)
	{
		Debug_Print("LuaScriptClass::ALERT -- %s\n", str);
		LuaScriptClass *script = LuaScriptClass::Get_Script_From_State(L);
		script->LastError = str;
	}
	
#ifndef NDEBUG
	static lua_Debug dbg;
	if (lua_getstack(L, 1, &dbg) == 1 && lua_getinfo(L, "lSn", &dbg) == 1)
	{
		Assert_Handler("Script crash or parse error - see AI log for details!", dbg.short_src ? dbg.short_src : "Unknown file", static_cast<unsigned int>(dbg.currentline));
	}
	else
	{
		Assert_Handler("Script crash or parse error - see AI log for details!", "Unknown file", 0);
	}
#endif
	return 0;
}

/**
 * Assign our alert function to the lua state.
 * @since 4/23/2005 5:34:15 PM -- BMH
 */
void LuaScriptClass::Set_Alert_Function(void)
{
	lua_pushstring(State, "_ALERT");
	lua_pushcfunction(State, Lua_Alert_Handler);
	lua_settable(State, LUA_GLOBALSINDEX);
}

void LuaScriptClass::Script_Message(const char *text, ...)
{
	char _temp_buffer[1024];

	if (text == NULL) return;

	if (!LogMessageCallback)
	{
		return;
	}

	va_list va;
	va_start(va, text);
	_vsnprintf(_temp_buffer, sizeof(_temp_buffer)-1, text, va);
	va_end(va);

	std::string message;
	message.reserve(8192);
	String_Printf(message, "LuaScript: \"%s\", Message: %s", Name.c_str(), _temp_buffer);
	LogMessageCallback(message.c_str());

	// Send message to the debugger
	message.append(std::string("\n"));
	LuaNetworkDebuggerClass::Send_Output_To_Debugger(message);
}

/**
 * Register this script as causing an error.
 * 
 * @param text   printf style text describing the error.
 * @since 4/23/2005 5:34:53 PM -- BMH
 */
void LuaScriptClass::Script_Error(const char *text, ...)
{
	char _temp_buffer[1024];

	if (text == NULL) return;

	if (!LogErrorCallback)
	{
		Set_Exit();
		return;
	}

	va_list va;
	va_start(va, text);
	_vsnprintf(_temp_buffer, sizeof(_temp_buffer)-1, text, va);
	va_end(va);

	std::string error_message;
	error_message.reserve(8192);
	int tid = Get_Current_Thread_Id();
	if (tid == -1 || ThreadData[tid].Thread == NULL)
	{
		LogErrorCallback("LuaScriptThread: Main State");
		Debug_Print_Current_Callstack(State, false);
	}
	else
	{
		String_Printf(error_message, "LuaScriptThread: %s", ThreadData[tid].Thread_Name.c_str());
		LogErrorCallback(error_message.c_str());
		Debug_Print_Current_Callstack(ThreadData[tid].Thread, false);
	}
	String_Printf(error_message, "LuaScript: \"%s\", Error: %s", Name.c_str(), _temp_buffer);
	LogErrorCallback(error_message.c_str());

	// Send message to the debugger
	error_message.append(std::string("\n"));
	LuaNetworkDebuggerClass::Send_Output_To_Debugger(error_message);

#ifndef NDEBUG
	static lua_Debug dbg;
	if (CurrentThreadId >= 0 && CurrentThreadId < static_cast<int>(ThreadData.size()))
	{ 
		if (lua_getstack(ThreadData[CurrentThreadId].Thread, 1, &dbg) == 1 && 
				lua_getinfo(ThreadData[CurrentThreadId].Thread, "lSn", &dbg) == 1)
		{
			Assert_Handler("Error encountered executing script function - see AI log for details", dbg.short_src ? dbg.short_src : "Unknown file", static_cast<unsigned int>(dbg.currentline));
		}
		else
		{
			Assert_Handler("Error encountered executing script function - see AI log for details", "Unknown file", 0);
		}
	}
	else if (lua_getstack(State, 1, &dbg) == 1 && lua_getinfo(State, "lSn", &dbg) == 1)
	{
		Assert_Handler("Error encountered executing script function - see AI log for details", dbg.short_src ? dbg.short_src : "Unknown file", static_cast<unsigned int>(dbg.currentline));
	}
	else
	{
		Assert_Handler("Error encountered executing script function - see AI log for details", "Unknown file", 0);
	}
#endif
	Set_Exit();
}

/**
 * Register this script as causing a warning.
 * 
 * @param text   printf style text describing the warning.
 * @since 4/23/2005 5:35:26 PM -- BMH
 */
void LuaScriptClass::Script_Warning(const char *text, ...)
{
	char _temp_buffer[1024];

	if (text == NULL) return;

	if (!LogWarningCallback)
	{
		return;
	}

	va_list va;
	va_start(va, text);
	_vsnprintf(_temp_buffer, sizeof(_temp_buffer)-1, text, va);
	va_end(va);

	std::string error_message;
	error_message.reserve(8192);
	int tid = Get_Current_Thread_Id();
	if (tid == -1 || ThreadData[tid].Thread == NULL)
	{
		LogWarningCallback("LuaScriptThread: Main State");
		Debug_Print_Current_Callstack(State, true);
	}
	else
	{
		String_Printf(error_message, "LuaScriptThread: %s", ThreadData[tid].Thread_Name.c_str());
		LogWarningCallback(error_message.c_str());
		Debug_Print_Current_Callstack(ThreadData[tid].Thread, true);
	}
	String_Printf(error_message, "LuaScript: \"%s\", Warning: %s", Name.c_str(), _temp_buffer);
	LogWarningCallback(error_message.c_str());

	// Send message to the debugger
	error_message.append(std::string("\n"));
	LuaNetworkDebuggerClass::Send_Output_To_Debugger(error_message);
}

/**
 * Add a new path to the static list of lua file search paths.
 * 
 * @param path   New path to add
 * @since 4/23/2005 5:36:06 PM -- BMH
 */
void LuaScriptClass::Add_Script_Path(const char *path)
{
	int slen = strlen(path);
	std::string tstr(path);
	if (path[slen - 1] != '/' && path[slen - 1] != '\\') {
		tstr += std::string("/");
	}
	//Check for duplicates
	for (unsigned int i = 0; i < ScriptPaths.size(); ++i)
	{
		if (ScriptPaths[i] == tstr)
		{
			return;
		}
	}
	ScriptPaths.push_back(tstr);
	Build_Script_Path_String();
}

/**
 * Static service function.  Called one a frame.
 * @since 4/28/2005 5:44:46 PM -- BMH
 */
void LuaScriptClass::Service(void)
{
	Manage_Lua_Tables();
}

/**
 * Take the base script name and find the disk file that would be
 * loaded based on the script path search rules.
 * @since 2/14/2006 6:23:21 PM -- BMH
 */
void LuaScriptClass::Generate_Full_Path_Name(const std::string &name, std::string &full_name)
{
	string directory_file_spec;
	directory_file_spec = std::string("./") + name + std::string(".lua");

	FileClass fval;
	if (fval.Open(directory_file_spec) == false)
	{
		directory_file_spec = std::string("./") + name + std::string(".lc");
		fval.Open(directory_file_spec);
	}

	if (fval.Is_Open())
	{
		fval.Close();
		full_name = directory_file_spec;
		return;
	}

	for (int i = 0; i < (int)ScriptPaths.size(); i++)
	{
		directory_file_spec = ScriptPaths[i] + name + std::string(".lua");

		if (fval.Open(directory_file_spec) == false)
		{
			directory_file_spec = ScriptPaths[i] + name + std::string(".lc");
			fval.Open(directory_file_spec);
		}

		if (fval.Is_Open() == false) continue;

		fval.Close();
		full_name = directory_file_spec;
		break;
	}
}

void LuaScriptClass::Generate_Full_Path_Name(void)
{
	Generate_Full_Path_Name(Name, FullName);
}

/**
 * Return the full pathname of the script.
 * 
 * @return string of the full pathname.
 * @since 3/11/2005 3:06:00 PM -- BMH
 */
const std::string &LuaScriptClass::Get_Full_Path_Name(void)
{
	return FullName;
}

/**
 * Setup the initial lua state.  Map in the base library functions
 * we want accessible to this state.
 * 
 * @return true if ok, false if there was a lua stack allocation error.
 * @since 4/23/2005 5:37:01 PM -- BMH
 */
bool LuaScriptClass::Init_State(void)
{
	State = lua_open();
	assert(State);
	lua_atpanic(State, Lua_Error_Handler);
	luaopen_base(State);
	luaopen_string(State);
	luaopen_table(State);
	luaopen_security(State);
	if (lua_checkstack (State, 16) == 0)
	{
		Script_Error("Initial Stack Allocation Failure!");
		Shutdown();
		return false;
	}

	Set_Alert_Function();
	Set_File_Handler();

	Map_Global_To_Lua(this, "Script");
	SmartPtr<LuaString> var = new LuaString(ScriptPathString);
	Map_Global_To_Lua(var, "LUA_PATH");

	std::pair<ActiveScriptListType::iterator, bool> retval = ActiveScriptList.insert(std::make_pair(ScriptID, this));
	assert(retval.second);
	retval.first->second = this;

	if (DebugShouldAttachAll)
	{
		Debug_Attach(true);
	}

	return true;
}

/**
 * Constructor for a lua script object
 * 
 * @param script filename of the script
 * @since 4/22/2004 2:38:15 PM -- BMH
 */
LuaScriptClass::LuaScriptClass(const std::string &script) : 
	ExitFlag(false)
,	CurrentThreadId(-1)
,	ThreadEventHandler(NULL)
,	State(NULL)
,	PoolFreshLoad(true)
,	PoolInUse(false)
,	ScriptIsPooled(false)
,	ScriptShouldCRC(true)
,	CRCCount(0)
,	SaveID(0)
,	ScriptID(NextScriptID++)
,	DebugTarget(0)
,	ScriptShouldReload(false)
{
	FAIL_IF(!Init_State()) return;

	Load_From_File(script);

	LUA_REGISTER_MEMBER_FUNCTION(LuaScriptClass, "Debug_Should_Issue_Event_Alert", &LuaScriptClass::Debug_Should_Issue_Event_Alert);

	UtilityCommandsClass::Register_Commands(this);
}

/**
 * Constructor for a lua script object
 * 
 * @since 4/22/2004 2:38:15 PM -- BMH
 */
LuaScriptClass::LuaScriptClass() :
	ExitFlag(false)
,	CurrentThreadId(-1)
,	ThreadEventHandler(NULL)
,	State(NULL)
,	PoolFreshLoad(true)
,	PoolInUse(false)
,	ScriptIsPooled(false)
,	ScriptShouldCRC(true)
,	CRCCount(0)
,	SaveID(0)
,	ScriptID(NextScriptID++)
,	DebugTarget(0)
,	ScriptShouldReload(false)
{
	FAIL_IF(!Init_State()) return;

	LUA_REGISTER_MEMBER_FUNCTION(LuaScriptClass, "Debug_Should_Issue_Event_Alert", &LuaScriptClass::Debug_Should_Issue_Event_Alert);

	UtilityCommandsClass::Register_Commands(this);
}

/**
 * Destructor
 * @since 4/23/2005 5:37:31 PM -- BMH
 */
LuaScriptClass::~LuaScriptClass()
{
	ActiveScriptListType::iterator it = ActiveScriptList.find(ScriptID);
	if (it != ActiveScriptList.end())
	{
		if (LuaDebugCallbacks) LuaDebugCallbacks->Script_Removed(this);
		ActiveScriptList.erase(it);
	}
}

/**
 * Utility function to strip path and extension from a filename
 * and create a script name from it.
 * 
 * @param filename filename with full path and ext
 * @since 4/22/2004 2:39:11 PM -- BMH
 */
void LuaScriptClass::Set_Name_From_Filename(const std::string &filename)
{
	Name = Strip_Path_And_Extension(filename.c_str());
}

/**
 * Function to override the internal lua file handler.  This allows
 * us to intercept the lua file management calls and remap file 
 * operations to Megafiles.
 * @since 9/20/2005 7:32:59 PM -- BMH
 */
void LuaScriptClass::Set_File_Handler(void)
{
	lua_filehandler_t handler;
	handler.open_file = Internal_Open_File;
	handler.close_file = Internal_Close_File;
	handler.read_file = Internal_Read_File;
	handler.write_file = NULL;
	handler.peek_char = Internal_Peek_Char;
	handler.error_file = Internal_Error_File;
	handler.L = State;
	handler.ud = this;

	luaL_setfilehandler(State, &handler);
}


/**
 * Return the LuaScriptClass object associated with this lua_state.
 * 
 * @param state  lua_State pointer.
 * 
 * @return LuaScriptClass pointer.
 * @since 2/1/2006 1:16:40 PM -- BMH
 */
LuaScriptClass *LuaScriptClass::Get_Script_From_State(lua_State *state)
{
	lua_filehandler_t *handler = luaL_getfilehandler(state);
	if (handler)
	{
		return (LuaScriptClass *)handler->ud;
	}
	return NULL;
}


/**
 * Opens a file and returns a file object to lua.
 * 
 * @param handler lua file handler object
 * @param name    name of the file to load
 * @param mode    fopen mode string that describes how the file should be opened.
 * 
 * @return returns a opaque file object pointer.  NULL if the file failed to open.
 * @since 9/20/2005 7:31:44 PM -- BMH
 */
void *LuaScriptClass::Internal_Open_File(lua_filehandler_t *handler, const char *name, const char *mode)
{
	mode;
	handler;
	FAIL_IF(!name) return NULL;

	FileClass *newfile = new FileClass();
	if (!newfile->Open(name))
	{
		delete newfile;
		return NULL;
	}
	return newfile;
}

/**
 * Close the current file.
 * 
 * @param handler lua file handler object
 * @param file    file object
 * 
 * @return returns 0 if successful, EOF on failure.
 * @since 9/20/2005 7:29:48 PM -- BMH
 */
int LuaScriptClass::Internal_Close_File(lua_filehandler_t *handler, void *file)
{
	handler;
	FileClass *newfile = (FileClass *)file;
	bool retval = newfile->Close();
	delete newfile;
	if (retval) return 0;
	return EOF;
}

/**
 * Read data from the current file.
 * 
 * @param handler lua file handler object
 * @param file    file object
 * @param size    returns the amount of data read.
 * 
 * @return pointer to the data buffer.  NULL if nothing left to read.
 * @since 9/20/2005 7:28:50 PM -- BMH
 */
const char *LuaScriptClass::Internal_Read_File(lua_filehandler_t *handler, void *file, size_t *size)
{
	FileClass *newfile = (FileClass *)file;
	LuaScriptClass *script = (LuaScriptClass *)handler->ud;
	unsigned int rcnt = newfile->Read(&script->HandlerBuff, sizeof(script->HandlerBuff));
	if (rcnt == FILE_READ_ERROR || rcnt == 0)
	{
		return NULL;
	}
	*size = rcnt;
	return script->HandlerBuff;
}

/**
 * Peek one char from the file stream, but don't advance the current
 * file pointer.
 * 
 * @param handler lua file handler object
 * @param file    file object
 * 
 * @return the char at the current file pointer.
 * @since 9/20/2005 7:26:45 PM -- BMH
 */
char LuaScriptClass::Internal_Peek_Char(lua_filehandler_t *handler, void *file)
{
	handler;
	FileClass *newfile = (FileClass *)file;
	char cval = 0;
	unsigned int rcnt = newfile->Read(&cval, 1);
	if (rcnt != FILE_READ_ERROR)
	{
		PG_VERIFY(newfile->Seek(-1, FileClass::FILE_SEEK_CURRENT) != FILE_SEEK_ERROR);
	}
	return cval;
}

/**
 * File error description callback.
 * 
 * @param handler lua file handler object
 * @param file    file object
 * 
 * @return text string describing the file error.
 * @since 9/20/2005 7:25:26 PM -- BMH
 */
const char *LuaScriptClass::Internal_Error_File(lua_filehandler_t *handler, void *file)
{
	handler;
	if (file) return NULL;
	return "";
}

/**
 * Handles the writing of files during the lua_dump_state.
 * 
 * @param lua_State lua state
 * @param p         data to write
 * @param sz        size of the data to write
 * @param ud        Data passed into the lua_dump_state
 * 
 * @return amount of data actually written.
 * @since 4/22/2004 2:41:01 PM -- BMH
 */
int LuaScriptClass::Lua_Write(lua_State * /*L*/, const void *p, size_t sz, void *ud)
{
	LuaScriptClass *t = (LuaScriptClass *)ud;
	assert((p && sz) || sz == 0);
	if (p && sz)
	{
		t->Writer->Write(p, sz);
	}
	return 0;
}

/**
 * Handles the reading of a file during a lua_undump_state.
 * 
 * @param lua_State lua_state.  Unused
 * @param ud        Data passed into lua_undump_state
 * @param sz        amount of data to read.  Set to the actual amount of data read on return.
 * 
 * @return pointer to a buffer containing the data.
 * @since 4/22/2004 2:43:33 PM -- BMH
 */
const char * LuaScriptClass::Lua_Read(lua_State * /*L*/, void *ud, size_t *sz)
{
	LuaScriptClass *t = (LuaScriptClass *)ud;
	int bytes = 0;
	bool ok = t->Reader->Read(t->ReadBuff, sizeof(t->ReadBuff), &bytes);
	assert(ok);
	if (!ok) return NULL;

	*sz = bytes;
	return (*sz > 0) ? t->ReadBuff : NULL;
}

/**
 * Return a persist ID for the given LuaFunction
 * 
 * @param func   lua function object
 * 
 * @return persist id
 * @since 4/23/2005 5:38:15 PM -- BMH
 */
int LuaScriptClass::Get_Persist_ID_For_Lua_Function(LuaFunction *func)
{
	return Get_Persist_ID_For_Lua_Function(State, func);
}

/**
 * return a LuaFunction object for the given persist id.
 * 
 * @param id     persist id
 * 
 * @return Lua Function object.
 * @since 4/23/2005 5:38:46 PM -- BMH
 */
LuaFunction *LuaScriptClass::Get_Lua_Function_For_Persist_ID(int id)
{
	return Get_Lua_Function_For_Persist_ID(State, id);
}

int LuaScriptClass::Get_Persist_ID_For_Lua_Function(lua_State *L, LuaFunction *func)
{
	return lua_getIDFromFunction(L, func->Value);
}

LuaFunction *LuaScriptClass::Get_Lua_Function_For_Persist_ID(lua_State *L, int id)
{
	void *func = lua_getFunctionFromID(L, id);

	if (func) {
		return new LuaFunction((lua_function_t)func);
	}

	return NULL;
}

/**
 * Tests for equality between two lua function objects.
 * 
 * @param func1  lua function object 1
 * @param func2  lua function object 2
 * 
 * @return true if equal
 * @since 9/29/2005 1:44:10 PM -- BMH
 */
bool LuaScriptClass::Compare_Lua_Functions(LuaFunction *func1, LuaFunction *func2)
{
	return Compare_Lua_Functions(State, func1, func2);
}

bool LuaScriptClass::Compare_Lua_Functions(lua_State *L, LuaFunction *func1, LuaFunction *func2)
{
	FAIL_IF(!func1 || !func2 || !func1->Value || !func2->Value) return false;
	return lua_compareFunctions(L, func1->Value, func2->Value) ? true : false;
}

enum {
	LUA_CHUNK_SCRIPT_DATA,
	LUA_CHUNK_SCRIPT_POINTER,
	LUA_CHUNK_SCRIPT_EVENT_HANDLER,
	LUA_CHUNK_SCRIPT_THREAD_COUNT,
	LUA_CHUNK_SCRIPT_THREAD_STATE,
	LUA_CHUNK_SCRIPT_THREAD_FUNCTION,
	LUA_CHUNK_SCRIPT_MAIN_STATE,
	LUA_CHUNK_SCRIPT_THREAD_ALERT_IDS,
	LUA_CHUNK_SCRIPT_GENERATOR_BASE
};

/**
 * Save the state of all the Lua threads currently active.
 * @since 4/22/2004 2:43:55 PM -- BMH
 */
bool LuaScriptClass::Save_State(ChunkWriterClass *writer)
{
	bool ok = true;
	int thread_count = (int)ThreadData.size();
	int last_thread = -1;
	Collect_Garbage();

	assert(ExitFlag == false);

	ok &= writer->Begin_Chunk( LUA_CHUNK_SCRIPT_DATA );
		WRITE_MICRO_CHUNK				(	LUA_CHUNK_SCRIPT_THREAD_COUNT, thread_count);
		WRITE_MICRO_CHUNK_THIS_PTR	(	LUA_CHUNK_SCRIPT_POINTER);
		WRITE_MICRO_CHUNK_MULTI_BASE_PTR(LUA_CHUNK_SCRIPT_GENERATOR_BASE, SignalGeneratorClass);
		for (int i = 0; i < (int)ThreadData.size(); i++)
		{
			if (ThreadData[i].Thread) last_thread = i;
			WRITE_MICRO_CHUNK_STRING	(	LUA_CHUNK_SCRIPT_THREAD_FUNCTION, ThreadData[i].Thread_Name);
			WRITE_MICRO_CHUNK				(	LUA_CHUNK_SCRIPT_THREAD_ALERT_IDS, ThreadData[i].Thread_Alert_ID);
		}
	ok &= writer->End_Chunk();

	LUA_WRITE_CHUNK_VALUE_PTR	(	LUA_CHUNK_SCRIPT_EVENT_HANDLER, ThreadEventHandler, this);

	bool saved_thread = false;

	for (int i = 0; i < (int)ThreadData.size(); i++)
	{
		if (ThreadData[i].Thread != 0) {
			ok &= writer->Begin_Chunk( LUA_CHUNK_SCRIPT_THREAD_STATE );
			Writer = writer;
			lua_dump_state(ThreadData[i].Thread, (lua_Chunkwriter)Lua_Write, this, LuaWrapper::Persist_Object, last_thread == i ? 1 : 0);
			Writer = NULL;
			ok &= writer->End_Chunk();
			saved_thread = true;
		}
	}

	if (saved_thread == false) {
		ok &= writer->Begin_Chunk( LUA_CHUNK_SCRIPT_MAIN_STATE );
		Writer = writer;
		lua_dump_state(State, (lua_Chunkwriter)Lua_Write, this, LuaWrapper::Persist_Object, 1);
		Writer = NULL;
		ok &= writer->End_Chunk();
	}

	return ok;
}

/**
 * Reload the state of each thread in this LuaScript.
 * @since 4/22/2004 2:44:23 PM -- BMH
 */
bool LuaScriptClass::Load_State(ChunkReaderClass *reader)
{
	bool ok = true;
	int thread_count = 0;
	std::string tstr;
	int state_count = 0;

	assert(ThreadData.size() == 0);
	assert(ExitFlag == false);

	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			LUA_READ_CHUNK_VALUE_PTR	(	LUA_CHUNK_SCRIPT_EVENT_HANDLER, ThreadEventHandler, this);

			case LUA_CHUNK_SCRIPT_DATA:
				while (reader->Open_Micro_Chunk()) {
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK				(	LUA_CHUNK_SCRIPT_THREAD_COUNT, thread_count);
						READ_MICRO_CHUNK_THIS_PTR	(	LUA_CHUNK_SCRIPT_POINTER);
						READ_MICRO_CHUNK_MULTI_BASE_PTR(LUA_CHUNK_SCRIPT_GENERATOR_BASE, SignalGeneratorClass);

						case LUA_CHUNK_SCRIPT_THREAD_FUNCTION:
							reader->Read_String(tstr);
							if (tstr.size()) {
								Create_Thread_Function(tstr.c_str(), NULL, true);
							} else {
								ThreadData.resize(ThreadData.size()+1);
							}
							break;

						case LUA_CHUNK_SCRIPT_THREAD_ALERT_IDS:
							{
								int tid = 0;
								reader->Read(&tid, sizeof(tid));
								ThreadData.back().Thread_Alert_ID = tid;
							}
							break;

						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			case LUA_CHUNK_SCRIPT_MAIN_STATE:
				Reader = reader;
				lua_undump_state(State, (lua_Chunkreader)Lua_Read, this, Name.c_str(), LuaWrapper::Persist_Object, true);
				break;

			case LUA_CHUNK_SCRIPT_THREAD_STATE:
				{
					//Skip past the null threads
					while (ThreadData[state_count].Thread == 0)
					{
						++state_count;
					}
					// determine if we're going to be loading more threads for this script.
					bool last_thread = true;
					int i;
					for (i = state_count+1; i < (int)ThreadData.size(); i++) {
						if (ThreadData[i].Thread) {
							last_thread = false;
							break;
						}
					}
					int add_count = last_thread ? (int)ThreadData.size() - (state_count+1) : 0;
					Reader = reader;
					lua_undump_state(ThreadData[state_count].Thread, (lua_Chunkreader)Lua_Read, this, Name.c_str(), LuaWrapper::Persist_Object, last_thread);
					state_count += add_count + 1;
					break;
				}

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	// Fixup Thread functions
	for (int i = 0; i < (int)ThreadData.size(); i++)
	{
		if (ThreadData[i].Thread && ThreadData[i].Thread_Function == NULL)
		{
			SmartPtr<LuaVar> var = Map_Global_From_Lua(ThreadData[i].Thread_Name.c_str());
			ThreadData[i].Thread_Function = PG_Dynamic_Cast<LuaFunction>(var);
		}
	}

	assert(thread_count == (int)ThreadData.size() && thread_count == state_count);

	LuaBool::Pointer should_crc = LUA_SAFE_CAST(LuaBool, Map_Global_From_Lua("ScriptShouldCRC"));
	if (should_crc)
	{
		ScriptShouldCRC = should_crc->Value;
	}

	if (LuaDebugCallbacks) LuaDebugCallbacks->Script_Added(this);

	return ok;
}

/**
 * This function ensures that we garbage collect this script before
 * the first lua user variable is saved.
 * @since 5/9/2005 1:48:44 PM -- BMH
 */
void LuaScriptClass::Prep_For_Save(void)
{
	if (SaveID != SaveLoadClass::Get_Current_Save_ID())
	{
		Collect_Garbage();
		SaveID = SaveLoadClass::Get_Current_Save_ID();

		// Free all the unused pooled scripts.
		SmartPtr<LuaScriptClass> script;
		ScriptPoolListType::iterator it = ScriptPool.find(Name);

		if (it != ScriptPool.end())
		{
			PoolListType::iterator pit = it->second.begin();
			for (; pit != it->second.end(); )
			{
				LuaScriptClass *tscript = *pit;
				if (tscript->PoolInUse == false)
				{
					tscript->ScriptIsPooled = false;
					tscript->Shutdown();
					pit = it->second.erase(pit);
					continue;
				}
				pit++;
			}
		}
	}
}

/**
 * Run the Lua garbage collection.
 * @since 1/13/2005 10:45:48 AM -- BMH
 */
void LuaScriptClass::Collect_Garbage(void)
{
	lua_setgcthreshold(State, 0);
	for (int i = 0; i < (int)ThreadData.size(); i++) {
		if (ThreadData[i].Thread) {
			lua_setgcthreshold(ThreadData[i].Thread, 0);
		}
	}
}

/**
 * Shutdown this LuaScript.  We can't call Shutdown from the destructor 
 * (actually we can it's just there's no point to doing it) since
 * the lua_State maintains refcounted pointers to this LuaScriptClass.  So we
 * have to call lua_close first to remove all references to this object.
 * 
 * THIS MEANS ALL LUASCRIPTCLASS OBJECTS NEED TO HAVE SHUTDOWN CALLED IMPLICITYLY
 * FOR THEIR SHUTDOWN TO HAPPEN CORRECTLY
 */
void LuaScriptClass::Shutdown(void)
{
	//Double shutdown is bad
	FAIL_IF(!State) { return; }

	ExitFlag = true;
	ThreadData.resize(0);

	// Clear the thread table.
	static const char lua_threadtable[] = "LuaThreadTable";
	lua_pushlstring(State, lua_threadtable, sizeof(lua_threadtable)-1);
	lua_pushnil(State);
	lua_settable(State, LUA_GLOBALSINDEX);

	SignalDispatcherClass::Get().Send_Signal(this, PG_SIGNAL_LUA_SCRIPT_SHUTDOWN, NULL);

	if (ScriptIsPooled)
	{
		Call_Function("Flush_G", NULL);
		Call_Function("Base_Definitions", NULL);
		// clear the stack
		lua_pop(State, lua_gettop(State));
		PoolInUse = false;
		PoolFreshLoad = false;
		Collect_Garbage();
	}
	else
	{
		if (UnregisterScriptCallback)
		{
			UnregisterScriptCallback(FullName.c_str());
		}
		Set_Thread_Event_Handler(NULL);
		if (State) lua_close(State);
		State = NULL;

		ActiveScriptListType::iterator it = ActiveScriptList.find(ScriptID);
		if (it != ActiveScriptList.end())
		{
			if (LuaDebugCallbacks) LuaDebugCallbacks->Script_Removed(this);
			ActiveScriptList.erase(it);
		}
	}
}

/**
 * Test to see if any threads are currently active.
 * 
 * @return true if at least one thread is running.
 * @since 12/14/2004 8:49:55 PM -- BMH
 */
bool LuaScriptClass::Is_One_Thread_Active(void) const
{
	for (int i = 0; i < (int)ThreadData.size(); i++)
	{
		if (ThreadData[i].Thread != NULL)
			return true;
	}
	return false;
}

/**
 * Return the number of currently active threads.
 * 
 * @return number of actively executing threads.
 * @since 2/27/2006 3:52:23 PM -- BMH
 */
int LuaScriptClass::Get_Active_Thread_Count(void) const
{
	int tcount = 0;
	for (int i = 0; i < (int)ThreadData.size(); i++)
	{
		if (ThreadData[i].Thread != NULL)
		{
			tcount++;
		}
	}
	return tcount;
}

/**
 * Return the name of the thread with the given ID.
 * 
 * @param id     thread id
 * 
 * @return name of the thread function
 * @since 9/14/2005 7:06:32 PM -- BMH
 */
const std::string *LuaScriptClass::Get_Thread_Name(int id) const
{
	FAIL_IF (id >= (int)ThreadData.size() || id < 0) { return NULL; }

	return &ThreadData[id].Thread_Name;
}

/**
 * Stop the given thread from executing.
 * 
 * @param id     id of the thread to stop
 * @since 9/14/2005 7:13:05 PM -- BMH
 */
void LuaScriptClass::Kill_Thread(int id)
{
	FAIL_IF (id >= (int)ThreadData.size() || id < 0) { return; }

	if (ThreadData[id].Thread)
	{
		Unregister_Thread(ThreadData[id].Thread);
		ThreadData[id] = LuaThreadStruct();
	}
}

/**
 * Set the current thread id given the lua state of the thread.
 * 
 * @param L      lua state of the thread in question.
 * @since 4/23/2005 5:40:01 PM -- BMH
 */
void LuaScriptClass::Set_Current_Thread(lua_State *L)
{
	CurrentThreadId = -1;
	if (L != State) {
		for (int i = 0; i < (int)ThreadData.size(); i++) {
			if (ThreadData[i].Thread == L) {
				CurrentThreadId = i;
				break;
			}
		}
	}
}

/**
 * Create a new Lua thread and start it executing at the lua
 * function <b>func_name</b>.
 * 
 * @param func_name Name of the lua function to begin execution of the thread.
 * @param param     optional thread parameter
 * 
 * @return Thread ID of the new Thread.
 * @since 5/22/2004 8:28:32 PM -- BMH
 */
int LuaScriptClass::Create_Thread_Function(const char *func_name, LuaVar *param /* = NULL*/, bool is_load /* = false*/)
{
	SmartPtr<LuaVar> var = Map_Global_From_Lua(func_name);
	SmartPtr<LuaFunction> func = PG_Dynamic_Cast<LuaFunction>(var);
	FAIL_IF(!func && !is_load) return -1;

   lua_checkstack(State, 1);
	// Alloc a new thread.
	lua_State *thread = lua_newthread(State);
	assert(thread);

	// Take the thread object on the stack and add it to our ThreadTable so it's
	// not immediatly garbage collected.
	Register_Thread();

	// Put the alert handler function at the top of the threads stack.
	lua_pushcfunction(thread, Lua_Alert_Handler);
	int alert_id = lua_alloc_thread_alert_handler(thread, 1);

	ThreadData.resize(ThreadData.size()+1);
	ThreadData.back().Thread = thread;
	ThreadData.back().Thread_Name = func_name;
	ThreadData.back().Thread_Function = var;
	ThreadData.back().Thread_Alert_ID = alert_id;
	ThreadData.back().Thread_Param = param;

	if (DebugTarget)
	{
		lua_sethook(ThreadData.back().Thread, Debug_Single_Step_Hook, LUA_MASKLINE, 0);
	}

	return (int)(ThreadData.size() - 1);
}

/**
 * Internal function to remove a registered thread from the thread
 * table.
 * 
 * @param thread thread to be removed
 * @since 8/10/2005 7:28:34 PM -- BMH
 */
void LuaScriptClass::Unregister_Thread(lua_State *thread)
{
	static const char lua_threadtable[] = "LuaThreadTable";
	lua_pushlstring(State, lua_threadtable, sizeof(lua_threadtable)-1);
	lua_gettable(State, LUA_GLOBALSINDEX);

	FAIL_IF (lua_istable(State, -1) == false) return;

	lua_pushthread(State, thread);
	lua_pushnil(State);
	lua_settable(State, -3); // clear any entry for this thread.

	// pop the table
	lua_pop(State, 1);
}

/**
 * Internal function to register the thread into a lua table of threads.
 * @since 8/8/2005 11:47:14 AM -- BMH
 */
void LuaScriptClass::Register_Thread(void)
{
	static const char lua_threadtable[] = "LuaThreadTable";
	lua_pushlstring(State, lua_threadtable, sizeof(lua_threadtable)-1);
	lua_gettable(State, LUA_GLOBALSINDEX);

	if (lua_istable(State, -1) == false)
	{
		lua_pop(State, 1); // Pop the invalid value in LuaThreadTable
		lua_newtable(State); // create the thread table.
	}

	lua_pushvalue(State, -2); // copy the thread to the top of the stack.
	lua_pushboolean(State, 1);
	lua_settable(State, -3); // add the thread to the table.

	lua_pushlstring(State, lua_threadtable, sizeof(lua_threadtable)-1);
	lua_pushvalue(State, -2); // copy the table to the top of the stack.

	// Assign the thread table to our global variable.
	lua_settable(State, LUA_GLOBALSINDEX);

	// pop the table and the thread.
	lua_pop(State, 2);
}

/**
 * Execute each Lua thread in turn until each thread either exits
 * or yields.
 * @since 4/22/2004 2:46:25 PM -- BMH
 */
void LuaScriptClass::Pump_Threads(void)
{
	if (ExitFlag) {
		Shutdown();
		return;
	}
	for (int i = 0; i < (int)ThreadData.size(); i++) {
		if (ThreadData[i].Thread == NULL)
			continue;
		Map_Var_To_Lua(ThreadData[i].Thread, ThreadData[i].Thread_Function);
		int nargs = 0;
		if (ThreadData[i].Thread_Param)
		{
			nargs = 1;
			Map_Var_To_Lua(ThreadData[i].Thread, ThreadData[i].Thread_Param);
			ThreadData[i].Thread_Param = NULL;
		}
		int res = lua_presume(ThreadData[i].Thread, nargs, ThreadData[i].Thread_Alert_ID);
		if (res) {
			Lua_Alert_Handler(ThreadData[i].Thread);
			Unregister_Thread(ThreadData[i].Thread);
			ThreadData[i] = LuaThreadStruct();
			continue;
		}
		SmartPtr<LuaVar> var = Map_Var_From_Lua(ThreadData[i].Thread);
		SmartPtr<LuaBool> bval = PG_Dynamic_Cast<LuaBool>(var);
		if (!bval || !bval->Value)
		{
			Unregister_Thread(ThreadData[i].Thread);
			ThreadData[i] = LuaThreadStruct();
		} 
		if (ExitFlag) break;
	}
	if (ExitFlag) {
		Shutdown();
		return;
	}
}

/**
 * Calculate the CRC of this LuaScript.
 * 
 * @param seed   Seed for the CRC.
 * 
 * @return CRC of the state.
 * @since 6/29/2005 1:25:05 PM -- BMH
 */
CRCValue LuaScriptClass::Calculate_CRC(CRCValue seed)
{
	return sec_crcstate(State, seed);
}


/**
 * Static init function.
 * @since 4/29/2005 1:47:19 PM -- BMH
 */
void LuaScriptClass::System_Initialize(void)
{
	Init_Lua_Table_Pool();
	ActiveScriptListType::iterator it = ActiveScriptList.begin();
	while (it != ActiveScriptList.end())
	{
		if (LuaDebugCallbacks) LuaDebugCallbacks->Script_Removed(it->second);
		it++;
	}
	ActiveScriptList.clear();
	NextScriptID = 1;
	ResetPerformed = true;

	LuaScriptWrapper::Init_Wrapper_Cache();
	LuaExternalFunction::Init_Wrapper_Cache();
}

/**
 * Static shutdown function.
 * @since 4/29/2005 1:47:29 PM -- BMH
 */
void LuaScriptClass::System_Shutdown(void)
{
	Free_Script_Pool();
	Shutdown_Lua_Table_Pool();
	ActiveScriptListType::iterator it = ActiveScriptList.begin();
	while (it != ActiveScriptList.end())
	{
		if (LuaDebugCallbacks) LuaDebugCallbacks->Script_Removed(it->second);
		it++;
	}
	ActiveScriptList.clear();
	Reset_Debugging();

	LuaExternalFunction::Shutdown_Wrapper_Cache();
	LuaScriptWrapper::Shutdown_Wrapper_Cache();
}

/**
 * Iterate the script pool and shutdown all pooled script.  Also
 * search for scripts that are still registered as InUse and assert.
 * @since 4/23/2005 5:40:45 PM -- BMH
 */
void LuaScriptClass::Free_Script_Pool(void)
{
	ScriptPoolListType::iterator it = ScriptPool.begin();
	for (; it != ScriptPool.end(); it++)
	{
		PoolListType::iterator pit = it->second.begin();
		for (; pit != it->second.end(); pit++)
		{
			LuaScriptClass *script = *pit;
			if (script->PoolInUse)
			{
				Debug_Print("LuaScript::Free_Script_Pool -- Script: %s:%8.8X is still in use.\n", it->first.c_str(), script);
#ifdef ENABLE_REF_COUNT_TRACKING
				script->Debug_Dump_Ref_Table();
#endif
				script->ScriptIsPooled = false;
				//assert(false);
			} 
			else
			{
				script->ScriptIsPooled = false;
				script->Shutdown();
			}
		}
	}
	ScriptPool.clear();
}

/**
 * Dumps a table show the current script pools statistics.
 * @since 4/24/2005 1:38:50 PM -- BMH
 */
void LuaScriptClass::Dump_Lua_Script_Pool_Counts(void)
{
	Debug_Print("LuaScriptClass::Dump_Lua_Script_Pool_Counts\n");
	Debug_Print("%50s%8s%8s%8s%8s%8s\n", "ScriptName", "InUse", "Total", "Threads", "CRCCnt", "CRCMiss");

	ScriptPoolListType::iterator it = ScriptPool.begin();
	for (; it != ScriptPool.end(); it++)
	{
		int pinuse = 0;
		int pcount = 0;
		int pthread_count = 0;
		int ccnt = 0;
		int cmiss = 0;
		PoolListType::iterator pit = it->second.begin();
		for (; pit != it->second.end(); pit++)
		{
			LuaScriptClass *script = *pit;
			if (script->PoolInUse)
			{
				pinuse++;
			} 

			if (script->CRCCount)
				ccnt += script->CRCCount;
			else
				cmiss++;

			pcount++;
			pthread_count += script->Get_Thread_Count();
		}
		Debug_Print("%50s%8d%8d%8d%8d%8d\n", it->first.c_str(), pinuse, pcount, pthread_count, ccnt, cmiss);
	}
}

/**
 * Calculate a CRC for the State of each lua script in the script pool
 * 
 * @param crc    Seed crc value.
 * @param quick  true to only calculate a few scripts.  false to do all the scripts.
 * 
 * @return crc of the lua script pool
 * @since 6/29/2005 2:09:59 PM -- BMH
 */
CRCValue LuaScriptClass::CRC_Lua_Script_Pool(CRCValue crc, bool quick)
{
	// This value isn't exact.  With a value of 3 the actual number of scripts crc'd could range from 0 to 9(n-squared).
	#define SCRIPTS_CRC_PER_FRAME 3

	int crc_count = 0;
	if (ScriptPool.size() == 0) return crc;

	int quantum = 1;
	if (quick) quantum = (int)((((float)ScriptPool.size()) / ((float)(SCRIPTS_CRC_PER_FRAME))) + 0.5f);

	quantum = max(quantum, 1);
	int index = ((int)(crc & 0xffff)) % quantum;

	int t = 0, i = 0;
	ScriptPoolListType::iterator it = ScriptPool.begin();
	for (; it != ScriptPool.end(); it++, t++)
	{
		if (t == index+(quantum*i))
		{
			i++;
			PoolListType::iterator pit = it->second.begin();

			int squantum = 1;
			if (quick) squantum = (int)((((float)it->second.size()) / ((float)(SCRIPTS_CRC_PER_FRAME))) + 0.5f);
			squantum = max(squantum, 1);
			int sindex = ((int)(crc & 0xffff)) % squantum;

			int x = 0, y = 0;
			for (; pit != it->second.end(); pit++, y++)
			{
				if (y == sindex+(squantum*x))
				{
					x++;
					LuaScriptClass *script = *pit;
					if (script->ScriptShouldCRC)
					{
						crc = script->Calculate_CRC(crc);
						//FrameSynchronizerClass::Print_Sync_Message_No_Stack(SYNC_LOG_LUA_CRC, "LuaScriptClass::CRC_Lua_Script_Pool -- %s:%d, CRC: 0x%8.8X\n", script->Get_Name().c_str(), y, crc);
						//Debug_Print("LuaScriptClass::CRC_Lua_Script_Pool -- %s:%d, CRC: 0x%8.8X\n", script->Get_Name().c_str(), y, crc);
						script->CRCCount++;
						crc_count++;
					}
					else
					{
						crc++;
					}
					sindex = ((int)(crc & 0xffff)) % squantum;
				}
			}
			index = ((int)(crc & 0xffff)) % quantum;
		}
	}

	//FrameSynchronizerClass::Print_Sync_Message_No_Stack(SYNC_LOG_LUA_CRC, "LuaScriptClass::CRC_Lua_Script_Pool -- Quick: %s, CRC: 0x%8.8X, Count: %d\n", quick ? "true" : "false", crc, crc_count);
	//Debug_Print("LuaScriptClass::CRC_Lua_Script_Pool -- Quick: %s, CRC: 0x%8.8X, Count: %d\n", quick ? "true" : "false", crc, crc_count);
	return crc;
}


/**
 * Search the script pool and return an active script with this name.
 * 
 * @param name   name of script to find
 * 
 * @return pointer to the script or NULL
 * @since 8/18/2005 3:51:01 PM -- BMH
 */
LuaScriptClass *LuaScriptClass::Find_Active_Script(const std::string &name)
{
	ScriptPoolListType::iterator it = ScriptPool.find(name);

	if (it != ScriptPool.end())
	{
		PoolListType::iterator pit = it->second.begin();
		for (; pit != it->second.end(); pit++)
		{
			LuaScriptClass *tscript = *pit;
			if (tscript->PoolInUse)
			{
				return tscript;
			}
		}
	}
	return NULL;
}


/**
 * Create a script of the given name.  Use the pooling system
 * so when the script is finished we don't destory it.  We keep
 * it around so it can be reused.
 * 
 * @param name   name of the script to load.
 * 
 * @return smart pointer to the new script.
 * @since 4/23/2005 5:42:45 PM -- BMH
 */
SmartPtr<LuaScriptClass> LuaScriptClass::Create_Script(const std::string &name, bool reload)
{
	SmartPtr<LuaScriptClass> script;
	ScriptPoolListType::iterator it = ScriptPool.find(name);

	if (reload && it != ScriptPool.end())
	{
		PoolListType::iterator pit = it->second.begin();
		for (; pit != it->second.end(); pit++)
		{
			LuaScriptClass *tscript = *pit;
			tscript->ScriptIsPooled = false;
			if (tscript->PoolInUse == false)
			{
				tscript->Shutdown();
			}
		}
		it->second.clear();
	}


	bool pool_script = true;
	if (it != ScriptPool.end())
	{
		PoolListType::iterator pit = it->second.begin();
		for (; pit != it->second.end(); pit++)
		{
			LuaScriptClass *tscript = *pit;
			if (tscript->PoolInUse) continue;
			script = tscript;
		}
		if (!script)
		{
			// All Pooled scripts are in use, allocate a new one.
			script = new LuaScriptClass();
			FAIL_IF(!script->Load_From_File(name))
			{
				script->Shutdown();
				return NULL;
			}
			it->second.push_back(script);
			script->PoolFreshLoad = true;
		}
	}
	else
	{
		// first occurence of this script
		script = new LuaScriptClass();
		FAIL_IF(!script->Load_From_File(name))
		{
			script->Shutdown();
			return NULL;
		}

		int pool_count = 0;
		LuaNumber::Pointer poolnum = LUA_SAFE_CAST(LuaNumber, script->Map_Global_From_Lua("ScriptPoolCount"));
		if (poolnum)
		{
			pool_count = (int)poolnum->Value;
			if (!pool_count) pool_script = false;
		}

		script->PoolFreshLoad = true;
		if (pool_script)
		{
			std::pair<ScriptPoolListType::iterator, bool> retval = ScriptPool.insert(std::make_pair(name, PoolListType()));
			assert(retval.second);
			retval.first->second.push_back(script);

			// load up our initial pool count.
			for (int i = 1; i < pool_count; i++)
			{
				SmartPtr<LuaScriptClass> new_script = new LuaScriptClass();
				FAIL_IF(!new_script->Load_From_File(name))
				{
					new_script->Shutdown();
					break;
				}
				retval.first->second.push_back(new_script);
				new_script->PoolFreshLoad = true;
				new_script->ScriptIsPooled = true;
			}
		}
	}

	script->ScriptIsPooled = pool_script;
	script->PoolInUse = true;
	script->ExitFlag = false;
	script->CurrentThreadId = -1;

	return script;
}


/**
 * Check the reload list for any scripts that are pooled.  If any
 * are found clear the pooled list so any new scripts are reloaded.
 * 
 * @param files  list of files being reloaded.
 * @since 7/27/2005 10:47:17 AM -- BMH
 */
void LuaScriptClass::Check_For_Script_Reload(std::vector<std::string> &files)
{
	std::vector<std::string> tfiles;
	for (int i = 0; i < (int)files.size(); i++)
	{
		tfiles.push_back(Build_Uppercase_String(Strip_Path_And_Extension(files[i].c_str())));
	}

	ActiveScriptListType::iterator it = ActiveScriptList.begin();
	for (; it != ActiveScriptList.end(); it++)
	{
		LuaScriptClass *script = it->second;
		std::vector<std::string> child_scripts;
		script->Debug_Get_Loaded_Child_Scripts(child_scripts);
		std::vector<std::string> tchild_scripts = child_scripts;

		int i;
		for (i = 0; i < (int)child_scripts.size(); i++)
		{
			child_scripts[i] = Build_Uppercase_String(Strip_Path_And_Extension(child_scripts[i].c_str()));
		}

		for (i = 0; i < (int)tfiles.size(); i++)
		{
			int t;
			for (t = 0; t < (int)child_scripts.size(); t++)
			{
				if (child_scripts[t] == tfiles[i]) break;
			}
			if (t != (int)child_scripts.size()) break;
		}
		if (i != (int)tfiles.size())
		{
			script->ScriptShouldReload = true;
			if (RegisterScriptCallback)
			{
				RegisterScriptCallback(script->Get_Full_Path_Name().c_str());
				for (int i = 0; i < (int)tchild_scripts.size(); i++)
				{
					RegisterScriptCallback(tchild_scripts[i].c_str());
				}
			}
		}
	}

	ScriptPoolListType::iterator sit = ScriptPool.begin();
	for ( ; sit != ScriptPool.end(); )
	{
		PoolListType::iterator pit = sit->second.begin();
		if (pit == sit->second.end()) continue;

		LuaScriptClass *tscript = *pit;
		if (tscript->Should_Script_Reload())
		{
			for (; pit != sit->second.end(); pit++)
			{
				tscript = *pit;
				tscript->ScriptIsPooled = false;
				tscript->ScriptShouldReload = true;
				if (tscript->PoolInUse == false)
				{
					tscript->Shutdown();
				}
			}
			sit = ScriptPool.erase(sit);
			continue;
		}
		sit++;
	}
}


/**
 * Execute the given text in the context of this lua script.  This
 * behaves just like the lua interpreter.
 * 
 * @param text   text to parse
 * @param result result of the parse and execute.  If result is empty then no
 *               error occurred or more input is required.
 * 
 * @return true if the command executed fine.  false there was a parse error
 *         or more input is required.
 * @since 8/18/2005 3:35:29 PM -- BMH
 */
bool LuaScriptClass::Execute_String(const std::string &text, std::string &result)
{
	static std::string ConsoleText;
	FAIL_IF (!State) return false;

	ConsoleText += text + "\n";
	result.resize(0);
	int status = luaL_loadbuffer(State, ConsoleText.c_str(), ConsoleText.size(), "=stdin");
	if (status == LUA_ERRSYNTAX && strstr(lua_tostring(State, -1), "near `<eof>'") != NULL)
	{
		return false;
	}
	ConsoleText.resize(0);
	if (status == 0) status = lua_pcall(State, 0, LUA_MULTRET, 0);
	if (status != 0)
	{
		// error
		const char *msg = lua_tostring(State, -1);
		if (!msg) msg = "(error with no message)";
		result = msg;
		lua_pop(State, 1);
		return false;
	}
	return true;
}


/**
 * Create a new script instance specific for loading from a saved
 * state.
 * 
 * @param name   Name of the script.
 * 
 * @return pointer to an luascript object ready to have Load_State called on it.
 * @since 12/14/2005 3:39:35 PM -- BMH
 */
LuaScriptClass *LuaScriptClass::Create_For_Load(const std::string &name)
{
	LuaScriptClass *script = new LuaScriptClass();
	script->Set_Name_From_Filename(name);
	script->Generate_Full_Path_Name();
	return script;
}


/**
 * Load a lua script from file.
 * 
 * @param name   name of file to load
 * 
 * @return true if load was successful.
 * @since 4/22/2004 2:47:04 PM -- BMH
 */
bool LuaScriptClass::Load_From_File(const std::string &name)
{
	Set_Name_From_Filename(name);
	if (!Load_Module(name)) 
		return false;

	Collect_Garbage(); // clean up extra memory left over from the load.
	Generate_Full_Path_Name();
	LuaBool::Pointer should_crc = LUA_SAFE_CAST(LuaBool, Map_Global_From_Lua("ScriptShouldCRC"));
	if (should_crc)
	{
		ScriptShouldCRC = should_crc->Value;
	}
	if (LuaDebugCallbacks) LuaDebugCallbacks->Script_Added(this);
	return true;
}

/**
 * Load a module into the current lua state.
 * 
 * @param name   name of the lua module to load
 * 
 * @return true if successful
 * @since 3/22/2006 2:12:42 PM -- BMH
 */
bool LuaScriptClass::Load_Module(const std::string &name)
{
	assert(State);
	std::string load_name = std::string("require (\"") + name + std::string("\")\n");

	int res = lua_dostring(State, load_name.c_str());
	if (res)
	{
		if (Lua_Compile_Error(State) == 1)
		{
			return Load_Module(name);
		}
		return false;
	}
	if (RegisterScriptCallback)
	{
		std::vector<std::string> child_scripts;
		Debug_Get_Loaded_Child_Scripts(child_scripts);
		for (int i = 0; i < (int)child_scripts.size(); i++)
		{
			RegisterScriptCallback(child_scripts[i].c_str());
		}
	}

	return true;
}

/**
 * Call a global lua function.
 * 
 * @param name   name of the function
 * @param params params to pass into the function
 * 
 * @return return value from the function call.
 * @since 4/22/2004 2:48:19 PM -- BMH
 */
SmartPtr<LuaVar> LuaScriptClass::Call_Function(const char *name, LuaTable *params, bool use_maps /*= false*/)
{
	SmartPtr<LuaFunction> func = PG_Dynamic_Cast<LuaFunction>(Map_Global_From_Lua(name));
	// assert(func);
	if (!func) return NULL;

   return Call_Function(func, params, use_maps);
}

SmartPtr<LuaVar> LuaScriptClass::Call_Function(LuaFunction *func, LuaTable *params, bool use_maps /*= false*/)
{
	assert(State);
	int status = 1;
	SmartPtr<LuaVar> res = NULL;
	if (State)
	{
		int s = lua_gettop(State);
		Map_Var_To_Lua(State, func);
		if (params) 
		{
			for (int i = 0; i < (int)params->Value.size(); i++)
			{
				Map_Var_To_Lua(State, params->Value[i]);
			}
		}
		// for now only accept 1 return value
		status = lua_pcall(State, params ? params->Value.size() : 0, 1, 0);
		if (status) {
			Lua_Alert_Handler(State);
			return NULL;
		}

		int rcnt = lua_gettop(State) - s;
		if (rcnt)
		{
			res = Map_Var_From_Lua(State, use_maps);
			LuaVoid *v = PG_Dynamic_Cast<LuaVoid>(res);
			if (v && v->Value == NULL) 
			{
				res = NULL;
			}
		}
	}
	return res;
}

LuaVar * LuaScriptClass::Map_Var_From_Lua(void)
{
	return Map_Var_From_Lua(State);
}
void LuaScriptClass::Map_Table_From_Lua(LuaTable *table)
{
	Map_Table_From_Lua(State, table);
}
void LuaScriptClass::Map_Table_To_Lua(LuaTable *table)
{
	Map_Table_To_Lua(State, table);
}
void LuaScriptClass::Map_Global_To_Lua(LuaVar *var, const char *name)
{
	Map_Global_To_Lua(State, var, name);
}
LuaVar *LuaScriptClass::Map_Global_From_Lua(const char *name, bool use_maps /*= false*/)
{
	return Map_Global_From_Lua(State, name, use_maps);
}
void LuaScriptClass::Map_Var_To_Lua(LuaVar *var)
{
	Map_Var_To_Lua(State, var);
}


/**
 * Takes a lua object at stack index -1 and maps it to a LuaVar object.
 * 
 * @param L      Lua state
 * 
 * @return new LuaVar object of the mapped variable
 * @since 4/22/2004 2:49:29 PM -- BMH
 */
LuaVar * LuaScriptClass::Map_Var_From_Lua(lua_State *L, bool use_maps /*= false*/, bool pop_var /*= true*/, bool test_table_recursion /*= false*/)
{
	LuaVar *retval = NULL;
	switch (lua_type(L, -1)) {
	case LUA_TNIL:
		retval = new LuaVoid(NULL);
		break;
	case LUA_TNUMBER:
		retval = new LuaNumber((float)lua_tonumber(L, -1));
		break;
	case LUA_TBOOLEAN:
		retval = new LuaBool(lua_toboolean(L, -1) == 0 ? false : true);
		break;
	case LUA_TSTRING:
		{
			// use lua_strlen to get the size of the string, 
			// that way std::string doesn't have to strlen the whole string again
			LuaString *str = new LuaString();
			str->Value.assign(lua_tostring(L, -1), lua_strlen(L, -1));
			retval = str;
			break;
		}
	case LUA_TTABLE:
		{
			if (use_maps) {
				LuaMap *var = new LuaMap();
				Map_Map_From_Lua(L, var, test_table_recursion);
				retval = var;
			} else {
				LuaTable *var = Alloc_Lua_Table();
				Map_Table_From_Lua(L, var, test_table_recursion);
				retval = var;
			}
			break;
		}
	case LUA_TFUNCTION:
		retval = new LuaFunction((lua_function_t)lua_topointer(L, -1));
		break;
	case LUA_TUSERDATA:
		{
			LuaWrapper *wrap = (LuaWrapper *)lua_topointer(L, -1);
			retval = wrap->Var;
			assert(PG_Is_Type<LuaUserVar>(retval));
         assert(lua_issamestate(wrap->State, L)); // This is bad if this hits.
			break;
		}

	case LUA_TTHREAD:
		retval = new LuaThread((lua_thread_t)lua_topointer(L, -1));
		break;
	case LUA_TLIGHTUSERDATA:
		retval = new LuaPointer((void *)lua_topointer(L, -1));
		break;
	default:
		Debug_Print("LuaScriptClass::Map_Var_From_Lua -- Unknown Lua_Type!\n");
		assert(false);
		retval = new LuaVoid(NULL);
		break;
	}
	if (pop_var) lua_pop(L, 1);
	return retval;
}

#ifndef NDEBUG
typedef stdext::hash_map<const void *, bool> TableRecurseCheckType;
static TableRecurseCheckType _TableRecurseCheck;
#endif

/**
 * Maps a hash map of objects at stack index -1 from lua to a LuaMap Object
 * 
 * @param L      lua state
 * @param mapvar LuaMap object that receives all the member objects
 * @since 11/23/2004 3:47:08 PM -- BMH
 */
void LuaScriptClass::Map_Map_From_Lua(lua_State *L, LuaMap *mapvar, bool test_table_recursion /*= false*/)
{

#ifndef NDEBUG
	if (test_table_recursion == false) 
		_TableRecurseCheck.clear();
	std::pair<TableRecurseCheckType::iterator, bool> retval = _TableRecurseCheck.insert(std::make_pair(lua_topointer(L, -1), true));
	// Infinite table recursion!!
	FAIL_IF(retval.second == false) return;
#else
	test_table_recursion;
#endif

	// table is in the stack at the top
	int t = lua_gettop(L);

	// first key
	lua_pushnil(L);
	while (lua_next(L, t) != 0) {
		// `key' is at index -2 and `value' at index -1
		// Map the value
		LuaVar *value = Map_Var_From_Lua(L, true, true, true);
		// don't pop the key since we need it to iterate.
		LuaVar *key = Map_Var_From_Lua(L, true, false, true);
		mapvar->Value[key] = value;
	}
}

/**
 * Maps a Table of objects at stack index -1 from lua to a LuaTable Object
 * 
 * @param L      lua state
 * @param table  LuaTable object that receives all the member objects
 * @since 4/22/2004 2:50:56 PM -- BMH
 */
void LuaScriptClass::Map_Table_From_Lua(lua_State *L, LuaTable *table, bool test_table_recursion /*= false*/)
{

#ifndef NDEBUG
	if (test_table_recursion == false) 
		_TableRecurseCheck.clear();
	std::pair<TableRecurseCheckType::iterator, bool> retval = _TableRecurseCheck.insert(std::make_pair(lua_topointer(L, -1), true));
	// Infinite table recursion!!
	FAIL_IF(retval.second == false) return;
#else
	test_table_recursion;
#endif

	// table is in the stack at the top
	int t = lua_gettop(L);

	// first key
	lua_pushnil(L);
	while (lua_next(L, t) != 0) {
		// `key' is at index -2 and `value' at index -1
		// Map the value and add it to our vector
		LuaVar *value = Map_Var_From_Lua(L, false, true, true);
		table->Value.push_back(value);
	}
}

struct functor {
	functor(lua_State *l) : L(l) {}
	lua_State *L;
	void operator() (const std::pair<SmartPtr<LuaVar>, SmartPtr<LuaVar> > &item)
	{
		LuaScriptClass::Map_Var_To_Lua(L, item.first);
		LuaScriptClass::Map_Var_To_Lua(L, item.second);
		lua_settable(L, -3);
	}
};

void LuaScriptClass::Map_Map_To_Lua(lua_State *L, LuaMap *mapvar)
{
	lua_newtable(L);
	std::for_each(mapvar->Value.begin(), mapvar->Value.end(), functor(L));
}

void LuaScriptClass::Map_Table_To_Lua(lua_State *L, LuaTable *table)
{
	lua_newtable(L);
	for (int i = 0; i < (int)table->Value.size(); i++) {
		lua_pushnumber(L, (float)(i+1));
		Map_Var_To_Lua(L, table->Value[i]);
		if (table->Value[i]->Get_Var_Type() == LUA_VAR_TYPE_TABLE)
		{
			Free_Lua_Table((const SmartPtr<LuaTable> &)(table->Value[i]));
		}
		lua_settable(L, -3);
	}
}

void LuaScriptClass::Map_Global_To_Lua(lua_State *L, LuaVar *var, const char *name)
{
	// Map the variable name::LuaWrapper into the global namespace.
	lua_pushstring(L, name);
	Map_Var_To_Lua(L,var);
	lua_settable(L, LUA_GLOBALSINDEX);
}

LuaVar *LuaScriptClass::Map_Global_From_Lua(lua_State *L, const char *name, bool use_maps /*= false*/)
{
	// Map the variable name::LuaWrapper into the global namespace.
	lua_pushstring(L, name);
	lua_gettable(L, LUA_GLOBALSINDEX);
	return Map_Var_From_Lua(L, use_maps);
}

/**
 * Create a new LuaScriptWrapper for the new script.
 * 
 * @param new_script new script we're about to map into
 * 
 * @return new instance of the wrapper.
 * @since 3/15/2006 4:41:54 PM -- BMH
 */
LuaVar *LuaScriptClass::Map_Into_Other_Script(LuaScriptClass *new_script)
{
	return LuaScriptWrapper::Create(this, new_script);
}


/**
* Compare a script to ourself.
* 
* @param lua_var
* 
* @return true if scripts are equal
* @since 4/05/2006 3:22:22 PM -- CSB
*/
bool LuaScriptClass::Is_Equal(const LuaVar *lua_var) const
{
	SmartPtr<LuaScriptClass> other_target = PG_Dynamic_Cast<LuaScriptClass>(const_cast<LuaVar*>(lua_var));
	SmartPtr<LuaScriptWrapper> other_target_wrapper = PG_Dynamic_Cast<LuaScriptWrapper>(const_cast<LuaVar*>(lua_var));
	if (other_target_wrapper) 
	{
		return other_target_wrapper->Get_Object() == this;
	}
	else if (other_target)
	{
		return other_target == this;
	}
	return false;		
}


/**
 * Takes a LuaVar object and places the corresponding lua object
 * at stack index -1.
 * 
 * @param L      lua state
 * @param var    LuaVar object to be mapped into Lua
 * @since 4/22/2004 2:51:48 PM -- BMH
 */
void LuaScriptClass::Map_Var_To_Lua(lua_State *L, LuaVar *var)
{
	switch (var->Get_Var_Type())
	{
		case LUA_VAR_TYPE_MAP:
			Map_Map_To_Lua(L, (LuaMap *)(var));
			break;
		case LUA_VAR_TYPE_TABLE:
			Map_Table_To_Lua(L, (LuaTable *)(var));
			break;
		case LUA_VAR_TYPE_VOID:
			//assert(false);
			lua_pushnil(L);
			break;
		case LUA_VAR_TYPE_POINTER:
			lua_pushlightuserdata(L, ((LuaPointer *)(var))->Value);
			break;
		case LUA_VAR_TYPE_NUMBER:
			lua_pushnumber(L, ((LuaNumber *)(var))->Value);
			break;
		case LUA_VAR_TYPE_BOOL:
			lua_pushboolean(L, ((LuaBool *)(var))->Value);
			break;
		case LUA_VAR_TYPE_STRING:
			lua_pushlstring(L, ((LuaString *)(var))->Value.c_str(), ((LuaString *)(var))->Value.size());
			break;
		case LUA_VAR_TYPE_THREAD:
			assert(false);
			lua_pushnil(L);
			break;
		case LUA_VAR_TYPE_FUNCTION:
			lua_pushlfunction(L, ((LuaFunction *)(var))->Value);
			break;
		case LUA_VAR_TYPE_USER_VAR:
			{
				LuaWrapper *wrap = ((LuaUserVar *)(var))->Get_Wrapper();
				LuaScriptClass *script = LuaScriptClass::Get_Script_From_State(L);
				// This would be very bad...
				FAIL_IF (!script) { lua_pushnil(L); break; }

				if ((wrap && lua_issamestate(wrap->State, L) == 0) || ((LuaUserVar *)(var))->Can_Map_Into_Script(script) == false)
				{
					// Create a new instance of the var for the new script.
					var = ((LuaUserVar *)(var))->Map_Into_Other_Script(script);

					// Failed to clone the script.  Push a nil and bail.
					FAIL_IF (!var) { lua_pushnil(L); break; }

					// Use the new vars wrapper if it has one.
					wrap = ((LuaUserVar *)(var))->Get_Wrapper();
					assert(!wrap || lua_issamestate(wrap->State, L));
				}

				if (!wrap)
				{
					// create a new LuaWrapper and push it onto the stack
					wrap = (LuaWrapper *)lua_newuserdata(L, sizeof(LuaWrapper));
					if (!wrap) {
						Debug_Print("LuaScriptClass::Map_Var_To_Lua -- Unable to allocate LuaWrapper in Lua context.\n");
						assert(false);
						lua_pop(L, 2);	// Remove the last two pushes
						return;
					}
					((LuaUserVar *)(var))->Set_Wrapper(wrap);
					memset(wrap, 0, sizeof(LuaWrapper));
					wrap->Var = ((LuaUserVar *)(var));
					// Set the wrapper state equal to the mainthread state since the thread states are
					// garbage collected on script shutdown.
					wrap->State = lua_getmainthread(L);
					// Setup the meta-table here also...
					LuaWrapper::Set_Meta_Table(L);
				} else {
					// This variable is already mapped into a different LuaScript.
					assert(lua_issamestate(wrap->State, L));
					// Push the wrapper onto the stack
					lua_pushfulluserdata(L, wrap);
				}
				assert(var == wrap->Var);
			}
			break;
		default:
			assert(false);
			break;
	}
}

void LuaScriptClass::To_String(std::string &outstr)
{
	String_Printf(outstr, "%s:%8.8X, Name: %s", Get_Class_Name(), this, Name.c_str());
}

void LuaScriptClass::Build_Script_Path_String()
{
	SECUROM_MARKER_SECURITY_ON(2)
	ScriptPathString.clear();
	ScriptPathString = "./?.lua;./?.lc";
	for (int i = 0; i < (int)ScriptPaths.size(); i++) 
	{
		ScriptPathString += std::string(";") + ScriptPaths[i] + std::string("?.lua;") + ScriptPaths[i] + std::string("?.lc");
	}
	SECUROM_MARKER_SECURITY_OFF(2)
}

void LuaScriptClass::Validate_All_Scripts()
{
	std::string extension_wildcard = "*.lua";
	for (unsigned int i = 0; i < ScriptPaths.size(); i++) 
	{
		string directory_file_spec;
		directory_file_spec = ScriptPaths[i] + extension_wildcard;

		WIN32_FIND_DATA find_data;
		HANDLE file_handle = TheMegaFileManager->Subfile_Find_First(directory_file_spec.c_str(), &find_data);

		if (file_handle == INVALID_HANDLE_VALUE)
		{
			continue;
		}

		for (BOOL found = true; found; found = TheMegaFileManager->Subfile_Find_Next(file_handle, &find_data)) 
		{
			if (strcmp(find_data.cFileName, ".") != 0 && strcmp(find_data.cFileName, "..") != 0)
			{
				std::string trimmed_name = find_data.cFileName;
				trimmed_name.erase(trimmed_name.length() - (extension_wildcard.size() - 1), extension_wildcard.size() - 1);
				SmartPtr<LuaScriptClass> script = Create_Script(trimmed_name, true);
				if (!script)
				{
					std::string error_message;
					error_message.reserve(8192);
					String_Printf(error_message, "Lua script %s failed to parse.  See AILog.txt for details.", trimmed_name.c_str());
					Message_Popup(error_message.c_str());
				}
				else if (!script->ScriptIsPooled)
				{
					//Only shutdown non-pooled scripts.  The pooled ones would attempt to run Base_Definitions to reset themselves which
					//could cause problems
					script->Shutdown();
				}
				else
				{
					//Mark pooled scripts as unused.  We'll flush them at the end of this process.
					script->PoolInUse = false;
				}
			}
		}
		PG_VERIFY(TheMegaFileManager->Subfile_Find_Close(file_handle));
	}

	//Now free up the pooled ones.
	Free_Script_Pool();
}