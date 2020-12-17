// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ApplyMarkup.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/ApplyMarkup.cpp $
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


#include "ApplyMarkup.h"
#include "BlockingStatus.h"
#include "AI/LuaScript/AITargetLocationWrapper.h"
#include "AI/LuaScript/PlayerWrapper.h"
#include "AI/LuaScript/GameObjectWrapper.h"

#include "Player.h"
#include "GameObject.h"
#include "DynamicEnum.h"
#include "AI/AILog.h"
#include "AI/AIPlayer.h"
#include "AI/TacticalAIManager.h"
#include "AI/AITargetLocation.h"
#include "AI/Perception/AIPerceptionSystem.h"
#include "AI/Perception/Evaluators/PerceptualEvaluator.h"

class MarkupBlockStatus : public BlockingStatus, public SignalListenerClass
{
public:
	typedef BlockingStatus BASECLASS;

	PG_DECLARE_RTTI();
	LUA_DECLARE_FACTORY(LUA_CHUNK_APPLY_MARKUP_BLOCK, MarkupBlockStatus);

	MarkupBlockStatus() : MarkedValue(0.0f) {}
	~MarkupBlockStatus();

	void Init(float value);
	virtual LuaTable *Is_Finished(LuaScriptClass *, LuaTable *) { return Return_Variable(new LuaBool(false)); }
	virtual LuaTable *Result(LuaScriptClass *, LuaTable *) { return NULL; }

	void Register_Evaluator_As_Marked(PerceptualEvaluatorClass *evaluator);

	virtual void Receive_Signal(SignalGeneratorClass *gen, PGSignalType, SignalDataClass *);

	virtual bool Save(LuaScriptClass *script, ChunkWriterClass *writer);
	virtual bool Load(LuaScriptClass *script, ChunkReaderClass *reader);

protected:

	float MarkedValue;
	std::vector<PerceptualEvaluatorClass*> MarkedEvaluators;
};

PG_IMPLEMENT_RTTI(MarkupBlockStatus, BlockingStatus);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_APPLY_MARKUP_BLOCK, MarkupBlockStatus);

/**************************************************************************************************
* MarkupBlockStatus::~MarkupBlockStatus -- destructor.  Clears out markup
*
* In:			
*
* Out:		
*
* History: 12/20/2004 4:37PM JSY
**************************************************************************************************/
MarkupBlockStatus::~MarkupBlockStatus()
{
	for (unsigned int i = 0; i < MarkedEvaluators.size(); ++i)
	{
		if (MarkedEvaluators[i])
		{
			MarkedEvaluators[i]->Add_Markup_Value(-MarkedValue);
		}
	}
}

/**************************************************************************************************
* MarkupBlockStatus::Register_Evaluator_As_Marked -- Keep track of evaluators that have been marked up so
*	that we can undo the effect when this block is over
*
* In:			
*
* Out:		
*
* History: 12/20/2004 4:43PM JSY
**************************************************************************************************/
void MarkupBlockStatus::Register_Evaluator_As_Marked(PerceptualEvaluatorClass *evaluator)
{
	MarkedEvaluators.push_back(evaluator);
	SignalDispatcherClass::Get().Add_Listener(evaluator, this, PG_SIGNAL_OBJECT_DESTRUCTION);
}

/**************************************************************************************************
* MarkupBlockStatus::Receive_Signal -- Handle destruction of an evaluator that we've marked
*
* In:			
*
* Out:		
*
* History: 12/20/2004 4:43PM JSY
**************************************************************************************************/
void MarkupBlockStatus::Receive_Signal(SignalGeneratorClass *gen, PGSignalType, SignalDataClass *)
{
	for (unsigned int i = 0; i < MarkedEvaluators.size(); ++i)
	{
		if (MarkedEvaluators[i] == gen)
		{
			MarkedEvaluators[i] = NULL;
		}
	}
}

void MarkupBlockStatus::Init(float value)
{
	for (unsigned int i = 0; i < MarkedEvaluators.size(); ++i)
	{
		if (MarkedEvaluators[i])
		{
			MarkedEvaluators[i]->Add_Markup_Value(-MarkedValue);
		}
	}
	MarkedEvaluators.resize(0);
	SignalDispatcherClass::Get().Remove_Listener_From_All(this);
	MarkedValue = value;
}

enum
{
	CHUNK_ID_BASE_CLASS,
	CHUNK_ID_MARKUP_BLOCK_DATA,
	CHUNK_ID_EVALUATOR_SET,
	CHUNK_ID_ATOMIC_DATA,
	CHUNK_ID_MARKUP_VALUE,
	CHUNK_ID_LISTENER_BASE,
};

/**************************************************************************************************
* MarkupBlockStatus::Save -- Write the blocking object to file
*
* In:			
*
* Out:		
*
* History: 12/20/2004 4:43PM JSY
**************************************************************************************************/
bool MarkupBlockStatus::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = writer->Begin_Chunk(CHUNK_ID_BASE_CLASS);
		BASECLASS::Save(script, writer);
	ok &= writer->End_Chunk();

	ok &= writer->Begin_Chunk(CHUNK_ID_MARKUP_BLOCK_DATA);
		ok &= writer->Begin_Chunk(CHUNK_ID_ATOMIC_DATA);
			WRITE_MICRO_CHUNK_MULTI_BASE_PTR(	CHUNK_ID_LISTENER_BASE,		SignalListenerClass);
			WRITE_MICRO_CHUNK(						CHUNK_ID_MARKUP_VALUE,		MarkedValue);
		ok &= writer->End_Chunk();
		WRITE_POINTER_STL_VECTOR(CHUNK_ID_EVALUATOR_SET, MarkedEvaluators);
	ok &= writer->End_Chunk();

	return ok;
}

/**************************************************************************************************
* MarkupBlockStatus::Load -- Read the blocking object from file
*
* In:			
*
* Out:		
*
* History: 12/20/2004 4:55PM JSY
**************************************************************************************************/
bool MarkupBlockStatus::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;

	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
			case CHUNK_ID_BASE_CLASS:
				ok &= BASECLASS::Load(script, reader);
				break;

			case CHUNK_ID_MARKUP_BLOCK_DATA:
				while (reader->Open_Chunk())
				{
					switch (reader->Cur_Chunk_ID())
					{
						READ_POINTER_STL_VECTOR(CHUNK_ID_EVALUATOR_SET, MarkedEvaluators);

						case CHUNK_ID_ATOMIC_DATA:
							while (reader->Open_Micro_Chunk())
							{
								switch (reader->Cur_Micro_Chunk_ID())
								{
									READ_MICRO_CHUNK_MULTI_BASE_PTR( CHUNK_ID_LISTENER_BASE,    SignalListenerClass);
									READ_MICRO_CHUNK(                CHUNK_ID_MARKUP_VALUE,     MarkedValue);

									default:
										assert(false);
										ok = false;
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



PG_IMPLEMENT_RTTI(ApplyMarkupClass, LuaUserVar);

/**************************************************************************************************
* ApplyMarkupClass::~Function_Call -- Entry point from script
*
* In:			
*
* Out:		
*
* History: 12/20/2004 4:37PM JSY
**************************************************************************************************/
LuaTable *ApplyMarkupClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	//Must have 3 or 4 parameters.
	if (params->Value.size() != 3 && params->Value.size() != 4)
	{
		script->Script_Error("Apply_Markup - Invalid number of parameters.  Expected 3 or 4, got %d.", params->Value.size());
		return NULL;
	}

	//Parameter 1 must be player requesting the markup
	SmartPtr<PlayerWrapper> my_player = PG_Dynamic_Cast<PlayerWrapper>(params->Value[0]);
	if (!my_player)
	{
		script->Script_Error("Apply_Markup - Invalid type for parameter 1.  Expected player.");
		return NULL;
	}

	//Parameter 2 must be game object or AITarget or player object, or else table thereof
	SmartPtr<GameObjectWrapper> object_target = PG_Dynamic_Cast<GameObjectWrapper>(params->Value[1]);
	SmartPtr<AITargetLocationWrapper> ai_target = PG_Dynamic_Cast<AITargetLocationWrapper>(params->Value[1]);
	SmartPtr<PlayerWrapper> player_target = PG_Dynamic_Cast<PlayerWrapper>(params->Value[1]);
	SmartPtr<LuaTable> table_target = PG_Dynamic_Cast<LuaTable>(params->Value[1]);
	
	//Parameter 3 must be value of markup
	SmartPtr<LuaNumber> markup_value = PG_Dynamic_Cast<LuaNumber>(params->Value[2]);
	if (!markup_value)
	{
		script->Script_Error("Apply_Markup - Invalid type for parameter 3.  Expected number.");
		return NULL;
	}

	MarkupBlockStatus *bs = NULL;
	if (params->Value.size() == 4)
	{
		bs = PG_Dynamic_Cast<MarkupBlockStatus>(params->Value[3]);
	}
	if (!bs)
	{
		bs = static_cast<MarkupBlockStatus*>(MarkupBlockStatus::FactoryCreate());
	}
	FAIL_IF(!bs) { return NULL; }

	bs->Init(markup_value->Value);

	if (object_target)
	{
		Apply_Markup(my_player->Get_Object(), object_target->Get_Object(), markup_value->Value, bs);
	}
	else if (ai_target)
	{
		Apply_Markup(ai_target->Get_Object(), markup_value->Value, bs);
	}
	else if (player_target)
	{
		Apply_Markup(my_player->Get_Object(), player_target->Get_Object(), markup_value->Value, bs);
	}
	else if (table_target)
	{
		Apply_Markup(my_player->Get_Object(), table_target, markup_value->Value, bs);
	}
	else
	{
		script->Script_Error("Apply_Markup - Invalid type for parameter 2.  Expected player(s) or ai target(s) or game object(s)");
		return NULL;
	}

	return Return_Variable(bs);
}

/**************************************************************************************************
* ApplyMarkupClass::~Apply_Markup -- Handle case where user passes target location for markup
*
* In:			
*
* Out:		
*
* History: 12/20/2004 4:39PM JSY
**************************************************************************************************/
void ApplyMarkupClass::Apply_Markup(const AITargetLocationClass *target, float value, MarkupBlockStatus *blocking_status)
{
	Apply_Markup(target->Get_Evaluator(), value, blocking_status);
}

/**************************************************************************************************
* ApplyMarkupClass::~Apply_Markup -- Handle case where user passes set of targets (possibly of different types) for markup
*
* In:			
*
* Out:		
*
* History: 12/20/2004 4:39PM JSY
**************************************************************************************************/
void ApplyMarkupClass::Apply_Markup(PlayerClass *for_player, LuaTable *target_list, float value, MarkupBlockStatus *blocking_status)
{
	FAIL_IF(!for_player) { return; }
	for (unsigned int i = 0; i < target_list->Value.size(); ++i)
	{
		SmartPtr<GameObjectWrapper> object_target = PG_Dynamic_Cast<GameObjectWrapper>(target_list->Value[i]);
		SmartPtr<AITargetLocationWrapper> ai_target = PG_Dynamic_Cast<AITargetLocationWrapper>(target_list->Value[i]);
		SmartPtr<PlayerWrapper> player_target = PG_Dynamic_Cast<PlayerWrapper>(target_list->Value[i]);

		if (object_target)
		{
			Apply_Markup(for_player, object_target->Get_Object(), value, blocking_status);
		}
		else if (ai_target)
		{
			Apply_Markup(ai_target->Get_Object(), value, blocking_status);
		}
		else if (player_target)
		{
			Apply_Markup(for_player, player_target->Get_Object(), value, blocking_status);
		}
		else
		{
			assert(false);
		}
	}
}

/**************************************************************************************************
* ApplyMarkupClass::~Apply_Markup -- Handle case where user passes single game object for markup
*
* In:			
*
* Out:		
*
* History: 12/20/2004 4:39PM JSY
**************************************************************************************************/
void ApplyMarkupClass::Apply_Markup(PlayerClass *for_player, GameObjectClass *target, float value, MarkupBlockStatus *blocking_status)
{
	FAIL_IF(!for_player) { return; }

	if (!target)
	{
		return;
	}

	AIPlayerClass *ai_player = for_player->Get_AI_Player();
	FAIL_IF(!ai_player) { return; }
	TacticalAIManagerClass *manager = ai_player->Get_Tactical_Manager_By_Mode(GameModeManager.Get_Sub_Type());
	FAIL_IF(!manager) { return; }
	AIPerceptionSystemClass *perception_system = manager->Get_Perception_System();
	FAIL_IF(!perception_system) { return; }

	//Planets and tactical mode units are handled differently
	PerceptionTokenType token;
	if (target->Behaves_Like(BEHAVIOR_PLANET))
	{
		token = target->Get_Planetary_Data()->Get_Evaluator_Token();
		FAIL_IF (token == PERCEPTION_TOKEN_INVALID)
		{
			AIERROR( ("There's no perception token for planet %s! How can this be?", target->Get_Type()->Get_Name()->c_str()) );
			return;
		}
	}
	else
	{
		FAIL_IF (!ThePerceptionTokenTypeConverterPtr->String_To_Enum(AIPerceptionSystemClass::Build_Game_Object_Token_String(target), token))
		{
			AIERROR( ("There's no perception token for game object with ID %d! How can this be?", target->Get_ID()) );
			return;
		}
	}

	PerceptualEvaluatorClass *evaluator = perception_system->Find_Root_Evaluator_By_Token(token);
	FAIL_IF (!evaluator)
	{
		AIERROR( ("Can't find evaluator for object (type %s, id %s)", target->Get_Type()->Get_Name()->c_str(), target->Get_ID()) );
		return;
	}

	Apply_Markup(evaluator, value, blocking_status);
}

/**************************************************************************************************
* ApplyMarkupClass::~Apply_Markup -- Handle case where user passes player for markup
*
* In:			
*
* Out:		
*
* History: 12/20/2004 4:39PM JSY
**************************************************************************************************/
void ApplyMarkupClass::Apply_Markup(PlayerClass *for_player, PlayerClass *target, float value, MarkupBlockStatus *blocking_status)
{
	FAIL_IF(!for_player) { return; }

	if (!target)
	{
		return;
	}

	AIPlayerClass *ai_player = for_player->Get_AI_Player();
	FAIL_IF(!ai_player) { return; }
	TacticalAIManagerClass *manager = ai_player->Get_Tactical_Manager_By_Mode(GameModeManager.Get_Sub_Type());
	FAIL_IF(!manager) { return; }
	AIPerceptionSystemClass *perception_system = manager->Get_Perception_System();
	FAIL_IF(!perception_system) { return; }

	PerceptionTokenType token = target->Get_Evaluator_Token();
	PerceptualEvaluatorClass *evaluator = perception_system->Find_Root_Evaluator_By_Token(token);
	FAIL_IF (!evaluator)
	{
		AIERROR( ("Can't find evaluator for player id %s", target->Get_ID()) );
		return;
	}

	Apply_Markup(evaluator, value, blocking_status);
}

/**************************************************************************************************
* ApplyMarkupClass::~Apply_Markup -- Helper function to perform the markup and register it with the blocking object
*
* In:			
*
* Out:		
*
* History: 12/20/2004 4:39PM JSY
**************************************************************************************************/
void ApplyMarkupClass::Apply_Markup(PerceptualEvaluatorClass *evaluator, float value, MarkupBlockStatus *blocking_status)
{
	evaluator->Add_Markup_Value(value);
	blocking_status->Register_Evaluator_As_Marked(evaluator);
}
 