#include "stdafx.h"
#include "StatesAndTransitions.h"

#include "projects/Shared/NavigationColliderElement.h"
#include "projects/Movement/SteeringBehaviors/PathFollow/PathFollowSteeringBehavior.h"
// FSM TODO:

//------------
//---STATES---
//------------
using namespace FSMState;
void PatrolState::OnEnter(Blackboard* pBlackboard)
{
	SmartAgent* pGuardAgent{};
	pBlackboard->GetData("Guard", pGuardAgent);
	std::vector<Vector2> patrolPath{};
	pBlackboard->GetData("PatrolPath", patrolPath);
	PathFollow* pGuardPathFollow{};
	pBlackboard->GetData("GuardPathFollow", pGuardPathFollow);
	pGuardPathFollow->SetPath(patrolPath);
	pGuardAgent->SetSteeringBehavior(pGuardPathFollow);
}

void PatrolState::Update(Blackboard* pBlackboard, float deltaTime)
{
	PathFollow* pGuardPathFollow{};
	pBlackboard->GetData("GuardPathFollow", pGuardPathFollow);
	if (pGuardPathFollow->HasArrived())
	{
		PatrolState::OnEnter(pBlackboard);
	}
}

void ChaseState::OnEnter(Blackboard* pBlackboard)
{
	SmartAgent* pGuardAgent{};
	pBlackboard->GetData("Guard", pGuardAgent);
	pGuardAgent->SetSteeringBehavior(pGuardAgent->GetSeekBehavior());
	

}

void ChaseState::Update(Blackboard* pBlackboard, float deltaTime)
{
	SmartAgent* pGuardAgent{};
	pBlackboard->GetData("Guard", pGuardAgent);
	SteeringAgent* pPlayer{};
	pBlackboard->GetData("Player", pPlayer);
	pGuardAgent->GetSeekBehavior()->SetTarget(pPlayer->GetPosition());
}

void SearchState::OnEnter(Blackboard* pBlackboard)
{
	SmartAgent* pGuardAgent{};
	pBlackboard->GetData("Guard", pGuardAgent);
	pGuardAgent->SetSteeringBehavior(pGuardAgent->GetWanderBehavior());
	pBlackboard->ChangeData("SearchTime", 0.0f);

}

void SearchState::Update(Blackboard* pBlackboard, float deltaTime)
{
	float searchTime{};
	pBlackboard->GetData("SearchTime", searchTime);
	searchTime += deltaTime;
	pBlackboard->ChangeData("SearchTime", searchTime);
}



//-----------------
//---TRANSITIONS---
//-----------------

using namespace FSMCondition;
bool IsTargetVisible::Evaluate(Blackboard* pBlackboard) const
{
	SmartAgent* pGuardAgent{};
	pBlackboard->GetData("Guard", pGuardAgent);
	SteeringAgent* pPlayer{};
	pBlackboard->GetData("Player", pPlayer);
	float detectionRadius{};
	pBlackboard->GetData("DetectionRadius", detectionRadius);


	return pGuardAgent->HasLineOfSight(pPlayer->GetPosition()) && 
		pGuardAgent->GetPosition().DistanceSquared(pPlayer->GetPosition()) <= detectionRadius* detectionRadius;
}

bool IsSearchingTooLong::Evaluate(Blackboard* pBlackboard) const
{
	float searchTime{};
	pBlackboard->GetData("SearchTime", searchTime);
	float maxSearchTime{};
	pBlackboard->GetData("MaxSearchTime", maxSearchTime);

	return searchTime > maxSearchTime;
}
