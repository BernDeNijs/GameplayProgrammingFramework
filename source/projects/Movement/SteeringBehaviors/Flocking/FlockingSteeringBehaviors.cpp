#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	//m_Target= m_pFlock->GetAverageNeighborPos();

	return Seek::CalculateSteering(deltaT, pAgent , m_pFlock->GetAverageNeighborPos());
	//return SteeringOutput{};
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering;
	const auto neighbors = m_pFlock->GetNeighbors();
	const auto nrOfNeighbors = m_pFlock->GetNrOfNeighbors();
	const auto agentPos = pAgent->GetPosition();
	for (int i = 0; i < nrOfNeighbors; i++)
	{
		const auto neighborPos = neighbors[i]->GetPosition();
		const float distance = agentPos.Distance(neighborPos);
		steering.LinearVelocity += (neighborPos - agentPos).GetNormalized()/distance;
	}
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	return steering;
}