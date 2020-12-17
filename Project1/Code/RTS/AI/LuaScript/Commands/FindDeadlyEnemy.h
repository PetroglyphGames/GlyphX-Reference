// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindDeadlyEnemy.h#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindDeadlyEnemy.h $
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

#ifndef _FIND_DEADLY_ENENY_H_
#define _FIND_DEADLY_ENEMY_H_

#include "AI/LuaScript/LuaRTSUtilities.h"


class GameObjectClass;
class TaskForceClass;
class AITargetLocationClass;
class PlayerClass;

//Script command to find the enemy who has hurt us the most recently
class FindDeadlyEnemyClass : public LuaUserVar
{
public:

	PG_DECLARE_RTTI();

	virtual LuaTable *Function_Call(LuaScriptClass *script, LuaTable *params);

	static GameObjectClass *Find_Deadly_Enemy(TaskForceClass *taskforce);
	static GameObjectClass *Find_Deadly_Enemy(GameObjectClass *target);
	static GameObjectClass *Find_Deadly_Enemy(const AITargetLocationClass *target, float range, PlayerClass *for_player);

private:
	static void Add_Object_To_Damage_Tracking_Map(GameObjectClass *object, stdext::hash_map<int, float> &damage_map);

};

#endif //_FIND_DEADLY_ENEMY_H_