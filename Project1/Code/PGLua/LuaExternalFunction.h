// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaExternalFunction.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaExternalFunction.h $
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

#ifndef _LUA_EXTERNAL_FUNCTION_H_
#define _LUA_EXTERNAL_FUNCTION_H_

#include "LuaScriptVariable.h"
#include "PGSignal/SignalListener.h"
#include "PairHashCompare.h"

//Wraps a function call across script contexts
class LuaExternalFunction : public LuaUserVar, public SignalListenerClass, public PooledObjectClass<LuaExternalFunction, LUA_WRAPPER_POOL_SIZE>
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_EXTERNAL_FUNCTION, LuaExternalFunction);

	virtual ~LuaExternalFunction();

	static LuaExternalFunction *Create(LuaScriptClass *target_script, LuaFunction *target_function, LuaScriptClass *script, bool persistable = true);
	void Init(LuaScriptClass *target_script, LuaFunction *target_function);

	virtual LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params);
	virtual bool Is_Equal(const LuaVar *var) const;
	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *);

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);
	void Post_Load_Callback();

	static void Init_Wrapper_Cache();
	static void Shutdown_Wrapper_Cache();

	virtual bool Does_Listener_Persist() const { return Persistable; }
	virtual LuaVar *Map_Into_Other_Script(LuaScriptClass *new_script);

	virtual LuaTable *Is_Pool_Safe(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(false)); }

	LuaTable *Is_Valid(LuaScriptClass *, LuaTable*) { return Return_Variable(new LuaBool(TargetScript != 0 && TargetFunction != 0)); }

private:
	LuaExternalFunction();

	void Remove_Cached_Wrapper();

	SmartPtr<LuaScriptClass>	TargetScript;
	SmartPtr<LuaFunction>		TargetFunction;
	LuaScriptClass					*Script;
	bool								Persistable;

	typedef std::pair<LuaFunction*, LuaScriptClass*> WrapperCachePairType;

   typedef stdext::hash_map<WrapperCachePairType, LuaExternalFunction*, PairHashCompareClass<WrapperCachePairType>, 
		PooledSTLAllocatorClass<std::pair<WrapperCachePairType, LuaExternalFunction*>, 128> > WrapperCacheType;

	static WrapperCacheType *WrapperCache;
};

#endif