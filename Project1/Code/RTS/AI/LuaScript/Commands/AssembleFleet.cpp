// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/AssembleFleet.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/AssembleFleet.cpp $
//
//    Original Author: James Yarrow
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

#pragma hdrstop

#include "AssembleFleet.h"

#include "GameObject.h"
#include "FleetManagementEvent.h"
#include "AI/LuaScript/GameObjectWrapper.h"

PG_IMPLEMENT_RTTI(AssembleFleetClass, LuaUserVar);


/**************************************************************************************************
* AssembleFleetClass::Function_Call -- Script function to merge a table of units into a single fleet.
*	The units must all be located at one planet.  Any units that are actually landed on the planet will
*	be pushed into orbit and then merged
*
* In:				
*
* Out:	
*
* History: 8/16/2005 2:54PM JSY
**************************************************************************************************/
LuaTable *AssembleFleetClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 1)
	{
		script->Script_Error("Assemble_Fleet -- invalid number of parameters.  Expected 1, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaTable> unit_table = PG_Dynamic_Cast<LuaTable>(params->Value[0]);
	if (!unit_table)
	{
		script->Script_Error("Assemble_Fleet -- invalid type for parameter 1.  Expected lua table of units.");
		return NULL;
	}

	if (unit_table->Value.size() == 0)
	{
		script->Script_Error("Assemble_Fleet -- table of units to assemble is empty.");
		return NULL;
	}

	GameObjectClass *master_fleet = NULL;

	FleetManagementEventClass event;
	for (unsigned int i = 0; i < unit_table->Value.size(); ++i)
	{
		SmartPtr<GameObjectWrapper> unit_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(unit_table->Value[i]);
		if (!unit_wrapper)
		{
			script->Script_Error("Assemble_Fleet -- unexpected type found in table of units to assemble.  These should all be GameObjectWrappers");
			return NULL;
		}

		GameObjectClass *unit = unit_wrapper->Get_Object();
		if (!unit)
		{
			script->Script_Error("Assemble_Fleet -- dead object found in table of units to assemble.");
			return NULL;
		}

		event.Set_Player_ID(unit->Get_Owner());

		GameObjectClass *parent = unit->Get_Parent_Container_Object();
		if (parent)
		{
			if (parent->Behaves_Like(BEHAVIOR_FLEET))
			{
				//The unit is already in a fleet
				GameObjectClass *planet = parent->Get_Parent_Container_Object();
				if (!planet)
				{
					script->Script_Error("Assemble_Fleet -- unit %s is in a fleet that is in transit.", unit->Get_Type()->Get_Name()->c_str());
					return NULL;
				}

				if (master_fleet)
				{
					//We've already determined a fleet that all others are going to be merged into.
					if (planet != master_fleet->Get_Parent_Container_Object())
					{
						script->Script_Error("Assemble_Fleet -- failed because not all units are at the same planet");
						return NULL;
					}

					// AJA 09/01/2006 - Only do a fleet merge action if they're not already in the same fleet.
					if ( parent != master_fleet )
					{
						event.Init(planet->Get_ID(), master_fleet->Get_ID(), parent->Get_ID(), FleetManagementEventClass::ACTION_MERGE);
						event.Execute();
					}
				}
				else
				{
					//This is the first fleet we've encountered.  We'll merge all subsequent fleets into it.
					master_fleet = parent;
				}
			}
			else if (parent->Behaves_Like(BEHAVIOR_PLANET))
			{
				//Unit is landed on the planet's surface
				if (master_fleet && master_fleet->Get_Parent_Container_Object() != parent)
				{
					script->Script_Error("Assemble_Fleet -- failed because not all units are at the same planet");
					return NULL;
				}

				//Support supplying fleets themselves to this function
				if (unit->Behaves_Like(BEHAVIOR_FLEET))
				{
					if (master_fleet)
					{
						//We've already determined a fleet that all others are going to be merged into.
						event.Init(parent->Get_ID(), master_fleet->Get_ID(), unit->Get_ID(), FleetManagementEventClass::ACTION_MERGE);
						event.Execute();
					}
					else
					{
						//This is the first fleet we've encountered.  We'll merge all subsequent fleets into it.
						master_fleet = unit;
					}
				}
				else if (unit->Behaves_Like(BEHAVIOR_TRANSPORT))
				{
					//Push the unit into orbit...
					event.Init(parent->Get_ID(), INVALID_OBJECT_ID, unit->Get_ID(), FleetManagementEventClass::ACTION_TO_ORBIT);
					event.Execute();

					if (unit->Get_Parent_Container_Object() == parent)
					{
						script->Script_Error("Assemble_Fleet - unknown error putting landed unit %s into orbit.", unit->Get_Type()->Get_Name()->c_str());
						return NULL;
					}

					//...then add it to a fleet.  If we don't have a fleet already the event will take care of creating one.
					event.Init(parent->Get_ID(), master_fleet ? master_fleet->Get_ID() : INVALID_OBJECT_ID, unit->Get_ID(), FleetManagementEventClass::ACTION_ADD);
					event.Execute();

					//Grab the fleet object in case we didn't haveone already
					master_fleet = unit->Get_Parent_Container_Object();

					if (!master_fleet || !master_fleet->Behaves_Like(BEHAVIOR_FLEET))
					{
						script->Script_Error("Assemble_Fleet - unknown error adding unit %s to a fleet.", unit->Get_Type()->Get_Name()->c_str());
						return NULL;
					}
				}
			}
			else
			{
				script->Script_Error("Assemble_Fleet - unit %s is neither in a fleet nor landed.  This is a very unhappy state; how did it happen?", unit->Get_Type()->Get_Name()->c_str());
				return NULL;
			}
		}
	}

	//Give the script a handle to the fleet that now contains all the objects passed in.
	return Return_Variable(GameObjectWrapper::Create(master_fleet, script));
}
