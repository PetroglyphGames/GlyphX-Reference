
// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/GameObjectWrapper.h#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/GameObjectWrapper.h $
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

#ifndef _GAME_OBJECT_WRAPPER_H_
#define _GAME_OBJECT_WRAPPER_H_

#include "LuaRTSUtilities.h"

#include "PGSignal/SignalListener.h"
#include "Assert.h"
#include "PairHashCompare.h"

class GameObjectClass;
class GameObjectTypeWrapper;
class PlayerWrapper;
class PositionWrapper;

class GameObjectWrapper : public LuaUserVar, public SignalListenerClass, public PooledObjectClass<GameObjectWrapper, LUA_WRAPPER_POOL_SIZE>
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_GAMEOBJECT_WRAPPER, GameObjectWrapper);

	GameObjectWrapper();
	~GameObjectWrapper();

	static GameObjectWrapper *Create(GameObjectClass *obj, LuaScriptClass *script, bool persistable = true);
	void Init(GameObjectClass *object);
	GameObjectClass *Get_Object(void) const;
	LuaTable* Release(LuaScriptClass * /*script*/, LuaTable * /*params*/);
	virtual void To_String(std::string &outstr);
	LuaTable* Lua_Is_Transport(LuaScriptClass * /*script*/, LuaTable * /*params*/);
   
	virtual bool Is_Equal(const LuaVar *var) const;

	LuaTable* Get_Owner(LuaScriptClass *script, LuaTable *);
	LuaTable* Lua_Set_Prefer_Ground_Over_Space(LuaScriptClass *script, LuaTable *params);
	LuaTable* Lua_Get_Type(LuaScriptClass *script, LuaTable *);
	LuaTable *Lua_Get_Game_Scoring_Type(LuaScriptClass *script, LuaTable *);
	LuaTable* Get_Hull(LuaScriptClass *, LuaTable *);
	LuaTable* Get_Shield(LuaScriptClass *, LuaTable *);
	LuaTable* Get_Energy(LuaScriptClass *, LuaTable *);
	LuaTable* Get_Rate_Of_Damage_Taken(LuaScriptClass *, LuaTable *);
	LuaTable* Get_Time_Till_Dead(LuaScriptClass *, LuaTable *);
	LuaTable* Fire_Special_Weapon(LuaScriptClass *, LuaTable *);
	LuaTable* Is_Category(LuaScriptClass *, LuaTable *);
	LuaTable* Get_Parent_Object(LuaScriptClass *, LuaTable *);
	LuaTable* Attack_Target(LuaScriptClass *script, LuaTable *params);
	LuaTable* Set_Targeting_Priorities(LuaScriptClass *script, LuaTable *params);
	LuaTable* Set_Targeting_Stickiness_Time_Threshold(LuaScriptClass *script, LuaTable *params);
	LuaTable* Move_To(LuaScriptClass *script, LuaTable *params);
	LuaTable* Guard_Target(LuaScriptClass *script, LuaTable *params);
	LuaTable* Attack_Move(LuaScriptClass *script, LuaTable *params);
	LuaTable* Contains_Hero(LuaScriptClass *script, LuaTable *params);
	LuaTable* Get_Contained_Heroes(LuaScriptClass *script, LuaTable *params);
	LuaTable* Are_Engines_Online(LuaScriptClass *script, LuaTable *params);
	LuaTable* Get_Distance(LuaScriptClass *script, LuaTable *params);
	LuaTable* Get_Build_Pad_Contents(LuaScriptClass *script, LuaTable *params);
	LuaTable* Sell(LuaScriptClass *script, LuaTable *params);
	LuaTable* Get_Starbase_Level(LuaScriptClass *script, LuaTable *);
	LuaTable *Get_Final_Blow_Player(LuaScriptClass *, LuaTable *);
	LuaTable *Lock_Current_Orders(LuaScriptClass *, LuaTable *);
	LuaTable *Event_Object_In_Range(LuaScriptClass *script, LuaTable *params);
	LuaTable *Cancel_Event_Object_In_Range(LuaScriptClass *script, LuaTable *params);
	LuaTable *Service_Wrapper(LuaScriptClass *script, LuaTable *params);
	LuaTable *Lua_Get_Position(LuaScriptClass *script, LuaTable *);
	LuaTable *Prevent_AI_Usage(LuaScriptClass *script, LuaTable *params);
	LuaTable *Set_Importance(LuaScriptClass *script, LuaTable *params);
	LuaTable *Take_Damage(LuaScriptClass *script, LuaTable *params);
	LuaTable *Despawn(LuaScriptClass *script, LuaTable *params);
	LuaTable *Mark_Parent_Mode_Object_For_Death(LuaScriptClass *script, LuaTable *);
	LuaTable *Set_Selectable(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Next_Starbase_Type(LuaScriptClass *script, LuaTable *);
	LuaTable *Change_Owner(LuaScriptClass *script, LuaTable *params);
	LuaTable *Divert(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_AI_Power_Vs_Unit(LuaScriptClass *script, LuaTable *params);
	LuaTable *Has_Active_Orders(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Contained_Object_Count(LuaScriptClass *script, LuaTable *params);
	LuaTable *Contains_Object_Type(LuaScriptClass *script, LuaTable *params);
	LuaTable *Destroy_Contained_Objects(LuaScriptClass *script, LuaTable *params);
	LuaTable *Has_Ability(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_Ability_Ready(LuaScriptClass *script, LuaTable *params);
	LuaTable *Activate_Ability(LuaScriptClass *script, LuaTable *params);
	LuaTable *Set_Single_Ability_Autofire(LuaScriptClass *script, LuaTable *params);
	LuaTable *Set_All_Abilities_Autofire(LuaScriptClass *script, LuaTable *params);
	LuaTable *Has_Property(LuaScriptClass *script, LuaTable *params);
	LuaTable *Unlock_Current_Orders(LuaScriptClass *, LuaTable *);
	LuaTable *Play_Animation(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_On_Diversion(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Affiliated_Indigenous_Type(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_Planet_Destroyed(LuaScriptClass *script, LuaTable *params);
	LuaTable *Turn_To_Face(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_Ability_Active(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_In_Nebula(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_In_Ion_Storm(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_In_Asteroid_Field(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_Under_Effects_Of_Ability(LuaScriptClass *script, LuaTable *params);
	LuaTable *Build(LuaScriptClass *script, LuaTable *params);
	LuaTable *Make_Invulnerable(LuaScriptClass *script, LuaTable *params);
	LuaTable *Teleport(LuaScriptClass *script, LuaTable *params);
	LuaTable *Teleport_And_Face(LuaScriptClass *script, LuaTable *params);
	LuaTable *Hyperspace_Away(LuaScriptClass *script, LuaTable *params);
	LuaTable *Cinematic_Hyperspace_In(LuaScriptClass *script, LuaTable *params);
	LuaTable *Cancel_Hyperspace(LuaScriptClass *script, LuaTable *params);
	LuaTable *Lock_Build_Pad_Contents(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Bone_Position(LuaScriptClass *script, LuaTable *params);
	LuaTable *Fire_Tactical_Superweapon(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_Tactical_Superweapon_Ready(LuaScriptClass *script, LuaTable *params);
	LuaTable *Set_Garrison_Spawn(LuaScriptClass *script, LuaTable *params);
	LuaTable *Prevent_Opportunity_Fire(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Hint(LuaScriptClass *script, LuaTable *);
	LuaTable *Set_Cannot_Be_Killed(LuaScriptClass *script, LuaTable *params);
	LuaTable *Play_SFX_Event(LuaScriptClass *script, LuaTable *params);
	LuaTable *Stop_SFX_Event(LuaScriptClass *script, LuaTable *params);
	LuaTable *Force_Test_Space_Conflict(LuaScriptClass *script, LuaTable *params);
	LuaTable *Hide(LuaScriptClass *script, LuaTable *params);
	LuaTable *Face_Immediate(LuaScriptClass *script, LuaTable *params);
	LuaTable *Reset_Ability_Counter(LuaScriptClass *script, LuaTable *params);
	LuaTable *Prevent_All_Fire(LuaScriptClass *script, LuaTable *params);
	LuaTable *Disable_Capture(LuaScriptClass *script, LuaTable *params);
	LuaTable *Suspend_Locomotor(LuaScriptClass *script, LuaTable *params);
	LuaTable *Explore_Area(LuaScriptClass *script, LuaTable *params);
	LuaTable *Highlight(LuaScriptClass *script, LuaTable *params);
	LuaTable *Highlight_Small(LuaScriptClass *script, LuaTable *params);
	LuaTable *Show_Emitter(LuaScriptClass *script, LuaTable *params);
	LuaTable *Has_Attack_Target(LuaScriptClass *script, LuaTable *params);
	LuaTable *Stop(LuaScriptClass *script, LuaTable *params);
	LuaTable *Override_Max_Speed(LuaScriptClass *script, LuaTable *params);
	LuaTable *Attach_Particle_Effect(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Planet_Location(LuaScriptClass *script, LuaTable *params);
	LuaTable *In_End_Cinematic(LuaScriptClass *script, LuaTable *params);
	LuaTable *Play_Cinematic_Engine_Flyby(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Is_Planet_AI_Usable(LuaScriptClass *script, LuaTable *params);
	LuaTable *Enable_Behavior(LuaScriptClass *script, LuaTable *params);
	LuaTable *Cancel_Ability(LuaScriptClass *script, LuaTable *params);
	LuaTable *Can_Land_On_Planet(LuaScriptClass *script, LuaTable *params);
	LuaTable *Set_Check_Contested_Space(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Attack_Target(LuaScriptClass *, LuaTable *);
	LuaTable *Can_Garrison(LuaScriptClass *script, LuaTable *params);
	LuaTable *Garrison(LuaScriptClass *script, LuaTable *params);
	LuaTable *Can_Garrison_Fire(LuaScriptClass *script, LuaTable *params);
	LuaTable *Leave_Garrison(LuaScriptClass *script, LuaTable *params);
	LuaTable *Eject_Garrison(LuaScriptClass *script, LuaTable *params);
	LuaTable *Has_Garrison(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Garrisoned_Units(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_In_Garrison(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_Good_Against(LuaScriptClass *script, LuaTable *params);
	LuaTable *Should_Switch_Weapons(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_Current_Projectile_Type(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_Selectable(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_Ability_Autofire(LuaScriptClass *script, LuaTable *params);
	LuaTable *Get_All_Projectile_Types(LuaScriptClass *script, LuaTable *params);
	LuaTable *Set_In_Limbo(LuaScriptClass *script, LuaTable *params);
	LuaTable *Invade(LuaScriptClass *script, LuaTable *params);
	LuaTable *Can_Move(LuaScriptClass *script, LuaTable *params);
	LuaTable *Enable_Dynamic_LOD(LuaScriptClass *script, LuaTable *params);
	LuaTable *Force_Ability_Recharge(LuaScriptClass *script, LuaTable *params);
	LuaTable *Is_Corrupted(LuaScriptClass *script, LuaTable *params);
	
	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);
	void Post_Load_Callback(void);

	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *);
	virtual bool Does_Listener_Persist(void) const { return Persistable; }

	LuaTable *Is_Valid(LuaScriptClass *, LuaTable *)
	{ 
		Debug_Validate_Wrapper_Cache();
		return Return_Variable(new LuaBool(Object != 0));
	}

	static void Init_Wrapper_Cache(void);
	static void Shutdown_Wrapper_Cache(void);

	bool Activate_Ability_Internal(LuaScriptClass *script, LuaTable *params);
	LuaTable* Activate_Ability_Galactic(LuaScriptClass *script, LuaTable *params);

	virtual LuaTable *Is_Pool_Safe(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(false)); }

private:

	void Remove_Cached_Wrapper(void);
	static void Debug_Validate_Wrapper_Cache(void);
	LuaTable* Move_Internal(LuaScriptClass *script, LuaTable *params, unsigned int move_flags);
	LuaTable* Move_Galactic(LuaScriptClass *script, LuaTable *params);

	virtual GameObjectWrapper &operator=(const GameObjectWrapper &) { assert(false); return *this; }

	struct ObjectInRangeItem {
		ObjectInRangeItem();
		LuaNumber::Pointer							Distance;
		LuaFunction::Pointer							Function;
		SmartPtr<GameObjectTypeWrapper>			Type;
		SmartPtr<PlayerWrapper>						Player;
		bool												EventSent;
	};

	bool													ObjectInRangeListModified;
	std::vector<ObjectInRangeItem>				ObjectInRangeList;
	SmartPtr<PositionWrapper>						Position;
	SmartPtr<GameObjectClass>						Object;
	LuaScriptClass										*Script;
	bool													Persistable;

	typedef std::pair<GameObjectClass *, LuaScriptClass *> WrapperCachePairType;

	typedef stdext::hash_map<WrapperCachePairType, GameObjectWrapper *, PairHashCompareClass<WrapperCachePairType>> WrapperCacheType;

	static WrapperCacheType *WrapperCache;
};

#endif

