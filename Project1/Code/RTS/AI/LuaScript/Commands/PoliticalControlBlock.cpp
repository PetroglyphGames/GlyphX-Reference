// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/PoliticalControlBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/PoliticalControlBlock.cpp $
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

// This module is deprecated...
#pragma hdrstop

#if 0
#include "AI/LuaScript/Commands/PoliticalControlBlock.h"

#include "GameObject.h"
#include "PlanetaryBehavior.h"
#include "AI/LuaScript/GameObjectWrapper.h"


PoliticalControlBlockStatus::PoliticalControlBlockStatus() : Control(0)
{
}

/**
 * Initializes the blocking status object
 * 
 * @param planet					The planet we're waiting to control.
 * @param control					The level of control we're waiting for.  
 *										+ve is friendly control, -ve enemy control - NOT good/evil.
 * @param waiting_allegiance		The allegiance of the waiting object so that we know
 *										how to interpret the control parameter.
 * @since 5/18/2004 11:40AM -- JSY
 */
void PoliticalControlBlockStatus::Init(GameObjectClass *planet, const double &control, const AllegianceClass &waiting_allegiance)
{
	Planet = planet;
	Control = control;
	WaitingAllegiance = waiting_allegiance;
}

/**
 * Checks whether to stop blocking
 * 
 * @param script		Unused
 * @param params		Unused
 *
 * @return				Whether the waiting object has achieved the desired level of political
 *							control over the planet.
 * @since 5/18/2004 11:40AM -- JSY
 */
LuaTable* PoliticalControlBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{	
	if (Planet->Get_Allegiance().Get_Current_Allegiance() >= Control)
	{
		bool aligned = Planet->Get_Allegiance().Is_Aligned_With(WaitingAllegiance);
		return Return_Variable(new LuaBool(Control > 0 ? aligned : !aligned));
	}

	return Return_Variable(new LuaBool(false));
}

LuaTable* PoliticalControlBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return NULL;
}

enum {
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_POLITICAL_BLOCK_DATA,
	CHUNK_ID_POLITICAL_BLOCK_PLANET,
	CHUNK_ID_POLITICAL_BLOCK_CONTROL,
	CHUNK_ID_POLITICAL_BLOCK_ALLEGIANCE,
};

bool PoliticalControlBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();
	
	ok &= writer->Begin_Chunk(CHUNK_ID_POLITICAL_BLOCK_DATA);
		WRITE_MICRO_CHUNK_OBJECT_PTR(CHUNK_ID_POLITICAL_BLOCK_PLANET, Planet);
		WRITE_MICRO_CHUNK (	CHUNK_ID_POLITICAL_BLOCK_CONTROL, Control);
		WRITE_MICRO_CHUNK	(	CHUNK_ID_POLITICAL_BLOCK_ALLEGIANCE, WaitingAllegiance);
	ok &= writer->End_Chunk();

	return (ok);
}

bool PoliticalControlBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;
	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case CHUNK_ID_BASE_CLASS:
				ok &= BlockingStatus::Load(script, reader);
				break;

			case CHUNK_ID_POLITICAL_BLOCK_DATA:
				while (reader->Open_Micro_Chunk()) {
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK_OBJECT_PTR(CHUNK_ID_POLITICAL_BLOCK_PLANET, Planet);
						READ_MICRO_CHUNK (	CHUNK_ID_POLITICAL_BLOCK_CONTROL, Control);
						READ_MICRO_CHUNK	(	CHUNK_ID_POLITICAL_BLOCK_ALLEGIANCE, WaitingAllegiance);
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;
			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}
	return (ok);
}

LUA_IMPLEMENT_FACTORY(LUA_CHUNK_POLITICAL_CONTROL_BLOCK, PoliticalControlBlockStatus);
PG_IMPLEMENT_RTTI(PoliticalControlBlockStatus, BlockingStatus);

#endif