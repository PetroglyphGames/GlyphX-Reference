// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/BombingBlock.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/BombingBlock.h $
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

#ifndef _BOMBING_BLOCK_H_
#define _BOMBING_BLOCK_H_

#include "BlockingStatus.h"
#include "AI/LuaScript/LuaRTSUtilities.h"
#include "PGSignal/SignalListener.h"
#include "Vector.h"

class GameObjectClass;

class BombingBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_BOMBING_BLOCK, BombingBlockStatus);

	BombingBlockStatus() : OriginalBomberCount(0), SurvivingBomberCount(0), ActiveBomberCount(0) {}

	void Init(LuaUserVar *command, int player_id, const DynamicVectorClass<GameObjectClass*> &bombers);
	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *);
	virtual LuaTable *Result(LuaScriptClass *, LuaTable *);

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *);

private:

	int	OriginalBomberCount;
	int	SurvivingBomberCount;
	int	ActiveBomberCount;
};

#endif //_BOMBING_BLOCK_H_