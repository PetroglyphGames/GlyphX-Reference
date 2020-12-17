// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ProduceForce.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ProduceForce.h $
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

#ifndef __PRODUCEFORCE_H__
#define __PRODUCEFORCE_H__


#include "PGSignal/SignalListener.h"
#include "PGSignal/SignalData.h"
#include "BlockingStatus.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class GameObjectWrapper;
class AIBuildTaskClass;

class ProduceForceBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_PRODUCE_FORCE_BLOCK, ProduceForceBlockStatus);

	ProduceForceBlockStatus();
	~ProduceForceBlockStatus();
	void Init(LuaUserVar *command);
	void Add_Build_Task(AIBuildTaskClass *task);
	virtual LuaTable* Is_Finished(LuaScriptClass *, LuaTable *);
	virtual bool Save(LuaScriptClass *, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *, ChunkReaderClass *reader);
	bool Internal_Is_Finished(void);

private:
	std::vector<SmartPtr<AIBuildTaskClass> >				TaskList;
};


#endif // __PRODUCEFORCE_H__