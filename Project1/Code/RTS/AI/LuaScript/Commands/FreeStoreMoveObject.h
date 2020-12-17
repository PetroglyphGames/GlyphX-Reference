// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FreeStoreMoveObject.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FreeStoreMoveObject.h $
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


#ifndef __FREESTOREMOVEOBJECT_H__
#define __FREESTOREMOVEOBJECT_H__

#include "PGSignal/SignalListener.h"
#include "PGSignal/SignalData.h"
#include "BlockingStatus.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class GameObjectWrapper;
class GameObjectClass;
class PlayerClass;

class FreeStoreMovementBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_FREE_MOVEMENT_BLOCK, FreeStoreMovementBlockStatus);

	FreeStoreMovementBlockStatus();
	GameObjectClass *Get_Fleet_Object(void) const;
	void Init(GameObjectWrapper *fleet, GameObjectWrapper *dest, PlayerClass *player, float threat_threshold);
	void Add_Merge_Object(GameObjectWrapper *obj);
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
	PlayerClass *									Player;
	float												ThreatThreshold;
};


#endif // __FREESTOREMOVEOBJECT_H__
