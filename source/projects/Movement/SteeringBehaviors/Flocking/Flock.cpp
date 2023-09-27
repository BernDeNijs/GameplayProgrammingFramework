#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"

using namespace Elite;
//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
{
	m_Agents.resize(m_FlockSize);
	m_Neighbors.resize(m_FlockSize - 1);
	//TODO: initialize the flock and the memory pool
	// Initialize steering
	m_pSeparationBehavior = new Separation(this);
	m_pCohesionBehavior = new Cohesion(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pSeekBehavior = new Seek();
	m_pWanderBehavior = new Wander();



	m_pBlendedSteering = new BlendedSteering({
		{m_pSeparationBehavior,0.5f},
		{m_pCohesionBehavior,0.5f},
		{m_pVelMatchBehavior,0.5f},
		{m_pSeekBehavior,0.5f},
		{m_pWanderBehavior,0.5f},
		});


	for (int idx = 0; idx < m_FlockSize; idx++)
	{
		//m_Agents.emplace_back(new SteeringAgent());
		m_Agents[idx] = new SteeringAgent();
		m_Agents[idx]->SetMass(0.f);
		m_Agents[idx]->SetMaxLinearSpeed(15.0f);
		m_Agents[idx]->SetPosition(randomVector2(m_WorldSize).GetAbs());
		m_Agents[idx]->SetRotation(randomFloat());
		m_Agents[idx]->SetSteeringBehavior(m_pBlendedSteering);
	}


	
}

Flock::~Flock()
{
	//TODO: Cleanup any additional data

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);

	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();
	m_Neighbors.clear();
}

void Flock::Update(float deltaT)
{



	// TODO: update the flock
	// for every agent:
		// register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
		// update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
		// trim the agent to the world
	for (const auto& agent : m_Agents)
	{
		RegisterNeighbors(agent);
		agent->Update(deltaT);
		agent->TrimToWorld(m_WorldSize);
	}

}

void Flock::Render(float deltaT)
{
	// TODO: Render all the agents in the flock
	
	if (true) //TODO: switch with imGUI checkbox
		RenderNeighborhood();
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	//TODO: implement ImGUI checkboxes for debug rendering here

	ImGui::Text("Behavior Weights");
	ImGui::Spacing();

	//TODO: implement ImGUI sliders for steering behavior weights here
	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RenderNeighborhood()
{
	// TODO: Implement
	// Register the neighbors for the first agent in the flock
	// DebugRender the neighbors in the memory pool
}


void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	m_NrOfNeighbors = 0;
	for (const auto& agent : m_Agents)
	{
		if (agent == pAgent) continue; //changed
		const float distanceSquared = pAgent->GetPosition().DistanceSquared(agent->GetPosition());
		
		if ( distanceSquared <= m_NeighborhoodRadius * m_NeighborhoodRadius)
		{
			m_Neighbors[m_NrOfNeighbors] = agent;
			++m_NrOfNeighbors;
		}
	}
}

Vector2 Flock::GetAverageNeighborPos() const
{
	Vector2 avgPosition = Elite::ZeroVector2;

	for (int i = 0; i < m_NrOfNeighbors; i++)
	{
		avgPosition += m_Neighbors[i]->GetPosition();
	}
	avgPosition /= static_cast<float>( m_NrOfNeighbors);

	// TODO: Implement
	
	return avgPosition;
}

Vector2 Flock::GetAverageNeighborVelocity() const
{
	Vector2 avgVelocity = Elite::ZeroVector2;

	for (int i = 0; i < m_NrOfNeighbors; i++)
	{
		avgVelocity += m_Neighbors[i]->GetPosition();
	}
	avgVelocity /= static_cast<float>(m_NrOfNeighbors);
	
	// TODO: Implement

	return avgVelocity;
}
void Flock::SetTarget_Seek(const TargetData& target)
{
	m_pSeekBehavior->SetTarget(target);
	// TODO: Implement
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
