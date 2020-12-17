// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/WeightedTypeList.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/WeightedTypeList.h $
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

#ifndef __WEIGHTEDTYPELIST_H__
#define __WEIGHTEDTYPELIST_H__

#include "AI/LuaScript/LuaRTSUtilities.h"
#include "GameObjectCategoryType.h"
#include "ReferenceClasses.h"

class GameObjectTypeClass;

class WeightedTypeListClass : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_WEIGHTED_TYPE_LIST, WeightedTypeListClass);

	WeightedTypeListClass();
	~WeightedTypeListClass();

//    virtual LuaTable* Function_Call(LuaScriptClass *script, LuaTable *params);
	virtual LuaTable* Lua_Create(LuaScriptClass *script, LuaTable *params);
	virtual LuaTable* Lua_Parse(LuaScriptClass *script, LuaTable *params);

	float Get_Type_Weight(GameObjectTypeClass *type);

	int Get_Weight_Count(void) const;
	bool Is_Category_At_Index(int index) const;
	GameObjectCategoryType Get_Category_At_Index(int index) const;
	GameObjectTypeClass *Get_Type_At_Index(int index) const;
	GameObjectPropertiesType Get_Properties_At_Index(int index) const;
	float Get_Weight_At_Index(int index) const;

	void Add_Entry(GameObjectPropertiesType properties, float weight);
	void Add_Entry(GameObjectCategoryType category, float weight);
	void Add_Entry(GameObjectTypeClass *type, float weight);
	void Add_Entry(const std::string &name, float weight);

	void Fixup();

	void Clear() { WeightedTypes.resize(0); }

	/**
	 * Save / Load
	 */
	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

private:
	struct WeightedTypeClass
	{
		WeightedTypeClass() : Weight(0.0), Category(GAME_OBJECT_CATEGORY_NONE), Properties(GAME_OBJECT_PROPERTIES_NONE) {}
		bool Save(ChunkWriterClass *writer);
		bool Load(ChunkReaderClass *reader);
		float											Weight;
		GameObjectCategoryType					Category;
		NameReferenceClass						Type;
		GameObjectPropertiesType				Properties;
	};

	std::vector<WeightedTypeClass>				WeightedTypes;
};


#endif // __WEIGHTEDTYPELIST_H__


