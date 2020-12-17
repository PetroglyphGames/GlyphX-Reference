// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScript.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaScript.h $
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

#ifndef LUASCRIPT_H
#define LUASCRIPT_H

#ifndef NDEBUG
#define SCRIPT_DEBUG
#endif


#include "LuaScriptVariable.h"
#include "GetEvent.h"
#include "CRC.h"
#include "PGSignal/SignalGenerator.h"

struct lua_State;
struct lua_filehandler_tag;
class GetEvent;
struct lua_Debug;
class LuaNetworkDebuggerClass;
class LuaDebugCallbackClass;


struct LuaTableMember
{
	int key_type;
	std::string key_value;

	int value_type;
	std::string value_string;
};

typedef std::vector<LuaTableMember> LuaTableMemberList;

/**
 * Wrapper class for a Lua script.  Also handles variable and function
 * mapping to and from Lua.
 */
class LuaScriptClass : public LuaUserVar, public SignalGeneratorClass {
public:

	PG_DECLARE_RTTI();

	friend class LuaNetworkDebuggerClass;

	LuaScriptClass();
	LuaScriptClass(const std::string &script);
	~LuaScriptClass();

	/**
	 * Initialization / Shutdown
	 */
	const std::string & Get_Name(void) const { return Name; }
	void Shutdown(void);
	bool Load_From_File(const std::string &name);
	bool Load_Module(const std::string &name);
	void Collect_Garbage(void);
	void Generate_Full_Path_Name(void);
	static void Generate_Full_Path_Name(const std::string &name, std::string &full_name);
	const std::string &Get_Full_Path_Name(void);
	bool Init_State(void);
	void Prep_For_Save(void);
	void Set_File_Handler(void);

	/**
	 * Variable Mapping To/From Lua
	 */
	LuaVar * Map_Var_From_Lua(void);
	void Map_Table_From_Lua(LuaTable *table);
	void Map_Table_To_Lua(LuaTable *table);
	void Map_Global_To_Lua(LuaVar *var, const char *name);
	LuaVar *Map_Global_From_Lua(const char *name, bool use_maps = false);
	void Map_Var_To_Lua(LuaVar *var);
	virtual LuaVar *Map_Into_Other_Script(LuaScriptClass *new_script);
	virtual bool Is_Equal(const LuaVar *var) const;

	/**
	 * Lua Thread Management
	 */
	int Create_Thread_Function(const char *func_name, LuaVar *param = NULL, bool is_load = false);
	bool Is_One_Thread_Active(void) const;
	int Get_Active_Thread_Count(void) const;
	int Get_Thread_Count(void) const { return (int)ThreadData.size(); }
	void Set_Current_Thread(lua_State *L);
	int Get_Current_Thread_Id(void) const { return CurrentThreadId; }
	GetEvent *Get_Thread_Event_Handler(void) const { return ThreadEventHandler; }
	void Set_Thread_Event_Handler(GetEvent *hand) { ThreadEventHandler = hand; }
	const std::string *Get_Thread_Name(int id) const;
	void Kill_Thread(int id);

	/**
	 * Script Save / Load
	 */
	lua_State *Get_State(void) const { return State; }
	bool Load_State(ChunkReaderClass *reader);
	bool Save_State(ChunkWriterClass *writer);
	ChunkReaderClass *Get_Chunk_Reader(void) { return Reader; }
	ChunkWriterClass *Get_Chunk_Writer(void) { return Writer; }
	int Get_Persist_ID_For_Lua_Function(LuaFunction *func);
	LuaFunction *Get_Lua_Function_For_Persist_ID(int id);

	/**
	 * Script Error Handling
	 */
	void Script_Message(const char *text, ...);
	void Script_Error(const char *text, ...);
	void Script_Warning(const char *text, ...);
	void Set_Alert_Function(void);

	/**
	 * Script Execution Management
	 */
	SmartPtr<LuaVar> Call_Function(const char *name, LuaTable *params, bool use_maps = false);
	SmartPtr<LuaVar> Call_Function(LuaFunction *func, LuaTable *params, bool use_maps = false);
	void Pump_Threads(void);
	void Set_Exit(void) { ExitFlag = true; }
	bool Is_Finished(void) const { return ExitFlag; }
	CRCValue Calculate_CRC(CRCValue seed);
	bool Execute_String(const std::string &text, std::string &result);
	bool Compare_Lua_Functions(LuaFunction *func1, LuaFunction *func2);
	bool Should_Script_Reload(void) const { return ScriptShouldReload; }
	static bool Compare_Lua_Functions(lua_State *L, LuaFunction *func1, LuaFunction *func2);

	/**
	 * Script Debug Support
	 */
	int Debug_Get_Callstack_Depth(int thread_id);
	int Debug_Get_Current_Line_For_Callstack_Level(int thread_id, int level);
	int Debug_Get_Current_Top_Most_Line_Number(int thread_id);
	const char *Debug_Get_Name_For_Callstack_Level(int thread_id, int level);
	void To_String(std::string &outstr);
	static void Debug_Print_Current_Callstack(lua_State *L, bool warning);
	static void Debug_Print_Current_Callstack(lua_State *L, std::string &outstr);
	const std::string & Debug_Dump_Callstack(void);
	static void Debug_Single_Step_Hook(lua_State *L, lua_Debug *dbg);
	LuaTable *Debug_Should_Issue_Event_Alert(LuaScriptClass *, LuaTable *params);
	void Debug_Set_Event_Alerts(int thread_id, bool onoff) { ThreadData[thread_id].EventAlert = onoff; }
	bool Debug_Get_Event_Alerts(int thread_id) const { return ThreadData[thread_id].EventAlert; }
	int Get_Script_ID(void) const { return ScriptID; }
	static void Debug_Get_Var_From_Lua(lua_State *L, int &ltype, std::string &value, bool pop_var = true);
	void Debug_Get_Var_From_Lua(const std::string &name, int &ltype, std::string &value);
	void Debug_Get_Table_From_Lua(const std::string &name,std::vector<int> &descension,LuaTableMemberList &table_members);
	bool Descend_Into_Table(lua_State *L, int index);
	void Debug_Dump_Table_Members(lua_State *L, LuaTableMemberList &values);

	void Debug_Build_Locals_Table(void);
	void Debug_Dump_Local_Variable_Names(std::vector<std::string> &names);
	void Debug_Set_Current_Callstack_Depth(int level);
	void Debug_Get_Callstack(std::vector<std::string> &call_stack);

	/**
	 * Script Pooling Support
	 */
	bool Pool_Is_Fresh_Load(void) const { return PoolFreshLoad; }
	static SmartPtr<LuaScriptClass> Create_Script(const std::string &name, bool reload = false);
	static void Free_Script_Pool(void);
	static void Dump_Lua_Script_Pool_Counts(void);
	static void Check_For_Script_Reload(std::vector<std::string> &files);
	static LuaScriptClass *Find_Active_Script(const std::string &name);

	/**
	 * Remote Debugging Support
	 */
	void Break_For_Step_Out(void);
	void Break_For_Step_Over(void);
	void Debug_Attach(bool auto_attach);
	void Debug_Detach(void);
	void Debug_Attach_Thread(int thread_id);
	void Debug_Get_Loaded_Child_Scripts(std::vector<std::string> &scripts);
	static void Reset_Debugging(void);
	static void Debug_Pre_Execute_String(void);
	static void Debug_Post_Execute_String(void);
	static void Debug_Continue(void);
	static void Debug_Step_In_To(void);
	static void Debug_Step_Over(void);
	static void Debug_Step_Out(void);
	static void Debug_Break_All(void);
	static void Debug_Break_Thread(int thread_id);
	static void Debug_Set_Script_Context(LuaScriptClass *script);
	static void Debug_Add_Breakpoint(LuaScriptClass *script, int thread_id, const std::string & filename, int line, const std::string &condition);
	static void Debug_Remove_Breakpoint(LuaScriptClass *script, int thread_id, const std::string & filename, int line);
	static void Debug_Disable_Attach_All(void);
	static void Debug_Enable_Attach_All(void);
	static void Debug_On_Debugger_Connect(void);


	/**
	 * Static Functions
	 */
	static LuaScriptClass *LuaScriptClass::Create_For_Load(const std::string &name);
	static CRCValue CRC_Lua_Script_Pool(CRCValue crc, bool quick);
	static void System_Initialize(void);
	static void System_Shutdown(void);
	static void Service(void);
	static LuaVar * Map_Var_From_Lua(lua_State *L, bool use_maps = false, bool pop_var = true, bool test_table_recursion = false);
	static void Map_Map_From_Lua(lua_State *L, LuaMap *mapvar, bool test_table_recursion = false);
	static void Map_Map_To_Lua(lua_State *L, LuaMap *mapvar);
	static void Map_Table_From_Lua(lua_State *L, LuaTable *table, bool test_table_recursion = false);
	static void Map_Table_To_Lua(lua_State *L, LuaTable *table);
	static void Map_Global_To_Lua(lua_State *L, LuaVar *var, const char *name);
	static LuaVar *Map_Global_From_Lua(lua_State *L, const char *name, bool use_maps = false);
	static void Map_Var_To_Lua(lua_State *L, LuaVar *var);
	static int Get_Persist_ID_For_Lua_Function(lua_State *L, LuaFunction *func);
	static LuaFunction *Get_Lua_Function_For_Persist_ID(lua_State *L, int id);
	static int Lua_Write(lua_State *L, const void *p, size_t sz, void *ud);
	static const char * Lua_Read(lua_State *L, void *ud, size_t *sz);
	static void Add_Script_Path(const char *path);
	static void Build_Script_Path_String();
	static void Validate_All_Scripts();
	static LuaScriptClass *Get_Script_From_State(lua_State *state);
	static bool Is_Reset_Performed(void) {return(ResetPerformed);}
	static void Clear_Reset_Performed(void) {ResetPerformed = false;}

	//Callbacks
	typedef void (*LogCallbackType)(const char *message);
	typedef void (*FileRegistrationCallbackType)(const char *filename);
	typedef void (*DebuggerServiceCallbackType)(void);

	static void Install_Log_Message_Callback(LogCallbackType callback)							{LogMessageCallback = callback;}
	static void Install_Log_Warning_Callback(LogCallbackType callback)							{LogWarningCallback = callback;}
	static void Install_Log_Error_Callback(LogCallbackType callback)								{LogErrorCallback = callback;}
	static void Install_Register_Script_Callback(FileRegistrationCallbackType callback)		{RegisterScriptCallback = callback;}
	static void Install_Unregister_Script_Callback(FileRegistrationCallbackType callback)	{UnregisterScriptCallback = callback;}
	static void Install_Debugger_Callbacks(LuaDebugCallbackClass *callback)						{LuaDebugCallbacks = callback;}

private:

	typedef std::list<SmartPtr<LuaScriptClass> >	PoolListType;
	typedef stdext::hash_map<std::string, PoolListType> ScriptPoolListType;
	typedef stdext::hash_map<int, LuaScriptClass *> ActiveScriptListType;


	void Unregister_Thread(lua_State *thread);
	void Register_Thread(void);
	void Set_Name_From_Filename(const std::string &filename);
	static const ActiveScriptListType &Get_Active_Script_List(void) { return ActiveScriptList; }
	static int Lua_Error_Handler(lua_State *L);
	static int Lua_Alert_Handler(lua_State *L);
	static int Lua_Compile_Error(lua_State *L);
	static void *Internal_Open_File(struct lua_filehandler_tag *handler, const char *name, const char *mode);
	static int Internal_Close_File(struct lua_filehandler_tag *handler, void *file);
	static char Internal_Peek_Char(struct lua_filehandler_tag *handler, void *file);
	static const char *Internal_Read_File(struct lua_filehandler_tag *handler, void *file, size_t *size);
	static const char *Internal_Error_File(struct lua_filehandler_tag *handler, void *file);

	struct LuaThreadStruct {
		LuaThreadStruct() : Thread(NULL), Thread_Alert_ID(0), EventAlert(false) {}
		lua_State								*Thread;
		LuaVar::Pointer						Thread_Function;
		std::string								Thread_Name;
		int										Thread_Alert_ID;
		bool										EventAlert;
		LuaVar::Pointer						Thread_Param;
	};

	std::vector<LuaThreadStruct>		ThreadData;

	lua_State *								State;
	int										CurrentThreadId;
	std::string								Name;
	std::string								FullName;
	std::string								LastError;
	ChunkReaderClass						*Reader;
	ChunkWriterClass						*Writer;
	SmartPtr<GetEvent>					ThreadEventHandler;
	char										ReadBuff[1];
	char										HandlerBuff[64];

	bool										ExitFlag;
	bool										PoolFreshLoad;
	bool										PoolInUse;
	bool										ScriptIsPooled;
	bool										ScriptShouldCRC;
	bool										ScriptShouldReload;
	int										CRCCount;
	int										SaveID;
	int										ScriptID;
	int										DebugTarget;
	
	static std::vector<std::string>	ScriptPaths;
	static std::string					ScriptPathString;
	static ScriptPoolListType			ScriptPool;
	static int								NextScriptID;
	static ActiveScriptListType		ActiveScriptList;
	static bool								DebugShouldAttachAll;
	static bool								ResetPerformed;

	//Callbacks
	static LogCallbackType						LogMessageCallback;
	static LogCallbackType						LogErrorCallback;
	static LogCallbackType						LogWarningCallback;
	static FileRegistrationCallbackType		RegisterScriptCallback;
	static FileRegistrationCallbackType		UnregisterScriptCallback;
	static SmartPtr<LuaDebugCallbackClass>	LuaDebugCallbacks;
};

class LuaDebugCallbackClass : public RefCountClass
{
public:

	LuaDebugCallbackClass() {}
	virtual ~LuaDebugCallbackClass() {}

	virtual void Suspended_Service(void) {}
	virtual void Script_Suspended(LuaScriptClass *script) { script; }
	virtual void Script_Added(LuaScriptClass *script) { script; }
	virtual void Script_Removed(LuaScriptClass *script) { script; }
};


#endif
