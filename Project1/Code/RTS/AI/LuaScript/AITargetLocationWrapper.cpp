// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/AITargetLocationWrapper.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/AITargetLocationWrapper.cpp $
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

#pragma hdrstop


#include "AITargetLocationWrapper.h"
#include "AI/AITargetLocation.h"
#include "GameObjectWrapper.h"
#include "PGSignal/SignalDispatcher.h"
#include "GameObject.h"

PG_IMPLEMENT_RTTI(AITargetLocationWrapper, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_AI_TARGET_LOCATION_WRAPPER, AITargetLocationWrapper);
MEMORY_POOL_INSTANCE(AITargetLocationWrapper, LUA_WRAPPER_POOL_SIZE);

AITargetLocationWrapper::WrapperCacheType *AITargetLocationWrapper::WrapperCache = NULL;

AITargetLocationWrapper::AITargetLocationWrapper() :
	Script(NULL),
	Persistable(true)
{
	LUA_REGISTER_MEMBER_FUNCTION(AITargetLocationWrapper, "Get_Game_Object", &AITargetLocationWrapper::Get_Game_Object);
	LUA_REGISTER_MEMBER_FUNCTION(AITargetLocationWrapper, "Is_Valid", &AITargetLocationWrapper::Is_Valid);
	LUA_REGISTER_MEMBER_FUNCTION(AITargetLocationWrapper, "Get_Distance", &AITargetLocationWrapper::Get_Distance);
}

AITargetLocationWrapper::~AITargetLocationWrapper()
{
	Remove_Cached_Wrapper();
}

void AITargetLocationWrapper::Init(const AITargetLocationClass *object)
{
	Object = const_cast<AITargetLocationClass*>(object);
	if (Object && Object->Get_Target_Game_Object())
	{
		SignalDispatcherClass::Get().Add_Listener(Object->Get_Target_Game_Object(), this, PG_SIGNAL_OBJECT_DELETE_PENDING);
	}
}

enum
{
	AI_TARGET_ID_MICRO_CHUNK,
	SIGNAL_LISTENER_BASE_MICRO_CHUNK,
};

bool AITargetLocationWrapper::Save(LuaScriptClass *, ChunkWriterClass *writer)
{
	bool ok = true;
	assert(Persistable);
	WRITE_MICRO_CHUNK_MULTI_BASE_PTR(SIGNAL_LISTENER_BASE_MICRO_CHUNK, SignalListenerClass);
	WRITE_MICRO_CHUNK_SMART_PTR(AI_TARGET_ID_MICRO_CHUNK, Object);
	return ok;
}

bool AITargetLocationWrapper::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	while (reader->Open_Micro_Chunk())
	{
		switch (reader->Cur_Micro_Chunk_ID())
		{
			READ_MICRO_CHUNK_MULTI_BASE_PTR(SIGNAL_LISTENER_BASE_MICRO_CHUNK, SignalListenerClass);
			READ_MICRO_CHUNK_SMART_PTR(AI_TARGET_ID_MICRO_CHUNK, Object);
		default:
			ok = false;
			break;
		}

		reader->Close_Micro_Chunk();
	}

	Script = script;
	SaveLoadClass::Register_Post_Load_Callback(Post_Load_Member_Callback<AITargetLocationWrapper>, this);

	return ok;
}

void AITargetLocationWrapper::Post_Load_Callback(void)
{
	if (Object && Script)
	{
		WrapperCache->insert(std::make_pair(std::make_pair(Object, Script), this));
	}
}

void AITargetLocationWrapper::Remove_Cached_Wrapper(void)
{
	if (Script)
	{
		WrapperCacheType::iterator it = WrapperCache->find(std::make_pair((AITargetLocationClass *)Object, Script));
		if (it != WrapperCache->end() && it->second == this)
		{
			WrapperCache->erase(it);
			Script = 0;
		}
	}
}

void AITargetLocationWrapper::Init_Wrapper_Cache(void)
{
	if (!WrapperCache)
	{
		WrapperCache = new WrapperCacheType();
	}
}

void AITargetLocationWrapper::Shutdown_Wrapper_Cache(void)
{
	if (WrapperCache)
	{
		delete WrapperCache;
		WrapperCache = NULL;
	}
}

AITargetLocationWrapper *AITargetLocationWrapper::Create(const AITargetLocationClass *cobj, LuaScriptClass *script, bool persistable)
{
	AITargetLocationClass *obj = const_cast<AITargetLocationClass *>(cobj);
	AITargetLocationWrapper *target;
	if (script)
	{
		std::pair<WrapperCacheType::iterator, bool> retval = WrapperCache->insert(std::make_pair(std::make_pair(obj, script), (AITargetLocationWrapper *)NULL));
		if (retval.second)
		{
			target = (AITargetLocationWrapper *) FactoryCreate();
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
		target = (AITargetLocationWrapper *) FactoryCreate();
		target->Init(obj);
	}

	target->Persistable = persistable;

	return target;
}

bool AITargetLocationWrapper::Is_Equal(const LuaVar *lua_var) const
{
	SmartPtr<AITargetLocationWrapper> other_target = PG_Dynamic_Cast<AITargetLocationWrapper>(const_cast<LuaVar*>(lua_var));
	if (!other_target)
	{
		return false;
	}

	return other_target->Get_Object() == Object;
}

LuaTable *AITargetLocationWrapper::Get_Game_Object(LuaScriptClass *script, LuaTable *)
{
	if (!Object || !Object->Get_Target_Game_Object())
	{
		return 0;
	}

	return Return_Variable(GameObjectWrapper::Create(Object->Get_Target_Game_Object(), script));
}

void AITargetLocationWrapper::Receive_Signal(SignalGeneratorClass *generator, PGSignalType, SignalDataClass *)
{
	SignalDispatcherClass::Get().Remove_Listener(generator, this, PG_SIGNAL_OBJECT_DELETE_PENDING);

	Remove_Cached_Wrapper();

	Object = 0;
}

void AITargetLocationWrapper::To_String(std::string &outstr)
{
	if (!Object)
	{
		outstr = "NULL";
	}
	else
	{
		outstr = Object->Get_Name();
	}
}

/**************************************************************************************************
* AITargetLocationWrapper::Get_Distance -- Script function to get the distance from this location to a position
*	(defined by another object, an AITargetLocation or a taskforce).
*
* In:				
*
* Out:		
*
* History: 8/11/2005 6:47PM JSY
**************************************************************************************************/
LuaTable *AITargetLocationWrapper::Get_Distance(LuaScriptClass *script, LuaTable *params)
{
	if (!Object)
	{
		script->Script_Error("AITargetLocationWrapper::Get_Distance -- this object is already dead.");
		return 0;
	}

	if (params->Value.size() != 1)
	{
		script->Script_Error("AITargetLocationWrapper::Get_Distance -- invalid number of parameters.  Expceted 1, got %d.", params->Value.size());
		return 0;
	}

	Vector3 target_position;
	if (!Lua_Extract_Position(params->Value[0], target_position))
	{
		script->Script_Error("AITargetLocationWrapper::Get_Distance -- could not extract a position from parameter 1.");
	}

	float distance = (Object->Get_Target_Position() - target_position).Length();

	return Return_Variable(new LuaNumber(distance));
}