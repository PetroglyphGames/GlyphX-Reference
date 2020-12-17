// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceReinforceBlock.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/SpaceReinforceBlock.h $
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

#ifndef _SPACE_REINFORCE_BLOCK_H_
#define _SPACE_REINFORCE_BLOCK_H_

#include "BlockingStatus.h"
#include "PGSignal/SignalListener.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class SpaceReinforceBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_SPACE_REINFORCE_BLOCK, SpaceReinforceBlockStatus);

	SpaceReinforceBlockStatus();
	void Init(LuaUserVar *command, const std::vector<int> &reinforcement_indices, const Vector3 &target_position, float time_out, LuaScriptClass *script, int player_id);
	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *);
	virtual LuaTable *Result(LuaScriptClass *, LuaTable *);
	void Set_Obey_Reinforce_Rules(bool val) { ObeyReinforceRules = val; }

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *);

protected:

	void Attempt_Reinforcement();

	LuaTable::Pointer ReinforceUnits;
	LuaScriptClass *Script;
	std::vector<int> ReinforcementIndices;
	Vector3 TargetPosition;
	int TimeOutFrame;
	int IncomingShipCount;
	float Radius;
	int PlayerID;
	bool ObeyReinforceRules;
};

#endif //_SPACE_REINFORCE_BLOCK_H_