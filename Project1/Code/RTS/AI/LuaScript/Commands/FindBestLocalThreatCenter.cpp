// $Id: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindBestLocalThreatCenter.cpp#1 $
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
//              $File: //depot/Projects/StarWars_Steam/FOC/Code/RTS/AI/LuaScript/Commands/FindBestLocalThreatCenter.cpp $
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

#pragma hdrstop

#include "FindBestLocalThreatCenter.h"

#include "AI/LuaScript/GameObjectWrapper.h"
#include "AI/LuaScript/PositionWrapper.h"

#include "FrameSynchronizer.h"

PG_IMPLEMENT_RTTI(FindBestLocalThreatCenterClass, LuaUserVar);

/**************************************************************************************************
* FindBestLocalThreatCenterClass::Function_Call -- script command to find the location at which to place a circle
*	of fixed radius in order to cover a subset of the passed objects that generate the maximum threat.
*
* In:				
*
* Out:		
*
* History: 11/9/2005 3:58PM JSY
**************************************************************************************************/
LuaTable *FindBestLocalThreatCenterClass::Function_Call(LuaScriptClass *script, LuaTable *params)
{
	if (params->Value.size() != 2)
	{
		script->Script_Error("Find_Best_Local_Threat_Center -- invalid number of parameters.  Expected 2, got %d.", params->Value.size());
		return NULL;
	}

	SmartPtr<LuaTable> lua_object_table = PG_Dynamic_Cast<LuaTable>(params->Value[0]);
	if (!lua_object_table)
	{
		script->Script_Error("Find_Best_Local_Threat_Center -- invalid type for parameter 1.  Expected table.");
		return NULL;
	}

	SmartPtr<LuaNumber> radius = PG_Dynamic_Cast<LuaNumber>(params->Value[1]);
	if (!radius)
	{
		script->Script_Error("Find_Best_Local_Threat_Center -- invalid type for parameter 2.  Expected number.");
		return NULL;
	}

	//Translate the lua table into real game objects
	static std::vector<GameObjectClass*> object_list;
	object_list.resize(0);
	for (unsigned int i = 0; i < lua_object_table->Value.size(); ++i)
	{
		SmartPtr<GameObjectWrapper> object_wrapper = PG_Dynamic_Cast<GameObjectWrapper>(lua_object_table->Value[i]);
		if (!object_wrapper)
		{
			script->Script_Warning("Find_Best_Local_Threat_Center -- invalid type for entry at index %d in object table.", i);
			continue;
		}

		if (!object_wrapper->Get_Object())
		{
			//We'll treat this as resonable
			continue;
		}

		object_list.push_back(object_wrapper->Get_Object());
		FrameSynchronizerClass::Print_Sync_Message_No_Stack(SYNC_LOG_LUA_CRC, "FindBestLocalThreatCenterClass::Function_Call -- Added object ID %d\n", object_wrapper->Get_Object()->Get_ID());
	}


	/*
	** Well, I don't know what the heck is going on here but a mega-kludge seems to be in order. ST - 2/11/2006 3:38AM
	*/
	for (unsigned int bub1=0 ; bub1 < object_list.size() ; bub1++) {
		for (unsigned int bub2=0 ; bub2 < object_list.size() - 1 ; bub2++) {
			if (object_list[bub2]->Get_ID() > object_list[bub2+1]->Get_ID()) {
				GameObjectClass *temp = object_list[bub2];
				object_list[bub2] = object_list[bub2 + 1];
				object_list[bub2 + 1] = temp;
			}
		}
	}

	FrameSynchronizerClass::Print_Sync_Message_No_Stack(SYNC_LOG_LUA_CRC, "FindBestLocalThreatCenterClass::Function_Call -- object_list.size() = %d, radius = %f\n", object_list.size(), radius->Value);
	
	if (object_list.size() == 0)
	{
		script->Script_Warning("Find_Best_Local_Threat_Center -- no objects supplied");
		return NULL;
	}

	//Take a guess at good center points by doing clustering 
	static std::vector<Vector2> cluster_centers;
	cluster_centers.resize(0);
	Find_Clusters(cluster_centers, object_list);

	FrameSynchronizerClass::Print_Sync_Message_No_Stack(SYNC_LOG_LUA_CRC, "FindBestLocalThreatCenterClass::Function_Call -- cluster_centers.size() = %d\n", cluster_centers.size());

	//Now score the cluster centers based on AI combat power within the radius and hand back the best
	float radius2 = radius->Value * radius->Value;
	Vector2 best_center = Vector2(0.0f, 0.0f);
	float best_score = 0.0f;
	for (unsigned int i = 0; i < cluster_centers.size(); ++i)
	{
		float score = 0.0f;
		for (unsigned int j = 0; j < object_list.size(); ++j)
		{
			if ((object_list[j]->Get_Position().Project_XY() - cluster_centers[i]).Length2() <= radius2)
			{
				score += object_list[j]->Get_Company_Type()->Get_AI_Combat_Power_Metric();
			}
		}

		if (score >= best_score)
		{
			best_center = cluster_centers[i];
			best_score = score;
		}
	}

	FrameSynchronizerClass::Print_Sync_Message_No_Stack(SYNC_LOG_LUA_CRC, "FindBestLocalThreatCenterClass::Function_Call -- best_score = %f\n", best_score);

	if (best_score <= 0.0f)
	{
		return NULL;
	}
	else
	{
		LuaTable *return_table = Alloc_Lua_Table();
		return_table->Value.push_back(PositionWrapper::Create(Vector3(best_center)));
		return_table->Value.push_back(new LuaNumber(best_score));
		return return_table;
	}
}

/**************************************************************************************************
* FindBestLocalThreatCenterClass::Find_Clusters -- Split objects up into a set of clusters and report back
*	the cluster centers.
*
* In:				
*
* Out:		
*
* History: 11/9/2005 3:58PM JSY
**************************************************************************************************/
void FindBestLocalThreatCenterClass::Find_Clusters(std::vector<Vector2> &clusters, const std::vector<GameObjectClass*> &object_list)
{
	static const unsigned int BUCKET_DIVISOR = 4;
	static const int ITERATION_COUNT = 5;
	static const unsigned int MAX_CLUSTERS = 10;

	//Scale the number of clusters to the total number of objects but don't let it get too big.
	unsigned int num_clusters = (object_list.size() + BUCKET_DIVISOR - 1) / BUCKET_DIVISOR;
	num_clusters = Min(num_clusters, MAX_CLUSTERS);

	//If we're only going to generate one cluster then just go with the centroid since it's what the
	//clustering algorithm is going to generate anyway
	if (num_clusters <= 1)
	{
		Vector2 centroid = Vector2(0.0f, 0.0f);
		for (unsigned int i = 0; i < object_list.size(); ++i)
		{
			centroid += object_list[i]->Get_Position().Project_XY();
		}
		centroid /= static_cast<float>(object_list.size());
		clusters.push_back(centroid);
		return;
	}

	//Initialize with random points within the boudning rectangle
	clusters.resize(num_clusters);
	float x_min = BIG_FLOAT;
	float x_max = -BIG_FLOAT;
	float y_min = BIG_FLOAT;
	float y_max = -BIG_FLOAT;
	for (unsigned int i = 0; i < object_list.size(); ++i)
	{
		x_max = Max(x_max, object_list[i]->Get_Position().X);
		x_min = Min(x_min, object_list[i]->Get_Position().X);
		//y_max = Max(x_max, object_list[i]->Get_Position().Y);	ST - 2/11/2006 3:38AM
		y_max = Max(y_max, object_list[i]->Get_Position().Y);
		y_min = Min(y_min, object_list[i]->Get_Position().Y);
	}

	FrameSynchronizerClass::Print_Sync_Message_No_Stack(SYNC_LOG_LUA_CRC, "FindBestLocalThreatCenterClass::Find_Clusters -- clusters.size() = %d\n", clusters.size());

	for (unsigned int i = 0; i < clusters.size(); ++i)
	{
		clusters[i] = Vector2(Get_Random_Uniform_Float(x_min, x_max), Get_Random_Uniform_Float(y_min, y_max));
	}

	static std::vector<unsigned int> bucket_assignments;
	static std::vector<float> normalization_factors;
	bucket_assignments.resize(object_list.size());
	normalization_factors.resize(clusters.size());

	/*
	** Test and set. ST - 2/11/2006 3:39AM (looking for sync bugs...)
	*/
	unsigned int test = 0;
	for (unsigned int b=0 ; b<bucket_assignments.size() ; b++) {
		//test += bucket_assignments[b];
		bucket_assignments[b] = 0;
	}
	FrameSynchronizerClass::Print_Sync_Message_No_Stack(SYNC_LOG_LUA_CRC, "FindBestLocalThreatCenterClass::Find_Clusters -- bucket_test = %d, size = %d\n", test, bucket_assignments.size());

	for (int i = 0; i < ITERATION_COUNT; ++i)
	{
		//Assign each object to a cluster
		for (unsigned int object_index = 0; object_index < object_list.size(); ++object_index)
		{
			float best_distance2 = BIG_FLOAT;
			for (unsigned int cluster_index = 0; cluster_index < clusters.size(); ++cluster_index)
			{
				float distance2 = (object_list[object_index]->Get_Position().Project_XY() - clusters[cluster_index]).Length2();
				if (distance2 < best_distance2)
				{
					bucket_assignments[object_index] = cluster_index;
					best_distance2 = distance2;
				}
			}
		}

		//Reset the normalization factors before generating the cluster centers for the next iteration
		for (unsigned int cluster_index = 0; cluster_index < clusters.size(); ++cluster_index)
		{
			normalization_factors[cluster_index] = 0.0f;
		}

		//Move the cluster point to the centroid of the objects that have been assigned to it.
		for (unsigned int object_index = 0; object_index < object_list.size(); ++object_index)
		{
			unsigned int cluster_index = bucket_assignments[object_index];
			if (normalization_factors[cluster_index] == 0.0f)
			{
				clusters[cluster_index] = Vector2(0.0f, 0.0f);
			}

			clusters[cluster_index] += object_list[object_index]->Get_Position().Project_XY();
			++normalization_factors[cluster_index];
		}

		for (unsigned int cluster_index = 0; cluster_index < clusters.size(); ++cluster_index)
		{
			if (normalization_factors[cluster_index] > 0.0f)
			{
				clusters[cluster_index] /= normalization_factors[cluster_index];
			}
		}
	}

	for (unsigned int c=0 ; c<clusters.size() ; c++) {
		FrameSynchronizerClass::Print_Sync_Message_No_Stack(SYNC_LOG_LUA_CRC, "FindBestLocalThreatCenterClass::Find_Clusters -- returning cluster %f, %f\n", clusters[c].X, clusters[c].Y);
	}
}
