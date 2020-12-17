// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaDiscreteDistribution.cpp#2 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/LuaDiscreteDistribution.cpp $
//
//    Original Author: James Yarrow
//
//            $Author: Brian_Hayes $
//
//            $Change: 641502 $
//
//          $DateTime: 2017/05/09 13:45:27 $
//
//          $Revision: #2 $
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma hdrstop

#include "LuaDiscreteDistribution.h"

PG_IMPLEMENT_RTTI(LuaDiscreteDistributionClass, LuaUserVar);
LUA_IMPLEMENT_FACTORY(LUA_CHUNK_DISCRETE_DISTRIBUTION, LuaDiscreteDistributionClass);

LuaDiscreteDistributionClass::LuaDiscreteDistributionClass()
{
	LUA_REGISTER_MEMBER_FUNCTION(LuaDiscreteDistributionClass, "Create", &LuaDiscreteDistributionClass::Create);
	LUA_REGISTER_MEMBER_FUNCTION_USE_MAPS(LuaDiscreteDistributionClass, "Insert", &LuaDiscreteDistributionClass::Insert);
	LUA_REGISTER_MEMBER_FUNCTION(LuaDiscreteDistributionClass, "Sample", &LuaDiscreteDistributionClass::Sample);
}


/**************************************************************************************************
* LuaDiscreteDistributionClass::Create -- Create a new distribution
*
* In:				
*
* Out:		
*
* History: 6/20/2005 1:42PM JSY
**************************************************************************************************/
LuaTable *LuaDiscreteDistributionClass::Create(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(FactoryCreate());
}

/**************************************************************************************************
* LuaDiscreteDistributionClass::Insert -- Add a weighted element to a distribution.  This function preserves
*	lua maps.
*
* In:				
*
* Out:		
*
* History: 6/20/2005 1:42PM JSY
**************************************************************************************************/
LuaTable *LuaDiscreteDistributionClass::Insert(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 2)
	{
		script->Script_Error("DiscreteDistribution::Insert -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaNumber> weight = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
	if (!weight)
	{
		script->Script_Error("DiscreteDistribution::Insert -- invalid type for parameter 2.  Expected number.");
		return NULL;
	}

	Distribution.Add_Element(params->Value[0], weight->Value);

	return NULL;
}

/**************************************************************************************************
* LuaDiscreteDistributionClass::Sample -- Randomly select an element from the collection according to the element weights
*
* In:				
*
* Out:		
*
* History: 6/20/2005 1:42PM JSY
**************************************************************************************************/
LuaTable *LuaDiscreteDistributionClass::Sample(LuaScriptClass *, LuaTable *)
{
	return Return_Variable(Distribution.Sample());
}

enum 
{
	DISTRIBUTION_DATA_CHUNK,
		DISTRIBUTION_SIZE_MICRO_CHUNK,
	DISTRIBUTION_ENTRY_CHUNK,
		DISTRIBUTION_ELEMENT_CHUNK,
		DISTRIBUTION_WEIGHT_CHUNK,
			DISTRIBUTION_WEIGHT_MICRO_CHUNK,

};

/**************************************************************************************************
* LuaDiscreteDistributionClass::Save -- 
*
* In:				
*
* Out:		
*
* History: 6/20/2005 1:42PM JSY
**************************************************************************************************/
bool LuaDiscreteDistributionClass::Save(LuaScriptClass *script, ChunkWriterClass *writer)
{
	bool ok = true;

	int distribution_size = static_cast<int>(Distribution.Num_Elements());

	ok &= writer->Begin_Chunk(DISTRIBUTION_DATA_CHUNK);
		WRITE_MICRO_CHUNK(DISTRIBUTION_SIZE_MICRO_CHUNK, distribution_size);
	ok &= writer->End_Chunk();

	for (int i = 0; i < distribution_size; ++i)
	{
		ok &= writer->Begin_Chunk(DISTRIBUTION_ENTRY_CHUNK);

			LUA_WRITE_CHUNK_VALUE_PTR(DISTRIBUTION_ELEMENT_CHUNK, Distribution.Get_Element_At_Index(i), script);

			float weight = Distribution.Get_Weight_At_Index(i);
			ok &= writer->Begin_Chunk(DISTRIBUTION_WEIGHT_CHUNK);
				WRITE_MICRO_CHUNK(DISTRIBUTION_WEIGHT_MICRO_CHUNK, weight);
			ok &= writer->End_Chunk();

		ok &= writer->End_Chunk();
	}

	return ok;
}

/**************************************************************************************************
* LuaDiscreteDistributionClass::Load -- 
*
* In:				
*
* Out:		
*
* History: 6/20/2005 1:42PM JSY
**************************************************************************************************/
bool LuaDiscreteDistributionClass::Load(LuaScriptClass *script, ChunkReaderClass *reader)
{
	bool ok = true;
	while (reader->Open_Chunk())
	{
		switch (reader->Cur_Chunk_ID())
		{
		case DISTRIBUTION_DATA_CHUNK:
			{
				int distribution_size = 0;
				while (reader->Open_Micro_Chunk())
				{
					switch (reader->Cur_Micro_Chunk_ID())
					{
						READ_MICRO_CHUNK(DISTRIBUTION_SIZE_MICRO_CHUNK, distribution_size);

					default:
						ok = false;
						assert(false);
						break;
					}
					reader->Close_Micro_Chunk();
				}
				Distribution.Reserve(distribution_size);
			}
			break;

		case DISTRIBUTION_ENTRY_CHUNK:
			{
				int final_index = static_cast<int>(Distribution.Num_Elements());
				Distribution.Add_Element(SmartPtr<LuaVar>(), 1.0f);
				float weight = 1.0f;
				while (reader->Open_Chunk())
				{
					switch (reader->Cur_Chunk_ID())
					{
						LUA_READ_CHUNK_VALUE_PTR(DISTRIBUTION_ELEMENT_CHUNK, Distribution.Get_Element_At_Index(final_index), script);

					case DISTRIBUTION_WEIGHT_CHUNK:
						while (reader->Open_Micro_Chunk())
						{
							switch (reader->Cur_Micro_Chunk_ID())
							{
								READ_MICRO_CHUNK(DISTRIBUTION_WEIGHT_MICRO_CHUNK, weight);

							default:
								ok = false;
								assert(false);
								break;
							}

							reader->Close_Micro_Chunk();
						}
						break;

					default:
						ok = false;
						assert(false);
						break;
					}

					reader->Close_Chunk();
				}
				Distribution.Set_Final_Weight(weight);
			}
			break;

		default:
			ok = false;
			assert(false);
			break;
		}

		reader->Close_Chunk();
	}

	return ok;
}
