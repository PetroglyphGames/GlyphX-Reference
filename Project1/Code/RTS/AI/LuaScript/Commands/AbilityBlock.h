// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/AbilityBlock.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/AbilityBlock.h $
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

#ifndef _ABILITY_BLOCK_H_
#define _ABILITY_BLOCK_H_

#include "AI/LuaScript/LuaRTSUtilities.h"
#include "BlockingStatus.h"
#include "PGSignal/SignalListener.h"
#include "Vector.h"

class GameObjectClass;

class AbilityBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_ABILITY_BLOCK, AbilityBlockStatus);

	AbilityBlockStatus() : UnfinishedAbilityCount(0) {}

	void Init(LuaUserVar *command);
	void Add_Object(GameObjectClass *object);
	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(UnfinishedAbilityCount == 0)); }
	virtual LuaTable *Result(LuaScriptClass *, LuaTable *) { return NULL; }

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *);

private:

	int UnfinishedAbilityCount;
};

#endif //_ABILITY_BLOCK_H_