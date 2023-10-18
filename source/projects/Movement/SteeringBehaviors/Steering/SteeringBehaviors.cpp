//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"
#include <limits>

using namespace Elite;

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	return Seek::CalculateSteering(deltaT, pAgent, m_Target);
}

SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent, const TargetData& altTarget)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = altTarget.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}

	return steering;
}

//Flee
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	const TargetData altTarget = pAgent->GetPosition() - (m_Target.Position - pAgent->GetPosition());
	steering = Seek::CalculateSteering(deltaT, pAgent, altTarget);

	if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, { 1.f, 0.f, 0.f });
	}

	return steering;
}

//Arrive
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	Vector2 currentPos = pAgent->GetPosition();
	Vector2 targetPos = m_Target.Position;

	Vector2 toTarget = targetPos - currentPos;

	float speed = pAgent->GetMaxLinearSpeed();

	float distance = toTarget.Magnitude();
	float a = distance - m_TargetRadius;
	float b = m_SlowRadius - m_TargetRadius;

	if (distance < m_SlowRadius)
	{
		speed *= (a / b);
	}

	steering.LinearVelocity = toTarget.GetNormalized() * speed;

	if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawDirection(currentPos, steering.LinearVelocity, 5.f, { 1.f, 0.f, 0.f });
		DEBUGRENDERER2D->DrawCircle(currentPos, m_SlowRadius, { 0.f, 0.f, 1.f }, DEBUGRENDERER2D->NextDepthSlice());
		DEBUGRENDERER2D->DrawCircle(currentPos, m_TargetRadius, { 0.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());

	}

	return steering;
}

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

	if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_EvadeRange, { 0.f, 0.f, 1.f, 0.5f }, 0.40f);
	}

	return steering;
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	TargetData altTarget = m_Target;
	if (m_Target.LinearVelocity.Magnitude() > 0.f)
	{
		altTarget.Position += m_Target.LinearVelocity * (m_Target.Position - pAgent->GetPosition()).Magnitude() / pAgent->GetMaxLinearSpeed();
	}

	if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawSolidCircle(altTarget.Position, 0.25f, {}, { 1.f, 0.f, 1.f }, DEBUGRENDERER2D->NextDepthSlice());
	}

	return Seek::CalculateSteering(deltaT, pAgent, altTarget);
}

//SteeringOutput Hide::CalculateSteering(float deltaT, SteeringAgent* pAgent)
//{
//
//
//	return;
//}
//
//SteeringOutput AvoidObstacle::CalculateSteering(float deltaT, SteeringAgent* pAgent)
//{
//
//
//	return;
//}
 
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	m_Angle += (Elite::randomFloat(0, 1) * (m_MaxAngle)) - (m_MaxAngle / 2.f) /** deltaT*/;
	Elite::ClampRef(m_Angle, Elite::ToRadians(-90), Elite::ToRadians(90));

	const Elite::Vector2 wanderCircleCenter{ pAgent->GetPosition() + pAgent->GetLinearVelocity().GetNormalized() * m_Offset };
	const Elite::Vector2 vecFromCenter{ cosf(m_Angle) * m_Radius,sinf(m_Angle) * m_Radius };

	m_Target.Position = (wanderCircleCenter + vecFromCenter);



	if (pAgent->GetDebugRenderingEnabled())
	{

		DEBUGRENDERER2D->DrawCircle(wanderCircleCenter, m_Radius, { 0.f, 0.f, 1.f, 0.5f }, 0.40f);
		DEBUGRENDERER2D->DrawSolidCircle(m_Target.Position, 0.1f, {}, { 0.f, 1.f, 0.f }, 0.40f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), m_Offset, { 0.f, 0.f, 1.f, 0.5f }, 0.40f);

		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetLinearVelocity(), pAgent->GetLinearVelocity().Magnitude(), { 1.f, 0.f, 1.f, 0.5f }, 0.40f);
	}

	return Seek::CalculateSteering(deltaT, pAgent);
}

//AvoidObstacle::AvoidObstacle(std::vector<Obstacle*> obstacles)
//	: m_Obstacles{ obstacles }
//{
//}

//Hide::Hide(std::vector<Obstacle*> obstacles)
//	: m_Obstacles{ obstacles }
//{
//}
