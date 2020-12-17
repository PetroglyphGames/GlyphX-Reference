// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaWideString.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaWideString.h $
//
//    Original Author: Justin Fic
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

#ifndef __LUAWIDESTRING_H__
#define __LUAWIDESTRING_H__


#include "Assert.h"
#include "LuaScriptVariable.h"
#include "LuaScript.h"

#include <xstring>

class LuaWideString : public LuaUserVar
{
public:
	PG_DECLARE_RTTI();
	//LuaWideString();
	LuaWideString::LuaWideString(std::wstring str);

	LuaTable *Lua_Append(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Assign(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_At(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Capacity(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Compare(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Copy(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Str(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Data(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Empty(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Erase(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Find(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Find_First_Not_Of(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Find_First_Of(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Find_Last_Not_Of(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Find_Last_Of(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Insert(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Length(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Max_Size(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_RFind(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Replace(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Reserve(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Resize(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Size(LuaScriptClass *script, LuaTable* params);
	LuaTable *Lua_Substr(LuaScriptClass *script, LuaTable* params);
	void To_String(std::string &outstr);
	bool Is_Equal(const LuaVar *var) const;
	LuaVar *Map_Into_Other_Script(LuaScriptClass *);
	size_t Hash_Function(void);

	std::wstring	Get_WString() {return Value;}
	std::wstring	Value;

protected:

};

#endif // __LUAWIDESTRING_H__


