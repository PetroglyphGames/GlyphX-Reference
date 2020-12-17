// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ExploreAreaBlock.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ExploreAreaBlock.h $
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

#ifndef _EXPLORE_AREA_BLOCK_H_
#define _EXPLORE_AREA_BLOCK_H_

#include "BlockingStatus.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class AITargetLocationClass;

class ExploreAreaBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_EXPLORE_AREA_BLOCK, ExploreAreaBlockStatus);

	ExploreAreaBlockStatus();
	~ExploreAreaBlockStatus();
	void Init(LuaUserVar *command, const AITargetLocationClass *target);
	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *);

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

	virtual void Receive_Signal(SignalGeneratorClass *gen, PGSignalType signal_type, SignalDataClass *);

private:

	void Init_Move_List();
	void Move_To_New_Destination();
	void Add_Object(GameObjectClass *object);

	MultiLinkedListClass<GameObjectClass>	*MoveList;
	const AITargetLocationClass				*Target;
	bool												NeedsNewDestination;
	bool												IsFinished;
	int												StartTime;
};

#endif //_EXPLORE_AREA_BLOCK_H_