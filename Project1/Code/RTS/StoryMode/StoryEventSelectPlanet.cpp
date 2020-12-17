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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/StoryMode/StoryEventSelectPlanet.cpp $
//
//             Author: Eric Yiskis
//
//               Date: 06/03/2005 3:52PM
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#pragma hdrstop

#include "gameobject.h"
#include "StoryEventSelectPlanet.h"
#include "FactionList.h"
#include "Faction.h"
#include "PlayerList.h"
#include "DatabaseMap.h"
#include "StorySubPlot.h"
#include "DebugPrint.h"
#include "FrameSynchronizer.h"
#include "GameObjectType.h"
#include "GameObjectTypeManager.h"
#include "GameObjectManager.h"
#include "ProductionBehavior.h"
#include "PlanetaryBehavior.h"
#include "StoryMode.h"
#include "StoryDialogManager.h"
#include "ChunkFile.h"
#include "CommandBar.h"
#include "SFXEventManager.h"
#include "SpeechEventManager.h"
#include "SpeechConversationManager.h"
#include "SFXConversationManager.h"
#include "Tutorial.h"
#include "GalacticCamera.h"
#include "AI/PathFinding/GalacticPath.h"
#include "AI/PathFinding/GalacticPathFinder.h"
#include "SelectEvent.h"
#include "MoveThroughObjectsEvent.h"
#include "OutgoingEventQueue.h"
#include "FleetManagementEvent.h"
#include "ICombatantBehavior.h"
#include "InvadeEvent.h"
#include "TransportBehavior.h"
#include "CampaignData.h"
#include "CampaignDataManager.h"
#include "AI/AIPlayer.h"
#include "AI/TacticalAIManager.h"
#include "AI/Execution/AIExecutionSystem.h"
#include "AI/Execution/AIFreeStore.h"
#include "FleetBehavior.h"
#include "EnumConversion.h"
#include "DifficultyLevelType.h"
#include "VictoryMonitor.h"
#include "RandomStoryMode.h"
#include "BaseShieldBehavior.h"
#include "Diagnostics/DebugWindow.h"
#include "Diagnostics/LogWindow.h"



/**************************************************************************************************
* StoryEventSelectPlanetClass::StoryEventSelectPlanetClass -- Enter event constructor
*
* In:		
*
* Out:	
*
*
* History: 06/03/2005 4:00PM ERK
**************************************************************************************************/
StoryEventSelectPlanetClass::StoryEventSelectPlanetClass() :
Filter(EVENT_FILTER_NONE)
{
}




/**************************************************************************************************
* StoryEventSelectPlanetClass::Shutdown -- Clear out data
*
* In:		
*
* Out:	
*
*
* History: 06/03/2005 4:00PM ERK
**************************************************************************************************/
void StoryEventSelectPlanetClass::Shutdown()
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		Planet[i].clear();
	}

	Planet.clear();
}




/**************************************************************************************************
* StoryEventSelectPlanetClass::Set_Param -- Set the parameters for this event
*
* This takes only one parameter, the planet. The planet can be empty, in which case 
* it assumes that any planet is acceptable.
* 
* In:		
*
* Out:	
*
*
* History: 06/03/2005 4:00PM ERK
**************************************************************************************************/
void StoryEventSelectPlanetClass::Set_Param(int index, std::vector<std::string> *param)
{
	assert(param);

	if (index == 0)
	{
		char name[ 256 ];
		Planet.resize(0);

		//Story_Debug_Printf("Enter param 1 - ");
		for (unsigned int i=0; i<param->size(); i++)
		{
			if ((*param)[i].size())
			{
				strcpy( name, (*param)[i].c_str() );
				_strupr( name );
				std::string planet_name(name);
				Planet.push_back(planet_name);

				const GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(name);
				if (type == NULL)
				{
					Story_Debug_Printf("Error!  Story Mode - Event %s, invalid planet name %s\r\n",EventName.c_str(),name);
					//assert(type);
				}
				//Story_Debug_Printf("%s ",name);
			}
		}
		//Story_Debug_Printf("\r\n");
	}
	else if (index == 1)
	{
		Filter = (StoryEventFilter)Lookup_Enum((*param)[0].c_str());
		//Story_Debug_Printf("Enter param 2 - %s\r\n",(*param)[0].c_str());
	}
}




/**************************************************************************************************
* StoryEventSelectPlanetClass::Evaluate_Event -- See if this event has been triggered
*
* This takes only one parameter, the planet. The planet can be empty, in which case 
* it assumes that any planet is acceptable.
* 
* In:		
*
* Out:	
*
*
* History: 06/03/2005 4:00PM ERK
**************************************************************************************************/
void StoryEventSelectPlanetClass::Evaluate_Event(void *param1,void * /*param2*/)
{
	if (param1 == NULL) 
	{
		return;
	}

	GameObjectClass *planet = (GameObjectClass *)param1;

	// If no planet is specified in the script, assume any planet will do
	if (Planet.empty())
	{
		Story_Debug_Printf("STORY EVENT - No planet specified in script.  Assuming any planet will trigger event\r\n");
		Event_Triggered(planet);
		return;
	}

	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == *planet->Get_Type()->Get_Name())
		{
			Event_Triggered(planet);
		}
	}
}





/**************************************************************************************************
* StoryEventSelectPlanetClass::Evaluate_Event -- See if this event has been triggered
*
* In:		
*
* Out:	
*
*
* History: 06/03/2005 4:00PM ERK
**************************************************************************************************/
void StoryEventSelectPlanetClass::Replace_Variable(const std::string &var_name, const std::string &new_name)
{
	for (unsigned int i=0; i<Planet.size(); i++)
	{
		if (Planet[i] == var_name)
		{
			Planet[i] = new_name;
			Story_Debug_Printf("EVENT %s, replacing %s with %s\r\n",EventName.c_str(),var_name.c_str(),new_name.c_str());
		}
	}

	Replace_Reward_Variable(var_name,new_name);
}


