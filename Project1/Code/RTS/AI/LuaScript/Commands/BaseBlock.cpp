// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/BaseBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/BaseBlock.cpp $
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

#pragma hdrstop
#include "BaseBlock.h"
#include "AI/LuaSCript/GameObjectWrapper.h"
#include "PlanetaryBehavior.h"
#include "GameObject.h"
#include "BlockingStatus.h"
#include "AI/AILog.h"

class BaseBlockStatus : public BlockingStatus
{
public:

	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_BASE_LEVEL_BLOCK, BaseBlockStatus);

	void Init(GameObjectClass *planet, int base_level, bool ground);

	virtual LuaTable *Is_Finished(LuaScriptClass *script, LuaTable *params);

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

private:

	GameObjectClass *Planet;
	bool GroundBase;
	int BaseLevel;
};

PG_IMPLEMENT_RTTI(BaseBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_BASE_LEVEL_BLOCK, BaseBlockStatus);

void BaseBlockStatus::Init(GameObjectClass *planet, int base_level, bool ground)
{
	Planet = planet;
	BaseLevel = base_level;
	GroundBase = ground;
}

LuaTable *BaseBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	PlanetaryBehaviorClass *behave = static_cast<PlanetaryBehaviorClass*>(Planet->Get_Behavior(BEHAVIOR_PLANET));
	if (GroundBase)
	{
		if (behave->Get_Current_Ground_Base_Level(Planet) >= BaseLevel)
		{
			return Return_Variable(new LuaBool(true));
		}
	}
	else
	{
		if (behave->Get_Current_Starbase_Level(Planet) >= BaseLevel)
		{
			return Return_Variable(new LuaBool(true));
		}
	}

	return Return_Variable(new LuaBool(false));
}

//    GameObjectClass *Planet;
//    bool GroundBase;
//    int BaseLevel;

enum {
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_BASEBLOCK_DATA,
	CHUNK_ID_BASEBLOCK_PLANET,
	CHUNK_ID_BASEBLOCK_BASE_LEVEL,
	CHUNK_ID_BASEBLOCK_GROUND_BASE,
};

bool BaseBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
		ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(CHUNK_ID_BASEBLOCK_DATA);
		WRITE_MICRO_CHUNK_OBJECT_PTR			(CHUNK_ID_BASEBLOCK_PLANET,				Planet);
		WRITE_MICRO_CHUNK							(CHUNK_ID_BASEBLOCK_BASE_LEVEL,			BaseLevel);
		WRITE_MICRO_CHUNK							(CHUNK_ID_BASEBLOCK_GROUND_BASE,			GroundBase);
	ok &= writer->End_Chunk();

	return (ok);
}

bool BaseBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;
	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case CHUNK_ID_BASE_CLASS:
				ok &= BlockingStatus::Load(script, reader);
				break;

			case CHUNK_ID_BASEBLOCK_DATA:
				while (reader->Open_Micro_Chunk()) {
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK_OBJECT_PTR			(CHUNK_ID_BASEBLOCK_PLANET,				Planet);
						READ_MICRO_CHUNK							(CHUNK_ID_BASEBLOCK_BASE_LEVEL,			BaseLevel);
						READ_MICRO_CHUNK							(CHUNK_ID_BASEBLOCK_GROUND_BASE,			GroundBase);
						default: assert(false); break;   // Unknown Chunk
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

PG_IMPLEMENT_RTTI(WaitForGroundbaseClass, LuaUserVar);

LuaTable *WaitForGroundbaseClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 2)
	{
		script->Script_Error("WaitForGroundbase : Invalid number of parameters: %d should be 2.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectWrapper> planet = LUA_SAFE_CAST(GameObjectWrapper, params->Value[0]);
	if (!planet || !planet->Get_Object()->Behaves_Like(BEHAVIOR_PLANET))
	{
		script->Script_Error("WaitForGroundbase : Invalid type for parameter 1.  Expected planet game object.");
		return NULL;
	}

	LuaNumber *base_level = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
	if (!base_level)
	{
		script->Script_Error("WaitForGroundbase : Invalid type for parameter 2.  Expected number.");
		return NULL;
	}

	if (base_level->Value < BASE_LEVEL_MINIMUM || base_level->Value > BASE_LEVEL_MAXIMUM)
	{
		script->Script_Error("WaitForGroundBase : Invalid value %d for parameter 2.  Must be between %d and %d.", static_cast<int>(base_level->Value),
									BASE_LEVEL_MINIMUM, BASE_LEVEL_MAXIMUM);
		return NULL;
	}

	BaseBlockStatus *bs = static_cast<BaseBlockStatus*>(BaseBlockStatus::FactoryCreate());
	bs->Init(planet->Get_Object(), static_cast<int>(base_level->Value), true);

	return Return_Variable(bs);
}

PG_IMPLEMENT_RTTI(WaitForStarbaseClass, LuaUserVar);

LuaTable *WaitForStarbaseClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 2)
	{
		script->Script_Error("WaitForGroundBase : Invalid number of parameters: %d should be 2.", params->Value.size());
		return NULL;
	}

	SmartPtr<GameObjectWrapper> planet = LUA_SAFE_CAST(GameObjectWrapper, params->Value[0]);
	if (!planet || !planet->Get_Object()->Behaves_Like(BEHAVIOR_PLANET))
	{
		script->Script_Error("WaitForStarbase : Invalid type for parameter 1.  Expected planet game object.");
		return NULL;
	}

	LuaNumber *base_level = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
	if (!base_level)
	{
		script->Script_Error("WaitForStarbase : Invalid type for parameter 2.  Expected number.");
		return NULL;
	}

	if (base_level->Value < BASE_LEVEL_MINIMUM || base_level->Value > BASE_LEVEL_MAXIMUM)
	{
		script->Script_Error("WaitForStarBase : Invalid value %d for parameter 2.  Must be between %d and %d.", static_cast<int>(base_level->Value),
									BASE_LEVEL_MINIMUM, BASE_LEVEL_MAXIMUM);
		return NULL;
	}

	BaseBlockStatus *bs = static_cast<BaseBlockStatus*>(BaseBlockStatus::FactoryCreate());
	bs->Init(planet->Get_Object(), static_cast<int>(base_level->Value), false);

	return Return_Variable(bs);
}