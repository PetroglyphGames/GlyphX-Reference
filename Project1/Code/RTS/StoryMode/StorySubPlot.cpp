

#pragma hdrstop
#include "StorySubPlot.h"
#include "DebugPrint.h"
#include "XML.h"
#include "GameObject.h"
#include "AI/LuaScript/Commands/GlobalCommands.h"
#include "UtilityCommands.h"
#include "AI/TheAIDataManager.h"
#include "FleetBehavior.h"

static const char *XML_DATA_FILE_PATH = ".\\Data\\XML\\";

#ifndef NDEBUG
static std::map<CRCValue,std::string> EventMap;
#endif



StorySubPlotClass::StorySubPlotClass(const char *name, bool load_plot) :
	Active(true)
{
	Story_Debug_Printf( "Creating story %s\r\n", name );

	// Randomly generated plots aren't loaded
	if (load_plot)
	{
		char xml_filepath[ MAX_PATH ];
		strcpy( xml_filepath, XML_DATA_FILE_PATH );
		strcat( xml_filepath, name );

		// Create a temporary database
		XMLDatabase *component_db = new XMLDatabase;
		assert( component_db != NULL );

		// Read in the temporary database from a file
		HRESULT result = component_db->Read( xml_filepath );
		if ( SUCCEEDED(result) == false ) 
		{
			Story_Debug_Printf( "Error: Can't create story %s\r\n", xml_filepath );
			assert( false );
		}
		assert( SUCCEEDED(result) );

		if ( SUCCEEDED(result) == false )
		{
			delete component_db;
		}

		//
		// Iterate through the database, without destroying original pointer, and create game object types
		//
		XMLDatabase *curr_database_parse = component_db;
		bool creating_object_types = true;
		int index = 0;

		while( creating_object_types == true )
		{
			std::string key_name;
			HRESULT result = component_db->Get_Attribute("Name", key_name);
			assert(SUCCEEDED(result));

			char event_name[ 256 ];
			assert( key_name.size() < sizeof( event_name ) );
			strcpy( event_name, key_name.c_str() );
			_strupr( event_name );
			CRCValue crc_name = CRCClass::Calculate_CRC( event_name, strlen( event_name ) );

			Story_Debug_Printf("___________ Event %s, CRC %u\r\n",event_name,(unsigned int)crc_name);

			stdext::hash_map<CRCValue, StoryEventClass *>::iterator eventptr;
			eventptr = StoryEvents.find(crc_name);
			if (eventptr != StoryEvents.end())
			{
				Story_Debug_Printf("\n!!!!!! Event %s, CRC %u already exists!\r\n",event_name,(unsigned int)crc_name);
				assert(0);
			}

			// Parse this node
			StoryDatabaseParserClass story_parser;
			story_parser.Parse_Database_Entry( curr_database_parse );

			// Create a new event based on the parsed node
			StoryEventClass *event = story_parser.Get_Event();
			FAIL_IF( event == NULL )
			{
				// Skip this event since it's broken.
				result = curr_database_parse->Next_Node();
				if ( result != S_OK )
				{
					creating_object_types = false;
				}
				continue;
			}
			event->Set_Sub_Plot(this);
			event->Set_Index(index++);
			StoryEvents[ crc_name ] = event;

#ifndef NDEBUG
			// Track all events and their CRC values to find NULL event
			EventMap[crc_name] = event_name;
#endif
		
			// Skip past database entry that we just created a type for, and point to next possible object entry
			result = curr_database_parse->Next_Node();
			if ( result != S_OK )
			{
				creating_object_types = false;
			}
		}

		//
		// Done with the game object database until next XML file is loaded
		//
		delete component_db;
		component_db = NULL;

		Sort_Events_And_Compute_Dependants();
	}

	Name = name;
}






StorySubPlotClass::~StorySubPlotClass()
{
	StoryEventListType::iterator eventptr;

	TimeoutEvents.Clear();

	if (LuaScript)
	{
		LuaScript->Shutdown();
		LuaScript = 0;
	}

	for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); eventptr++)
	{
		StoryEventClass *event = eventptr->second;

		// MBL 12.10.2005 - Attempt to fix NULL pointer crash in this function
		// assert(event);
		ENFORCED_IF( event != NULL )
		{
			event->Shutdown();
			delete event;
		}
	}

	StoryEvents.clear();

	for (int i=0; i<STORY_COUNT; i++)
	{
		SortedEvents[i].Clear();
	}
}






StoryEventClass *StorySubPlotClass::Get_Event(const char *name)
{
	char event_name[ 256 ];
	assert( sizeof( name ) < sizeof( event_name ) );
	strcpy( event_name, name );
	_strupr( event_name );
	CRCValue crc_name = CRCClass::Calculate_CRC( event_name, strlen( event_name ) );

	StoryEventListType::iterator eventptr;

	eventptr = StoryEvents.find(crc_name);
	if (eventptr == StoryEvents.end())
	{
		return (NULL);
	}
	else
	{
		return (eventptr->second);
	}
}



void StorySubPlotClass::Lua_Trigger_Event(const std::string &event)
{
	if (LuaScript)
	{
		LuaTable::Pointer params = Lua_Return_Variable(new LuaString(event));
		LuaScript->Call_Function("Story_Event_Trigger", params);
		Free_Lua_Table(params);
	}
}


bool StorySubPlotClass::Attach_Lua_Script(const std::string &name, bool is_load /*= false*/, bool reload /*=false*/)
{
	if (reload && LuaScript)
	{
		LuaScript->Shutdown();
		LuaScript = 0;
	}

	assert(!LuaScript);
	if (is_load)
	{
		LuaScript = LuaScriptClass::Create_For_Load(name);
		UtilityCommandsClass::Register_Commands(LuaScript);
		GlobalCommandsClass::Register_Commands(LuaScript);
	}
	else
	{
		LuaScript = LuaScriptClass::Create_Script(name, reload);
		FAIL_IF(!LuaScript) return false;

		UtilityCommandsClass::Register_Commands(LuaScript);
		GlobalCommandsClass::Register_Commands(LuaScript);

		if (LuaScript->Pool_Is_Fresh_Load())
		{
			LuaScript->Call_Function("Base_Definitions", NULL);
		}

		LuaScript->Create_Thread_Function("main");

		if (LuaScript->Get_Full_Path_Name().size())
		{
			TheAIDataManagerClass::Get().Register_AI_File(LuaScript->Get_Full_Path_Name());
		}
	}

	return true;
}



void StorySubPlotClass::Lua_Script_Service(void)
{
	if (LuaScript) {
		PG_PROFILE("LuaScript Service");
		LuaNumber::Pointer rate = LUA_SAFE_CAST(LuaNumber, LuaScript->Map_Global_From_Lua("ServiceRate"));
		LuaNumber::Pointer last = LUA_SAFE_CAST(LuaNumber, LuaScript->Map_Global_From_Lua("LastService"));

		float elapsed = FrameSynchronizer.Get_Seconds_Elapsed();
		if (rate && (!last || elapsed - last->Value > rate->Value)) {
			LuaScript->Pump_Threads();
			if (!LuaScript->Is_Finished())
			{
				if (!last) last = new LuaNumber;
				last->Value = elapsed;
				LuaScript->Map_Global_To_Lua(last, "LastService");
			}
			else
			{
				LuaScript = NULL;
			}
		}
	}
}



bool StorySubPlotClass::Reload_Script(std::vector<std::string> &file_list)
{
	if (LuaScript)
	{
		for (int i = 0; i < (int)file_list.size(); i++)
		{
			if (_stricmp(file_list[i].c_str(), LuaScript->Get_Full_Path_Name().c_str()) == 0)
			{
				// save off a copy of the script name since Attach_Script is going to delete the name reference returned 
				// from LuaScript->Get_Name
				std::string tmp_name(LuaScript->Get_Name());
				Attach_Lua_Script(tmp_name, false, true);
				return true;
			}
		}
	}
	return true;
}





void StorySubPlotClass::Sort_Events_And_Compute_Dependants()
{
	StoryEventListType::iterator eventptr;

	for (int i=0; i<STORY_COUNT; i++)
	{
		SortedEvents[i].Truncate();
	}

	for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); eventptr++)
	{
		StoryEventClass *event = eventptr->second;
		if (event)
		{
			StoryEventEnum type = event->Get_Event_Type();
			SortedEvents[type].Add(event);

			event->Compute_Dependants();
		}
		else
		{
			Determine_Null_Event(eventptr->first);
		}
	}
}





void StorySubPlotClass::Story_Event(StoryEventEnum event_type, PlayerClass *player, void *param1, void *param2)
{
	// Pass this event on to all the events of this type in the plot
	DynamicVectorClass<StoryEventClass *> *enter_events;

	enter_events = &SortedEvents[event_type];
	for (int i=0; i<enter_events->Size(); i++)
	{
		StoryEventClass *event = (*enter_events)[i];

		if (event)
		{
			if (Is_Event_Active(event) && ((player == NULL) || (player == LocalPlayer)) )
			{
				event->Evaluate_Event(param1,param2);
			}
		}
	}
}





bool StorySubPlotClass::Is_Event_Active(StoryEventClass *event)
{
	if (!event || !event->Is_Active() || event->Is_Triggered() || event->Is_Disabled())
	{
		return (false);
	}

	GameModeClass::GameModeType game_type = GameModeManager.Get_Type();

	// Event is active and not triggered, but some events don't occur in multiplayer
	if ((game_type == GameModeClass::SOLO) || (game_type == GameModeClass::SKIRMISH))
	{
		// Not a multiplayer game, so no reason to disable
		return (true);
	}
	else
	{
		if (event->Is_Multiplayer_Active())
		{
			// This event is still active in multiplayer games
			return (true);
		}
		else
		{
			// This event does not occur in multiplayer games
			return (false);
		}
	}
}







void StorySubPlotClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	StoryEventListType::iterator eventptr;

	for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); eventptr++)
	{
		StoryEventClass *event = eventptr->second;

		if (event)
		{
			event->Replace_Variable(var_name,new_name);
		}
		else
		{
			Determine_Null_Event(eventptr->first);
		}
	}
}







void StorySubPlotClass::Check_Inactive(float elapsed)
{
	StoryEventListType::iterator eventptr;

	for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); eventptr++)
	{
		StoryEventClass *event = eventptr->second;
		if (event)
		{
			float delay = event->Get_Inactive_Delay();

			// If the delay value is greater than zero, then this event is triggered by inactivity
			if ((delay > 0) && Is_Event_Active(event))
			{
				float cur_elapsed = event->Get_Inactive_Elapsed();

				if (cur_elapsed == -1)
				{
					// This event's starting time hasn't been set.  Set it.
					event->Set_Inactive_Elapsed(elapsed);
				}
				else
				{
					float total = elapsed - cur_elapsed;

					if (total >= delay)
					{
						Story_Debug_Printf("STORY INACTIVE TRIGGER - Event %s triggered due to inactivity.  Total elapsed %f, elapsed %f\r\n",event->Get_Name()->c_str(),total,elapsed);
						event->Event_Triggered(NULL,true);
					}
				}
			}
		}
		else
		{
			Determine_Null_Event(eventptr->first);
		}
	}
}







void StorySubPlotClass::Disable_Branch(const char *branch, bool onoff)
{
	assert(branch);

	std::string branch_name = branch;
	StoryEventListType::iterator eventptr;

	// Find all the events with the same branch name
	for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); eventptr++)
	{
		StoryEventClass *event = eventptr->second;

		if (event)
		{
			if (branch_name == event->Get_Branch_Name())
			{
				event->Disable_Event(onoff);
			}
		}
		else
		{
			Determine_Null_Event(eventptr->first);
		}
	}
}





void StorySubPlotClass::Planet_Destroyed(const std::string &planet_name)
{
	StoryEventListType::iterator eventptr;

	for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); eventptr++)
	{
		StoryEventClass *event = eventptr->second;

		// MLL: Check for NULL.
		if (event)
		{
			event->Planet_Destroyed(planet_name);
		}
		else
		{
			Determine_Null_Event(eventptr->first);
		}
	}
}







bool StorySubPlotClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );
	bool ok = true;
	int id = -1;
	std::string event_name;
	std::string to_event_name;
	StoryEventClass *event = NULL;
	std::string script_name;
	StoryEventEnum type = STORY_NONE;

	//Read code for hash map now clears the map before filling it (in order to avoid
	//registering pointers for fixup when they weren't loaded).  We'll need to manually
	//implement the layering that we want.
	StoryEventListType story_events_from_xml = StoryEvents;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
			case STORY_PLOT_DATA_CHUNK:
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK_THIS_PTR		(STORY_PLOT_THIS_ID_CHUNK);
						READ_MICRO_CHUNK					(STORY_PLOT_ACTIVE_CHUNK,					Active);
						READ_MICRO_CHUNK					(STORY_PLOT_PLAYER_ID_CHUNK,				id);
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			case STORY_PLOT_EVENT_DATA_CHUNK:
				{
					while (reader->Open_Micro_Chunk())
					{
						switch ( reader->Cur_Micro_Chunk_ID() )
						{
							READ_MICRO_CHUNK_STRING(STORY_PLOT_EVENT_NAME_CHUNK, event_name);
							READ_MICRO_CHUNK(STORY_PLOT_EVENT_TYPE_CHUNK, type)
							default: assert(false); break;	// Unknown Chunk
						}
						reader->Close_Micro_Chunk();
					}
				}
				break;

			case STORY_PLOT_EVENT_CHUNK:
				event = Get_Event(event_name.c_str());
				if (!event)
				{
					event = StoryDatabaseParserClass::Create_Event(type);
					event->Set_Name(&event_name);
					event->Set_Sub_Plot(this);
				}
				if (event && event->Get_Event_Type() == type)
				{
					ok &= event->Load(reader);
				}
				break;

			case STORY_LUA_SCRIPT_NAME_CHUNK:
				ok &= reader->Read_String(script_name);
				break;

			case STORY_LUA_SCRIPT_DATA_CHUNK:
				if (LuaScript) {
					LuaScript->Shutdown();
					LuaScript = 0;
				}
				assert(script_name.size() != 0);
				Attach_Lua_Script(script_name.c_str(), true);
				ok &= LuaScript->Load_State(reader);
				break;

			case STORY_PLOT_TIMEOUT_DATA_CHUNK:
				// JAC - Removing since these are no longer needed with fixed up audio
				/*
				StoryEventClass *to_event = NULL;
				while (reader->Open_Micro_Chunk())
				{
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
						READ_MICRO_CHUNK_STRING(STORY_PLOT_TIMEOUT_CHUNK, to_event_name);
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();

					to_event = Get_Event(to_event_name.c_str());
					if (to_event)
					{
						TimeoutEvents.Add(to_event);
					}
				}
				*/
				break;

			READ_STL_HASHMAP_NP_P			(STORY_PLOT_EVENT_LIST_CHUNK, 		StoryEvents);
			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	//Now add in story events that exist in xml but not in the save game
	for (StoryEventListType::iterator xml_event_iterator = story_events_from_xml.begin(); 
			xml_event_iterator != story_events_from_xml.end(); 
			++xml_event_iterator)
	{
		CRCValue xml_name_crc = xml_event_iterator->first;
		StoryEventListType::iterator loaded_event_iterator = StoryEvents.find(xml_name_crc);
		if (loaded_event_iterator == StoryEvents.end())
		{
			//This event wasn't loaded.  Either it's new, or the save game is busted.
			StoryEvents.insert(*xml_event_iterator);
		}
	}

	assert(id != -1);
	LocalPlayer = PlayerList.Get_Player_By_ID(id);

	return (ok);
}






bool StorySubPlotClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL );

	bool ok = true;

	int id = LocalPlayer->Get_ID();

	ok &= writer->Begin_Chunk(STORY_PLOT_DATA_CHUNK);
		WRITE_MICRO_CHUNK_THIS_PTR		(STORY_PLOT_THIS_ID_CHUNK);
		WRITE_MICRO_CHUNK					(STORY_PLOT_ACTIVE_CHUNK,					Active);
		WRITE_MICRO_CHUNK					(STORY_PLOT_PLAYER_ID_CHUNK,				id);
	ok &= writer->End_Chunk();

	StoryEventListType::iterator eventptr;
	for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); eventptr++)
	{
		// HY don't know how it could get to NULL but continue if it is
		StoryEventClass *event = eventptr->second;

		if (event)
		{
			ok &= writer->Begin_Chunk(STORY_PLOT_EVENT_DATA_CHUNK);
				WRITE_MICRO_CHUNK_STRING(STORY_PLOT_EVENT_NAME_CHUNK,(*eventptr->second->Get_Name()));
				StoryEventEnum type = eventptr->second->Get_Event_Type();
				WRITE_MICRO_CHUNK(STORY_PLOT_EVENT_TYPE_CHUNK, type)
			ok &= writer->End_Chunk();
			ok &= writer->Begin_Chunk(STORY_PLOT_EVENT_CHUNK);
				ok &= eventptr->second->Save(writer);
			ok &= writer->End_Chunk();
		}
		else
		{
			Determine_Null_Event(eventptr->first);
		}
	}

	if (LuaScript) {
		ok &= writer->Begin_Chunk(STORY_LUA_SCRIPT_NAME_CHUNK);
			ok &= writer->Write_String(LuaScript->Get_Name().c_str());
		ok &= writer->End_Chunk();

		ok &= writer->Begin_Chunk(STORY_LUA_SCRIPT_DATA_CHUNK);
			ok &= LuaScript->Save_State(writer);
		ok &= writer->End_Chunk();
	}

	WRITE_STL_HASHMAP_NP_P			(STORY_PLOT_EVENT_LIST_CHUNK, 		StoryEvents);

	// JAC - Removing since these are no longer needed with fixed up audio
	/*
	if (TimeoutEvents.Size() != 0)
	{
		ok &= writer->Begin_Chunk(STORY_PLOT_TIMEOUT_DATA_CHUNK);
		for (int i=0; i<TimeoutEvents.Size(); i++)
		{
			StoryEventClass *event = TimeoutEvents[i];
			WRITE_MICRO_CHUNK_STRING(STORY_PLOT_TIMEOUT_CHUNK,(*event->Get_Name()) );
		}
		ok &= writer->End_Chunk();
	}
	*/

	return (ok);
}






void StorySubPlotClass::Dump_Status()
{
	Story_Debug_Printf("########################################################\r\n");
	Story_Debug_Printf("\tPlot %s\r\n",Name.c_str());

	//Story_Debug_Printf("\r\n\tEvents :\r\n");
	
	Story_Debug_Printf("\r\n\tTriggered Events:\r\n");  // Dump Status now displays events in 3 separate lists for readability. -Eric_Y 6/6/5
	Dump_Status_Filtered(true,true,false);

	Story_Debug_Printf("\r\n\tActive Events:\r\n");
	Dump_Status_Filtered(true,false,false);

	Story_Debug_Printf("\r\n\tWaiting Events:\r\n");
	Dump_Status_Filtered(false,false,false);

	Story_Debug_Printf("\r\n\tDisabled Events:\r\n");
	Dump_Status_Filtered(false,false,true);

	Story_Debug_Printf("\r\n");
}





void StorySubPlotClass::Dump_Status_Filtered(bool active, bool triggered, bool disabled)
{
	StoryEventListType::iterator eventptr;
	int plot_size = StoryEvents.size();

	for (int i=0; i<plot_size; i++)
	{
		// Really slow sequential search through all the events to find the one with the proper index.  But this is a debug dump anyway
		for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); eventptr++)
		{
			StoryEventClass *event = eventptr->second;

			if (event->Get_Index() == i)
			{
				if (
						(disabled && event->Is_Disabled()) ||
						(!disabled && 
							(
								(event->Is_Active() == active) && 
								(event->Is_Triggered() == triggered) && 
								!event->Is_Disabled()
							)
						)
					)
				{
					event->Dump_Status();
				}

				break;
			}
		}
	}

}





void StorySubPlotClass::Check_Flags()
{
	DynamicVectorClass<StoryEventClass *> *events = &SortedEvents[STORY_FLAG];

	for (int i=0; i<events->Size(); i++)
	{
		StoryEventClass *event = (*events)[i];
		if (Is_Event_Active(event))
		{
			event->Evaluate_Event(NULL,NULL);
		}
	}
}








bool StorySubPlotClass::Check_Special_Land_Tactical_Map(GameObjectClass *hero, GameObjectClass *planet, bool check_land_only)
{
	DynamicVectorClass<StoryEventClass *> *events = &SortedEvents[STORY_LOAD_TACTICAL_MAP];

	// In certain cases we don't want to check for a special map in a LOAD_TACTICAL event
	if (!check_land_only)
	{
		for (int i=0; i<events->Size(); i++)
		{
			StoryEventLoadTacticalClass *event = (StoryEventLoadTacticalClass *)(*events)[i];
			if (Is_Event_Active(event))
			{
				if (event->Check_Special_Land_Tactical_Map(hero,planet))
				{
					return (true);
				}
			}
		}
	}

	// Check for a linked tactical tied to a land_on event, but not if a hero is passed in since they use STORY_DEPLOY
	if ((hero == NULL) || !hero->Get_Original_Object_Type()->Is_Named_Hero())
	{
		events = &SortedEvents[STORY_LAND_ON];
		for (int i=0; i<events->Size(); i++)
		{
			StoryEventLoadTacticalClass *event = (StoryEventLoadTacticalClass *)(*events)[i];
			if (Is_Event_Active(event))
			{
				if (event->Check_Special_Land_Tactical_Map(hero,planet))
				{
					return (true);
				}
			}
		}
	}

	// Check for a linked tactical tied to a hero deploy
	// In order to match the behavior of StoryModeClass::Land_On_Planet we must also check the contents of the transport
	DynamicVectorClass<GameObjectClass *> *transport_contents = NULL;
	if (hero && hero->Behaves_Like(BEHAVIOR_TRANSPORT))
	{
		transport_contents = hero->Get_Transport_Contents();
	}

	events = &SortedEvents[STORY_DEPLOY];
	for (int i=0; i<events->Size(); i++)
	{
		StoryEventLoadTacticalClass *event = (StoryEventLoadTacticalClass *)(*events)[i];
		if (Is_Event_Active(event))
		{
			if (event->Check_Special_Land_Tactical_Map(hero,planet))
			{
				return (true);
			}

			if (transport_contents)
			{
				for (int j = 0; j < transport_contents->Size(); ++j)
				{
					GameObjectClass *contained_unit = transport_contents->Get_At(j);
					if (event->Check_Special_Land_Tactical_Map(contained_unit, planet))
					{
						return true;
					}
				}
			}
		}
	}
	
	return (false);
}






void StorySubPlotClass::Reset_Branch(const char *branch)
{
	assert(branch);

	std::string branch_name = branch;
	StoryEventListType::iterator eventptr;

	// Find all the events with the same branch name
	for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); eventptr++)
	{
		StoryEventClass *event = eventptr->second;
		if (event)
		{
			if (branch_name == event->Get_Branch_Name())
			{
				event->Clear_Triggered();
			}
		}
		else
		{
			Determine_Null_Event(eventptr->first);
		}
	}

	// A second pass needs to be done so each event can determine whether or not it should be active
	// This must be done in a seperate pass since this events are probably interdependant and the
	// prereq evaulation would be invalid until all events are reset.
	for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); eventptr++)
	{
		StoryEventClass *event = eventptr->second;

		if (event)
		{
			if (branch_name == event->Get_Branch_Name())
			{
				// This will force the event to evaluate its prereqs and possibly become active or trigger (STORY_TRIGGER events)
				event->Parent_Triggered();
			}
		}
		else
		{
			Determine_Null_Event(eventptr->first);
		}
	}
}







void StorySubPlotClass::Reset_Event(const char *event_name)
{
	assert(event_name);

	StoryEventClass *event = Get_Event(event_name);

	if (event != NULL)
	{
		event->Clear_Triggered();
	}
}





void StorySubPlotClass::Trigger_Event(const char *event_name)
{
	assert(event_name);

	StoryEventClass *event = Get_Event(event_name);

	if (event != NULL)
	{
		event->Event_Triggered();
	}
}




void StorySubPlotClass::Disable_Event(const char *event_name, bool onoff)
{
	assert(event_name);

	StoryEventClass *event = Get_Event(event_name);

	if (event != NULL)
	{
		event->Disable_Event(onoff);
	}
}




void StorySubPlotClass::Reset_All_Events()
{
	StoryEventListType::iterator eventptr;

	// Find all the events with the same branch name
	for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); eventptr++)
	{
		StoryEventClass *event = eventptr->second;
		if (event)
		{
			event->Clear_Triggered();
			event->Reset();
		}
		else
		{
			Determine_Null_Event(eventptr->first);
		}
	}
}





void StorySubPlotClass::Speech_Killed()
{
	DynamicVectorClass<StoryEventClass *> *events = &SortedEvents[STORY_SPEECH_DONE];
	DynamicVectorClass<StoryEventClass *> events_copy;

	// Make up a list of all active STORY_SPEECH_DONE events at this time.  Calling Event_Triggered on 
	// these events may add new active STORY_SPEECH_DONE events and we don't want to trigger the newly activated ones.
	for (int i=0; i<events->Size(); i++)
	{
		StoryEventClass *event = (*events)[i];
		if (event)
		{
			if (Is_Event_Active(event))
			{
				events_copy.Add(event);
			}
		}
	}

	for (int i=0; i<events_copy.Size(); i++)
	{
		StoryEventClass *event = events_copy[i];
		if (Is_Event_Active(event))
		{
			Story_Debug_Printf("- Speech cancelled - Triggering %s\r\n",event->Get_Name()->c_str());
			event->Event_Triggered();
		}
	}
}




bool StorySubPlotClass::Check_Planet_Entry_Restrictions(GameObjectClass *fleet, GameObjectClass *planet)
{
	FAIL_IF(!fleet) { return false; }
	FAIL_IF(!planet) { return false; }

	DynamicVectorClass<StoryEventClass *> *events = &SortedEvents[STORY_ENTER];
	for (int i = 0; i < events->Size(); ++i)
	{
		StoryEventClass *event = events->Get_At(i);
		if (Is_Event_Active(event))
		{
			if (event->Check_Planet_Entry_Restrictions(fleet, planet))
			{
				return true;
			}
		}
	}

	events = &SortedEvents[STORY_MOVE];
	for (int i = 0; i < events->Size(); ++i)
	{
		StoryEventClass *event = events->Get_At(i);
		if (Is_Event_Active(event))
		{
			if (event->Check_Planet_Entry_Restrictions(fleet, planet))
			{
				return true;
			}
		}
	}

	FleetBehaviorClass *fleet_behavior = static_cast<FleetBehaviorClass*>(fleet->Get_Behavior(BEHAVIOR_FLEET));
	FAIL_IF(!fleet_behavior) { return false; }

	if (fleet_behavior->Is_Instant_Land())
	{
		events = &SortedEvents[STORY_LAND_ON];
		for (int i = 0; i < events->Size(); ++i)
		{
			StoryEventClass *event = events->Get_At(i);
			if (Is_Event_Active(event))
			{
				if (event->Check_Planet_Entry_Restrictions(fleet, planet))
				{
					return true;
				}
			}
		}

		events = &SortedEvents[STORY_DEPLOY];
		for (int i = 0; i < events->Size(); ++i)
		{
			StoryEventClass *event = events->Get_At(i);
			if (Is_Event_Active(event))
			{
				if (event->Check_Planet_Entry_Restrictions(fleet, planet))
				{
					return true;
				}
			}
		}
	}
	
	return false;
}





bool StorySubPlotClass::Check_Special_Space_Tactical_Map(GameObjectClass *planet, GameObjectClass *hero)
{
	assert(planet);

	DynamicVectorClass<StoryEventClass *> *events = &SortedEvents[STORY_ENTER];
	for (int i = 0; i < events->Size(); ++i)
	{
		StoryEventClass *event = events->Get_At(i);
		if (Is_Event_Active(event))
		{
			StoryRewardEnum reward = event->Get_Reward_Type();
			if (reward == REWARD_LINK_TACTICAL)
			{
				if (event->Check_Special_Space_Tactical_Map(hero,planet))
				{
					return (true);
				}
			}
		}
	}

	events = &SortedEvents[STORY_MOVE];
	for (int i = 0; i < events->Size(); ++i)
	{
		StoryEventClass *event = events->Get_At(i);
		if (Is_Event_Active(event))
		{
			StoryRewardEnum reward = event->Get_Reward_Type();
			if (reward == REWARD_LINK_TACTICAL)
			{
				if (event->Check_Special_Space_Tactical_Map(hero,planet))
				{
					return (true);
				}
			}
		}
	}

	return false;
}





void StorySubPlotClass::Add_Timout_Event(StoryEventClass * /*event*/)
{
	// JAC - Removing since these are no longer needed with fixed up audio
	/*
	if (TimeoutEvents.Get_Index(event) == -1)
	{
		TimeoutEvents.Add(event);
	}
	*/
}





void StorySubPlotClass::Remove_Timeout_Event(StoryEventClass * /*event*/)
{
	// JAC - Removing since these are no longer needed with fixed up audio
	//TimeoutEvents.Delete(event);
}







void StorySubPlotClass::Check_Timeout(float /* elapsed */)
{
	// Just to be safe, I'm disabling this, but it should work fine
	#if 0
	// JAC - Updated so that this checks all active speech events
	DynamicVectorClass<StoryEventClass *> *events = &SortedEvents[STORY_SPEECH_DONE];
	for (int i = 0; i < events->Size(); ++i)
	{
		StoryEventClass *event = events->Get_At(i);
		if (Is_Event_Active(event))
		{
			float timeout = event->Get_Timeout_Time();
			if (timeout != -1)
			{
				float start_time = event->Get_Start_Time();
				if (start_time == -1)
				{
					// For some reason, the start time didn't get set.  Set it now
					start_time = GameModeManager.Get_Frame_Timer() * FrameSynchronizer.Get_Inv_Logical_FPS();
					event->Set_Start_Time(start_time);
				}

				float total_elapsed = elapsed - start_time;
				if ((total_elapsed >= timeout) && !event->Is_Disabled())
				{
					// The event has timedout.  Force it to fire
					Story_Debug_Printf("STORY MODE - Event %s is being triggered due to timeout\r\n",event->Get_Name()->c_str());
					assert(timeout > total_elapsed);	// Notify the designer that the script has had to timeout an event
					event->Event_Triggered();
				}
				else
				{
					//Debug_Printf("...... Event %s, remainder %f\n",event->Get_Name()->c_str(),timeout - total_elapsed);
				}
			}
		}
	}
	#endif

	// This is the old method that supported any event
	// JAC - Removing since these are no longer needed with fixed up audio
	/*
	for (int i=0; i<TimeoutEvents.Size(); i++)
	{
		StoryEventClass *event = TimeoutEvents[i];

		float timeout = event->Get_Timeout_Time();
		if (timeout != -1)
		{
			float start_time = event->Get_Start_Time();
			if (start_time == -1)
			{
				// For some reason, the start time didn't get set.  Set it now
				start_time = GameModeManager.Get_Frame_Timer() * FrameSynchronizer.Get_Inv_Logical_FPS();
				event->Set_Start_Time(start_time);
			}

			float total_elapsed = elapsed - start_time;
			if ((total_elapsed >= timeout) && !event->Is_Disabled())
			{
				// The event has timedout.  Force it to fire
				Story_Debug_Printf("STORY MODE - Event %s is being triggered due to timeout\r\n",event->Get_Name()->c_str());
				assert(timeout > total_elapsed);	// Notify the designer that the script has had to timeout an event
				event->Event_Triggered();
			}
			else
			{
				//Debug_Printf("...... Event %s, remainder %f\n",event->Get_Name()->c_str(),timeout - total_elapsed);
			}
		}
	}
	*/
}




#ifndef NDEBUG

void StorySubPlotClass::Check_For_Null_Events()
{
	// Sanity check to remove bad events
	StoryEventListType::iterator eventptr;
	for (eventptr = StoryEvents.begin(); eventptr != StoryEvents.end(); )
	{
		StoryEventClass *event = eventptr->second;
		CRCValue crc_index = eventptr->first;
		eventptr++;

		assert (event != NULL);
		if (event == NULL)
		{
			Determine_Null_Event(crc_index);
			StoryEvents.erase(crc_index);
		}
	}
}





void StorySubPlotClass::Determine_Null_Event(CRCValue crc_index)
{
	std::map<CRCValue,std::string>::iterator eventptr;
	eventptr = EventMap.find(crc_index);

	if (eventptr != EventMap.end())
	{
		Debug_Printf("\n\n!!!!! ERROR!!!! NULL STORY EVENT!!!! Name %s\n\n",eventptr->second.c_str());
	}
	else
	{
		Debug_Printf("\n\n!!!!! ERROR!!!! NULL STORY EVENT!!!! CRC %d\n\n",eventptr->first);
	}

	// Something's gone wrong if we hit this function
	assert(0);
}

#else

void StorySubPlotClass::Determine_Null_Event(CRCValue)
{
}


#endif

