// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ProduceObject.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ProduceObject.cpp $
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
#include "RefCount.h"
#include "ProduceObject.h"
#include "DataPacks.h"
#include "GameObject.h"
#include "GameObjectType.h"
#include "GameObjectTypeManager.h"
#include "FleetBehavior.h"
#include "Player.h"
#include "ProductionEvent.h"
#include "OutgoingEventQueue.h"
#include "PGSignal/SignalListener.h"
#include "PGSignal/SignalDispatcher.h"
#include "BlockingStatus.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/GameObjectTypeWrapper.h"
#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/BudgetWrapper.h"
#include "ObjectUnderConstruction.h"

enum 
{
	CHUNK_ID_BASE_CLASS,
	PRODUCTION_WHO_CHUNK,
	PRODUCTION_WHAT_CHUNK,
	PRODUCTION_WHERE_CHUNK,
	PRODUCTION_HASBEGUN_CHUNK,
	PRODUCTION_ISFINISHED_CHUNK,
	PRODUCTION_BUILDID_CHUNK,
	PRODUCTION_NEWBUILDID_CHUNK,
	PRODUCTION_DATA_CHUNK,
	PRODUCTION_RESULT_OBJECT_CHUNK,
	PRODUCTION_BUDGET_CHUNK,
	CHUNK_ID_PRODUCTION_LISTENER_BASE_ID,

};

/**
 * This class keeps track of the blocking status for the ProduceObject command.
 * @since 4/22/2004 1:55:46 PM -- BMH
 */
class ProductionBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_PRODUCTION_BLOCK, ProductionBlockStatus);

	ProductionBlockStatus() : HasBegun(false), bIsFinished(false), BuildId(-1), HasBeenScheduled(false)
	{
	}

	void Init(LuaUserVar *command, PlayerWrapper *who, GameObjectTypeWrapper *what, GameObjectWrapper *where, int build_id, BudgetWrapper *budget)
	{
		Set_Command(command);
		Who = who;
		What = what;
		Where = where;
		BuildId = build_id;
		Budget = budget;
		if (Budget) Budget->Mark_Resources_For_Spend(static_cast<float>(what->Get_Object()->Get_Build_Cost_Credits()));
		Post_Load(NULL);
	}

	virtual void Post_Load(LuaScriptClass * /*script*/)
	{
		SignalDispatcherClass::Get().Add_Listener( Where->Get_Object(), this, PG_SIGNAL_OBJECT_PRODUCTION_BEGIN );
		SignalDispatcherClass::Get().Add_Listener( Where->Get_Object(), this, PG_SIGNAL_OBJECT_PRODUCTION_FINISHED );
	}

	virtual LuaTable* Is_Finished(LuaScriptClass *, LuaTable *)
	{
		return Return_Variable(new LuaBool(HasBegun && bIsFinished));
	}

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer)
	{
		assert(writer != NULL);
		bool ok = true;

		ok &= writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
		ok &= BlockingStatus::Save(script, writer);
		ok &= writer->End_Chunk();

		ok &= writer->Begin_Chunk(PRODUCTION_DATA_CHUNK);
			WRITE_MICRO_CHUNK						(PRODUCTION_HASBEGUN_CHUNK,			HasBegun);
			WRITE_MICRO_CHUNK						(PRODUCTION_ISFINISHED_CHUNK,			bIsFinished);
			WRITE_MICRO_CHUNK						(PRODUCTION_BUILDID_CHUNK,				BuildId);
			WRITE_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_PRODUCTION_LISTENER_BASE_ID, SignalListenerClass);
		ok &= writer->End_Chunk();

		LUA_WRITE_CHUNK_VALUE_PTR			(PRODUCTION_WHO_CHUNK,				Who, 				script);
		LUA_WRITE_CHUNK_VALUE_PTR			(PRODUCTION_WHAT_CHUNK,				What, 			script);
		LUA_WRITE_CHUNK_VALUE_PTR			(PRODUCTION_WHERE_CHUNK,			Where, 			script);
		LUA_WRITE_CHUNK_VALUE_PTR			(PRODUCTION_RESULT_OBJECT_CHUNK,	ResultObject,	script);
		LUA_WRITE_CHUNK_VALUE_PTR			(PRODUCTION_BUDGET_CHUNK,			Budget,			script);

		return (ok);
	}

	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader)
	{
		assert(reader != NULL);
		bool ok = true;

		while (reader->Open_Chunk()) {
			switch (reader->Cur_Chunk_ID()) {

				LUA_READ_CHUNK_VALUE_PTR			(PRODUCTION_WHO_CHUNK,				Who, 				script);
				LUA_READ_CHUNK_VALUE_PTR			(PRODUCTION_WHAT_CHUNK,				What, 			script);
				LUA_READ_CHUNK_VALUE_PTR			(PRODUCTION_WHERE_CHUNK,			Where, 			script);
				LUA_READ_CHUNK_VALUE_PTR			(PRODUCTION_RESULT_OBJECT_CHUNK,	ResultObject,	script);
				LUA_READ_CHUNK_VALUE_PTR			(PRODUCTION_BUDGET_CHUNK,			Budget,			script);

				case CHUNK_ID_BASE_CLASS:
					ok &= BlockingStatus::Load(script, reader);
					break;

				case PRODUCTION_DATA_CHUNK:
					while (reader->Open_Micro_Chunk()) {
						switch (reader->Cur_Micro_Chunk_ID()) {
							READ_MICRO_CHUNK						(PRODUCTION_HASBEGUN_CHUNK,			HasBegun);
							READ_MICRO_CHUNK						(PRODUCTION_ISFINISHED_CHUNK,			bIsFinished);
							READ_MICRO_CHUNK						(PRODUCTION_BUILDID_CHUNK,				BuildId);
							READ_MICRO_CHUNK_MULTI_BASE_PTR(CHUNK_ID_PRODUCTION_LISTENER_BASE_ID, SignalListenerClass);
							default: assert(false); break;   // Unknown Chunk
						}
						reader->Close_Micro_Chunk();
					}
					break;
			}
			reader->Close_Chunk();
		}
		return (ok);
	}

	virtual LuaTable* Result(LuaScriptClass *, LuaTable *)
	{
		return Return_Variable(ResultObject);
	}

	/// signal dispatcher interface
	virtual void Receive_Signal(SignalGeneratorClass *, PGSignalType type, SignalDataClass *data)
	{
		ObjectUnderConstructionClass *prod = PG_Dynamic_Cast<ObjectUnderConstructionClass>(data);

		if (!prod) return;

		switch (type) {
			case PG_SIGNAL_OBJECT_PRODUCTION_BEGIN:
				if (prod->GameObjectType->Get_ID() == What->Get_Object()->Get_ID() &&
					 prod->PlayerID == Who->Get_Object()->Get_ID() &&
					 prod->BuildID == BuildId)
				{
					if (Budget) Budget->Spend_Resources(static_cast<float>(What->Get_Object()->Get_Build_Cost_Credits()));
					HasBegun = true;
				}
				break;
			case PG_SIGNAL_OBJECT_PRODUCTION_CANCELED: // For now do the same thing....
//				break;
			case PG_SIGNAL_OBJECT_PRODUCTION_FINISHED:
				if (prod->GameObjectType->Get_ID() == What->Get_Object()->Get_ID() &&
					 prod->PlayerID == Who->Get_Object()->Get_ID() &&
					 prod->BuildID == BuildId)
				{

					GameObjectClass *obj = prod->BuiltObject;
					bIsFinished = true;
					if (!obj) break;

					FleetBehaviorClass *behave = (FleetBehaviorClass *)obj->Get_Behavior(BEHAVIOR_FLEET);
					if (behave) {
						assert(behave->Get_Contained_Objects_Count(obj) == 1);

						obj = behave->Get_Contained_Object(obj, 0);
					}

					assert(obj);
					ResultObject = GameObjectWrapper::Create(obj, NULL);
				}
				break;
			default:
				return;
		}
	}

private:
	SmartPtr<PlayerWrapper>						Who;
	SmartPtr<GameObjectTypeWrapper>			What;
	SmartPtr<GameObjectWrapper>				Where;
	SmartPtr<GameObjectWrapper>				ResultObject;
	SmartPtr<BudgetWrapper>						Budget;
	bool												HasBegun;
	bool												HasBeenScheduled;
	bool												bIsFinished;
	int												BuildId;
};

int ProduceObjectClass::NewBuildId = 1;

LUA_IMPLEMENT_FACTORY(LUA_CHUNK_PRODUCTION_BLOCK, ProductionBlockStatus);
PG_IMPLEMENT_RTTI(ProductionBlockStatus, BlockingStatus);

LUA_IMPLEMENT_FACTORY(LUA_CHUNK_PRODUCEOBJECT_CHUNK, ProduceObjectClass);
PG_IMPLEMENT_RTTI(ProduceObjectClass, LuaUserVar);

bool ProduceObjectClass::Save(LuaScriptClass *, ChunkWriterClass *writer)
{
	assert(writer != NULL);
	bool ok = true;
	WRITE_MICRO_CHUNK						(PRODUCTION_NEWBUILDID_CHUNK,				NewBuildId);
	return (ok);
}

bool ProduceObjectClass::Load(LuaScriptClass *, ChunkReaderClass *reader)
{
	assert(reader != NULL);
	bool ok = true;

	while (reader->Open_Micro_Chunk()) {
		switch (reader->Cur_Micro_Chunk_ID()) {
			READ_MICRO_CHUNK						(PRODUCTION_NEWBUILDID_CHUNK,				NewBuildId);
			default: assert(false); break;   // Unknown Chunk
		}
		reader->Close_Micro_Chunk();
	}
	return (ok);
}

/**
 * Static function that schedules the production event.
 * 
 * @param player Player to build the object as.
 * @param where  Planet to build the object at.
 * @param object Gameobject type to build
 * 
 * @return true if build was scheduled
 * @since 4/22/2004 1:55:25 PM -- BMH
 */
bool ProduceObjectClass::ProduceObject(PlayerClass *who, GameObjectTypeClass *what, GameObjectClass *where, int build_id)
{
	if (who->Can_Produce_Object(what, where, false, true)) {
		// Add the event to the build queue.
		ProductionEventClass event;
		event.Init(what->Get_Type_Index(), where->Get_ID(), build_id, false, PRODUCTION_QUEUE_INVALID );
		event.Set_Player_ID(who->Get_ID());
		event.Execute();
		return true;
	}
	return false;
}

/**
 * Entry point for Lua function call.
 * 
 * @param script The script associated with this command.
 * @param params Lua parameters passed into this function.
 *               1) PlayerWrapper of the building player.
 *               2) GameObjectTypeWrapper of the type of object to build.
 *               3) GameObjectWrapper of the planet/starbase that will build the object.
 * 
 * @return Table of return values to pass to Lua.
 * @since 4/22/2004 1:52:56 PM -- BMH
 */
LuaTable* ProduceObjectClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{

	if (params->Value.size() != 3) {
		script->Script_Error("ProduceObjectClass::Function_Call -- Invalid Number of parameters: %d should be 3.", 
									params->Value.size());
	}
	SmartPtr<PlayerWrapper> player = LUA_SAFE_CAST(PlayerWrapper, params->Value[0]);
	SmartPtr<GameObjectTypeWrapper> object = LUA_SAFE_CAST(GameObjectTypeWrapper, params->Value[1]);
	if (!object) {
		LuaString *type_name = PG_Dynamic_Cast<LuaString>(params->Value[1]);
		if (type_name) {
			GameObjectTypeClass *type = GameObjectTypeManager.Find_Object_Type(type_name->Value.c_str());
			if (type) {
				object = GameObjectTypeWrapper::Create(type, script);
			}
		}
	}
	SmartPtr<GameObjectWrapper> planet = LUA_SAFE_CAST(GameObjectWrapper, params->Value[2]);

	if (!player || !object || !planet) {
		script->Script_Error("ProduceObjectClass::Function_Call -- Invalid parameter, Player: %s, Object: %s, Planet: %s",
									player ? "Ok" : "Invalid", object ? "Ok" : "Invalid", planet ? "Ok" : "Invalid");
		return NULL;
	}

	// Create a blocking status for this produce object call.
	ProductionBlockStatus *block = (ProductionBlockStatus *)ProductionBlockStatus::FactoryCreate();

	BudgetWrapper *budget = static_cast<BudgetWrapper*>(script->Map_Global_From_Lua("Budget"));

	block->Init(this, player, object, planet, (NewBuildId & 0x7f), budget);

	// Add the production event
	bool ok = ProduceObject(player->Get_Object(), object->Get_Object(), planet->Get_Object(), NewBuildId);

	NewBuildId++;

	if (!ok) {
		script->Script_Error("ProduceObjectClass::Function_Call -- Scheduling Production Event Failed!");
		return NULL;
	}

	return Return_Variable(block);
}

