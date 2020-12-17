// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/PositionWrapper.cpp#3 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/PositionWrapper.cpp $
//
//    Original Author: James Yarrow
//
//            $Author: Brian_Hayes $
//
//            $Change: 747268 $
//
//          $DateTime: 2020/10/27 14:46:31 $
//
//          $Revision: #3 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */

#pragma hdrstop

#include "PositionWrapper.h"

PG_IMPLEMENT_RTTI(PositionWrapper, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_POSITION_WRAPPER, PositionWrapper);
MEMORY_POOL_INSTANCE(PositionWrapper, LUA_WRAPPER_POOL_SIZE);

PositionWrapper::PositionWrapper() :
	Position(VECTOR3_INVALID)
{
	LUA_REGISTER_MEMBER_FUNCTION(PositionWrapper, "Is_Valid", &PositionWrapper::Is_Valid);
	LUA_REGISTER_MEMBER_FUNCTION(PositionWrapper, "Get_XYZ", &PositionWrapper::Get_XYZ);
}

PositionWrapper *PositionWrapper::Create(const Vector3 &position)
{
	PositionWrapper *wrapper = static_cast<PositionWrapper*>(FactoryCreate());
	wrapper->Set_Position(position);
	return wrapper;
}

enum
{
	POSITION_MICRO_CHUNK,
};

bool PositionWrapper::Save(LuaScriptClass *, ChunkWriterClass *writer)
{
	bool ok = true;
	WRITE_MICRO_CHUNK(POSITION_MICRO_CHUNK, Position);
	return ok;
}

bool PositionWrapper::Load(LuaScriptClass *, ChunkReaderClass *reader)
{
	bool ok = true;
	while (reader->Open_Micro_Chunk())
	{
		switch (reader->Cur_Micro_Chunk_ID())
		{
			READ_MICRO_CHUNK(POSITION_MICRO_CHUNK, Position);

		default:
			ok = false;
			assert(false);
			break;
		}
		reader->Close_Micro_Chunk();
	}

	return ok;
}

bool PositionWrapper::Is_Equal(const LuaVar *var) const
{
	SmartPtr<PositionWrapper> other_position = PG_Dynamic_Cast<PositionWrapper>(const_cast<LuaVar*>(var));
	if (!other_position)
	{
		return false;
	}

	return other_position->Get_Position() == Position;
}

LuaTable *PositionWrapper::Get_XYZ(LuaScriptClass *, LuaTable *) 
{ 
	LuaTable *retval = Return_Variable(new LuaNumber(Position.X));
	retval->Value.push_back(new LuaNumber(Position.Y));
	retval->Value.push_back(new LuaNumber(Position.Z));
	return retval;
}
