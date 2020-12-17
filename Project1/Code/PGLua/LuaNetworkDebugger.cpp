// $Id: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaNetworkDebugger.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/PGLua/LuaNetworkDebugger.cpp $
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

#include "Always.h"
#include "PGNet/Network.h"
#include "LuaNetworkDebugger.h"
#include "PGNet/PacketType.h"
#include "PGNet/PacketAllocator.h"
#include "Text.h"


#define LUA_DEBUGGER_PACKET_MAGIC_NUMBER 					  	13
#define LUA_DEBUGGER_PACKET_SIGNIFICANT_BITS				  	4
PacketTypeClass LuaDebuggerPacketType(LUA_DEBUGGER_PACKET_MAGIC_NUMBER, LUA_DEBUGGER_PACKET_SIGNIFICANT_BITS);

int LuaNetworkDebuggerClass::DefaultNetworkPort = DEFAULT_NETWORK_PORT;


class LuaDebugCallbacks : public LuaDebugCallbackClass
{
public:
	LuaDebugCallbacks(LuaNetworkDebuggerClass *parent) : Parent(parent) {}
	virtual void Script_Suspended(LuaScriptClass *script)
	{
		Parent->Send_Script_Suspended_Response(script);
	}
	virtual void Suspended_Service(void)
	{
		Parent->Service();
	}
	virtual void Script_Added(LuaScriptClass *script)
	{
		Parent->Send_Script_Added_Response(script);
	}
	virtual void Script_Removed(LuaScriptClass *script)
	{
		Parent->Send_Script_Removed_Response(script);
	}

private:
	LuaDebugCallbacks() { assert(false); }
	LuaNetworkDebuggerClass *Parent;
};

/**
 * Constructor
 * @since 1/29/2006 4:26:32 PM -- BMH
 */
LuaNetworkDebuggerClass::LuaNetworkDebuggerClass() : 
	NetworkInitByLua(false)
,	IsInitialized(false)
,	ActiveClientIndex(-1)
{
	assert(_Instance == NULL);
	_Instance = this;
}

/**
 * Destructor
 * @since 1/29/2006 4:26:37 PM -- BMH
 */
LuaNetworkDebuggerClass::~LuaNetworkDebuggerClass()
{
	if (IsInitialized) 
	{
		System_Shutdown();
	}
	if (_Instance == this)
	{
		_Instance = NULL;
	}
}

LuaNetworkDebuggerClass * LuaNetworkDebuggerClass::_Instance = NULL;
static SmartPtr<LuaNetworkDebuggerClass> smart_instance;
static bool our_init = false;

bool LuaNetworkDebuggerClass::Start_Debug_Server(void)
{
	if (_Instance == NULL)
	{
		smart_instance = new LuaNetworkDebuggerClass();
		FAIL_IF (smart_instance->System_Initialize() == false)
		{
			smart_instance = NULL;
			return false;
		}
		FAIL_IF (smart_instance->Start_Server() == false)
		{
			smart_instance->System_Shutdown();
			smart_instance = NULL;
			return false;
		}
	}
	else
	{
		if (_Instance->Is_Initialized() == false)
		{
			FAIL_IF(_Instance->System_Initialize() == false) { return false; }
			our_init = true;
		}
		FAIL_IF (_Instance->Start_Server() == false)
		{
			return false;
		}
	}
	return true;
}

void LuaNetworkDebuggerClass::Stop_Debug_Server(void)
{
	if (smart_instance)
	{
		smart_instance->Stop_Server();
		smart_instance->System_Shutdown();
	}
	else if (_Instance)
	{
		_Instance->Stop_Server();
		if (our_init)
		{
			_Instance->System_Shutdown();
		}
	}
	smart_instance = NULL;
}

void LuaNetworkDebuggerClass::Service_Debug_Server(void)
{
	if (smart_instance)
	{
		smart_instance->Service();
	}
}


bool LuaNetworkDebuggerClass::System_Initialize(void)
{
	bool ok = true;

	if (NetworkClass::Is_Initialized() == false)
	{
		ok = false;
		unsigned short port = (unsigned short) DefaultNetworkPort;
		if (!NetworkClass::Is_Initialized()) {
			for (int i = 0; i < 10; i++)
			{
				ok = NetworkClass::Init(port++);
				if (ok) {
					NetworkInitByLua = true;
					break;
				}
			}
		} else {
			ok = true;
		}
		if (!ok) 
			return false;

		ConnectionManagerClass *conmgr = NetworkClass::Get_Connection_Manager();
		FAIL_IF(!conmgr)
		{
			NetworkClass::Shutdown();
			return false;
		}

		char filename[MAX_PATH];
		if (GetModuleFileName(NULL, &filename[0], MAX_PATH))
		{
			filename[MAX_PATH-1] = 0;
			std::string tname;
			String_Printf(tname, "%s:%d", Strip_Path_And_Extension(filename), GetCurrentProcessId());
			conmgr->Set_My_Name(tname.c_str());
		}
	}

	char tbuff[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, &tbuff[0]);
	tbuff[MAX_PATH-1] = 0;
	BasePath = Build_Uppercase_String(std::string(tbuff));
	LuaScriptClass::Install_Debugger_Callbacks(new LuaDebugCallbacks(this));
	ActiveClientIndex = -1;

	IsInitialized = true;

	return (ok);
}

void LuaNetworkDebuggerClass::System_Shutdown(void)
{
	if (IsInitialized == false) return;

	if (NetworkInitByLua)
	{
		if (NetworkClass::Is_Initialized()) {
			NetworkClass::Shutdown();
			NetworkInitByLua = false;
		}
	}
	LuaScriptClass::Install_Debugger_Callbacks(NULL);
	IsInitialized = false;
}

bool LuaNetworkDebuggerClass::Start_Server(void)
{
	ConnectionManagerClass *conmgr = NetworkClass::Get_Connection_Manager();
	FAIL_IF(!conmgr) return false;

	if (conmgr->Is_Listening() == false)
	{
		return conmgr->Start_Listening();
	}
	return true;
}

bool LuaNetworkDebuggerClass::Stop_Server(void)
{
	ConnectionManagerClass *conmgr = NetworkClass::Get_Connection_Manager();
	FAIL_IF(!conmgr) return false;

	if (conmgr->Is_Listening())
	{
		return conmgr->Stop_Listening();
	}
	return true;
}

void LuaNetworkDebuggerClass::Set_Callbacks(LuaNetworkDebuggerCallbackClass *callbacks)
{
	Callbacks = callbacks;
}

const char *LuaNetworkDebuggerClass::Get_Remote_Name(int connection_id)
{
	FAIL_IF(connection_id < 0 || connection_id >= (int)ClientConnections.size()) { return NULL; }
	ClientConnectionClass &conn = ClientConnections[connection_id];

	if (conn.Connection)
	{
		return conn.Connection->Get_Their_Name();
	}
	return NULL;
}

void LuaNetworkDebuggerClass::Get_Debugger_Connections(std::vector<int> &connections)
{
	connections.resize(0);
	for (int i = 0; i < (int)ClientConnections.size(); i++)
	{
		if (ClientConnections[i].AsServer == false && ClientConnections[i].ConnectState == CONNECT_IDLE)
		{
			connections.push_back(i);
		}
	}
}

void LuaNetworkDebuggerClass::Remove_Connection(int connection_id)
{
	// Note: once the connection goes away, this becomes false, and it's not an error in this case.
	// It just means we are trying to close a connection that just self-closed. -Eric_Y
	if (connection_id < 0 || connection_id >= (int)ClientConnections.size()) 
		return;

	ClientConnectionClass &conn = ClientConnections[connection_id];

	if (ClientConnections[connection_id].ConnectState != CONNECT_CLIENT_CONNECTING)
	{
		Send_Goodbye(conn);
	}
	else
	{
		ConnectionManagerClass *conmgr = NetworkClass::Get_Connection_Manager();
		ENFORCED_IF(conmgr)
		{
			ConnectionClass *cconn = conmgr->Get_Connection(&ClientConnections[connection_id].Address);
			if (cconn)
			{
				conmgr->Remove_Connection(cconn);
			}
		}
	}

	if (conn.Connection)
	{
		ConnectionManagerClass *conmgr = NetworkClass::Get_Connection_Manager();
		ENFORCED_IF(conmgr)
		{
			conmgr->Remove_Connection(conn.Connection);
		}
	}

	if (ActiveClientIndex == connection_id)
	{
		ActiveClientIndex = -1;
	}

	ClientConnections.erase(ClientConnections.begin()+connection_id);
	for (int i = 0; i < (int)ClientConnections.size(); i++)
	{
		ClientConnections[i].ID = i;
		if (ActiveClientIndex == -1 && 
			 ClientConnections[i].ConnectState == CONNECT_IDLE && 
			 ClientConnections[i].AsServer == false)
		{
			ActiveClientIndex = i;
		}
	}

	if (ActiveClientIndex == -1)
	{
		LuaScriptClass::Reset_Debugging();
	}
}

void LuaNetworkDebuggerClass::Stop_Connection_Attempt(void)
{
	for (int i = 0; i < (int)ClientConnections.size(); i++)
	{
		if (ClientConnections[i].ConnectState == CONNECT_CLIENT_CONNECTING)
		{
			Remove_Connection(i);
			Stop_Connection_Attempt();
			return;
		}
	}
}

bool LuaNetworkDebuggerClass::Connection_Unreachable(int connection_id)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return true;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	return conn.Connection->Is_Unreachable();
}

bool LuaNetworkDebuggerClass::Connection_Timed_Out(int connection_id)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return true;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	return conn.Connection->Is_Timed_Out();
}

bool LuaNetworkDebuggerClass::Connect_To_Local_Server(int network_port)
{
	if (NetworkClass::Get_Bound_Port() == network_port)
	{
		network_port++;
	}
	IPAddressClass localaddr(ntohl(inet_addr("127.0.0.1")), (unsigned short) network_port);

	return Connect_To_Server(localaddr);
}

bool LuaNetworkDebuggerClass::Connect_To_Server(const IPAddressClass &address)
{
	Debug_Print("LuaNetworkDebuggerClass::Connect_To_Server -- Connecting to %s!\n", address.As_String());

	for (int i = 0; i < (int)ClientConnections.size(); i++)
	{
		if (ClientConnections[i].Address == address)
		{
			Debug_Print("LuaNetworkDebuggerClass::Connect_To_Server -- Already Connected to %s!\n", address.As_String());
			return true;
		}
	}

	ConnectionManagerClass *conmgr = NetworkClass::Get_Connection_Manager();
	FAIL_IF(!conmgr) return false;

	conmgr->Connect(const_cast<IPAddressClass *>(&address));
	ClientConnections.resize(ClientConnections.size()+1);
	ClientConnections.back().Address = address;
	ClientConnections.back().AsServer = false;
	ClientConnections.back().ID = ClientConnections.size() - 1;
	ClientConnections.back().ConnectState = CONNECT_CLIENT_CONNECTING;
	return true;
}


void LuaNetworkDebuggerClass::Send_Heartbeat(ClientConnectionClass &conn)
{
	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_HEARTBEAT);

	conn.Connection->Send_Guaranteed(packet);
}


void LuaNetworkDebuggerClass::Send_Hello(ClientConnectionClass &conn)
{
	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_HELLO);

	conn.Connection->Send_Guaranteed(packet);
	conn.ConnectState = CONNECT_SENT_HELLO;
}

void LuaNetworkDebuggerClass::Send_Goodbye(ClientConnectionClass &conn)
{
	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_GOODBYE);

	conn.Connection->Send_Guaranteed(packet);
}

void LuaNetworkDebuggerClass::Send_Scripts_Request(int connection_id)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_SCRIPTS);

	conn.Connection->Send_Guaranteed(packet);
}


void LuaNetworkDebuggerClass::Process_Scripts_Request(ClientConnectionClass &conn, PacketClass *)
{
	const LuaScriptClass::ActiveScriptListType &active_list = LuaScriptClass::Get_Active_Script_List();
	LuaScriptClass::ActiveScriptListType::const_iterator it = active_list.begin();

	PacketClass *opacket = PacketAllocator.Allocate(256 + (128 * active_list.size()));
	FAIL_IF (!opacket) return;

	opacket->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(opacket);
	opacket->Put_Int(LUA_PACKET_RESPONSE_SCRIPTLIST);

	opacket->Put_Int(active_list.size());
	for ( ; it != active_list.end(); it++)
	{
		opacket->Put_Int(it->first);
		opacket->Put_String(it->second->Get_Full_Path_Name());
	}
	conn.Connection->Send_Guaranteed(opacket);
}


void LuaNetworkDebuggerClass::Process_Scripts_Response(ClientConnectionClass &conn, PacketClass *ipacket)
{
	int script_count = ipacket->Get_Int();
	Debug_Print("LuaNetworkDebuggerClass::Process_Scripts_Response -- Script Count: %d\n", script_count);

	LuaNetworkDebuggerCallbackClass::ScriptListType script_list;
	for (int i = 0; i < script_count; i++)
	{
		script_list.resize(script_list.size() + 1);
		script_list.back().first = ipacket->Get_Int();
		ipacket->Get_String(script_list.back().second);
		Debug_Print("LuaNetworkDebuggerClass::Process_Scripts_Response -- Script: %d, Name: %s\n", script_list.back().first, script_list.back().second.c_str());
	}

	if (Callbacks)
	{
		Callbacks->Scripts_Response(conn.ID, script_list);
	}
}


void LuaNetworkDebuggerClass::Send_Threads_Request(int connection_id, int script_id)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_THREADS);
	packet->Put_Int(script_id);

	conn.Connection->Send_Guaranteed(packet);
}


void LuaNetworkDebuggerClass::Process_Threads_Request(ClientConnectionClass &conn, PacketClass *ipacket)
{
	int script_id = ipacket->Get_Int();
	Debug_Print("LuaNetworkDebuggerClass::Process_Threads_Request -- Script ID: %d\n", script_id);

	const LuaScriptClass::ActiveScriptListType &active_list = LuaScriptClass::Get_Active_Script_List();
	LuaScriptClass::ActiveScriptListType::const_iterator it = active_list.find(script_id);

	PacketClass *opacket = PacketAllocator.Allocate(256 + (64 * it->second->Get_Thread_Count()));
	FAIL_IF (!opacket) return;

	opacket->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(opacket);
	opacket->Put_Int(LUA_PACKET_RESPONSE_THREADLIST);

	int tcount = 0;
	if (it != active_list.end() && it->second)
	{
		tcount = it->second->Get_Active_Thread_Count();
	}

	opacket->Put_Int(script_id);
	opacket->Put_Int(tcount);
	for (int i = 0; i < it->second->Get_Thread_Count(); i++)
	{
		if (it->second->Get_Thread_Name(i)->size())
		{
			opacket->Put_Int(i);
			opacket->Put_String(*it->second->Get_Thread_Name(i));
		}
	}
	conn.Connection->Send_Guaranteed(opacket);
}


void LuaNetworkDebuggerClass::Process_Threads_Response(ClientConnectionClass &conn, PacketClass *ipacket)
{
	int script_id = ipacket->Get_Int();
	int thread_count = ipacket->Get_Int();
	Debug_Print("LuaNetworkDebuggerClass::Process_Threads_Response -- Script: %d, Thread Count: %d\n", script_id, thread_count);

	LuaNetworkDebuggerCallbackClass::ThreadListType thread_list;
	for (int i = 0; i < thread_count; i++)
	{
		thread_list.resize(thread_list.size() + 1);
		thread_list.back().first = ipacket->Get_Int();
		ipacket->Get_String(thread_list.back().second);
		Debug_Print("LuaNetworkDebuggerClass::Process_Threads_Response -- Thread: %d, Name: %s\n", thread_list.back().first, thread_list.back().second.c_str());
	}

	if (Callbacks)
	{
		Callbacks->Threads_Response(conn.ID, thread_list);
	}
}

void LuaNetworkDebuggerClass::Send_Debug_Break_All_Request(int connection_id)
{
	Send_Debugger_Control_Request(connection_id, LUA_PACKET_BREAK_ALL);
}

void LuaNetworkDebuggerClass::Send_Debug_Step_Into_Request(int connection_id)
{
	Send_Debugger_Control_Request(connection_id, LUA_PACKET_STEP_IN_TO);
}

void LuaNetworkDebuggerClass::Send_Debug_Step_Out_Request(int connection_id)
{
	Send_Debugger_Control_Request(connection_id, LUA_PACKET_STEP_OUT);
}

void LuaNetworkDebuggerClass::Send_Debug_Step_Over_Request(int connection_id)
{
	Send_Debugger_Control_Request(connection_id, LUA_PACKET_STEP_OVER);
}

void LuaNetworkDebuggerClass::Send_Debug_Continue_Request(int connection_id)
{
	Send_Debugger_Control_Request(connection_id, LUA_PACKET_CONTINUE);
}


void LuaNetworkDebuggerClass::Send_Debugger_Control_Request(int connection_id, LuaDebuggerPacketEnum ptype)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(ptype);

	conn.Connection->Send_Guaranteed(packet);
}

void LuaNetworkDebuggerClass::Send_Add_Breakpoint_Request(int connection_id, int script_id, int thread_id, const char *filename, int line, const char *condition)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_ADD_BREAKPOINT);
	packet->Put_Int(script_id);
	packet->Put_Int(thread_id);
	std::string tstr;
	if (filename) tstr = filename;
	packet->Put_String(tstr);
	packet->Put_Int(line);
	tstr.empty();
	if (condition) tstr = condition;
	packet->Put_String(tstr);

	conn.Connection->Send_Guaranteed(packet);
}


void LuaNetworkDebuggerClass::Process_Add_Breakpoint_Request(ClientConnectionClass &conn, PacketClass *ipacket)
{
	conn;
	int script_id = ipacket->Get_Int();
	int thread_id = ipacket->Get_Int();
	std::string filename;
	ipacket->Get_String(filename);
	int line = ipacket->Get_Int();
	std::string condition;
	ipacket->Get_String(condition);
	Debug_Print("LuaNetworkDebuggerClass::Process_Add_Breakpoint_Request -- Script ID: %d, Thread ID: %d, Name: %s, Line: %d, Condition: %s\n", 
					script_id, thread_id, filename.c_str(), line, condition.c_str());

	const LuaScriptClass::ActiveScriptListType &active_list = LuaScriptClass::Get_Active_Script_List();
	LuaScriptClass::ActiveScriptListType::const_iterator it = active_list.find(script_id);
	LuaScriptClass *script = it != active_list.end() ? it->second : NULL;

	LuaScriptClass::Debug_Add_Breakpoint(script, thread_id, filename, line, condition);
}

void LuaNetworkDebuggerClass::Send_Remove_Breakpoint_Request(int connection_id, int script_id, int thread_id, const char *filename, int line)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_REMOVE_BREAKPOINT);
	packet->Put_Int(script_id);
	packet->Put_Int(thread_id);
	std::string tstr;
	if (filename) tstr = filename;
	packet->Put_String(tstr);
	packet->Put_Int(line);

	conn.Connection->Send_Guaranteed(packet);
}


void LuaNetworkDebuggerClass::Process_Remove_Breakpoint_Request(ClientConnectionClass &conn, PacketClass *ipacket)
{
	conn;
	int script_id = ipacket->Get_Int();
	int thread_id = ipacket->Get_Int();
	std::string filename;
	ipacket->Get_String(filename);
	int line = ipacket->Get_Int();
	std::string condition;
	ipacket->Get_String(condition);
	Debug_Print("LuaNetworkDebuggerClass::Process_Remove_Breakpoint_Request -- Script ID: %d, Thread ID: %d, Name: %s, Line: %d\n", 
					script_id, thread_id, filename.c_str(), line);

	const LuaScriptClass::ActiveScriptListType &active_list = LuaScriptClass::Get_Active_Script_List();
	LuaScriptClass::ActiveScriptListType::const_iterator it = active_list.find(script_id);
	LuaScriptClass *script = it != active_list.end() ? it->second : NULL;

	LuaScriptClass::Debug_Remove_Breakpoint(script, thread_id, filename, line);
}

void LuaNetworkDebuggerClass::Send_Select_Thread_Request(int connection_id, int thread_id)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_SELECT_THREAD);
	packet->Put_Int(thread_id);

	conn.Connection->Send_Guaranteed(packet);
}


void LuaNetworkDebuggerClass::Process_Select_Thread_Request(ClientConnectionClass &conn, PacketClass *ipacket)
{
	conn;
	int thread_id = ipacket->Get_Int();
	Debug_Print("LuaNetworkDebuggerClass::Process_Select_Thread_Request -- Thread ID: %d\n", thread_id);

	LuaScriptClass::Debug_Break_Thread(thread_id);
}

void LuaNetworkDebuggerClass::Send_Select_Script_Request(int connection_id, int script_id)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_SELECT_SCRIPT);
	packet->Put_Int(script_id);

	conn.Connection->Send_Guaranteed(packet);
}


void LuaNetworkDebuggerClass::Process_Select_Script_Request(ClientConnectionClass &conn, PacketClass *ipacket)
{
	conn;
	int script_id = ipacket->Get_Int();
	Debug_Print("LuaNetworkDebuggerClass::Process_Select_Script_Request -- Script ID: %d\n", script_id);

	const LuaScriptClass::ActiveScriptListType &active_list = LuaScriptClass::Get_Active_Script_List();
	LuaScriptClass::ActiveScriptListType::const_iterator it = active_list.find(script_id);

	if (it != active_list.end() && it->second)
	{
		LuaScriptClass::Debug_Set_Script_Context(it->second);
	}
}

void LuaNetworkDebuggerClass::Send_Set_Callstack_Depth_Request(int connection_id, int script_id, int level)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_SET_CALLSTACK_DEPTH);
	packet->Put_Int(script_id);
	packet->Put_Int(level);

	conn.Connection->Send_Guaranteed(packet);
}


void LuaNetworkDebuggerClass::Process_Set_Callstack_Depth_Request(ClientConnectionClass &conn, PacketClass *ipacket)
{
	conn;
	int script_id = ipacket->Get_Int();
	int level = ipacket->Get_Int();
	Debug_Print("LuaNetworkDebuggerClass::Process_Set_Callstack_Depth_Request -- Script ID: %d, Level: %d\n", script_id, level);

	const LuaScriptClass::ActiveScriptListType &active_list = LuaScriptClass::Get_Active_Script_List();
	LuaScriptClass::ActiveScriptListType::const_iterator it = active_list.find(script_id);

	if (it != active_list.end() && it->second)
	{
		it->second->Debug_Set_Current_Callstack_Depth(level);
	}
}

void LuaNetworkDebuggerClass::Send_Attach_Request(int connection_id, int script_id)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_ATTACH);
	packet->Put_Int(script_id);

	conn.Connection->Send_Guaranteed(packet);
}


void LuaNetworkDebuggerClass::Process_Attach_Request(ClientConnectionClass &conn, PacketClass *ipacket)
{
	conn;
	int script_id = ipacket->Get_Int();
	Debug_Print("LuaNetworkDebuggerClass::Process_Attach_Request -- Script ID: %d\n", script_id);

	const LuaScriptClass::ActiveScriptListType &active_list = LuaScriptClass::Get_Active_Script_List();
	LuaScriptClass::ActiveScriptListType::const_iterator it = active_list.find(script_id);

	if (it != active_list.end() && it->second)
	{
		it->second->Debug_Attach(false);
	}

	Send_Child_Script_List_Response(conn, it->second);
}

void LuaNetworkDebuggerClass::Send_Detach_Request(int connection_id, int script_id)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_ATTACH);
	packet->Put_Int(script_id);

	conn.Connection->Send_Guaranteed(packet);
}


void LuaNetworkDebuggerClass::Process_Detach_Request(ClientConnectionClass &conn, PacketClass *ipacket)
{
	conn;
	int script_id = ipacket->Get_Int();
	Debug_Print("LuaNetworkDebuggerClass::Process_Detach_Request -- Script ID: %d\n", script_id);

	const LuaScriptClass::ActiveScriptListType &active_list = LuaScriptClass::Get_Active_Script_List();
	LuaScriptClass::ActiveScriptListType::const_iterator it = active_list.find(script_id);

	if (it != active_list.end() && it->second)
	{
		it->second->Debug_Detach();
	}
}

void LuaNetworkDebuggerClass::Send_Script_Added_Response(LuaScriptClass *script)
{
	if (ActiveClientIndex >= (int)ClientConnections.size() || ActiveClientIndex < 0) 
	{
		return;
	}
	ClientConnectionClass &conn = ClientConnections[ActiveClientIndex];
	FAIL_IF(conn.AsServer == false) return;

	PacketClass *opacket = PacketAllocator.Allocate();
	FAIL_IF (!opacket) return;

	opacket->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(opacket);
	opacket->Put_Int(LUA_PACKET_RESPONSE_SCRIPT_ADDED);
	opacket->Put_Int(script->Get_Script_ID());
	opacket->Put_String(script->Get_Full_Path_Name());

	conn.Connection->Send_Guaranteed(opacket);
}

void LuaNetworkDebuggerClass::Process_Script_Added_Response(ClientConnectionClass &conn, PacketClass *ipacket)
{
	int script_id = ipacket->Get_Int();
	std::string filename;
	ipacket->Get_String(filename);

	Debug_Print("LuaNetworkDebuggerClass::Process_Script_Added_Response -- Script: %d, File: %s\n", script_id, filename.c_str());
	if (Callbacks)
	{
		Callbacks->Script_Added_Response(conn.ID, script_id, filename);
	}
}

void LuaNetworkDebuggerClass::Send_Script_Removed_Response(LuaScriptClass *script)
{
	if (ActiveClientIndex >= (int)ClientConnections.size() || ActiveClientIndex < 0) 
	{
		return;
	}
	ClientConnectionClass &conn = ClientConnections[ActiveClientIndex];
	FAIL_IF(conn.AsServer == false) return;

	PacketClass *opacket = PacketAllocator.Allocate();
	FAIL_IF (!opacket) return;

	opacket->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(opacket);
	opacket->Put_Int(LUA_PACKET_RESPONSE_SCRIPT_REMOVED);
	opacket->Put_Int(script->Get_Script_ID());

	conn.Connection->Send_Guaranteed(opacket);
}

void LuaNetworkDebuggerClass::Process_Script_Removed_Response(ClientConnectionClass &conn, PacketClass *ipacket)
{
	int script_id = ipacket->Get_Int();

	Debug_Print("LuaNetworkDebuggerClass::Process_Script_Removed_Response -- Script: %d\n", script_id);
	if (Callbacks)
	{
		Callbacks->Script_Removed_Response(conn.ID, script_id);
	}
}

void LuaNetworkDebuggerClass::Send_Script_Suspended_Response(LuaScriptClass *script)
{
	if (ActiveClientIndex >= (int)ClientConnections.size() || ActiveClientIndex < 0) 
	{
		std::string outstr;
		String_Printf(outstr, "LuaScript: %s is currently suspended with no debugger attached.\n"
						  "Please attach a lua debugger to resume process execution.\n\n"
						  "*NOTE* You must dismiss this dialog before connecting with the lua debugger. *NOTE*\n", 
						  script->Get_Name().c_str());
		Message_Popup(outstr.c_str());
		return;
	}
	ClientConnectionClass &conn = ClientConnections[ActiveClientIndex];
	FAIL_IF(conn.AsServer == false) return;

	std::vector<std::string> call_stack;
	script->Debug_Get_Callstack(call_stack);
	PacketClass *opacket = PacketAllocator.Allocate(256 + (128 * (call_stack.size() + script->Get_Thread_Count())));
	FAIL_IF (!opacket) return;

	opacket->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(opacket);
	opacket->Put_Int(LUA_PACKET_RESPONSE_SCRIPT_SUSPENDED);
	opacket->Put_Int(script->Get_Script_ID());
	opacket->Put_Int(script->Get_Current_Thread_Id());
	opacket->Put_String(script->Get_Full_Path_Name());

	opacket->Put_Int((int)call_stack.size());
	int i;
	for (i = 0; i < (int)call_stack.size(); i++)
	{
		opacket->Put_String(call_stack[i]);
	}

	opacket->Put_Int(script->Get_Active_Thread_Count());
	for (i = 0; i < script->Get_Thread_Count(); i++)
	{
		if (script->Get_Thread_Name(i)->size())
		{
			opacket->Put_Int(i);
			opacket->Put_String(*script->Get_Thread_Name(i));
		}
	}
	conn.Connection->Send_Guaranteed(opacket);
}


void LuaNetworkDebuggerClass::Process_Script_Suspended_Response(ClientConnectionClass &conn, PacketClass *ipacket)
{
	int script_id = ipacket->Get_Int();
	int thread_id = ipacket->Get_Int();
	std::string filename;
	ipacket->Get_String(filename);

	Debug_Print("LuaNetworkDebuggerClass::Process_Script_Suspended_Response -- Script: %d, Thread: %d, File: %s\n", script_id, thread_id, filename.c_str());
	std::vector<std::string> call_stack;
	int call_stack_size = ipacket->Get_Int();
	call_stack.resize(call_stack_size);
	for (int i = 0; i < call_stack_size; i++)
	{
		ipacket->Get_String(call_stack[i]);
		Debug_Print("LuaNetworkDebuggerClass::Process_Script_Suspended_Response -- Callstack %2.2d: %s\n", i, call_stack[i].c_str());
	}

	int thread_count = ipacket->Get_Int();
	LuaNetworkDebuggerCallbackClass::ThreadListType thread_list;
	for (int i = 0; i < thread_count; i++)
	{
		thread_list.resize(thread_list.size() + 1);
		thread_list.back().first = ipacket->Get_Int();
		ipacket->Get_String(thread_list.back().second);
		Debug_Print("LuaNetworkDebuggerClass::Process_Script_Suspended_Response -- Thread: %d, Name: %s\n", thread_list.back().first, thread_list.back().second.c_str());
	}

	if (Callbacks)
	{
		Callbacks->Script_Suspended_Response(conn.ID, script_id, thread_id, filename, call_stack, thread_list);
	}
}


void LuaNetworkDebuggerClass::Process_Child_Script_List_Response(ClientConnectionClass &conn, PacketClass *ipacket)
{
	int script_id = ipacket->Get_Int();
	int scount = ipacket->Get_Int();

	Debug_Print("LuaNetworkDebuggerClass::Process_Child_Script_List_Response -- Script: %d, Count: %d\n", script_id, scount);

	std::vector<std::string> slist;
	for (int i = 0; i < scount; i++)
	{
		std::string sval;
		ipacket->Get_String(sval);
		Debug_Print("LuaNetworkDebuggerClass::Process_Child_Script_List_Response -- Child Script: %s\n", sval.c_str());
		slist.push_back(sval);
	}

	if (Callbacks)
	{
		Callbacks->Child_Script_List_Response(conn.ID, script_id, slist);
	}
}

void LuaNetworkDebuggerClass::Send_Child_Script_List_Response(ClientConnectionClass &conn, LuaScriptClass *script)
{
	std::vector<std::string> slist;
	script->Debug_Get_Loaded_Child_Scripts(slist);

	PacketClass *opacket = PacketAllocator.Allocate(256 + (128 * slist.size()));
	FAIL_IF (!opacket) return;

	opacket->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(opacket);
	opacket->Put_Int(LUA_PACKET_RESPONSE_CHILD_SCRIPT_LIST);
	opacket->Put_Int(script->Get_Script_ID());

	opacket->Put_Int((int)slist.size());
	for (int i = 0; i < (int)slist.size(); i++)
	{
		opacket->Put_String(slist[i]);
	}

	conn.Connection->Send_Guaranteed(opacket);
}

// Dump var command.  Takes 1 string parameter.
// if string parameter matches a local var symbol name then return that value.
// if not then try a map_global_from_lua.
// result is <name>, <type>, <value>

void LuaNetworkDebuggerClass::Send_Dump_Var_Request(int connection_id, int script_id, const std::string &name)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_DUMP_VAR);
	packet->Put_Int(script_id);
	packet->Put_String(name);

	conn.Connection->Send_Guaranteed(packet);
}

void LuaNetworkDebuggerClass::Process_Dump_Var_Request(ClientConnectionClass &conn, PacketClass *ipacket)
{
	PacketClass *opacket = PacketAllocator.Allocate();
	FAIL_IF (!opacket) return;

	opacket->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(opacket);
	opacket->Put_Int(LUA_PACKET_RESPONSE_DUMP_VAR);

	int script_id = ipacket->Get_Int();
	std::string name;
	ipacket->Get_String(name);
	Debug_Print("LuaNetworkDebuggerClass::Process_Dump_Var_Request -- Script ID: %d, Name: %s\n", script_id, name.c_str());

	const LuaScriptClass::ActiveScriptListType &active_list = LuaScriptClass::Get_Active_Script_List();
	LuaScriptClass::ActiveScriptListType::const_iterator it = active_list.find(script_id);

	int ltype = -1;
	std::string result;
	if (it != active_list.end() && it->second)
	{
		it->second->Debug_Get_Var_From_Lua(name, ltype, result);
	}
	opacket->Put_Int(script_id);
	opacket->Put_String(name);
	opacket->Put_Int(ltype);
	opacket->Put_String(result);

	conn.Connection->Send_Guaranteed(opacket);
}

void LuaNetworkDebuggerClass::Process_Dump_Var_Response(ClientConnectionClass &conn, PacketClass *ipacket)
{
	int script_id = ipacket->Get_Int();
	std::string name, result;
	ipacket->Get_String(name);
	int ltype = ipacket->Get_Int();
	ipacket->Get_String(result);

	Debug_Print("LuaNetworkDebuggerClass::Process_Dump_Var_Response -- Script: %d, Name: %s, Type: %d, Result: %s\n", 
					script_id, name.c_str(), ltype, result.c_str());

	if (Callbacks)
	{
		Callbacks->Dump_Var_Response(conn.ID, script_id, name, ltype, result);
	}
}

// OLD		// Dump table command. Takes 2 strings.  <tablename> <num_sub_tables> <subtablename_1> <subtablename_2> <...>
// if tablename parameter matches a local var symbol name then return the members of that table.
// if not then try a map_global_from_lua.
// result is <tablename> <num_sub_tables> <subtablename_1> <subtablename_2> <...> 
//           <num_members> <name_1> <type_1> <value_1> 
//                         <name_2> <type_2> <value_2>
//                         <...>

// NEW: Ask the game for the values in a LuaTable var. This this sends the root table name, and then a path of table indexes to follow.
// The table_id is an identifier used by the debugger to identify this table dump request. It needs to be sent back in the response.
void LuaNetworkDebuggerClass::Send_Dump_Table_Request(int connection_id, int script_id, const std::string &name, std::vector<int> &decension, int table_id)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_DUMP_TABLE);
	packet->Put_Int(script_id);
	packet->Put_Int(table_id);
	packet->Put_String(name);
	packet->Put_Int((int) decension.size());
	for(int i=0; i<(int) decension.size(); i++)
		packet->Put_Int(decension[i]);

	conn.Connection->Send_Guaranteed(packet);
}

void LuaNetworkDebuggerClass::Process_Dump_Table_Request(ClientConnectionClass &conn, PacketClass *ipacket)
{
	// Read input packet
	int script_id = ipacket->Get_Int();
	int table_id = ipacket->Get_Int();
	std::string name;
	ipacket->Get_String(name);
	int decension_size = ipacket->Get_Int();
	std::vector<int> decension;
	for(int i=0; i<decension_size; i++)
		decension.push_back(ipacket->Get_Int());

	Debug_Print("LuaNetworkDebuggerClass::Process_Dump_Table_Request -- Script ID: %d, Name: %s, Decension Size %d\n", script_id, name.c_str(),decension_size);

	const LuaScriptClass::ActiveScriptListType &active_list = LuaScriptClass::Get_Active_Script_List();
	LuaScriptClass::ActiveScriptListType::const_iterator it = active_list.find(script_id);

	LuaTableMemberList table_members;

	if (it != active_list.end() && it->second)
		it->second->Debug_Get_Table_From_Lua(name,decension,table_members);

	// Construct output packet
	PacketClass *opacket = PacketAllocator.Allocate(256 + (256 * table_members.size()));
	FAIL_IF (!opacket) return;

	opacket->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(opacket);
	opacket->Put_Int(LUA_PACKET_RESPONSE_DUMP_TABLE);
	opacket->Put_Int(table_id); // identify this dump
	opacket->Put_Int((int) table_members.size());
	for(int i=0; i<(int) table_members.size(); i++)
	{
		opacket->Put_Int(table_members[i].key_type);
		opacket->Put_String(table_members[i].key_value);
		opacket->Put_Int(table_members[i].value_type);
		opacket->Put_String(table_members[i].value_string);
	}

	conn.Connection->Send_Guaranteed(opacket);
}

void LuaNetworkDebuggerClass::Process_Dump_Table_Response(ClientConnectionClass &conn, PacketClass *ipacket)
{
	int table_id = ipacket->Get_Int();
	int member_count = ipacket->Get_Int();
	
	LuaTableMemberList table_members;
	for(int i=0; i<member_count; i++)
	{
		table_members.resize(table_members.size()+1);

		table_members.back().key_type = ipacket->Get_Int();
		ipacket->Get_String(table_members.back().key_value);

		table_members.back().value_type = ipacket->Get_Int();
		ipacket->Get_String(table_members.back().value_string);
	}

	Debug_Print("LuaNetworkDebuggerClass::Process_Dump_Table_Response -- ID: %d, Members: %d\n", 
		table_id,member_count);

	if (Callbacks)
		Callbacks->Dump_Table_Response(conn.ID, table_id, table_members);
}


// This method is used by the application to send debug output back to the debugger (to display in an output window)
void LuaNetworkDebuggerClass::Send_Output_To_Debugger(const std::string &message,int destination)
{
	if (!Get_Instance() || !Get_Instance()->Is_Initialized())
		return;

	if (Get_Instance()->ActiveClientIndex >= (int)Get_Instance()->ClientConnections.size() || Get_Instance()->ActiveClientIndex < 0) 
		return;

	Debug_Print("LuaNetworkDebuggerClass::Send_Output_To_Debugger -- Message: %s\n", message.c_str());

	ClientConnectionClass &conn = Get_Instance()->ClientConnections[Get_Instance()->ActiveClientIndex];
	FAIL_IF(conn.AsServer == false) return;

	PacketClass *opacket = PacketAllocator.Allocate();
	FAIL_IF (!opacket) return;

	opacket->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(opacket);
	opacket->Put_Int(LUA_PACKET_OUTPUT_TO_DEBUGGER);
	opacket->Put_Int(destination);
	opacket->Put_String(message);

	// Send packed to all valid clients
	conn.Connection->Send_Guaranteed(opacket);
}

// The debugger uses this method to receive debug output from the application being debugged.
void LuaNetworkDebuggerClass::Process_Output_To_Debugger(ClientConnectionClass &, PacketClass *ipacket)
{
	std::string message;
	int destination = ipacket->Get_Int();
	ipacket->Get_String(message);

	Debug_Print("LuaNetworkDebuggerClass::Process_Output_To_Debugger -- Message: %s\n", message.c_str());

	if (Callbacks)
	{
		Callbacks->Print_Output_To_Debugger(message,destination);
		return;
	}
}

// Ask the Lua Interpreter (on the game side) to execute the text in the context of script_id
void LuaNetworkDebuggerClass::Send_Execute_Text_Request(int connection_id, int script_id, const char* text)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	PacketClass *packet = PacketAllocator.Allocate();
	FAIL_IF (!packet) return;

	packet->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(packet);
	packet->Put_Int(LUA_PACKET_EXECUTE_TEXT);
	packet->Put_Int(script_id);
	packet->Put_String(text);

	conn.Connection->Send_Guaranteed(packet);
}


void LuaNetworkDebuggerClass::Send_Confirm_Connection(int connection_id)
{
	FAIL_IF(connection_id >= (int)ClientConnections.size() || connection_id < 0) return;
	ClientConnectionClass &conn = ClientConnections[connection_id];
	FAIL_IF(conn.AsServer) return;

	Send_Heartbeat(conn);
}


void LuaNetworkDebuggerClass::Process_Execute_Text_Request(ClientConnectionClass &conn, PacketClass *ipacket)
{
	int script_id = ipacket->Get_Int(); // Get Script_id
	std::string text;
	ipacket->Get_String(text); // Get the text we want the Lua interpreter to execute
	Debug_Print("LuaNetworkDebuggerClass::Process_Execute_Text_Request -- Script ID: %d, %s\n", script_id, text.c_str());

	// Find the script
	const LuaScriptClass::ActiveScriptListType &active_list = LuaScriptClass::Get_Active_Script_List();
	LuaScriptClass::ActiveScriptListType::const_iterator it = active_list.find(script_id);

	if (it == active_list.end() || !it->second)
	{
		Debug_Print("LuaNetworkDebuggerClass::Process_Execute_Text_Request -- Couldn't find script! Script ID: %d, %s\n", script_id, text.c_str());
		return;
	}

	// Execute it
	LuaScriptClass::Debug_Pre_Execute_String();
	std::string result;
	if (false == it->second->Execute_String(text,result))
	{
		if (result.size() == 0)
		{
			// more input expected
			result = "\r\n  > ";
		}
		else
		{
			// result = error message
			result.insert(0,"\r\n");
			result.append("\r\n> ");
		}
	}
	else
	{
		// Command executed successfully
		result.append("\r\n> ");
	}
	LuaScriptClass::Debug_Post_Execute_String();

	// Send back the results
	PacketClass *opacket = PacketAllocator.Allocate();
	FAIL_IF (!opacket) return;

	opacket->Clear_For_Write();
	LuaDebuggerPacketType.Add_Type_Header(opacket);
	opacket->Put_Int(LUA_PACKET_RESPONSE_EXECUTE_TEXT);

	opacket->Put_Int(script_id);
	opacket->Put_String(result);

	conn.Connection->Send_Guaranteed(opacket);
}


void LuaNetworkDebuggerClass::Process_Execute_Text_Response(ClientConnectionClass &conn, PacketClass *ipacket)
{
	int script_id = ipacket->Get_Int();
	std::string result;
	ipacket->Get_String(result);

	Debug_Print("LuaNetworkDebuggerClass::Process_Execute_Text_Response\n");

	if (Callbacks)
	{
		Callbacks->Execute_Text_Response(conn.ID, script_id, result);
	}
}



void LuaNetworkDebuggerClass::Parse_Packet(ClientConnectionClass &conn, PacketClass *packet)
{
	LuaDebuggerPacketEnum packet_type = (LuaDebuggerPacketEnum)packet->Get_Int();

	switch (packet_type)
	{
		case LUA_PACKET_HELLO:
			if (conn.AsServer)
			{
				Send_Hello(conn);
			}
			conn.ConnectState = CONNECT_IDLE;
			if (conn.AsServer)
			{
				LuaScriptClass::Debug_On_Debugger_Connect();
			}
			if (conn.AsServer == false && Callbacks)
			{
				Callbacks->Server_Connected_Response(conn.ID);
			}
			break;
		case LUA_PACKET_GOODBYE:
			Remove_Connection(conn.ID);
			break;
		case LUA_PACKET_SCRIPTS:
			Process_Scripts_Request(conn, packet);
			break;
		case LUA_PACKET_THREADS:
			Process_Threads_Request(conn, packet);
			break;
		case LUA_PACKET_BREAK_ALL:
			LuaScriptClass::Debug_Break_All();
			break;
		case LUA_PACKET_STEP_OVER:
			LuaScriptClass::Debug_Step_Over();
			break;
		case LUA_PACKET_CONTINUE:
			LuaScriptClass::Debug_Continue();
			break;
		case LUA_PACKET_STEP_IN_TO:
			LuaScriptClass::Debug_Step_In_To();
			break;
		case LUA_PACKET_STEP_OUT:
			LuaScriptClass::Debug_Step_Out();
			break;
		case LUA_PACKET_ATTACH:
			Process_Attach_Request(conn, packet);
			break;
		case LUA_PACKET_SET_CALLSTACK_DEPTH:
			Process_Set_Callstack_Depth_Request(conn, packet);
			break;
		case LUA_PACKET_CONSOLE_COMMAND:
			break;
		case LUA_PACKET_PARSE:
			break;
		case LUA_PACKET_HEARTBEAT:
			break;
		case LUA_PACKET_ADD_BREAKPOINT:
			Process_Add_Breakpoint_Request(conn, packet);
			break;
		case LUA_PACKET_REMOVE_BREAKPOINT:
			Process_Remove_Breakpoint_Request(conn, packet);
			break;
		case LUA_PACKET_DUMPLOCALS:
			break;
		case LUA_PACKET_DUMP_VAR:
			Process_Dump_Var_Request(conn, packet);
			break;
		case LUA_PACKET_DUMP_TABLE:
			Process_Dump_Table_Request(conn, packet);
			break;
		case LUA_PACKET_RESPONSE_CODE:
			break;
		case LUA_PACKET_RESPONSE_STRING:
			break;
		case LUA_PACKET_RESPONSE_SCRIPTLIST:
			Process_Scripts_Response(conn, packet);
			break;
		case LUA_PACKET_RESPONSE_THREADLIST:
			Process_Threads_Response(conn, packet);
			break;
		case LUA_PACKET_RESPONSE_BREAKPOINT_HIT:
			break;
		case LUA_PACKET_RESPONSE_SCRIPT_ADDED:
			Process_Script_Added_Response(conn, packet);
			break;
		case LUA_PACKET_RESPONSE_SCRIPT_REMOVED:
			Process_Script_Removed_Response(conn, packet);
			break;
		case LUA_PACKET_RESPONSE_SCRIPT_SUSPENDED:
			Process_Script_Suspended_Response(conn, packet);
			break;
		case LUA_PACKET_RESPONSE_DUMP_VAR:
			Process_Dump_Var_Response(conn, packet);
			break;
		case LUA_PACKET_RESPONSE_DUMP_TABLE:
			Process_Dump_Table_Response(conn, packet);
			break;
		case LUA_PACKET_RESPONSE_CHILD_SCRIPT_LIST:
			Process_Child_Script_List_Response(conn, packet);
			break;
		case LUA_PACKET_SELECT_SCRIPT:
			Process_Select_Script_Request(conn, packet);
			break;
		case LUA_PACKET_SELECT_THREAD:
			Process_Select_Thread_Request(conn, packet);
			break;
		case LUA_PACKET_OUTPUT_TO_DEBUGGER:
			Process_Output_To_Debugger(conn, packet);
			break;
		case LUA_PACKET_EXECUTE_TEXT:
			Process_Execute_Text_Request(conn, packet);
			break;
		case LUA_PACKET_RESPONSE_EXECUTE_TEXT:
			Process_Execute_Text_Response(conn, packet);
			break;

		default:
			// Unknown packet type
			assert(false);
			break;
	}
}

bool LuaNetworkDebuggerClass::Is_Being_Debugged(void)
{
	if (Get_Instance() == NULL)
	{
		return false;
	}
	if (!Get_Instance()->IsInitialized) return false;

	for (int i = 0; i < (int)Get_Instance()->ClientConnections.size(); )
	{
		// Search for dead connections.
		if (Get_Instance()->ClientConnections[i].Connection && Get_Instance()->ClientConnections[i].AsServer)
		{
			return true;
		}
	}
	return false;
}


/**
 * Service the debugger.
 * @since 3/31/2005 5:51:28 PM -- BMH
 */
void LuaNetworkDebuggerClass::Service(void)
{
	int i;
	if (!IsInitialized) return;

	ConnectionManagerClass *conmgr = NetworkClass::Get_Connection_Manager();
	FAIL_IF(!conmgr) return;

	NetworkClass::Service();

	// Service any outgoing connects.
	for (i = 0; i < (int)ClientConnections.size(); i++)
	{
		if (ClientConnections[i].ConnectState == CONNECT_CLIENT_CONNECTING && conmgr->Is_Connection_Established(&ClientConnections[i].Address))
		{
			ClientConnections[i].Connection = conmgr->Get_Connection(&ClientConnections[i].Address);
			assert(ClientConnections[i].Connection);
			Send_Hello(ClientConnections[i]);
		}
	}

	// search for new incoming server connections.
	for (i = 0; i < conmgr->Get_Num_Connections(); i++)
	{
		ConnectionClass *conn = conmgr->Get_Connection_By_Index(i);

		// only interested in listening connections.
		if (conn->Was_Listen_Connection() == false) continue;
		int sidx;
		for (sidx = 0; sidx < (int)ClientConnections.size(); sidx++)
		{
			if (ClientConnections[sidx].Connection == conn) break;
		}
		if (sidx == (int)ClientConnections.size())
		{
			// Not in our list.  See if it has any packets that match the lua debugger.
			PacketClass *packet = conn->Get_Guaranteed(&LuaDebuggerPacketType);
			if (packet)
			{
				// add the new connection.
				ClientConnections.resize(ClientConnections.size()+1);
				ClientConnections.back().Connection = conn;
				ClientConnections.back().Address = *conn->Get_Address();
				ClientConnections.back().AsServer = true;
				ClientConnections.back().ID = ClientConnections.size() - 1;
				ClientConnections.back().ConnectState = CONNECT_SERVER_CONNECTING;
				ActiveClientIndex = ClientConnections.back().ID;
				FAIL_IF (LuaDebuggerPacketType.Remove_Type_Header(packet) == false) continue;
				Parse_Packet(ClientConnections.back(), packet);
				PacketAllocator.Free(packet);
			}
		}
	}

	static DWORD last_heartbeat = TIMEGETTIME();
   
	// Process packets from existing connections.
	for (i = 0; i < (int)ClientConnections.size(); )
	{
		// Search for dead connections.
		if (ClientConnections[i].Connection && 
			 ClientConnections[i].AsServer &&
//			 ClientConnections[i].ConnectState == CONNECT_IDLE &&
			 (ClientConnections[i].Connection->Is_Timed_Out() || ClientConnections[i].Connection->Is_Unreachable()))
		{
			Remove_Connection(i);
			continue;
		}

		PacketClass *packet = NULL;
		if (ClientConnections[i].Connection)
		{
			packet = ClientConnections[i].Connection->Get_Guaranteed(&LuaDebuggerPacketType);
		}
		if (!packet) 
		{
			// no more packets on this connection, so advance to the next connection.
			i++;
			continue;
		}

		FAIL_IF (LuaDebuggerPacketType.Remove_Type_Header(packet) == false) continue;
		Parse_Packet(ClientConnections[i], packet);
		PacketAllocator.Free(packet);
	}

	if (TIMEGETTIME() - last_heartbeat > 5000)
	{
		for (i = 0; i < (int)ClientConnections.size(); i++)
		{
			// Search for dead connections.
			if (ClientConnections[i].Connection && 
				 ClientConnections[i].AsServer &&
				 ClientConnections[i].ConnectState == CONNECT_IDLE)
			{
				Send_Heartbeat(ClientConnections[i]);
				continue;
			}
		}
		last_heartbeat = TIMEGETTIME();
	}
}

