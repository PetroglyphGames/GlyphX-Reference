// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/PlayerWrapper.h#3 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/PlayerWrapper.h $
//
//    Original Author: Brian Hayes
//
//            $Author: Brian_Hayes $
//
//            $Change: 747267 $
//
//          $DateTime: 2020/10/27 14:46:23 $
//
//          $Revision: #3 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */

#ifndef _PLAYER_WRAPPER_H_
#define _PLAYER_WRAPPER_H_

#include "LuaRTSUtilities.h"
#include "PairHashCompare.h"

class PlayerClass;

class PlayerWrapper : public LuaUserVar, public PooledObjectClass<PlayerWrapper, LUA_WRAPPER_POOL_SIZE>
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_PLAYER_WRAPPER, PlayerWrapper);
	PlayerWrapper();
	~PlayerWrapper();

	static PlayerWrapper *Create(PlayerClass *obj, LuaScriptClass *script);
	void Init(PlayerClass *object);
	PlayerClass *Get_Object(void) const;
	LuaTable* Lua_Is_Neutral(LuaScriptClass *script, LuaTable *);
	LuaTable* Lua_Get_Name(LuaScriptClass *script, LuaTable *);
	LuaTable* Lua_Get_ID(LuaScriptClass *script, LuaTable *);
	LuaTable* Lua_Get_GameSpy_Stats_Player_ID(LuaScriptClass *script, LuaTable *);
	LuaTable* Lua_Give_Money(LuaScriptClass *script, LuaTable *);
	LuaTable* Lua_Set_Tech_Level(LuaScriptClass *script, LuaTable *);
	LuaTable* Lua_Release_Credits_For_Tactical(LuaScriptClass *script, LuaTable *params);
	LuaTable* Lua_Get_Credits(LuaScriptClass *script, LuaTable *params);
	LuaTable* Lua_Get_Enemy(LuaScriptClass *script, LuaTable *params);
	LuaTable* Lua_Get_Faction_Name(LuaScriptClass *script, LuaTable *params);
	LuaTable* Lua_Get_Tech_Level(LuaScriptClass *script, LuaTable *params);
	LuaTable *Lua_Retreat(LuaScriptClass *script, LuaTable *);
	LuaTable *Lua_Give_Random_Sliceable_Tech(LuaScriptClass *, LuaTable *);
	LuaTable *Lua_Unlock_Tech(LuaScriptClass *script, LuaTable *params);
	LuaTable *Lua_Lock_Tech(LuaScriptClass *script, LuaTable *params);
	LuaTable *Lua_Is_Human(LuaScriptClass *, LuaTable *);
	LuaTable *Lua_Enable_As_Actor(LuaScriptClass *, LuaTable *);
	LuaTable *Lua_Is_Ally(LuaScriptClass *script, LuaTable *params);
	LuaTable *Lua_Is_Enemy(LuaScriptClass *script, LuaTable *params);
	LuaTable *Lua_Select_Object(LuaScriptClass *script, LuaTable *params);
	LuaTable *Disable_Bombing_Run(LuaScriptClass *script, LuaTable *params);
	LuaTable *Disable_Orbital_Bombardment(LuaScriptClass *script, LuaTable *params);
	LuaTable *Remove_Orbital_Bombardment(LuaScriptClass *script, LuaTable *params);
	LuaTable *Enable_Advisor_Hints(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Difficulty(LuaScriptClass *script, LuaTable *params);
	LuaTable* Lua_Set_Black_Market_Tutorial(LuaScriptClass *script, LuaTable *params);
	LuaTable* Lua_Set_Sabotage_Tutorial(LuaScriptClass *script, LuaTable *params);
	LuaTable *Make_Enemy(LuaScriptClass *script, LuaTable *params);
	LuaTable *Make_Ally(LuaScriptClass *script, LuaTable *params);
	LuaTable* Lua_Get_Clan_ID(LuaScriptClass *script, LuaTable *);
	LuaTable* Lua_Get_Team(LuaScriptClass *script, LuaTable *);
	LuaTable* Get_Space_Station(LuaScriptClass *script, LuaTable *);

	virtual bool Is_Equal(const LuaVar *lua_var) const;

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);
	void Post_Load_Callback(void);

	virtual LuaTable *Is_Valid(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(Object != 0)); }

	static void Init_Wrapper_Cache(void);
	static void Shutdown_Wrapper_Cache(void);

	virtual LuaTable *Is_Pool_Safe(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(false)); }


private:
	void Remove_Cached_Wrapper(void);

	PlayerClass										*Object;
	LuaScriptClass									*Script;

	typedef std::pair<PlayerClass *, LuaScriptClass *> WrapperCachePairType;

	typedef stdext::hash_map<WrapperCachePairType, PlayerWrapper *, PairHashCompareClass<WrapperCachePairType>> WrapperCacheType;

	static WrapperCacheType *WrapperCache;
};

#endif

