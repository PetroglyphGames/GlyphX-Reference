// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ReinforceBlock.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ReinforceBlock.h $
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

#ifndef _REINFORCE_BLOCK_H_
#define _REINFORCE_BLOCK_H_

#include "BlockingStatus.h"
#include "PGSignal/SignalListener.h"
#include "GameObjectType.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class ReinforceBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_REINFORCE_BLOCK, ReinforceBlockStatus);

	ReinforceBlockStatus() : TransportCount(0), CanPlanBeAbandoned(true), ResultValue(false), Script(NULL), LandPoint(VECTOR3_INVALID) {}
	~ReinforceBlockStatus();

	void Remove_Land_Point(void);
	void Init(LuaUserVar *command, int transport_count, bool result, LuaScriptClass *script, const Vector3 &land_point);
	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *);

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

	virtual LuaTable *Result(LuaScriptClass *, LuaTable *);
	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *);

private:

	int	TransportCount;
	bool	CanPlanBeAbandoned;
	bool	ResultValue;
	LuaTable::Pointer ReinforceUnits;
	LuaScriptClass *Script;
	Vector3 LandPoint;
};

class LuaReinforceCommandClass : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *params);
	Vector3 Find_Reinforcement_Landing_Location(PlayerClass *player, const Vector3 &target_position, float distance_to_check, const GameObjectTypeClass *type, bool obey_zones);
	static std::vector<Vector3> LandingLocations;
};


#endif //_REINFORCE_BLOCK_H_