// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/UnitAnimationBlock.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/UnitAnimationBlock.h $
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

#ifndef _UNIT_ANIMATION_BLOCK_H_
#define _UNIT_ANIMATION_BLOCK_H_

#include "BlockingStatus.h"
#include "ModelAnim.h"
#include "AI/LuaScript/LuaRTSUtilities.h"

//Blocking object for a unit playing an animation.  The block ends when the animation is done or the unit starts
//playing a different animation
class UnitAnimationBlockStatus : public BlockingStatus
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_UNIT_ANIMATION_BLOCK, UnitAnimationBlockStatus);

	UnitAnimationBlockStatus() : AnimationType(ANIM_INVALID) {}
	void Init(LuaUserVar *command,  ModelAnimType animation_type);
	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *);
	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);
	virtual LuaTable *Result(LuaScriptClass *, LuaTable *) { return NULL; }

private:

	ModelAnimType AnimationType;
};

#endif //_UNIT_ANIMATION_BLOCK_H_

