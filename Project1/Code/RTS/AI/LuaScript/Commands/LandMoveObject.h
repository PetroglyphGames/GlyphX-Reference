// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LandMoveObject.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LandMoveObject.h $
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
/** @file */


#ifndef __LANDMOVEOBJECT_H__
#define __LANDMOVEOBJECT_H__

#include "PGSignal/SignalListener.h"
#include "PGSignal/SignalData.h"
#include "BlockingStatus.h"
#include "HardPointData.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class GameObjectWrapper;
class GameObjectClass;
class AITargetLocationWrapper;
class HardPointClass;
class AITargetLocationClass;

class LandMovementBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_LAND_MOVEMENT_BLOCK, LandMovementBlockStatus);

	LandMovementBlockStatus();
	virtual ~LandMovementBlockStatus();
	void Init(LuaUserVar *command, GameObjectClass *dest, float threat_tolerance, bool do_zone_path, bool attack, int move_flags);
	void Init(LuaUserVar *command, const Vector3 &pos, float threat_tolerance, bool do_zone_path, int move_flags);
	virtual LuaTable* Is_Finished(LuaScriptClass *, LuaTable *);
	bool Internal_Is_Finished(void);
	virtual bool Save(LuaScriptClass *, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *, ChunkReaderClass *reader);
	virtual LuaTable* Result(LuaScriptClass *, LuaTable *);
	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType type, SignalDataClass *data);
	void Execute_Move(const Vector3 &pos, bool init);
	void Execute_Move(GameObjectClass *target, bool init);
	void Do_Zone_Advance(bool init);

private:

	virtual void Init_Move_List();

	bool												IsFinished;
	SmartPtr<GameObjectClass>					DestinationObject;
	float												ThreatTolerance;
	bool												DoZonePath;
	bool												DoAdvance;
	bool												Attack;
	MultiLinkedListClass<GameObjectClass>	*MoveList;
	MultiLinkedListClass<GameObjectClass>	*Stragglers;
	Vector3											DestinationPosition;
	SmartPtr<AITargetLocationClass>			StartZone;
	SmartPtr<AITargetLocationClass>			EndZone;
	int												MoveFlags;
};


#endif // __LANDMOVEOBJECT_H__
