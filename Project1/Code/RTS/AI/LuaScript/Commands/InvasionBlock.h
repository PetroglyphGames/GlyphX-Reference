// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/InvasionBlock.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/InvasionBlock.h $
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

#ifndef __INVASIONBLOCK_H__
#define __INVASIONBLOCK_H__



#include "BlockingStatus.h"
#include "AI/LuaScript/LuaRTSUtilities.h"
#include "PGSignal/SignalListener.h"
#include "PGSignal/SignalData.h"

class GameObjectClass;
class TaskForceClass;

//Signal data indicating the result of an invasion.
//Success == true is a win for the invaders.
class InvasionResultClass : public SignalDataClass
{
public:
	PG_DECLARE_RTTI();
	bool Success;
};

//Blocking status object to wait on an invasion
class InvasionBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:

	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_INVASION_BLOCK, InvasionBlockStatus);

	InvasionBlockStatus();
	void Init(TaskForceClass *command, GameObjectClass *planet);
	virtual LuaTable* Is_Finished(LuaScriptClass *script, LuaTable *params);
	virtual bool Save(LuaScriptClass *, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *, ChunkReaderClass *reader);
	virtual LuaTable* Result(LuaScriptClass *, LuaTable *);
	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType type, SignalDataClass *data);

private:

	bool Success;
	bool IsFinished;
};

#endif //__INVASIONBLOCK_H__