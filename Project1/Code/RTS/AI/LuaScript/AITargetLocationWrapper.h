// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/AITargetLocationWrapper.h#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/AITargetLocationWrapper.h $
//
//    Original Author: James Yarrow
//
//            $Author: Brian_Hayes $
//
//            $Change: 641508 $
//
//          $DateTime: 2017/05/09 13:57:37 $
//
//          $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __AITARGETLOCATIONWRAPPER_H__
#define __AITARGETLOCATIONWRAPPER_H__

#include "LuaRTSUtilities.h"
#include "PGSignal/SignalListener.h"
#include "PairHashCompare.h"

#include "Assert.h"

class AITargetLocationClass;

class AITargetLocationWrapper : public LuaUserVar, public SignalListenerClass, public PooledObjectClass<AITargetLocationWrapper, LUA_WRAPPER_POOL_SIZE>
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_AI_TARGET_LOCATION_WRAPPER, AITargetLocationWrapper);
	AITargetLocationWrapper();
	~AITargetLocationWrapper();

	static AITargetLocationWrapper *Create(const AITargetLocationClass *obj, LuaScriptClass *script, bool persistable = true);
	void Init(const AITargetLocationClass *object);
	const AITargetLocationClass *Get_Object(void) {return Object;}
	LuaTable *Get_Game_Object(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Distance(LuaScriptClass *script, LuaTable *params);

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);
	void Post_Load_Callback(void);

	virtual bool Is_Equal(const LuaVar *var) const;

	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *);

	virtual void To_String(std::string &outstr);

	LuaTable *Is_Valid(LuaScriptClass *, LuaTable*) { return Return_Variable(new LuaBool(Object != 0)); }

	static void Init_Wrapper_Cache(void);
	static void Shutdown_Wrapper_Cache(void);
	virtual bool Does_Listener_Persist(void) const { return Persistable; }

	virtual LuaTable *Is_Pool_Safe(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(false)); }

private:

	virtual AITargetLocationWrapper &operator=(const AITargetLocationWrapper &) { assert(false); return *this; }

	void Remove_Cached_Wrapper(void);
	SmartPtr<AITargetLocationClass> Object;
	LuaScriptClass									*Script;
	bool												Persistable;

	typedef std::pair<AITargetLocationClass *, LuaScriptClass *> WrapperCachePairType;

   typedef stdext::hash_map<WrapperCachePairType, AITargetLocationWrapper *, PairHashCompareClass<WrapperCachePairType>> WrapperCacheType;

	static WrapperCacheType *WrapperCache;
};

#endif // __AITARGETLOCATIONWRAPPER_H__


