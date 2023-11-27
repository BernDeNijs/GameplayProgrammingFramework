/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/DecisionMaking/SmartAgent.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "projects/Movement/SteeringBehaviors/PathFollow/PathFollowSteeringBehavior.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
// BT TODO:
using namespace Elite;


template<typename T> bool LoadFromBlackboard(Blackboard* pBlackboard, const std::string& name, T& data,const std::string& functionName)
{
	
	if (const bool loadedObject = pBlackboard->GetData(name, data); loadedObject)
	{
		return true;
	}
	std::cout << "Blackboard failed to load (" << name << ") in function (" << functionName << ")\n";
	return false;
	
}


namespace BT_Actions 
{
	BehaviorState ChangeToPatrol(Blackboard* pBlackboard)
	{
		const std::string functionName = "ChangeToPatrol";

		SmartAgent* pAgent;
		if (!LoadFromBlackboard(pBlackboard, "Agent", pAgent, functionName)) return BehaviorState::Failure;
		PathFollow* pPathFollow;
		if (!LoadFromBlackboard(pBlackboard, "PatrolBehavior", pPathFollow, functionName)) return BehaviorState::Failure;

		pAgent->SetSteeringBehavior(pPathFollow);
		if (pPathFollow->HasArrived())
		{
			std::vector<Vector2> path;
			if (!LoadFromBlackboard(pBlackboard, "PatrolPath", path, functionName)) return BehaviorState::Failure;
			pPathFollow->SetPath(path);
		}

		return BehaviorState::Success;
	}

	BehaviorState ChangeToChase(Blackboard* pBlackboard)
	{
		const std::string functionName = "ChangeToChase";

		SmartAgent* pAgent;
		if (!LoadFromBlackboard(pBlackboard, "Agent", pAgent, functionName)) return BehaviorState::Failure;
		SteeringAgent* pPlayer;
		if (!LoadFromBlackboard(pBlackboard, "TargetAgent", pPlayer, functionName)) return BehaviorState::Failure;

		const auto seek = pAgent->GetSeekBehavior();
		seek->SetTarget(pPlayer->GetPosition());
		pAgent->SetSteeringBehavior(seek);
		
		return BehaviorState::Success;
	}
	BehaviorState SeekSearchpoint(Blackboard* pBlackboard)
	{
		const std::string functionName = "SeekSearchpoint";

		constexpr float arriveRadius = 2.f;

		SmartAgent* pAgent;
		if (!LoadFromBlackboard(pBlackboard, "Agent", pAgent, functionName)) return BehaviorState::Failure;
		SteeringAgent* pPlayer;
		if (!LoadFromBlackboard(pBlackboard, "TargetAgent", pPlayer, functionName)) return BehaviorState::Failure;

		const auto seek = pAgent->GetSeekBehavior();
		seek->SetTarget(pPlayer->GetPosition());
		pAgent->SetSteeringBehavior(seek);
		if (pAgent->GetPosition().DistanceSquared(pPlayer->GetPosition()) <= arriveRadius* arriveRadius)
		{
			return BehaviorState::Success;
		}
		return BehaviorState::Running;
	}
	BehaviorState SearchAround(Blackboard* pBlackboard)
	{
		const std::string functionName = "SearchAround";


		SmartAgent* pAgent;
		if (!LoadFromBlackboard(pBlackboard, "Agent", pAgent, functionName)) return BehaviorState::Failure;
		

		const auto wander = pAgent->GetWanderBehavior();
		pAgent->SetSteeringBehavior(wander);
		return BehaviorState::Running;
	}


	BehaviorState Template(Blackboard* pBlackboard)
	{
		const std::string functionName = "Template";

		return BehaviorState::Failure;
	}

}

namespace BT_Conditions
{
	bool HasSeenPlayer(Blackboard* pBlackboard)
	{
		const std::string functionName = "HasSeenPlayer";

		SmartAgent* pGuard;
		if (!LoadFromBlackboard(pBlackboard, "Agent", pGuard, functionName)) return false;

		SteeringAgent* pPlayer;
		if (!LoadFromBlackboard(pBlackboard, "TargetAgent", pPlayer, functionName)) return false;

		float detectionRadius;
		if (!LoadFromBlackboard(pBlackboard, "DetectionRadius", detectionRadius, functionName)) return false;

		const bool hasLineOfSight = pGuard->HasLineOfSight(pPlayer->GetPosition());
		const bool isInRadius = pGuard->GetPosition().DistanceSquared(pPlayer->GetPosition()) <= detectionRadius * detectionRadius;

		

		if (hasLineOfSight && isInRadius)
		{
			const auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch());
			pBlackboard->ChangeData("TimeLastSeen", currentTime);
			pBlackboard->ChangeData("LastKnownLocation", pPlayer->GetPosition());
			pBlackboard->ChangeData("PlayerSpottedRecently", true);
			return true;
		}
		return false;

	}
	bool ShouldSearch(Blackboard* pBlackboard)
	{
		const std::string functionName = "ShouldSearch";
		bool recentlySpotted;
		if (!LoadFromBlackboard(pBlackboard, "PlayerSpottedRecently", recentlySpotted, functionName)) return false;	
		return recentlySpotted;
	}

	bool IsSearchingTooLong(Blackboard* pBlackboard)
	{
		const std::string functionName = "IsSearchingTooLong";

		
		std::chrono::milliseconds timeLastSeen;
		if (!LoadFromBlackboard(pBlackboard, "TimeLastSeen", timeLastSeen, functionName)) return false;
		float maxSearchTime;
		if (!LoadFromBlackboard(pBlackboard, "MaxSearchTime", maxSearchTime, functionName)) return false;

		const auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());

		const auto timeSearching = currentTime - timeLastSeen;

		if (timeSearching.count() > maxSearchTime * 1000)
		{
			pBlackboard->ChangeData("PlayerSpottedRecently", false);
			return true;
		}
	}


	bool Template(Blackboard* pBlackboard)
	{
		const std::string functionName = "Template";
		return false;
	}

}


#endif