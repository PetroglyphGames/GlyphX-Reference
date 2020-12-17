// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScriptVariable.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScriptVariable.cpp $
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

#include "Text.h"
#include "LuaScriptVariable.h"
#include "LuaScript.h"
#include "SaveLoad.h"

extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
}

stdext::hash_map<int, LuaFactoryReg::CreateFunctionType>* LuaFactoryReg::FunctionMap = NULL;
std::vector<FunctionFixup> LuaWrapper::function_fixups;

PG_IMPLEMENT_RTTI_ROOT(LuaVar);
PG_IMPLEMENT_RTTI(LuaUserVar, LuaVar);
PG_IMPLEMENT_RTTI(LuaMap, LuaVar);
PG_IMPLEMENT_RTTI(LuaTable, LuaVar);
PG_IMPLEMENT_RTTI(LuaVoid, LuaVar);
PG_IMPLEMENT_RTTI(LuaNumber, LuaVar);
PG_IMPLEMENT_RTTI(LuaBool, LuaVar);
PG_IMPLEMENT_RTTI(LuaString, LuaVar);
PG_IMPLEMENT_RTTI(LuaThread, LuaVar);
PG_IMPLEMENT_RTTI(LuaFunction, LuaVar);
PG_IMPLEMENT_RTTI(LuaPointer, LuaVar);

MEMORY_POOL_INSTANCE(LuaTable, LUA_VALUE_POOL_SIZE);
MEMORY_POOL_INSTANCE(LuaMap, LUA_VALUE_POOL_SIZE);
MEMORY_POOL_INSTANCE(LuaVoid, LUA_VALUE_POOL_SIZE);
MEMORY_POOL_INSTANCE(LuaNumber, LUA_VALUE_POOL_SIZE);
MEMORY_POOL_INSTANCE(LuaBool, LUA_VALUE_POOL_SIZE);
MEMORY_POOL_INSTANCE(LuaString, LUA_VALUE_POOL_SIZE);
MEMORY_POOL_INSTANCE(LuaFunction, LUA_VALUE_POOL_SIZE);
MEMORY_POOL_INSTANCE(LuaThread, LUA_VALUE_POOL_SIZE);
MEMORY_POOL_INSTANCE(LuaPointer, LUA_VALUE_POOL_SIZE);


/**
 * Sets the meta-table for the lua object at stack index -1.
 * 
 * @param L      Lua state
 * @since 4/22/2004 2:23:16 PM -- BMH
 */
void LuaWrapper::Set_Meta_Table(lua_State *L)
{
	// change the metatable.  false on failure.
	int res = lua_getmetatable(L, -1);
	if (!res)
	{
		static const char lua_metatable[] = "LuaWrapperMetaTable";
		lua_pushlstring(L, lua_metatable, sizeof(lua_metatable)-1);
		lua_gettable(L, LUA_GLOBALSINDEX);

		if (lua_istable(L, -1) == false)
		{
			lua_pop(L, 1); // Pop the invalid value in LuaWrapperMetaTable
			lua_pushlstring(L, lua_metatable, sizeof(lua_metatable)-1);  // push the global name of our meta table
			lua_newtable(L); // create the new meta table.

			// Assign the meta table functions.
			lua_setmetafunc(L, "__index", 7, LuaWrapper::Index_Function);
			lua_setmetafunc(L, "__call", 6, LuaWrapper::Function_Call);
			lua_setmetafunc(L, "__gc", 4, LuaWrapper::Garbage_Collector);
			lua_setmetafunc(L, "__tostring", 10, LuaWrapper::To_String);
			lua_setmetafunc(L, "__eq", 4, LuaWrapper::Test_Equal);

			// Assign the meta table to our global variable.
			lua_settable(L, LUA_GLOBALSINDEX);

			// Query the global variable and put it on the stack
			lua_pushlstring(L, lua_metatable, sizeof(lua_metatable)-1);
			lua_gettable(L, LUA_GLOBALSINDEX);
			assert(lua_istable(L, -1));
		}
	}

	res = lua_setmetatable(L, -2);
	assert(res);
}

/**
 * Index meta-method function called when an index is made
 * on a lua object.  Index methods Object["blah"], or 
 * Object.blah
 * 
 * @param L      lua state
 * 
 * @return number of return values
 * @since 4/22/2004 2:24:58 PM -- BMH
 */
int LuaWrapper::Index_Function (lua_State *L)
{
	// function gettable_event (table, key)
	// This better be userdata.
	assert(lua_type(L, 1) == LUA_TUSERDATA);
	LuaUserVar *uservar = (LuaUserVar *)(((LuaWrapper *)lua_topointer(L, 1))->Var);

	if (uservar->MemberMap) {
		std::string tstr(lua_tostring(L, 2), lua_strlen(L, 2));
		LuaUserVar::MemberMapType::iterator it = uservar->MemberMap->find(tstr);
		if (it != uservar->MemberMap->end() && it->second)
		{
			LuaScriptClass::Map_Var_To_Lua(L, it->second);
			return 1;
		}
	}
	else
	{
		LuaVar::Pointer retval = uservar->Index_Function(lua_tostring(L, 2));
		if (retval)
		{
         LuaScriptClass::Map_Var_To_Lua(L, retval);
			return 1;
		}
	}
	return 0;
}

void LuaWrapper::Request_Function_Smart_Fixup(LuaScriptClass *script, SmartPtr<LuaVar> *var, int pid)
{
	if (function_fixups.size() == 0) {
		SaveLoadClass::Register_Post_Load_Callback(LuaWrapper::Do_Fixup, NULL);
		function_fixups.reserve(4096);
	}
	assert(var);
	function_fixups.push_back(FunctionFixup(script, pid, NULL, var));
	FunctionFixup &f = function_fixups.back();
	SAVE_LOAD_REQUEST_FIXUP((void **)&f.script, script);
	assert(function_fixups.size() < 4096);
}

void LuaWrapper::Request_Function_Fixup(LuaScriptClass *script, void **var, int pid)
{
	if (function_fixups.size() == 0) {
		SaveLoadClass::Register_Post_Load_Callback(LuaWrapper::Do_Fixup, NULL);
		function_fixups.reserve(4096);
	}
	assert(var);
	function_fixups.push_back(FunctionFixup(script, pid, var, NULL));
	FunctionFixup &f = function_fixups.back();
	SAVE_LOAD_REQUEST_FIXUP((void **)&f.script, script);
	assert(function_fixups.size() < 4096);
}

void LuaWrapper::Do_Fixup(void *)
{
	int i = 0;

	for (i = 0; i < (int)function_fixups.size(); i++) {
		FunctionFixup &fixup = function_fixups[i];
		LuaFunction *func = fixup.script->Get_Lua_Function_For_Persist_ID(fixup.pid);
		if (fixup.smart_varptr) {
			(*fixup.smart_varptr) = func;
		} else if (fixup.varptr) {
			(*fixup.varptr) = func;
		} else {
			assert(false);
		}
	}
	function_fixups.clear();
}

//    stdext::hash_map<std::string, SmartPtr<LuaVar> >      *MemberMap;

bool LuaUserVar::Internal_Save(ChunkWriterClass *)
{
	bool ok = true;
	//void *this_ptr = this;

	return (ok);
}

bool LuaUserVar::Internal_Load(ChunkReaderClass *)
{
	bool ok = true;

	return (ok);
}


/**
 * lua_dump_state persistence function.  Called during the dump and
 * undump of a lua_State.
 * 
 * @param lua_State Lua state
 * @param saving    saving state. 0 == loading, 1 == should this object persist,
 *                  2 == saving this instance of the object.
 * @param id        persisted id lua has assigned to the object
 * @param ud        pointer to the userdata object(our LuaWrapper)
 * @param sz        size of the userdata object.
 * @param data      data object passed into the dump/undump function.
 * 
 * @return when saving == 1 return 1 to enable persistence on the object
 *         return 0 to disable.
 * @since 4/22/2004 2:29:13 PM -- BMH
 */
int LuaWrapper::Persist_Object(lua_State *L, int saving, int /*id*/, void *ud, size_t sz, void * data)
{
	LuaWrapper *wrapper = (LuaWrapper *)ud;
	int ret = 0;

	LuaScriptClass *script = PG_Dynamic_Cast<LuaScriptClass>(data);
	assert(script);

	script->Set_Current_Thread(L);

	if (saving == 0) {
		memset(ud, 0, sz);

		ChunkReaderClass *reader = script->Get_Chunk_Reader();

		reader->Close_Chunk();
		bool ok = reader->Open_Chunk();
		assert(ok);
		assert(reader->Cur_Chunk_ID() == LUA_CHUNK_USER_VAR);

		wrapper->State = lua_getmainthread(L);
		ok &= Lua_Load_User_Var(reader, script, wrapper->Var, ud);
		assert(ok);

		reader->Close_Chunk();
		ok &= reader->Open_Chunk();
		assert(ok);
		assert(reader->Cur_Chunk_ID() == LUA_CHUNK_SCRIPT_INTERNAL);
		ret = 1;

	} else {

		LuaUserVar *uvar = PG_Dynamic_Cast<LuaUserVar>(wrapper->Var);
		assert(uvar);
		if (!uvar || uvar->Get_Chunk_Id() == LUA_CHUNK_INVALID)
		{
			// This is an invalid uservar tell lua not to persist it.
			return 0;
		}

		// Let Lua know we want to save this variable.
		if (saving == 1) return 1;

		ChunkWriterClass *writer = script->Get_Chunk_Writer();
		writer->End_Chunk();

		writer->Begin_Chunk(LUA_CHUNK_USER_VAR);
		Lua_Save_User_Var(writer, script, uvar);
		writer->End_Chunk();

		writer->Begin_Chunk(LUA_CHUNK_SCRIPT_INTERNAL);
		ret = 1;
	}
	return ret;
}

bool Lua_Save_Variable(ChunkWriterClass *writer, LuaVar *var, LuaScriptClass *script)
{
	bool ok = true;
	LuaPersistMethod meth;

	if (!var) {
		meth = LUA_PERSIST_METHOD_NULL;
	} else if (SaveLoadClass::Is_Object_Saved(var) == false) {
		meth = LUA_PERSIST_METHOD_OBJECT;
	} else {
		meth = LUA_PERSIST_METHOD_LINK;
	}
	ok &= writer->Begin_Chunk(LUA_CHUNK_VAR_CHUNK_DATA);
		ok &= writer->Write(&meth, sizeof(meth));
		ok &= writer->Write(&var, sizeof(var));
	ok &= writer->End_Chunk();

	if (meth == LUA_PERSIST_METHOD_NULL) return ok;
	if (var->Get_Var_Type() != LUA_VAR_TYPE_FUNCTION)
	{
		TEST_PERSISTED_POINTER(var);
	}
	if (meth == LUA_PERSIST_METHOD_LINK) return ok;

	LuaVarType vt = var->Get_Var_Type();
	ok &= writer->Begin_Chunk(var->Get_Var_Type());
	// ok &= writer->Write(&vt, sizeof(vt));
	switch (vt) {
		case LUA_VAR_TYPE_VOID:
			break;
		case LUA_VAR_TYPE_FUNCTION:
			{
				LuaScriptClass *scr = (script);
				assert(scr);
				TEST_PERSISTED_POINTER(scr);
				int pid = scr->Get_Persist_ID_For_Lua_Function((LuaFunction *)var);
				ok &= writer->Write(&pid, sizeof(pid));
				ok &= writer->Write(&scr, sizeof(scr));
				// need to set meth to something other than method_object since we can't register this object as saved.
				// function pointers need to be saved individually so they're fixed up individually on load.
				meth = LUA_PERSIST_METHOD_INVALID;
				break;
			}
		case LUA_VAR_TYPE_TABLE:
			{
				LuaTable *tab = PG_Dynamic_Cast<LuaTable>(var);
				assert(tab);
				SaveLoadClass::Set_Object_As_Saved(tab);  // Make sure we don't infinitly recurse
				int vcount = tab->Value.size();
				ok &= writer->Begin_Chunk(LUA_VAR_TYPE_TABLE);
				ok &= writer->Write(&vcount, sizeof(vcount));
				ok &= writer->End_Chunk();
				for (int i = 0; i < vcount; i++) {
					ok &= Lua_Save_Variable(writer, tab->Value[i], script);
				}
				break;
			}
		case LUA_VAR_TYPE_POINTER:
			{
			LuaPointer *num = PG_Dynamic_Cast<LuaPointer>(var);
			ok &= writer->Write(&num->Value, sizeof(num->Value));
			break;
			}
		case LUA_VAR_TYPE_NUMBER:
			{
			LuaNumber *num = PG_Dynamic_Cast<LuaNumber>(var);
			ok &= writer->Write(&num->Value, sizeof(num->Value));
			break;
			}
		case LUA_VAR_TYPE_BOOL:
			{
			LuaBool *num = PG_Dynamic_Cast<LuaBool>(var);
			ok &= writer->Write(&num->Value, sizeof(num->Value));
			break;
			}
		case LUA_VAR_TYPE_STRING:
			{
			LuaString *num = PG_Dynamic_Cast<LuaString>(var);
			int str_len = num->Value.size();
			ok &= writer->Write(&str_len, sizeof(str_len));
			ok &= writer->Write(num->Value.c_str(), str_len);
			break;
			}
		case LUA_VAR_TYPE_USER_VAR:
			if (script) {
				script->Prep_For_Save();
			}
			Lua_Save_User_Var(writer, script, PG_Dynamic_Cast<LuaUserVar>(var));
			break;
		case LUA_VAR_TYPE_MAP:
			{
				LuaMap *tab = PG_Dynamic_Cast<LuaMap>(var);
				assert(tab);
				SaveLoadClass::Set_Object_As_Saved(tab);  // Make sure we don't infinitly recurse
				int vcount = tab->Value.size();
				ok &= writer->Begin_Chunk(LUA_VAR_TYPE_MAP);
				ok &= writer->Write(&vcount, sizeof(vcount));
				ok &= writer->End_Chunk();
				LuaMapType::iterator it = tab->Value.begin();
				for (int i = 0; it != tab->Value.end(); i++, it++) {
					ok &= Lua_Save_Variable(writer, it->first, script);
					ok &= Lua_Save_Variable(writer, it->second, script);
				}
				assert(i == vcount);
				break;
			}
		case LUA_VAR_TYPE_THREAD:
		default:
			assert(false);
			break;
	}
	ok &= writer->End_Chunk();

	if (meth == LUA_PERSIST_METHOD_OBJECT) 
		SaveLoadClass::Set_Object_As_Saved(var);

	return (ok);
}

typedef std::vector<std::pair<LuaVar::Pointer, LuaVar::Pointer> > LuaTempMapType;
struct LuaTempMapStruct
{
	LuaMap::Pointer var;
	LuaTempMapType temp_map;
};

void Lua_Map_Post_Load_Callback(void *data)
{
	LuaTempMapStruct *tmap = (LuaTempMapStruct *)data;
	for (int i = 0; i < (int)tmap->temp_map.size(); i++)
	{
      tmap->var->Value[tmap->temp_map[i].first] = tmap->temp_map[i].second;
	}

	delete tmap;
}

bool Lua_Load_Variable(ChunkReaderClass *reader, SmartPtr<LuaVar> &var, LuaScriptClass *script)
{
	LuaVarType vt = LUA_VAR_TYPE_INVALID;
	LuaPersistMethod meth;
	void *this_ptr = NULL;
	bool ok = true;

	FAIL_IF (reader->Open_Chunk() == false) return false;
	FAIL_IF (reader->Cur_Chunk_ID() != LUA_CHUNK_VAR_CHUNK_DATA) return false;
		ok &= reader->Read(&meth, sizeof(meth));
		ok &= reader->Read(&this_ptr, sizeof(this_ptr));
	reader->Close_Chunk();

	if (meth == LUA_PERSIST_METHOD_NULL) 
		return ok;
	if (meth == LUA_PERSIST_METHOD_LINK)
	{
		var = NULL;
		SAVE_LOAD_REQUEST_REF_FIXUP((void **)&var, ((LuaVar*)1), this_ptr);
		return true;
	}

	FAIL_IF(meth != LUA_PERSIST_METHOD_OBJECT) return false;
	FAIL_IF(reader->Open_Chunk() == false) return false;
	// reader->Read(&vt, sizeof(vt));
	vt = (LuaVarType)reader->Cur_Chunk_ID();
	switch (vt)
	{
		case LUA_VAR_TYPE_FUNCTION:
			{
				int pid = 0;
				LuaScriptClass *scr = NULL;
				ok &= reader->Read(&pid, sizeof(pid));
				ok &= reader->Read(&scr, sizeof(scr));
				LuaWrapper::Request_Function_Smart_Fixup(scr, &var, pid);
				break;
			}
		case LUA_VAR_TYPE_TABLE:
			{
				LuaTable *tab = new LuaTable();
				int vcount = 0;
				FAIL_IF(reader->Open_Chunk() == false) return false;
				FAIL_IF(reader->Cur_Chunk_ID() != LUA_VAR_TYPE_TABLE) return false;
				reader->Read(&vcount, sizeof(vcount));
				reader->Close_Chunk();
				tab->Value.resize(vcount);
				for (int i = 0; i < vcount; i++) {
					Lua_Load_Variable(reader, tab->Value[i], script);
				}
				var = tab;
				SaveLoadClass::Register_Pointer(this_ptr, tab);
				break;
			}
		case LUA_VAR_TYPE_VOID:
			var = new LuaVoid(NULL);
			SaveLoadClass::Register_Pointer(this_ptr, var);
			break;
		case LUA_VAR_TYPE_POINTER:
			{
				LuaPointer *num = new LuaPointer();
				reader->Read(&num->Value, sizeof(num->Value));
				var = num;
				SaveLoadClass::Register_Pointer(this_ptr, var);
				break;
			}
		case LUA_VAR_TYPE_NUMBER:
			{
				LuaNumber *num = new LuaNumber();
				reader->Read(&num->Value, sizeof(num->Value));
				var = num;
				SaveLoadClass::Register_Pointer(this_ptr, var);
				break;
			}
		case LUA_VAR_TYPE_BOOL:
			{
				LuaBool *num = new LuaBool();
				reader->Read(&num->Value, sizeof(num->Value));
				var = num;
				SaveLoadClass::Register_Pointer(this_ptr, var);
				break;
			}
		case LUA_VAR_TYPE_STRING:
			{
				LuaString *num = new LuaString();
				int str_len = 0;
				reader->Read(&str_len, sizeof(str_len));
				num->Value.resize(str_len);
				reader->Read((void *)num->Value.c_str(), str_len);
				var = num;
				SaveLoadClass::Register_Pointer(this_ptr, var);
				break;
			}
		case LUA_VAR_TYPE_USER_VAR:
			Lua_Load_User_Var(reader, script, var, NULL);
			break;
		case LUA_VAR_TYPE_MAP:
			{
				LuaTempMapStruct *tmap = new LuaTempMapStruct();
				tmap->var = new LuaMap();
				int vcount = 0;
				FAIL_IF(reader->Open_Chunk() == false) return false;
				FAIL_IF(reader->Cur_Chunk_ID() != LUA_VAR_TYPE_MAP) return false;
				reader->Read(&vcount, sizeof(vcount));
				reader->Close_Chunk();
				tmap->temp_map.resize(vcount);
				for (int i = 0; i < vcount; i++) {

					Lua_Load_Variable(reader, tmap->temp_map[i].first, script);
					Lua_Load_Variable(reader, tmap->temp_map[i].second, script);
				}
				var = tmap->var;
				SaveLoadClass::Register_Post_Load_Callback(Lua_Map_Post_Load_Callback, tmap);
				SaveLoadClass::Register_Pointer(this_ptr, tmap->var);
				break;
			}

		case LUA_VAR_TYPE_THREAD:
		default:
			assert(false);
			break;
	}
	reader->Close_Chunk();
	return ok;
}

bool Lua_Save_User_Var(ChunkWriterClass *writer, LuaScriptClass *script, LuaUserVar *val)
{
	bool ok = true;
	LuaPersistMethod meth;

	if (!val || val->Get_Chunk_Id() == LUA_CHUNK_INVALID)
	{
		meth = LUA_PERSIST_METHOD_NULL;
		ok &= writer->Begin_Chunk(LUA_CHUNK_USERVAR_PERSIST_METHOD);
		ok &= writer->Write(&meth, sizeof(meth));
		ok &= writer->End_Chunk();
		return true;
	}

	int chunk_id = val->Get_Chunk_Id();

	if (SaveLoadClass::Is_Object_Saved(val) == false) {
		meth = LUA_PERSIST_METHOD_OBJECT;
	} else {
		meth = LUA_PERSIST_METHOD_LINK;
	}

	ok &= writer->Begin_Chunk(LUA_CHUNK_USERVAR_PERSIST_METHOD);
	ok &= writer->Write(&meth, sizeof(meth));
	ok &= writer->End_Chunk();

	void *wrapper_id = val->Get_Wrapper();
	if (wrapper_id) {
		assert(val == val->Get_Wrapper()->Var);
		ok &= writer->Begin_Chunk(LUA_CHUNK_USERVAR_WRAPPER_ID);
		ok &= writer->Write(&wrapper_id, sizeof(wrapper_id));
		ok &= writer->End_Chunk();

		SaveLoadClass::Set_Object_As_Saved(wrapper_id);
	}

	TEST_PERSISTED_POINTER(val);
	TEST_PERSISTED_POINTER(wrapper_id);

	ok &= writer->Begin_Chunk(LUA_CHUNK_USERVAR_LINK_ID);
	ok &= writer->Write(&val, sizeof(val));
	ok &= writer->End_Chunk();

	if (meth == LUA_PERSIST_METHOD_OBJECT) {
		ok &= writer->Begin_Chunk(LUA_CHUNK_USERVAR_CHUNK_ID);
		ok &= writer->Write(&chunk_id, sizeof(chunk_id));
		ok &= writer->End_Chunk();
	
		ok &= writer->Begin_Chunk(LUA_CHUNK_USERVAR_INTERNAL_DATA);
		ok &= val->Internal_Save(writer);
		ok &= writer->End_Chunk();
	
		ok &= writer->Begin_Chunk(LUA_CHUNK_USERVAR_CHUNK_DATA);
		ok &= val->Save(script, writer);
		ok &= writer->End_Chunk();

		SaveLoadClass::Set_Object_As_Saved(val);
	}

	return (ok);
}

/**
 * "eq" equality meta-method handler.  Test two LuaUserVar for
 * equality.  Calls the LuaUserVar::Is_Equal virtual function.
 * 
 * @param L      lua state
 * 
 * @return LuaBool representing the equality of var1 and var2.
 * @since 7/28/2004 11:53:41 AM -- BMH
 */
int LuaWrapper::Test_Equal(lua_State *L)
{
	assert(lua_type(L, 1) == LUA_TUSERDATA);
	assert(lua_type(L, 2) == LUA_TUSERDATA);

	LuaWrapper *wrapper1 = (LuaWrapper *)lua_touserdata(L, 1);
	LuaWrapper *wrapper2 = (LuaWrapper *)lua_touserdata(L, 2);

	assert(wrapper1 && wrapper2);

	LuaUserVar *var1 = PG_Dynamic_Cast<LuaUserVar>(wrapper1->Var);
	LuaUserVar *var2 = PG_Dynamic_Cast<LuaUserVar>(wrapper2->Var);

	assert(var1 && var2);

	LuaBool retval;
	retval.Value = var1->Is_Equal(var2);
	LuaScriptClass::Map_Var_To_Lua(L, &retval);
	return 1;
}

/**
 * To String meta-method.  Called when a luauserdata
 * object has a tostring(x) called on it.
 * 
 * @param L      lua state
 * 
 * @return none
 * @since 4/30/2004 5:04:10 PM -- BMH
 */
int LuaWrapper::To_String(lua_State *L)
{
	assert(lua_type(L, 1) == LUA_TUSERDATA);

	LuaWrapper *wrapper = (LuaWrapper *)lua_touserdata(L, 1);
	assert(PG_Is_Type<LuaUserVar>(wrapper->Var));

	LuaString str;
	wrapper->Var->To_String(str.Value);
	LuaScriptClass::Map_Var_To_Lua(L, &str);
	return 1;
}

/**
 * Garbage collection meta-method.  Called when a luauserdata
 * object is about to be collected.
 * 
 * @param L      lua state
 * 
 * @return none
 * @since 4/22/2004 2:31:36 PM -- BMH
 */
int LuaWrapper::Garbage_Collector(lua_State *L)
{
	assert(lua_type(L, 1) == LUA_TUSERDATA);

	LuaWrapper *wrapper = (LuaWrapper *)lua_touserdata(L, 1);
	FAIL_IF(!PG_Is_Type<LuaUserVar>(wrapper->Var)) return 0;

	// Remove the variables reference to the wrapper.
	wrapper->Var->Set_Wrapper(NULL);
	// Remove lua's reference to the object.
	wrapper->Var = 0;
	return 0;
}

std::list<SmartPtr<LuaTable>> *LuaTablePool = NULL;

#define LUA_TABLE_POOL_MIN 256
#define LUA_TABLE_POOL_MAX 512
PG_STATIC_ASSERT(LUA_TABLE_POOL_MAX > LUA_TABLE_POOL_MIN);

/**
 * Init the lua table pool.
 * @since 4/29/2005 1:59:00 PM -- BMH
 */
void Init_Lua_Table_Pool(void)
{
	if (!LuaTablePool)
	{
		LuaTablePool = new std::list<SmartPtr<LuaTable>>();
	}
}

/**
 * Shutdown the lua table pool
 * @since 4/29/2005 1:58:49 PM -- BMH
 */
void Shutdown_Lua_Table_Pool(void)
{
	if (LuaTablePool)
	{
		delete LuaTablePool;
		LuaTablePool = NULL;
	}
}

/**
 * Maintain a pool of lua tables between Min and Max.
 * @since 4/29/2005 1:58:40 PM -- BMH
 */
void Manage_Lua_Tables(void)
{
	if (LuaTablePool->size() < LUA_TABLE_POOL_MIN)
	{
		int diff = LUA_TABLE_POOL_MAX - LuaTablePool->size();
		for (int i = 0; i < diff; i++)
		{
			SmartPtr<LuaTable> tab = new LuaTable();
			tab->Value.reserve(8);
			Free_Lua_Table(tab);
		}
		assert(LuaTablePool->size() == LUA_TABLE_POOL_MAX);
	}
}

/**
 * Alloc a lua table from the list of free tables.
 * 
 * @return lua table pointer
 * @since 4/29/2005 1:58:05 PM -- BMH
 */
LuaTable * Alloc_Lua_Table(void)
{
	if (LuaTablePool->size() != 0)
	{
		LuaTable *retval = LuaTablePool->front();
		retval->Add_Ref();
		LuaTablePool->pop_front();
		ENFORCED_IF(retval->Get_Reference_Count() == 1)
		{
			retval->Set_Reference_Count(0);
			return retval;
		}
		retval->Release_Ref();
	}
	return new LuaTable();
}

/**
 * Add this lua table to the list of free lua tables if it
 * has a refcount of 1.
 * 
 * @param table  lua table to free.
 * @since 4/29/2005 1:57:36 PM -- BMH
 */
void Free_Lua_Table(const SmartPtr<LuaTable> &table)
{
	if (table && table->Get_Reference_Count() == 1)
	{
		table->Value.resize(0);
		LuaTablePool->push_back(table);
	}
}

/**
 * Function call meta-method.  called when an object has the
 * '()' applied to it.
 * 
 * @param L      Lua state
 * 
 * @return 
 * @since 4/22/2004 2:31:36 PM -- BMH
 */
int LuaWrapper::Function_Call(lua_State *L)
{
	assert(lua_type(L, 1) == LUA_TUSERDATA);

	LuaWrapper *wrapper = (LuaWrapper *)lua_touserdata(L, 1);
	assert(PG_Is_Type<LuaUserVar>(wrapper->Var));
	bool use_maps = wrapper->Var->Get_Use_Maps();

	int n = lua_gettop(L); // number of arguments

	SmartPtr<LuaTable> params = Alloc_Lua_Table();
	for (int i = 2; i <= n; i++)
	{
		// start at the second argument
		lua_pushvalue(L, i);
		params->Value.push_back(LuaScriptClass::Map_Var_From_Lua(L, use_maps));
		//lua_pop(L, 1);
	}
	// Call the function with the table.  If the function returns a table
	// then iterate the items in the table and return those as parameters.
	LuaScriptClass *script = LuaScriptClass::Get_Script_From_State(L);
	// LuaScriptClass *script = PG_Dynamic_Cast<LuaScriptClass>(LuaScriptClass::Map_Global_From_Lua(L, "Script"));
	assert(script);
	script->Set_Current_Thread(L);
	SmartPtr<LuaTable> rval = wrapper->Var->Function_Call(script, params);
	Free_Lua_Table(params);
	int rcnt = 0;

	if (rval)
	{
		// Map the return values here.
		for (int i = 0; i < (int)rval->Value.size(); i++)
		{
			LuaScriptClass::Map_Var_To_Lua(L, rval->Value[i]);
			if (rval->Value[i]->Get_Var_Type() == LUA_VAR_TYPE_TABLE)
			{
				Free_Lua_Table((const SmartPtr<LuaTable> &)(rval->Value[i]));
			}
			rcnt++;
		}
		Free_Lua_Table(rval);
	}
	return rcnt;
}

/**
 * Constructor
 * 
 * @param id     ChunkId for this object
 * @since 4/22/2004 2:32:34 PM -- BMH
 */
LuaUserVar::LuaUserVar(int id /*= LUA_CHUNK_INVALID*/, bool register_member_functions /*= true*/) : 
	ChunkId(id)
, Wrapper(NULL)
, MemberMap(NULL)
{
	if (register_member_functions)
	{
		LUA_REGISTER_MEMBER_FUNCTION(LuaUserVar, "Is_Pool_Safe", &LuaUserVar::Is_Pool_Safe);
	}
}

/**
 * destructor
 */
LuaUserVar::~LuaUserVar() 
{
	delete MemberMap;
}

LuaUserVar::LuaUserVar(const LuaUserVar &other) :
	ChunkId(other.ChunkId),
	Wrapper(NULL),
	MemberMap(NULL)
{
	if (other.MemberMap)
	{
		LUA_REGISTER_MEMBER_FUNCTION(LuaUserVar, "Is_Pool_Safe", &LuaUserVar::Is_Pool_Safe);
	}
}

/**
 * Index function on a LuaUserVar.  Looks into the hash_map of LuaVar
 * objects and returns the member object.
 * 
 * @param Key    Object that was passed as the key to the index.
 *               IE: Object["key"], Object.key
 * 
 * @return The LuaVar object associated with "key".
 * @since 4/22/2004 2:34:41 PM -- BMH
 */
LuaVar *LuaUserVar::Index_Function(const char *)
{
	return NULL;
}
size_t LuaUserVar::Hash_Function(void)
{
	return stdext::hash_value<void *>(this);
}
bool LuaUserVar::Hash_Compare(const LuaUserVar *val) const
{
	return this < val;
}
bool LuaUserVar::Is_Equal(const LuaVar *val) const
{
	return this == val;
}
LuaTable *LuaUserVar::Function_Call(LuaScriptClass * /*script*/, LuaTable * /*params*/)
{
	return NULL;
}

void LuaUserVar::To_String(std::string &outstr)
{
	String_Printf(outstr, "%s:%8.8X", Get_Class_Name(), this);
}

const std::string &LuaUserVar::Get_To_String(void)
{
	static std::string tstr;
	To_String(tstr);
	return tstr;
}

/**
 * Virtual function to map a user var from one script into another
 * script.
 * 
 * @param script script we're being mapped into
 * 
 * @return new instance of a uservar object to be mapped into the new script.
 * @since 3/15/2006 4:30:07 PM -- BMH
 */
LuaVar *LuaUserVar::Map_Into_Other_Script(LuaScriptClass *)
{
	// This user var doesn't support mapping into other scripts.
	assert(false);
	return NULL;
}

/**
 * Register a member with this Object.
 * 
 * @param name     name to associate with this object.  This will result in a key
 *                 of LuaString.
 * @param function The LuaVar object to associated with Key.
 * @since 4/22/2004 2:36:22 PM -- BMH
 */
void LuaUserVar::Register_Member(const char *name, LuaVar *function)
{
	if (!MemberMap) {
		MemberMap = new MemberMapType();
	}
	(*MemberMap)[name] = function;
}

size_t LuaHashCompare::operator()(const SmartPtr<LuaVar>& Left) const
{	// hash _Keyval to size_t value
	switch (Left->Get_Var_Type())
	{
		case LUA_VAR_TYPE_NUMBER:
			return ((size_t)stdext::hash_value(_LUA_PTR_CAST(Left, LuaPointer)->Value));

		case LUA_VAR_TYPE_BOOL:
			return ((size_t)stdext::hash_value(_LUA_PTR_CAST(Left, LuaBool)->Value));

		case LUA_VAR_TYPE_STRING:
			return ((size_t)stdext::hash_value(_LUA_PTR_CAST(Left, LuaString)->Value));

		case LUA_VAR_TYPE_USER_VAR:
			return _LUA_PTR_CAST(Left, LuaUserVar)->Hash_Function();

		case LUA_VAR_TYPE_MAP:
			{
				LuaMap *left_map = _LUA_PTR_CAST(Left, LuaMap);
				size_t total_hash = 0;
				for (LuaMapType::const_iterator i = left_map->Value.begin(); 
						i != left_map->Value.end();
						++i)
				{
					total_hash += operator()(i->first);
					total_hash += operator()(i->second);
				}

				return total_hash;
			}
			break;

		case LUA_VAR_TYPE_TABLE:
			{
				LuaTable *left_table = _LUA_PTR_CAST(Left, LuaTable);
				size_t total_hash = 0;
				for (std::vector<SmartPtr<LuaVar> >::const_iterator i = left_table->Value.begin(); 
						i != left_table->Value.end();
						++i)
				{
					total_hash += operator()(*i);
				}

				return total_hash;
			}
			break;
	}
	return ((size_t)stdext::hash_value(_LUA_PTR_CAST(Left, LuaPointer)->Value));
}

bool LuaHashCompare::operator()(const SmartPtr<LuaVar>& Left, const SmartPtr<LuaVar>& Right) const
{	
	return Left < Right;
}

