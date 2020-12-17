// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaNetworkDebugger.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaNetworkDebugger.h $
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

#ifndef __LUANETWORKDEBUGGER_H__
#define __LUANETWORKDEBUGGER_H__

#include "RefCount.h"
#include "PGNet/IPAddress.h"
#include "PGNet/Network.h"
#include "LuaScript.h"
#include <vector>


class ConnectionClass;
class PacketClass;
class LuaNetworkDebuggerCallbackClass;
class LuaDebugCallbacks;
class LuaScriptClass;

enum LuaDebuggerPacketEnum {
	LUA_PACKET_INVALID = 0,
	LUA_PACKET_HELLO,
	LUA_PACKET_GOODBYE,
	LUA_PACKET_SCRIPTS,
	LUA_PACKET_THREADS,
	LUA_PACKET_CONSOLE_COMMAND,
	LUA_PACKET_PARSE,
	LUA_PACKET_ADD_BREAKPOINT,
	LUA_PACKET_REMOVE_BREAKPOINT,
	LUA_PACKET_BREAK_ALL,
	LUA_PACKET_STEP_OVER,
	LUA_PACKET_STEP_IN_TO,
	LUA_PACKET_STEP_OUT,
	LUA_PACKET_HEARTBEAT,
	LUA_PACKET_ATTACH,
	LUA_PACKET_SELECT_SCRIPT,
	LUA_PACKET_SELECT_THREAD,
	LUA_PACKET_CONTINUE,
	LUA_PACKET_DUMPLOCALS,
	LUA_PACKET_DUMP_VAR,
	LUA_PACKET_DUMP_TABLE,
	LUA_PACKET_SET_CALLSTACK_DEPTH,
	LUA_PACKET_RESPONSE_CODE,
	LUA_PACKET_RESPONSE_STRING,
	LUA_PACKET_RESPONSE_SCRIPTLIST,
	LUA_PACKET_RESPONSE_THREADLIST,
	LUA_PACKET_RESPONSE_BREAKPOINT_HIT,
	LUA_PACKET_RESPONSE_SCRIPT_ADDED,
	LUA_PACKET_RESPONSE_SCRIPT_REMOVED,
	LUA_PACKET_RESPONSE_SCRIPT_SUSPENDED,
	LUA_PACKET_RESPONSE_DUMP_VAR,
	LUA_PACKET_RESPONSE_DUMP_TABLE,
	LUA_PACKET_RESPONSE_CHILD_SCRIPT_LIST,
	LUA_PACKET_OUTPUT_TO_DEBUGGER,
	LUA_PACKET_EXECUTE_TEXT,
	LUA_PACKET_RESPONSE_EXECUTE_TEXT
};

class LuaNetworkDebuggerClass : public RefCountClass
{
public:

	friend class LuaDebugCallbacks;

	LuaNetworkDebuggerClass();
	virtual ~LuaNetworkDebuggerClass();

	bool System_Initialize(void);
	void System_Shutdown(void);
	bool Start_Server(void);
	bool Stop_Server(void);
	bool Connect_To_Server(const IPAddressClass &address);
	bool Connect_To_Local_Server(int network_port = DEFAULT_NETWORK_PORT);
	bool Is_Initialized(void) const { return IsInitialized; }
	void Service(void);
	void Remove_Connection(int connection_id);
	void Stop_Connection_Attempt(void);
	bool Connection_Unreachable(int connection_id);
	bool Connection_Timed_Out(int connection_id);

	void Set_Callbacks(LuaNetworkDebuggerCallbackClass *callbacks);
	void Get_Debugger_Connections(std::vector<int> &connections);
	const char *Get_Remote_Name(int connection_id);
	void Send_Threads_Request(int connection_id, int script_id);
	void Send_Scripts_Request(int connection_id);
	void Send_Debugger_Control_Request(int connection_id, LuaDebuggerPacketEnum ptype);
	void Send_Attach_Request(int connection_id, int script_id);
	void Send_Detach_Request(int connection_id, int script_id);
	void Send_Debug_Break_All_Request(int connection_id);
	void Send_Debug_Step_Into_Request(int connection_id);
	void Send_Debug_Step_Out_Request(int connection_id);
	void Send_Debug_Step_Over_Request(int connection_id);
	void Send_Debug_Continue_Request(int connection_id);
	void Send_Select_Script_Request(int connection_id, int script_id);
	void Send_Select_Thread_Request(int connection_id, int thread_id);
	void Send_Dump_Var_Request(int connection_id, int script_id, const std::string &name);
	void Send_Dump_Table_Request(int connection_id, int script_id, const std::string &name, std::vector<int> &decension, int table_id);
	void Send_Add_Breakpoint_Request(int connection_id, int script_id, int thread_id, const char *filename, int line, const char *condition);
	void Send_Remove_Breakpoint_Request(int connection_id, int script_id, int thread_id, const char *filename, int line);
	void Send_Set_Callstack_Depth_Request(int connection_id, int script_id, int level);
	void Send_Execute_Text_Request(int connection_id, int script_id, const char* text);
	void Send_Confirm_Connection(int connection_id);

	static bool Start_Debug_Server(void);
	static void Stop_Debug_Server(void);
	static void Service_Debug_Server(void);
	static LuaNetworkDebuggerClass *Get_Instance(void) { return _Instance; }
	static bool Is_Being_Debugged(void);
	static void Set_Default_Network_Port(int port) {DefaultNetworkPort = port;}

	// Debug output destinations
	enum {
		DEST_DEBUG_OUTPUT = 0,
		DEST_LUA_CONSOLE
	};

	static void Send_Output_To_Debugger(const std::string &message,int destination = 0);


private:

	struct ClientConnectionClass;

	void Process_Set_Callstack_Depth_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Select_Thread_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Select_Script_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Child_Script_List_Response(ClientConnectionClass &conn, PacketClass *ipacket);
	void Send_Child_Script_List_Response(ClientConnectionClass &conn, LuaScriptClass *script);
	void Send_Script_Suspended_Response(LuaScriptClass *script);
	void Send_Script_Added_Response(LuaScriptClass *script);
	void Send_Script_Removed_Response(LuaScriptClass *script);
	void Process_Script_Suspended_Response(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Script_Added_Response(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Script_Removed_Response(ClientConnectionClass &conn, PacketClass *ipacket);
	void Send_Goodbye(ClientConnectionClass &conn);
	void Send_Heartbeat(ClientConnectionClass &conn);
	void Send_Hello(ClientConnectionClass &conn);
	void Parse_Packet(ClientConnectionClass &conn, PacketClass *packet);
	void Process_Scripts_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Scripts_Response(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Threads_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Threads_Response(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Attach_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Detach_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Dump_Var_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Dump_Var_Response(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Dump_Table_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Dump_Table_Response(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Add_Breakpoint_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Remove_Breakpoint_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Output_To_Debugger(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Execute_Text_Request(ClientConnectionClass &conn, PacketClass *ipacket);
	void Process_Execute_Text_Response(ClientConnectionClass &conn, PacketClass *ipacket);


	enum ConnectStateEnum {
		CONNECT_STATE_INVALID = 0,
		CONNECT_CLIENT_CONNECTING,
		CONNECT_SERVER_CONNECTING,
		CONNECT_SENT_HELLO,
		CONNECT_RECEIVED_HELLO,
		CONNECT_SENT_GOODBYE,
		CONNECT_RECEIVED_GOODBYE,
		CONNECT_IDLE,
	};

	struct ClientConnectionClass
	{
		ClientConnectionClass() : Connection(NULL), ConnectState(CONNECT_STATE_INVALID), AsServer(false) {}
		IPAddressClass			Address;
		ConnectionClass		*Connection;
		ConnectStateEnum		ConnectState;
		bool						AsServer;
		int						ID;
	};

	static LuaNetworkDebuggerClass 			*_Instance;

	std::vector<ClientConnectionClass>		ClientConnections;
	bool												IsInitialized;
	bool												NetworkInitByLua;
	int												ActiveClientIndex;
	SmartPtr<LuaNetworkDebuggerCallbackClass>		Callbacks;
	std::string										BasePath;

	static int										DefaultNetworkPort;
};


class LuaNetworkDebuggerCallbackClass : public RefCountClass
{
public:

	LuaNetworkDebuggerCallbackClass() {}
	virtual ~LuaNetworkDebuggerCallbackClass() {}

	typedef std::vector<std::pair<int, std::string> > ThreadListType;
	typedef std::vector<std::pair<int, std::string> > ScriptListType;

	virtual void Script_Suspended_Response(int connection_id, int script_id, int thread_id, std::string &filename, 
														const std::vector<std::string> &call_stack, ThreadListType &thread_list) = 0;

	virtual void Dump_Var_Response(int connection_id, int script_id, std::string &name, int ltype, const std::string &result) = 0;
	virtual void Dump_Table_Response(int connection_id, int table_id, LuaTableMemberList& table_members) = 0;
	virtual void Threads_Response(int connection_id, ThreadListType &) = 0;
	virtual void Scripts_Response(int connection_id, ScriptListType &) = 0;
	virtual void Server_Connected_Response(int connection_id) = 0;
	virtual void Child_Script_List_Response(int connection_id, int script_id, std::vector<std::string> &slist) = 0;
	virtual void Print_Output_To_Debugger(std::string &,int destination) = 0;
	virtual void Script_Added_Response(int connection_id, int script_id, std::string &filename) = 0;
	virtual void Script_Removed_Response(int connection_id, int script_id) = 0;
	virtual void Execute_Text_Response(int connection_id, int script_id, std::string &result) = 0;
};



#endif // __LUANETWORKDEBUGGER_H__
