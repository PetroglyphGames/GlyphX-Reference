// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FOWReveal.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FOWReveal.cpp $
//
//    Original Author: Brian Hayes
//
//            $Author: Brian_Hayes $
//
//            $Change: 641502 $
//
//          $DateTime: 2017/05/09 13:45:27 $
//
//          $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */
#pragma hdrstop

#include "FOWReveal.h"
#include "GameModeManager.h"
#include "GameObjectManager.h"
#include "GameObjectTypeManager.h"
#include "AI/LuaScript/GameObjectTypeWrapper.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/GameObjectWrapper.h"



class LuaFOWCellsClass : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_FOW_CELLS, LuaFOWCellsClass);
	LuaFOWCellsClass() : PlayerID(-1)
	{ 
		LUA_REGISTER_MEMBER_FUNCTION(LuaFOWCellsClass, "Undo_Reveal", &LuaFOWCellsClass::Undo_Reveal);
	}

	~LuaFOWCellsClass()
	{
	}

	std::vector<int> Cells;
	int PlayerID;

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		return Undo_Reveal(script, params);
	}
	LuaTable *Undo_Reveal(LuaScriptClass *script, LuaTable *params)
	{
		script; params;
		GameModeManager.Get_Active_Mode()->Undo_FOW_Reveal(PlayerID, Cells);
		Cells.clear();
		return NULL;
	}

	enum {
		CHUNK_ID_FOW_CELLS_DATA,
		CHUNK_ID_FOW_CELLS_VECTOR,
		CHUNK_ID_FOW_CELLS_PLAYER_ID,
	};

	bool Save( LuaScriptClass *script, ChunkWriterClass *writer )
	{
		script;
		bool ok = true;

		ok &= writer->Begin_Chunk(CHUNK_ID_FOW_CELLS_DATA);
			WRITE_MICRO_CHUNK(CHUNK_ID_FOW_CELLS_PLAYER_ID, PlayerID);
		ok &= writer->End_Chunk();

		WRITE_STL_VECTOR(CHUNK_ID_FOW_CELLS_VECTOR, Cells);

		return ok;
	}

	void On_FOW_Init(void)
	{
		FogOfWarClass *fow = GameModeManager.Get_Active_Mode()->Get_Fog_Of_War(PlayerID);
		FAIL_IF(!fow) return;
		for (int i = 0; i < (int)Cells.size(); i++)
		{
			fow->Reveal_Cell(Cells[i]);
		}
	}

	bool Load( LuaScriptClass *script, ChunkReaderClass *reader )
	{
		script;
		bool ok = true;
		GameModeClass::Register_FOW_Init_Callback(FOW_Member_Callback<LuaFOWCellsClass>, this);
		while (reader->Open_Chunk()) {
			switch ( reader->Cur_Chunk_ID() )
			{
				case CHUNK_ID_FOW_CELLS_DATA:
					while (reader->Open_Micro_Chunk()) {
						switch ( reader->Cur_Micro_Chunk_ID() )
						{
							READ_MICRO_CHUNK(CHUNK_ID_FOW_CELLS_PLAYER_ID, PlayerID);
							default: assert(false); break;   // Unknown Chunk
						}
						reader->Close_Micro_Chunk();
					}
					break;

				READ_STL_VECTOR(CHUNK_ID_FOW_CELLS_VECTOR, Cells);
				default: assert(false); break;	// Unknown Chunk
			}
			reader->Close_Chunk();
		}
		return ok;
	}
};

LUA_IMPLEMENT_FACTORY(LUA_CHUNK_FOW_CELLS, LuaFOWCellsClass);
PG_IMPLEMENT_RTTI(LuaFOWCellsClass, LuaUserVar);


PG_IMPLEMENT_RTTI(LuaFOWRevealCommandClass, LuaUserVar);

LuaFOWRevealCommandClass::LuaFOWRevealCommandClass()
{
	LUA_REGISTER_MEMBER_FUNCTION(LuaFOWRevealCommandClass, "Reveal", &LuaFOWRevealCommandClass::Reveal);
	LUA_REGISTER_MEMBER_FUNCTION(LuaFOWRevealCommandClass, "Reveal_All", &LuaFOWRevealCommandClass::Reveal_All);
	LUA_REGISTER_MEMBER_FUNCTION(LuaFOWRevealCommandClass, "Disable_Rendering", &LuaFOWRevealCommandClass::Disable_Rendering);
	LUA_REGISTER_MEMBER_FUNCTION(LuaFOWRevealCommandClass, "Temporary_Reveal", &LuaFOWRevealCommandClass::Temporary_Reveal);
}


LuaTable *LuaFOWRevealCommandClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	script; params;
	return Return_Variable(this);
}


LuaTable *LuaFOWRevealCommandClass::Disable_Rendering(LuaScriptClass *script, LuaTable *params)
{
	if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Command only valid in a tactical game.");
		return NULL;
	}

	bool enable = false;
	if (params->Value.size())
	{
		LuaBool::Pointer bval = PG_Dynamic_Cast<LuaBool>(params->Value[0]);
		enable = !bval->Value;
	}

	GameModeClass::Enable_Fog_Of_War(enable);
	alGraphicsDriver::Enable_Fog_Of_War_Rendering(enable);
	return NULL;
}


LuaTable *LuaFOWRevealCommandClass::Temporary_Reveal(LuaScriptClass *script, LuaTable *params)
{
	if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Command only valid in a tactical game.");
		return NULL;
	}

	if (params->Value.size() < 2)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Requires 2 parameters: (player, object, [radius]).");
		return NULL;
	}

	SmartPtr<PlayerWrapper> pval = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!pval)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Expected player object as first parameter.");
		return NULL;
	}

	SmartPtr<GameObjectWrapper> object = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[1]);
	if (!object)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Expected object as 2nd parameter.");
		return NULL;
	}

	LuaNumber::Pointer num;
	if (params->Value.size() > 2)
	{
		num = PG_Dynamic_Cast<LuaNumber>(params->Value[2]);
		if (!num)
		{
			script->Script_Error("LuaFOWRevealCommandClass -- Expected number as 3rd parameter.");
			return NULL;
		}
	}

	GameModeManager.Get_Active_Mode()->Temporary_FOW_Reveal(pval->Get_Object()->Get_ID(), object->Get_Object(), num ? num->Value : 0.0f);
	return NULL;
}


// Params: player, position, normal_radius, dense_radius...
LuaTable *LuaFOWRevealCommandClass::Reveal_All(LuaScriptClass *script, LuaTable *params)
{
	if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Command only valid in a tactical game.");
		return NULL;
	}

	if (params->Value.size() < 1)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Requires at least 1 parameters: (player).");
		return NULL;
	}

	SmartPtr<PlayerWrapper> pval = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!pval)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Expected player object as first parameter.");
		return NULL;
	}

	// AJA 05/04/2006 - Use the built-in "reveal whole map" function instead of doing a really large
	// circular reveal. This way is much more efficient.
	GameModeManager.Get_Active_Mode()->FOW_Reveal_Entire_Map(pval->Get_Object()->Get_ID());
	return NULL;

	/*LuaFOWCellsClass *cell_obj = (LuaFOWCellsClass *)LuaFOWCellsClass::FactoryCreate();
	cell_obj->PlayerID = pval->Get_Object()->Get_ID();
	Vector3 extent_vector = GameModeManager.Get_Active_Mode()->Get_Map_Bounds().Get_Extent();
	Vector3 outpos = GameModeManager.Get_Active_Mode()->Get_Map_Bounds().Center;
	float radius = PGMath::Sqrt(extent_vector.x * extent_vector.x + extent_vector.y * extent_vector.y);
	radius *= 1.5f;
	GameModeManager.Get_Active_Mode()->FOW_Reveal(pval->Get_Object()->Get_ID(), outpos, radius, radius, cell_obj->Cells);
	return Return_Variable(cell_obj);*/
}


// Params: player, position, normal_radius, dense_radius...
LuaTable *LuaFOWRevealCommandClass::Reveal(LuaScriptClass *script, LuaTable *params)
{
	if (GameModeManager.Get_Active_Mode()->Is_Sub_Type_Tactical() == false)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Command only valid in a tactical game.");
		return NULL;
	}

	if (params->Value.size() < 2)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Requires at least 2 parameters: (player, object).");
		return NULL;
	}

	SmartPtr<PlayerWrapper> pval = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!pval)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Expected player object as first parameter.");
		return NULL;
	}

	SmartPtr<GameObjectWrapper> object = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[1]);
	float radius = 0.0f;
	float dense_radius = 0.0f;
	Vector3 out_pos(VECTOR3_INVALID);
	if (!object || params->Value.size() > 2)
	{
		object = NULL;
		if (params->Value.size() < 3)
		{
			script->Script_Error("LuaFOWRevealCommandClass -- Expected 3rd parameter for reveal radius.");
			return NULL;
		}
		LuaNumber::Pointer radval = PG_Dynamic_Cast<LuaNumber>(params->Value[2]);
		dense_radius = radius = radval->Value;
		if (params->Value.size() > 3)
		{
			radval = PG_Dynamic_Cast<LuaNumber>(params->Value[3]);
			dense_radius = radval->Value;
		}
		Lua_Extract_Position(params->Value[1], out_pos);
	}
	else
	{
		out_pos = object->Get_Object()->Get_Position();
	}

	if (out_pos == VECTOR3_INVALID)
	{
		script->Script_Error("LuaFOWRevealCommandClass -- Undable to get a valid position from parameter 2.");
		return NULL;
	}

	LuaFOWCellsClass *cell_obj = (LuaFOWCellsClass *)LuaFOWCellsClass::FactoryCreate();
	cell_obj->PlayerID = pval->Get_Object()->Get_ID();
	if (object)
	{
		GameModeManager.Get_Active_Mode()->FOW_Reveal(pval->Get_Object()->Get_ID(), object->Get_Object(), cell_obj->Cells);
	}
	else
	{
		GameModeManager.Get_Active_Mode()->FOW_Reveal(pval->Get_Object()->Get_ID(), out_pos, radius, dense_radius, cell_obj->Cells);
	}

	return Return_Variable(cell_obj);
}
