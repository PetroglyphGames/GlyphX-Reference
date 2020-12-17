// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/UnitMoveObject.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/UnitMoveObject.cpp $
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

#pragma hdrstop


#include "UnitMoveObject.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "GameObject.h"
#include "PGSignal/SignalDispatcher.h"
#include "GameObjectManager.h"


LUA_IMPLEMENT_FACTORY(LUA_CHUNK_UNIT_MOVEMENT_BLOCK, UnitMovementBlockStatus);
PG_IMPLEMENT_RTTI(UnitMovementBlockStatus, BlockingStatus);


/**
 * Constructor
 * @since 6/27/2005 11:39:24 AM -- BMH
 */
UnitMovementBlockStatus::UnitMovementBlockStatus() :
	IsFinished(false),
	Target(NULL),
	OwnerID(-1),
	Stragglers(0)
{
}

/**
 * Destructor
 * @since 6/27/2005 11:39:33 AM -- BMH
 */
UnitMovementBlockStatus::~UnitMovementBlockStatus()
{
	if (Stragglers)
	{
		Free_Lua_Table(Stragglers);
		Stragglers = 0;
	}
}

/**
 * Internal Is_Finished
 * 
 * @return Is the block command complete?
 * @since 6/27/2005 11:39:56 AM -- BMH
 */
bool UnitMovementBlockStatus::Internal_Is_Finished(void)
{
	return IsFinished;
}

/**
 * Lua Is_Finished function.
 * 
 * @return Is the block command complete?
 * @since 6/27/2005 11:40:17 AM -- BMH
 */
LuaTable* UnitMovementBlockStatus::Is_Finished(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(new LuaBool(Internal_Is_Finished()));
}

enum {
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_MOVEMENT_BLOCK_DATA,
	CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED,
	CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID,
	CHUNK_ID_MOVEMENT_BLOCK_STRAGGLERS,
	CHUNK_ID_MOVEMENT_BLOCK_TARGET,
	CHUNK_ID_MOVEMENT_BLOCK_OWNER_ID,
};

/**
 * Lua Save Function
 * 
 * @param script lua script
 * @param writer Chunkwriter
 * 
 * @return true if ok
 * @since 6/27/2005 11:40:38 AM -- BMH
 */
bool UnitMovementBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;

	ok &= writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
	ok &= BlockingStatus::Save(script, writer);
	ok &= writer->End_Chunk();
	
	ok &= writer->Begin_Chunk(CHUNK_ID_MOVEMENT_BLOCK_DATA);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED, IsFinished);
		WRITE_MICRO_CHUNK_OBJECT_PTR(CHUNK_ID_MOVEMENT_BLOCK_TARGET, Target);
		WRITE_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_OWNER_ID, OwnerID);
		WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
	ok &= writer->End_Chunk();

	LUA_WRITE_CHUNK_VALUE_PTR	(	CHUNK_ID_MOVEMENT_BLOCK_STRAGGLERS, Stragglers, script);

	return (ok);
}

/**
 * Lua Load function
 * 
 * @param script lua script
 * @param reader Chunkreader
 * 
 * @return true if ok
 * @since 6/27/2005 11:40:59 AM -- BMH
 */
bool UnitMovementBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;
	while (reader->Open_Chunk()) {
		switch ( reader->Cur_Chunk_ID() )
		{
			case CHUNK_ID_BASE_CLASS:
				ok &= BlockingStatus::Load(script, reader);
				break;

			case CHUNK_ID_MOVEMENT_BLOCK_DATA:
				while (reader->Open_Micro_Chunk()) {
					switch ( reader->Cur_Micro_Chunk_ID() )
					{
							READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_ISFINISHED, IsFinished);
							READ_MICRO_CHUNK_OBJECT_PTR(CHUNK_ID_MOVEMENT_BLOCK_TARGET, Target);
							READ_MICRO_CHUNK(CHUNK_ID_MOVEMENT_BLOCK_OWNER_ID, OwnerID);
							READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_MOVEMENT_BLOCK_LISTENER_BASE_ID, SignalListenerClass);
						default: assert(false); break;	// Unknown Chunk
					}
					reader->Close_Micro_Chunk();
				}
				break;

			LUA_READ_CHUNK_VALUE_PTR	(	CHUNK_ID_MOVEMENT_BLOCK_STRAGGLERS, Stragglers, script);

			default: assert(false); break;	// Unknown Chunk
		}
		reader->Close_Chunk();
	}
	return (ok);
}

/**
 * Return the result of the blocking command.
 * 
 * @return blocking command result.
 * @since 6/27/2005 11:41:25 AM -- BMH
 */
LuaTable* UnitMovementBlockStatus::Result(LuaScriptClass *, LuaTable *)
{
	return NULL;
}


/**
 * Process signals.
 * 
 * @param gen    Signal generator
 * @param type   type of signal
 * @since 6/27/2005 11:41:56 AM -- BMH
 */
void UnitMovementBlockStatus::Receive_Signal(SignalGeneratorClass *gen, PGSignalType type, SignalDataClass *)
{
	GameObjectClass *object = (GameObjectClass *)gen;

	switch (type) {
		case PG_SIGNAL_OBJECT_MOVEMENT_BEGIN:
			break;

		case PG_SIGNAL_OBJECT_MOVEMENT_CANCELED:
		case PG_SIGNAL_OBJECT_OWNER_CHANGED:
		case PG_SIGNAL_OBJECT_DELETE_PENDING:
			//This object is lost to us.
			if (object == Target)
			{
				Target = NULL;
				IsFinished = true;
			}
			else
			{
				Remove_Object(object);
			}
			break;

		case PG_SIGNAL_OBJECT_MOVEMENT_FINISHED:
			//The blocking object should live for as long as the move does.  Only call Remove_Object if the move isn't 
			//going to continue (no object to chase or no visibility on the target)
			if (!Target || (OwnerID != -1 && Target->Get_Manager()->Get_Game_Mode()->Is_Fogged(OwnerID, Target)))
			{
				Remove_Object(object);
			}
			break;

		default:
			return;
	}
}

/**
 * Remove the object from the list of stragglers.
 * 
 * @param object object to remove.
 * @since 6/27/2005 11:42:22 AM -- BMH
 */
void UnitMovementBlockStatus::Remove_Object(GameObjectClass *object)
{
	SignalDispatcherClass::Get().Remove_Listener( object, this, PG_SIGNAL_OBJECT_MOVEMENT_BEGIN );
	SignalDispatcherClass::Get().Remove_Listener( object, this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED );
	SignalDispatcherClass::Get().Remove_Listener( object, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED );
	SignalDispatcherClass::Get().Remove_Listener( object, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
	SignalDispatcherClass::Get().Remove_Listener( object, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );

	if (!Stragglers)
	{
		IsFinished = true;
		return;
	}

	for (int i = 0; i < (int)Stragglers->Value.size(); i++)
	{
		GameObjectWrapper *wrap = PG_Dynamic_Cast<GameObjectWrapper>(Stragglers->Value[i]);

		if (!wrap || !wrap->Get_Object() || wrap->Get_Object()->Is_Delete_Pending())
		{
			Stragglers->Value.erase(Stragglers->Value.begin() + i);
			i--;
			continue;
		}
		GameObjectClass *unit = wrap->Get_Object();
		if (object == unit)
		{
			Stragglers->Value.erase(Stragglers->Value.begin() + i);
			i--;
			continue;
		}
	}

	if (Stragglers->Value.size() == 0)
	{
		IsFinished = true;
		Free_Lua_Table(Stragglers);
		Stragglers = 0;
	}
}

/**
 * Init the blocking status
 * 
 * @param command command that spawned this blocking status.
 * @param units   table of units to track.
 * @since 6/27/2005 11:43:01 AM -- BMH
 */
void UnitMovementBlockStatus::Init(LuaUserVar *command, LuaTable *units, GameObjectClass *target)
{
	GameObjectWrapper *wrap = PG_Dynamic_Cast<GameObjectWrapper>(command);
	FAIL_IF(!wrap) 
	{
		IsFinished = true;
		return;
	}

	Set_Command(command);
	Stragglers = units;

	if (!Stragglers || Stragglers->Value.size() == 0)
	{
		IsFinished = true;
		return;
	}

	Target = target;
	if (Target)
	{
		SignalDispatcherClass::Get().Add_Listener( Target, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
		SignalDispatcherClass::Get().Add_Listener( Target, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );
	}

	for (int i = 0; i < (int)Stragglers->Value.size(); i++)
	{
		GameObjectWrapper *wrap = PG_Dynamic_Cast<GameObjectWrapper>(Stragglers->Value[i]);

		if (!wrap || !wrap->Get_Object() || wrap->Get_Object()->Is_Delete_Pending())
		{
			Stragglers->Value.erase(Stragglers->Value.begin() + i);
			i--;
			continue;
		}
		GameObjectClass *object = wrap->Get_Object();
		OwnerID = object->Get_Owner();
		SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_MOVEMENT_BEGIN );
		SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_MOVEMENT_FINISHED );
		SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_MOVEMENT_CANCELED );
		SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_DELETE_PENDING );
		SignalDispatcherClass::Get().Add_Listener( object, this, PG_SIGNAL_OBJECT_OWNER_CHANGED );
	}

	assert(OwnerID != -1);
}



