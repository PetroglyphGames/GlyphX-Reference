// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaWideString.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaWideString.cpp $
//
//    Original Author: Justin Fic
//
//            $Author: Brian_Hayes $
//
//            $Change: 641585 $
//
//          $DateTime: 2017/05/10 10:42:50 $
//
//          $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma hdrstop

#include "LuaWideString.h"
#include "LuaScript.h"
#include "Text.h"

PG_IMPLEMENT_RTTI(LuaWideString, LuaUserVar);
//LUA_IMPLEMENT_FACTORY(LUA_CHUNK_LUA_SCRIPT_WRAPPER, LuaScriptClassWrapper);
MEMORY_POOL_INSTANCE(LuaWideString, LUA_WRAPPER_POOL_SIZE);

LuaWideString::LuaWideString(std::wstring str) :
	Value(str)
{
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "append", &LuaWideString::Lua_Append);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "assign", &LuaWideString::Lua_Assign);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "at", &LuaWideString::Lua_At);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "capacity", &LuaWideString::Lua_Capacity);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "compare", &LuaWideString::Lua_Compare);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "lua_str", &LuaWideString::Lua_Str);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "empty", &LuaWideString::Lua_Empty);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "erase", &LuaWideString::Lua_Erase);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "find", &LuaWideString::Lua_Find);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "find_first_not_of", &LuaWideString::Lua_Find_First_Not_Of);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "find_first_of", &LuaWideString::Lua_Find_First_Of);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "find_last_not_of", &LuaWideString::Lua_Find_Last_Not_Of);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "find_last_of", &LuaWideString::Lua_Find_Last_Of);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "insert", &LuaWideString::Lua_Insert);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "length", &LuaWideString::Lua_Length);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "max_size", &LuaWideString::Lua_Max_Size);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "rfind", &LuaWideString::Lua_RFind);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "replace", &LuaWideString::Lua_Replace);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "reserve", &LuaWideString::Lua_Reserve);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "resize", &LuaWideString::Lua_Resize);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "size", &LuaWideString::Lua_Size);
	LUA_REGISTER_MEMBER_FUNCTION(LuaWideString, "substr", &LuaWideString::Lua_Substr);
}

/**************************************************************************************************
* GameObjectWrapper::Is_Equal -- 
*
* In:				
*
* Out:		
*
* History: 11/11/2004 4:16PM JSY
**************************************************************************************************/
bool LuaWideString::Is_Equal(const LuaVar *var) const
{
	SmartPtr<LuaWideString> other_object = PG_Dynamic_Cast<LuaWideString>(const_cast<LuaVar*>(var));
	if (!other_object)
	{
		return false;
	}

	return other_object->Value == Value;
}


/**************************************************************************************************
* LuaWideString::Lua_Append -- equivalent to wstring::append
*
* In:				The calling Lua Script
*					Parameter Table {
*						(string/wstring) string_to_append
*						(int)    [pos]			(default: 0)
*						(int)    [num]	(default: string_to_append.size())
*					}
*
* Out:			A string consisting of num chars from pos of string_to_append tacked
*					on to the end of Value. 
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Append(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() < 1 || params->Value.size() > 3) {
		Debug_Printf("Lua_Append: Expected 1-3 parameters, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaString::Pointer lua_string_to_append = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	LuaWideString* lua_wstring_to_append = PG_Dynamic_Cast<LuaWideString>(params->Value[0]);
	
	FAIL_IF (!lua_wstring_to_append && !lua_string_to_append) {
		Debug_Printf("Lua_Append: Incorrect parameter type for string_to_append. Expected LuaString or LuaWideString.");
		return NULL;
	}
	std::wstring string_to_append;
	if (lua_wstring_to_append)
		string_to_append = lua_wstring_to_append->Get_WString();
	else
		string_to_append = To_WideChar(lua_string_to_append->Value); 

	int pos = 0;
	int num = string_to_append.size();

	LuaNumber::Pointer lua_pos;
	if (params->Value.size() >= 2) lua_pos = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);

	LuaNumber::Pointer lua_num;
	if (params->Value.size() == 3) lua_num = PG_Dynamic_Cast<LuaNumber>(params->Value[2]);

	if (lua_pos) pos = (int)lua_pos->Value - 1;
	if (lua_num) num = (int)lua_num->Value;

	return Return_Variable(new LuaWideString(Value.append(string_to_append, pos, num)));
}


/**************************************************************************************************
* LuaWideString::Lua_Assign -- equivalent to wstring::assign
*
* In:				The calling Lua Script
*					Parameter Table {
*						(string) string_to_append
*						(int)    [pos]			(default: 0)
*						(int)    [num]	(default: string_to_append.size())
*					}
*
* Out:			a string consisting of num elements of string_to_append, starting at position pos
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Assign(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() < 1 || params->Value.size() > 3) {
		Debug_Printf("Lua_Assign: Expected 1-3 parameters, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaString::Pointer lua_string_to_assign = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	FAIL_IF (!lua_string_to_assign) {
		Debug_Printf("Lua_Assign: Incorrect parameter type for string_to_append. Expected LuaString.");
		return NULL;
	}
	std::wstring string_to_assign = To_WideChar(lua_string_to_assign->Value);

	int pos = 0;
	int num = string_to_assign.size();

	LuaNumber::Pointer lua_pos;
	if (params->Value.size() >= 2) lua_pos = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
	LuaNumber::Pointer lua_num;
	if (params->Value.size() >= 3) lua_num  = PG_Dynamic_Cast<LuaNumber>(params->Value[2]);

	if (lua_pos) pos = (int)lua_pos->Value - 1;
	if (lua_num) num = (int)lua_num->Value;

	return Return_Variable(new LuaWideString(Value.assign(string_to_assign, pos, num)));
}


/**************************************************************************************************
* LuaWideString::Lua_At -- equivalent to wstring::at
*
* In:				The calling Lua Script
*					Parameter Table {
*						(int) pos
*					}
*
* Out:			the character at position pos in Value. 
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_At(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() != 1) {
		Debug_Printf("Lua_At: Expected 1 parameter, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaNumber::Pointer pos = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	FAIL_IF (!pos) {
		Debug_Printf("Lua_At: Incorrect parameter type for pos. Expected int.");
		return NULL;
	}
	return Return_Variable(new LuaWideString(std::wstring(&Value.at((int)pos->Value - 1))));
}


/**************************************************************************************************
* LuaWideString::Lua_Capacity -- equivalent to wstring::capacity
*
* In:				The calling Lua Script
*					Parameter Table {
*					}
*
* Out:			The current storage capacity of the string
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Capacity(LuaScriptClass *, LuaTable* )
{
	return Return_Variable(new LuaNumber((float)Value.capacity()));
}


/**************************************************************************************************
* LuaWideString::Lua_Compare -- equivalent to wstring::compare
*
* In:				The calling Lua Script
*					Parameter Table {
*						(string) string_to_compare
*					}
*
* Out:				
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Compare(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() != 1) {
		Debug_Printf("Lua_Compare: Expected 1 parameter, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaString::Pointer lua_string_to_compare = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	LuaWideString* lua_wstring_to_compare = PG_Dynamic_Cast<LuaWideString>(params->Value[0]);
	FAIL_IF (!lua_string_to_compare) {
		Debug_Printf("Lua_Compare: Incorrect parameter type for string_to_compare. Expected LuaString or LuaWideString.");
		return NULL;
	}
	
	std::wstring string_to_compare;
	if (lua_string_to_compare) string_to_compare = To_WideChar(lua_string_to_compare->Value);
	if (lua_wstring_to_compare) string_to_compare = lua_wstring_to_compare->Get_WString();

	return Return_Variable(new LuaNumber((float)Value.compare(string_to_compare)));
}

/**************************************************************************************************
* LuaWideString::To_String -- converts this string to a LuaString
*
* In:				The calling Lua Script
*					Parameter Table {
*					}
*
* Out:			A LuaString equivalent of this string.  
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
void LuaWideString::To_String(std::string &outstr)
{
   outstr = To_MultiByte(Value.c_str());
}

/**************************************************************************************************
* LuaWideString::Lua_Str -- converts this string to a LuaString
*
* In:				The calling Lua Script
*					Parameter Table {
*					}
*
* Out:			A LuaString equivalent of this string.  
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Str(LuaScriptClass *, LuaTable* )
{

	return Return_Variable(new LuaString(To_MultiByte(Value.c_str())));
}


/**************************************************************************************************
* LuaWideString::Lua_Empty -- equivalent to wstring::empty
*
* In:				The calling Lua Script
*					Parameter Table {
*					}
*
* Out:			A LuaBool saying whether or not the string is empty
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Empty(LuaScriptClass *, LuaTable* )
{
	return Return_Variable(new LuaBool(Value.empty()));
}


/**************************************************************************************************
* LuaWideString::Lua_Erase -- equivalent to wstring::erase
*
* In:				The calling Lua Script
*					Parameter Table {
*						(int) [starting_pos] ( default: 0 )
*						(int) [elements_to_delete] ( default: Value.size() )
*					}
*
* Out:			The string with elements_to_delete elements removed from position starting_pos. 
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Erase(LuaScriptClass *, LuaTable* params)
{
	int starting_pos = 0;
	int elements_to_delete = Value.size();

	LuaNumber::Pointer lua_starting_pos;
	LuaNumber::Pointer lua_elements_to_delete;

	if (params) {
		if (params->Value.size() >= 1) lua_starting_pos = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
		if (params->Value.size() >= 2) lua_elements_to_delete = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
		if (lua_starting_pos) starting_pos = (int)lua_starting_pos->Value - 1;
		if (lua_elements_to_delete) elements_to_delete = (int)lua_elements_to_delete->Value;
	}

	return Return_Variable(new LuaWideString(Value.erase(starting_pos, elements_to_delete)));
}


/**************************************************************************************************
* LuaWideString::Lua_Find -- equivalent to wstring::find
*
* In:				The calling Lua Script
*					Parameter Table {
*						(LuaString/LuaWideString) string_to_check
*					}
*
* Out:			Index of the first occurance of string_to_check in Value. If it does not occur, 
*						Value.size() is returned. 
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Find(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() != 1) {
		Debug_Printf("Lua_Find: Expected 1 parameter, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaString::Pointer lua_string_to_check = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	LuaWideString* lua_wstring_to_check = PG_Dynamic_Cast<LuaWideString>(params->Value[0]);
	FAIL_IF (!lua_string_to_check) {
		Debug_Printf("Lua_Find: Incorrect parameter type for string_to_compare. Expected LuaString or LuaWideString.");
		return NULL;
	}
	
	std::wstring string_to_check;
	if (lua_string_to_check) string_to_check = To_WideChar(lua_string_to_check->Value);
	if (lua_wstring_to_check) string_to_check = lua_wstring_to_check->Get_WString();

	return Return_Variable(new LuaNumber((float)Value.find(string_to_check)));
}


/**************************************************************************************************
* LuaWideString::Lua_Find_First_Not_Of -- equivalent to wstring::find_first_not_of
*
* In:				The calling Lua Script
*					Parameter Table {
*						(LuaString/LuaWideString) string_to_check
*					}
*
* Out:			Index of the first element of this string that is not equal to any element of string_to_check
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Find_First_Not_Of(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() != 1) {
		Debug_Printf("Lua_Find_First_Not_Of: Expected 1 parameter, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaString::Pointer lua_string_to_check = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	LuaWideString* lua_wstring_to_check = PG_Dynamic_Cast<LuaWideString>(params->Value[0]);
	FAIL_IF (!lua_string_to_check) {
		Debug_Printf("Lua_Find_First_Not_Of: Incorrect parameter type for string_to_compare. Expected LuaString or LuaWideString.");
		return NULL;
	}
	
	std::wstring string_to_check;
	if (lua_string_to_check) string_to_check = To_WideChar(lua_string_to_check->Value);
	if (lua_wstring_to_check) string_to_check = lua_wstring_to_check->Get_WString();

	return Return_Variable(new LuaNumber((float)Value.find_first_not_of(string_to_check)));
}


/**************************************************************************************************
* LuaWideString::Lua_Find_First_Of -- equivalent to wstring::find_first_of
*
* In:				The calling Lua Script
*					Parameter Table {
*						(LuaString/LuaWideString) string_to_check
*					}
*
* Out:			Index of the first element of this string that is equal to any element of string_to_check			
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Find_First_Of(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() != 1) {
		Debug_Printf("Lua_Find_First_Of: Expected 1 parameter, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaString::Pointer lua_string_to_check = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	LuaWideString* lua_wstring_to_check = PG_Dynamic_Cast<LuaWideString>(params->Value[0]);
	FAIL_IF (!lua_string_to_check) {
		Debug_Printf("Lua_Find_First_Of: Incorrect parameter type for string_to_compare. Expected LuaString or LuaWideString.");
		return NULL;
	}
	
	std::wstring string_to_check;
	if (lua_string_to_check) string_to_check = To_WideChar(lua_string_to_check->Value);
	if (lua_wstring_to_check) string_to_check = lua_wstring_to_check->Get_WString();

	return Return_Variable(new LuaNumber((float)Value.find_first_of(string_to_check)));
}


/**************************************************************************************************
* LuaWideString::Lua_Find_Last_Not_Of -- equivalent to wstring::find_last_not_of
*
* In:				The calling Lua Script
*					Parameter Table {
*						(LuaString/LuaWideString) string_to_check
*					}
*
* Out:			Index of the last element of this string that is not equal to any element of string_to_check				
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Find_Last_Not_Of(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() != 1) {
		Debug_Printf("Lua_Find_Last_Not_Of: Expected 1 parameter, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaString::Pointer lua_string_to_check = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	LuaWideString* lua_wstring_to_check = PG_Dynamic_Cast<LuaWideString>(params->Value[0]);
	FAIL_IF (!lua_string_to_check) {
		Debug_Printf("Lua_Find_Last_Not_Of: Incorrect parameter type for string_to_compare. Expected LuaString or LuaWideString.");
		return NULL;
	}
	
	std::wstring string_to_check;
	if (lua_string_to_check) string_to_check = To_WideChar(lua_string_to_check->Value);
	if (lua_wstring_to_check) string_to_check = lua_wstring_to_check->Get_WString();

	return Return_Variable(new LuaNumber((float)Value.find_last_not_of(string_to_check)));
}


/**************************************************************************************************
* LuaWideString::Lua_Find_Last_Of -- equivalent to wstring::find_last_of
*
* In:				The calling Lua Script
*					Parameter Table {
*						(LuaString/LuaWideString) string_to_check
*					}
*
* Out:			Index of the last element of this string that is equal to any element of string_to_check			
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Find_Last_Of(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() != 1) {
		Debug_Printf("Lua_Find_Last_Of: Expected 1 parameter, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaString::Pointer lua_string_to_check = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	LuaWideString* lua_wstring_to_check = PG_Dynamic_Cast<LuaWideString>(params->Value[0]);
	FAIL_IF (!lua_string_to_check) {
		Debug_Printf("Lua_Find_Last_Of: Incorrect parameter type for string_to_compare. Expected LuaString or LuaWideString.");
		return NULL;
	}
	
	std::wstring string_to_check;
	if (lua_string_to_check) string_to_check = To_WideChar(lua_string_to_check->Value);
	if (lua_wstring_to_check) string_to_check = lua_wstring_to_check->Get_WString();

	return Return_Variable(new LuaNumber((float)Value.find_last_of(string_to_check)));
}


/**************************************************************************************************
* LuaWideString::Lua_Insert -- equivalent to wstring::insert
*
* In:				The calling Lua Script
*					Parameter Table {
*						(LuaString/LuaWideString)	string_to_insert
*						(LuaNumber)						pos
*					}
*
* Out:			Inserts string_to_insert into Value at position pos and returns the resulting string				
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Insert(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() < 1) {
		Debug_Printf("Lua_Insert: Expected 1-2 parameter(s), got %i.\n", params->Value.size());
		return NULL;
	}

	LuaString::Pointer lua_string_to_insert = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	LuaWideString* lua_wstring_to_insert = PG_Dynamic_Cast<LuaWideString>(params->Value[0]);
	FAIL_IF (!lua_string_to_insert) {
		Debug_Printf("Lua_Insert: Incorrect parameter type for string_to_compare. Expected LuaString or LuaWideString.");
		return NULL;
	}
	
	std::wstring string_to_insert;
	if (lua_string_to_insert) string_to_insert = To_WideChar(lua_string_to_insert->Value);
	if (lua_wstring_to_insert) string_to_insert = lua_wstring_to_insert->Get_WString();

	LuaNumber::Pointer lua_pos;
	if (params->Value.size() >= 2) lua_pos = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
	int pos = 0;
	if (lua_pos) pos = (int) lua_pos->Value - 1;

	return Return_Variable(new LuaWideString(Value.insert(pos, string_to_insert)));
}


/**************************************************************************************************
* LuaWideString::Lua_Length -- equivalent to wstring::length
*
* In:				The calling Lua Script
*					Parameter Table {
*					}
*
* Out:			The length of the string
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Length(LuaScriptClass *, LuaTable* )
{
	return Return_Variable(new LuaNumber((float)Value.length()));
}


/**************************************************************************************************
* LuaWideString::Lua_Max_Size -- equivalent to wstring::max_size
*
* In:				The calling Lua Script
*					Parameter Table {
*					}
*
* Out:			The maximum possible size of this string
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Max_Size(LuaScriptClass *, LuaTable* )
{
	return Return_Variable(new LuaNumber((float)Value.max_size()));
}


/**************************************************************************************************
* LuaWideString::Lua_RFind -- equivalent to wstring::rfind
*
* In:				The calling Lua Script
*					Parameter Table {
*						(LuaString/LuaWideString) string_to_check
*					}
*
* Out:			Index of the first element of the last occuring substring string_to_check in Value. 	
*						If substring does not occur, Value.size() is returned. 
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_RFind(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() != 1) {
		Debug_Printf("Lua_RFind: Expected 1 parameter, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaString::Pointer lua_string_to_check = PG_Dynamic_Cast<LuaString>(params->Value[0]);
	LuaWideString* lua_wstring_to_check = PG_Dynamic_Cast<LuaWideString>(params->Value[0]);
	FAIL_IF (!lua_string_to_check) {
		Debug_Printf("Lua_RFind: Incorrect parameter type for string_to_compare. Expected LuaString or LuaWideString.");
		return NULL;
	}
	
	std::wstring string_to_check;
	if (lua_string_to_check) string_to_check = To_WideChar(lua_string_to_check->Value);
	if (lua_wstring_to_check) string_to_check = lua_wstring_to_check->Get_WString();

	return Return_Variable(new LuaNumber((float)Value.rfind(string_to_check)));
}


/**************************************************************************************************
* LuaWideString::Lua_Replace -- equivalent to wstring::replace
*
* In:				The calling Lua Script
*					Parameter Table {
*						(LuaString/LuaWideString)	string_to_replace
*						(LuaNumber)						pos
*						(LuaNumber)						num
*						(LuaNumber)						[pos2]
*						(LuaNumber)						[num2]
*					}
*
* Out:			Replaces num elements of Value starting at pos with (optionally) num2 elements of
*						string_to_replace, starting at pos2. If pos2 and num2 are not specified, the 
*						entire string_to_replace is used. 
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Replace(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() != 3) {
		Debug_Printf("Lua_Replace: Expected 3 parameter(s), got %i.\n", params->Value.size());
		return NULL;
	}

	LuaString::Pointer lua_string_to_replace = PG_Dynamic_Cast<LuaString>(params->Value[2]);
	LuaWideString* lua_wstring_to_replace = PG_Dynamic_Cast<LuaWideString>(params->Value[2]);
	FAIL_IF (!lua_string_to_replace) {
		Debug_Printf("Lua_Replace: Incorrect parameter type for string_to_replace. Expected LuaString or LuaWideString.");
		return NULL;
	}
	
	std::wstring string_to_replace;
	if (lua_string_to_replace) string_to_replace = To_WideChar(lua_string_to_replace->Value);
	if (lua_wstring_to_replace) string_to_replace = lua_wstring_to_replace->Get_WString();

	LuaNumber::Pointer lua_pos = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	int pos = 0;
	if (lua_pos) pos = (int) lua_pos->Value - 1;

	LuaNumber::Pointer lua_num = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
	int num = 0;
	if (lua_num) num = (int) lua_num->Value;

	LuaNumber::Pointer lua_pos2;
	if (params->Value.size() >= 4) lua_pos2 = PG_Dynamic_Cast<LuaNumber>(params->Value[3]);
	int pos2 = 0;
	if (lua_pos2) pos2 = (int) lua_pos2->Value - 1;

	LuaNumber::Pointer lua_num2;
	if (params->Value.size() >= 5) lua_num2 = PG_Dynamic_Cast<LuaNumber>(params->Value[4]);
	int num2 = string_to_replace.size();
	if (lua_num2) num2 = (int) lua_num2->Value;

	return Return_Variable(new LuaWideString(Value.replace(pos, num, string_to_replace, pos2, num2)));
}


/**************************************************************************************************
* LuaWideString::Lua_Reserve -- equivalent to wstring::reserve
*
* In:				The calling Lua Script
*					Parameter Table {
*						(int) new_capacity
*					}
*
* Out:			A whole lotta nothin' 
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Reserve(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() != 1) {
		Debug_Printf("Lua_Reserve: Expected 1 parameter, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaNumber::Pointer new_capacity = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	FAIL_IF (!new_capacity) {
		Debug_Printf("Lua_Reserve: Incorrect parameter type for new_capacity. Expected int.");
		return NULL;
	}

	Value.reserve((int)new_capacity->Value);
	return NULL;
}


/**************************************************************************************************
* LuaWideString::Lua_Resize -- equivalent to wstring::resize
*
* In:				The calling Lua Script
*					Parameter Table {
*						(int) new_capacity
*					}
*
* Out:				
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Resize(LuaScriptClass *, LuaTable* params)
{
	FAIL_IF(!params || params->Value.size() != 1) {
		Debug_Printf("Lua_Resize: Expected 1 parameter, got %i.\n", params->Value.size());
		return NULL;
	}

	LuaNumber::Pointer new_size = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
	FAIL_IF (!new_size) {
		Debug_Printf("Lua_Resize: Incorrect parameter type for new_size. Expected int.");
		return NULL;
	}

	Value.resize((int)new_size->Value);
	return NULL;
}


/**************************************************************************************************
* LuaWideString::Lua_Size -- equivalent to wstring::size
*
* In:				The calling Lua Script
*					Parameter Table {
*						(int) new_size
*					}
*
* Out:			Number of elements contained in this string
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Size(LuaScriptClass *, LuaTable* )
{
	return Return_Variable(new LuaNumber((float)Value.size()));
}


/**************************************************************************************************
* LuaWideString::Lua_Substr -- equivalent to wstring::substr
*
* In:				The calling Lua Script
*					Parameter Table {
*						(int) [pos]		(default : 0)
*						(int) [num]		(default : Value.size())
*					}
*
* Out:			Substring of Value of length num starting from position pos		
*
* History: 4/13/2006 2:36PM JustinFic
**************************************************************************************************/
LuaTable *LuaWideString::Lua_Substr(LuaScriptClass *, LuaTable* params)
{
	int pos = 0;
	int num = Value.size();

	LuaNumber::Pointer lua_pos;
	LuaNumber::Pointer lua_num;

	if (params) {
		if (params->Value.size() >= 1) lua_pos = PG_Dynamic_Cast<LuaNumber>(params->Value[0]);
		if (params->Value.size() >= 2) lua_num = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
		if (lua_pos) pos = (int)lua_pos->Value - 1;
		if (lua_num) num = (int)lua_num->Value;
	}

	return Return_Variable(new LuaWideString(Value.substr(pos, num)));
}


LuaVar *LuaWideString::Map_Into_Other_Script(LuaScriptClass *)
{
	return new LuaWideString(Value);
}

size_t LuaWideString::Hash_Function(void)
{
	return stdext::hash_value(Value);
}

