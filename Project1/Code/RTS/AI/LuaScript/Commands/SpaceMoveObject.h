// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceMoveObject.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceMoveObject.h $
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


#ifndef __SPACEMOVEOBJECT_H__
#define __SPACEMOVEOBJECT_H__

#include "PGSignal/SignalListener.h"
#include "PGSignal/SignalData.h"
#include "BlockingStatus.h"
#include "HardPointData.h"
#include "AI/Movement/SpaceCollisionType.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class GameObjectWrapper;
class GameObjectClass;
class AITargetLocationWrapper;
class HardPointClass;

class SpaceMovementBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_SPACE_MOVEMENT_BLOCK, SpaceMovementBlockStatus);

	SpaceMovementBlockStatus();
	~SpaceMovementBlockStatus();
	void Init(LuaUserVar *command, AITargetLocationWrapper *dest, bool attack, bool repeat, HardPointType hard_point_type, float threat_tolerance, int move_flags, SpaceCollisionType avoidance);
	void Init(LuaUserVar *command, GameObjectClass *dest, bool attack, bool repeat, HardPointType hard_point_type, float threat_tolerance, int move_flags, SpaceCollisionType avoidance);
	void Init(LuaUserVar *command, const Vector3 &pos, float threat_tolerance, int move_flags, SpaceCollisionType avoidance);
	virtual LuaTable* Is_Finished(LuaScriptClass *, LuaTable *);
	bool Internal_Is_Finished(void);
	virtual bool Save(LuaScriptClass *, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *, ChunkReaderClass *reader);
	virtual LuaTable* Result(LuaScriptClass *, LuaTable *);
	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType type, SignalDataClass *data);
	void Execute_Move(const Vector3 &pos, bool init);
	void Execute_Move(GameObjectClass *target, bool init, HardPointType hard_point_type);

private:

	const HardPointClass *Find_Best_Hard_Point(HardPointType hard_point_type);

	void Init_Move_List();

	bool												IsFinished;
	bool												Attack;
	bool												Repeat;
	SmartPtr<GameObjectWrapper>				ResultObject; // return a pointer to the object
	SmartPtr<GameObjectClass>					Destination;
	const HardPointClass							*HardPoint;
	float												ThreatTolerance;
	int												MoveFlags;
	SpaceCollisionType							Avoidance;
	MultiLinkedListClass<GameObjectClass>	*Stragglers;
	MultiLinkedListClass<GameObjectClass>	*MoveList;
};


#endif // __SPACEMOVEOBJECT_H__
