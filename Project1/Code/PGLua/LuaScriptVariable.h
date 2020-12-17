// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScriptVariable.h#3 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScriptVariable.h $
//
//    Original Author: Brian Hayes
//
//            $Author: Steve_Tall $
//
//            $Change: 747006 $
//
//          $DateTime: 2020/10/20 16:15:42 $
//
//          $Revision: #3 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////
/** @file */

#ifndef LUASCRIPTVARIABLE_H
#define LUASCRIPTVARIABLE_H


#include "Always.h"
#include "MemoryPool.h"
#include "RunTimeType.h"
#include "RefCount.h"
#include "ChunkFile.h"
#include "SaveLoad.h"
#include "PooledSTLAllocator.h"
#include <map>

#define LUA_VALUE_POOL_SIZE 128
#define LUA_WRAPPER_POOL_SIZE 64

/**
 * Chunkids for persistent LuaUserVar objects.
 * @since 4/22/2004 2:06:05 PM -- BMH
 */
enum LuaChunkId
{
	LUA_CHUNK_INVALID = 0,

	LUA_CHUNK_USER_VAR = 55,
	LUA_CHUNK_SCRIPT_MESSAGE,
	LUA_CHUNK_SCRIPT_EXIT,
	LUA_CHUNK_SCRIPT_GETTIME,
	LUA_CHUNK_GETEVENT,
	LUA_CHUNK_DEBUG_PRINT,
	LUA_CHUNK_SCRIPT_GETEVENT,
	LUA_CHUNK_SCRIPT_HEADER,
	LUA_CHUNK_SCRIPT_INTERNAL,
	LUA_CHUNK_NEVER_BLOCK,
	LUA_CHUNK_THREAD_VALUE,
	LUA_CHUNK_LUA_SCRIPT_WRAPPER,
	LUA_CHUNK_EXTERNAL_FUNCTION,

	LUA_CHUNK_LIBRARY_RESERVED = 1024,

	LUA_CHUNK_LIBRARY_FIRST,

	//Space for libraries
	LUA_CHUNK_GUI_BEGIN = LUA_CHUNK_LIBRARY_FIRST,
	LUA_CHUNK_GUI_RESERVED = 2048,

	LUA_CHUNK_LIBRARY_LAST,

	//Application ID space
	LUA_USER_APP_BEGIN = LUA_CHUNK_LIBRARY_LAST,
};

enum LuaVarType
{
	LUA_VAR_TYPE_INVALID = 0,
	LUA_VAR_TYPE_MAP,
	LUA_VAR_TYPE_TABLE,
	LUA_VAR_TYPE_VOID,
	LUA_VAR_TYPE_NUMBER,
	LUA_VAR_TYPE_BOOL,
	LUA_VAR_TYPE_STRING,
	LUA_VAR_TYPE_THREAD,
	LUA_VAR_TYPE_FUNCTION,
	LUA_VAR_TYPE_USER_VAR,
	LUA_VAR_TYPE_POINTER,
};

/**
 * Forward Declarations
 */
struct lua_State;
struct LuaWrapper;
class LuaVar;
class LuaUserVar;
class LuaScriptClass;
class LuaFunctionCallHelper;
class LuaHashCompare;
template <class T, LuaVarType _VarType> class LuaValue;

/**
 * Lua Typedefs
 */
typedef void (*lua_function_t)();
typedef void (*lua_thread_t)(int);
typedef std::map<SmartPtr<LuaVar>, SmartPtr<LuaVar>, LuaHashCompare> 			LuaMapType;
typedef LuaValue<LuaMapType, LUA_VAR_TYPE_MAP> 											LuaMap;
typedef LuaValue<std::vector<SmartPtr<LuaVar> >, LUA_VAR_TYPE_TABLE> 			LuaTable;
typedef LuaValue<void *, LUA_VAR_TYPE_VOID> 												LuaVoid;
typedef LuaValue<float, LUA_VAR_TYPE_NUMBER> 											LuaNumber;
typedef LuaValue<bool, LUA_VAR_TYPE_BOOL> 												LuaBool;
typedef LuaValue<std::string, LUA_VAR_TYPE_STRING> 									LuaString;
typedef LuaValue<lua_thread_t, LUA_VAR_TYPE_THREAD> 									LuaThread;
typedef LuaValue<lua_function_t, LUA_VAR_TYPE_FUNCTION> 								LuaFunction;
typedef LuaValue<void *, LUA_VAR_TYPE_POINTER> 											LuaPointer;

/**
 * Safe Cast Macro for LuaVar objects.  Wraps the cast in a SmartPtr so
 * that a cast from the result of an object factory doesn't leak.
 * @since 4/22/2004 2:09:10 PM -- BMH
 */
#define LUA_SAFE_CAST(type, var) (PG_Dynamic_Cast<type>(SmartPtr<LuaVar>(var)))

/**
 * Tracks the Creation factory function for this Lua Object.
 * 
 * @see LUA_DECLARE_FACTORY
 * @since 4/22/2004 2:09:01 PM -- BMH
 */
#define LUA_IMPLEMENT_FACTORY(chunk_id, class_name) \
	LuaFactoryReg		PG_JOIN(__, PG_JOIN(class_name, Reg)) (chunk_id, class_name::FactoryCreate) 

/**
 * Macro for the Declaration of a LuaUserVar CreationFactory.
 * 
 * @see LUA_IMPLEMENT_FACTORY
 * @since 4/22/2004 2:10:04 PM -- BMH
 */
#define LUA_DECLARE_FACTORY(chunk_id, class_name) \
	public: \
	static LuaUserVar *FactoryCreate(int cid = chunk_id) { LuaUserVar *var = new class_name(); \
		var->Set_Chunk_Id(cid); return var; }

/**
 * Macro to aid in registering a Member function to Lua.  Takes the type,
 * Lua name, and MemberFunction pointer and creates a Wrapper object that
 * maps the function call to the Member.
 * @since 4/22/2004 2:16:14 PM -- BMH
 */
#define LUA_REGISTER_MEMBER_FUNCTION(type, name, func) \
	Register_Member(name, new LuaMemberFunctionWrapper<type>(this, func))
	
#define LUA_REGISTER_MEMBER_FUNCTION_USE_MAPS(type, name, func) \
	Register_Member(name, new LuaMemberFunctionWrapper<type>(this, func, true))

/**
 * Utility macro for use in setting up Lua meta-tables.
 */
#define lua_setmetafunc(L, n, s, f) ( \
		lua_pushlstring(L, n, s), \
		lua_pushcfunction(L, f), \
		lua_rawset(L, -3) )

#define LUA_WRITE_CHUNK_VALUE_PTR(id, val, script)			\
{																			\
	ok &= writer->Begin_Chunk(id);								\
	ok &= Lua_Save_Variable(writer, val, script);			\
	ok &= writer->End_Chunk();										\
}																			\

#define LUA_READ_CHUNK_VALUE_PTR(id, val, script)			\
	case id:																\
	{																		\
		ok &= Lua_Load_Variable(reader, (SmartPtr<LuaVar> &)(val), script);		\
		break;															\
	}																		\

/**
 * Lua User Variable registration factory.  Maintains the creation factory
 * hash_map.  Create new objects on load.
 * 
 * @see LUA_DECLARE_FACTORY
 * @since 4/22/2004 2:11:49 PM -- BMH
 */
class LuaFactoryReg
{
public:
	typedef LuaUserVar *(*CreateFunctionType)(int type);

	LuaFactoryReg(int type, CreateFunctionType function)
	{
		if (!FunctionMap) {
			FunctionMap = new stdext::hash_map<int, CreateFunctionType>();
		}

		// Check for Chunk ID Collision.
		assert(FunctionMap->find(type) == FunctionMap->end());

		(*FunctionMap)[type] = function;
	}
	~LuaFactoryReg()
	{
		if (FunctionMap) delete FunctionMap;
		FunctionMap = NULL;
	}

	/**
	 * Creates a LuaUserVar object based on the ChunkId the variable
	 * is associated with.
	 * 
	 * @param type   ChunkId of the Variable to be created.
	 * 
	 * @return The new LuaUserVar object.
	 * @since 4/22/2004 2:13:15 PM -- BMH
	 */
	static LuaUserVar *Create_User_Var(int type)
	{
		stdext::hash_map<int, CreateFunctionType>::iterator it = FunctionMap->find(type);
		if (it == FunctionMap->end()) return NULL;
		CreateFunctionType func = it->second;
		return func(type);
	}

private:
	static stdext::hash_map<int, CreateFunctionType>		*FunctionMap;
};

/**
 * Base class representation of a Lua Variable.
 */
class LuaVar : public RefCountClass
{
public:

	typedef SmartPtr<LuaVar> Pointer;

	PG_DECLARE_RTTI();
	virtual ~LuaVar() {}
	virtual LuaVarType Get_Var_Type(void) = 0;
};

/**
 * Templated class for the implicit Lua variables.
 */
template <class T, LuaVarType _VarType>
class LuaValue : public LuaVar, public PooledObjectClass<LuaValue<T, _VarType>, LUA_VALUE_POOL_SIZE> {
public:

	typedef SmartPtr<LuaValue<T, _VarType> > Pointer;

	LuaValue() {}
	LuaValue(const T &val) : Value(val) {}
	PG_DECLARE_RTTI();

	size_t Hash_Function(void)
	{
		return stdext::hash_value<T>(Value);
	}
	bool Hash_Compare(const T&val) const
	{
		return Value < val;
	}
	LuaVarType Get_Var_Type(void)
	{
		return _VarType;
	}
	T	Value;
};

enum LuaPersistMethod {
	LUA_PERSIST_METHOD_INVALID,
	LUA_PERSIST_METHOD_LINK,
	LUA_PERSIST_METHOD_OBJECT,
	LUA_PERSIST_METHOD_NULL,
};

enum {
	LUA_CHUNK_USERVAR_PERSIST_METHOD,
	LUA_CHUNK_USERVAR_CHUNK_ID,
	LUA_CHUNK_USERVAR_CHUNK_DATA,
	LUA_CHUNK_USERVAR_LINK_ID,
	LUA_CHUNK_USERVAR_WRAPPER_ID,
	LUA_CHUNK_USERVAR_INTERNAL_DATA,
	LUA_CHUNK_VAR_CHUNK_DATA,
};

bool Lua_Save_Variable(ChunkWriterClass *writer, LuaVar *var, LuaScriptClass *script);
bool Lua_Load_Variable(ChunkReaderClass *reader, SmartPtr<LuaVar> &var, LuaScriptClass *script);

bool Lua_Save_User_Var(ChunkWriterClass *writer, LuaScriptClass *script, LuaUserVar *val);
template <typename T>
bool Lua_Load_User_Var(ChunkReaderClass *reader, LuaScriptClass *script, SmartPtr<T> &val, void *new_ud)
{
	bool ok = true;
	LuaPersistMethod meth = LUA_PERSIST_METHOD_INVALID;
	int chunk_id = LUA_CHUNK_INVALID;
	void *link_id = NULL;
	void *wrapper_id = NULL;
	LuaUserVar * uvar = NULL;

	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case LUA_CHUNK_USERVAR_PERSIST_METHOD:
				ok &= reader->Read(&meth, sizeof(meth));
				break;

			case LUA_CHUNK_USERVAR_INTERNAL_DATA:
				assert(meth == LUA_PERSIST_METHOD_OBJECT);
				assert(chunk_id != LUA_CHUNK_INVALID);
				uvar = LuaFactoryReg::Create_User_Var((LuaChunkId)chunk_id);
				assert(uvar);
				if (uvar) ok &= uvar->Internal_Load(reader);
				break;

			case LUA_CHUNK_USERVAR_CHUNK_DATA:
				assert(meth == LUA_PERSIST_METHOD_OBJECT);
				assert(uvar);
				if (uvar) ok &= uvar->Load(script, reader);
				// Assert if the user var didn't close it's chunk properly.
				assert(reader->Cur_Chunk_ID() == LUA_CHUNK_USERVAR_CHUNK_DATA);
				break;

			case LUA_CHUNK_USERVAR_CHUNK_ID:
				assert(meth == LUA_PERSIST_METHOD_OBJECT);
				ok &= reader->Read(&chunk_id, sizeof(chunk_id));
				break;

			case LUA_CHUNK_USERVAR_WRAPPER_ID:
				ok &= reader->Read(&wrapper_id, sizeof(wrapper_id));
				break;

			case LUA_CHUNK_USERVAR_LINK_ID:
				ok &= reader->Read(&link_id, sizeof(link_id));
				break;

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	if (wrapper_id && new_ud) {
		SaveLoadClass::Register_Pointer(wrapper_id, new_ud);
	}

	if (meth == LUA_PERSIST_METHOD_OBJECT) {
		assert(uvar);
		SAVE_LOAD_REQUEST_FIXUP((void **)&uvar->Internal_Get_Wrapper(), wrapper_id);
		SaveLoadClass::Register_Pointer(link_id, uvar);
		val = NULL;
		SAVE_LOAD_REQUEST_REF_FIXUP((void **)&val, ((T*)1), link_id);
	} else if (meth == LUA_PERSIST_METHOD_LINK) {
		val = NULL;
		SAVE_LOAD_REQUEST_REF_FIXUP((void **)&val, ((T*)1), link_id);
	} else {
		assert(meth == LUA_PERSIST_METHOD_NULL);
	}

	return (ok);
}

LuaTable * Alloc_Lua_Table(void);
void Free_Lua_Table(const SmartPtr<LuaTable> &table);
void Manage_Lua_Tables(void);
void Init_Lua_Table_Pool(void);
void Shutdown_Lua_Table_Pool(void);

/**
 * Convienent function to stuff a Lua var into a table for return
 * values or as parameters to lua functions.
 * 
 * @param var    lua variable.
 * 
 * @return new lua table.
 * @since 3/10/2005 6:59:00 PM -- BMH
 */
inline LuaTable *Lua_Return_Variable(LuaVar *var)
{
	if (!var) return NULL;
	LuaTable *tab = Alloc_Lua_Table();
	tab->Value.push_back(var);
	return tab;
}

/**
 * Base class all User-Defined Lua variables need to derive from.
 */
class LuaUserVar : public LuaVar
{
public:
	PG_DECLARE_RTTI();
	LuaUserVar(int id = LUA_CHUNK_INVALID, bool register_member_functions = true);
	~LuaUserVar();
	LuaUserVar(const LuaUserVar &other);

	friend struct LuaWrapper;
	virtual LuaVar *Index_Function(const char *Key);
	virtual size_t Hash_Function(void);
	virtual bool Hash_Compare(const LuaUserVar *val) const;
	virtual bool Is_Equal(const LuaVar *val) const;
	virtual LuaTable *Function_Call(LuaScriptClass * /*script*/, LuaTable * /*params*/);
	void Register_Member(const char *name, LuaVar *function);
	virtual void To_String(std::string &outstr);
	const std::string &Get_To_String(void);
	virtual LuaVar *Map_Into_Other_Script(LuaScriptClass *new_script);
	virtual bool Can_Map_Into_Script(LuaScriptClass * /*new_script*/) { return true; }
	virtual bool Save(LuaScriptClass * /*script*/, ChunkWriterClass * /*writer*/) { return true; };
	virtual bool Load(LuaScriptClass * /*script*/, ChunkReaderClass * /*reader*/) { return true; };
	virtual void Post_Load(LuaScriptClass * /*script*/) {};

	int Get_Chunk_Id(void) const { return ChunkId; }
	void Set_Chunk_Id(int id) { ChunkId = id; }

	void Set_Wrapper(LuaWrapper *wrap) { Wrapper = wrap; }
	LuaWrapper *Get_Wrapper(void) { return Wrapper; }
	bool Internal_Save(ChunkWriterClass *writer);
	bool Internal_Load(ChunkReaderClass *reader);
	LuaWrapper *&Internal_Get_Wrapper(void) { return Wrapper; }

	// Derived Uservars are not allowed to overload this.
	LuaVarType Get_Var_Type() { return LUA_VAR_TYPE_USER_VAR; } 

	LuaTable *Return_Variable(LuaVar *var) { return Lua_Return_Variable(var); }

	virtual bool Get_Use_Maps() const { return false; }

	virtual LuaTable *Is_Pool_Safe(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(true)); }

protected:
	typedef stdext::hash_map<std::string, SmartPtr<LuaVar>, stdext::hash_compare<std::string, std::less<std::string> >> MemberMapType;
	MemberMapType				*MemberMap;

private:
//    typedef std::map<std::string, SmartPtr<LuaVar>, std::less<std::string>,
//       PooledSTLAllocatorClass<std::pair<std::string, SmartPtr<LuaVar> >, 128> > MemberMapType;

	LuaWrapper					*Wrapper;
	int							ChunkId;
};

/**
 * Wrapper class the stores a LuaUserVar object and pointer to a 
 * LuaUserVar member function.  When the object is called the call
 * maps back to the member function.
 * @since 4/22/2004 2:19:12 PM -- BMH
 */
template <typename T>
class LuaMemberFunctionWrapper : public LuaUserVar
{
public:

	typedef LuaTable * (T::*MemberFunctionPtr)(LuaScriptClass *, LuaTable *);

	LuaMemberFunctionWrapper(T * obj, MemberFunctionPtr func, bool use_maps = false) : 
			LuaUserVar(LUA_CHUNK_INVALID, false), MemberFunction(func), Object(obj), UseMaps(use_maps) {}

	LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params)
	{
		assert(Object);
		assert(MemberFunction);
	
		// Call the member function pointer using Object's v-table.
		// see http://linuxquality.sunsite.dk/articles/memberpointers/ for a better explanation of member pointers.
		return ((*Object).*MemberFunction)(script, params);
	}

	virtual bool Get_Use_Maps() const { return UseMaps; }

private:
	MemberFunctionPtr		MemberFunction;
	T *						Object;
	bool						UseMaps;
};

struct FunctionFixup
{
	FunctionFixup(LuaScriptClass *s, int p, void **var, SmartPtr<LuaVar> *sp) : 
		script(s), pid(p), varptr(var), smart_varptr(sp) {}
	LuaScriptClass 			*script;
	int							pid;
	SmartPtr<LuaVar> 			*smart_varptr;
	void 							**varptr;
};

/**
 * LuaUserVar Wrapper object.  This is the Lua side representation
 * of a LuaUserVar object.  Maps the Lua meta-methods of Garbage
 * Collection, Function calling, Indexing to appropriate LuaUserVar
 * methods.
 * @since 4/22/2004 2:21:30 PM -- BMH
 */
struct LuaWrapper
{
	// Need to implement the Garbage collection callback and set Var = 0 in it.
	SmartPtr<LuaUserVar>		Var;
	lua_State *					State;

	static void Set_Meta_Table(lua_State *L);
	static int Index_Function (lua_State *L);
	static int Garbage_Collector(lua_State *L);
	static int Function_Call(lua_State *L);
	static int To_String(lua_State *L);
	static int Test_Equal(lua_State *L);
	static int Persist_Object(lua_State *L, int loading, int id, void *ud, size_t sz, void *data);
	static void Do_Fixup(void *);
	static void Request_Function_Fixup(LuaScriptClass *script, void **var, int pid);
	static void Request_Function_Smart_Fixup(LuaScriptClass *script, SmartPtr<LuaVar> *var, int pid);

private:
	static std::vector<FunctionFixup> function_fixups;
};

#define _LUA_PTR_CAST(x, y) ((y *)((LuaVar *)x))

class LuaHashCompare /*: stdext::hash_compare<SmartPtr<LuaVar> >*/
{
public:
	enum {   // parameters for hash table
		bucket_size = 4,  // 0 < bucket_size
		min_buckets = 8   // min_buckets = 2 ^^ N, 0 < N
	};

	size_t operator()(const SmartPtr<LuaVar>& Left) const;

	bool operator()(const SmartPtr<LuaVar>& Left, const SmartPtr<LuaVar>& Right) const;
};

// specialized SmartPtr<LuaVar> comparison
// template <class T>
bool SmartPtr<LuaVar>::operator==(const SmartPtr<LuaVar> &Right) const
{
	LuaVar *Left = Obj;
	if (Left->Get_Var_Type() == Right->Get_Var_Type())
	{
		switch (Left->Get_Var_Type())
		{
			case LUA_VAR_TYPE_NUMBER:
				return _LUA_PTR_CAST(Left, LuaNumber)->Value == _LUA_PTR_CAST(Right, LuaNumber)->Value;

			case LUA_VAR_TYPE_BOOL:
				return _LUA_PTR_CAST(Left, LuaBool)->Value == _LUA_PTR_CAST(Right, LuaBool)->Value;

			case LUA_VAR_TYPE_STRING:
				return _LUA_PTR_CAST(Left, LuaString)->Value == _LUA_PTR_CAST(Right, LuaString)->Value;

			case LUA_VAR_TYPE_USER_VAR:
				return _LUA_PTR_CAST(Left, LuaUserVar)->Is_Equal(Right);

			case LUA_VAR_TYPE_MAP:
				return _LUA_PTR_CAST(Left, LuaMap)->Value == _LUA_PTR_CAST(Right, LuaMap)->Value;

			case LUA_VAR_TYPE_TABLE:
				return _LUA_PTR_CAST(Left, LuaTable)->Value == _LUA_PTR_CAST(Right, LuaTable)->Value;

			case LUA_VAR_TYPE_VOID:
			case LUA_VAR_TYPE_FUNCTION:
			case LUA_VAR_TYPE_THREAD:
			case LUA_VAR_TYPE_POINTER:
			default:
				return _LUA_PTR_CAST(Left, LuaPointer)->Value == _LUA_PTR_CAST(Right, LuaPointer)->Value;
		}
	}

	if (Left->Get_Var_Type() == LUA_VAR_TYPE_USER_VAR) {
		return _LUA_PTR_CAST(Left, LuaUserVar)->Is_Equal(Right);
	} else if (Right->Get_Var_Type() == LUA_VAR_TYPE_USER_VAR) {
		return _LUA_PTR_CAST(Right, LuaUserVar)->Is_Equal(Left);
	}
	return false;
}

bool SmartPtr<LuaVar>::operator!=(const SmartPtr<LuaVar> &Right) const
{
	return !operator==(Right);
}

// specialized SmartPtr<LuaVar> comparison
// Hashmap calls this when there's a hash collision
bool SmartPtr<LuaVar>::operator< (const SmartPtr<LuaVar> &Right) const
{
	LuaVar *Left = Obj;
	if (Left->Get_Var_Type() == Right->Get_Var_Type())
	{
		switch (Left->Get_Var_Type())
		{
			case LUA_VAR_TYPE_NUMBER:
				return _LUA_PTR_CAST(Left, LuaNumber)->Value < _LUA_PTR_CAST(Right, LuaNumber)->Value;

			case LUA_VAR_TYPE_BOOL:
				return _LUA_PTR_CAST(Left, LuaBool)->Value < _LUA_PTR_CAST(Right, LuaBool)->Value;

			case LUA_VAR_TYPE_STRING:
				return _LUA_PTR_CAST(Left, LuaString)->Value < _LUA_PTR_CAST(Right, LuaString)->Value;

			case LUA_VAR_TYPE_USER_VAR:
				return _LUA_PTR_CAST(Left, LuaUserVar)->Hash_Compare(_LUA_PTR_CAST(Right, LuaUserVar));

			case LUA_VAR_TYPE_MAP:
				{
					LuaMap *left_map = _LUA_PTR_CAST(Left, LuaMap);
					LuaMap *right_map = _LUA_PTR_CAST(Right, LuaMap);
					return left_map->Value < right_map->Value;
				}
				break;

			case LUA_VAR_TYPE_TABLE:
				{
					LuaTable *left_table = _LUA_PTR_CAST(Left, LuaTable);
					LuaTable *right_table = _LUA_PTR_CAST(Right, LuaTable);
					return left_table->Value < right_table->Value;
				}
				break;

			case LUA_VAR_TYPE_VOID:
			case LUA_VAR_TYPE_FUNCTION:
			case LUA_VAR_TYPE_THREAD:
			case LUA_VAR_TYPE_POINTER:
			default:
				return _LUA_PTR_CAST(Left, LuaPointer)->Value < _LUA_PTR_CAST(Right, LuaPointer)->Value;
		}
	}
	return (Left->Get_Var_Type() < Right->Get_Var_Type());
}

#endif // XMLSCRIPTVARIABLE_H

