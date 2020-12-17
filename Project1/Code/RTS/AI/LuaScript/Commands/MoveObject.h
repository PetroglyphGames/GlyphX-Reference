// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/MoveObject.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/MoveObject.h $
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


#ifndef __MOVEOBJECT_H__
#define __MOVEOBJECT_H__

#include "PGSignal/SignalListener.h"
#include "PGSignal/SignalData.h"
#include "BlockingStatus.h"
#include "AI/Goal/AIGoalReachabilityType.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class GameObjectWrapper;
class GameObjectClass;

class MovementBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_MOVEMENT_BLOCK, MovementBlockStatus);

	MovementBlockStatus();
	void Init(LuaUserVar *command, GameObjectWrapper *fleet, GameObjectWrapper *dest, AIGoalReachabilityType reachability);
	virtual LuaTable* Is_Finished(LuaScriptClass *, LuaTable *);
	bool Internal_Is_Finished(void);
	virtual bool Save(LuaScriptClass *, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *, ChunkReaderClass *reader);
	virtual LuaTable* Result(LuaScriptClass *, LuaTable *);
	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType type, SignalDataClass *data);

private:
	const GameObjectClass *Get_Next_Hop(void);
	bool Advance(void);

	bool												IsFinished;
	bool												DoAdvance;
	SmartPtr<GameObjectWrapper>				ResultObject; // return a pointer to the fleet
	SmartPtr<GameObjectWrapper>				Destination;
	AIGoalReachabilityType						Reachability;
};


#endif // __MOVEOBJECT_H__
