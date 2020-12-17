// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceAmbushBlock.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceAmbushBlock.h $
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

#ifndef _SPACE_AMBUSH_BLOCK_H_
#define _SPACE_AMBUSH_BLOCK_H_

#include "BlockingStatus.h"
#include "PGSignal/SignalListener.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class GameObjectClass;

class SpaceAmbushBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_SPACE_AMBUSH_BLOCK, SpaceAmbushBlockStatus);

	SpaceAmbushBlockStatus();
	~SpaceAmbushBlockStatus();
	void Init(LuaUserVar *command, GameObjectClass *target, int offset_direction, float offset_distance, float threat_tolerance);
	virtual LuaTable* Is_Finished(LuaScriptClass *, LuaTable *);
	virtual bool Save(LuaScriptClass *, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *, ChunkReaderClass *reader);
	virtual LuaTable* Result(LuaScriptClass *, LuaTable *);
	virtual void Receive_Signal(SignalGeneratorClass *gen, PGSignalType signal_type, SignalDataClass *);

private:

	Vector3 Find_Ambush_Point();
	void Update_Movement(bool init);

	void Init_Move_List();

	int												LastMoveUpdateFrame;
	bool												AmbushReady;
	bool												IsFinished;
	GameObjectClass								*Target;
	int												OffsetDirection;
	float												OffsetDistance;
	float												ThreatTolerance;
	MultiLinkedListClass<GameObjectClass>	*Stragglers;
	MultiLinkedListClass<GameObjectClass>	*MoveList;
};


#endif // _SPACE_AMBUSH_BLOCK_H_