// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/GameObjectTypeWrapper.h#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/GameObjectTypeWrapper.h $
//
//    Original Author: Brian Hayes
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
/** @file */


#ifndef __GAME_OBJECT_TYPE_WRAPPER_H__
#define __GAME_OBJECT_TYPE_WRAPPER_H__

#include "LuaRTSUtilities.h"
#include "PairHashCompare.h"

class GameObjectTypeClass;

class GameObjectTypeWrapper : public LuaUserVar, public PooledObjectClass<GameObjectTypeWrapper, LUA_WRAPPER_POOL_SIZE>
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_GAMEOBJECTTYPE_WRAPPER, GameObjectTypeWrapper);

	GameObjectTypeWrapper();
	~GameObjectTypeWrapper();

	static GameObjectTypeWrapper *Create(GameObjectTypeClass *obj, LuaScriptClass *script);
	void Init(GameObjectTypeClass *object);
	GameObjectTypeClass *Get_Object(void) const;

	LuaTable *Get_Build_Cost(LuaScriptClass *, LuaTable *);
	LuaTable *Get_Combat_Rating(LuaScriptClass *, LuaTable *);
	LuaTable *Lua_Is_Hero(LuaScriptClass *, LuaTable *);
	LuaTable *Lua_Get_Name(LuaScriptClass *, LuaTable *);
	LuaTable *Get_Base_Level(LuaScriptClass *, LuaTable *);
	LuaTable *Get_Tech_Level(LuaScriptClass *, LuaTable *);
	LuaTable *Is_Affiliated_With(LuaScriptClass *, LuaTable *);
	LuaTable *Is_Build_Locked(LuaScriptClass *, LuaTable *);
	LuaTable *Is_Obsolete(LuaScriptClass *, LuaTable *);
	LuaTable *Get_Tactical_Build_Cost(LuaScriptClass *, LuaTable *);
	LuaTable *Get_Score_Cost_Credits(LuaScriptClass *, LuaTable *);
	LuaTable *Get_Max_Range(LuaScriptClass *, LuaTable *);
	LuaTable *Get_Min_Range(LuaScriptClass *, LuaTable *);
	LuaTable *Get_Bribe_Cost(LuaScriptClass *, LuaTable *);
	LuaTable *Is_Affected_By_Missile_Shield(LuaScriptClass *, LuaTable *);
	LuaTable *Is_Affected_By_Laser_Defense(LuaScriptClass *, LuaTable *);

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

	virtual bool Is_Equal(const LuaVar *var) const;

	LuaTable *Is_Valid(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(Object != 0)); }

	static void Init_Wrapper_Cache(void);
	static void Shutdown_Wrapper_Cache(void);

	virtual LuaTable *Is_Pool_Safe(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(false)); }

private:
	void Remove_Cached_Wrapper(void);
	SmartPtr<GameObjectTypeClass>				Object;
	LuaScriptClass									*Script;

	typedef std::pair<GameObjectTypeClass *, LuaScriptClass *> WrapperCachePairType;

	typedef stdext::hash_map<WrapperCachePairType, GameObjectTypeWrapper *, PairHashCompareClass<WrapperCachePairType>> WrapperCacheType;

	static WrapperCacheType *WrapperCache;
};

#endif //__GAME_OBJECT_TYPE_WRAPPER_H__
