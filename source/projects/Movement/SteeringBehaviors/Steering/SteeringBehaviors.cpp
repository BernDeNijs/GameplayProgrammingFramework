//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework/EliteMath/EMatrix2x3.h"

#include <numbers>
using namespace Elite;

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	return Seek::CalculateSteering(deltaT, pAgent, m_Target);
}

SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent, TargetData altTarget)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = altTarget.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}

	return steering;
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	const TargetData altTarget = pAgent->GetPosition() - (m_Target.Position - pAgent->GetPosition());
	steering = Seek::CalculateSteering(deltaT, pAgent, altTarget);

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}



	return steering;
}

//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	const float slowArea{ m_Outer };
	const float stopDistance{ m_InnerRadius };

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();


	const float distance = steering.LinearVelocity.Magnitude();
	steering.LinearVelocity.Normalize();



	if (abs(distance - stopDistance) < 0.01f)
	{
		steering.LinearVelocity *= 0.f;
	}
	else if (distance < slowArea)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * ((distance - stopDistance) / slowArea);
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}


	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, slowArea, Color{ 0.f,0.f,1.f },DEBUGRENDERER2D->NextDepthSlice());
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, stopDistance, Color{ 1.f,0.f,0.f }, DEBUGRENDERER2D->NextDepthSlice());
	}

	return steering;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput  steering{};
	pAgent->SetAutoOrient(false);

	float angle = VectorToOrientation(m_Target.Position - pAgent->GetPosition()) - pAgent->GetRotation();
	constexpr auto floatPi = static_cast<float>(E_PI);
	if (angle <= -floatPi)
	{
		angle += 2 * floatPi;
	}
	if (angle > floatPi)
	{
		angle -= 2 * floatPi;
	}

	if (angle > 0)
	{
		steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
	}
	else
	{
		steering.AngularVelocity = -pAgent->GetMaxAngularSpeed();
	}

	return steering;
}

//WANDER
//****
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	
	m_Angle +=  (Elite::randomFloat(0,1) * (m_MaxAngle)) - (m_MaxAngle / 2.f) /** deltaT*/ ;
	Elite::ClampRef(m_Angle, Elite::ToRadians(-90), Elite::ToRadians(90));

	const Elite::Vector2 wanderCircleCenter{ pAgent->GetPosition() + pAgent->GetLinearVelocity().GetNormalized() * m_Offset };
	const Elite::Vector2 vecFromCenter{ cosf(m_Angle) * m_Radius,sinf(m_Angle) * m_Radius };

	m_Target.Position = (wanderCircleCenter + vecFromCenter);



	if (pAgent->CanRenderBehavior())
	{
		
		DEBUGRENDERER2D->DrawCircle(wanderCircleCenter, m_Radius, { 0.f, 0.f, 1.f, 0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawSolidCircle(m_Target.Position, 0.1f, {}, { 0.f, 1.f, 0.f }, 0.40f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), m_Offset, {0.f, 0.f, 1.f, 0.5f}, 0.40f);

		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), pAgent->GetLinearVelocity().Magnitude(), { 1.f, 0.f, 1.f, 0.5f }, 0.40f);
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}

//PURSUIT
//****
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	TargetData altTarget = m_Target;
	if (m_Target.LinearVelocity.Magnitude() > 0.f)
	{
		altTarget.Position += m_Target.LinearVelocity * (m_Target.Position - pAgent->GetPosition()).Magnitude() / pAgent->GetMaxLinearSpeed();
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawSolidCircle(altTarget.Position, 0.25f, {}, { 1.f, 0.f, 1.f }, DEBUGRENDERER2D->NextDepthSlice());
	}

	return Seek::CalculateSteering(deltaT, pAgent, altTarget);
}

//EVADE
//****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const float distanceToTargetSquared = pAgent->GetPosition().DistanceSquared(m_Target.Position);
	
	if (m_EvadeRange != -1)
	{
		if (m_EvadeRange * m_EvadeRange < distanceToTargetSquared)
		{
			SteeringOutput output{};
			output.IsValid = false;
			return output;
		}
	}

	SteeringOutput steering{ Pursuit::CalculateSteering(deltaT, pAgent) };
	steering.LinearVelocity = -steering.LinearVelocity;

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_EvadeRange, {0.f, 0.f, 1.f, 0.5f}, 0.40f);
	}

	return steering;
}