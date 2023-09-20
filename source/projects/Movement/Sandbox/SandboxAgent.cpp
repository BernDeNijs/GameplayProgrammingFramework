#include "stdafx.h"
#include "SandboxAgent.h"

using namespace Elite;

SandboxAgent::SandboxAgent() : BaseAgent()
{
	m_Target = GetPosition();
}

void SandboxAgent::Update(float dt)
{
	constexpr float speed = 1000.f;


	const auto pos = GetPosition();
	const auto dir = m_Target - pos;
	const auto velocity = dir.GetNormalized() * speed * dt;

	SetLinearVelocity(velocity);

	DEBUGRENDERER2D->DrawSegment(pos, pos+(dir.GetNormalized()*5.f), Color{0.f,1.f,0.f});
	//Orientation
	AutoOrient();
}

void SandboxAgent::Render(float dt)
{
	BaseAgent::Render(dt); //Default Agent Rendering
}

void SandboxAgent::AutoOrient()
{
	//Determine angle based on direction
	Vector2 velocity = GetLinearVelocity();
	if (velocity.Magnitude() > 0)
	{
		
		SetRotation(VectorToOrientation(velocity));
	}
}