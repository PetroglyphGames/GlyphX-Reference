// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/TacticalBuildBlock.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/TacticalBuildBlock.h $
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

#ifndef _TACTICAL_BUILD_BLOCK_H_
#define _TACTICAL_BUILD_BLOCK_H_

#include "BlockingStatus.h"
#include "PGSignal/SignalListener.h"
#include "MultiLinkedList.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class GameObjectClass;

class TacticalBuildBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:

	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_TACTICAL_BUILD_BLOCK, TacticalBuildBlockStatus);

	TacticalBuildBlockStatus() : PendingBuilds(0) {}

	void Init(LuaUserVar *command);
	void Add_Tactical_Build(GameObjectClass *pad);
	virtual LuaTable *Is_Finished(LuaScriptClass *script, LuaTable *params);
	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *);

protected:

	MultiLinkedListClass<GameObjectClass> *PendingBuilds;
};

#endif //_TACTICAL_BUILD_BLOCK_H_