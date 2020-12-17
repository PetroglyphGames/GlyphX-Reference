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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/StoryMode/RandomStoryMode.h $
//
//             Author: Jason Curtice
//
//               Date: 04/20/2005 1:07PM
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef RANDOMSTORYMODE_H
#define RANDOMSTORYMODE_H


class StoryEventClass;


#define MAX_DIALOG_SIZE 4096


class RandomStoryModeClass
{
public:

	RandomStoryModeClass() : Count(0), Unlocked(NULL) {}

	void Init();

	void Generate_Random_Story();
	void Reject_Story();
	void Accept_Story();

private:

	StoryEventClass *Add_Trigger(std::string *event_name, int index);
	StoryEventClass *Add_Reward(int num_steps, bool no_buildable = false);
	void Add_Story_Dialog(StoryEventClass *event, int index, char *name);
	StoryEventClass *Add_Remove_Goal_Event(StoryEventClass *prev_event, int index);
	void Add_Prereqs(StoryEventClass *event, int index);

	int Count;
	const DynamicVectorClass<std::string> *Triggers;
	const DynamicVectorClass<std::string> *Rewards;
	const DynamicVectorClass<std::string> *Construct;
	const DynamicVectorClass<std::string> *Destroy;
	const DynamicVectorClass<std::string> *Buildable;
	const DynamicVectorClass<std::string> *Unit;
	DynamicVectorClass<GameObjectClass *> FriendlyPlanets;
	DynamicVectorClass<GameObjectClass *> EnemyPlanets;
	DynamicVectorClass<GameObjectClass *> FriendlyHeroes;
	DynamicVectorClass<GameObjectClass *> EnemyHeroes;

	bool *Unlocked;

	std::string StoryDialog;
	const char *Speaker;
	const char *SpeakerImage;
	const char *DialogTitle;
	const char *AcceptDialog;
	const char *RejectDialog;

	// Current plot under construction
	static StorySubPlotClass *SubPlot;
	static int EventIndex;
};



// Global instance of this class
extern RandomStoryModeClass TheRandomStoryMode;




#endif