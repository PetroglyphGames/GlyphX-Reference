// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GenericSignalBlock.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/GenericSignalBlock.h $
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

#ifndef _GENERIC_SIGNAL_BLOCK_H_
#define _GENERIC_SIGNAL_BLOCK_H_

#include "BlockingStatus.h"
#include "PGSignal/SignalListener.h"
#include "MultiLinkedList.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class GameObjectClass;

class GenericSignalBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:

	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_GENERIC_SIGNAL_BLOCK, GenericSignalBlockStatus);

	GenericSignalBlockStatus() : IsFinished(false) {}

	void Init(LuaUserVar *command, SignalGeneratorClass *gen, PGSignalType signal_type);
	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(IsFinished)); }
	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *) { IsFinished = true; }

protected:

	bool IsFinished;
};

#endif 