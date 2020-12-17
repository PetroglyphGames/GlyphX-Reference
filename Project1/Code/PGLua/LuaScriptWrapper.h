// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScriptWrapper.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScriptWrapper.h $
//
//    Original Author: Brian Hayes
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

#ifndef __LUASCRIPTWRAPPER_H__
#define __LUASCRIPTWRAPPER_H__

#include "PGSignal/SignalListener.h"
#include "PairHashCompare.h"
#include "LuaScriptVariable.h"

#include "Assert.h"

class LuaScriptClass;

class LuaScriptWrapper : public LuaUserVar, public SignalListenerClass, public PooledObjectClass<LuaScriptWrapper, LUA_WRAPPER_POOL_SIZE>
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_LUA_SCRIPT_WRAPPER, LuaScriptWrapper);
	LuaScriptWrapper();
	~LuaScriptWrapper();

	static LuaScriptWrapper *Create(const LuaScriptClass *obj, LuaScriptClass *script, bool persistable = true);
	void Init(const LuaScriptClass *object);
	const LuaScriptClass *Get_Object(void) {return Object;}

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);
	void Post_Load_Callback(void);

	virtual bool Is_Equal(const LuaVar *var) const;
	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *);
	virtual void To_String(std::string &outstr);

	static void Init_Wrapper_Cache(void);
	static void Shutdown_Wrapper_Cache(void);
	virtual bool Does_Listener_Persist(void) const { return Persistable; }
	virtual LuaVar *Map_Into_Other_Script(LuaScriptClass *new_script);

	virtual LuaTable *Is_Pool_Safe(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(false)); }

	LuaTable *Is_Valid(LuaScriptClass *, LuaTable*) { return Return_Variable(new LuaBool(Object != 0)); }
	LuaTable *Call_Function(LuaScriptClass *script, LuaTable *params);
	LuaTable *Set_Variable(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Variable(LuaScriptClass *script, LuaTable *params);

private:

	virtual LuaScriptWrapper &operator=(const LuaScriptWrapper &) { assert(false); return *this; }

	void Remove_Cached_Wrapper(void);
	SmartPtr<LuaScriptClass> Object;
	LuaScriptClass									*Script;
	bool												Persistable;

	typedef std::pair<LuaScriptClass *, LuaScriptClass *> WrapperCachePairType;

   typedef stdext::hash_map<WrapperCachePairType, LuaScriptWrapper *, PairHashCompareClass<WrapperCachePairType>, 
		PooledSTLAllocatorClass<std::pair<WrapperCachePairType, LuaScriptWrapper *>, 128> > WrapperCacheType;

	static WrapperCacheType *WrapperCache;
};

#endif // __LUASCRIPTWRAPPER_H__


