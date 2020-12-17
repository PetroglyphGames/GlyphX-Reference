// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/UnitAnimationBlock.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/UnitAnimationBlock.cpp $
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

#include "UnitAnimationBlock.h"

#include "AI/LuaScript/GameObjectWrapper.h"
#include "TeamBehavior.h"

PG_IMPLEMENT_RTTI(UnitAnimationBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_UNIT_ANIMATION_BLOCK, UnitAnimationBlockStatus);

/**************************************************************************************************
* UnitAnimationBlockStatus::Init -- Set up this blocking object
*
* In:				
*
* Out:	
*
* History: 7/05/2005 3:45PM JSY
**************************************************************************************************/
void UnitAnimationBlockStatus::Init(LuaUserVar *command, ModelAnimType animation_type)
{
	assert(PG_Dynamic_Cast<GameObjectWrapper>(command));
	Set_Command(command);

	//Track the animation we're blocking on so we can release the block if the animation changes
	AnimationType = animation_type;
}

/**************************************************************************************************
* UnitAnimationBlockStatus::Is_Finished -- Check whether this block is over
*
* In:				
*
* Out:	
*
* History: 7/05/2005 3:45PM JSY
**************************************************************************************************/
LuaTable *UnitAnimationBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	//If the object is dead then quit blocking
	SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(Get_Command());
	if (!object_wrapper || !object_wrapper->Get_Object())
	{
		return Return_Variable(new LuaBool(true));		
	}

	GameObjectClass *animating_object = object_wrapper->Get_Object();
	FAIL_IF(!animating_object) { return NULL; }

	//Block is over when either the requested animation finishes or the active animation changes.  In the case where we asked a team to animate
	//we wait for all team members to finish.
	bool all_done = true;
	if (animating_object->Behaves_Like(BEHAVIOR_TEAM))
	{
		TeamBehaviorClass *team = static_cast<TeamBehaviorClass*>(animating_object->Get_Behavior(BEHAVIOR_TEAM));
		FAIL_IF(!team) { return Return_Variable(new LuaBool(true)); }
		for (int i = 0; i < team->Get_Team_Member_Count(); ++i)
		{
			GameObjectClass *team_member = team->Get_Team_Member_By_Index(i);
			FAIL_IF(!team_member) { continue; }
			if (team_member->Get_Active_Animation_Type() == AnimationType && team_member->Get_Active_Animation_State() != ANIM_STATE_DONE)
			{
				all_done = false;
			}
		}
	}
	else
	{
		all_done = (animating_object->Get_Active_Animation_Type() != AnimationType || 
						animating_object->Get_Active_Animation_State() == ANIM_STATE_DONE);
	}

	return Return_Variable(new LuaBool(all_done));
}

enum
{
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_UNIT_ANIMATION_BLOCK_DATA,
	CHUNK_ID_UNIT_ANIMATION_BLOCK_ANIMATION_TYPE,
};

/**************************************************************************************************
* UnitAnimationBlockStatus::Save -- Write this blocking object to file
*
* In:				
*
* Out:	
*
* History: 7/05/2005 3:45PM JSY
**************************************************************************************************/
bool UnitAnimationBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(CHUNK_ID_UNIT_ANIMATION_BLOCK_DATA);
		WRITE_MICRO_CHUNK(CHUNK_ID_UNIT_ANIMATION_BLOCK_ANIMATION_TYPE, AnimationType);
	ok &= writer->End_Chunk();

	return ok;
}

/**************************************************************************************************
* UnitAnimationBlockStatus::Save -- Read this blocking object from file
*
* In:				
*
* Out:	
*
* History: 7/05/2005 3:45PM JSY
**************************************************************************************************/
bool UnitAnimationBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
		case CHUNK_ID_BASE_CLASS:
			ok &= BlockingStatus::Load(script, reader);
			break;

		case CHUNK_ID_UNIT_ANIMATION_BLOCK_DATA:
			while (reader->Open_Micro_Chunk())
			{
				switch (reader->Cur_Micro_Chunk_ID())
				{
					READ_MICRO_CHUNK(CHUNK_ID_UNIT_ANIMATION_BLOCK_ANIMATION_TYPE, AnimationType);

				default:
					ok = false;
					assert(false);
					break;
				}

				reader->Close_Micro_Chunk();
			}
			break;

		default:
			assert(false);
			ok = false;
			break;
		}

		reader->Close_Chunk();
	}

	return ok;
}