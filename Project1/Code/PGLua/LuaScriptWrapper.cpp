// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScriptWrapper.cpp#3 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScriptWrapper.cpp $
//
//    Original Author: Brian Hayes
//
//            $Author: Steve_Tall $
//
//            $Change: 747006 $
//
//          $DateTime: 2020/10/20 16:15:42 $
//
//          $Revision: #3 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma hdrstop

#include "LuaScript.h"
#include "LuaScriptWrapper.h"
#include "PGSignal/SignalDispatcher.h"
#include "Text.h"

PG_IMPLEMENT_RTTI(LuaScriptWrapper, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_LUA_SCRIPT_WRAPPER, LuaScriptWrapper);
MEMORY_POOL_INSTANCE(LuaScriptWrapper, LUA_WRAPPER_POOL_SIZE);

LuaScriptWrapper::WrapperCacheType *LuaScriptWrapper::WrapperCache = NULL;

LuaScriptWrapper::LuaScriptWrapper() :
	Script(NULL),
	Persistable(true)
{
	LUA_REGISTER_MEMBER_FUNCTION(LuaScriptWrapper, "Is_Valid", &LuaScriptWrapper::Is_Valid);
	LUA_REGISTER_MEMBER_FUNCTION_USE_MAPS(LuaScriptWrapper, "Call_Function", &LuaScriptWrapper::Call_Function);
	LUA_REGISTER_MEMBER_FUNCTION_USE_MAPS(LuaScriptWrapper, "Set_Variable", &LuaScriptWrapper::Set_Variable);
	LUA_REGISTER_MEMBER_FUNCTION(LuaScriptWrapper, "Get_Variable", &LuaScriptWrapper::Get_Variable);
}

LuaScriptWrapper::~LuaScriptWrapper()
{
	Remove_Cached_Wrapper();
}

void LuaScriptWrapper::Init(const LuaScriptClass *object)
{
	Object = const_cast<LuaScriptClass*>(object);
	if (Object && Object->Is_Finished() == false && Object->Get_State())
	{
		SignalDispatcherClass::Get().Add_Listener(Object, this, PG_SIGNAL_LUA_SCRIPT_SHUTDOWN);
	}
}

enum
{
	LUA_SCRIPT_ID_MICRO_CHUNK,
	SIGNAL_LISTENER_BASE_MICRO_CHUNK,
};

bool LuaScriptWrapper::Save(LuaScriptClass *, ChunkWriterClass *writer)
{
	bool ok = true;
	assert(Persistable);
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(SIGNAL_LISTENER_BASE_MICRO_CHUNK, SignalListenerClass);
	WRITE_MICRO_CHUNK_SMART_PTR(LUA_SCRIPT_ID_MICRO_CHUNK, Object);
	return ok;
}

bool LuaScriptWrapper::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	while (reader->Open_Micro_Chunk())
	{
		switch (reader->Cur_Micro_Chunk_ID())
		{
			READ_MICRO_CHUNK_MULTI_BASE_PTR(SIGNAL_LISTENER_BASE_MICRO_CHUNK, SignalListenerClass);
			READ_MICRO_CHUNK_SMART_PTR(LUA_SCRIPT_ID_MICRO_CHUNK, Object);
		default:
			ok = false;
			break;
		}

		reader->Close_Micro_Chunk();
	}

	Script = script;
	SaveLoadClass::Register_Post_Load_Callback(Post_Load_Member_Callback<LuaScriptWrapper>, this);

	return ok;
}

void LuaScriptWrapper::Post_Load_Callback(void)
{
	if (Object && Script)
	{
		WrapperCache->insert(std::make_pair(std::make_pair(Object, Script), this));
	}
}

void LuaScriptWrapper::Remove_Cached_Wrapper(void)
{
	if (Script)
	{
		WrapperCacheType::iterator it = WrapperCache->find(std::make_pair((LuaScriptClass *)Object, Script));
		if (it != WrapperCache->end() && it->second == this)
		{
			WrapperCache->erase(it);
			Script = 0;
		}
	}
}

void LuaScriptWrapper::Init_Wrapper_Cache(void)
{
	if (!WrapperCache)
	{
		WrapperCache = new WrapperCacheType();
	}
}

void LuaScriptWrapper::Shutdown_Wrapper_Cache(void)
{
	if (WrapperCache)
	{
		delete WrapperCache;
		WrapperCache = NULL;
	}
}

LuaScriptWrapper *LuaScriptWrapper::Create(const LuaScriptClass *cobj, LuaScriptClass *script, bool persistable)
{
	LuaScriptClass *obj = const_cast<LuaScriptClass *>(cobj);
	LuaScriptWrapper *target;
	if (script)
	{
		std::pair<WrapperCacheType::iterator, bool> retval = WrapperCache->insert(std::make_pair(std::make_pair(obj, script), (LuaScriptWrapper *)NULL));
		if (retval.second)
		{
			target = (LuaScriptWrapper *) FactoryCreate();
			target->Init(obj);
			retval.first->second = target;
			target->Script = script;
		}
		else
		{
			target = retval.first->second;
			assert(target);
			assert(target->Script == script);
		}
	}
	else
	{
		target = (LuaScriptWrapper *) FactoryCreate();
		target->Init(obj);
	}

	target->Persistable = persistable;

	return target;
}

bool LuaScriptWrapper::Is_Equal(const LuaVar *lua_var) const
{
	SmartPtr<LuaScriptClass> other_target = PG_Dynamic_Cast<LuaScriptClass>(const_cast<LuaVar*>(lua_var));
	SmartPtr<LuaScriptWrapper> other_target_wrapper = PG_Dynamic_Cast<LuaScriptWrapper>(const_cast<LuaVar*>(lua_var));
	if (other_target_wrapper) 
	{
		return other_target_wrapper->Get_Object() == Object;
	}
	else if (other_target)
	{
		return other_target == Object;
	}
	return false;		
}

void LuaScriptWrapper::Receive_Signal(SignalGeneratorClass *generator, PGSignalType, SignalDataClass *)
{
	SignalDispatcherClass::Get().Remove_Listener(generator, this, PG_SIGNAL_LUA_SCRIPT_SHUTDOWN);

	Remove_Cached_Wrapper();

	Object = 0;
}

void LuaScriptWrapper::To_String(std::string &outstr)
{
	if (!Object)
	{
		String_Printf(outstr, "%s: (NULL)", Get_Class_Name());
	}
	else
	{
		String_Printf(outstr, "%s:%8.8X, Name: %s", Get_Class_Name(), (unsigned int)&Object, Object->Get_Name().c_str());
	}
}

/**
 * Create a new LuaScriptWrapper for the new script.
 * 
 * @param new_script new script we're about to map into
 * 
 * @return new instance of the wrapper.
 * @since 3/15/2006 4:41:54 PM -- BMH
 */
LuaVar *LuaScriptWrapper::Map_Into_Other_Script(LuaScriptClass *new_script)
{
	return LuaScriptWrapper::Create(Object, new_script, Persistable);
}


LuaTable *LuaScriptWrapper::Call_Function(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() < 1)
	{
		script->Script_Error("LuaScriptWrapper::Call_Function -- invalid number of parameters.  Expected at least 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> function_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!function_name)
	{
		script->Script_Error("LuaScriptWrapper::Call_Function -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	LuaTable *call_params = NULL;
	if (params->Value.size() > 1)
	{
		params->Value.erase(params->Value.begin());
		call_params = params;
	}

	SmartPtr<LuaFunction> func = PG_Dynamic_Cast<LuaFunction>(Object->Map_Global_From_Lua(function_name->Value.c_str()));
	if (!func) return NULL;

	LuaVar::Pointer result = Object->Call_Function(func, call_params, true);
	if (result)
	{
		return Return_Variable(result);
	}

	return NULL;
}

LuaTable *LuaScriptWrapper::Set_Variable(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 2)
	{
		script->Script_Error("LuaScriptWrapper::Set_Variable -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> var_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!var_name)
	{
		script->Script_Error("LuaScriptWrapper::Set_Variable -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	Object->Map_Global_To_Lua(params->Value[1], var_name->Value.c_str());

	return NULL;
}

LuaTable *LuaScriptWrapper::Get_Variable(LuaScriptClass *script, LuaTable *params)
{
	FAIL_IF(!Object) { return NULL; }

	if (params->Value.size() != 1)
	{
		script->Script_Error("LuaScriptWrapper::Get_Variable -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaString> var_name = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	if (!var_name)
	{
		script->Script_Error("LuaScriptWrapper::Get_Variable -- invalid type for parameter 1.  Expected string.");
		return NULL;
	}

	LuaVar *result = Object->Map_Global_From_Lua(var_name->Value.c_str(), true);
	if (result)
	{
		return Return_Variable(result);
	}

	return NULL;
}

