// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaExternalFunction.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaExternalFunction.cpp $
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

#include "Always.h"

#include "LuaExternalFunction.h"
#include "PGSignal/SignalDispatcher.h"
#include "LuaScript.h"

PG_IMPLEMENT_RTTI(LuaExternalFunction, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_EXTERNAL_FUNCTION, LuaExternalFunction);
MEMORY_POOL_INSTANCE(LuaExternalFunction, LUA_WRAPPER_POOL_SIZE);

LuaExternalFunction::WrapperCacheType *LuaExternalFunction::WrapperCache = NULL;

/**************************************************************************************************
* LuaExternalFunction::LuaExternalFunction -- Constructor
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:02AM JSY
**************************************************************************************************/
LuaExternalFunction::LuaExternalFunction() :
	Script(NULL),
	Persistable(true)
{
}

/**************************************************************************************************
* LuaExternalFunction::~LuaExternalFunction -- Destructor
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:02AM JSY
**************************************************************************************************/
LuaExternalFunction::~LuaExternalFunction()
{
	Remove_Cached_Wrapper();
}

/**************************************************************************************************
* LuaExternalFunction::Remove_Cached_Wrapper -- Quit tracking this wrapper
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:02AM JSY
**************************************************************************************************/
void LuaExternalFunction::Remove_Cached_Wrapper()
{
	if (Script)
	{
		WrapperCacheType::iterator it = WrapperCache->find(std::make_pair(static_cast<LuaFunction*>(TargetFunction), Script));
		if (it != WrapperCache->end() && it->second == this)
		{
			WrapperCache->erase(it);
			Script = 0;
		}
	}
}

/**************************************************************************************************
* LuaExternalFunction::Init_Wrapper_Cache --
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:04AM JSY
**************************************************************************************************/
void LuaExternalFunction::Init_Wrapper_Cache()
{
	if (!WrapperCache)
	{
		WrapperCache = new WrapperCacheType();
	}
}

/**************************************************************************************************
* LuaExternalFunction::Shutdown_Wrapper_Cache --
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:04AM JSY
**************************************************************************************************/
void LuaExternalFunction::Shutdown_Wrapper_Cache()
{
	if (WrapperCache)
	{
		delete WrapperCache;
		WrapperCache = NULL;
	}
}

/**************************************************************************************************
* LuaExternalFunction::Create -- 
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:04AM JSY
**************************************************************************************************/
LuaExternalFunction *LuaExternalFunction::Create(LuaScriptClass *target_script, LuaFunction *target_function, LuaScriptClass *script, bool persistable)
{
	LuaExternalFunction *new_wrapper = NULL;
	if (script)
	{
		std::pair<WrapperCacheType::iterator, bool> retval = WrapperCache->insert(std::make_pair(std::make_pair(target_function, script), new_wrapper));
		if (retval.second)
		{
			new_wrapper = static_cast<LuaExternalFunction*>(FactoryCreate());
			new_wrapper->Init(target_script, target_function);
			retval.first->second = new_wrapper;
			new_wrapper->Script = script;
		}
		else
		{
			new_wrapper = retval.first->second;
			assert(new_wrapper);
			assert(new_wrapper->Script == script);
		}
	}
	else
	{
		new_wrapper = static_cast<LuaExternalFunction*>(FactoryCreate());
		new_wrapper->Init(target_script, target_function);
	}

	new_wrapper->Persistable = persistable;

	return new_wrapper;
}

/**************************************************************************************************
* LuaExternalFunction::Init -- 
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:15AM JSY
**************************************************************************************************/
void LuaExternalFunction::Init(LuaScriptClass *target_script, LuaFunction *target_function)
{
	if (!target_script || !target_script->Get_State() || target_script->Is_Finished())
	{
		return;
	}

	TargetScript = target_script;
	TargetFunction = target_function;
	SignalDispatcherClass::Get().Add_Listener(TargetScript, this, PG_SIGNAL_LUA_SCRIPT_SHUTDOWN);
}

/**************************************************************************************************
* LuaExternalFunction::Is_Equal -- 
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:15AM JSY
**************************************************************************************************/
bool LuaExternalFunction::Is_Equal(const LuaVar *var) const
{
	SmartPtr<const LuaExternalFunction> other_target = PG_Dynamic_Cast<LuaExternalFunction>(var);
	if (!other_target)
	{
		return false;
	}

	return other_target->TargetFunction == TargetFunction;
}

/**************************************************************************************************
* LuaExternalFunction::Receive_Signal -- Watch for the target script being shutdown and invalidate this
*	wrapper
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:15AM JSY
**************************************************************************************************/
void LuaExternalFunction::Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *)
{
	SignalDispatcherClass::Get().Remove_Listener(TargetScript, this, PG_SIGNAL_LUA_SCRIPT_SHUTDOWN);

	Remove_Cached_Wrapper();

	TargetScript = NULL;
	TargetFunction = NULL;
}

/**************************************************************************************************
* LuaExternalFunction::Map_Into_Other_Script -- 
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:15AM JSY
**************************************************************************************************/
LuaVar *LuaExternalFunction::Map_Into_Other_Script(LuaScriptClass *new_script)
{
	return LuaExternalFunction::Create(TargetScript, TargetFunction, new_script, Persistable);
}

/**************************************************************************************************
* LuaExternalFunction::Function_Call -- 
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:15AM JSY
**************************************************************************************************/
LuaTable *LuaExternalFunction::Function_Call(LuaScriptClass *, LuaTable *params)
{
	FAIL_IF(!TargetScript) { return NULL; }
	FAIL_IF(!TargetFunction) { return NULL; }

	return Return_Variable(TargetScript->Call_Function(TargetFunction, params));
}

enum
{
	TARGET_SCRIPT_MICRO_CHUNK,
	TARGET_FUNCTION_MICRO_CHUNK,
	SIGNAL_LISTENER_BASE_MICRO_CHUNK,
};

/**************************************************************************************************
* LuaExternalFunction::Save -- 
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:15AM JSY
**************************************************************************************************/
bool LuaExternalFunction::Save(LuaScriptClass *, ChunkWriterClass *writer)
{
	FAIL_IF(!Persistable) { return false; }

	bool ok = true;
	WRITE_MICRO_CHUNK_SMART_PTR(TARGET_SCRIPT_MICRO_CHUNK, TargetScript);
	WRITE_MICRO_CHUNK_SMART_PTR(TARGET_FUNCTION_MICRO_CHUNK, TargetFunction);
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(SIGNAL_LISTENER_BASE_MICRO_CHUNK, SignalListenerClass);
	return ok;
}

/**************************************************************************************************
* LuaExternalFunction::Load -- 
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:15AM JSY
**************************************************************************************************/
bool LuaExternalFunction::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	while (reader->Open_Micro_Chunk())
	{
		switch (reader->Cur_Micro_Chunk_ID())
		{
			READ_MICRO_CHUNK_SMART_PTR(TARGET_SCRIPT_MICRO_CHUNK, TargetScript);
			READ_MICRO_CHUNK_SMART_PTR(TARGET_FUNCTION_MICRO_CHUNK, TargetFunction);
			READ_MICRO_CHUNK_MULTI_BASE_PTR(SIGNAL_LISTENER_BASE_MICRO_CHUNK, SignalListenerClass);

		default:
			ok = false;
			assert(false);
			break;
		}

		reader->Close_Micro_Chunk();
	}

	Script = script;
	SaveLoadClass::Register_Post_Load_Callback(Post_Load_Member_Callback<LuaExternalFunction>, this);

	return ok;
}

/**************************************************************************************************
* LuaExternalFunction::Post_Load_Callback -- Put this loaded wrapper into the cache
*
* In:			
*
* Out:			
*
* History: 3/24/2006 10:15AM JSY
**************************************************************************************************/
void LuaExternalFunction::Post_Load_Callback()
{
	if (TargetFunction && Script)
	{
		WrapperCache->insert(std::make_pair(std::make_pair(static_cast<LuaFunction*>(TargetFunction), Script), this));
	}
}

