// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/StoryPlotWrapper.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/StoryPlotWrapper.h $
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

#ifndef _STORY_PLOT_WRAPPER_H_
#define _STORY_PLOT_WRAPPER_H_

#include "LuaRTSUtilities.h"
#include "PGSignal/SignalListener.h"

class StoryPlotWrapper : public LuaUserVar, public SignalListenerClass, public PooledObjectClass<StoryPlotWrapper, LUA_WRAPPER_POOL_SIZE>
{
public:

	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_STORYPLOT_WRAPPER, StoryPlotWrapper);

	StoryPlotWrapper();

	static StoryPlotWrapper *Create(StorySubPlotClass *plot, LuaScriptClass *script);

	LuaTable *Get_Event(LuaScriptClass *script, LuaTable *params);
	LuaTable *Activate(LuaScriptClass *script, LuaTable *params);
	LuaTable *Suspend(LuaScriptClass *script, LuaTable *params);
	LuaTable *Reset(LuaScriptClass *script, LuaTable *params);

	LuaTable *Is_Valid(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(Plot != NULL)); }

	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType, SignalDataClass *);

	virtual bool Save(LuaScriptClass *, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *, ChunkReaderClass *reader);

	virtual LuaTable *Is_Pool_Safe(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(false)); }


private:

	StorySubPlotClass *Plot;
	LuaScriptClass *Script;
};

#endif //_STORY_PLOT_WRAPPER_H_
