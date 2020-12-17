// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LandUnits.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LandUnits.h $
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

#ifndef __LANDUNITS_H__
#define __LANDUNITS_H__


#include "PGSignal/SignalListener.h"
#include "PGSignal/SignalData.h"
#include "BlockingStatus.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

class GameObjectWrapper;
class GameObjectClass;

class LandUnitsBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_LAND_UNITS_BLOCK, LandUnitsBlockStatus);

	LandUnitsBlockStatus();
	void Init(LuaUserVar *command, GameObjectWrapper *fleet, bool land_units = true);
	void Add_Merge_Object(GameObjectClass *obj, bool land_units = true);
	virtual LuaTable* Is_Finished(LuaScriptClass *, LuaTable *);
	virtual bool Save(LuaScriptClass *, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *, ChunkReaderClass *reader);
	virtual LuaTable* Result(LuaScriptClass *, LuaTable *);
	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType type, SignalDataClass *data);
	bool Internal_Is_Finished(void);

private:
	std::vector<int>								MergeObjectIDs;
	SmartPtr<GameObjectWrapper>				ResultObject; // return a pointer to the fleet
};


#endif // __LANDUNITS_H__