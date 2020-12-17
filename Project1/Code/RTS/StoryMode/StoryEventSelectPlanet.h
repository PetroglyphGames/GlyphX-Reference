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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/StoryMode/StoryEventSelectPlanet.h $
//
//             Author: Eric Yiskis
//
//               Date: 06/03/2005 3:52PM
//
///////////////////////////////////////////////////////////////////////////////////////////////////




#ifndef STORYEVENTSELECTPLANET_H
#define STORYEVENTSELECTPLANET_H


#include "StoryEvent.h"

// Supports STORY_SELECT_PLANET, STORY_ZOOM_INTO_PLANET, STORY_ZOOM_OUT_PLANET
class StoryEventSelectPlanetClass : public StoryEventClass
{
public:

	StoryEventSelectPlanetClass();

	virtual void Shutdown();
	virtual void Evaluate_Event(void *param1,void *param2);
	virtual void Set_Param(int index, std::vector<std::string> *param);
	virtual void Replace_Variable(const std::string &var_name, const std::string &new_name);

private:

	std::vector<std::string> Planet;
	StoryEventFilter Filter;
};


#endif
