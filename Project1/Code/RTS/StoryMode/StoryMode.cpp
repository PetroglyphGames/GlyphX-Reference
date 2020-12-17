//////////////////////////////////////////////////////////////////////////////////////////////////
//
// (C) Petroglyph Games, LLC
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
//
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/StoryMode/StoryMode.cpp $
//
//             Author: Jason Curtice
//
//               Date: 07/21/2004 6:23PM
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma hdrstop

#include "StoryMode.h"

#include "CommandBar.h"
#include "File.h"
#include "DebugPrint.h"
#include "GameObject.h"
#include "GameObjectType.h"
#include "GameObjectTypeManager.h"
#include "FactionList.h"
#include "Faction.h"
#include "PlayerList.h"
#include "ChunkFile.h"
#include "TransportBehavior.h"
#include "Tutorial.h"
#include "GameObjectManager.h"
#include "SpeechEvent.h"
#include "Diagnostics/DebugWindow.h"
#include "Diagnostics/LogWindow.h"
#include "WeatherSystem.h"
#include "GameText.h"
#include "GameConstants.h"
#include "SpawnIndigenousUnitsBehavior.h"
#include "FleetBehavior.h"
#include "ScheduledEventQueue.h"

static const char *XML_DATA_FILE_PATH = ".\\Data\\XML\\";
//StoryModeClass TheStoryMode;

StoryModeClass::StoryFlagListType StoryModeClass::Flags;
DynamicVectorClass<int> StoryModeClass::LandForces;
const bool *StoryModeClass::IsForegroundApp = NULL;



/**************************************************************************************************
* ::~StoryModeClass -- Story Mode destructor
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:24PM JAC
**************************************************************************************************/
StoryModeClass::~StoryModeClass()
{
	Remove_Plots();
}









/**************************************************************************************************
* StoryModeClass::Remove_Plots -- Remove all loaded plots
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:24PM JAC
**************************************************************************************************/
void StoryModeClass::Remove_Plots()
{
	SubPlotListType::iterator plotptr;

	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		assert(subplot);
		delete subplot;
	}

	SubPlots.clear();
}



/**************************************************************************************************
* StoryModeClass::Reload_Scripts -- Reload any lua scripts as necessary
*
* In:		
*
* Out:	
*
*
* History: 5/3/2005 2:23:08 PM -- BMH
**************************************************************************************************/
void StoryModeClass::Reload_Scripts(std::vector<std::string> &files)
{
	SubPlotListType::iterator plotptr;

	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		subplot->Reload_Script(files);
	}
}


/**************************************************************************************************
* StoryModeClass::Find_Lua_Script -- Find the lua script in the story mode system.
*
* In:		
*
* Out:	
*
*
* History: 8/18/2005 4:28:42 PM -- BMH
**************************************************************************************************/
LuaScriptClass *StoryModeClass::Find_Lua_Script(const std::string &name)
{
	SubPlotListType::iterator plotptr;

	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		LuaScriptClass *script = subplot->Get_Lua_Script();
		if (script && _stricmp(script->Get_Name().c_str(), name.c_str()) == 0)
		{
			return script;
		}
	}
	return NULL;
}


/**************************************************************************************************
* StoryModeClass::Handle_ESC -- Pass ESC event into active lua scripts.
*
* In:		
*
* Out:	
*
*
* History: 9/15/2005 3:57:20 PM -- BMH
**************************************************************************************************/
void StoryModeClass::Handle_ESC(void)
{
	SubPlotListType::iterator plotptr;

	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		if (subplot->Is_Active())
		{
			LuaScriptClass *script = subplot->Get_Lua_Script();
			if (script)
			{
				script->Call_Function("Story_Handle_Esc", NULL);
			}
		}
	}
}


/**************************************************************************************************
* StoryModeClass::Load_Plots -- Load all plots listed in XML file
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:25PM JAC
**************************************************************************************************/
bool StoryModeClass::Load_Plots(const std::string &name, PlayerClass *player)
{
	//if (SubPlots.size())
	//{
	//	Remove_Plots();
	//}

	FAIL_IF(!player) { return false; }

	if ( _stricmp( name.c_str(), "" ) == 0 )
	{
		return true;
	}

	PlotName = name;
	std::string fullname = ".\\Data\\XML\\" + name;

	//
	// Allocate and read a temporary database of the filenames
	//
	XMLDatabase *file_list_db = new XMLDatabase;
	assert( file_list_db != NULL );
	HRESULT result = file_list_db->Read( fullname );
	//assert( SUCCEEDED(result) );
	if ( SUCCEEDED(result) == false )
	{
		Story_Debug_Printf("Error!  Can't find story XML file called %s\r\n",fullname.c_str());
		delete file_list_db;
		return (false);
	}

	// Save off the filename
	Plots.push_back(std::make_pair(player->Get_ID(), name));

	//
	// Build a list of XML files by adding to the list
	//	
	XMLDatabase *temp_db = file_list_db;
	result = S_OK;
	std::string lua_script;
	do 
	{
		std::string key;
		result = temp_db->Get_Key( &key );
		assert(SUCCEEDED(result));

		if (_stricmp(key.c_str(), "Lua_Script") == 0)
		{
			std::string value;
			result = temp_db->Get_Value( &value );
			assert(SUCCEEDED(result));
         lua_script = Trim_Whitespace(value.c_str());
			result = temp_db->Next_Node();
			continue;
		}

		assert( (key == "Active_Plot") || (key == "Suspended_Plot") );

		bool active = (key == "Active_Plot");

		std::string value;
		result = temp_db->Get_Value( &value );
		assert(SUCCEEDED(result));

		if ( value.empty() == false )
		{
			// Add the file name to the list
			std::string xml_file_path = XML_DATA_FILE_PATH + value;
			FileClass xml_file( xml_file_path );
			bool opened = xml_file.Open();
			if( opened == false )
			{
				Story_Debug_Printf( "Error: Story mode unable to open listed XML file '%s'\r\n", xml_file_path.c_str() );
				assert( false );
			}
			else
			{
				Load_Single_Plot(value,lua_script,active,player);

				xml_file.Close();
			}
		}

		lua_script.resize(0);
		result = temp_db->Next_Node();
	} while (result == S_OK);

	//
	// We're done with the files list database, since the files are now in a list
	//
	delete file_list_db;
	file_list_db = NULL;

	return (true);
}








StorySubPlotClass *StoryModeClass::Load_Single_Plot(std::string &value, std::string &lua_script, bool active, PlayerClass *player)
{
	StorySubPlotClass *subplot = new StorySubPlotClass(value.c_str());
	assert(subplot);

	subplot->Set_Story_Mode(this);
	subplot->Set_Active(active);
	subplot->Set_Local_Player(player);
	if (lua_script.size())
	{
		subplot->Attach_Lua_Script(lua_script);
	}

	CRCValue xml_file_name_crc = CRCClass::Calculate_CRC( value.c_str(), strlen( value.c_str() ) );

	SubPlots[xml_file_name_crc] = subplot;

	return (subplot);
}






/**************************************************************************************************
* StoryModeClass::Get_Sub_Plot -- Get specific plot
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:25PM JAC
**************************************************************************************************/
StorySubPlotClass *StoryModeClass::Get_Sub_Plot(const std::string &name)
{
	CRCValue name_crc = CRCClass::Calculate_CRC( name.c_str(), strlen( name.c_str() ) );
	SubPlotListType::iterator plotptr;

	plotptr = SubPlots.find(name_crc);
	if (plotptr != SubPlots.end())
	{
		return (plotptr->second);
	}

	return (NULL);
}








/**************************************************************************************************
* StoryModeClass::Activate_Sub_Plot -- Activate plot
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:25PM JAC
**************************************************************************************************/
bool StoryModeClass::Activate_Sub_Plot(const std::string &name)
{
	StorySubPlotClass *subplot = Get_Sub_Plot(name);

	if (subplot)
	{
		subplot->Set_Active(true);
		return (true);
	}

	return (false);
}




bool StoryModeClass::Deactivate_Sub_Plot(const std::string &name)
{
	StorySubPlotClass *subplot = Get_Sub_Plot(name);

	if (subplot)
	{
		subplot->Set_Active(false);
		return (true);
	}

	return (false);
}




/**************************************************************************************************
* StoryModeClass::Story_Event -- Pass event onto all loaded plots
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:26PM JAC
**************************************************************************************************/
void StoryModeClass::Story_Event(StoryEventEnum event, PlayerClass *player, void *param1, void *param2)
{
	// MLL: No need to queue story events in demo attract mode.
	if ((NULL == IsForegroundApp) || GameModeManager.Is_Demo_Attract_Mode())
		return;

	if ((ParentMode != GameModeManager.Get_Active_Mode()) || (*IsForegroundApp == false && GameModeManager.Is_Multiplayer_Mode() == false && ScheduledEventQueue.Is_Playing_Back() == false))
	{
		// If we delay the tactical destroy event, the parameters will be invalid
		if ((event != STORY_TACTICAL_DESTROY) && (event != STORY_SPACE_TACTICAL) && (event != STORY_LAND_TACTICAL) )
		{
			// If this mode isn't active, save off this event and wait until it is active
			DelayedEventStruct *delayed_event = new DelayedEventStruct;
			assert(delayed_event);

			delayed_event->Event = event;
			delayed_event->Player = player;
			delayed_event->Param1 = param1;
			delayed_event->Param2 = param2;

			// Some parameters are strings that may not be valid in the future
			switch (event)
			{
				case STORY_SPEECH_DONE:
					delayed_event->Param1String = (const char *)param1;
					delayed_event->Param1 = (void *)delayed_event->Param1String.c_str();
					break;

				default:
					break;
			}

			DelayedEvents.Add(delayed_event);

			Story_Debug_Printf("STORY DELAYED EVENT - This event is being sent to an inactive mode.\r\n");
		}
		return;
	}

	SubPlotListType::iterator plotptr;
	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		assert(subplot);

		if (subplot->Is_Active())
		{
			subplot->Story_Event(event,player,param1,param2);
		}
	}
}




/**************************************************************************************************
* StoryModeClass::Check_Plots -- Check plots for events that may happen every game frame
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:17AM JAC
**************************************************************************************************/
void StoryModeClass::Check_Plots(float elapsed)
{
	SubPlotListType::iterator plotptr;
	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		assert(subplot);

		subplot->Lua_Script_Service();

		if (subplot->Is_Active())
		{
			subplot->Check_Inactive(elapsed);
			subplot->Check_Flags();
			subplot->Check_Timeout(elapsed);
#ifndef NDEBUG
			//subplot->Check_For_Null_Events();
#endif
		}
	}
}








/**************************************************************************************************
* StoryModeClass::Execute_Delayed_Events -- Execute any galactic events that may have happened during tactical
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:17AM JAC
**************************************************************************************************/
void StoryModeClass::Execute_Delayed_Events()
{
	if(NULL == IsForegroundApp)
		return;
	// Don't execute delayed events if the game is paused
	if ((ParentMode == GameModeManager.Get_Active_Mode()) && (FrameSynchronizer.Is_Single_Step_Mode_Enabled() == false) && 
		 (*IsForegroundApp == true || GameModeManager.Is_Multiplayer_Mode() == true))
	{
		for (int i=0; i<DelayedEvents.Size(); i++)
		{
			DelayedEventStruct *delayed_event = DelayedEvents[i];

			Story_Debug_Printf("STORY DELAYED EVENT - Executing event.\r\n");
			Story_Event(delayed_event->Event,delayed_event->Player,delayed_event->Param1,delayed_event->Param2);
			delete delayed_event;
		}

		DelayedEvents.Resize(0);
	}
}



/**************************************************************************************************
* StoryModeClass::Enter_System -- Enter System event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:30PM JAC
**************************************************************************************************/
void StoryModeClass::Enter_System(GameObjectClass *planet, GameObjectClass *fleet)
{
	assert(planet);
	{
		Event_Debug_Printf("STORY - ENTERED SYSTEM %s\r\n",planet->Get_Type()->Get_Name()->c_str());
	}
	Story_Event(STORY_ENTER,NULL,planet,fleet);

	// Check for the special case of a hero entering a system
	if (fleet)
	{
		DynamicVectorClass<GameObjectClass*> fleet_objects;
		fleet->Get_Fleet_Breakdown(fleet_objects);

		std::string planetname = *planet->Get_Type()->Get_Name();

		// Check all the fleet objects
		for (int i=0; i<fleet_objects.Size(); i++)
		{
			// The hero should be in a transport
			GameObjectClass *member = fleet_objects[i];
			const GameObjectTypeClass *objtype = member->Get_Original_Object_Type();

			// Sometimes the ship itself is the hero (the hero never leaves the ship)
			if (objtype->Is_Named_Hero())
			{
				std::string heroname = *objtype->Get_Name();
				Story_Event(STORY_MOVE,NULL,&heroname,planet);
			}

			// Check the ship contents for a hero
			if (member->Behaves_Like(BEHAVIOR_TRANSPORT))
			{
				// Check the transport contents for a hero
				DynamicVectorClass<GameObjectClass *> *cargo = member->Get_Transport_Contents();
				for (int i=0; i<cargo->Size(); i++)
				{
					objtype = (*cargo)[i]->Get_Type();
					if (objtype->Is_Named_Hero() || objtype->Is_Generic_Hero())
					{
						std::string heroname = *objtype->Get_Name();
						{
							Event_Debug_Printf("STORY - HERO %s ENTERED %s\r\n",heroname.c_str(),planetname.c_str());
						}
						Story_Event(STORY_MOVE,NULL,&heroname,planet);
					}
				}
			}
		}
	}
}




/**************************************************************************************************
* StoryModeClass::Land_On_Planet -- Land on planet event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:31PM JAC
**************************************************************************************************/
void StoryModeClass::Land_On_Planet(GameObjectClass *planet, GameObjectClass *transport)
{
	assert(planet);

	Event_Debug_Printf("STORY - LANDED ON %s\r\n",planet->Get_Type()->Get_Name()->c_str());

	// The StoryEventEnterClass expects to get a fleet! (not a transport) -Eric_Y
	if (transport->Get_Parent_Container_Object() && transport->Get_Parent_Container_Object()->Behaves_Like(BEHAVIOR_FLEET))
	{
		Story_Event(STORY_LAND_ON,NULL,planet,transport->Get_Parent_Container_Object());
	}
	else
	{
		Story_Event(STORY_LAND_ON,NULL,planet,transport);
	}


	// Check for the special case of a hero landing on a planet
	if (transport)
	{
		std::string planetname = *planet->Get_Type()->Get_Name();

		// Check the type that the transport holds
		const GameObjectTypeClass *hero_team = transport->Get_Original_Object_Type();
		if (hero_team->Is_Named_Hero())
		{
			std::string heroname = *hero_team->Get_Name();
			Story_Event(STORY_DEPLOY,NULL,&heroname,planet);
			Event_Debug_Printf("STORY - HERO TEAM %s DEPLOYED ON %s\r\n",heroname.c_str(),planetname.c_str());
		}

		// Check each item in the transport
		DynamicVectorClass<GameObjectClass *> *cargo = transport->Get_Transport_Contents();
		for (int i=0; i<cargo->Size(); i++)
		{
			const GameObjectTypeClass *objtype = (*cargo)[i]->Get_Type();
			if (objtype->Is_Named_Hero() || objtype->Is_Generic_Hero())
			{
				std::string heroname = *objtype->Get_Name();
				{
					Event_Debug_Printf("STORY - HERO %s DEPLOYED ON %s\r\n",heroname.c_str(),planetname.c_str());
				}
				Story_Event(STORY_DEPLOY,NULL,&heroname,planet);
			}
		}
	}
}


/**************************************************************************************************
* StoryModeClass::Hero_Land_On_Planet -- A hero has "landed" on the given planet. I say "landed"
*	in quotes because it may be a fictional landing (ex: a hero activating a GROUND_ACTIVATED
*	special ability - like Siphon Credits - never physically moves down to the planet's surface,
*	but from the player's point of view, the hero is on the ground stealing money).
*
* In:		planet - The planet the hero "landed" on.
*			hero - The unit that has "landed" on the planet.
*
* Out:	
*
*
* History: 08/13/2004 10:37AM AJA
**************************************************************************************************/
void StoryModeClass::Hero_Land_On_Planet(GameObjectClass *planet, GameObjectClass *hero)
{
	FAIL_IF( planet == NULL )			{ return; }
	FAIL_IF( hero == NULL )				{ return; }

	// Trigger a general "Landed" story event.
	{
		Event_Debug_Printf("STORY - LANDED ON %s\r\n", planet->Get_Type()->Get_Name()->c_str());
	}
	Story_Event(STORY_LAND_ON, NULL, planet, NULL);

	// Trigger a "Deployed" story event for the given hero.
	assert( hero->Is_Hero() );	// is he really a hero?
	const std::string *heroname = hero->Get_Type()->Get_Name();
	{
		const std::string *planetname = planet->Get_Type()->Get_Name();
		Event_Debug_Printf("STORY - HERO %s DEPLOYED ON %s\r\n", heroname->c_str(), planetname->c_str());
	}
	Story_Event(STORY_DEPLOY, NULL, (void*)heroname, planet);
}




/**************************************************************************************************
* StoryModeClass::Conquer -- Conquer planet event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:31PM JAC
**************************************************************************************************/
void StoryModeClass::Conquer(GameObjectClass *planet)
{
	assert(planet);

	//PlayerClass *local_player = PlayerList.Get_Local_Player();
	//const FactionClass *planet_faction = FactionList.Get_Faction_From_Allegiance(planet->Get_Allegiance());

	//if (local_player->Get_Faction() == planet_faction)
	{
		std::string objname = *planet->Get_Type()->Get_Name();
		{
			Event_Debug_Printf("STORY - PLANET CONQUERED %s\r\n",objname.c_str());
		}
		Story_Event(STORY_CONQUER,NULL,&objname,planet);
		Story_Event(STORY_CONQUER_COUNT,NULL,planet,NULL);
	}
}




/**************************************************************************************************
* StoryModeClass::Construct -- Construct item event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:31PM JAC
**************************************************************************************************/
void StoryModeClass::Construct(GameObjectClass *planet, const GameObjectTypeClass *object_type)
{
	//assert(object);
	if (/*(planet == NULL) || */(object_type == NULL))
	{
		return;
	}

	// Make sure the player produced this object
	//PlayerClass *local_player = PlayerList.Get_Local_Player();
	//const FactionClass *planet_faction = FactionList.Get_Faction_From_Allegiance(planet->Get_Allegiance());

	//if (local_player->Get_Faction() == planet_faction)
	{
		std::string objname = *object_type->Get_Name();
		{
			Event_Debug_Printf("STORY - CONSTRUCTED %s\r\n",objname.c_str());
		}
		Story_Event(STORY_CONSTRUCT,NULL,&objname,planet);

		// There's an additional event that may be triggered by this construction - base level
		if (planet && (object_type->Behaves_Like( BEHAVIOR_DUMMY_GROUND_BASE ) || object_type->Behaves_Like( BEHAVIOR_DUMMY_STAR_BASE )))
		{
			std::string planetname = *planet->Get_Type()->Get_Name();
			{
				Event_Debug_Printf("STORY - CONSTRUCTED LEVEL %s\r\n",objname.c_str());
			}
			Story_Event(STORY_CONSTRUCT_LEVEL,NULL,planet,(void *)object_type);
		}
	}
}





/**************************************************************************************************
* StoryModeClass::Political_Control -- Political Control Level event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:32PM JAC
**************************************************************************************************/
void StoryModeClass::Political_Control(GameObjectClass *planet, int allegiance)
{
	assert (planet);

	//PlayerClass *local_player = PlayerList.Get_Local_Player();
	//const FactionClass *planet_faction = FactionList.Get_Faction_From_Allegiance(planet->Get_Allegiance());

	PlayerClass *owner = PlayerList.Get_Player_By_ID(planet->Get_Owner());

	//if (local_player->Get_Faction() == planet_faction)
	{
		std::string objname = *planet->Get_Type()->Get_Name();
		{
			Event_Debug_Printf("STORY - POLITICAL CONTROL %s\r\n",objname.c_str());
		}
		Story_Event(STORY_POLITICAL_CONTROL,owner,planet,&allegiance);
	}
}




/**************************************************************************************************
* StoryModeClass::Accumulate -- Accumulate event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:33PM JAC
**************************************************************************************************/
void StoryModeClass::Accumulate(PlayerClass *player, int credits)
{
	//PlayerClass *local_player = PlayerList.Get_Local_Player();

	//if (local_player == player)
	{
		{
			//Event_Debug_Printf("STORY - ACCUMULATE %s\r\n",objname.c_str());
		}
		Story_Event(STORY_ACCUMULATE,player,&credits,NULL);
	}
}




/**************************************************************************************************
* StoryModeClass::Elapsed -- Time Elapsed event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:33PM JAC
**************************************************************************************************/
void StoryModeClass::Elapsed(float elapsed)
{
	bool galactic_paused = false;

	if (GameModeManager.Get_Sub_Type() == SUB_GAME_MODE_GALACTIC)
	{
		if (GameModeManager.Is_Galactic_Gameplay_Paused())
		{
			galactic_paused = true;
		}
	}

	Execute_Delayed_Events();

	Story_Event(STORY_ELAPSED,NULL,&elapsed,&galactic_paused);

	// Some events are triggered by inaction
	Check_Plots(elapsed);

	// Check to see if any units are near a specific target object
	Check_Proximity();

	// Check to see if all units/structures/both are destroyed
	Check_Destroyed();

	// Fog reveal events are checked every time
	Story_Event(STORY_FOG_OBJECT_REVEAL,NULL,NULL,NULL);
	Story_Event(STORY_FOG_POSITION_REVEAL,NULL,NULL,NULL);
	Story_Event(STORY_OBJECTIVE_TIMEOUT,NULL,&elapsed,&galactic_paused);

	// Check for a delayed battle end dialog event
	if (DelayedBattleEnd)
	{
		Tutorial_Generic("battle_end_closed");
		DelayedBattleEnd = false;
	}
}




/**************************************************************************************************
* StoryModeClass::Begin_Time_Era -- Begin Time Era event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:33PM JAC
**************************************************************************************************/
void StoryModeClass::Begin_Time_Era(int era)
{
	{
		//Event_Debug_Printf("STORY - BEGIN_ERA %d\r\n",era);
	}
	Story_Event(STORY_BEGIN_ERA,NULL,&era,NULL);
}





void StoryModeClass::Tech_Level(int level, PlayerClass *player)
{
	//if (player == PlayerList.Get_Local_Player())
	{
		{
			Event_Debug_Printf("STORY - TECH LEVEL %d\r\n",level);
		}
		Story_Event(STORY_TECH_LEVEL,player,&level,NULL);
	}
}


/**************************************************************************************************
* StoryModeClass::Destroy -- Destroy Object event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:33PM JAC
**************************************************************************************************/
void StoryModeClass::Destroy(GameObjectClass *object)
{
	assert(object);
	GameObjectClass *parent = object->Get_Parent_Container_Object();
	while (parent && !parent->Behaves_Like(BEHAVIOR_PLANET))
	{
		parent = parent->Get_Parent_Container_Object();
	}

	if ((parent == NULL) || !parent->Behaves_Like(BEHAVIOR_PLANET))
	{
		return;
	}

	const GameObjectTypeClass *object_type = object->Get_Type();
	Story_Event(STORY_DESTROY,NULL,(void *)object_type,parent);

	const GameObjectTypeClass *original_type = object->Get_Original_Object_Type();
	if (original_type != object_type)	// Don't want to send the same destroy event twice
	{
		Story_Event(STORY_DESTROY,NULL,(void *)original_type,parent);
	}
}





/**************************************************************************************************
* StoryModeClass::Destroy_Base -- Base destroyed
*
* In:		
*
* Out:	
*
*
* History: 11/23/2004 2:43PM JAC
**************************************************************************************************/
void StoryModeClass::Destroy_Base(GameObjectClass *planet, StoryBaseFilter type)
{
	assert(planet->Behaves_Like(BEHAVIOR_PLANET));
	{
		Event_Debug_Printf("STORY - BASE DESTROYED on planet %s\r\n",planet->Get_Type()->Get_Name()->c_str());
	}
	Story_Event(STORY_DESTROY_BASE,NULL,planet,&type);
}




/**************************************************************************************************
* StoryModeClass::Tactical_Destroy -- Unit destroyed in tactical combat
*
* In:		
*
* Out:	
*
*
* History: 11/23/2004 2:43PM JAC
**************************************************************************************************/
void StoryModeClass::Tactical_Destroy(GameObjectClass *object)
{
	assert(object);

	const GameObjectTypeClass *object_type = object->Get_Type();
	Story_Event(STORY_TACTICAL_DESTROY,NULL,(void *)object_type,NULL);

	const GameObjectTypeClass *original_type = object->Get_Original_Object_Type();
	if (original_type != object_type)	// Don't want to send the same destroy event twice
	{
		Story_Event(STORY_TACTICAL_DESTROY,NULL,(void *)original_type,NULL);
	}

	if (object->Behaves_Like(BEHAVIOR_TRANSPORT))
	{
		DynamicVectorClass<GameObjectClass *> *cargo = object->Get_Transport_Contents();
		for (int i=0; i<cargo->Size(); i++)
		{
			const GameObjectTypeClass *cargo_type = (*cargo)[i]->Get_Type();
			if (cargo_type->Is_Named_Hero() && (cargo_type != original_type))
			{
				Story_Event(STORY_TACTICAL_DESTROY,NULL,(void *)cargo_type,NULL);
			}
		}
	}
}





/**************************************************************************************************
* StoryModeClass::Capture_Hero -- Capture Hero event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:34PM JAC
**************************************************************************************************/
void StoryModeClass::Capture_Hero(GameObjectClass *hero)
{
	assert(hero);
	GameObjectClass *parent = hero->Get_Parent_Container_Object();
	while (parent && !parent->Behaves_Like(BEHAVIOR_PLANET))
	{
		parent = parent->Get_Parent_Container_Object();
	}

	if ((parent == NULL) || !parent->Behaves_Like(BEHAVIOR_PLANET))
	{
		return;
	}

	std::string heroname = *hero->Get_Type()->Get_Name();
	{
		Event_Debug_Printf("STORY - HERO CAPTURED %s\r\n",heroname.c_str());
	}
	Story_Event(STORY_CAPTURE_HERO,NULL,&heroname,parent);
}





/**************************************************************************************************
* StoryModeClass::Defeat_Hero -- Defeat Hero event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:34PM JAC
**************************************************************************************************/
void StoryModeClass::Defeat_Hero(GameObjectClass *hero)
{
	FAIL_IF( hero == NULL )		{ return; }

	// We only care about major heroes, minor heroes don't count.
	if ( hero->Get_Type()->Is_Named_Hero() == false )
		return;

	GameObjectClass *parent = hero->Get_Parent_Container_Object();
	while (parent && !parent->Behaves_Like(BEHAVIOR_PLANET))
	{
		parent = parent->Get_Parent_Container_Object();
	}

	if ((parent == NULL) || !parent->Behaves_Like(BEHAVIOR_PLANET))
	{
		return;
	}

	std::string heroname = *hero->Get_Type()->Get_Name();
	{
		Event_Debug_Printf("STORY - HERO DEFEATED %s\r\n",heroname.c_str());
	}
	Story_Event(STORY_DEFEAT_HERO,NULL,&heroname,parent);
}




/**************************************************************************************************
* StoryModeClass::Win_Space_Battle -- Win Space Battle event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:34PM JAC
**************************************************************************************************/
void StoryModeClass::Win_Space_Battle(GameObjectClass *planet, PlayerClass *player)
{
	assert(planet);

	{
		Event_Debug_Printf("STORY - %s WON SPACE BATTLE\r\n",player->Get_Faction()->Get_Name()->c_str());
	}
	StoryBaseFilter location = BASE_SPACE;
	Story_Event(STORY_WIN_BATTLES,player,&location,planet);

	if (planet) {
		if (player == PlayerList.Get_Local_Player()) {
			if (!GameModeManager.Is_Multiplayer_Mode()) {
				TheTutorial.Remove_Auto_Resolve_Prevention_At_Location(planet, false);
			}			
		}			
	}
}


void StoryModeClass::Lose_Space_Battle(GameObjectClass *planet, PlayerClass *player)
{
	{
		Event_Debug_Printf("STORY - %s LOST SPACE BATTLE\r\n",player->Get_Faction()->Get_Name()->c_str());
	}
	StoryBaseFilter location = BASE_SPACE;
	Story_Event(STORY_LOSE_BATTLES,player,&location,planet);
}




/**************************************************************************************************
* StoryModeClass::Win_Land_Battle -- Win Land Battle event
*
* In:		
*
* Out:	
*
*
* History: 07/21/2004 6:34PM JAC
**************************************************************************************************/
void StoryModeClass::Win_Land_Battle(GameObjectClass *planet, PlayerClass *player)
{
	assert(planet);
	
	{
		Event_Debug_Printf("STORY - %s WON LAND BATTLE\r\n",player->Get_Faction()->Get_Name()->c_str());
	}
	StoryBaseFilter location = BASE_GROUND;
	Story_Event(STORY_WIN_BATTLES,player,&location,planet);

	if (planet) {
		if (player == PlayerList.Get_Local_Player()) {
			if (!GameModeManager.Is_Multiplayer_Mode()) {
				TheTutorial.Remove_Auto_Resolve_Prevention_At_Location(planet, true);
			}			
		}			
	}
}


void StoryModeClass::Lose_Land_Battle(GameObjectClass *planet, PlayerClass *player)
{
	{
		Event_Debug_Printf("STORY - %s LOST LAND BATTLE\r\n",player->Get_Faction()->Get_Name()->c_str());
	}
	StoryBaseFilter location = BASE_GROUND;
	Story_Event(STORY_LOSE_BATTLES,player,&location,planet);
}





/**************************************************************************************************
* StoryModeClass::Tutorial_Click_UI -- Detect a click on a GUI element
*
* In:		
*
* Out:	
*
*
* History: 11/18/2004 3:09PM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Click_UI(const char *name )
{
	if (name)
	{
		char name_copy[256];
		assert(strlen(name) < 256);
		strcpy(name_copy,name);

		{
			Event_Debug_Printf("TUTORIAL - GUI item clicked on %s\r\n",name);
		}
		Story_Event(STORY_CLICK_GUI,NULL,name_copy,NULL);
	}
}




/**************************************************************************************************
* StoryModeClass::Tutorial_Select_Unit -- Detect when a unit is selected
*
* In:		
*
* Out:	
*
*
* History: 11/18/2004 3:09PM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Select_Unit(GameObjectClass * unit, int player_id)
{
	if (player_id == PlayerList.Get_Local_Player_ID())
	{
		{
			Event_Debug_Printf("TUTORIAL - Unit(s) selected\r\n");
		}

		std::string name_copy = *unit->Get_Type()->Get_Name();
		Story_Event(STORY_SELECT_UNIT,NULL,&name_copy,NULL);
	}

	//Display_Dialog_Box_Text("TEXT_TEST_TUTORIAL_TEXT");
}




/**************************************************************************************************
* StoryModeClass::Tutorial_Command_Unit -- Detect when a unit is given a command
*
* In:		
*
* Out:	
*
*
* History: 11/18/2004 3:10PM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Command_Unit(ReferenceListClass<GameObjectClass> * units, const Vector3 &pos)
{
	ReferenceListIterator<GameObjectClass> it(units);
	for ( it.First(); !it.Is_Done(); it.Next() ) 
	{
		GameObjectClass *object = it.Current_Object();
		assert( object != NULL );

		if (object->Get_Owner() == PlayerList.Get_Local_Player_ID())
		{
			std::string name_copy = *object->Get_Type()->Get_Name();
			{
				Event_Debug_Printf("TUTORIAL - Unit %s moved\r\n",name_copy.c_str());
			}
			Vector3 pos_copy = pos;
			Story_Event(STORY_COMMAND_UNIT,NULL,&name_copy,&pos_copy);
		}
	}
}




/**************************************************************************************************
* StoryModeClass::Tutorial_Guard_Unit -- Gaurd unit event has happened
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:18AM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Guard_Unit(ReferenceListClass<GameObjectClass> * units, const GameObjectClass *target_object)
{
	assert(target_object);
	std::string target_name = *target_object->Get_Type()->Get_Name();
	std::string name_copy;

	ReferenceListIterator<GameObjectClass> it(units);
	for ( it.First(); !it.Is_Done(); it.Next() ) 
	{
		GameObjectClass *object = it.Current_Object();
		assert( object != NULL );

		if (object->Get_Owner() == PlayerList.Get_Local_Player_ID())
		{
			name_copy = *object->Get_Type()->Get_Name();
			{
				Event_Debug_Printf("TUTORIAL - Unit %s guarding %s\r\n",name_copy.c_str(),target_name.c_str());
			}
			Story_Event(STORY_GUARD_UNIT,NULL,&name_copy,&target_name);
		}
	}
}




/**************************************************************************************************
* StoryModeClass::Tutorial_Unit_Arrives -- Detect when a unit arrives at its destination
*
* In:		
*
* Out:	
*
*
* History: 11/18/2004 3:10PM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Unit_Arrives(GameObjectClass *unit)
{
	if (unit->Get_Owner() == PlayerList.Get_Local_Player_ID())
	{
		std::string name_copy = *unit->Get_Type()->Get_Name();
		{
			Event_Debug_Printf("TUTORIAL - Unit %s arrived at destination\r\n",name_copy.c_str());
		}

		Story_Event(STORY_UNIT_ARRIVED,NULL,&name_copy,NULL);
	}
}





/**************************************************************************************************
* StoryModeClass::Tutorial_Full_Stop -- Full stop event has happened
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:18AM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Full_Stop(GameObjectClass *unit)
{
	if (unit->Get_Owner() == PlayerList.Get_Local_Player_ID())
	{
		std::string name_copy = *unit->Get_Type()->Get_Name();
		{
			Event_Debug_Printf("TUTORIAL - Unit %s arrived at destination\r\n",name_copy.c_str());
		}

		Story_Event(STORY_FULL_STOP,NULL,&name_copy,NULL);
	}
}





/**************************************************************************************************
* StoryModeClass::Begin_Space_Tactical -- A space tactical game has been started
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:19AM JAC
**************************************************************************************************/
void StoryModeClass::Begin_Space_Tactical(StoryModeClass *space_story)
{
	{
		Event_Debug_Printf("STORY - Starting a space battle\r\n");
	}
	Story_Event(STORY_SPACE_TACTICAL,NULL,space_story,TheTutorial.Get_Tactical_Planet());
}





/**************************************************************************************************
* StoryModeClass::Begin_Land_Tactical -- A land tactical game has been started
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:19AM JAC
**************************************************************************************************/
void StoryModeClass::Begin_Land_Tactical(StoryModeClass *land_story)
{
	{
		Event_Debug_Printf("STORY - Starting a land battle\r\n");
	}
	Story_Event(STORY_LAND_TACTICAL,NULL,land_story,TheTutorial.Get_Tactical_Planet());
}





/**************************************************************************************************
* StoryModeClass::Tutorial_Select_Planet -- A planet has been selected in galactic mode
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:19AM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Select_Planet(GameObjectClass *planet)
{
	if (planet)
	{
		{
			Event_Debug_Printf("TUTORIAL - Clicked on planet %s\r\n",planet->Get_Type()->Get_Name()->c_str());
		}
		Story_Event(STORY_SELECT_PLANET,NULL,planet,NULL);
	}
}



/**************************************************************************************************
* StoryModeClass::Tutorial_Zoom_In_Planet -- Player has zoomed into a planet in galactic
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:20AM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Zoom_In_Planet(GameObjectClass *planet)
{
	if (planet)
	{
		{
			Event_Debug_Printf("TUTORIAL - Zoomed in on planet %s\r\n",planet->Get_Type()->Get_Name()->c_str());
		}
		Story_Event(STORY_ZOOM_INTO_PLANET,NULL,planet,NULL);
	}
}





/**************************************************************************************************
* StoryModeClass::Tutorial_Zoom_Out_Planet -- Player has zoomed out of planet in galactic
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:20AM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Zoom_Out_Planet(GameObjectClass *planet)
{
	if (planet)
	{
		{
			Event_Debug_Printf("TUTORIAL - Zoomed out of planet %s\r\n",planet->Get_Type()->Get_Name()->c_str());
		}
		Story_Event(STORY_ZOOM_OUT_PLANET,NULL,planet,NULL);
	}
}





/**************************************************************************************************
* StoryModeClass::Tutorial_Generic -- Some events are triggered by matching a string
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:20AM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Generic(const char *name )
{
	if (name)
	{
		char name_copy[256];
		assert(strlen(name) < 256);
		strcpy(name_copy,name);

		{
			Event_Debug_Printf("TUTORIAL - Generic string based trigger - %s\r\n",name);
		}
		Story_Event(STORY_GENERIC,NULL,name_copy,NULL);
	}
}






/**************************************************************************************************
* StoryModeClass::Tutorial_Speech_Done -- Speech playback has finished
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:20AM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Speech_Done(const char *speech )
{
	if (speech)
	{
		char name_copy[256];
		assert(strlen(speech) < 256);
		strcpy(name_copy,speech);

		{
			Story_Debug_Printf("- Speech done - %s\r\n",speech);
		}
		Story_Event(STORY_SPEECH_DONE,NULL,name_copy,NULL);

		char text_name[128];
		sprintf(text_name,"TEXT_SPEECH_%s",speech);
		TheCommandBar.Remove_Tutorial_Text(text_name);
	}
}






/**************************************************************************************************
* StoryModeClass::Speech_Callback -- Callback used by speech manager to let story mode know speech has ended
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:21AM JAC
**************************************************************************************************/
void StoryModeClass::Speech_Callback(const SpeechEventClass *speech_event)
{
	Debug_Printf("STORY MODE - Speech finished callback for speech %s\n",speech_event->Get_Name().c_str());

	if (GameModeManager.Get_Active_Mode())
	{
		GameModeManager.Get_Active_Mode()->Get_Story_Mode().Tutorial_Speech_Done(speech_event->Get_Name().c_str());
	}
}







/**************************************************************************************************
* StoryModeClass::Replace_Variable -- Replace a string in all events with another string
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:21AM JAC
**************************************************************************************************/
void StoryModeClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	SubPlotListType::iterator plotptr;

	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *plot = plotptr->second;

		plot->Replace_Variable(var_name,new_name);
	}
}




/**************************************************************************************************
* StoryModeClass::Retreat -- A retreat has been started
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:21AM JAC
**************************************************************************************************/
void StoryModeClass::Retreat(PlayerClass *player)
{
	if (player)
	{
		{
			Event_Debug_Printf("STORY - Player/AI retreated\r\n");
		}
		Story_Event(STORY_RETREAT,NULL,player,NULL);
	}
}





/**************************************************************************************************
* StoryModeClass::Reinforce -- Reinforcements are being called in
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:22AM JAC
**************************************************************************************************/
void StoryModeClass::Reinforce(PlayerClass *player, const GameObjectClass *object)
{
	if (object)
	{
		const GameObjectTypeClass *object_type = object->Get_Type();

		if (object->Behaves_Like(BEHAVIOR_TRANSPORT))
		{
			// Transports are special cases
			TransportBehaviorClass *tbehavior = static_cast<TransportBehaviorClass *> (object->Get_Behavior(BEHAVIOR_TRANSPORT));
			object_type = tbehavior->Get_Dummy_Company_Object_Type(object);
		}

		std::string object_name = *object_type->Get_Name();
		{
			Event_Debug_Printf("STORY - Reinforcing with unit %s\r\n",object_name.c_str());
		}
		Story_Event(STORY_REINFORCE,player,&object_name,NULL);
	}
}







/**************************************************************************************************
* StoryModeClass::Tutorial_Continue_Tutorial -- Player has elected to continue tutorial
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:22AM JAC
**************************************************************************************************/
void StoryModeClass::Tutorial_Continue_Tutorial()
{
	Event_Debug_Printf("STORY EVENT - Load next tutorial\r\n");
	Tutorial_Generic("Continue_Tutorial");
}






/**************************************************************************************************
* StoryModeClass::Planet_Destroyed -- Planet has been destroyed (by Death Star) in galactic
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:22AM JAC
**************************************************************************************************/
void StoryModeClass::Planet_Destroyed(GameObjectClass *planet)
{
	assert(planet);

	Event_Debug_Printf("STORY EVENT - Planet %s destroyed\r\n",planet->Get_Type()->Get_Name()->c_str());
	Story_Event(STORY_PLANET_DESTROYED,NULL,planet,NULL);

	Remove_Destroyed_Planet(*planet->Get_Type()->Get_Name());
}




/**************************************************************************************************
* StoryModeClass::Remove_Destroyed_Planet -- Update plots so destroyed planet no longer referenced
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:23AM JAC
**************************************************************************************************/
void StoryModeClass::Remove_Destroyed_Planet(const std::string &planet_name)
{
	SubPlotListType::iterator plotptr;

	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *plot = plotptr->second;

		plot->Planet_Destroyed(planet_name);
	}
}




/**************************************************************************************************
* StoryModeClass::Attack_Hardpoint -- Player has attacked a hardpoint
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:23AM JAC
**************************************************************************************************/
void StoryModeClass::Attack_Hardpoint(const std::string *name)
{
	assert(name);

	Event_Debug_Printf("STORY EVENT - Attack hardpoint %s\r\n",name->c_str());

	std::string name_copy = *name;
	Story_Event(STORY_ATTACK_HARDPOINT,NULL,&name_copy,NULL);
}






/**************************************************************************************************
* StoryModeClass::Check_Proximity -- Trigger based on proximity of player units to a target
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:23AM JAC
**************************************************************************************************/
void StoryModeClass::Check_Proximity()
{
	int player_id = PlayerList.Get_Local_Player_ID();

	// (gth) 9/3/2005 switching back to the selection_list.  VTune shows these per-frame Find_Objects calls as big spikes
	// so we need to eliminate them.  Testing shows that pre-placed units do seem to work ok now.
	// (jason) Get_Selectable_Objects call fails to include units placed on the map by the designers
	// Get selectable objects should be more efficient
	//const DynamicVectorClass<GameObjectClass *> *object_list = GAME_OBJECT_MANAGER.Find_Objects( BEHAVIOR_SELECTABLE, player_id );
	ReferenceListClass<GameObjectClass> *selection_list = GAME_OBJECT_MANAGER.Get_Selectable_Objects();
	ReferenceListIterator<GameObjectClass> it(selection_list);

	//for (int i=0; i<object_list->Size(); i++)
	for (; !it.Is_Done(); it.Next())
	{
		//GameObjectClass *object = (*object_list)[i];
		GameObjectClass *object = it.Current_Object();

		// Don't need to do this if check since the Find_Objects already screens for player ownership
		// (gth) not using Find_Objects now
		if ((object->Get_Owner() == player_id) && (!object->Behaves_Like(BEHAVIOR_PROJECTILE)))
		{
			Story_Event(STORY_UNIT_PROXIMITY,PlayerList.Get_Local_Player(),object,NULL);
		}
	}
}






/**************************************************************************************************
* StoryModeClass::Load -- Load saved data
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:23AM JAC
**************************************************************************************************/
bool StoryModeClass::Load(ChunkReaderClass *reader)
{
	assert( reader != NULL );

	std::string plot_name;
	StorySubPlotClass *plot = NULL;
	ParentMode = NULL;
	SubGameModeType mode = SUB_GAME_MODE_INVALID;

	bool ok = true;

	Flags.clear();
	Remove_Plots();

	std::wstring displayText;

	while (reader->Open_Chunk())
	{
		switch ( reader->Cur_Chunk_ID() )
		{
			case STORY_MODE_MICRO_DATA_CHUNK:
				{
					while (reader->Open_Micro_Chunk())
					{
						switch ( reader->Cur_Micro_Chunk_ID() )
						{
							READ_MICRO_CHUNK_THIS_PTR		(STORY_MODE_THIS_ID_CHUNK);
							READ_MICRO_CHUNK					(STORY_MODE_PARENT_TYPE_CHUNK,			mode);
							READ_MICRO_CHUNK					(STORY_MODE_CURRENT_OBJECTIVE_CHUNK,	CurrentObjective);
							default: assert(false); break;	// Unknown Chunk
						}
						reader->Close_Micro_Chunk();
					}
					assert(mode != SUB_GAME_MODE_INVALID);
					GameModeClass *game_mode = GameModeManager.Get_Game_Mode_By_Sub_Type(mode);
					ParentMode = game_mode;
					break;
				}

			case STORY_MODE_ALL_PLOTS_CHUNK:
				{
					int player_id = PlayerClass::INVALID_PLAYER_ID;
					std::string plot_file_name;
					while (reader->Open_Micro_Chunk())
					{
						switch (reader->Cur_Micro_Chunk_ID())
						{
							READ_MICRO_CHUNK			(STORY_MODE_PLAYER_PLOT_FILE_PLAYER_CHUNK,	player_id);
							READ_MICRO_CHUNK_STRING	(STORY_MODE_PLAYER_PLOT_FILE_NAME_CHUNK,		plot_file_name);

						default:
							ok = false;
							assert(false);
							break;
						}
						reader->Close_Micro_Chunk();
					}

					PlayerClass *player = PlayerList.Get_Player_By_ID(player_id);
					ENFORCED_IF(player)
					{
						Load_Plots(plot_file_name, player);
					}
				}
				break;



			case STORY_MODE_PLOT_DATA_CHUNK:
				{
					while (reader->Open_Micro_Chunk())
					{
						switch ( reader->Cur_Micro_Chunk_ID() )
						{
							READ_MICRO_CHUNK_STRING(STORY_MODE_PLOT_NAME_CHUNK, plot_name);
							default: assert(false); break;	// Unknown Chunk
						}
						reader->Close_Micro_Chunk();
					}
				}
				break;

			case STORY_MODE_PLOT_CHUNK:
				plot = Get_Sub_Plot(plot_name);
				if (plot)
				{
					ok &= plot->Load(reader);
				}
				else
				{
					std::string lua_script;
					plot = Load_Single_Plot(plot_name,lua_script,true,PlayerList.Get_Local_Player());
					assert(plot);
					ok &= plot->Load(reader);
				}
				break;

			case STORY_MODE_OBJECTIVE_DISPLAY_NON_MICRO_TEXT_CHUNK:
				// MLL: Since the display text can be greater than a micro chunk, use a regular chunk. 
				reader->Read_String(displayText);
				break;

			case STORY_MODE_ALL_OBJECTIVES_CHUNK:
				{
					ObjectiveStruct objective;
					objective.DisplayText = displayText;

					while (reader->Open_Micro_Chunk())
					{
						switch (reader->Cur_Micro_Chunk_ID())
						{
							READ_MICRO_CHUNK_STRING	(STORY_MODE_OBJECTIVE_TEXT_CHUNK,				objective.ObjectiveText);
							READ_MICRO_CHUNK_STRING	(STORY_MODE_OBJECTIVE_DISPLAY_TEXT_CHUNK,		objective.DisplayText);
							READ_MICRO_CHUNK			(STORY_MODE_OBJECTIVE_STATUS_CHUNK,				objective.Status);
							READ_MICRO_CHUNK			(STORY_MODE_OBJECTIVE_SUGGESTION_CHUNK,		objective.Suggestion);
							READ_MICRO_CHUNK			(STORY_MODE_OBJECTIVE_NUMBER_CHUNK,				objective.ObjectiveNumber);

						default:
							ok = false;
							assert(false);
							break;
						}
						reader->Close_Micro_Chunk();
					}
					

					ObjectiveList.Add(objective);
				}
				break;

			READ_STL_HASHMAP_NP_P			(STORY_MODE_SUB_PLOT_LIST_CHUNK, 		SubPlots);
			READ_STL_HASHMAP_NP_NP			(STORY_MODE_VARIABLE_LIST_CHUNK, 		Flags);

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}

	return( ok );
}




/**************************************************************************************************
* StoryModeClass::Save -- Save story mode data
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:24AM JAC
**************************************************************************************************/
bool StoryModeClass::Save(ChunkWriterClass *writer)
{
	assert( writer != NULL );

	bool ok = true;

	int mode;
	if (ParentMode != NULL)
	{
		mode = ParentMode->Get_Sub_Type();
	}
	else
	{
		mode = SUB_GAME_MODE_INVALID;
	}

	ok &= writer->Begin_Chunk(STORY_MODE_MICRO_DATA_CHUNK);
		WRITE_MICRO_CHUNK_THIS_PTR			(STORY_MODE_THIS_ID_CHUNK);
		WRITE_MICRO_CHUNK						(STORY_MODE_PARENT_TYPE_CHUNK,			mode);
		WRITE_MICRO_CHUNK						(STORY_MODE_CURRENT_OBJECTIVE_CHUNK,	CurrentObjective);
	ok &= writer->End_Chunk();

	for (unsigned int i = 0; i < Plots.size(); ++i)
	{
		ok &= writer->Begin_Chunk(STORY_MODE_ALL_PLOTS_CHUNK);
			WRITE_MICRO_CHUNK			(STORY_MODE_PLAYER_PLOT_FILE_PLAYER_CHUNK,	Plots[i].first);
			WRITE_MICRO_CHUNK_STRING(STORY_MODE_PLAYER_PLOT_FILE_NAME_CHUNK,		Plots[i].second);
		ok &= writer->End_Chunk();
	}

	SubPlotListType::iterator subptr;
	for (subptr = SubPlots.begin(); subptr != SubPlots.end(); subptr++)
	{
		ok &= writer->Begin_Chunk(STORY_MODE_PLOT_DATA_CHUNK);
			WRITE_MICRO_CHUNK_STRING(STORY_MODE_PLOT_NAME_CHUNK,subptr->second->Get_Name());
		ok &= writer->End_Chunk();
		ok &= writer->Begin_Chunk(STORY_MODE_PLOT_CHUNK);
			ok &= subptr->second->Save(writer);
		ok &= writer->End_Chunk();
	}

	for (int i=0; i<ObjectiveList.Size(); i++)
	{
		// MLL: The read code assumes that this comes before the rest of the micro chunks.
		WRITE_CHUNK_STRING(STORY_MODE_OBJECTIVE_DISPLAY_NON_MICRO_TEXT_CHUNK,ObjectiveList[i].DisplayText);

		ok &= writer->Begin_Chunk(STORY_MODE_ALL_OBJECTIVES_CHUNK);
			WRITE_MICRO_CHUNK_STRING(STORY_MODE_OBJECTIVE_TEXT_CHUNK,				ObjectiveList[i].ObjectiveText);
			// MLL: Switched to a non micro chunk because some descriptions were too big in non-English languages.
			//WRITE_MICRO_CHUNK_STRING(STORY_MODE_OBJECTIVE_DISPLAY_TEXT_CHUNK,		ObjectiveList[i].DisplayText);
			WRITE_MICRO_CHUNK			(STORY_MODE_OBJECTIVE_STATUS_CHUNK,				ObjectiveList[i].Status);
			WRITE_MICRO_CHUNK			(STORY_MODE_OBJECTIVE_SUGGESTION_CHUNK,		ObjectiveList[i].Suggestion);
			WRITE_MICRO_CHUNK			(STORY_MODE_OBJECTIVE_NUMBER_CHUNK,				ObjectiveList[i].ObjectiveNumber);
		ok &= writer->End_Chunk();

	}

	WRITE_STL_HASHMAP_NP_P			(STORY_MODE_SUB_PLOT_LIST_CHUNK, 		SubPlots);
	WRITE_STL_HASHMAP_NP_NP			(STORY_MODE_VARIABLE_LIST_CHUNK, 		Flags);

	return (ok);
}







/**************************************************************************************************
* StoryModeClass::Dump_Status -- Debug output
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:24AM JAC
**************************************************************************************************/
void StoryModeClass::Dump_Status()
{
	SubPlotListType::iterator plotptr;
	StoryFlagListType::iterator varptr;

	// Print out all the variables and their values
	Story_Debug_Printf("########################################################\r\n");
	Story_Debug_Printf("\tFlags :\r\n");
	for (varptr = Flags.begin(); varptr != Flags.end(); varptr++)
	{
		FlagStruct *flag = &varptr->second;
		CRCValue crc = CRCClass::Calculate_CRC( flag->Name, strlen(flag->Name) );
		Story_Debug_Printf("\t\t%s = %d, CRC %u\r\n",flag->Name,flag->Value,crc);
	}
	Story_Debug_Printf("\r\n");

	// Print out all of the plots
	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		subplot->Dump_Status();
	}
}






/**************************************************************************************************
* StoryModeClass::Remove_Plot -- Remove an unneeded plot (used by random story generator)
*
* In:		
*
* Out:	
*
*
* History: 04/26/2005 10:24AM JAC
**************************************************************************************************/
void StoryModeClass::Remove_Plot(CRCValue crc)
{
	if (SubPlots.find(crc) != SubPlots.end())
	{
		SubPlots.erase(crc);
	}
}






void StoryModeClass::Set_Flag(const char *name, int value)
{
	FlagStruct var;

	// Just make all variables upper case to remove any case mismatching
	char new_name[256];
	strcpy(new_name,name);
	_strupr(new_name);

	strncpy(var.Name, new_name, sizeof(var.Name)-1);
	var.Name[sizeof(var.Name)-1] = 0;
	var.Value = value;

	CRCValue crc = CRCClass::Calculate_CRC( var.Name, strlen(var.Name) );
	Flags[crc] = var;

	Story_Debug_Printf("Flag %s set to value %d, CRC %u\r\n",new_name,value,crc);
}






int StoryModeClass::Get_Flag(const char *name)
{
	assert(name);

	// Just make all variables upper case to remove any case mismatching
	char new_name[256];
	strcpy(new_name,name);
	_strupr(new_name);

	CRCValue crc = CRCClass::Calculate_CRC( new_name, strlen(new_name) );
	StoryFlagListType::iterator varptr;

	varptr = Flags.find(crc);
	if (varptr != Flags.end())
	{
		FlagStruct *var = &varptr->second;
		return (var->Value);
	}
	else
	{
		// What to return for an undefined variable???
		return (UNDEFINED_STORY_FLAG);
	}
}





int StoryModeClass::Increment_Flag(const char *name, int increment)
{
	assert(name);

	// Just make all variables upper case to remove any case mismatching
	char new_name[256];
	strcpy(new_name,name);
	_strupr(new_name);

	CRCValue crc = CRCClass::Calculate_CRC( new_name, strlen(new_name) );
	StoryFlagListType::iterator varptr;

	varptr = Flags.find(crc);
	if (varptr != Flags.end())
	{
		FlagStruct *var = &varptr->second;
		var->Value += increment;

		Story_Debug_Printf("Flag %s incremented by %d to new value %d\r\n",var->Name,increment,var->Value);
		return (var->Value);
	}
	else
	{
		Story_Debug_Printf("Flag %s undefined.  CRC %u\r\n",new_name,crc);
		// What to return for an undefined variable???
		return (UNDEFINED_STORY_FLAG);
	}
}







void StoryModeClass::Load_Tactical_Map(GameObjectClass *planet, StoryBaseFilter *location)
{
	if (planet && location)
	{
		Story_Event(STORY_LOAD_TACTICAL_MAP,NULL,planet,location);
	}
}




void StoryModeClass::Fleet_Bounced(GameObjectClass *planet, GameObjectClass *fleet)
{
	if (planet && fleet)
	{
		Story_Event(STORY_FLEET_BOUNCED, fleet->Get_Owner_Player(), planet, fleet);
	}
}






void StoryModeClass::Invasion_Bounced(GameObjectClass *planet)
{
	if (planet)
	{
		Story_Event(STORY_INVASION_BOUNCED, NULL, planet, NULL);
	}
}





void StoryModeClass::Structure_Captured(GameObjectClass *object, PlayerClass *new_owner)
{
	if (!object || !new_owner)
	{
		return;
	}

	const FactionClass *new_faction = new_owner->Get_Faction();

	Story_Event(STORY_CAPTURE_STRUCTURE,NULL,object,(void *)new_faction);
}






bool StoryModeClass::Check_Special_Land_Tactical_Map(GameObjectClass *hero, GameObjectClass *planet, bool check_land_only)
{
	SubPlotListType::iterator plotptr;
	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		assert(subplot);

		if (subplot->Is_Active())
		{
			bool result = subplot->Check_Special_Land_Tactical_Map(hero,planet,check_land_only);
			if (result == true)
			{
				return (true);
			}
		}
	}

	return (false);
}



/**************************************************************************************************
* Event_Debug_Print -- Standard debug print function.
*
* In:  text string
*
* Out: nothing
*
*
*
* History: 6/1/2005 JAC
**************************************************************************************************/
void Event_Debug_Printf(char *text, ...)
{
	if (TheTutorial.Is_Story_Logging())
	{
		//assert(text != NULL);
		char _temp_buffer[1024];
		char *str_ptr = _temp_buffer;

		int len = strlen(text);

		if (len > 256) {
			str_ptr = (char*)_alloca(min(len*3, 64*1024));
		}

		if (text != NULL) {

			/*
			** Format the string.
			*/
			va_list va;
			va_start(va, text);
			vsprintf(str_ptr, text, va);
			va_end(va);

			len = strlen(str_ptr);

			if (TheEventLogWindowPtr)
			{
				TheEventLogWindowPtr->Debug_Log_Callback(str_ptr,len);
			}
			Debug_Printf(str_ptr);
		}
	}
}


/**************************************************************************************************
* Event_Debug_Print -- Standard debug print function.
*
* In:  text string
*
* Out: nothing
*
*
*
* History: 6/1/2005 JAC
**************************************************************************************************/
void Event_Debug_Printf(const char *text, ...)
{
	if (TheTutorial.Is_Story_Logging())
	{
		//assert(text != NULL);
		static char _temp_buffer[1024];
		char *str_ptr = _temp_buffer;

		int len = strlen(text);

		if (len > 256) {
			str_ptr = (char*)_alloca(min(len*3, 64*1024));
		}

		if (text != NULL) {

			/*
			** Format the string.
			*/
			va_list va;
			va_start(va, text);
			vsprintf(str_ptr, text, va);
			va_end(va);

			len = strlen(str_ptr);

			if (TheEventLogWindowPtr)
			{
				TheEventLogWindowPtr->Debug_Log_Callback(str_ptr,len);
			}
			Debug_Printf(str_ptr);
		}
	}
}




void StoryModeClass::Trigger_Event(const char *event_name)
{
	SubPlotListType::iterator plotptr;

	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		subplot->Trigger_Event(event_name);
	}
}







void StoryModeClass::Disable_Event(const char *event_name, bool onoff)
{
	SubPlotListType::iterator plotptr;

	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		subplot->Disable_Event(event_name,onoff);
	}
}







void StoryModeClass::Check_Destroyed()
{
	Story_Event(STORY_CHECK_DESTROYED,NULL,NULL,NULL);
}





void StoryModeClass::Set_Land_Forces(ObjectIDType *forces, int count)
{
	if ((forces == NULL) || (count == 0))
	{
		LandForces.Resize(0);
	}
	else
	{
		for (int i=0; i<count; i++)
		{
			LandForces.Add(forces[i]);
		}
	}
}






void StoryModeClass::Speech_Killed(DynamicVectorClass<std::string> *killed_names)
{
	for (int i=0; i<killed_names->Size(); i++)
	{
		std::string name = (*killed_names)[i];
		Tutorial_Speech_Done(name.c_str());
	}
}





bool StoryModeClass::Check_Planet_Entry_Restrictions(GameObjectClass *fleet, GameObjectClass *planet)
{
	// Stealth, non-combat fleets can get through restrictions except in the tutorials
	if (fleet->Is_Stealth_Object() && fleet->Skips_Space_Combat() && !PlayerList.Get_Local_Player()->Get_Is_Tutorial_Player())
	{
		bool no_heroes = true;

		FleetBehaviorClass *behave = (FleetBehaviorClass *)fleet->Get_Behavior(BEHAVIOR_FLEET);
		for (int i=0; i<(int)behave->Get_Contained_Objects_Count(); i++)
		{
			GameObjectClass *unit = behave->Get_Contained_Object(fleet, i);
			if (unit->Get_Original_Object_Type()->Is_Named_Hero())
			{
				no_heroes = false;
				break;
			}
		}

		if (no_heroes)
		{
			return (true);
		}
	}

	SubPlotListType::iterator plotptr;
	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		assert(subplot);

		if (subplot->Is_Active())
		{
			bool result = subplot->Check_Planet_Entry_Restrictions(fleet, planet);
			if (result == true)
			{
				return (true);
			}
		}
	}

	return (false);
}





bool StoryModeClass::Check_Special_Space_Tactical_Map(GameObjectClass *planet, GameObjectClass *hero)
{
	SubPlotListType::iterator plotptr;
	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		assert(subplot);

		if (subplot->Is_Active())
		{
			bool result = subplot->Check_Special_Space_Tactical_Map(planet,hero);
			if (result == true)
			{
				return (true);
			}
		}
	}

	return (false);
}







void StoryModeClass::Victory(PlayerClass *player)
{
	Story_Event(STORY_VICTORY,NULL,(void *)player->Get_Faction(),NULL);

	// Check to see if we should tell the galactic level script if the player lost a tactical mission
	if (!PlotName.empty() && (player != PlayerList.Get_Local_Player()))
	{
		GameModeClass *galactic = GameModeManager.Get_Parent_Game_Mode(GameModeManager.Get_Active_Mode());
		if (galactic && (galactic->Get_Sub_Type() == SUB_GAME_MODE_GALACTIC))
		{
			galactic->Get_Story_Mode().Mission_Lost(PlotName);
		}
	}
}







void StoryModeClass::Mission_Lost(const std::string &plot_name)
{
	// This event will be called from the tactical level but applied to the galactic level script
	// The event will be queued up and executed when the game returns to galactic.  For this 
	// reason, we need to keep a copy of the string at the galactic level since the tactical
	// level StoryMode will be deleted by the time the event is processed
	MissionPlotName = plot_name;
	Story_Event(STORY_MISSION_LOST,NULL,(void *)MissionPlotName.c_str(),NULL);
}






void StoryModeClass::Add_Objective(const std::string &objective, const std::wstring *display_text, bool suggestion, int index)
{
	if (objective.empty())
	{
		return;
	}

	ObjectiveStruct obj_data;

	obj_data.ObjectiveText = objective;
	obj_data.Status = OBJECTIVE_NOT_COMPLETE;
	obj_data.Suggestion = suggestion;
	obj_data.ObjectiveNumber = CurrentObjective++;

	if (display_text != NULL)
	{
		// The unicode text has already been built
		obj_data.DisplayText = *display_text;
	}
	else
	{
		// Create the unicode text string
		TheGameText.Get(objective,obj_data.DisplayText);
	}

	if ((index < 0) || (index > ObjectiveList.Size()))
	{
		ObjectiveList.Add(obj_data);
	}
	else
	{
		ObjectiveList.Insert(obj_data,index);
	}

	TheCommandBar.Rebuild_Objectives();
}





int StoryModeClass::Remove_Objective(const std::string &objective, bool rebuild)
{
	int index = -1;

	for (int i=0; i<ObjectiveList.Size(); i++)
	{
		if (ObjectiveList[i].ObjectiveText == objective)
		{
			ObjectiveList.Delete_By_Index(i);
			// There are cases when we don't want to rebuild objectives right away
			if (rebuild)
			{
				TheCommandBar.Rebuild_Objectives();
			}
			index = i;
			return (index);
		}
	}

	return (index);
}





void StoryModeClass::Remove_All_Objectives()
{
	ObjectiveList.Truncate();
	TheCommandBar.Rebuild_Objectives();
}






void StoryModeClass::Replace_Objective(const std::string &old_objective, const std::string &new_objective)
{
	#if 0
	// Find the position of the old objective we're removing
	int old_index = Remove_Objective(old_objective,false);
	// Insert the new objective where the old one was
	Add_Objective(new_objective,NULL,false,old_index);
	#else
	// Instead of removing the old and adding the new, try just replacing the text string
	for (int i=0; i<ObjectiveList.Size(); i++)
	{
		if (ObjectiveList[i].ObjectiveText == old_objective)
		{
			ObjectiveList[i].ObjectiveText = new_objective;
			// Just in case, flag this as incomplete.  The script can always flag it as complete right away if needed
			ObjectiveList[i].Status = OBJECTIVE_NOT_COMPLETE;
			ObjectiveList[i].DisplayText = TheGameText.Get(new_objective);     
			TheCommandBar.Rebuild_Objectives();
			return;
		}
	}
	#endif
}






void StoryModeClass::Objective_Completed(const std::string &objective)
{
	for (int i=0; i<ObjectiveList.Size(); i++)
	{
		if (ObjectiveList[i].ObjectiveText == objective)
		{
			ObjectiveList[i].Status = OBJECTIVE_COMPLETE;
			TheCommandBar.Rebuild_Objectives(true);
			return;
		}
	}
}



void StoryModeClass::Objective_Failed(const std::string &objective)
{
	for (int i=0; i<ObjectiveList.Size(); i++)
	{
		if (ObjectiveList[i].ObjectiveText == objective)
		{
			ObjectiveList[i].Status = OBJECTIVE_FAILED;
			TheCommandBar.Rebuild_Objectives(true);
			return;
		}
	}
}



ObjectiveStatusEnum StoryModeClass::Objective_Status(const std::string &objective)
{
	for (int i=0; i<ObjectiveList.Size(); i++)
	{
		if (ObjectiveList[i].ObjectiveText == objective)
			return ObjectiveList[i].Status;
	}

	return OBJECTIVE_DOESNT_EXIST;
}




void StoryModeClass::Trigger_All_Speech_Done_Events()
{
	SubPlotListType::iterator plotptr;
	for (plotptr = SubPlots.begin(); plotptr != SubPlots.end(); plotptr++)
	{
		StorySubPlotClass *subplot = plotptr->second;
		assert(subplot);

		if (subplot->Is_Active())
		{
			subplot->Speech_Killed();
		}
	}
}




void StoryModeClass::Load_Default_Sandbox_Script(PlayerClass *defender)
{
	if (ParentMode == NULL)
	{
		return;
	}

	std::string plot_name, lua_script;

	PlayerClass *player = PlayerList.Get_Local_Player();

	if ((defender == player) || (ParentMode->Get_Sub_Type() == SUB_GAME_MODE_SPACE))
	{
		plot_name = "sandbox_standard.xml";
	}
	else
	{
		const GameObjectTypeClass *object_type = TheTutorial.Get_Sandbox_Random_Object();
		if ( object_type == NULL )
		{
			plot_name = "sandbox_standard.xml";
		}
		else
		{
			if (object_type->Behaves_Like(BEHAVIOR_CAPTURE_POINT))
			{
				plot_name = "sandbox_land_capture.xml";
			}
			else
			{
				plot_name = "sandbox_land_destroy.xml";
			}
		}
	}

	// Load the sandbox plot and add it to the plot list
	SandboxPlot = Load_Single_Plot(plot_name,lua_script,true,player);
	SandboxDefender = defender;
}







void StoryModeClass::Get_Sandbox_Primary_Objective(std::string *win_text, PlayerClass *defender, PlayerClass *player)
{
	std::string location = "LAND";

	if (ParentMode->Get_Sub_Type() == SUB_GAME_MODE_SPACE)
	{
		location = "SPACE";
	}

	if (defender == player)
	{
		*win_text = "TEXT_TACTICAL_PRIMARY_" + location + "_DEFEND";
	}
	else
	{
		*win_text = "TEXT_TACTICAL_PRIMARY_" + location + "_ATTACK";
	}
}








void StoryModeClass::Set_Sandbox_Objectives()
{
	if (SandboxPlot == NULL)
	{
		Story_Debug_Printf("STORY MODE - Unable to set sandbox objectives due to missing subplot.\r\n");
		return;
	}

	std::string win_text, secondary_text;
	std::vector<std::string> object_name, param2, win_faction;
	PlayerClass *player = PlayerList.Get_Local_Player();
	std::string good_name = TheGameConstants.Get_Good_Side_Name();
	std::string evil_name = TheGameConstants.Get_Evil_Side_Name();
	std::string corrupt_name = TheGameConstants.Get_Corrupt_Side_Name();
	bool good_side = (*player->Get_Faction()->Get_Name() == good_name);
	bool evil_side = (*player->Get_Faction()->Get_Name() == evil_name);
	bool corrupt_side = (*player->Get_Faction()->Get_Name() == corrupt_name);
	bool capture = true;
	bool land_mode = true;
	bool good_hero_found = false, evil_hero_found = false, corrupt_hero_found = false;
	std::string good_leader_name = TheGameConstants.Get_Good_Side_Leader_Name();
	std::string evil_leader_name = TheGameConstants.Get_Evil_Side_Leader_Name();
	const std::vector<std::string> corrupt_leader_name = TheGameConstants.Get_Corrupt_Side_Leader_Name();
	const GameObjectTypeClass *good_hero_type = GameObjectTypeManager.Find_Object_Type(good_leader_name);
	const GameObjectTypeClass *evil_hero_type = GameObjectTypeManager.Find_Object_Type(evil_leader_name);
	std::string protect_text, kill_text;
	std::vector<std::string> protect_name;
	std::vector<std::string> kill_name;
	std::vector<std::string> o_param;
	const ConflictInfoStruct *conflict = NULL;

	if (ParentMode->Get_Sub_Type() == SUB_GAME_MODE_SPACE)
	{
		// Get the conflict info
		conflict = GameModeManager.Get_Parent_Game_Mode(ParentMode)->Get_Object_Manager().Get_Space_Conflict_Info();

		// Find any hero leaders in space tactical
		land_mode = false;

		const DynamicVectorClass<GameObjectClass *> *transport_list = GAME_OBJECT_MANAGER.Find_Objects(BEHAVIOR_TRANSPORT);

		for (int i=0; i<transport_list->Size(); i++)
		{
			GameObjectClass *transport = (*transport_list)[i];
			if (transport)
			{
				DynamicVectorClass<GameObjectClass *> *cargo = transport->Get_Transport_Contents();
				for (int i=0; i<cargo->Size(); i++)
				{
					const GameObjectTypeClass *object_type = (*cargo)[i]->Get_Type();

					if (object_type == good_hero_type)
					{
						good_hero_found = true;
					}
					else if (object_type == evil_hero_type)
					{
						evil_hero_found = true;
					}
					else
					{
						// So many versions of the corrupt hero!
						for (unsigned int j=0; j<corrupt_leader_name.size(); j++)
						{
							std::string leader_name = corrupt_leader_name[j];
							const GameObjectTypeClass *corrupt_hero_type = GameObjectTypeManager.Find_Object_Type(leader_name);
							if (corrupt_hero_type && (object_type == corrupt_hero_type))
							{
								corrupt_hero_found = true;
								break;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		// Get the conflict info
		conflict = GameModeManager.Get_Parent_Game_Mode(ParentMode)->Get_Object_Manager().Get_Land_Invasion_Info();

		// Find the secondary objective
		const GameObjectTypeClass *object_type = TheTutorial.Get_Sandbox_Random_Object();
		if (object_type)
		{
			if (object_type->Behaves_Like(BEHAVIOR_CAPTURE_POINT))
			{
				capture = true;
				param2.push_back(*player->Get_Faction()->Get_Name());
			}
			else
			{
				capture = false;
				std::string temp_str = "1";
				param2.push_back(temp_str);
			}

			object_name.push_back(*object_type->Get_Name());
			secondary_text = object_type->Get_Secondary_Objective();
		}

		// Find any hero leaders
		// Good side leader doesn't appear in land mode
		#if 0
		const DynamicVectorClass<GameObjectClass *> *good_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(good_hero_type);
		if (good_list->Size() > 0)
		{
			good_hero_found = true;
		}
		#endif

		const DynamicVectorClass<GameObjectClass *> *evil_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(evil_hero_type);

		if (evil_list->Size() > 0)
		{
			evil_hero_found = true;
		}

		for (unsigned int j=0; j<corrupt_leader_name.size(); j++)
		{
			std::string leader_name = corrupt_leader_name[j];
			const GameObjectTypeClass *corrupt_hero_type = GameObjectTypeManager.Find_Object_Type(leader_name);
			const DynamicVectorClass<GameObjectClass *> *corrupt_list = GAME_OBJECT_MANAGER.Find_All_Objects_Of_Type(corrupt_hero_type);

			if (corrupt_list->Size() > 0)
			{
				corrupt_hero_found = true;
				break;
			}
		}
	}

	// Search for hero leaders
	if (good_hero_found || evil_hero_found || corrupt_hero_found)
	{
		bool kill_good = false;
		bool kill_evil = false;
		bool kill_corrupt = false;

		// Determine whether there is a defend leader victory condition
		if (conflict)
		{
			int attacker_id = conflict->InvadingPlayerID;
			int defender_id = conflict->DefendingPlayerID;
			int local_id = PlayerList.Get_Local_Player_ID();
			const VictoryConditionClass *victory = NULL;

			// We want to see if the player we're fighting against will win if our leader is killed
			if (local_id == attacker_id)
			{
				victory = ParentMode->Get_Victory_Monitor()->Get_Victory_Condition(VICTORY_SUB_TACTICAL_ENEMY_LEADER,defender_id);
			}
			else
			{
				victory = ParentMode->Get_Victory_Monitor()->Get_Victory_Condition(VICTORY_SUB_TACTICAL_ENEMY_LEADER,attacker_id);
			}

			if (victory)
			{
				const std::vector<std::string> hero_strings = victory->Get_String_Params();
				// If there are no strings, then all hero deaths will result in end of game
				if (hero_strings.size() == 0)
				{
					kill_good = true;
					kill_evil = true;
					kill_corrupt = true;
				}
				else
				{
					// Search through the strings to see which hero death triggers the victory
					for (unsigned int i=0; i<hero_strings.size(); i++)
					{
						if (good_hero_found && !kill_good && (hero_strings[i] == good_leader_name))
						{
							kill_good = true;
						}
						else if (evil_hero_found && !kill_evil && (hero_strings[i] == evil_leader_name))
						{
							kill_evil = true;
						}
						else if (corrupt_hero_found && !kill_corrupt)
						{
							for (unsigned int j=0; j<corrupt_leader_name.size(); j++)
							{
								if (corrupt_leader_name[j] == hero_strings[i])
								{
									kill_corrupt = true;
									break;
								}
							}
						}
					}
				}
			}
		}

		std::string temp_str = "1";
		o_param.push_back(temp_str);

		if (good_side)
		{
			if (good_hero_found && kill_good)
			{
				protect_text = "TEXT_TACTICAL_PRIMARY_PROTECT_GOOD_LEADER";
				temp_str = good_leader_name;
				protect_name.push_back(temp_str);
			}

			// Only 1 of the following 2 should be found
			if (evil_hero_found)
			{
				kill_text = "TEXT_TACTICAL_PRIMARY_KILL_EVIL_LEADER";
				temp_str = evil_leader_name;
				kill_name.push_back(temp_str);
			}

			if (corrupt_hero_found)
			{
				kill_text = "TEXT_TACTICAL_PRIMARY_KILL_CORRUPT_LEADER";
				temp_str = corrupt_leader_name[0];
				kill_name.push_back(temp_str);
			}
		}
		else if (evil_side)
		{
			if (evil_hero_found && kill_evil)
			{
				protect_text = "TEXT_TACTICAL_PRIMARY_PROTECT_EVIL_LEADER";
				temp_str = evil_leader_name;
				protect_name.push_back(temp_str);
			}

			// Only 1 of the following 2 should be found
			if (good_hero_found)
			{
				kill_text = "TEXT_TACTICAL_PRIMARY_KILL_EVIL_LEADER";
				temp_str = good_leader_name;
				kill_name.push_back(temp_str);
			}

			if (corrupt_hero_found)
			{
				kill_text = "TEXT_TACTICAL_PRIMARY_KILL_CORRUPT_LEADER";
				temp_str = corrupt_leader_name[0];
				kill_name.push_back(temp_str);
			}
		}
		else
		{
			if (corrupt_hero_found && kill_corrupt)
			{
				// Corrupt is misspelled in XLS file, so just carry it over here
				protect_text = "TEXT_TACTICAL_PRIMARY_PROTECT_CORUPT_LEADER";
				temp_str = corrupt_leader_name[0];
				protect_name.push_back(temp_str);
			}

			// Only 1 of the following 2 should be found
			if (good_hero_found)
			{
				kill_text = "TEXT_TACTICAL_PRIMARY_KILL_EVIL_LEADER";
				temp_str = good_leader_name;
				kill_name.push_back(temp_str);
			}

			if (evil_hero_found)
			{
				kill_text = "TEXT_TACTICAL_PRIMARY_KILL_EVIL_LEADER";
				temp_str = evil_leader_name;
				kill_name.push_back(temp_str);
			}
		}
	}


	// Get the win objective
	StoryEventClass *event = SandboxPlot->Get_Event("Sandbox_Objective_1");
	if (event)
	{
		Get_Sandbox_Primary_Objective(&win_text,SandboxDefender,player);
		event->Set_Reward_Param(0,&win_text);
	}

	// Detect win
	event = SandboxPlot->Get_Event("Sandbox_Objective_1_Complete");
	if (event)
	{
		event->Set_Reward_Param(0,&win_text);

		win_faction.push_back(*player->Get_Faction()->Get_Name());
		event->Set_Param(0,&win_faction);
	}

	// Set up the secondary objective if there is one
	if (!secondary_text.empty())
	{
		event = SandboxPlot->Get_Event("Sandbox_Objective_2");
		if (event)
		{
			event->Set_Reward_Param(0,&secondary_text);
		}

		event = SandboxPlot->Get_Event("Sandbox_Objective_2_Complete");
		if (event)
		{
			event->Set_Reward_Param(0,&secondary_text);

			// Set the object we want to destroy/capture
			event->Set_Param(0,&object_name);

			if (capture)
			{
				// If this is a capture event, set the second parameter
				event->Set_Param(1,&param2);
			}
			else
			{
				// If this is a destroy event, set the third parameter
				event->Set_Param(2,&param2);
			}
		}
	}

	// Set up the protect hero leader objective if leader exists
	if (!protect_text.empty())
	{
		event = SandboxPlot->Get_Event("Sandbox_Objective_3");
		if (event)
		{
			event->Set_Reward_Param(0,&protect_text);
		}

		event = SandboxPlot->Get_Event("Sandbox_Objective_3_Complete");
		if (event)
		{
			event->Set_Reward_Param(0,&protect_text);
		}

		event = SandboxPlot->Get_Event("Sandbox_Objective_3_Failed");
		if (event)
		{
			event->Set_Reward_Param(0,&protect_text);

			// Set the hero we want to protect
			event->Set_Param(0,&protect_name);
			event->Set_Param(2,&o_param);
		}

		// Enable this event
		event = SandboxPlot->Get_Event("Sandbox_Objective_3_Enable");
		if (event)
		{
			event->Event_Triggered();
		}
	}

	// Set up the kill hero leader objective if leader exists
	if (!kill_text.empty())
	{
		event = SandboxPlot->Get_Event("Sandbox_Objective_4");
		if (event)
		{
			event->Set_Reward_Param(0,&kill_text);
		}

		event = SandboxPlot->Get_Event("Sandbox_Objective_4_Complete");
		if (event)
		{
			event->Set_Reward_Param(0,&kill_text);

			// Set the hero we want to kill
			event->Set_Param(0,&kill_name);
			event->Set_Param(2,&o_param);
		}

		// Enable this event
		event = SandboxPlot->Get_Event("Sandbox_Objective_4_Enable");
		if (event)
		{
			event->Event_Triggered();
		}
	}

	// Land mode has additional objectives
	if (land_mode)
	{
		// Add the weather objective
		std::string weather_info = TheWeatherSystem.Get_Weather_Text_Objective_ID();
		Add_Objective(weather_info,NULL,true);

		// Add the capture planet credit reward objective
		GameObjectClass *planet = TheTutorial.Get_Tactical_Planet();
		if (planet && (planet->Get_Owner_Player() != player))
		{
			if (player->Is_Object_Type_On_Captured_Planets_List(planet->Get_Type()) == false)
			{
				int credits = planet->Get_Type()->Get_Planet_Capture_Bonus_Reward();

				std::wstring credit_info;
				std::string credit_id = "TEXT_TACTICAL_PLANETARY_CREDIT_BONUS";
				TheGameText.Get(credit_id,credit_info);

				wchar_t wchar_buffer[ 128 ];
				swprintf(wchar_buffer, 128, credit_info.c_str(), credits);

				credit_info = wchar_buffer;
				Add_Objective(credit_id,&credit_info,true);
			}
		}

		bool events_added = false;
		// Add the indigenous forces objective
		DynamicVectorClass<GameObjectClass *> spawn_list;
		SpawnIndigenousUnitsBehaviorClass::Get_Unique_Spawn_Objects(spawn_list);
		for (int i = 0; i < spawn_list.Size(); ++i)
		{
			GameObjectClass *spawner = spawn_list[i];
			const GameObjectTypeClass *spawn_type = spawner->Get_Type();
			std::string spawn_text;
			int text_index = 1;

			if (good_side)
			{
				text_index = 0;
			}
			else if (corrupt_side)
			{
				text_index = 2;
			}

			if (spawner->Is_Ally(PlayerList.Get_Local_Player()))
			{
				spawn_text = spawn_type->Get_Friendly_Spawn_Text(text_index);
			}
			else
			{
				spawn_text = spawn_type->Get_Enemy_Spawn_Text(text_index);
			}

			if (!spawn_text.empty())
			{
				bool kill_spawners = spawner->Is_Enemy(player);

				Add_Objective(spawn_text,NULL,!kill_spawners);

				//If this was a kill objective then we'll add a code-generated event to mark it complete
				//once all the spawners have been killed.
				if (kill_spawners)
				{
					std::string scratch_string;
					std::vector<std::string> event_params(1);

					//Event to kill the spawners and check off the objective
					StoryEventClass *kill_event = StoryDatabaseParserClass::Create_Event(STORY_CHECK_DESTROYED);
					FAIL_IF(!kill_event) { continue; }

					String_Printf(scratch_string, "%s_00", spawn_type->Get_Name()->c_str());
					kill_event->Set_Name(&scratch_string);
					kill_event->Set_Sub_Plot(SandboxPlot);
					kill_event->Set_Active(true);

					event_params[0] = *player->Get_Faction()->Get_Name();
					kill_event->Set_Param(0, &event_params);
					event_params[0] = "DESTROY_ALL_INDIGENOUS_SPAWNERS";
					kill_event->Set_Param(1, &event_params);
					event_params[0] = *spawn_type->Get_Name();
					kill_event->Set_Param(2, &event_params);

					kill_event->Set_Reward_Type(REWARD_OBJECTIVE_COMPLETE);
					kill_event->Set_Reward_Param(0, &spawn_text);

					CRCValue name_crc = CRCClass::Calculate_CRC(kill_event->Get_Name()->c_str(), kill_event->Get_Name()->length());
					SandboxPlot->Add_Event(name_crc, kill_event);

					//While we've still got the original event name lying around let's construct the
					//prerequisites for the reward event.
					DynamicVectorClass<std::string> and_prereqs;
					and_prereqs.Add(scratch_string);
					DynamicVectorClass< DynamicVectorClass<std::string> > or_prereqs;
					or_prereqs.Add(and_prereqs);

					//Event to reward the player with credits
					StoryEventClass *reward_event = StoryDatabaseParserClass::Create_Event(STORY_TRIGGER);
					FAIL_IF(!reward_event) { continue; }

					String_Printf(scratch_string, "%s_01", spawn_type->Get_Name()->c_str());
					reward_event->Set_Name(&scratch_string);
					reward_event->Set_Sub_Plot(SandboxPlot);

					String_Printf(scratch_string, "%d", TheGameConstants.Get_Indigenous_Spawn_Destruction_Reward());
					reward_event->Set_Reward_Type(REWARD_CREDITS);
					reward_event->Set_Reward_Param(0, &scratch_string);
					reward_event->Add_Prereqs(&or_prereqs);

					name_crc = CRCClass::Calculate_CRC(reward_event->Get_Name()->c_str(), reward_event->Get_Name()->length());
					SandboxPlot->Add_Event(name_crc, reward_event);

					events_added = true;
				}
			}
		}

		if (events_added)
		{
			SandboxPlot->Sort_Events_And_Compute_Dependants();
		}
	}
}






/**************************************************************************************************
* StoryModeClass::Corruption_Increased -- Corruption level of a planet has increased
*
* In:		
*
* Out:	
*
*
* History: 03/23/2006 4:15PM JAC
**************************************************************************************************/
void StoryModeClass::Corruption_Increased(GameObjectClass *planet, int corruption_type)
{
	if (planet)
	{
		Story_Event(STORY_CORRUPTION_CHANGED, NULL, planet, &corruption_type);
	}
}




/**************************************************************************************************
* StoryModeClass::Corruption_Decreased -- Corruption level of a planet has decreased
*
* In:		
*
* Out:	
*
*
* History: 03/23/2006 4:15PM JAC
**************************************************************************************************/
void StoryModeClass::Corruption_Decreased(GameObjectClass *planet, int corruption_type)
{
	if (planet)
	{
		Story_Event(STORY_CORRUPTION_CHANGED, NULL, planet, &corruption_type);
	}
}






/**************************************************************************************************
* StoryModeClass::Corruption_Tactical_Complete - Player has successfully completed a tactical corruption mission
*
* In:		
*
* Out:	
*
*
* History: 08/04/2006 4:15PM JAC
**************************************************************************************************/
void StoryModeClass::Corruption_Tactical_Complete(GameObjectClass *planet, int corruption_type)
{
	if (planet)
	{
		Story_Event(STORY_CORRUPTION_TACTICAL_COMPLETE, NULL, planet, &corruption_type);
	}
}






/**************************************************************************************************
* StoryModeClass::Corruption_Tactical_Complete - Player has failed a tactical corruption mission
*
* In:		
*
* Out:	
*
*
* History: 08/04/2006 4:15PM JAC
**************************************************************************************************/
void StoryModeClass::Corruption_Tactical_Failed(GameObjectClass *planet, int corruption_type)
{
	if (planet)
	{
		Story_Event(STORY_CORRUPTION_TACTICAL_FAILED, NULL, planet, &corruption_type);
	}
}





/**************************************************************************************************
* StoryModeClass::Corruption_Dialog_Opened - Player has opened corruption dialog on a planet
*
* In:		
*
* Out:	
*
*
* History: 08/04/2006 4:15PM JAC
**************************************************************************************************/
void StoryModeClass::Corruption_Dialog_Opened(GameObjectClass *planet)
{
	if (planet)
	{
		std::string planet_name = *planet->Get_Type()->Get_Name();
		Story_Event(STORY_OPEN_CORRUPTION, NULL, (void *)planet_name.c_str(), NULL);
	}
}







/**************************************************************************************************
* StoryModeClass::Garrison_Unit -- A unit has been garrisoned
*
* In:		
*
* Out:	
*
*
* History: 03/23/2006 5:13PM JAC
**************************************************************************************************/
void StoryModeClass::Garrison_Unit(GameObjectClass *unit)
{
	if (unit)
	{
		Story_Event(STORY_GARRISON_UNIT, NULL, unit, NULL);
	}
}





/**************************************************************************************************
* StoryModeClass::Galactic_Sabotage -- Story event fired off when used does galactic sabotage
*
* In:		
*
* Out:	
*
*
* History: 05/19/2006 2:41PM JAC
**************************************************************************************************/
void StoryModeClass::Galactic_Sabotage(GameObjectClass *object)
{
	if (object)
	{
		std::string name = *object->Get_Type()->Get_Name();
		Story_Event(STORY_GALACTIC_SABOTAGE, NULL, (void *)name.c_str(), NULL);
	}
}





/**************************************************************************************************
* StoryModeClass::Buy_Black_Market -- Story event fired off when user buys an item on the black market
*
* In:		
*
* Out:	
*
*
* History: 05/19/2006 2:41PM JAC
**************************************************************************************************/
void StoryModeClass::Buy_Black_Market(const std::string &item_name)
{
	Story_Event(STORY_BUY_BLACK_MARKET, NULL, (void *)item_name.c_str(), NULL);
}



