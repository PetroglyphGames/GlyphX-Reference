// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/WeightedTypeList.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/WeightedTypeList.cpp $
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
#include "WeightedTypeList.h"
#include "GameObjectTypeManager.h"
#include "GameObjectType.h"
#include "GameObjectManager.h"
#include "GameObjectCategoryType.h"
#include "DynamicEnum.h"
#include "Text.h"

PG_IMPLEMENT_RTTI(WeightedTypeListClass, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_WEIGHTED_TYPE_LIST, WeightedTypeListClass);


WeightedTypeListClass::WeightedTypeListClass()
{
	LUA_REGISTER_MEMBER_FUNCTION(WeightedTypeListClass, "Create", &WeightedTypeListClass::Lua_Create);
	LUA_REGISTER_MEMBER_FUNCTION(WeightedTypeListClass, "Parse", &WeightedTypeListClass::Lua_Parse);
}

WeightedTypeListClass::~WeightedTypeListClass()
{
}

float WeightedTypeListClass::Get_Type_Weight(GameObjectTypeClass *type)
{
	if (!type) return 0.0;

	//Use the weight for this specific type if any, or eslse the average
	//weight for matching categories.
	float avg_weight = 0.0f;
	int found_weight_count = 0;
	for (int i = 0; i < (int)WeightedTypes.size(); i++) {
		if (WeightedTypes[i].Type.Get() == type)
			return WeightedTypes[i].Weight;

		if (type->Get_Category_Mask() & ((unsigned int)WeightedTypes[i].Category))
		{
			avg_weight += WeightedTypes[i].Weight;
			++found_weight_count;
		}
		else if (type->Get_Property_Mask() & ((unsigned int)WeightedTypes[i].Properties))
		{
			avg_weight += WeightedTypes[i].Weight;
			++found_weight_count;
		}
	}
	return found_weight_count == 0 ? 0.0f : (avg_weight / found_weight_count);
}

LuaTable* WeightedTypeListClass::Lua_Create(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(FactoryCreate());
}

LuaTable* WeightedTypeListClass::Lua_Parse(LuaScriptClass *script, LuaTable *params)
{
	// param1 == table of strings
	// param2 == table of weights
	if (params->Value.size() < 2) {
		script->Script_Error("WeightedTypeListClass -- Not enough parameters");
		return NULL;
	}

	LuaTable::Pointer strings = LUA_SAFE_CAST(LuaTable, params->Value[0]);
	LuaTable::Pointer weights = LUA_SAFE_CAST(LuaTable, params->Value[1]);

	if (!strings || !weights) {
		script->Script_Error("WeightedTypeListClass -- Invalid parameters");
		return NULL;
	}

	WeightedTypes.resize(0);
	WeightedTypes.reserve(strings->Value.size());

	for (int i = 0; i < (int)strings->Value.size(); i++) {
		if (i >= (int)weights->Value.size()) {
			script->Script_Error("WeightedTypeListClass -- Types and Weights are mismatched!");
			return NULL;
		}
		LuaString::Pointer str = LUA_SAFE_CAST(LuaString, strings->Value[i]);
		LuaNumber::Pointer num = LUA_SAFE_CAST(LuaNumber, weights->Value[i]);
		if (!num)
		{
			script->Script_Error("WeightedTypeListClass -- Invalid Entry: %d, for Weight list.!", i);
			continue;
		}

		Add_Entry(str->Value.c_str(), num->Value);

		if (WeightedTypes.back().Type.Get_Name())
		{
			WeightedTypes.back().Type.Fixup();
			if (!WeightedTypes.back().Type.Get())
			{
				script->Script_Error("WeightedTypeListClass -- Unable to find type or category: %s!", str->Value.c_str());
				continue;
			}
		}
	}

	return NULL;
}

int WeightedTypeListClass::Get_Weight_Count(void) const
{
	return (int)WeightedTypes.size();
}

bool WeightedTypeListClass::Is_Category_At_Index(int index) const
{
	return WeightedTypes[index].Type.Get() == NULL;
}

GameObjectCategoryType WeightedTypeListClass::Get_Category_At_Index(int index) const
{
	return WeightedTypes[index].Category;
}

GameObjectTypeClass *WeightedTypeListClass::Get_Type_At_Index(int index) const
{
	return const_cast<GameObjectTypeClass*>(WeightedTypes[index].Type.Get());
}

GameObjectPropertiesType WeightedTypeListClass::Get_Properties_At_Index(int index) const
{
	return WeightedTypes[index].Properties;
}

float WeightedTypeListClass::Get_Weight_At_Index(int index) const
{
	return WeightedTypes[index].Weight;
}

void WeightedTypeListClass::Add_Entry(GameObjectTypeClass *type, float weight)
{
	WeightedTypes.push_back(WeightedTypeClass());
	WeightedTypeClass &wt = WeightedTypes.back();
	wt.Type.Set(type);
	wt.Weight = weight;
}

void WeightedTypeListClass::Add_Entry(GameObjectCategoryType category, float weight)
{
	WeightedTypes.push_back(WeightedTypeClass());
	WeightedTypeClass &wt = WeightedTypes.back();
	wt.Category = category;
	wt.Weight = weight;
}

void WeightedTypeListClass::Add_Entry(GameObjectPropertiesType properties, float weight)
{
	WeightedTypes.push_back(WeightedTypeClass());
	WeightedTypeClass &wt = WeightedTypes.back();
	wt.Properties = properties;
	wt.Weight = weight;
}

void WeightedTypeListClass::Add_Entry(const std::string &name, float weight)
{
	WeightedTypes.push_back(WeightedTypeClass());
	WeightedTypeClass &wt = WeightedTypes.back();
	if (!TheGameObjectCategoryTypeConverterPtr->String_To_Enum(name, wt.Category))
	{
		if (!TheGameObjectPropertiesTypeConverterPtr->String_To_Enum(name, wt.Properties))
		{
			wt.Type.Set_Name(name);
		}
	}
	wt.Weight = weight;
}

void WeightedTypeListClass::Fixup()
{
	for (unsigned int i = 0; i < WeightedTypes.size(); ++i)
	{
		WeightedTypes[i].Type.Fixup();
	}
}

//    struct WeightedTypeClass
//    {
//       WeightedTypeClass() : Weight(0.0), Category(GAME_OBJECT_CATEGORY_NONE), Type(NULL) {}
//       float                                 Weight;
//       GameObjectCategoryType                 Category;
//       GameObjectTypeClass *                  Type;
//    };
//
//    std::vector<WeightedTypeClass>            WeightedTypes;
enum {
	CHUNK_ID_WEIGHTED_TYPE_DATA,
	CHUNK_ID_WEIGHTED_TYPE_WEIGHT,
	CHUNK_ID_WEIGHTED_TYPE_CATEGORY,
	CHUNK_ID_WEIGHTED_TYPE_TYPE,
	CHUNK_ID_WEIGHTED_TYPE_PROPERTIES,
};
bool WeightedTypeListClass::WeightedTypeClass::Save(ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;
	std::string type_name;

	if (Type.Get_Name())
	{
		type_name = Type.Get_Name();
	}

	WRITE_MICRO_CHUNK					(	CHUNK_ID_WEIGHTED_TYPE_WEIGHT, Weight);
	WRITE_MICRO_CHUNK					(	CHUNK_ID_WEIGHTED_TYPE_CATEGORY, Category);
	WRITE_MICRO_CHUNK_STRING		(	CHUNK_ID_WEIGHTED_TYPE_TYPE, type_name);
	WRITE_MICRO_CHUNK					(	CHUNK_ID_WEIGHTED_TYPE_PROPERTIES, Properties);

	return (ok);
}

bool WeightedTypeListClass::WeightedTypeClass::Load(ChunkReaderClass *reader)
{
	// Don't use pointers to objects in here unless you change the vector
	// so it's pre-sized on load.
	assert(reader != NULL);
	bool ok = true;

	std::string type_name;

	while (reader->Open_Micro_Chunk()) {
		switch ( reader->Cur_Micro_Chunk_ID() ) {
			READ_MICRO_CHUNK				(	CHUNK_ID_WEIGHTED_TYPE_WEIGHT, Weight);
			READ_MICRO_CHUNK				(	CHUNK_ID_WEIGHTED_TYPE_CATEGORY, Category);
			READ_MICRO_CHUNK_STRING		(	CHUNK_ID_WEIGHTED_TYPE_TYPE, type_name);
			READ_MICRO_CHUNK				(	CHUNK_ID_WEIGHTED_TYPE_PROPERTIES, Properties);

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Micro_Chunk();
	}

	if (!type_name.empty())
	{
		Type.Set_Name(type_name);
	}
	Type.Fixup();

	return (ok);
}


bool WeightedTypeListClass::Save(LuaScriptClass *, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;

	for (int i = 0; i < (int)WeightedTypes.size(); i++) {
		ok &= writer->Begin_Chunk(CHUNK_ID_WEIGHTED_TYPE_DATA);
		ok &= WeightedTypes[i].Save(writer);
		ok &= writer->End_Chunk();
	}

	return (ok);
}

bool WeightedTypeListClass::Load(LuaScriptClass *, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;

	WeightedTypes.resize(0);

	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case CHUNK_ID_WEIGHTED_TYPE_DATA:
				WeightedTypes.push_back(WeightedTypeClass());
				ok &= WeightedTypes.back().Load(reader);
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}
	return (ok);
}