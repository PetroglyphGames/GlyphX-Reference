// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/PositionWrapper.h#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/PositionWrapper.h $
//
//    Original Author: James Yarrow
//
//            $Author: Brian_Hayes $
//
//            $Change: 747268 $
//
//          $DateTime: 2020/10/27 14:46:31 $
//
//          $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */


#ifndef _POSITION_WRAPPER_H_
#define _POSITION_WRAPPER_H_

#include "LuaRTSUtilities.h"
#include "Vector3.h"

class PositionWrapper : public LuaUserVar, public PooledObjectClass<PositionWrapper, LUA_WRAPPER_POOL_SIZE>
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_POSITION_WRAPPER, PositionWrapper);

	PositionWrapper();

	static PositionWrapper *Create(const Vector3 &position);
	const Vector3 &Get_Position(void) const	{ return Position; }
	void Init(const Vector3 &position)			{ Position = position; }
	void Set_Position(const Vector3 &position){ Position = position; }

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

	virtual bool Is_Equal(const LuaVar *var) const;

	LuaTable *Is_Valid(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(Position != VECTOR3_INVALID)); }
	LuaTable *Get_XYZ(LuaScriptClass *, LuaTable *);

private:
	Vector3				Position;
};

#endif //_POSITION_WRAPPER_H_