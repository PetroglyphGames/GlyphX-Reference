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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/StoryMode/RandomStoryMode.cpp $
//
//             Author: Jason Curtice
//
//               Date: 04/20/2005 1:07PM
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma hdrstop		// Needed for pch
#include "StorySubPlot.h"
#include "RandomStoryMode.h"
#include "GameConstants.h"
#include "GameObjectManager.h"
#include "FactionList.h"
#include "PlayerList.h"
#include "PlanetaryBehavior.h"
#include "StoryDialogManager.h"
#include "SpeechEventManager.h"


// Global instance of this class
RandomStoryModeClass TheRandomStoryMode;
StorySubPlotClass *RandomStoryModeClass::SubPlot = NULL;
int RandomStoryModeClass::EventIndex = 0;


/**************************************************************************************************
* RandomStoryModeClass::Init -- Initialize random story arc generator
*
* In:		
*
* Out:	
*
*
* History: 04/22/2005 5:27PM JAC
**************************************************************************************************/
void RandomStoryModeClass::Init()
{
	// Save off pointers to lists of valid parameters for generating random story events
	Triggers = TheGameConstants.Get_Random_Story_Triggers();
	assert(Triggers);
	Rewards = TheGameConstants.Get_Random_Story_Rewards();
	assert(Rewards);

	// Get list of what we might construct/destroy based on player faction
	const FactionClass *faction = PlayerList.Get_Local_Player()->Get_Faction();
	if (faction == FactionList.Find_Faction_By_Name("Rebel"))
	{
		Construct = TheGameConstants.Get_Random_Story_Rebel_Construction();
		Destroy = TheGameConstants.Get_Random_Story_Rebel_Destroy();
		Buildable = TheGameConstants.Get_Random_Story_Reward_Rebel_Buildable();
		Unit = TheGameConstants.Get_Random_Story_Reward_Rebel_Unit();
		Speaker = "RC0%2.2d_MON%2.2d0";
		SpeakerImage = "Mon_Mothma.tga";
		DialogTitle = "TEXT_STORY_TRANSMISSION_17";
		AcceptDialog = "RC001_MON330";
		RejectDialog = "RC001_MON310";
	}
	else
	{
		Construct = TheGameConstants.Get_Random_Story_Empire_Construction();
		Destroy = TheGameConstants.Get_Random_Story_Empire_Destroy();
		Buildable = TheGameConstants.Get_Random_Story_Reward_Empire_Buildable();
		Unit = TheGameConstants.Get_Random_Story_Reward_Empire_Unit();
		Speaker = "EC0%2.2d_TAR%2.2d0";
		SpeakerImage = "Tarkin.tga";
		DialogTitle = "TEXT_STORY_TRANSMISSION_18";
		AcceptDialog = "EC001_TAR330";
		RejectDialog = "EC001_TAR310";
	}

	if (Unlocked != NULL)
	{
		delete Unlocked;
	}

	// Keep track of what buildable items have been offered as a reward
	Unlocked = new bool[Buildable->Size()];
	memset(Unlocked,0,sizeof(bool)*Buildable->Size());

	assert(Construct);
	assert(Destroy);
	assert(Buildable);
	assert(Unit);
}





/**************************************************************************************************
* RandomStoryModeClass::Generate_Random_Story -- Generate a random story arc
*
* In:		
*
* Out:	
*
*
* History: 04/22/2005 5:28PM JAC
**************************************************************************************************/
void RandomStoryModeClass::Generate_Random_Story()
{
	if (GameModeManager.Get_Active_Mode() == NULL)
	{
		return;
	}

	// Clear out the old planet and hero lists if they aren't already empty
	FriendlyPlanets.Truncate();
	EnemyPlanets.Truncate();
	FriendlyHeroes.Truncate();
	EnemyHeroes.Truncate();
	StoryDialog.resize(0);

	const DynamicVectorClass<GameObjectClass *> *object_list = GAME_OBJECT_MANAGER.Find_Objects( BEHAVIOR_SELECTABLE );

	// Create lists of friendly and enemy/neutral planets and heroes every time since things may have changed since last time
	int player_id = PlayerList.Get_Local_Player_ID();
	for (int i=0; i<object_list->Size(); i++)
	{
		GameObjectClass *object = (*object_list)[i];
		if (object->Get_Owner() == player_id)
		{
			if (object->Behaves_Like(BEHAVIOR_PLANET))
			{
				FriendlyPlanets.Add(object);
			}
			else if (object->Get_Type()->Is_Named_Hero())
			{
				FriendlyHeroes.Add(object);
			}
		}
		else
		{
			if (object->Behaves_Like(BEHAVIOR_PLANET))
			{
				// Only add a planet if it can be viewed
				if (object->Get_Planetary_Data())
				{
					if (object->Get_Planetary_Data()->Get_Is_Locally_Revealed())
					{
						EnemyPlanets.Add(object);
					}
				}
			}
			else if (object->Get_Type()->Is_Named_Hero())
			{
				EnemyHeroes.Add(object);
			}
		}
	}

	// Create the sub plot
	char name[32];
	char plot_name[32];
	sprintf(plot_name,"Random_Plot_%d",Count++);
	StorySubPlotClass *sub_plot = new StorySubPlotClass(plot_name,false);
	assert(sub_plot);

	CRCValue plot_crc = CRCClass::Calculate_CRC( plot_name, strlen( plot_name ) );
	StoryEventClass *first_event = NULL;
	StoryEventClass *last_event = NULL;
	int debug_index = 0;

	// Add a random number of events to the sub plot
	int num_steps = FreeRandom.Get(1,TheGameConstants.Get_Random_Story_Max_Triggers());
	for (int i=0; i<num_steps; i++)
	{
		// Set the event name.  It needs to be unique within the subplot
		sprintf(name,"RANDOM_TRIGGER_%d",i);
		std::string trigger_name(name);

		// Create a new event
		// Set the type
		StoryEventClass *event = Add_Trigger(&trigger_name,i);
		if (i == 0)
		{
			first_event = event;
		}

		// Set the index.  This is mainly used for debugging se we can print it out in order
		event->Set_Index(debug_index++);

		// Add prereqs for the event (the one that came before it)
		Add_Prereqs(event,i);

		// Add the story dialog data
		Add_Story_Dialog(event,i,plot_name);

		// Point back to parent plot
		event->Set_Sub_Plot(sub_plot);

		// Add this event to the plot
		CRCValue crc = CRCClass::Calculate_CRC( name, strlen( name ) );
		sub_plot->Add_Event(crc,event);

		// Generate an event to remove the above event's goal from the story summary dialog
		StoryEventClass *remove_event = Add_Remove_Goal_Event(event,i);
		if (remove_event)
		{
			remove_event->Set_Sub_Plot(sub_plot);
			remove_event->Set_Index(debug_index++);

			const char *name_ptr = remove_event->Get_Name()->c_str();
			crc = CRCClass::Calculate_CRC( name_ptr, strlen( name_ptr ) );
			sub_plot->Add_Event(crc,remove_event);
		}

		last_event = event;

		StoryDialog.append("\n");
	}

	// Specify a reward after the last event only
	if (last_event != NULL)
	{
		StoryEventClass *reward_event = Add_Reward(num_steps);
		if (reward_event)
		{
			reward_event->Set_Sub_Plot(sub_plot);
			reward_event->Set_Index(debug_index++);
			Add_Story_Dialog(reward_event,num_steps,plot_name);

			const char *name_ptr = reward_event->Get_Name()->c_str();
			CRCValue crc = CRCClass::Calculate_CRC( name_ptr, strlen( name_ptr ) );
			sub_plot->Add_Event(crc,reward_event);
		}
	}

	sub_plot->Set_Local_Player(PlayerList.Get_Local_Player());
	sub_plot->Sort_Events_And_Compute_Dependants();

	GameModeManager.Get_Active_Mode()->Get_Story_Mode().Add_Plot(plot_crc,sub_plot);

	Debug_Printf("\n--------- Story Dialog Script ---------\n");
	Debug_Printf("%s\n",StoryDialog.c_str());
	Debug_Printf("----------------------------\n");

	bool result = StoryDialogManagerClass::Add_Story(plot_name, (char *)StoryDialog.c_str());
	if (result)
	{
		StoryDialogManagerClass::Launch_Story_Event(plot_name,0,NULL,first_event->Get_Story_Dialog_Tag(),true);
	}
}







/**************************************************************************************************
* RandomStoryModeClass::Add_Trigger -- Select a random trigger and generate random parameters
*
* In:		
*
* Out:	
*
*
* History: 04/22/2005 5:29PM JAC
**************************************************************************************************/
StoryEventClass *RandomStoryModeClass::Add_Trigger(std::string *event_name, int index)
{
	int enemy_id = PlayerList.Get_Local_Player()->Get_Enemy()->Get_ID();

	int event_index = FreeRandom.Get(0,Triggers->Size()-1);
	StoryEventEnum event_type = (StoryEventEnum)StoryEventClass::Lookup_Enum((*Triggers)[event_index].c_str());

	// The type of trigger determines what class will be used when creating the event
	StoryEventClass *event = StoryDatabaseParserClass::Create_Event(event_type);
	assert(event);
	event->Set_Name(event_name);

	Debug_Printf("___________ Event %s, Type %s, ",event->Get_Name()->c_str(),(*Triggers)[event_index].c_str());

	std::vector<std::string> params;

	// Set the trigger parameters
	switch (event_type)
	{
		case STORY_ENTER:
			{
				int planet_index = FreeRandom.Get(0,EnemyPlanets.Size()-1);
				GameObjectClass *planet = EnemyPlanets[planet_index];
				params.push_back(*planet->Get_Type()->Get_Name());
				event->Set_Param(0,&params);
				Debug_Printf("Param1 %s\n",params[0].c_str());
			}
			break;

		case STORY_LAND_ON:
			{
				int planet_index = FreeRandom.Get(0,EnemyPlanets.Size()-1);
				GameObjectClass *planet = EnemyPlanets[planet_index];
				params.push_back(*planet->Get_Type()->Get_Name());
				event->Set_Param(0,&params);
				Debug_Printf("Param1 %s\n",params[0].c_str());
			}
			break;

		case STORY_CONQUER:
			{
				int planet_index = FreeRandom.Get(0,EnemyPlanets.Size()-1);
				GameObjectClass *planet = EnemyPlanets[planet_index];
				params.push_back(*planet->Get_Type()->Get_Name());
				event->Set_Param(0,&params);
				Debug_Printf("Param1 %s\n",params[0].c_str());
			}
			break;

		case STORY_CONSTRUCT:
			{
				int con_index = FreeRandom.Get(0,Construct->Size()-1);
				params.push_back((*Construct)[con_index]);
				event->Set_Param(0,&params);
				Debug_Printf("Param1 %s\n",params[0].c_str());
			}
			break;

		case STORY_CONSTRUCT_LEVEL:
			{
				bool valid = false;
				int tries = 0;

				while (!valid && (tries < 100))
				{
					// First pick a planet
					int planet_index = FreeRandom.Get(0,FriendlyPlanets.Size()-1);
					GameObjectClass *planet = FriendlyPlanets[planet_index];
					PlanetaryBehaviorClass *pbehavior = static_cast<PlanetaryBehaviorClass *> (planet->Get_Behavior( BEHAVIOR_PLANET ));
					int base_level = 0;
					int max_level = 0;
					std::string base;

					// Second pick a base
					if (FreeRandom.Get(0,1) == 0)
					{
						// Ground
						base = "GROUND";
						base_level = pbehavior->Get_Current_Ground_Base_Level(planet);
						max_level = planet->Get_Type()->Get_Max_Ground_Base_Level(planet);
					}
					else
					{
						// Space
						base = "SPACE";
						base_level = pbehavior->Get_Current_Starbase_Level(planet);
						max_level = planet->Get_Type()->Get_Max_Starbase_Level(planet);
					}

					if (base_level < max_level)
					{
						valid = true;
						// Set the planet
						params.push_back(*planet->Get_Type()->Get_Name());
						event->Set_Param(0,&params);
						Debug_Printf("Param1 %s, ",params[0].c_str());
						params.resize(0);

						// Set the target base level
						char value[4];
						sprintf(value,"%d",FreeRandom.Get(base_level+1,max_level));
						std::string level(value);
						params.push_back(level);
						event->Set_Param(1,&params);
						Debug_Printf("Param2 %s, ",params[0].c_str());
						params.resize(0);

						// Set the base type
						params.push_back(base);
						event->Set_Param(2,&params);
						Debug_Printf("Param3 %s\n",params[0].c_str());
					}
					else
					{
						tries++;
					}
				}

				// We couldn't find a valid base to upgrade
				if (!valid)
				{
					// Try again!
					delete event;
					Debug_Printf("FAILED!\n");
					return (Add_Trigger(event_name,index));
				}
			}
			break;

		case STORY_DESTROY:
			{
				// Pick a unit to destroy
				int dest_index = FreeRandom.Get(0,Destroy->Size()-1);
				params.push_back((*Destroy)[dest_index]);
				event->Set_Param(0,&params);
				Debug_Printf("Param1 %s, ",params[0].c_str());
				params.resize(0);

				// How many to destroy
				int destroy_count = FreeRandom.Get(1,3);
				char value[4];
				sprintf(value,"%d",destroy_count);
				std::string num_destroy(value);
				params.push_back(num_destroy);
				event->Set_Param(1,&params);
				Debug_Printf("Param2 %s\n",params[0].c_str());
			}
			break;

		case STORY_DESTROY_BASE:
			{
				// According to the design doc, the base can be on any planet
				#if 0
				// First parameter says what planet to destroy base on
				int planet_index;
				int tries = 0;
				GameObjectClass *planet = NULL;
				// Make sure the planet is an enemy planet and not neutral or pirate
				do
				{
					planet_index = FreeRandom.Get(0,EnemyPlanets.Size()-1);
					planet = EnemyPlanets[planet_index];
					tries++;
				}
				while ((planet->Get_Owner() != enemy_id) && (tries < 100));

				if (tries == 100)
				{
					// We can't seem to do this event.  Delete it and start all over
					delete event;
					Debug_Printf("FAILED!\n");
					return (Add_Trigger(event_name,index));
				}

				params.push_back(*planet->Get_Type()->Get_Name());
				event->Set_Param(0,&params);
				Debug_Printf("Param1 %s, ",params[0].c_str());
				params.resize(0);
				#endif

				// Second parameter tells what type of base to destroy
				std::string base;
				if (FreeRandom.Get(0,1) == 0)
				{
					base = "GROUND";
				}
				else
				{
					base = "SPACE";
				}
				params.push_back(base);
				event->Set_Param(1,&params);
				Debug_Printf("Param1 %s\n",params[0].c_str());
				//Debug_Printf("Param2 %s\n",params[0].c_str());
			}
			break;

		case STORY_DEPLOY:
		case STORY_MOVE:
			{
				// Which hero to deploy/move
				int hero_index = FreeRandom.Get(0,FriendlyHeroes.Size()-1);
				GameObjectClass *hero = FriendlyHeroes[hero_index];
				params.push_back(*hero->Get_Type()->Get_Name());
				event->Set_Param(0,&params);
				Debug_Printf("Param1 %s, ",params[0].c_str());
				params.resize(0);

				// What planet to deploy/move him on
				int planet_index;
				int tries = 0;
				GameObjectClass *planet = NULL;

				// If this is STORY_DEPLOY, make sure we select an enemy planet
				do
				{
					planet_index = FreeRandom.Get(0,EnemyPlanets.Size()-1);
					planet = EnemyPlanets[planet_index];
					tries++;
				}
				while ((event_type == STORY_DEPLOY) && (planet->Get_Owner() != enemy_id) && (tries < 100));

				if (tries == 100)
				{
					// We couldn't find a planet to deploy on
					delete event;
					Debug_Printf("FAILED!\n");
					return (Add_Trigger(event_name,index));
				}

				params.push_back(*planet->Get_Type()->Get_Name());
				event->Set_Param(1,&params);
				Debug_Printf("Param2 %s\n",params[0].c_str());
			}
			break;

		case STORY_ACCUMULATE:
			{
				// For now just pick a random value to get to
				int total = (FreeRandom.Get(10,30) * 1000) + (int)PlayerList.Get_Local_Player()->Get_Credits();
				char value[16];
				sprintf(value,"%d",total);
				std::string credits(value);
				params.push_back(credits);
				event->Set_Param(0,&params);
				Debug_Printf("Param1 %s\n",params[0].c_str());
			}
			break;

		case STORY_DEFEAT_HERO:
			{
				// Which hero to defeat
				int hero_index = FreeRandom.Get(0,EnemyHeroes.Size()-1);
				GameObjectClass *hero = EnemyHeroes[hero_index];
				params.push_back(*hero->Get_Type()->Get_Name());
				event->Set_Param(0,&params);
				Debug_Printf("Param1 %s\n",params[0].c_str());
			}
			break;

		case STORY_WIN_BATTLES:
			{
				// First parameter tells how many battles to win
				int total = FreeRandom.Get(1,3);
				char value[4];
				sprintf(value,"%d",total);
				std::string num_battles(value);
				params.push_back(num_battles);
				event->Set_Param(0,&params);
				Debug_Printf("Param1 %s, ",params[0].c_str());

				// Second parameter tells what type of battle to win ground/space
				params.resize(0);
				std::string base;
				if (FreeRandom.Get(0,1) == 0)
				{
					base = "GROUND";
				}
				else
				{
					base = "SPACE";
				}
				params.push_back(base);
				event->Set_Param(1,&params);
				Debug_Printf("Param2 %s\n",params[0].c_str());
			}
			break;

		default:
			Debug_Printf("\nWarning! Random story trigger %s currently unsupported!\n",(*Triggers)[event_index].c_str());
			break;
	}

	// Generate the story dialog script for this event

	// Add the chapter designation into the story dialog script
	char dialog_data[64];
	sprintf(dialog_data,"[CHAPTER %d]\n",index);
	StoryDialog.append(dialog_data);

	// Add the title of the dialog
	sprintf(dialog_data,"TITLE %s\n",DialogTitle);
	StoryDialog.append(dialog_data);

	// Add the image of the speaker
	sprintf(dialog_data,"IMAGE %s\n",SpeakerImage);
	StoryDialog.append(dialog_data);

	// Add the dialog line.
	int line_index = FreeRandom.Get(1,3);
	sprintf(dialog_data,"DIALOG %s%d\n",Speaker,line_index);
	if (index == 0)
	{
		sprintf(dialog_data,dialog_data,event_index+1,event_index+1);
	}
	else
	{
		sprintf(dialog_data,dialog_data,Triggers->Size()+event_index+1,event_index+1);
	}
	StoryDialog.append(dialog_data);

	// Add the text 
	// Temporarily using the same text as the title
	sprintf(dialog_data,"TEXT %s\n",DialogTitle);
	StoryDialog.append(dialog_data);

	return (event);
}








/**************************************************************************************************
* RandomStoryModeClass::Add_Prereqs -- Compute an events prerequisites
*
* In:		
*
* Out:	
*
*
* History: 04/22/2005 5:29PM JAC
**************************************************************************************************/
void RandomStoryModeClass::Add_Prereqs(StoryEventClass *event, int index)
{
	if (index == 0)
	{
		// The first event should be active
		event->Set_Active(true);
	}
	else
	{
		DynamicVectorClass< DynamicVectorClass<std::string> > all_prereqs;
		DynamicVectorClass< std::string > prereq;

		char name[32];
		sprintf(name,"RANDOM_TRIGGER_%d",index-1);
		std::string trigger_name(name);
		prereq.Add(trigger_name);
		all_prereqs.Add(prereq);

		event->Add_Prereqs(&all_prereqs);
		Debug_Printf("\t\t\t\t\tPrereq %s\n",name);
	}
	Debug_Printf("\n");
}










/**************************************************************************************************
* RandomStoryModeClass::Add_Reward -- Generate a random reward for completing all of the events
*
* In:		
*
* Out:	
*
*
* History: 04/22/2005 5:30PM JAC
**************************************************************************************************/
StoryEventClass *RandomStoryModeClass::Add_Reward(int num_steps, bool no_buildable)
{
	StoryEventClass *event = StoryDatabaseParserClass::Create_Event(STORY_TRIGGER);
	assert(event);

	char name[32];
	sprintf(name,"RANDOM_TRIGGER_%d",num_steps);
	std::string trigger_name(name);
	event->Set_Name(&trigger_name);

	Debug_Printf("___________ Event %s, Type STORY_TRIGGER\n",event->Get_Name()->c_str());

	Add_Prereqs(event,num_steps);

	int reward_index;
	StoryRewardEnum reward_type = REWARD_NONE;

	// If we know that there are no buildable units to unlock, don't pick that as a reward
	do
	{
		reward_index = FreeRandom.Get(0,Rewards->Size()-1);
		reward_type = (StoryRewardEnum)StoryEventClass::Lookup_Enum((*Rewards)[reward_index].c_str());
	}
	while (no_buildable && (reward_type == REWARD_BUILDABLE_UNIT));

	if (reward_type == REWARD_NONE)
	{
		Debug_Printf("Warning!  Random story reward %s is unknown!\n",(*Rewards)[reward_index].c_str());
	}
	else
	{
		event->Set_Reward_Type(reward_type);

		switch (reward_type)
		{
			case REWARD_BUILDABLE_UNIT:
				{
					// Check to see if there are actually any buildable slots available to unlock
					bool free_slot = false;
					for (int i=0; i<Buildable->Size(); i++)
					{
						if (Unlocked[i] == false)
						{
							free_slot = true;
							break;
						}
					}

					if (!free_slot)
					{
						// We can't unlock anything else, try again
						delete event;
						return (Add_Reward(num_steps,true));
					}

					Debug_Printf("\t\t\tReward type %s, ",(*Rewards)[reward_index].c_str());

					int param_index = 0;
					bool found = false;
					while (!found)
					{
						param_index = FreeRandom.Get(0,Buildable->Size()-1);
						if (Unlocked[param_index] == false)
						{
							// Flag this buildable item as now used up so it won't be offered again
							Unlocked[param_index] = true;
							break;
						}
					}
					event->Set_Reward_Param(0,&(*Buildable)[param_index]);
					Debug_Printf("Param1 %s\n",(*Buildable)[param_index].c_str());
				}
				break;

			case REWARD_UNIQUE_UNIT:
				{
					Debug_Printf("\t\t\tReward type %s, ",(*Rewards)[reward_index].c_str());

					int param_index = FreeRandom.Get(0,Unit->Size()-1);
					event->Set_Reward_Param(0,&(*Unit)[param_index]);
					Debug_Printf("Param1 %s, ",(*Unit)[param_index].c_str());

					// Probably should specify a planet as well
					int planet_index = FreeRandom.Get(0,FriendlyPlanets.Size()-1);
					GameObjectClass *planet = FriendlyPlanets[planet_index];
					assert(planet);
					event->Set_Reward_Param(1,planet->Get_Type()->Get_Name());
					Debug_Printf("Param2 %s\n",planet->Get_Type()->Get_Name()->c_str());
				}
				break;

			case REWARD_CREDITS:
				{
					Debug_Printf("\t\t\tReward type %s, ",(*Rewards)[reward_index].c_str());

					char value[16];
					sprintf(value,"%d",10000*num_steps);
					std::string credits(value);
					event->Set_Reward_Param(0,&credits);
					Debug_Printf("Param1 %s\n",value);
				}
				break;

			default:
				Debug_Printf("Warning!  Random story reward %s unsupported!\n",(*Rewards)[reward_index].c_str());
				break;
		}
	}
	Debug_Printf("\n");

	// Add the chapter designation into the story dialog script
	char dialog_data[64];
	sprintf(dialog_data,"[CHAPTER %d]\n",num_steps);
	StoryDialog.append(dialog_data);

	// Add the title of the dialog
	sprintf(dialog_data,"TITLE %s\n",DialogTitle);
	StoryDialog.append(dialog_data);

	// Add the image of the speaker
	sprintf(dialog_data,"IMAGE %s\n",SpeakerImage);
	StoryDialog.append(dialog_data);

	// Add the dialog line.

	// Add the text 
	// Temporarily using the same text as the title
	sprintf(dialog_data,"TEXT %s\n",DialogTitle);
	StoryDialog.append(dialog_data);

	return (event);
}






/**************************************************************************************************
* RandomStoryModeClass::Reject_Story -- Player has rejected the random story arc
*
* In:		
*
* Out:	
*
*
* History: 04/22/2005 5:30PM JAC
**************************************************************************************************/
void RandomStoryModeClass::Reject_Story()
{
	int story_index = Count - 1;
	assert(story_index >= 0);

	char name[32];
	sprintf(name,"Random_Plot_%d",story_index);
	CRCValue plot_crc = CRCClass::Calculate_CRC( name, strlen( name ) );

	GameModeManager.Get_Active_Mode()->Get_Story_Mode().Remove_Plot(plot_crc);
	StoryDialogManagerClass::Remove_Story(name);

	sprintf(name,"%s%d",RejectDialog,FreeRandom.Get(1,2));
	//TheSpeechEventManager.Queue_Speech_Event(name);
}






/**************************************************************************************************
* RandomStoryModeClass::Accept_Story -- Player has accepted the random story arc
*
* In:		
*
* Out:	
*
*
* History: 04/22/2005 5:30PM JAC
**************************************************************************************************/
void RandomStoryModeClass::Accept_Story()
{
	char name[32];
	sprintf(name,"%s%d",AcceptDialog,FreeRandom.Get(1,3));
	//TheSpeechEventManager.Queue_Speech_Event(name);
}



/**************************************************************************************************
* RandomStoryModeClass::Add_Story_Dialog -- Set event's story dialog parameters
*
* In:		
*
* Out:	
*
*
* History: 04/22/2005 5:31PM JAC
**************************************************************************************************/
void RandomStoryModeClass::Add_Story_Dialog(StoryEventClass *event, int index, char *name)
{
	assert(event);

	std::string story_name(name);
	event->Set_Story_Dialog(&story_name);
	event->Set_Story_Dialog_Chapter(index);

	Debug_Printf("\t\t\tStory dialog %s, Chapter %d, ",name,index);

	story_name.append(*event->Get_Name());
	event->Set_Story_Dialog_Tag(&story_name);

	Debug_Printf("Tag %s\n\n",story_name.c_str());
}





/**************************************************************************************************
* RandomStoryModeClass::Add_Remove_Goal_Event -- Create an event that will remove the previous event from summary dialog
*
* In:		
*
* Out:	
*
*
* History: 04/22/2005 5:31PM JAC
**************************************************************************************************/
StoryEventClass *RandomStoryModeClass::Add_Remove_Goal_Event(StoryEventClass *prev_event, int index)
{
	assert(prev_event);

	// This event automatically fires when the previous one is finished
	StoryEventClass *event = StoryDatabaseParserClass::Create_Event(STORY_TRIGGER);
	assert(event);

	// Name this event something similar to the event that this will remove from the summary dialog
	const char *name = prev_event->Get_Name()->c_str();
	std::string event_name(name);
	event_name.append("a");
	event->Set_Name(&event_name);

	Debug_Printf("___________ Event %s, Type STORY_TRIGGER\n",event->Get_Name()->c_str());

	// Add one to index because the Add_Prereqs function assumes you want to link to the previous event
	Add_Prereqs(event,index+1);

	// Figure out the tag of the previous event
	std::string tag = *prev_event->Get_Story_Dialog_Tag();

	event->Set_Reward_Type(REWARD_REMOVE_STORY_GOAL);
	event->Set_Reward_Param(0,&tag);

	Debug_Printf("\t\t\tReward type REMOVE_STORY_GOAL, Param1 %s\n\n",tag.c_str());

	return (event);
}








